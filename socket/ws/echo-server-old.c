/*
 * Copyright (c) 2014 Putilov Andrey
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "websocket.h"
#include "socket/socket_tcp.h"

//#define PORT 8088
#define BUF_LEN 0xFFFF
#define PACKET_DUMP

static uint8_t gBuffer[BUF_LEN];

__attribute__((noreturn))
static void
error(const char* msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}

static int
safeSend(int clientSocket, const uint8_t* buffer, size_t bufferSize)
{
    #ifdef PACKET_DUMP
	printf("out packet:\n");
//    fwrite(buffer, 1, bufferSize, stdout);
//    printf("\n");
    #endif
	ssize_t written = send(clientSocket, buffer, bufferSize, 0);
	if (written == -1)
	{
		close(clientSocket);
		perror("send failed");
		return EXIT_FAILURE;
	}
	else if ((size_t)written != bufferSize)
	{
		close(clientSocket);
		perror("written not all bytes");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

static void
clientWorker(int clientSocket)
{
	memset(gBuffer, 0, BUF_LEN);
	size_t readedLength = 0;
	size_t frameSize = BUF_LEN;
	enum wsState state = WS_STATE_OPENING;
	uint8_t* data = NULL;
	size_t dataSize = 0;
	enum wsFrameType frameType = WS_INCOMPLETE_FRAME;
	struct handshake hs;
	nullHandshake(&hs);

    #define prepareBuffer frameSize = BUF_LEN; memset(gBuffer, 0, BUF_LEN);
    #define initNewFrame frameType = WS_INCOMPLETE_FRAME; readedLength = 0; memset(gBuffer, 0, BUF_LEN);

	while (frameType == WS_INCOMPLETE_FRAME)
	{
		ssize_t readed = recv(clientSocket, gBuffer + readedLength, BUF_LEN - readedLength, 0);
		if (readed < 1)
		{
			close(clientSocket);
			perror("recv failed");
			return;
		}
#ifdef PACKET_DUMP
		printf("in packet:\n");
//        fwrite(gBuffer, 1, readed, stdout);
		printf("\n");
#endif
		readedLength += (size_t)readed;
		assert(readedLength <= BUF_LEN);

		if (state == WS_STATE_OPENING)
			frameType = wsParseHandshake(gBuffer, readedLength, &hs);
		else
			frameType = wsParseInputFrame(gBuffer, readedLength, &data, &dataSize);


		if ((frameType == WS_INCOMPLETE_FRAME && readedLength == BUF_LEN) || frameType == WS_ERROR_FRAME)
		{
			if (frameType == WS_INCOMPLETE_FRAME)
				printf("buffer too small\n");
			else
				printf("error in incoming frame\n");

			if (state == WS_STATE_OPENING)
			{
				prepareBuffer;

				int printed = sprintf((char*)gBuffer,
						      "HTTP/1.1 400 Bad Request\r\n"
						      "%s%s\r\n\r\n",
						      versionField,
						      version);
				if (printed < 0)
					perror("error while printing into buffer\n");
				else
				{
					frameSize = (size_t)printed;
					safeSend(clientSocket, gBuffer, frameSize);
				}
				break;
			}
			else
			{
				prepareBuffer;
				wsMakeFrame(NULL, 0, gBuffer, &frameSize, WS_CLOSING_FRAME);
				if (safeSend(clientSocket, gBuffer, frameSize) == EXIT_FAILURE)
					break;
				state = WS_STATE_CLOSING;
				initNewFrame;
			}
		}


		if (state == WS_STATE_OPENING)
		{
			assert(frameType == WS_OPENING_FRAME);
			if (frameType == WS_OPENING_FRAME)
			{
				// if resource is right, generate answer handshake and send it
				if (strcmp(hs.resource, "/echo") != 0)
				{
					int printed = sprintf((char*)gBuffer, "HTTP/1.1 404 Not Found\r\n\r\n");
					if (printed < 0)
					{
						perror("Error while printing to buffer\n");
						return;
					}
					else
						frameSize = (size_t)safeSend(clientSocket, gBuffer, frameSize);
					break;
				}

				prepareBuffer;
				wsGetHandshakeAnswer(&hs, gBuffer, &frameSize);
				freeHandshake(&hs);
				if (safeSend(clientSocket, gBuffer, frameSize) == EXIT_FAILURE)
					break;
				state = WS_STATE_NORMAL;
				initNewFrame;
			}
		}
		else
		{
			if (frameType == WS_CLOSING_FRAME)
			{
				if (state == WS_STATE_CLOSING)
					break;
				else
				{
					prepareBuffer;
					wsMakeFrame(NULL, 0, gBuffer, &frameSize, WS_CLOSING_FRAME);
					safeSend(clientSocket, gBuffer, frameSize);
					break;
				}
			}
			else if (frameType == WS_TEXT_FRAME)
			{
				uint8_t* recievedString = NULL;
				recievedString = malloc(dataSize + 1);
				assert(recievedString);
				memcpy(recievedString, data, dataSize);
				recievedString[ dataSize ] = 0;

				prepareBuffer;
				wsMakeFrame(recievedString, dataSize, gBuffer, &frameSize, WS_TEXT_FRAME);
				free(recievedString);
				if (safeSend(clientSocket, gBuffer, frameSize) == EXIT_FAILURE)
					break;
				initNewFrame;
			}
		}
	} // read/write cycle

	close(clientSocket);
}

static TCPSocket* server_socket;

static void
handle_sigint(int signal)
{
	/* close server socket, otherwise it takes some time to reconnect */
	close(server_socket->socket.fd);
	TCPSocket_free(server_socket);
}

int
main(int argc, char** argv)
{
	signal(SIGINT, handle_sigint);

	const char* ip = "127.0.0.1";
	uint16_t port =  (uint16_t)atoi(argv[1]);
	server_socket = TCPSocket_new(ip, port);

	if (Socket_serve((Socket*)server_socket) == SOCKET_LISTEN)
	{
		printf("Listening to socket: %s:%d\n", ip, port);
		while (TRUE)
		{
			int clientSocket = accept(server_socket->socket.fd, NULL, NULL);
			if (clientSocket == -1)
				error("accept failed");
			clientWorker(clientSocket);
			printf("disconnected\n");
		}
	}
	else
		printf("Failed to listen to socket: %s:%d\n", ip, port);
//	close(listenSocket);
	return EXIT_SUCCESS;
}

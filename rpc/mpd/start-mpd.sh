#!/bin/sh

MPD_DIR=/tmp/mpd

mkdir -p ${MPD_DIR}/playlists
mkdir -p ${MPD_DIR}/music

mpd --no-daemon --stderr -v rpc/mpd/mpd.conf
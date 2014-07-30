var ws = require("nodejs-websocket");

var url = process.argv[2] ? process.argv[2] : "ws://localhost:8088";

var c = ws.connect(url, function() {
    // ignore
});

var connected = false;
var command = null;
var exit = false;

function prompt()
{
    if (Boolean(process.stdin.isTTY)) 
        process.stdout.write("\n>> ");
}

c.on("text", function(text) {
    process.stdout.write(text);
    
    if (exit)
        process.exit(0);
    else
       prompt(); 

});

c.on("connect", function() {
    connected = true;
    prompt();
    
    if (command != null) {
        c.sendText(command);
    }
});

process.stdin.on('readable', function() {
    var chunk = process.stdin.read();
    if (chunk !== null) {
        if (connected) {
            c.sendText(chunk);
        } else {
            command = chunk;
        }
    }
});

process.stdin.on('end', function() {
    exit = true;
});

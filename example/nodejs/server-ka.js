// keep-alive server implementation in node.js

var util    = require('util');
var http    = require('http');
var printf  = require('printf');
var fs      = require('fs');

var onQuit = function() {
    console.log('user sends a quit request.\nGoodbye.\n');
    server.close();
    // try to remove UNIX socket file
    if ( fs.existsSync(defaultListeningPort) )
        fs.unlinkSync(defaultListeningPort);
    process.exit(0);
}

// main

var defaultListeningPort = 10022;
// tcp port number or UNIX socket path
if ( process.argv.length >= 3 )
    defaultListeningPort = process.argv[2];


// create the HTTP server
var server = http.createServer(function(req, res){
    var body = '';

    req.on('data', function(chunk){
        body += chunk;
    });

    req.on('end', function(){

        var jsonObj = {};
        try {
            jsonObj = JSON.parse(body);
        } catch(err){
            console.log('parsing body to JSon failed. ' + err);
        }

        if ( jsonObj != null ) {
            if ( jsonObj.name != null    &&    jsonObj.arguments != null ) {

                var args = jsonObj.arguments;
                if ( jsonObj.name == 'add'    &&   args.length > 0 ) {
                    var total = 0;
                    for (var i = 0;    i < args.length;   i++) {
                        total += args[i];
                    }

                    jsonObj.arguments = total;

                    var stringified = JSON.stringify(jsonObj);
                    res.setHeader('content-length', stringified.length);

                    res.writeHead(200);
                    res.end(stringified);
                    return;
                }
            }
        }

        res.writeHead(400);
        res.end('bad request: the json value is not present or invalid!\n');
    });

    if ( req.headers != null    &&    req.headers['command'] == 'quit' ) {
        onQuit();
    }
    
}).listen(defaultListeningPort); // listen on TCP or UNIX socket

server.on('connection', function(socket) {
    console.log('new connection from: ' + socket.remoteAddress + ':' +
    socket.remotePort);
});

process.on('SIGINT', onQuit);

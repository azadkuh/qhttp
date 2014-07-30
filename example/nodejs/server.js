// server implementation in node.js
// have a look at example/benchmark for `QHttp` version in c++11 and Qt.

var util    = require('util');
var http    = require('http');
var printf  = require('printf');
var fs      = require('fs');

// utility functions
var dateTimeString = function(date) {
    return printf('%04d-%02d-%02d %02d:%02d:%02d',
                    date.getFullYear(), date.getMonth(), date.getDate(),
                    date.getHours(), date.getMinutes(), date.getSeconds());
}

var onQuit = function() {
    console.log('user sends a quit request.\nGoodbye.\n');
    server.close();
    // try to remove UNIX socket file
    if ( fs.existsSync(defaultListeningPort) )
        fs.unlinkSync(defaultListeningPort);
    process.exit(0);
}

// main

var defaultListeningPort = 8080;
// tcp port number or UNIX socket path
if ( process.argv.length >= 3 )
    defaultListeningPort = process.argv[2];

var totalConnections     = 0;
var intervalConnections  = 0;
var timeElapsed          = process.hrtime();

// shows a performance log in every 10sec.
setInterval(function(){
    totalConnections   += intervalConnections;
    var timeDiff        = process.hrtime(timeElapsed);
    var miliSec         = timeDiff[0]*1000 + timeDiff[1]/1000000;
    var aveTps          = intervalConnections * 1000 / miliSec;

    console.log(printf('%s,%.1f,%u,%u,%u',
                        dateTimeString(new Date), aveTps,
                        miliSec, intervalConnections, totalConnections));

    intervalConnections = 0;
    timeElapsed         = process.hrtime();
}, 10000);

// create the HTTP server
var server = http.createServer(function(req, res){
    var body = '';

    req.on('data', function(chunk){
        body += chunk;
    });

    req.on('end', function(){
        intervalConnections++;
        res.setHeader('connection', 'close');

        var jsonObj = {};
        try {
            jsonObj = JSON.parse(body);
        } catch(err){
        }
        
        if ( jsonObj != null ) {
            if ( jsonObj.command != null    &&
                jsonObj.clientId != null    &&    jsonObj.requestId != null ) {

                jsonObj.requestId++;
                jsonObj.command = 'response';

                var stringified = JSON.stringify(jsonObj);
                res.setHeader('content-length', stringified.length);

                res.writeHead(200);
                res.end(stringified);
                return;
            }
        }

        res.writeHead(400);
        res.end('bad request: the json value is not present or invalid!\n');
    });

    if ( req.headers != null    &&    req.headers['command'] == 'quit' ) {
        onQuit();
    }
}).listen(defaultListeningPort); // listen on TCP or UNIX socket

console.log('\nDateTime,AveTps,miliSecond,Count,TotalCount\n');
process.on('SIGINT', onQuit);

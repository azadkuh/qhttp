// server implementation in node.js
// have a look at example/benchmark for `QHttp` version in c++11 and Qt.

var util    = require('util');
var http    = require('http');
var printf  = require('printf');

var defaultListeningPort = 8080;
// tcp port numver or UNIX socket path
if ( process.argv.length >= 3 )
    defaultListeningPort = process.argv[2];


var totalConnections     = 0;
var intervalConnections  = 0;
var timeElapsed          = process.hrtime();

var server = http.createServer(function(req, res){
    var body = '';

    req.on('data', function(chunk){
        body += chunk;
    });

    req.on('end', function(){
        intervalConnections++;
        res.setHeader('connection', 'close');

        var jsonObj = JSON.parse(body);
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
        console.log('user sends a quit request.\nGoodbye.\n');
        server.close();
        process.exit(0);
    }

});

setInterval(function(){
    totalConnections   += intervalConnections;
    var timeDiff        = process.hrtime(timeElapsed);
    var miliSec         = timeDiff[0]*1000 + timeDiff[1]/1000000;
    var aveTps          = intervalConnections * 1000 / miliSec;
    var now             = new Date();

    console.log(printf('%04d-%02d-%02d %02d:%02d:%02d,%.1f,%u,%u,%u',
                        now.getFullYear(), now.getMonth(), now.getDate(),
                        now.getHours(), now.getMinutes(), now.getSeconds(),
                        aveTps, miliSec, intervalConnections, totalConnections));

    intervalConnections = 0;
    timeElapsed         = process.hrtime();
}, 10000);

server.listen(defaultListeningPort); // listen on TCP or UNIX socket
console.log('\nDateTime,AveTps,miliSecond,Count,TotalCount\n');

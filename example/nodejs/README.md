# nodejs

this sample implements the server mode of [benchmark](../benchmardk/README.md) app in `Node.js`. A single thread `c++` version is also demonstrated.


## setup
```bash
$> cd ./example/nodejs

# resolve all the dependencies
$> npm install
```


## usage
to start the server:
```bash
$> node server.js [port/socket]

# start server on a tcp socket: (port = 8080)
$> node server.js 8080

# start server on a UNIX (local) soket (socket path = /tmp/qhttp.benchmark)
$> node server.js /tmp/qhttp.benchmark
```

then start attacking clients by [benchmark](../benchmardk/README.md) app:
```bash
# tcp socket
$> ./xbin/benchmark client --port 8080 --count 50 --timeout 1 --threads 2

# UNIX (local) socket
$> ./xbin/benchmark client --backend local --count 50 --timeout 1 --threads 2
```

## javascript source
this fragment shows the core of HTTP server used in benchmarking:
```js
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
```

## QHttp source
`main.cpp` emulates the `node.js` sample by `QHttp` under `c++11`:
```cpp
// QString iportOrPath: a tcp port number or a UNIX socket path.
iserver.listen(iportOrPath, [this](QHttpRequest* req, QHttpResponse* res){
    req->collectData(512);

    req->onEnd([this, req, res](){
        iintervalConnections++;
        res->addHeader("connection", "close");

        const QByteArray& body = req->collectedData();
        QJsonObject root       = QJsonDocument::fromJson(body).object();
        if ( !root.isEmpty()  && root.contains("command")  &&
             root.contains("clientId")  && root.contains("requestId") ) {

            root["command"]     = "response";
            root["requestId"]   = root["requestId"].toInt() + 1;
            QByteArray doc      = QJsonDocument(root).toJson(QJsonDocument::Compact);

            res->addHeader("content-length", QByteArray::number((int)strlen(doc)));
            res->setStatusCode(qhttp::ESTATUS_OK);
            res->end(doc);
            return;

        }

        res->setStatusCode(qhttp::ESTATUS_BAD_REQUEST);
        res->end("bad request: the json value is not present or invalid!\n");
    });

    if ( req->headers().keyHasValue("command", "quit" ) ) {
        puts("user sends a quit request.\nGoodbye.");
        quit();
    }
});

```

apart from `JSON` parsing and building, writing HTTP server application in `QHttp` is as easy as `node.js`.
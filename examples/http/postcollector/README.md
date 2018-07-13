# POST (data) collector

this example shows how to collect data from clients in POST/PUT/... commands


## usage

to start the http server:
```bash
$> ./postcollector 8090 #listening port
```

to stop the server just press the `ctrl+c`.

the server will dump each POST data into a file (`dump.bin`):
```cpp
server.listen(port, [](QHttpRequest* req, QHttpResponse* res) {
    req->collectData(8*1024*1024); // maximum 8MB of data for each post request
    // the better approach is to use req->onData(...)

    req->onEnd([req, res](){
        res->setStatusCode(qhttp::ESTATUS_OK);
        res->addHeader("connection", "close"); // optional header (added by default)

        int size = req->collectedData().size();
        auto message = [size]() -> QByteArray {
            if ( size == 0 )
                return "Hello World!\n";

            char buffer[65] = {0};
            qsnprintf(buffer, 64, "Hello!\nyou've sent me %d bytes!\n", size);
            return buffer;
        };

        res->end(message());  // response body data

        if ( size > 0 ) { // dump the incoming data into a file
            QFile f("dump.bin");
            if ( f.open(QFile::WriteOnly) )
                f.write(req->collectedData());
        }
    });
});
```

to POST some data in a handy way:
```bash
$> curl -X POST --data-binary "@sample.jpeg" ip:port
```
where `sample.jpeg` should be in current working directory.

to check the integrity of transmitted data:
`SHA1(dump.bin) == SHA1(sample.jpeg)` shall return `true`.


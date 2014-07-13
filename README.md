# QHttp
*this doc is under construction.*


## Table of contents
- [About](#about)
- [Features](#features)
- [Setup](#setup)
- [Source tree](#source-tree)
- [Examples](#examples)
- [Disclaimer](#disclaimer)
- [License](#license)

## About
[TOC](#table-of-contents)

QHttp is a lightweight, asynchronous and fast HTTP library, containing both server and client side classes for managing connections, parsing and building HTTP requests and responses.

* this project is inspired by [nikhilm/qhttpserver](https://github.com/nikhilm/qhttpserver) effort to implement a Qt HTTP server. QHttp pushes the idea further by implementing client classes and better memory management, c++11, lambda connections, clean API, ...

* the fantastic [joyent/http-parser](https://github.com/joyent/http-parser) is the core parser of HTTP requests (server mode) and responses (client mode). I have tried to keep the API similar to the [Node.js](http://nodejs.org/api/http.html)' http module.

* the objective of QHttp is being light weight with a simple API for Qt developers to implement RESTful web services in private (internal) zones. for a fast c++ Json parser / builder, have a look at [azadkuh/gason++](https://github.com/azadkuh/gason--)


a HelloWorld HTTP server by QHttp looks like:
``` cpp
int main(int argc, char** argv) {

    QCoreApplication app(argc, argv);

    using namespace qhttp::server;

    QHttpServer server(&app);
    if ( !server.listen(8080) ) { // listening port
        fprintf(stderr, "failed. some useful error message!\n");
        return -1;
    }

    QObject::connect(&server,  &QHttpServer::newRequest,
                    [](QHttpRequest* req, QHttpResponse* res) {
        // status 200
        res->setStatusCode(qhttp::ESTATUS_OK);
        
        // it's the default header, this line can be omitted.
        res->addHeader("connection", "close"); 
        
        // body data of the response
        res->end("Hello World!\n");

        // when "connection: close" (default mode), the req and res instances
        //  will be deleted automatically as soon as the socket `disconnected()`.
    });


    // application's main event loop
    return app.exec();
}
```



## Features
[TOC](#table-of-contents)

* separate `namespace`s for server and client classes.

* HTTP server classes: [QHttpServer](./src/qhttpserver.hpp), [QHttpConnection](./src/qhttpserverconnection.hpp), [QHttpRequest](./src/qhttpserverrequest.hpp) and [QHttpResponse](./src/qhttpserverresponse.hpp).

* HTTP client classes: [QHttpClient](./src/qhttpclient.hpp), [QHttpRequest](./src/qhttpclientrequest.hpp) and [QHttpResponse](./src/qhttpclientresponse.hpp).

* **automatic memory management** of objects. Instances of connections, requests and replies will be deleted automatically when socket drops or disconnected.

* **PIMPL** (Private classes) to achieve better ABI compatibility and cleaner API.

* API is quite similar to **Node.js** http module.

* the only dependencies are: [Qt 5](http://qt-project.org/downloads), [c++11](http://en.wikipedia.org/wiki/C%2B%2B11) and [joyent/http-parser](https://github.com/joyent/http-parser)

* **high throughput**, I have tried the QHttp and [gason++](https://github.com/azadkuh/gason--) to implement a REST/Json web service on an Ubuntu VPS (dual core + 512MB ram) with peak TPS > 5000 (stress test)

* a simple benchmarking tool, implementing both a RESTful/Json server and client.

* Tested under **Linux** (Ubuntu 12.04 LTS, 14.04 LTS) and **OS X** (10.9). Easily portable where ever Qt 5 works. I have no **Windows** machine (or time or interest), but this lib should work just fine under Windows, although I've not tried by myself.

## Setup
[TOC](#table-of-contents)

instructions:
```bash
# first clone the QHttp:
$> git clone --depth=1 https://github.com/azadkuh/qhttp.git

# then clone the dependency:
$> mkdir -p qhttp/3rdparty
$> cd qhttp/3rdparty
$> git clone --depth=1 https://github.com/joyent/http-parser.git
$> cd ..

# now build the library and the examples
$> qmake qhttp.pro
$> make -j 8
```

## Source tree
[TOC](#table-of-contents)



## examples
[TOC](#table-of-contents)



## Disclaimer
[TOC](#table-of-contents)

* Implementing a lightweight and simple HTTP server/client in Qt is the main purpose of QHttp.
* There are lots of features in a full blown HTTP server which are out of scope of this small library, although those can be added on top of QHttp.
* I'm a busy person.

## License
[TOC](#table-of-contents)

Distributed under the MIT license. Copyright (c) 2014, Amir Zamani.




> If you have any ideas, critiques, suggestions or whatever you want to call it, please open an issue. I'll be happy to hear from you what you'd see in this lib. I think about all suggestions, and I try to add those that make sense.

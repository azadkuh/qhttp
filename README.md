# QHttp


### Table of contents
- [About](#about)
- [Features](#features)
- [Sample codes](#sample-codes)
- [Setup](#setup)
- [Source tree](#source-tree)
- [Disclaimer](#disclaimer)
- [License](#license)

## About
[TOC](#table-of-contents)

`QHttp` is a lightweight, asynchronous and fast HTTP library, containing both server and client side classes for managing connections, parsing and building HTTP requests and responses.

* this project is inspired by [nikhilm/qhttpserver](https://github.com/nikhilm/qhttpserver) effort to implement a Qt HTTP server. `QHttp` pushes the idea further by implementing client classes and better memory management, c++11, lambda connections, clean API, ...

* the fantastic [joyent/http-parser](https://github.com/joyent/http-parser) is the core parser of HTTP requests (server mode) and responses (client mode). I have tried to keep the API similar to the [Node.js](http://nodejs.org/api/http.html)' http module.

* the objective of `QHttp` is being light weight with a simple API for Qt developers to implement RESTful web services in private (internal) zones. for a fast c++ Json parser / builder, have a look at [azadkuh/gason++](https://github.com/azadkuh/gason--)



## Features
[TOC](#table-of-contents)

* separate `namespace`s for server and client classes.

* HTTP server classes: [QHttpServer](./src/qhttpserver.hpp), [QHttpConnection](./src/qhttpserverconnection.hpp), [QHttpRequest](./src/qhttpserverrequest.hpp) and [QHttpResponse](./src/qhttpserverresponse.hpp).

* HTTP client classes: [QHttpClient](./src/qhttpclient.hpp), [QHttpRequest](./src/qhttpclientrequest.hpp) and [QHttpResponse](./src/qhttpclientresponse.hpp).

* **automatic memory management** of objects. Instances of connections, requests and replies will be deleted automatically when socket drops or disconnected.

* **PIMPL** (Private classes) to achieve better ABI compatibility and cleaner API.

* API is quite similar to **Node.js** http module. have a look at [sample code](#sample-codes).


* the only dependencies are: [Qt 5](http://qt-project.org/downloads), [c++11](http://en.wikipedia.org/wiki/C%2B%2B11) and [joyent/http-parser](https://github.com/joyent/http-parser)

* **high throughput**, I have tried the `QHttp` and [gason++](https://github.com/azadkuh/gason--) to implement a REST/Json web service on an Ubuntu VPS (dual core + 512MB ram) with peak TPS > 5000 (stress test)

* a simple benchmarking tool, implementing both a RESTful/Json server and client.

* Tested under **Linux** (Ubuntu 12.04 LTS, 14.04 LTS) and **OS X** (10.9). Easily portable where ever Qt 5 works. I have no **Windows** machine (or time or interest), but this lib should work just fine under Windows, although I've not tried by myself.


## Sample codes
[TOC](#table-of-contents)

a HelloWorld **HTTP server** by `QHttp` looks like:
``` cpp
int main(int argc, char** argv) {

    QCoreApplication app(argc, argv);

    using namespace qhttp::server;

    QHttpServer server(&app);
    // listening on 0.0.0.0:8080
    server.listen(QHostAddress::Any, 8080, [](QHttpRequest* req, QHttpResponse* res) {

        res->setStatusCode(qhttp::ESTATUS_OK);      // status 200
        res->addHeader("connection", "close");      // it's the default header, this line can be omitted.
        res->end("Hello World!\n");                 // response body data

        // when "connection: close", the req and res will be deleted automatically.
    });


    if ( !server.isListening() ) {
        fprintf(stderr, "failed. can not listen at port 8080!\n");
        return -1;
    }

    // application's main event loop
    return app.exec();
}
```

to request weather information by **HTTP client**:
```cpp
int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    using namespace qhttp::client;

    QHttpClient  client(&app);
    QByteArray   httpBody;

    QUrl weatherUrl("http://api.openweathermap.org/data/2.5/weather?q=tehran,ir&units=metric&mode=xml");

    client.request(qhttp::EHTTP_GET, weatherUrl, [&httpBody](QHttpResponse* res) {
        // response handler, called when the HTTP headers of the response are ready

        // gather HTTP response data
        res->onData([&httpBody](const QByteArray& chunk) {
            httpBody.append(chunk);
        });

        // called when all data in HTTP response have been read.
        res->onEnd([&httpBody]() {
            // print the XML body of the response
            puts("\n[incoming response:]");
            puts(httpBody.constData());
            puts("\n\n");

            QCoreApplication::instance()->quit();
        });

        // just for fun! print headers:
        puts("\n[Headers:]");
        const qhttp::THeaderHash& hs = res->headers();
        for ( auto cit = hs.constBegin(); cit != hs.constEnd(); cit++) {
            printf("%s : %s\n", cit.key().constData(), cit.value().constData());
        }
    });


    return app.exec();
}
```

## Setup
[TOC](#table-of-contents)

instructions:
```bash
# first clone this repository:
$> git clone --depth=1 https://github.com/azadkuh/qhttp.git
$> cd qhttp

# prepare dependencies:
$> ./update-dependencies.sh

# now build the library and the examples
$> qmake qhttp.pro
$> make -j 8
```

## Source tree
[TOC](#table-of-contents)


* **`3rdparty/`**:
will contain `http-parser` source tree as the only dependency.
this directory is created by setup. see also: [setup](#setup).

* **`example/`**:
contains some sample applications representing the `QHttp` usage:
    * **`helloworld/`**:
    the HelloWorld example of `QHttp`, both server + client are represented.
    see: [README@helloworld](./example/helloworld/README.md)

    * **`basic-server/`**:
    a basic HTTP server shows how to collect the request body, and respond to the clients.
    see: [README@basic-server](./example/basic-server/README.md)
    

    * **`benchmark/`**:
    a simple utility to measure the throughput (requests per second) of `QHttp` as a REST/Json server. this app provides both the server and attacking clinets.
    see: [README@benchmark](./example/benchmark/README.md)
    

* **`src/`**:
holds the source code of `QHttp`. server classes are prefixed by `qhttpserver*` and client classes by `qhttpclient*`.
    * **`private/`**:
    Private classes of the library. see: [d-pointers](https://qt-project.org/wiki/Dpointer).

* **`tmp/`**:
a temporary directory which is created while `make`ing the library and holds all the `.o`, `moc files`, etc.

* **`xbin/`**:
all the executable binaries will be placed on this folder by `make`.




## Disclaimer
[TOC](#table-of-contents)

* Implementing a lightweight and simple HTTP server/client in Qt is the main purpose of `QHttp`.

* There are lots of features in a full blown HTTP server which are out of scope of this small library, although those can be added on top of `QHttp`.

* The client classes are by no mean designed as a `QNetworkAccessManager` replacement. `QHttpClient` is simpler and lighter, for serious scenarios just use `QNetworkAccessManager`.

* I'm a busy person.


> If you have any ideas, critiques, suggestions or whatever you want to call it, please open an issue. I'll be happy to hear from you what you'd see in this lib. I think about all suggestions, and I try to add those that make sense.


## License
[TOC](#table-of-contents)

Distributed under the MIT license. Copyright (c) 2014, Amir Zamani.


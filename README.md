# QHttp


### Table of contents
- [About](#about)
- [Sample codes](#sample-codes)
- [Features](#features)
- [Setup](#setup)
- [Multi-threading](#multi-threading)
- [Source tree](#source-tree)
- [Disclaimer](#disclaimer)
- [License](#license)

## About
[TOC](#table-of-contents)

`QHttp` is a lightweight, asynchronous and fast HTTP library in `c++14 / Qt5`,
containing both server and client side classes for managing connections,
parsing and building HTTP requests and responses.

- the objective of `QHttp` is being light weight with a simple API for Qt
developers to implement RESTful web services in private (internal) zones.
[more](#disclaimer)
- by using `std::function` and `c++14 generic lambda`, the API is intentionally similar
to the [Node.js' http module](http://nodejs.org/api/http.html). Asynchronous
and non-blocking HTTP programming is quite easy with `QHttp`. have a look at
[sample codes](#sample-codes).
- the fantastic [nodejs/http-parser](https://github.com/nodejs/http-parser)
(which is a single pair of `*.h/*.c` files) is the only dependency of the
`QHttp`.

> **attention**: c++14 is the minimum requirement for version 3.0+
> please see [releases](https://github.com/azadkuh/qhttp/releases)

This project was inspired by
[nikhilm/qhttpserver](https://github.com/nikhilm/qhttpserver) effort to
implement a Qt HTTP server. `QHttp` pushes the idea further by implementing
client side classes, better memory management, a lot more Node.js-like API, ...

## Sample codes
[TOC](#table-of-contents)

a HelloWorld **HTTP server** by `QHttp` looks like:
```cpp
int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);

    using namespace qhttp::server;
    QHttpServer server(&app);
    server.listen( // listening on 0.0.0.0:8080
        QHostAddress::Any, 8080,
        [](QHttpRequest* req, QHttpResponse* res) {
            // http status 200
            res->setStatusCode(qhttp::ESTATUS_OK);
            // the response body data
            res->end("Hello World!\n");
            // automatic memory management for req/res
    });

    if ( !server.isListening() ) {
        qDebug("failed to listen");
        return -1;
    }

    return app.exec();
}
```

to request weather information by **HTTP client**:
```cpp
int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);

    using namespace qhttp::client;
    QHttpClient client(&app);
    QUrl        weatherUrl("http://wttr.in/tehran");

    client.request(qhttp::EHTTP_GET, weatherUrl, [](QHttpResponse* res) {
        // response handler, called when the incoming HTTP headers are ready

        // gather HTTP response data (HTTP body)
        res->collectData();

        // when all data in HTTP response have been read:
        res->onEnd([&]() {
            writeTo("weather.html", res->collectedData());

            // done! now quit the application
            qApp->quit();
        });

        // just for fun! print incoming headers:
        qDebug("\n[Headers:]");
        res->headers().forEach([](auto cit) {
            qDebug("%s : %s", cit.key().constData(), cit.value().constData());
        });
    });

    // set a timeout for the http connection
    client.setConnectingTimeOut(10000, []{
        qDebug("connecting to HTTP server timed out!");
        qApp->quit();
    });

    return app.exec();
}
```


## Features
[TOC](#table-of-contents)

- the only dependencies are: `Qt5`, `c++14` and the `http-parser`
- both TCP and UNIX (local) sockets are supported as backend.
- separate `namespace`s for server and client classes.
- HTTP server classes: [QHttpServer](./src/qhttpserver.hpp),
 [QHttpConnection](./src/qhttpserverconnection.hpp),
 [QHttpRequest](./src/qhttpserverrequest.hpp) and
 [QHttpResponse](./src/qhttpserverresponse.hpp).
- **optional** HTTP client classes: [QHttpClient](./src/qhttpclient.hpp),
 [QHttpRequest](./src/qhttpclientrequest.hpp) and
 [QHttpResponse](./src/qhttpclientresponse.hpp). the client classes can be
 disabled at build time by commenting `QHTTP_HAS_CLIENT` in
 [common.dir](./commondir.pri)
- **automatic memory management** of objects. Instances of connections,
 requests and replies will be deleted automatically when socket drops or
 *disconnected*.
- **PIMPL** (Private implementaion) to achieve better ABI compatibility and cleaner
 API and faster compile time.
- **Asynchronous** and **non-blocking**. You can handle thousands of concurrent
 HTTP connections efficiently by a single thread, although a multi-threaded HTTP
 server is easy to implement.
- **high throughput**, I have tried the `QHttp` and
[gason++](https://github.com/azadkuh/gason--) to implement a REST/Json web
 service on an Ubuntu VPS (dual core + 512MB ram) with more than **5800**
 connections per second (stress test). On a MacBook Pro (i5 4258U, 8GB ram),
 `QHttp` easily reaches to more than **11700** connections / second. Generally
 `QHttp` is **1.5x ~ 3x** faster than `Node.js` depending on your machine / OS.
- Easily portable where ever `Qt5 / c++14` works. Tested under:
  - **Linux** Ubuntu 12.04 ~ 16.04 LTS, g++ 5.3+
  - **OS X** 10.9+, clang 3.7+
  - **Windows** 7/8.1, msvs2015 / mingw (g++ 6.1)


## Setup
[TOC](#table-of-contents)

instructions:
```bash
# first clone this repository:
$> git clone https://github.com/azadkuh/qhttp.git
$> cd qhttp

# prepare dependencies:
$qhttp/> ./update-dependencies.sh

# now build the library and the examples
$qhttp/> qmake -r qhttp.pro
$qhttp/> make -j 8
```

## Multi-threading
[TOC](#table-of-contents)

As `QHttp` is **asynchronous** and **non-blocking**, your app can handle
thousands of concurrent HTTP connections by a single thread.

in some rare scenarios you may want to use multiple handler threads (although
 it's not always the best solution):

* there are some blocking APIs (QSql, system calls, ...) in your connection
 handler (adopting asynchronous layer over the blocking API is a better
 approach).
* the hardware has lots of free cores and the measurement shows that the load
 on the main `QHttp` thread is close to highest limit. There you can spawn some
 other handler threads.


## Source tree
[TOC](#table-of-contents)


- **`src/`**: holds the source code of `QHttp`. server classes are prefixed by
`qhttpserver*` and client classes by `qhttpclient*`.
  - **`private/`**: Private classes of the library.
- **`3rdparty/`**: will contain `http-parser` source tree as the only
dependency.  this directory is created by setup. see also: [setup](#setup).
- **`example/`**: contains some sample applications representing the `QHttp`
usage:
  - **`helloworld/`**: the HelloWorld example of `QHttp`, both server + client
  are represented.  see: [README@helloworld](./example/helloworld/README.md)
  - **`basic-server/`**: a basic HTTP server shows how to collect the request
  body, and respond to the clients. see:
  [README@basic-server](./example/basic-server/README.md)
  - **`keep-alive`**: shows how to keep an http connection open and
  transmitting many requests/responses. see:
  [README@keep-alive](./example/keep-alive/README.md)
  - **`post-collector`**: another server example shows how to collect large
  data by POST requests. see:
  [README@post-collector](./example/postcollector/README.md)
- **`tmp/`**: a temporary directory which is created while `make`ing the
library and holds all the `.o`, `moc files`, etc.
* **`xbin/`**: all the executable and libraries will be placed on this folder by
build system.



## Disclaimer
[TOC](#table-of-contents)

- Implementing a lightweight and simple HTTP server/client in Qt with Node.js
like API, is the main purpose of `QHttp`.
- There are lots of features in a full blown HTTP server which are out of scope
of this small library, although those can be added on top of `QHttp`.
- The client classes are by no mean designed as a `QNetworkAccessManager`
replacement. `QHttpClient` is simpler and lighter, for serious scenarios just
use `QNetworkAccessManager` which supports proxy, redirections, authentication,
 cookie jar, ssl, ...
- I'm a busy person.


> If you have any ideas, critiques, suggestions or whatever you want to call
> it, please open an issue. I'll be happy to hear different ideas, will think
> about them, and I try to add those that make sense.


## License
[TOC](#table-of-contents)

Distributed under the MIT license. Copyright (c) 2014, Amir Zamani.


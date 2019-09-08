# basic-server

this example shows how to write a basic REST Server based on `QHttp` server class.

`simpleRESTServer` collects the incoming request data (body of a GET/POST request) and prints it, then responds to the client with JSON object.

##usage

```bash
$> ./simpleRESTServer
```
now the server is listening on `address=QHostAddress::Any`, `port=8080` (press `ctrl+c` to stop the server).

<br/>
test the server by any REST client pointing to [localhost:8080](localhost:8080):
```bash
# GET
$> curl localhost:8080

# POST
$> curl --data 'Hello QHttp server!' localhost:8080/dummyPath/id
```

you shall see the server logs as:
```text
connection (#2): request from 127.0.0.1:62808
url: /dummyPath/id
    body: "Hello QHttp server!"
```

and the client shall prints:
```text
Hello World
    packet count = 2
    time = 2014-07-20 14:06:23
```

to close the server by a client, send a "command: quit" header as:
```bash
$> curl --data 'I want to finish you!' localhost:8080/dummyPath/id -H "command: quit"
```


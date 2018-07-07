# helloworld

this example shows the usage of `QHttp` as an HTTP server or a client.

##usage

```bash
$> ./helloworld -h

#Usage:
$> ./helloworld {mode} [options]
```

three different modes are available:

### server

to start a new http server:
```bash
$> ./helloworld server --listen 8080
```

to stop the server just press the `ctrl+c` or send a `command: quit` as an http
header to this server (by an http client).

sample clients:
```bash
# GET by curl
$> curl 127.0.0.1:8080/login/?username=admin
# send an http header to stop the server
$> curl -H "command: quit" 127.0.0.1:8080

# POST by curl, custom headers, custom body (data)
$> curl -X POST -H "my_key: my_value" -H "connection: close" \
 -d "this is http body of POST request" \
 127.0.0.1:8080/path/?cmd=have_fun

```

### client
if the `qhttp` has been configured by `QHTTP_HAS_CLIENT` (in `commondir.pri`):

to fetch a custom http url:
```bash
$> ./helloworld client --url https://www.google.com/?gws_rd=ssl#q=qt+c%2B%2B11
$> ./helloworld client --url 127.0.0.1:8080
```

the client mode dumps both headers and body data.


### weather
to fetch the weather information of your favorite city (by awesome
        [wttr.in](http://wttr.in) service):

```bash
$> ./helloworld weather --geolocation Tehran
$> ./helloworld weather --geolocation Paris,Fr
```

the result will be save into `weather.html` file, you can open it by a browser.


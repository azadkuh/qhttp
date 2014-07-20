# helloworld

this example shows the usage of `QHttp` as an HTTP server or a client.

##usage

```bash
$> ./helloworld [mode]
# where mode is 'server' or 'client'.
```

###server
to test `QHttp` server classes, start `helloworld` in server mode:
```bash
$> ./helloworld server
```
now the server is listening on `address=QHostAddress::Any`, `port=8080` (press `ctrl+c` to stop the server).

<br/>
then test the server by pointing your browser to [localhost:8080](localhost:8080) or try any other HTTP client app:
```bash
$> curl localhost:8080
```
you shall see the `Hello World!` message from the server.


###client
the simple client mode tries to fetch the weather information from [openweathermap.org](openweathermap.org):
```bash
$> ./helloworld client
```
you shall see the headers:
```
access-control-allow-credentials : true
access-control-allow-methods : get, post
transfer-encoding : chunked
server : nginx
content-type : text/xml; charset=utf-8
access-control-allow-origin : *
date : sun, 20 jul 2014 09:21:19 gmt
connection : close
x-source : redis
```


and XML body as:
```xml
<?xml version="1.0" encoding="utf-8"?>
<current>
  <city id="112931" name="Tehran">
    <coord lon="51.42" lat="35.69"/>
    <country>IR</country>
    <sun rise="2014-07-20T01:33:35" set="2014-07-20T15:47:49"/>
  </city>
  <temperature value="38" min="38" max="38" unit="celsius"/>
  <humidity value="8" unit="%"/>
  <pressure value="1006" unit="hPa"/>
  <wind>
    <speed value="6.2" name="Moderate breeze"/>
    <direction value="140" code="SE" name="SouthEast"/>
  </wind>
  <clouds value="20" name="few clouds"/>
  <visibility/>
  <precipitation mode="no"/>
  <weather number="801" value="few clouds" icon="02d"/>
  <lastupdate value="2014-07-20T09:00:00"/>
</current>
```
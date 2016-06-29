# keep-alive

this example shows the usage of `QHttp` and `keep-alive` connections.

##usage

```bash
$xbin/> ./keepalive --help
```

server mode:
```bash
$xbin/> ./keepalive server --port 8090
```

client mode:
```bash
$xbin/> ./keepalive client -p 8090 --count 1000
```

transimts 1000 packets in a single HTTP connection, which is definitly a
lot more efficient than making 1000 connection.



@TODO must create one directory more to contain the public web (and chroot) devices,log and sessions contains private information, cannot be publictouch README.md!

 Build fake easy OTA server:

```
$ openssl req -x509 -nodes -days 365 -newkey rsa:2048 -keyout server.pem -out server.pem
  [...]
  Common Name (e.g. server FQDN or YOUR name) []: localhost_
  [...]
$ openssl x509 -req -days 1024 -in server.csr -signkey server.key -out server.crt

$ cat server.crt server.key > server.pem

$ ./launchOTAServer.py 

```

```
#!/usr/bin/python

import BaseHTTPServer, SimpleHTTPServer
import ssl

httpd = BaseHTTPServer.HTTPServer(('localhost', 4443), SimpleHTTPServer.SimpleHTTPRequestHandler)
httpd.socket = ssl.wrap_socket (httpd.socket, certfile='/path/to/server.pem', server_side=True)
httpd.serve_forever()
```


./thttpd -p 6969 -d ../site -D
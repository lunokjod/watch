#!/usr/bin/python2

import BaseHTTPServer, SimpleHTTPServer
import ssl

#print("@NOTE: NEVER, NEVER NEVER use this code for production or public site")

httpd = BaseHTTPServer.HTTPServer(('192.168.69.66', 4443), SimpleHTTPServer.SimpleHTTPRequestHandler)
httpd.socket = ssl.wrap_socket (httpd.socket, certfile='server.pem', server_side=True)
httpd.serve_forever()

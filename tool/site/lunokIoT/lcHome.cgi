#!/bin/python3

# tool/thttpd-2.29$ ./thttpd -p 6969 -d ../site/public -D -l ../site/log/thttpd.log -c \*\*.cgi

# https://www.tutorialspoint.com/python3/python_cgi_programming.htm
# $ ./thttpd -p 6969 -d ../site -D -l ../site/log/thttpd.log -c \*\*.cgi
# https://docs.python.org/3/library/json.html
# https://techspirited.com/types-of-http-errors

import cgi
import os
import json
from types import NoneType
import uuid
import hashlib
import datetime
import time
import sys 
#sys.stderr = sys.stdout 
#import cgitb
#cgitb.enable()
from http import cookies 
from urllib.request import urlopen
import http.cookiejar

def get_cookie(_match): 
  # Returns the value from the matching cookie or '' if not defined.
  if 'HTTP_COOKIE' in os.environ:
    cookies = os.environ['HTTP_COOKIE']
    cookies = cookies.split('; ')
    for cookie in cookies:
      (_name, _value) = cookie.split('=')
      if (_match.lower() == _name.lower()):
        return _value
  return('')

sessionUUID = str(uuid.uuid4())
if not get_cookie('lunokWatchSession'):
    print("Set-Cookie: lunokWatchSession="+sessionUUID)
else:
    sessionUUID = get_cookie('lunokWatchSession')

sessionMD5 = hashlib.md5(sessionUUID.encode("utf-8")).hexdigest()
sessionFilePath = "../sessions/" + sessionMD5

sessionContents = {}

def CheckUserAgent():
    userAgent = str(os.environ.get("HTTP_USER_AGENT"))
    desiredUserAgent = "lunokIoT_"
    if ( desiredUserAgent != userAgent[0:len(desiredUserAgent)]):
        print ("Content-type: text/html\r\n\r\n")
        #401
        return False
    return True

# https://en.wikipedia.org/w/api.php?action=query&origin=*&prop=extracts&explaintext&titles=Lunokhod&format=json
# https://en.wikipedia.org/wiki/Special:Random

def BuildResponse():
    global sessionContents
    shit = json.dumps(sessionContents)
    print(shit)
    return
    for key in sessionContents:
        print("key:", key)
    
def SaveDeviceHistory():
    userAgent = os.environ.get("HTTP_USER_AGENT")
    if NoneType == type(userAgent):
        userAgent=""
    userHash = hashlib.md5(userAgent.encode("utf-8")).hexdigest()
    filePath = "../devices/" + userHash
    #print("The byte equivalent of hash is:", userHash)
    userIP = os.environ.get("REMOTE_ADDR")
    now = int( time.time() )
    with open(filePath, 'a') as f:
        lineToWrite = str(now) + ":" + str(userAgent) + ":" + str(userIP) +"\n"
        f.write(lineToWrite)

def LoadSession():
    global sessionContents
    try:
        file = open(sessionFilePath,mode='r')
        json.load(file,sessionContents)
        #sessionContentsRAW = file.read()
        file.close()
        #sessionContents = json.loads(sessionContentsRAW)
    except: # no previous session found, generate new one
        if (None == sessionContents.get("profile") ):
            sessionContents["profile"] = {}
            sessionContents["profile"]["sessionID"] = sessionUUID
            sessionContents["profile"]["signup"] = int( time.time() )
        if (None == sessionContents.get("notifications") ):
            sessionContents["notifications"] = []
        sessionContents["notifications"].append({
            "v":0,
            "uuid": str(uuid.uuid4()),
            "title":"Welcome to lunoKloud!",
            "body":"This is your first session on this node",
            "date":datetime.datetime.now().strftime("%H:%M:%S %d/%m %Y"),
            "from":"lunoKloud team",
            "read":False
        })
        if (None == sessionContents.get("storage") ):
            sessionContents["storage"] = {
                "local": { },
                "remotes": { },
                "all": { },
            }
        sessionContents["profile"]["now"] = int( time.time() ) # update everytime


def SaveSession():
    global sessionContents

    sessionContents["profile"]["lastSeen"] = int( time.time() )
    try:
        textfile = open(sessionFilePath, "w")
        json.dump(sessionContents, textfile)
        textfile.close()
    except:
        pass

#if ( True == CheckUserAgent() ):<
LoadSession()
SaveDeviceHistory() #history usage
print ("Content-type: text/json\r\n\r\n")
BuildResponse()
SaveSession()


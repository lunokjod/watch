#!/bin/python3
import os
import string
import random
import uuid
import socket
import configparser
from os.path import exists

Import('env')
build_type = env.GetProjectOption("build_type")

print(" * lunokWatch build script")

h_name = socket.gethostname()
IP_addres = socket.gethostbyname(h_name)
#print("Host Name is:" + h_name)
#print("Computer IP Address is:" + IP_addres)
def get_ip():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.settimeout(0)
    try:
        # doesn't even have to be reachable
        s.connect(('10.255.255.255', 1))
        IP = s.getsockname()[0]
    except Exception:
        IP = '127.0.0.1'
    finally:
        s.close()
    return IP

KEYLEN = 32
buildKey = ''.join(random.choices(string.digits + string.ascii_letters, k = KEYLEN))
SERIALLEN = 24
buildSerialNumber = ''.join(random.choices(string.digits, k = SERIALLEN))
buildUniqueID = uuid.uuid4()

# get the path of the current directory
basePath = os.getcwd()
outputFilename = basePath + "/src/lunokiot_config.hpp"
templateFilename = basePath + "/tool/lunokiot.hpp.template"

counterFile = basePath + "/tool/.buildCount.txt"
openWeatherFile = basePath + "/openWeatherKey.txt"

# check files
template_exists = exists(templateFilename)
#weatherkey_exists = exists(openWeatherFile)
if False == template_exists:
        print("")
        print("-----------> NO template file found for generate header, ABORT BUILD")
        print("")
        exit(100)
#if False == weatherkey_exists:
#        print("")
#        print("-----------> NO WEATHER KEY FILE FOUND, ABORT BUILD")
#        print("")
#        exit(100)

templateFile = open(templateFilename, "r")
templateData = templateFile.read() 
templateFile.close()
buildCount = 0
openweatherKey = "PLEASE_SET_OPENWEATHER_KEY"

if "debug" == build_type:
    print(" -> Debug build increment buildcount")
    if not os.path.exists(counterFile):
        with open(counterFile,'w') as f:
            f.write(buildCount)
    with open(counterFile,'r') as f:
        buildCount = int(f.read())
        buildCount+=1 
    with open(counterFile,'w') as f:
        f.write(str(buildCount))
    with open(openWeatherFile,'r') as f:
        openweatherKey = f.read()
elif "release" == build_type:
    print(" -> Release build don't increment the buildcont")
    with open(counterFile,'r') as f:
        buildCount = int(f.read())
    

openweatherKey = openweatherKey.replace('\n','').replace('\r','').strip()
outputData = templateData.replace("@@LUNOKIOT_GENERATED_FILE_WARNING@@", str("DONT EDIT THIS FILE DIRECTLY!!! IS A GENERATED FILE on build time USE .template instead"))
outputData = outputData.replace("@@BUILD_NUMBER@@", str(buildCount))
outputData = outputData.replace("@@LUNOKIOT_KEY@@", str(buildKey))
if "debug" == build_type:
    outputData = outputData.replace("@@OPENWEATHER_APIKEY@@", str(openweatherKey))
    outputData = outputData.replace("@@LUNOKIOT_LOCAL_CLOUD@@", str(get_ip()))
else: # clover the openweather key and localcloud node
    outputData = outputData.replace("@@OPENWEATHER_APIKEY@@", "")
    outputData = outputData.replace("@@LUNOKIOT_LOCAL_CLOUD@@", "127.0.0.1")
outputData = outputData.replace("@@LUNOKIOT_LOCAL_CLOUD_PORT@@", str("6969"))

#outputData = outputData.replace("@@LUNOKIOT_SERIALNUMBER@@", str(buildSerialNumber))
#outputData = outputData.replace("@@LUNOKIOT_UNIQUEID@@", str(buildUniqueID))

outFile = open(outputFilename, "w")
outFile.write(outputData)
outFile.close()

#!/bin/python3
import os
import string
import random
import uuid
import socket
import configparser
from os.path import exists
import subprocess

print(" * lunokWatch build script")

Import('env')
build_type = env.GetProjectOption("build_type")

# try to get the device UART type to determine the battery capacity
p = subprocess.Popen("pio --no-ansi device list", stdout=subprocess.PIPE, shell=True)
(output, err) = p.communicate()
p_status = p.wait()
dataLines = output.decode().split()
device_port="NOT FOUND"
if len(dataLines) > 0:
    device_port=dataLines[0]
else:
    print("WARNING: device not found!!! unable to determine the battery type")
batteryCapacity="380" #default battery (black, can be replaced)
normalBatteryDev="/dev/ttyUSB"

bigBatteryDev="/dev/ttyACM"
if bigBatteryDev in device_port: 
    batteryCapacity="500" # the v3 have 2 versions, 380 and 500 mAh

print(f" * Detected battery: {batteryCapacity}mAh")

#print(" * Reset otadata partition")
#os.system("otatool.py --port \"{device_port}\" erase_otadata")
#os.system("esptool.py erase_region 0xD000 0x2000")

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
    print("ADVICE: build in debug mode can leak personal information inside the flash image, like the OpenWeather key or your IP on LAN better don't share your debug builds")
elif "release" == build_type:
    print(" -> Release build don't increment the buildcont, and removes OpenWeatherKey")
    with open(counterFile,'r') as f:
        buildCount = int(f.read()) 
print("BUILD #"+str(buildCount))
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

outputData = outputData.replace("@@LILYGOTWATCH_BATTERY_CAPACITY@@", batteryCapacity)



#outputData = outputData.replace("@@LUNOKIOT_SERIALNUMBER@@", str(buildSerialNumber))
#outputData = outputData.replace("@@LUNOKIOT_UNIQUEID@@", str(buildUniqueID))

outFile = open(outputFilename, "w")
outFile.write(outputData)
outFile.close()

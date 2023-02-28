#!/bin/python3

"""
getScreenshot -- utility for takes a screenshot via Bluetooth

Usage:
  getScreenshot  <macaddress> 
  getScreenshot  <macaddress> -s
  getScreenshot -h | --help
  getScreenshot -v | --version

Options:
  -h --help                     Show help screen.
  -v --version                  Show version.
  -s --verbose                  Enable verbose output
  macaddress                    The MAC address of your paired device

"""

from time import time
import asyncio, logging
from ble_serial.scan import main as scanner
from ble_serial.bluetooth.ble_interface import BLE_interface
import numpy as np
import sys
from PIL import Image
from docopt import docopt

PIXELSIZE=240
TIMEPROGRESS=10000
RGBData = [0]*3
RGBData=[RGBData]*PIXELSIZE
RGBData=[RGBData]*PIXELSIZE
RGBData = np.array(RGBData, dtype=np.uint8)

milliseconds = int(time() * 1000)+TIMEPROGRESS

def GenerateImage():
    global RGBData
    new_image = Image.fromarray(RGBData,"RGB")
    new_image.save('screenshoot.png')

timeBegindownload = 0
totalReceived=0
downloadedData=0
MAXDATA = (PIXELSIZE*PIXELSIZE)*2
numberOfCalls=0
def receive_callback(valueByteArray: bytes):
    global milliseconds,TIMEPROGRESS,totalReceived,timeBegindownload,numberOfCalls,downloadedData
    downloadedData+=5
    funkyShit = bytearray()
    funkyShit.append(valueByteArray[2])
    upper=bytearray(funkyShit)
    funkyShit = bytearray()
    funkyShit.append(valueByteArray[0])
    funkyShit.append(valueByteArray[1])
    lower=bytearray(funkyShit)
    repeatInt = valueByteArray[2]
    rgb565Color = int.from_bytes(lower, byteorder='little')
    #invert byte order
    pixel = ((rgb565Color&0xFF)<<8)|(rgb565Color>>8)
    R = pixel&0b1111100000000000
    G = pixel&0b0000011111100000
    B = pixel&0b0000000000011111
    R = R>>8
    G = G>>3
    B = B<<3

    funkyShit = bytearray()
    funkyShit.append(valueByteArray[4])
    upper=bytearray(funkyShit)
    coordX=int.from_bytes(upper, byteorder='little')
    funkyShit = bytearray()
    funkyShit.append(valueByteArray[3])
    upper=bytearray(funkyShit)
    coordY=int.from_bytes(upper, byteorder='little')
    originY = coordY
    originX = coordX
    for i in range(repeatInt):
        RGBData[originX,originY,0] = R
        RGBData[originX,originY,1] = G
        RGBData[originX,originY,2] = B
        totalReceived+=2
        originY-=1
        if ( originY < 0 ):
            originY=PIXELSIZE-1
            originX-=1
            if ( coordX < 0 ):
                print("SOME ERROR HERE: repeat:",repeatInt,"X:",originX,"Y:",originY)
                originX=0 # pathetic!
    now =int(time() * 1000) 
    if ( now > milliseconds):
        timeUsed = -1 # (now-timeBegindownload)
        remaining = -1 # ((timeUsed/totalReceived)*MAXDATA)-timeUsed
        print("Received data size:", totalReceived,"/", MAXDATA,"time:",timeUsed,"ETA:",remaining,"secs")
        milliseconds = int(time() * 1000)+TIMEPROGRESS
    numberOfCalls+=1


async def hello_sender(ble: BLE_interface):
    ble.queue_send(b"GETSCREENSHOOT\n")

async def main(target, verbose=False):
    print("Searching for lunokIoT device..."+target)
    ### general scan
    ADAPTER = "hci0"
    SCAN_TIME = 5 #seconds
    SERVICE_UUID = None # optional filtering
    n = len(sys.argv)
    if ( n == 1 ):
        devices = await scanner.scan(ADAPTER, SCAN_TIME, SERVICE_UUID)
        found=False
        DEVICEBASENAME = "lunokIoT_"
        DEVICE = target
        # DEVICE = target
        for dev in devices:
            if dev.name[0:len(DEVICEBASENAME)] == DEVICEBASENAME:
                DEVICE=dev.address
                print("Found at: ", DEVICE)
                found=True
                break
    else:
        DEVICE=sys.argv[0]
        found=True
    if found:
        ### deep scan get's services/characteristics
        #services = await scanner.deep_scan(DEVICE, devices)
        #scanner.print_details(services)
        #print() # newline

        READ_UUID = None
        WRITE_UUID = None

        ble = BLE_interface(ADAPTER, SERVICE_UUID)
        ble.set_receiver(receive_callback)
        timeBegindownload = int(time() * 1000)
        try:
            await ble.connect(DEVICE, "public", 10.0)
            await ble.setup_chars(WRITE_UUID, READ_UUID, "rw")
            print("Downloading screenshoot (this may take a long time)...")
            await asyncio.gather(ble.send_loop(), hello_sender(ble))
        finally:
            await ble.disconnect()
        GenerateImage()
        print("Downloaded data:",downloadedData," byte")
        print("Uncompressed data:",totalReceived," byte")
        # manual indexing by uuid
        #print(services.get_service('0000ffe0-0000-1000-8000-00805f9b34fb'))
        #print(services.get_characteristic('0000ffe1-0000-1000-8000-00805f9b34fb'))
        # or by handle
        #print(services.services[16])
        #print(services.characteristics[17])
    else:
        print("No lunoIoT device found!")

if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO)
    arguments = docopt(__doc__, version='0.0.2')
    verbose = arguments["--verbose"]
    target = arguments["<macaddress>"]

    if verbose:
        print(arguments)

    asyncio.run(main(target, verbose))

#!/bin/python3
from time import time
import asyncio, logging
from ble_serial.scan import main as scanner
from ble_serial.bluetooth.ble_interface import BLE_interface
import numpy as np
from PIL import Image

rawRGB565 = []
PIXELSIZE=240
TIMEPROGRESS=15000
RGBData = [0]*3
RGBData=[RGBData]*PIXELSIZE
RGBData=[RGBData]*PIXELSIZE
RGBData = np.array(RGBData, dtype=np.uint8)

milliseconds = int(time() * 1000)+TIMEPROGRESS
def GenerateImage():
    global RGBData
    new_image = Image.fromarray(RGBData,"RGB")
    new_image.save('screenshoot.png')
    #format='PNG')

timeBegindownload = 0
totalReceived=0
downloadedData=0
MAXDATA = (PIXELSIZE*PIXELSIZE)*2
numberOfCalls=0
def receive_callback(valueByteArray: bytes):
    global milliseconds,TIMEPROGRESS,totalReceived,timeBegindownload,numberOfCalls,downloadedData
    downloadedData+=5
    #print(valueByteArray)
    funkyShit = bytearray()
    funkyShit.append(valueByteArray[2])
    upper=bytearray(funkyShit)
    funkyShit = bytearray()
    funkyShit.append(valueByteArray[0])
    funkyShit.append(valueByteArray[1])
    lower=bytearray(funkyShit)
    repeatInt = int.from_bytes(upper, byteorder='little')
    rgb565Color = int.from_bytes(lower, byteorder='little')
    #invert byte order
    pixel = ((rgb565Color&0xFF)<<8)|(rgb565Color>>8)
    #separa as cores
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
    coordY-=repeatInt
    for i in range(repeatInt):
        RGBData[coordX,coordY,0] = R
        RGBData[coordX,coordY,1] = G
        RGBData[coordX,coordY,2] = B
        totalReceived+=2
        coordY+=1
        if ( coordY > PIXELSIZE-1 ):
            coordY=0
            coordX+=1
            if ( coordX > PIXELSIZE-1 ):
                print("SOME ERROR HERE: repeat:",repeatInt,"X:",coordX,"Y:",coordY)
                coordX=PIXELSIZE-1 # pathetic!
    #rawRGB565.append(value)
    now =int(time() * 1000) 
    if ( now > milliseconds):
        timeUsed = -1 # (now-timeBegindownload)
        remaining = -1 # ((timeUsed/totalReceived)*MAXDATA)-timeUsed
        print("Received data size:", totalReceived,"/", MAXDATA,"time:",timeUsed,"ETA:",remaining,"secs")
        milliseconds = int(time() * 1000)+TIMEPROGRESS
    numberOfCalls+=1


async def hello_sender(ble: BLE_interface):
    ble.queue_send(b"GETSCREENSHOOT\n")

async def main():
    print("Searching for lunokIoT device...")
    ### general scan
    ADAPTER = "hci0"
    SCAN_TIME = 5 #seconds
    SERVICE_UUID = None # optional filtering

    devices = await scanner.scan(ADAPTER, SCAN_TIME, SERVICE_UUID)

    #print() # newline
    #scanner.print_list(devices)

    # manual indexing
    #print(devices[0].name, devices[0].address)
    found=False
    DEVICEBASENAME = "lunokIoT_"
    DEVICE = "DE:AD:BE:EF:FE:ED"
    for dev in devices:
        if dev.name[0:len(DEVICEBASENAME)] == DEVICEBASENAME:
            DEVICE=dev.address
            print("Found at: ", DEVICE)
            found=True
            break

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
        #GenerateRGB()
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
    asyncio.run(main())

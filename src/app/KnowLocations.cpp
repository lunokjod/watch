//
//    LunokWatch, a open source smartwatch software
//    Copyright (C) 2022,2023  Jordi Rubió <jordi@binarycell.org>
//    This file is part of LunokWatch.
//
// LunokWatch is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software 
// Foundation, either version 3 of the License, or (at your option) any later 
// version.
//
// LunokWatch is distributed in the hope that it will be useful, but WITHOUT 
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more 
// details.
//
// You should have received a copy of the GNU General Public License along with 
// LunokWatch. If not, see <https://www.gnu.org/licenses/>. 
//
#include "../lunokiot_config.hpp"
#include "../UI/AppLuITemplate.hpp"
#include "KnowLocations.hpp"
#include "../UI/controls/base/Container.hpp"
#include "../UI/controls/Button.hpp"
#include "../UI/controls/Text.hpp"
#include "../UI/controls/Image.hpp"
#include "../UI/controls/Check.hpp"
#include "../UI/controls/IconMenu.hpp"
#include "../UI/controls/XBM.hpp"
#include "../UI/controls/View3D/View3D.hpp"
#include "../UI/controls/Buffer.hpp"
#include "LogView.hpp"

#include "../UI/UI.hpp"
#include <LilyGoWatch.h>
using namespace LuI;
#include "../resources.hpp"
#include "../system/Datasources/database.hpp"
#include "../system/Network/BLE.hpp"
extern TFT_eSPI * tft;
static int totalKnowBLEDEvices = 0;


const IconMenuEntry KnowLocationsIcons[] = {
    {"Unknown",img_no_64_bits, img_no_64_height, img_no_64_width, [](void *obj) {
        KnowLocationApplication * self = (KnowLocationApplication *)obj;
    } },
    {"Home",img_home_64_bits, img_home_64_height, img_home_64_width, [](void *obj) {
        KnowLocationApplication * self = (KnowLocationApplication *)obj;
    } },
    {"Work",img_work_64_bits, img_work_64_height, img_work_64_width, [](void *obj) {
        KnowLocationApplication * self = (KnowLocationApplication *)obj;
    } },
    {"Rest",img_sleep_64_bits, img_sleep_64_height, img_sleep_64_width, [](void *obj) {
        KnowLocationApplication * self = (KnowLocationApplication *)obj;
    } },
    {"Mark",img_milestone_64_bits, img_milestone_64_height, img_milestone_64_width, [](void *obj) {
        KnowLocationApplication * self = (KnowLocationApplication *)obj;
    } },
};
int KnowLocationsIconsNumber = sizeof(KnowLocationsIcons) / sizeof(KnowLocationsIcons[0])-1;


static int ObtainedTotalKnowBLEDevices(void *data, int argc, char **argv, char **azColName) {
    for (int i = 0; i<argc; i++){
        if ( 0 == strcmp(azColName[i],"COUNT(1)")) {
            totalKnowBLEDEvices=atoi(argv[i]);
            return 0;
        }
    }
    return 0;
}

void KnowLocationApplication::RefreshTotalKnowDevices() {
    const char getTotals[] = "SELECT COUNT(1) from bluetooth;";
    if ( nullptr != systemDatabase ) {  
        systemDatabase->SendSQL(getTotals,ObtainedTotalKnowBLEDevices);
    }
}

KnowLocationApplication::KnowLocationApplication() {
    directDraw=false;
    canvas->fillSprite(ThCol(background));
    Container * screen = new Container(LuI_Horizontal_Layout,2); 
    // bottom buttons container
    Container * bottomButtonContainer = new Container(LuI_Vertical_Layout,3);
    // main view space
    Container * viewContainer = new Container(LuI_Horizontal_Layout,4);

    RefreshTotalKnowDevices();

    devicesFoundText = new Text("");
    devicesFoundText->SetBackgroundColor(ThCol(background));
    totalDBText = new Text("");
    totalDBText->SetBackgroundColor(ThCol(background));

    statusText = new Text("Scanning...",TFT_WHITE,false,1,&FreeMono9pt7b);
    statusText->SetBackgroundColor(ThCol(background));

    viewContainer->AddChild(totalDBText,0.5);
    viewContainer->AddChild(devicesFoundText,0.5);
    viewContainer->AddChild(statusText,0.5);
    
    screen->AddChild(viewContainer,1.65);
    screen->AddChild(bottomButtonContainer,0.35);

    // add back button to dismiss
    Button *backButton = new Button(LuI_Vertical_Layout,1,NO_DECORATION);
    backButton->border=0;
    backButton->tapCallback=[](void * obj){ LaunchWatchface(); }; // callback when tap
    // load icon in XBM format
    XBM * backButtonIcon = new XBM(img_backscreen_42_width,img_backscreen_42_height,img_backscreen_42_bits);
    //new XBM(img_backscreen_24_width,img_backscreen_24_height,img_backscreen_24_bits);
    // put the icon inside the button
    backButton->AddChild(backButtonIcon);
    // shrink button to left and empty control oversized (want button on left bottom)
    bottomButtonContainer->AddChild(backButton,0.45);

    Button * setButton = new Button(LuI_Vertical_Layout,1);
    setButton->AddChild(new Text("Set zone"));

    setButton->tapCallbackParam=this;
    setButton->tapCallback=[&](void *obj) {
        KnowLocationApplication * self = (KnowLocationApplication*)obj;
        if( xSemaphoreTake( BLEKnowDevicesSemaphore, LUNOKIOT_EVENT_DONTCARE_TIME_TICKS) == pdTRUE )  {
            bool alreadyKnown = false;
            for (auto const& dev : BLEKnowDevices) {
                lAppLog("UPDATE MAC %s TO ZONE: %d\n",dev->addr.toString().c_str(),self->locationMenu->selectedEntry);
                dev->locationGroup=self->locationMenu->selectedEntry;
                dev->dbSync=false;
                SqlAddBluetoothDevice(dev->addr.toString().c_str(),dev->distance,self->locationMenu->selectedEntry);
                SqlUpdateBluetoothDevice(dev->addr.toString().c_str(),dev->distance,self->locationMenu->selectedEntry);
                //TickType_t nextCheck = xTaskGetTickCount();     // get the current ticks
                //BaseType_t isDelayed = xTaskDelayUntil( &nextCheck, (100 / portTICK_PERIOD_MS) ); // wait a ittle bit
            }
            BLELocationZone = (BLEZoneLocations)self->locationMenu->selectedEntry;
            xSemaphoreGive( BLEKnowDevicesSemaphore );
        }
    };
    /*
    Button * scanButton = new Button(LuI_Vertical_Layout,1);
    scanButton->AddChild(new Text("Scan"));
    scanButton->tapCallback=[&](void * obj){
        lAppLog("Launch BLE passive scan...\n");
        if ( LoT().GetBLE()->IsAdvertising() ) {
            NimBLEScan *pBLEScan = BLEDevice::getScan();
            if (LoT().GetBLE()->IsEnabled() ) {
                if (pBLEScan->isScanning()) {
                    pBLEScan->stop();
                    lNetLog("BLE: Location recognition: Last Scan stopped!\n");
                }
                lNetLog("BLE: Location recognition: Scan begin\n");
                //pBLEScan->clearDuplicateCache();
                //pBLEScan->clearResults();
                pBLEScan->setMaxResults(5);
                pBLEScan->setDuplicateFilter(true);
                BLEScanResults foundDevices = pBLEScan->start(1);
                lNetLog("BLE: Devices found: %d\n",foundDevices.getCount());
                //pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
                //pBLEScan->clearDuplicateCache();
            }
        }
    };
    bottomButtonContainer->AddChild(scanButton,1.0);
    */
    bottomButtonContainer->AddChild(nullptr,1.0);
    bottomButtonContainer->AddChild(setButton,1.55);

    locationMenu = new IconMenu(LuI_Horizontal_Layout,KnowLocationsIconsNumber,KnowLocationsIcons);
    locationMenu->tapCallback=nullptr; // dont launch on tap
    locationMenu->showCatalogue=true; // show other icons
    locationMenu->backgroundColor=ThCol(background);
    locationMenu->pageCallbackParam=this;
    locationMenu->tapCallbackParam=this;
    locationMenu->pageCallback = [&](void * obj){
        KnowLocationApplication * self = (KnowLocationApplication *)obj;
        lAppLog("Zone selected: %d\n",self->locationMenu->selectedEntry);
        //(KnowLocationApplication[self->locationMenu->selectedEntry].callback)(self->locationMenu->tapCallbackParam);
    };
    //launchButton->tapCallbackParam=locationMenu; // for launchButton
    viewContainer->AddChild(locationMenu,2.5);

    AddChild(screen);
    directDraw=true;
}

KnowLocationApplication::~KnowLocationApplication() {

}

bool KnowLocationApplication::Tick() {
    if ( millis() > lastCheck) {
        char devicesFndStr[60];
        size_t devicesInRange = BLEKnowDevices.size();
        sprintf(devicesFndStr,"%u in range",devicesInRange);
        devicesFoundText->SetText(devicesFndStr);
        RefreshTotalKnowDevices();
        sprintf(devicesFndStr,"%d references",totalKnowBLEDEvices);
        totalDBText->SetText(devicesFndStr);
        if ( devicesInRange < 1  ) {
            statusText->SetText("Waiting for more");
            //@TODO Disable button "make zone"
            //@TODO Disable iconMenu
        } else {
            statusText->SetText("Select area icon");
        }
        lAppLog("BLE REPORTED ZONE LOCATION: %d\n",BLELocationZone);
        if ( ( BLEZoneLocations::UNKNOWN != BLELocationZone )
                && ( BLELocationZone != locationMenu->selectedEntry )) {
            locationMenu->selectedEntry = BLELocationZone;
            locationMenu->dirty=true;
        }
        lastCheck = millis()+15000;
    }
    EventHandler();
    return false;
}

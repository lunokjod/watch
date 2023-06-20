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
#include "PartitionBrowser.hpp"
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
#include <esp_partition.h>
#include <esp_ota_ops.h>

#include "../UI/UI.hpp"
#include <LilyGoWatch.h>
using namespace LuI;
#include "../resources.hpp"
#include "FileBrowser.hpp"
#include "AppPartitionBrowser.hpp"
//#include "../system/Datasources/database.hpp"
//extern TFT_eSPI * tft;

extern const PROGMEM uint8_t partiton_csv_start[] asm("_binary_twatch16MB_csv_start");
extern const PROGMEM uint8_t partiton_csv_end[] asm("_binary_twatch16MB_csv_end");


PartitionExplorerApplication::PartitionExplorerApplication(const char *path) {
    directDraw=false;
    canvas->fillSprite(ThCol(background));
    Container * screen = new Container(LuI_Horizontal_Layout,2); 
    // bottom buttons container
    Container * bottomButtonContainer = new Container(LuI_Vertical_Layout,3);
    // main view space
    Container * viewContainer = new Container(LuI_Horizontal_Layout,2);
    //viewContainer->border=5;
    // add back button to dismiss
    Button *backButton = new Button(LuI_Vertical_Layout,1,NO_DECORATION);
    backButton->border=0;
    backButton->tapCallback=[](void * obj){ LaunchWatchface(); }; // callback when tap
    // load icon in XBM format
    XBM * backButtonIcon = new XBM(img_backscreen_42_width,img_backscreen_42_height,img_backscreen_42_bits);
    // put the icon inside the button
    backButton->AddChild(backButtonIcon);
    // shrink button to left and empty control oversized (want button on left bottom)
    bottomButtonContainer->AddChild(backButton,0.45);
    // get compile csv data
    partTableCSVSize= partiton_csv_end - partiton_csv_start;
    partTableCSV=(char*)ps_malloc(partTableCSVSize+1);
    memcpy(partTableCSV,partiton_csv_start,partTableCSVSize);
    partTableCSV[partTableCSVSize]='\0'; // EOL
    lAppLog("--[BEGIN]-- Build time partitions:\n");
    lLog("%s\n", partTableCSV);
    lAppLog("--[END]-- Build time partitions\n");
    // get flash partitions
    const esp_partition_t *whereIAm = esp_ota_get_boot_partition();

    lAppLog("Boot from: '%s'\n",whereIAm->label);

    // obtain number of data partitions
    esp_partition_iterator_t it;
    it = esp_partition_find(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, NULL);
    appPartitionNumber=0;
    for (; it != NULL; it = esp_partition_next(it)) {
        const esp_partition_t *part = esp_partition_get(it);
        lAppLog("Found app partition '%s' at offset 0x%x with size 0x%x\n", part->label, part->address, part->size);
        appPartitionNumber++;
    }
    // create buttons for every one
    esp_partition_iterator_release(it);
    //Container * headerAppPartitionsContainer= new Container(LuI_Horizontal_Layout,2);
    Container * appPartitionsContainer= new Container(LuI_Vertical_Layout,appPartitionNumber);
    //appPartitionsContainer->AddChild(new Text("Code:"),0.8);
    //headerAppPartitionsContainer->AddChild(new Text("Code:"),0.8);
    it = esp_partition_find(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, NULL);
    for (; it != NULL; it = esp_partition_next(it)) {
        const esp_partition_t *part = esp_partition_get(it);
        
        Button *partButton=new Button(LuI_Horizontal_Layout,1);
        if ( 0 == strcmp(whereIAm->label,part->label)) {
            char buff[19];
            sprintf(buff,"*%s", part->label);
            partButton->AddChild(new Text(buff));
        } else {
            partButton->AddChild(new Text(part->label));
        }
        partButton->tapCallbackParam=(void*)part;
        partButton->tapCallback=[](void * obj){
            const esp_partition_t *part = (const esp_partition_t *)obj;
            lLog("SELECTED APP partition '%s' at offset 0x%x with size 0x%x\n", part->label, part->address, part->size);
            LaunchApplication(new AppPartitionApplication(part));

        }; // callback when tap
        appPartitionsContainer->AddChild(partButton);
    }
    esp_partition_iterator_release(it);
    //headerAppPartitionsContainer->AddChild(appPartitionsContainer,1.3);
    //viewContainer->AddChild(headerAppPartitionsContainer,0.4);
    viewContainer->AddChild(appPartitionsContainer,0.4);
    // get data partitions availiable
    dataPartitionNumber=0;
    it = esp_partition_find(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, NULL);
    for (; it != NULL; it = esp_partition_next(it)) {
        const esp_partition_t *part = esp_partition_get(it);
        lAppLog("Found data partition '%s' at offset 0x%x with size 0x%x\n", part->label, part->address, part->size);
        dataPartitionNumber++;
    }
    it = esp_partition_find(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, NULL);
    // draw data partitions
    //Container * headerDataPartitionsContainer= new Container(LuI_Horizontal_Layout,2);
    //headerDataPartitionsContainer->border=2;
    Container * dataPartitionsContainer= new Container(LuI_Horizontal_Layout,dataPartitionNumber);
    //Container * rotateColorContainer= new Container(LuI_Horizontal_Layout,1);
    //headerDataPartitionsContainer->AddChild(new Text("Data partitions:"),0.2);
    for (; it != NULL; it = esp_partition_next(it)) {
        const esp_partition_t *part = esp_partition_get(it);
        //lAppLog("Found data partition '%s' at offset 0x%x with size 0x%x\n", part->label, part->address, part->size);
        Button *partButton=new Button(LuI_Horizontal_Layout,1);
        partButton->AddChild(new Text(part->label));
        partButton->tapCallbackParam=(void*)part;
        partButton->tapCallback=[](void * obj){
            const esp_partition_t *part = (const esp_partition_t *)obj;
            lLog("SELECTED DATA partition '%s' at offset 0x%x with size 0x%x\n", part->label, part->address, part->size);
            if ( 0 == strcmp("spiffs",part->label)) { LaunchApplication(new FileExplorerApplication()); return; }
        }; // callback when tap
        dataPartitionsContainer->AddChild(partButton);
    }
    esp_partition_iterator_release(it);
    //headerDataPartitionsContainer->AddChild(dataPartitionsContainer,1.8);
    //rotateColorContainer->AddChild(headerDataPartitionsContainer);
    //viewContainer->AddChild(rotateColorContainer,1.6);
    viewContainer->AddChild(dataPartitionsContainer,1.6);


    // @TODO ITERATE "path"
    // @TODO CREATE BUFFER OF PATH SIZE
    // @TODO TOUCH AVAILIABLE
    screen->AddChild(viewContainer,1.7);
    screen->AddChild(bottomButtonContainer,0.3);

    AddChild(screen);
    directDraw=true;
}

PartitionExplorerApplication::~PartitionExplorerApplication() {
    free(partTableCSV);
    partTableCSV=nullptr;
}

bool PartitionExplorerApplication::Tick() {
    EventHandler();
    return false;
}

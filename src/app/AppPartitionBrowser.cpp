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
#include "AppPartitionBrowser.hpp"
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
#include "PartitionBrowser.hpp"
#include "../UI/UI.hpp"
#include <LilyGoWatch.h>
using namespace LuI;
#include "../resources.hpp"
//#include "../system/Datasources/database.hpp"
//extern TFT_eSPI * tft;

#include <LittleFS.h>

#include <esp_partition.h>
#include <esp_ota_ops.h>
#include <esp_flash.h>
#include <esp_app_format.h>

//const __attribute__((section(".rodata_custom_desc"))) esp_custom_app_desc_t custom_app_desc = {  };

AppPartitionApplication::AppPartitionApplication(const esp_partition_t *part) {
    directDraw=false;
    canvas->fillSprite(ThCol(background));
    Container * screen = new Container(LuI_Horizontal_Layout,2); 
    // bottom buttons container
    Container * bottomButtonContainer = new Container(LuI_Vertical_Layout,2);
    // main view space
    Container * viewContainer = new Container(LuI_Horizontal_Layout,2);
    viewContainer->border=5;
    // add back button to dismiss
    Button *backButton = new Button(LuI_Vertical_Layout,1,NO_DECORATION);
    backButton->border=0;
    backButton->tapCallback=[](void * obj){ 
        LaunchApplication(new PartitionExplorerApplication());

     }; // callback when tap
    // load icon in XBM format
    XBM * backButtonIcon = new XBM(img_backscreen_42_width,img_backscreen_42_height,img_backscreen_42_bits);
    // put the icon inside the button
    backButton->AddChild(backButtonIcon);
    // shrink button to left and empty control oversized (want button on left bottom)
    bottomButtonContainer->AddChild(backButton,0.3);
    const size_t MEMBLOCK=sizeof(esp_image_header_t);
    esp_image_header_t * header=(esp_image_header_t*)ps_malloc(MEMBLOCK);
    esp_flash_read(part->flash_chip,(void*)header,part->address,MEMBLOCK);
    lAppLog("Reading flash at: %u size: %u\n",header,part->address,MEMBLOCK);
    char buffer[512];
    const int16_t FONTHEIGHT=20;
    if ( header->magic == ESP_IMAGE_HEADER_MAGIC ) {
        //AppView = new Buffer(canvas->width()-(viewContainer->border*2),90+(header->segment_count*(FONTHEIGHT*2)));
        AppView = new Buffer(canvas->width()-(viewContainer->border*2),90+(header->segment_count*(FONTHEIGHT*4)));
        AppView->GetBuffer()->fillSprite(ByteSwap(ThCol(background)));
        //AppView->GetBuffer()->fillSprite(TFT_BLACK);
        AppView->GetBuffer()->setTextFont(1);
        AppView->GetBuffer()->setTextSize(2);
        AppView->GetBuffer()->setTextColor(TFT_WHITE);

        sprintf(buffer, "Valid %s", part->label);
        viewContainer->AddChild(new Text(buffer),0.2);
        lAppLog("Partition name '%s'\n",part->label);        
        int32_t yText=5;
        const int32_t xText=25;
        const char * SPIMODES[] = {
            "QIO",
            "QOUT",
            "DIO",
            "DOUT",
            "FAST READ",
            "SLOW READ"
        };
        sprintf(buffer, "SPI mode: %s",SPIMODES[header->spi_mode]);
        AppView->GetBuffer()->drawString(buffer,xText,yText);
        const char * APPSIZE[] = {
            "1",
            "2",
            "4",
            "8",
            "16",
            "32",
            "64",
            "128"
        };
        sprintf(buffer, "size: %s Mb",APPSIZE[header->spi_size]);
        yText+=FONTHEIGHT;
        AppView->GetBuffer()->drawString(buffer,xText,yText);
        const char * APPSPEED[] = {
            "40",
            "26",
            "20",
            "80"
        };
        uint8_t adaptedEnum=header->spi_speed;
        if ( ESP_IMAGE_SPI_SPEED_80M == adaptedEnum ) { adaptedEnum = 3; }
        sprintf(buffer, "speed: %s Mhz",APPSPEED[adaptedEnum]);
        yText+=FONTHEIGHT;
        AppView->GetBuffer()->drawString(buffer,xText,yText);
        sprintf(buffer, "Segments: %u\n", header->segment_count);
        yText+=FONTHEIGHT;
        AppView->GetBuffer()->drawString(buffer,xText,yText);
        uint32_t offsetHeader = part->address+sizeof(esp_image_header_t);//+sizeof(esp_image_segment_header_t);
        const size_t SegmentSize=sizeof(esp_image_segment_header_t);
        yText+=FONTHEIGHT/2;
        for( uint8_t off=0;off<header->segment_count;off++ ) {
            esp_image_segment_header_t * segmentHeader=(esp_image_segment_header_t*)ps_malloc(SegmentSize);
            esp_flash_read(part->flash_chip,(void*)segmentHeader,offsetHeader,SegmentSize);
            lAppLog("Reading flash at: 0x%04x size: %u\n",offsetHeader,SegmentSize);
            //sprintf(buffer, "%u 0x%04x~0x%04x(0x%04x)",off+1,segmentHeader->load_addr,segmentHeader->load_addr+segmentHeader->data_len,segmentHeader->data_len);
            //sprintf(buffer, "%u 0x%04x~0x%04x",off+1,segmentHeader->load_addr,segmentHeader->load_addr+segmentHeader->data_len);
            sprintf(buffer, "%u s: 0x%04x",off+1,segmentHeader->data_len);
            lAppLog("%s\n",buffer);
            yText+=FONTHEIGHT/2;
            AppView->GetBuffer()->setTextSize(2);
            AppView->GetBuffer()->drawString(buffer,xText,yText);
            //sprintf(buffer, "   (0x%04xbyte)",segmentHeader->data_len);
            sprintf(buffer, " 0x%04x ~",segmentHeader->load_addr);
            lAppLog("%s\n",buffer);
            yText+=FONTHEIGHT;
            AppView->GetBuffer()->setTextSize(2);
            AppView->GetBuffer()->drawString(buffer,xText,yText);
            sprintf(buffer, "  ~ 0x%04x",segmentHeader->load_addr+segmentHeader->data_len);
            lAppLog("%s\n",buffer);
            yText+=FONTHEIGHT;
            AppView->GetBuffer()->setTextSize(2);
            AppView->GetBuffer()->drawString(buffer,xText,yText);
            yText+=FONTHEIGHT;
            esp_app_desc_t * desc = (esp_app_desc_t *)ps_malloc(sizeof(esp_app_desc_t));
            // point next struct
            offsetHeader+=SegmentSize; // remains on the begin of section content
            lAppLog("Reading flash at: 0x%04x size: %u\n",offsetHeader,sizeof(esp_app_desc_t));
            esp_flash_read(part->flash_chip,(void*)desc,offsetHeader,sizeof(esp_app_desc_t));
            if ( ESP_APP_DESC_MAGIC_WORD == desc->magic_word ) {
                AppView->GetBuffer()->setTextSize(1);
                sprintf(buffer, "%s",desc->project_name);
                lAppLog("%s\n",buffer);
                AppView->GetBuffer()->drawString(buffer,xText,yText);
                lAppLog("Project name: '%s'\n",desc->project_name);
                sprintf(buffer, "%s",desc->version);
                lAppLog("%s\n",buffer);
                yText+=FONTHEIGHT/2;
                AppView->GetBuffer()->drawString(buffer,xText,yText);
                lAppLog("Project version: '%s'\n",desc->version);
                sprintf(buffer, "%s",desc->idf_ver);
                lAppLog("%s\n",buffer);
                yText+=FONTHEIGHT/2;
                AppView->GetBuffer()->drawString(buffer,xText,yText);
                lAppLog("IDF version: '%s'\n",desc->idf_ver);
            }
            free(desc);
            offsetHeader+=segmentHeader->data_len; // pass over whole section
            free(segmentHeader);
        }

        viewContainer->AddChild(AppView,1.8);
    } else {
        sprintf(buffer, "Unused %s", part->label);
        viewContainer->AddChild(new Text(buffer),0.2);
    }
    free(header);

    screen->AddChild(viewContainer,1.65);
    screen->AddChild(bottomButtonContainer,0.35);

    AddChild(screen);

    directDraw=true;
}

AppPartitionApplication::~AppPartitionApplication() {
}

bool AppPartitionApplication::Tick() {
    EventHandler();
    return false;
}

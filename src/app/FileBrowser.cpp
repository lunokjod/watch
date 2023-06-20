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
#include "FileBrowser.hpp"
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
//#include "../system/Datasources/database.hpp"
//extern TFT_eSPI * tft;

#include <LittleFS.h>




FileExplorerApplication::FileExplorerApplication(const char *path) {
    directDraw=false;
    //lAppLog("CREATEDCREATEDCREATEDCREATEDCREATEDCREATED: %s\n",(CREATED?"true":"false"));
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
    backButton->tapCallback=[](void * obj){ LaunchWatchface(); }; // callback when tap
    // load icon in XBM format
    XBM * backButtonIcon = new XBM(img_backscreen_42_width,img_backscreen_42_height,img_backscreen_42_bits);
    // put the icon inside the button
    backButton->AddChild(backButtonIcon);
    // shrink button to left and empty control oversized (want button on left bottom)
    bottomButtonContainer->AddChild(backButton,0.3);
    // label current path
    currentPath=(char*)ps_malloc(strlen(path)+1);
    strcpy(currentPath,path);
    lAppLog("Current path to explore: '%s'\n", currentPath);
    char *pathLabelText = (char*)ps_malloc(255);
    sprintf(pathLabelText,"LittleFS: '%s'",currentPath);
    viewContainer->AddChild(new Text(pathLabelText),0.2);
    free(pathLabelText);
    // iterate path
    uint32_t itemsCount=0;
    uint32_t maxFileLen=0;
    fs::File root = LittleFS.open(currentPath);
    fs::File file = root.openNextFile();
    while (file) {
        size_t fileLen = strlen(file.name());
        if ( fileLen > maxFileLen ) { maxFileLen = fileLen; }
        itemsCount++;
        file = root.openNextFile();
    }
    if ( 0 == strcmp("/",currentPath) ) {
        size_t totalSPIFFS = LittleFS.totalBytes();
        size_t usedSPIFFS = LittleFS.usedBytes();
        char buf[60];
        sprintf(buf,"Free: %u KB\n",(totalSPIFFS-usedSPIFFS)/1024);
        bottomButtonContainer->AddChild(new Text(buf),1.7);
    }
    file.close();
    root.close();
    const int FontHeight=20;
    const int FontWidth=12;
    const int MAXFILENAME=maxFileLen;
    const int MAXNUMLENGHT=3;
    const char SizeBytesPrintf[]="%u b";
    const char SizeKBytesPrintf[]="%u Kb";
    const char SizeMBytesPrintf[]="%u Mb";
    const int BeginMargin=20;
    const int FinalMargin=5;
    const uint16_t IconColor=ByteSwap(TFT_YELLOW);
    const uint16_t FileColor=ByteSwap(TFT_WHITE);
    const uint16_t FileSizeColor=ByteSwap(TFT_GREENYELLOW);
    // 3 for the "("")"
    FileView = new Buffer(FontWidth*(MAXFILENAME+MAXNUMLENGHT+strlen(SizeMBytesPrintf))+BeginMargin+FinalMargin,(itemsCount*FontHeight));
    FileView->showBars=false;
    FileView->GetBuffer()->fillSprite(ByteSwap(ThCol(background)));
    FileView->GetBuffer()->setTextSize(1);

    root = LittleFS.open(currentPath);
    file = root.openNextFile();
    int32_t itemOffset=0;
    int32_t count=0;
    while (file) {
        if ( 0 == (count%2)) { FileView->GetBuffer()->fillRect(0,itemOffset,FileView->GetBuffer()->width(),FontHeight,ByteSwap(ThCol(background_alt))); }
        // file name
        FileView->GetBuffer()->setTextColor(FileColor);
        FileView->GetBuffer()->setFreeFont(&FreeMono12pt7b);
        FileView->GetBuffer()->setTextDatum(TL_DATUM);
        FileView->GetBuffer()->drawString(file.name(),BeginMargin,itemOffset);
        if ( file.isDirectory()) {
            //lAppLog("AAAAAAAAAAAAAAAAAAAAAAAAAAAAA: %s DIRRRRR\n",file.name());
            FileView->GetBuffer()->drawXBitmap(0,(int16_t)itemOffset,(const uint8_t *)img_folder_16_bits,img_folder_16_width,img_folder_16_height,IconColor);
        } else {
            // file size
            FileView->GetBuffer()->setTextColor(FileSizeColor);
            FileView->GetBuffer()->setFreeFont(&FreeMono9pt7b);
            FileView->GetBuffer()->setTextDatum(TR_DATUM);
            char buff[255];
            size_t fileSize = file.size();
            if( fileSize > (1024*1024)) {
                sprintf(buff,SizeMBytesPrintf,fileSize/1024/1024);

            } else if( fileSize > (1024)) {
                sprintf(buff,SizeKBytesPrintf,fileSize/1024);

            } else {
                sprintf(buff,SizeBytesPrintf,fileSize);
            }
            FileView->GetBuffer()->drawString(buff,FileView->GetBuffer()->width()-FinalMargin,itemOffset);

            // draw icon from type
            const char DbFilePattern[]=".db";
            const char LogFilePattern[]=".log";
            if ( 0 == strcmp(file.name()+(strlen(file.name())-strlen(DbFilePattern)),DbFilePattern)) {
               //FileView->GetBuffer()->drawXBitmap(0,(int16_t)itemOffset,(const uint8_t *)img_file_log_16_bits,img_file_log_16_width,img_file_log_16_height,TFT_WHITE);
            } else if ( 0 == strcmp(file.name()+(strlen(file.name())-strlen(LogFilePattern)),LogFilePattern)) {
                //FileView->GetBuffer()->drawXBitmap(0,(int16_t)itemOffset,(const uint8_t *)img_file_log_16_bits,img_file_log_16_width,img_file_log_16_height,TFT_WHITE);
            }
            FileView->GetBuffer()->drawXBitmap(0,(int16_t)itemOffset,(const uint8_t *)img_file_16_bits,img_file_16_width,img_file_16_height,IconColor);
            // increment draw offset
        }
        itemOffset+=FontHeight;
        count++;
        file = root.openNextFile();
    }
    viewContainer->AddChild(FileView,1.8);

    // @TODO TOUCH AVAILIABLE
    screen->AddChild(viewContainer,1.65);
    screen->AddChild(bottomButtonContainer,0.35);

    AddChild(screen);

    directDraw=true;
}

FileExplorerApplication::~FileExplorerApplication() {
    if ( nullptr != currentPath ) {
        free(currentPath);
        currentPath=nullptr;
    }
}

bool FileExplorerApplication::Tick() {
    EventHandler();
    return false;
}

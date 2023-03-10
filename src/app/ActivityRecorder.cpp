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

#include <Arduino.h>

#include "lunokiot_config.hpp"
#include "ActivityRecorder.hpp"

#include "../static/img_record_48.xbm"
#include "../static/img_stop_48.xbm"
#include "../static/img_save_48.xbm"
#include "../static/img_search_48.xbm"

ActivityRecorderApplication::ActivityRecorderApplication() : LunokIoTApplication() {
    uint32_t wcolor = canvas->color24to16(0x27273d);
    uint32_t mcolor = canvas->color24to16(0x3b3b5b);

    accXGraph = new GraphWidget(24,79,0,360,TFT_RED,mcolor);
    accYGraph = new GraphWidget(24,79,0,360,TFT_GREEN,mcolor);
    accZGraph = new GraphWidget(24,79,0,360,TFT_BLUE,mcolor);

    playbackGraph = new GraphWidget(62,180,0,36,canvas->color24to16(0x4c70a4));

    btnStop = new ButtonImageXBMWidget(5,240-73,68,68,[&, this](void *bah) {
        Serial.println("ActivityRecorder: Event: User wants stop!");
        recordOffset = recordEntries-1; // this cause end of recording
    },img_stop_48_bits,img_stop_48_width,img_stop_48_height,TFT_WHITE,wcolor);
    btnStop->enabled = false;

    btnRecord = new ButtonImageXBMWidget(5,240-73,68,68,[&, this](void *bah) {
        Serial.println("ActivityRecorder: Begin activity recording...");
        this->btnRecord->enabled = false;
        this->btnStop->enabled = true;
        this->btnSave->enabled = false;
        this->btnMatch->enabled = false;

        countdownOffset=0;
        //playbackGraph->graph->canvas->fillSprite(CanvasWidget::MASK_COLOR);

        if ( nullptr != recordX ) {
            free(recordX);
            recordX = nullptr;
        }
        if ( nullptr != recordY ) {
            free(recordY);
            recordY = nullptr;
        }
        if ( nullptr != recordZ ) {
            free(recordZ);
            recordZ = nullptr;
        }
        Serial.printf("Allocating: %d bytes\n",(sizeof(float)*recordEntries)*3);
        recordX = (float*)ps_calloc(recordEntries,sizeof(float));
        recordY = (float*)ps_calloc(recordEntries,sizeof(float));
        recordZ = (float*)ps_calloc(recordEntries,sizeof(float));
        recordData->canvas->fillSprite(TFT_BLACK);

        recordingTime = millis();
        this->recording = true;
        this->recordOffset=0;
    },img_record_48_bits,img_record_48_width,img_record_48_height,TFT_WHITE, TFT_RED);

    btnSave = new ButtonImageXBMWidget(78,240-73,68,68,[&, this](void *bah) {
        this->btnSave->enabled = false;

        if ( ( nullptr == recordX ) || ( nullptr == recordY ) || ( nullptr == recordZ ) ) {
            Serial.println("ActivityRecorder: Nothing to save! abort");
            return;
        }
        Serial.println("ActivityRecorder: Save wave");
        if ( nullptr != recordX ) {
            if ( nullptr != historyRecordsX[historyRecordsOffset] ) {
                free(historyRecordsX[historyRecordsOffset]);
                Serial.printf("ActivityRecorder: Freed old slot X at %d\n",historyRecordsOffset);
                historyRecordsX[historyRecordsOffset] = nullptr;
            }
            historyRecordsX[historyRecordsOffset] = recordX;
            recordX = nullptr;
        }
        if ( nullptr != recordY ) {
            if ( nullptr != historyRecordsY[historyRecordsOffset] ) {
                free(historyRecordsY[historyRecordsOffset]);
                Serial.printf("ActivityRecorder: Freed old slot Y at %d\n",historyRecordsOffset);
                historyRecordsY[historyRecordsOffset] = nullptr;
            }
            historyRecordsY[historyRecordsOffset] = recordY;
            recordY = nullptr;
        }
        if ( nullptr != recordZ ) {
            if ( nullptr != historyRecordsZ[historyRecordsOffset] ) {
                free(historyRecordsZ[historyRecordsOffset]);
                Serial.printf("ActivityRecorder: Freed old slot Z at %d\n",historyRecordsOffset);
                historyRecordsZ[historyRecordsOffset] = nullptr;
            }
            historyRecordsZ[historyRecordsOffset] = recordZ;
            recordZ = nullptr;
        }
        historyRecordsLenght[historyRecordsOffset] = lastRecordOffset;
        Serial.printf("ActivityRecorder: Saved at slot %d (lenght %d)\n",historyRecordsOffset, lastRecordOffset-1);
        historyRecordsOffset++;
        if (historyRecordsOffset > (historyRecordMAX-1) ) { historyRecordsOffset = 0; }
        this->btnMatch->enabled = true;

    },img_save_48_bits,img_save_48_width,img_save_48_height,TFT_WHITE, canvas->color24to16(0x2e2e48));
    btnSave->enabled = false;

    btnMatch = new ButtonImageXBMWidget(78+68+5,240-73,68,68,[&, this](void *bah) {
        Serial.println("Building match stream...");
        // * iterate last record
        size_t lasthistoryRecordsOffset=historyRecordsOffset-1;
        if ( -1 == lasthistoryRecordsOffset ) { lasthistoryRecordsOffset = historyRecordMAX-1; }
        Serial.printf("lasthistoryRecordsOffset: %d\n", lasthistoryRecordsOffset);
        // * use threshold to simplify the data
        int32_t lrX=0;
        int32_t lrY=0;
        int32_t lrZ=0;
        size_t validEntries=0;

        if ( nullptr != matchX[currentMatchOffset] ) {
            free(matchX[currentMatchOffset]);
            matchX[currentMatchOffset] = nullptr;
            Serial.println("Freed X");
        }
        matchX[currentMatchOffset] =  (int8_t*)ps_calloc(recordEntries,sizeof(uint8_t));
        if ( nullptr != matchY[currentMatchOffset] ) {
            free(matchY[currentMatchOffset]);
            matchY[currentMatchOffset] = nullptr;
            Serial.println("Freed Y");
        }
        matchY[currentMatchOffset] =  (int8_t*)ps_calloc(recordEntries,sizeof(uint8_t));
        if ( nullptr != matchZ[currentMatchOffset] ) {
            free(matchZ[currentMatchOffset]);
            matchZ[currentMatchOffset] = nullptr;
            Serial.println("Freed Z");
        }
        matchZ[currentMatchOffset] =  (int8_t*)ps_calloc(recordEntries,sizeof(uint8_t));

        for (size_t offset = 0; offset < historyRecordsLenght[lasthistoryRecordsOffset]; offset++) {
            int32_t rX = historyRecordsX[lasthistoryRecordsOffset][offset];
            int32_t rY = historyRecordsY[lasthistoryRecordsOffset][offset];
            int32_t rZ = historyRecordsZ[lasthistoryRecordsOffset][offset];
            rX /=threshold;
            rY /=threshold;
            rZ /=threshold;
            // discard repeated values
            if ( ( rX != lrX ) || ( rY != lrY ) || ( rZ != lrZ ) ) {
                if ( offset > 0 ) { // the first value raise from 0, isn't valid
                    int8_t rfX = lrX-rX;
                    int8_t rfY = lrY-rY;
                    int8_t rfZ = lrZ-rZ;
                    Serial.printf("OFFSET: %d:%d X: %d Y: %d Z: %d\n", currentMatchOffset, offset, rfX, rfY, rfZ);
                    playbackGraph->PushValue((rX+rY+rZ)/3);
                    matchX[currentMatchOffset][validEntries] = rfX;
                    matchY[currentMatchOffset][validEntries] = rfY;
                    matchZ[currentMatchOffset][validEntries] = rfZ;
                    validEntries++;
                }
            }
            lrX=rX;
            lrY=rY;
            lrZ=rZ;
            //playbackGraph->PushValue(((lrX+lrY+lrZ)/3)*10);
        }
        matchLenght[currentMatchOffset] = validEntries;
        Serial.printf("Entries from this match: %d\n", validEntries);
        currentMatchOffset++;
        if ( currentMatchOffset > historyRecordMAX-1) {
            currentMatchOffset = 0;
        }
        btnMatch->enabled=false;
        //historyRecordsLenght[lasthistoryRecordsOffset]
        // playbackGraph


    },img_search_48_bits,img_search_48_width,img_search_48_height,TFT_WHITE, canvas->color24to16(0x2e2e48));
    btnMatch->enabled = false;

    progressBar = new CanvasWidget(14,200);
    recordData = new CanvasWidget(64,64);
    recordData->canvas->fillSprite(TFT_BLACK);
}

bool ActivityRecorderApplication::Tick() {

    accXGraph->PushValue(degX);
    accYGraph->PushValue(degY);
    accZGraph->PushValue(degZ);

    btnRecord->Interact(touched, touchX, touchY);
    btnStop->Interact(touched, touchX, touchY);
    btnSave->Interact(touched, touchX, touchY);
    btnMatch->Interact(touched, touchX, touchY);


    int8_t simplifiedX = degX/threshold;
    int8_t simplifiedY = degY/threshold;
    int8_t simplifiedZ = degZ/threshold;
    static int8_t lastSimplifiedX = simplifiedX;
    static int8_t lastSimplifiedY = simplifiedY;
    static int8_t lastSimplifiedZ = simplifiedZ;

    if ( ( lastSimplifiedX != simplifiedX ) || ( lastSimplifiedY != simplifiedY ) || ( lastSimplifiedZ != simplifiedZ ) ) {
        int8_t cmatchX = lastSimplifiedX-simplifiedX;
        int8_t cmatchY = lastSimplifiedY-simplifiedY;
        int8_t cmatchZ = lastSimplifiedZ-simplifiedZ;
        //Serial.printf("Movement detected X: %d Y: %d Z: %d\n", cmatchX, cmatchY, cmatchZ);
        static unsigned long sampleTime = 0;
        if ( sampleTime < millis() ) {
            Serial.printf("Resample, last activity: %d\n", visualPatternOffset);
            for ( size_t off=0;off<historyRecordMAX-1;off++) {
                matchPoints[off] = 0; // clean hit stats
            }
            sampleTime=millis()+ResampleMS;
        }

        for(size_t offst=0; offst< historyRecordMAX-1;offst++) {
            if ( matchLenght[offst] > 0 ) {
                for (size_t pos=0;pos<matchLenght[offst];pos++) {
                    int8_t sx = matchX[offst][pos];
                    int8_t sy = matchY[offst][pos];
                    int8_t sz = matchZ[offst][pos];
                    if ( ( sx == cmatchX ) && ( sy == cmatchY ) && ( sz == cmatchZ ) ) {
                        //Serial.printf("MATCH with pattern: %d:%d\n", offst, pos);
                        matchPoints[offst]++;
                    }
                }
            }
        }

        // hit counter
        Serial.printf("Activity Hits: ");
        size_t moreMatched = -1;
        size_t mostPoints = 0;
        for ( size_t off=0;off<historyRecordMAX-1;off++) {
            if ( matchPoints[off] > 0 ) {
                Serial.printf("%d(%d), ", off, matchPoints[off]);
                if (  matchPoints[off] > mostPoints ) {
                    mostPoints = matchPoints[off];
                    moreMatched = off;
                }
            }
        }
        Serial.println("");

        visualPatternPos = 0;
        visualPatternOffset = moreMatched;

        //Serial.println("");
        lastSimplifiedX = simplifiedX;
        lastSimplifiedY = simplifiedY;
        lastSimplifiedZ = simplifiedZ;
    }


    static unsigned long nextFullRedraw = 0;
    if ( millis() > nextFullRedraw ) {

        canvas->fillSprite(canvas->color24to16(0x191927));
        
        accXGraph->DrawTo(canvas,0,0);
        accYGraph->DrawTo(canvas,80,0);
        accZGraph->DrawTo(canvas,160,0);

        playbackGraph->DrawTo(canvas);
        
        progressBar->canvas->fillSprite(CanvasWidget::MASK_COLOR);
        int32_t calcFull = (recordOffset * progressBar->canvas->width()) /recordEntries;
        progressBar->canvas->fillRoundRect(0,0,progressBar->canvas->width(),progressBar->canvas->height(),6,TFT_BLACK);
        progressBar->canvas->fillRoundRect(0,0,calcFull,progressBar->canvas->height(),6,TFT_ORANGE);
        progressBar->DrawTo(canvas,20,142);

        if ( recording ) {
            btnStop->DrawTo(canvas);
        } else {
            btnRecord->DrawTo(canvas);
        }
        btnSave->DrawTo(canvas);
        btnMatch->DrawTo(canvas);

        canvas->setFreeFont(&FreeMonoBold18pt7b);
        canvas->setTextSize(1);
        canvas->setTextDatum(TL_DATUM);
        canvas->setTextWrap(false,false);
        canvas->setTextColor(TFT_WHITE);
        char textBuff[64] = { 0 };
        //sprintf(textBuff,"Match: %d:%d", visualPatternOffset, visualPatternPos );
        sprintf(textBuff,"Match: %d", visualPatternOffset );
        canvas->drawString(textBuff, 0,100);


        nextFullRedraw = millis()+(1000/4);
        return true;
    }

    if ( recording ) {
        UINextTimeout = millis()+UITimeout; // disable screen timeout during recording

        recordX[recordOffset] = degX;
        recordY[recordOffset] = degY;
        recordZ[recordOffset] = degZ;
        /*
        float med = (recordX[recordOffset]+recordY[recordOffset]+recordZ[recordOffset])/3;
        playbackGraph->PushValue(med);
        */
        recordOffset++;
        //Serial.printf("Offset: %lu\n",recordOffset);
        if ( recordOffset >= recordEntries ) {
            Serial.printf("Recorded time: %lu secs\n", (millis()-recordingTime)/1000);
            this->recording = false;
            this->btnRecord->enabled = true;
            this->btnStop->enabled = false;
            this->btnSave->enabled = true;
            //this->accXGraph->graph->canvas->fillSprite(this->accXGraph->backgroundColor);
            //this->accYGraph->graph->canvas->fillSprite(this->accYGraph->backgroundColor);
            //this->accZGraph->graph->canvas->fillSprite(this->accZGraph->backgroundColor);
            recordOffset = 0;
            Serial.println("ActivityRecorder: End activity recording.");
            //commented by new graphwidget playbackGraph->graph->canvas->fillSprite(CanvasWidget::MASK_COLOR);
            //playbackGraphMed->graph->canvas->fillSprite(CanvasWidget::MASK_COLOR);
            //countdownOffset = lastRecordOffset;
        } else {
            lastRecordOffset = recordOffset;
        }
        return true;
    }
    /* DataImage
    if ( recording ) {
        int16_t mapY = recordOffset / recordData->canvas->width();
        int16_t mapX = recordOffset % recordData->canvas->width();
        //Serial.printf("mapX: %d mapY: %d\n",mapX,mapY);
        uint32_t dataPacked = canvas->color565(recordX[recordOffset],recordY[recordOffset],recordZ[recordOffset]);
        recordData->canvas->drawPixel(mapX,mapY,dataPacked);
    }
    */
    return false;
}

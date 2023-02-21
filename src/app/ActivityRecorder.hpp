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

#ifndef __LUNOKIOT__ACTIVITY_RECORDER_APP__
#define __LUNOKIOT__ACTIVITY_RECORDER_APP__
#include <Arduino.h>
#include <LilyGoWatch.h>
#include "lunokiot_config.hpp"
#include "../system/Application.hpp"
#include "../UI/widgets/CanvasWidget.hpp"
#include "../UI/widgets/ButtonWidget.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../UI/widgets/NotificationWidget.hpp"
#include "../UI/widgets/SwitchWidget.hpp"
#include "../UI/representation/Point2D.hpp"
#include "../UI/widgets/GraphWidget.hpp"
#include "../UI/widgets/GaugeWidget.hpp"

class ActivityRecorderApplication: public LunokIoTApplication {
    public:
        const char *AppName() override { return "Activity recorder"; };
        GraphWidget * accXGraph = nullptr;
        GraphWidget * accYGraph = nullptr;
        GraphWidget * accZGraph = nullptr;

        GraphWidget * playbackGraph = nullptr;
        //GraphWidget * playbackGraphMed = nullptr;

        ButtonImageXBMWidget * btnRecord = nullptr;
        ButtonImageXBMWidget * btnStop = nullptr;

        ButtonImageXBMWidget * btnSave = nullptr;
        ButtonImageXBMWidget * btnMatch = nullptr;

        CanvasWidget * progressBar = nullptr;
        CanvasWidget * recordData = nullptr;
        bool recording = false;
        unsigned long recordingTime = 0;
        size_t recordOffset = 0;
        size_t lastRecordOffset =0;
        size_t countdownOffset = 0;
        //const unsigned long maxRecordingTime = 60*1000;
        const static size_t recordEntries = (60/8)*110; // DEBUG *4
   
        ActivityRecorderApplication();
        bool Tick();
        float *recordX = nullptr;
        float *recordY = nullptr;
        float *recordZ = nullptr;
        size_t historyRecordsOffset = 0;
        const float threshold = 40;
        static const unsigned long ResampleMS = 2000;
        static const size_t historyRecordMAX = 32;
        size_t historyRecordsLenght[historyRecordMAX] = { 0 };
        float *historyRecordsX[historyRecordMAX] = { nullptr };
        float *historyRecordsY[historyRecordMAX] = { nullptr };
        float *historyRecordsZ[historyRecordMAX] = { nullptr };

        size_t currentMatchOffset = 0;
        size_t matchLenght[historyRecordMAX] = { 0 };
        int8_t *matchX[historyRecordMAX] = { nullptr };
        int8_t *matchY[historyRecordMAX] = { nullptr };
        int8_t *matchZ[historyRecordMAX] = { nullptr };

        size_t matchPoints[historyRecordMAX] = { 0 };

        size_t visualPatternPos = 0;
        size_t visualPatternOffset = -1;
};

#endif

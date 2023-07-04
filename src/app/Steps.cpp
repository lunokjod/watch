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
#include <LilyGoWatch.h>
#include "Settings.hpp"
#include "../static/img_back_32.xbm"
#include "../static/img_step_32.xbm"
#include "../static/img_distance_32.xbm"
#include "../static/img_milestone_32.xbm"
#include "../static/img_setup_32.xbm"
#include "Steps.hpp"
#include "StepsSetup.hpp"
#include "StepsMilestoneSetup.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../UI/widgets/SwitchWidget.hpp"
#include "../UI/widgets/GraphWidget.hpp"
//#include "../system/Tasks.hpp"
#include <ArduinoNvs.h>
#include "LogView.hpp"
#include "../system/SystemEvents.hpp"
#include <Ticker.h>
#include "../system/Datasources/database.hpp"
#include "../lunokIoT.hpp"
#include <sqlite3.h>
#include <sys/param.h>

//Ticker StepTicker;

extern TTGOClass *ttgo; // ttgo lib
uint8_t userTall = 197;
uint32_t weekSteps[7] = { 0 }; // 0~6
bool userMaleFemale = false;
float stepDistanceCm = userTall * MAN_STEP_PROPORTION; // change man/woman @TODO this must be in settings
uint8_t lastStepsDay = 8; // impossible day to force next trigger (0~6)
extern uint32_t lastBootStepCount;

StepsApplication::~StepsApplication() {

    if ( nullptr != btnMilestone ) { delete btnMilestone; }
    if ( nullptr != weekGraph ) { delete weekGraph; }
    if ( nullptr != btnSetup ) { delete btnSetup; }
    if ( nullptr != activityGraph ) { delete activityGraph; }

}


void StepsApplication::CreateStats() {
    
    // load last values!
    for(int a=0;a<7;a++) {
        char keyName[16] = {0};
        sprintf(keyName,"lWSteps_%d",a);
        weekSteps[a] = NVS.getInt(keyName);
    }
    /*
    lAppLog("@DEBUG Fill with fake data!!! <============================ DISABLE ME!!\n");
    stepCount=random(0,12500);
    for(int a=0;a<7;a++) {
        weekSteps[a] = random(0,12500);
    }*/

    // get minMax for the graph
    for(int a=0;a<7;a++) {
        maxVal = MAX(maxVal,weekSteps[a]);
        minVal = MIN(minVal,weekSteps[a]);
        //if ( maxVal < weekSteps[a] ) { maxVal = weekSteps[a]; }
        //if ( minVal > weekSteps[a] ) { minVal = weekSteps[a]; }
    }
    milestone = NVS.getInt("smilestone");
    // rebuild new graph with min/max of the week
    if ( nullptr != weekGraph ) { delete weekGraph; }
    weekGraph = new GraphWidget(50,180,minVal,maxVal,TFT_GREEN, ThCol(background));
    
    // push values from weekSteps
    int t=6;
    while (t>=0) {
        int c=25; // this draws a bar on GraphWidget (repeat value X times draws a bar)
        while (c>0) {
            weekGraph->PushValue(weekSteps[t]);
            c--;
        }
        weekGraph->PushValue(0); // separator of 1 pixel
        t--;
    }

    if ( nullptr != activityGraph ) { delete activityGraph; }
    activityGraph = new GraphWidget(5,200,0,1,TFT_GREEN, Drawable::MASK_COLOR);
    activityGraph->inverted=false;
    //systemDatabase->SendSQL("SELECT COUNT(1) FROM rawlogSession;");
    const char sqlQuery[]="SELECT message FROM rawlogSession ORDER BY id DESC LIMIT %d;";
    char sqlQueryBuffer[strlen(sqlQuery)+10];
    sprintf(sqlQueryBuffer,sqlQuery,activityGraph->canvas->width());    
    //const char sqlQuery[]="SELECT message FROM rawlogSession;";
    systemDatabase->SendSQL(sqlQueryBuffer, [](void *data, int argc, char **argv, char **azColName) {
    //systemDatabase->SendSQL("SELECT message FROM rawlogSession;", [](void *data, int argc, char **argv, char **azColName) {
        GraphWidget * myGraph=(GraphWidget*)data;
        int i;
        for (i = 0; i<argc; i++){
            //lSysLog("   SQL: %s = %s\n", azColName[i], (argv[i] ? argv[i] : "NULL"));
            if ( 0 != strcmp(azColName[i],"message")) { continue; }

            if ( 0 == strcmp(argv[i],"Activity: Running")) { myGraph->markColor = TFT_RED; }
            else if ( 0 == strcmp(argv[i],"Activity: Walking")) { myGraph->markColor = TFT_YELLOW; }
            else if ( 0 == strcmp(argv[i],"Activity: None")) { myGraph->markColor = TFT_GREEN; }
        }
        myGraph->PushValue(1);
        return 0;
    }, (void*)activityGraph);
}
StepsApplication::StepsApplication() {
    btnSetup=new ButtonImageXBMWidget(TFT_WIDTH-32,TFT_HEIGHT-32,32,32,[&,this](IGNORE_PARAM){
        LaunchApplication(new StepsSetupApplication());
    },img_setup_32_bits,img_setup_32_height,img_setup_32_width,ThCol(background_alt),ThCol(button),false);

    btnMilestone=new ButtonImageXBMWidget(TFT_WIDTH-96,TFT_HEIGHT-32,32,32,[&,this](IGNORE_PARAM){
        LaunchApplication(new StepsMilestoneSetupApplication());
    },img_milestone_32_bits,img_milestone_32_height,img_milestone_32_width,ThCol(background_alt),ThCol(button),false);
    CreateStats();
    Tick();
}

bool StepsApplication::Tick() {
    btnSetup->Interact(touched,touchX, touchY);
    btnMilestone->Interact(touched,touchX, touchY);
    TemplateApplication::btnBack->Interact(touched,touchX, touchY);
    
    if (millis() > nextRedraw ) {
        canvas->fillSprite(ThCol(background));
        TemplateApplication::Tick();
        btnSetup->DrawTo(canvas);
        btnMilestone->DrawTo(canvas);
        int32_t x = 20;
        int32_t y = 20;
        int16_t border=4;

        canvas->setTextFont(0);
        canvas->setTextSize(1);
        canvas->setTextDatum(BL_DATUM);
        canvas->drawString("Recent activities:", 40,y-(border*2));

        canvas->fillRect(x-border,y-border,activityGraph->canvas->width()+(border*2),activityGraph->canvas->height()+(border*2),ThCol(background_alt));
        activityGraph->DrawTo(canvas,x,y);

        x=30;
        y=45;

        canvas->setTextSize(1);
        canvas->setTextDatum(BL_DATUM);
        canvas->drawString("Week comparision:", 40,y-(border*2));

        weekGraph->DrawTo(canvas,x,y);

        canvas->setTextSize(1);
        canvas->setTextDatum(TL_DATUM);
        canvas->setTextWrap(false,false);
        canvas->setTextColor(ThCol(text));
        canvas->drawString("Mo.", 34,y+60);
        canvas->drawString("Tu.", 60,y+60);
        canvas->drawString("We.", 86,y+60);
        canvas->drawString("Th.", 114,y+60);
        canvas->drawString("Fr.", 140,y+60);
        canvas->drawString("Sa.", 166,y+60);
        canvas->drawString("Su.", 192,y+60);

        time_t now;
        struct tm * tmpTime;
        time(&now);
        tmpTime = localtime(&now);
        // my weeks begins on monday not sunday
        int correctedDay = tmpTime->tm_wday-1;
        //lLog("Steps: WeekDay: %d\n",correctedDay);
        if ( -1 == correctedDay ) { correctedDay=6; }
        //drawrect under the current week day
        canvas->fillRect(32+(25*correctedDay),y+70,25,5,ThCol(light));

        char bufferText[32] = {0};
        x=30;
        y=140;

        canvas->setTextSize(1);
        canvas->setTextDatum(BL_DATUM);
        canvas->drawString("Today:", 40,y-(border*2));

        canvas->drawXBitmap(x,y,img_step_32_bits,img_step_32_width,img_step_32_height,ThCol(text));
        canvas->setTextSize(2);
        sprintf(bufferText,"%d",stepCount);
        canvas->setTextDatum(BL_DATUM);
        canvas->drawString(bufferText, x+30,y+20);
        canvas->setTextSize(1);
        canvas->setTextDatum(TL_DATUM);
        canvas->drawString("steps", x+30,y+20);

        x=130;
        y=135;

        canvas->setTextSize(1);
        canvas->setTextDatum(BL_DATUM);
        canvas->drawString("Distance:", x+10,y-(border*2));

        canvas->drawXBitmap(x,y-5,img_distance_32_bits,img_distance_32_width,img_distance_32_height,ThCol(text));
        canvas->setTextSize(2);
        canvas->setTextDatum(BL_DATUM);
        float distance = (stepCount*stepDistanceCm)/100;
        if ( distance > 1000 ) {
            sprintf(bufferText,"%.2f",distance/1000);
        } else {
            sprintf(bufferText,"%.2f",distance);
        }
        canvas->drawString(bufferText, x+40,y+15);
        canvas->setTextSize(1);
        canvas->setTextDatum(TL_DATUM);
        if ( distance > 1000 ) {
            canvas->drawString("Km", x+40,y+15);
        } else {
            canvas->drawString("mts.", x+40,y+15);
        }

        uint32_t totalStepsValues = stepsBMAActivityStationary
                +stepsBMAActivityWalking+stepsBMAActivityRunning
                +stepsBMAActivityInvalid+stepsBMAActivityNone;
        uint32_t barWidth = 220;
        x=10;
        y=canvas->height()-48;
        int32_t h=5;


        canvas->setTextSize(2);
        canvas->setTextDatum(BR_DATUM);
        float calories = stepCount/25.0; // 1 kcal =  25 steps
        sprintf(bufferText,"%0.1f KCal", calories);
        canvas->drawString(bufferText,canvas->width()-5,y-(border*2));

        canvas->setTextSize(1);
        canvas->setTextDatum(BL_DATUM);
        canvas->drawString("Percentage:", 40,y-(border*2));



        canvas->fillRect(x-2,y-2,barWidth+border,h+border,ThCol(background_alt));
        canvas->fillRect(x,y,barWidth,h,ThCol(low));

        if ( totalStepsValues > 0 ) {
            //Serial.printf("Steps: TotalSteps: %d\n", totalStepsValues);
            uint32_t pcOther=(stepsBMAActivityStationary*barWidth)/totalStepsValues;
            //Serial.printf("-------> pcOther: %d\n", pcOther);
            uint32_t pcWalking=(stepsBMAActivityWalking*barWidth)/totalStepsValues;
            //Serial.printf("-------> stepsBMAActivityWalking: %d\n", pcWalking);
            uint32_t pcRunning=(stepsBMAActivityRunning*barWidth)/totalStepsValues;
            //Serial.printf("-------> stepsBMAActivityRunning: %d\n", pcRunning);
            canvas->fillRect(x,y,pcWalking,h,ThCol(medium));
            canvas->fillRect(x+pcWalking,y,pcRunning,h,ThCol(high));

            canvas->drawFastVLine(x+pcWalking,y+8,10,ThCol(text));
            canvas->drawFastVLine(x+pcWalking+pcRunning,y+8,10,ThCol(text));
        }
        // draw milestone
        float range=maxVal-minVal;
        float correctedValue=milestone-minVal;
        int32_t pcValue = 0;
        if ( 0 == correctedValue ) { pcValue = 0; } // don't perform divide by zero x'D
        else if ( 0 == range ) { pcValue = 0; }     // don't perform divide by zero x'D
        else { pcValue = (correctedValue/range)*weekGraph->canvas->height(); }
        canvas->drawFastHLine(20,(weekGraph->canvas->height()+45)-pcValue,200,ThCol(high));
        
        nextRedraw=millis()+(1000/8);
        return true;
    }
    return false;
}

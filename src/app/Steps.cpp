#include <Arduino.h>
#include <LilyGoWatch.h>
#include "Settings.hpp"
#include "../static/img_back_32.xbm"
#include "../static/img_step_32.xbm"
#include "../static/img_distance_32.xbm"
#include "../static/img_setup_32.xbm"
#include "Steps.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../UI/widgets/SwitchWidget.hpp"
#include "../UI/widgets/GraphWidget.hpp"
//#include "../system/Tasks.hpp"
#include <ArduinoNvs.h>
#include "LogView.hpp"
#include "StepsSetup.hpp"
#include "../system/SystemEvents.hpp"
#include <Ticker.h>
Ticker StepTicker;

extern TTGOClass *ttgo; // ttgo lib
uint8_t userTall = 197;
uint32_t weekSteps[7] = { 0 }; // 0~6
bool userMaleFemale = false;
float stepDistanceCm = userTall * MAN_STEP_PROPORTION; // change man/woman @TODO this must be in settings
//TimedTaskDescriptor * stepsManager = nullptr;
uint8_t lastStepsDay = 0; // impossible day to force next trigger (0~6)
extern uint32_t lastBootStepCount;

void StepManagerCallback() {
    time_t now;
    struct tm * tmpTime;
    time(&now);
    tmpTime = localtime(&now);
    // the last register is from yesterday?
    if ( lastStepsDay != tmpTime->tm_wday ) {
        // run beyond the midnight
        //if ( tmpTime->tm_hour >= 0 ) { // a bit stupid....mah
            lEvLog("StepManagerCallback: Rotating stepcounter...\n");
            
            // my weeks begins on monday not sunday
            int correctedDay = tmpTime->tm_wday-2; // use the last day recorded to save on it
            if ( correctedDay == -2 ) { correctedDay = 5; }
            else if ( correctedDay == -1 ) { correctedDay = 6; }
            weekSteps[correctedDay] = stepCount;
            stepCount = 0;
            lastBootStepCount = 0;
            NVS.setInt("stepCount",0,false);
            lastStepsDay = tmpTime->tm_wday; // set the last register is today in raw


            // reset all counters
            ttgo->bma->resetStepCounter();
            beginStepsBMAActivity=0; // sorry current activity is discarded
            beginBMAActivity=0;
            
            stepsBMAActivityStationary = 0; // clean all
            timeBMAActivityStationary = 0;
            stepsBMAActivityWalking = 0;
            timeBMAActivityWalking = 0;
            stepsBMAActivityRunning = 0;
            timeBMAActivityRunning = 0;
            stepsBMAActivityInvalid = 0;
            timeBMAActivityInvalid = 0;
            stepsBMAActivityNone = 0;
            timeBMAActivityNone = 0;

            for(int a=0;a<7;a++) { // Obtain the last days steps
                char keyName[16] = {0};
                sprintf(keyName,"lWSteps_%d",a);
                //lLog("SAVING: %s %d\n",keyName,weekSteps[a]);
                NVS.setInt(keyName,weekSteps[a],false);
            }
            NVS.setInt("lstWeekStp",lastStepsDay,false);
            lEvLog("StepManagerCallback: Ends\n");
    }

}
void InstallStepManager() {
    StepTicker.attach(60*40,StepManagerCallback);
    return;
    /*
    if ( nullptr == stepsManager ) {
        for(int a=0;a<7;a++) { // Obtain the last days steps
            char keyName[16] = {0};
            sprintf(keyName,"lWSteps_%d",a);
            weekSteps[a] = NVS.getInt(keyName);
            //lLog("Steps: Weekday %d stats '%s'=%d\n",a,keyName,weekSteps[a]);
        }
        lastStepsDay = NVS.getInt("lstWeekStp");

        stepsManager = new TimedTaskDescriptor();
        stepsManager->name = (char *)"Stepcounter manager";
        stepsManager->everyTimeMS = (60*1000)*40; // every 40 minutes
        stepsManager->_lastCheck=millis();
        stepsManager->_nextTrigger=0; // launch NOW (as soon as system wants)
        stepsManager->callback = [&](void *unused) {
            time_t now;
            struct tm * tmpTime;
            time(&now);
            tmpTime = localtime(&now);
            // the last register is from yesterday?
            if ( lastStepsDay != tmpTime->tm_wday ) {
                // run beyond the midnight
                //if ( tmpTime->tm_hour >= 0 ) { // a bit stupid....mah
                    lEvLog("%s: Rotating stepcounter\n",stepsManager->name);
                    
                    // my weeks begins on monday not sunday
                    int correctedDay = tmpTime->tm_wday-2; // use the last day recorded to save on it
                    if ( correctedDay == -2 ) { correctedDay = 5; }
                    else if ( correctedDay == -1 ) { correctedDay = 6; }
                    weekSteps[correctedDay] = stepCount;
                    stepCount = 0;
                    lastBootStepCount = 0;
                    NVS.setInt("stepCount",0,false);
                    lastStepsDay = tmpTime->tm_wday; // set the last register is today in raw


                    // reset all counters
                    ttgo->bma->resetStepCounter();
                    beginStepsBMAActivity=0; // sorry current activity is discarded
                    beginBMAActivity=0;
                    
                    stepsBMAActivityStationary = 0; // clean all
                    timeBMAActivityStationary = 0;
                    stepsBMAActivityWalking = 0;
                    timeBMAActivityWalking = 0;
                    stepsBMAActivityRunning = 0;
                    timeBMAActivityRunning = 0;
                    stepsBMAActivityInvalid = 0;
                    timeBMAActivityInvalid = 0;
                    stepsBMAActivityNone = 0;
                    timeBMAActivityNone = 0;

                    for(int a=0;a<7;a++) { // Obtain the last days steps
                        char keyName[16] = {0};
                        sprintf(keyName,"lWSteps_%d",a);
                        //lLog("SAVING: %s %d\n",keyName,weekSteps[a]);
                        NVS.setInt(keyName,weekSteps[a],false);
                    }
                    NVS.setInt("lstWeekStp",lastStepsDay,false);
                //}
            }

            //RTC_Date d = ttgo->rtc->getDateTime();
            //Serial.printf("CURRENT WEEKDAY: %d HOUR: %d\n",tmpTime->tm_wday,tmpTime->tm_hour);
            return true;
        };
        AddTimedTask(stepsManager);
    }
    */
}

StepsApplication::~StepsApplication() {

    if ( nullptr != btnBack ) { delete btnBack; }
    if ( nullptr != weekGraph ) { delete weekGraph; }
    if ( nullptr != btnSetup ) { delete btnSetup; }

}
void StepsApplication::CreateStats() {
    int32_t maxVal = 0;
    int32_t minVal = 0;
    /*
    lAppLog("@DEBUG Filling with fake data!!!\n");
    for(int a=0;a<7;a++) {
        weekSteps[a] = random(0,8000);
    }
    */
    for(int a=0;a<7;a++) {
        if ( maxVal < weekSteps[a] ) { maxVal = weekSteps[a]; }
        if ( minVal > weekSteps[a] ) { minVal = weekSteps[a]; }
    }



    // rebuild new graph with min/max of the week
    if ( nullptr != weekGraph ) { delete weekGraph; }
    weekGraph = new GraphWidget(50,180,minVal,maxVal,ThCol(mark), ThCol(background));
    
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
}
StepsApplication::StepsApplication() {
    btnBack=new ButtonImageXBMWidget(TFT_WIDTH-69,TFT_HEIGHT-69,64,64,[&,this](void *unused){
        LaunchWatchface();
    },img_back_32_bits,img_back_32_height,img_back_32_width,ThCol(text),ThCol(button),false);

    btnSetup=new ButtonImageXBMWidget(5,TFT_HEIGHT-69,64,64,[&,this](void *unused){
        LaunchApplication(new StepsSetupApplication());
    },img_setup_32_bits,img_setup_32_height,img_setup_32_width,ThCol(text),ThCol(button),false);
    
    CreateStats();
    Tick();
}

bool StepsApplication::Tick() {
    btnBack->Interact(touched,touchX, touchY);
    btnSetup->Interact(touched,touchX, touchY);
    if (millis() > nextRedraw ) {
        canvas->fillSprite(ThCol(background));
        btnBack->DrawTo(canvas);
        btnSetup->DrawTo(canvas);
        weekGraph->DrawTo(canvas,30,30);

        canvas->setTextFont(0);
        canvas->setTextSize(1);
        canvas->setTextDatum(TL_DATUM);
        canvas->setTextWrap(false,false);
        canvas->setTextColor(ThCol(text));
        canvas->drawString("Mo.", 34,90);
        canvas->drawString("Tu.", 60,90);
        canvas->drawString("We.", 86,90);
        canvas->drawString("Th.", 114,90);
        canvas->drawString("Fr.", 140,90);
        canvas->drawString("Sa.", 166,90);
        canvas->drawString("Su.", 192,90);

        canvas->drawXBitmap(30,120,img_step_32_bits,img_step_32_width,img_step_32_height,ThCol(text));
        canvas->drawXBitmap(130,120,img_distance_32_bits,img_distance_32_width,img_distance_32_height,ThCol(text));

        char bufferText[32] = {0};
        canvas->setTextSize(2);
        sprintf(bufferText,"%d",stepCount);
        canvas->setTextDatum(BL_DATUM);
        canvas->drawString(bufferText, 60,140);
        canvas->setTextSize(1);
        canvas->setTextDatum(TL_DATUM);
        canvas->drawString("steps", 60,140);

        canvas->setTextSize(2);
        canvas->setTextDatum(BL_DATUM);
        float distance = (stepCount*stepDistanceCm)/100;
        if ( distance > 1000 ) {
            sprintf(bufferText,"%.2f",distance/1000);
        } else {
            sprintf(bufferText,"%.2f",distance);
        }
        canvas->drawString(bufferText, 170,140);
        canvas->setTextSize(1);
        canvas->setTextDatum(TL_DATUM);
        if ( distance > 1000 ) {
            canvas->drawString("Km", 170,140);
        } else {
            canvas->drawString("mts.", 170,140);
        }


        time_t now;
        struct tm * tmpTime;
        time(&now);
        tmpTime = localtime(&now);
        // my weeks begins on monday not sunday
        int correctedDay = tmpTime->tm_wday-1;
        //lLog("Steps: WeekDay: %d\n",correctedDay);
        if ( -1 == correctedDay ) { correctedDay=6; }
        //drawrect under the current week day
        canvas->fillRect(35+(25*correctedDay),100,25,5,ThCol(light));


        uint32_t totalStepsValues = stepsBMAActivityStationary
                +stepsBMAActivityWalking+stepsBMAActivityRunning
                +stepsBMAActivityInvalid+stepsBMAActivityNone;
        uint32_t barWidth = 220;

        canvas->fillRect(8,165,barWidth+4,9,ThCol(background_alt));
        canvas->fillRect(10,167,barWidth,5,ThCol(high));

        if ( totalStepsValues > 0 ) {
            //Serial.printf("Steps: TotalSteps: %d\n", totalStepsValues);
            uint32_t pcOther=(stepsBMAActivityStationary*barWidth)/totalStepsValues;
            //Serial.printf("-------> pcOther: %d\n", pcOther);
            uint32_t pcWalking=(stepsBMAActivityWalking*barWidth)/totalStepsValues;
            //Serial.printf("-------> stepsBMAActivityWalking: %d\n", pcWalking);
            uint32_t pcRunning=(stepsBMAActivityRunning*barWidth)/totalStepsValues;
            //Serial.printf("-------> stepsBMAActivityRunning: %d\n", pcRunning);
            canvas->fillRect(10,167,pcWalking,5,ThCol(low));
            canvas->fillRect(10+pcWalking,167,pcRunning,5,ThCol(medium));
        }


/*
                    stepsBMAActivityStationary = 0;
                    timeBMAActivityStationary = 0;
                    stepsBMAActivityWalking = 0;
                    timeBMAActivityWalking = 0;
                    stepsBMAActivityRunning = 0;
                    timeBMAActivityRunning = 0;
                    stepsBMAActivityInvalid = 0;
                    timeBMAActivityInvalid = 0;
                    stepsBMAActivityNone = 0;
                    timeBMAActivityNone = 0;
*/

        nextRedraw=millis()+(1000/16);
        return true;
    }
    return false;
}

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
#include "../system/Datasources/database.hpp"
#include <sqlite3.h>
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
}

StepsApplication::~StepsApplication() {

    //if ( nullptr != btnBack ) { delete btnBack; }
    if ( nullptr != weekGraph ) { delete weekGraph; }
    if ( nullptr != btnSetup ) { delete btnSetup; }
    if ( nullptr != activityGraph ) { delete activityGraph; }

}

// callback from sqlite3 used to build the activity bar
static int StepsAppInspectLogGraphGenerator(void *data, int argc, char **argv, char **azColName) {
    GraphWidget * myGraph=(GraphWidget*)data;
    int i;
    //myGraph->markColor = TFT_GREEN;
    for (i = 0; i<argc; i++){
        //lSysLog("   SQL: %s = %s\n", azColName[i], (argv[i] ? argv[i] : "NULL"));
        if ( 0 != strcmp(azColName[i],"message")) { continue; }        

        if ( 0 == strcmp(argv[i],"Activity: Running")) { myGraph->markColor = TFT_RED; }
        else if ( 0 == strcmp(argv[i],"Activity: Walking")) { myGraph->markColor = TFT_YELLOW; }
        else if ( 0 == strcmp(argv[i],"Activity: None")) { myGraph->markColor = TFT_GREEN; }
    }
    myGraph->PushValue(1);
    return 0;
}
extern SemaphoreHandle_t SqlLogSemaphore;

void StepsApplication::CreateStats() {
    int32_t maxVal = 0;
    int32_t minVal = 0;
    /*
    lAppLog("@DEBUG Fill with fake data!!! <============================ DISABLE ME!!\n");
    stepCount=random(0,8000);
    for(int a=0;a<7;a++) {
        weekSteps[a] = random(0,8000);
    }
    */
    
    // get minMax for the graph
    for(int a=0;a<7;a++) {
        if ( maxVal < weekSteps[a] ) { maxVal = weekSteps[a]; }
        if ( minVal > weekSteps[a] ) { minVal = weekSteps[a]; }
    }

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
    
    const char sqlQuery[]="SELECT * FROM rawlog ORDER BY id DESC LIMIT %d";
    char sqlQueryBuffer[400] = { 0 };
    sprintf(sqlQueryBuffer,sqlQuery,activityGraph->canvas->width());

    char *zErrMsg;
    if( xSemaphoreTake( SqlLogSemaphore, portMAX_DELAY) == pdTRUE )  {
        sqlite3_exec(lIoTsystemDatabase, sqlQueryBuffer, StepsAppInspectLogGraphGenerator, (void*)activityGraph, &zErrMsg);
        xSemaphoreGive( SqlLogSemaphore );
    }
}
StepsApplication::StepsApplication() {
    btnSetup=new ButtonImageXBMWidget(TFT_WIDTH-32,TFT_HEIGHT-32,32,32,[&,this](void *unused){
        LaunchApplication(new StepsSetupApplication());
    },img_setup_32_bits,img_setup_32_height,img_setup_32_width,ThCol(background_alt),ThCol(button),false);
    
    CreateStats();
    Tick();
}

bool StepsApplication::Tick() {
    btnSetup->Interact(touched,touchX, touchY);
    if (millis() > nextRedraw ) {
        canvas->fillSprite(ThCol(background));
        TemplateApplication::Tick();
        btnSetup->DrawTo(canvas);

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
        y=140;

        canvas->setTextSize(1);
        canvas->setTextDatum(BL_DATUM);
        canvas->drawString("Distance:", x+10,y-(border*2));

        canvas->drawXBitmap(x,y,img_distance_32_bits,img_distance_32_width,img_distance_32_height,ThCol(text));
        canvas->setTextSize(2);
        canvas->setTextDatum(BL_DATUM);
        float distance = (stepCount*stepDistanceCm)/100;
        if ( distance > 1000 ) {
            sprintf(bufferText,"%.2f",distance/1000);
        } else {
            sprintf(bufferText,"%.2f",distance);
        }
        canvas->drawString(bufferText, x+40,y+20);
        canvas->setTextSize(1);
        canvas->setTextDatum(TL_DATUM);
        if ( distance > 1000 ) {
            canvas->drawString("Km", x+40,y+20);
        } else {
            canvas->drawString("mts.", x+40,y+20);
        }




        uint32_t totalStepsValues = stepsBMAActivityStationary
                +stepsBMAActivityWalking+stepsBMAActivityRunning
                +stepsBMAActivityInvalid+stepsBMAActivityNone;
        uint32_t barWidth = 220;
        x=10;
        y=canvas->height()-48;
        int32_t h=5;

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



        nextRedraw=millis()+(1000/16);
        return true;
    }
    return false;
}

#include "BootSplash.hpp"
#include "lunokiot_config.hpp"
#include "Animation.hpp"

/*
const int32_t mod0_fx = 10;
const int32_t mod0_fy = TFT_HEIGHT-50-35;
int32_t mod0_x=mod0_fx;
int32_t mod0_y=TFT_HEIGHT;
uint8_t mode0_steps_left = 10;
*/

void DrawLoadBoxTest0Anim() {

}
void DrawLoadBoxTest0(uint32_t color=0x5DFC, int32_t x=15, int32_t y=TFT_HEIGHT-60) {
    Serial.println("DrawLoadBoxTest0");
    ttgo->tft->fillRoundRect(x,y,35,35,5,color);  
}

//void animLoadModule0();

//TickTwo animLoadModule(animLoadModule0, 1000, 100, MILLIS); // internal resolution is milli seconds

void animLoadModule0() {
    // 5bc0eb,fde74c,9bc53d,e55934,fa7921 https://coolors.co/palettes/trending
    // must be swapped to get GBR
    // try on https://trolsoft.ru/en/articles/rgb565-color-picker

    Serial.println("DEBUUUUG");
    int32_t posX = 15;
    int32_t posY = TFT_HEIGHT-60;
    ttgo->tft->fillRoundRect(posX,posY,35,35,5,0x5DFC);  
    posX+=35+10;
    ttgo->tft->fillRoundRect(posX,posY,35,35,5,0xF729);
    posX+=35+10;
    ttgo->tft->fillRoundRect(posX,posY,35,35,5,0x9607);
    posX+=35+10;
    ttgo->tft->fillRoundRect(posX,posY,35,35,5,0xDAA6);
    posX+=35+10;
    ttgo->tft->fillRoundRect(posX,posY,35,35,5,0xF3A4);
    
    /*
    const char descriptionText0[] = "Hello world!";

    TFT_eSprite * testTextSprite = new TFT_eSprite(ttgo->tft);
    testTextSprite->setColorDepth(16);
    testTextSprite->createSprite(12*strlen(descriptionText0), 20);
    //testTextSprite->setSwapBytes(true);
    testTextSprite->fillSprite(TFT_BLACK);
    testTextSprite->setFreeFont(&FreeMonoBold9pt7b);
    testTextSprite->setTextDatum(TL_DATUM);
    testTextSprite->setTextWrap(false,false);
    testTextSprite->setTextPadding(0);
    testTextSprite->setTextSize(1);
    testTextSprite->setTextColor(TFT_WHITE);
    testTextSprite->drawString(descriptionText0,0,0);
    posX = 10+17; // half box
    posY = TFT_HEIGHT-50-35-7;
    ttgo->tft->setPivot(posX, posY);
    testTextSprite->setPivot(0,0);
    testTextSprite->pushRotated(-45,TFT_BLACK);
    */
}
TFT_eSprite * splashBuffer = nullptr;

void DrawSplash() {
    splashBuffer = new TFT_eSprite(ttgo->tft);
    splashBuffer->setColorDepth(16);
    splashBuffer->createSprite(TFT_WIDTH, TFT_HEIGHT);
    // lunokIoT luna
    splashBuffer->setSwapBytes(true);
    splashBuffer->pushImage(0,0,img_splash.width,img_splash.height, (uint16_t *)img_splash.pixel_data);
    splashBuffer->setSwapBytes(false);
    
    splashBuffer->setTextSize(1);
    splashBuffer->setFreeFont(&FreeMonoBold9pt7b);
    splashBuffer->setTextDatum(TR_DATUM);
    splashBuffer->setTextWrap(false,false);
    int32_t posX = TFT_WIDTH-10;
    int32_t posY = 10;
    char buildNumberAsString[16] = { 0 };
    sprintf(buildNumberAsString,"#%u", LUNOKIOT_BUILD_NUMBER );
    splashBuffer->setTextColor(TFT_BLACK);
    splashBuffer->drawString(buildNumberAsString, posX+2, posY+2);
    splashBuffer->setTextColor(TFT_WHITE);
    splashBuffer->drawString(buildNumberAsString, posX, posY);
    ttgo->tft->setPivot(TFT_WIDTH/2, TFT_HEIGHT/2);
    splashBuffer->pushSprite(0,0);
    splashBuffer->deleteSprite();
    delete splashBuffer;
    splashBuffer = nullptr;
    ttgo->openBL();
}

void DrawSplashLoopAsTask( void * parameter ) {
    delay(2000); // begin 2 seconds after
    auto test = new AnimationDescriptor(240,TFT_HEIGHT-60,15, TFT_HEIGHT-60, 5);
    while(true) {
        delay(1000/24);
        test->Step();
        DrawSplash();
        test->Draw();
        //@TODO monitor LUNOKIOT_EVENT_LOAD_* events for show icons
    }
}

void LaunchSplash() {
    DrawSplash();
    xTaskCreate(
        DrawSplashLoopAsTask,            /* Task function. */
        "drwLoop",              /* String with name of task. */
        LUNOKIOT_TASK_STACK_SIZE,                  /* Stack size in bytes. */
        NULL,                   /* Parameter passed as input of the task */
        1,                      /* Priority of the task. */
        NULL);  /* Task handle. */
}

void DrawSplashStatus(const char *textBanner) {
    int32_t x = 0;
    int32_t y = 160;

    ttgo->tft->fillRect(20,y-10,200,5, TFT_BLACK);
    ttgo->tft->fillRect(20,y+45,200,5, TFT_BLACK);

    ttgo->tft->fillRect(x,y,240,40, TFT_BLACK);

    ttgo->tft->setTextSize(1);
    ttgo->tft->setFreeFont(&FreeMonoBold12pt7b);
    ttgo->tft->setTextDatum(TC_DATUM);
    ttgo->tft->setTextWrap(true,true);
    ttgo->tft->setTextColor(TFT_WHITE,TFT_BLACK);
    ttgo->tft->drawString(textBanner, TFT_WIDTH/2, y+10);
}

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
#include "../lunokiot_config.hpp"

#include "../UI/widgets/CanvasWidget.hpp" // for color mask
#include "IATest.hpp"

uint8_t eyeSize = 8;

IATestApplication::IATestApplication() {

    // Buffers to do the work :)

    eyeLensBuffer = new CanvasWidget(TFT_HEIGHT-40, TFT_WIDTH-40);
    compositeBuffer = new CanvasWidget(TFT_HEIGHT, TFT_WIDTH); // dirty buffer to do composition and push
    watchFacebackground = new CanvasWidget(TFT_HEIGHT,TFT_WIDTH); // static background (fast copy)

    accelSphere = new CanvasWidget(150, 150); // BMP info on a spin
    compositeBuffer150 = new CanvasWidget(150,150); // dirty buffer to do things with accelSphere

    eyeLensBuffer->canvas->fillCircle(eyeLensBuffer->canvas->height()/2, eyeLensBuffer->canvas->width()/2,98,canvas->color24to16(0x2a040c));//0x686890));
    eyeLensBuffer->canvas->fillCircle(eyeLensBuffer->canvas->height()/2, eyeLensBuffer->canvas->width()/2,87,CanvasWidget::MASK_COLOR);
    eyeLensBuffer->canvas->fillRect(0,0,100,200,CanvasWidget::MASK_COLOR);


    // generating back mask (circle)
    // base sphere (using transparent on center)
    compositeBuffer->canvas->fillSprite(TFT_BLACK);
    compositeBuffer->canvas->fillCircle(TFT_HEIGHT/2, TFT_WIDTH/2,80,CanvasWidget::MASK_COLOR);

    // outher circle decoration
    compositeBuffer->canvas->fillCircle(TFT_HEIGHT/2, TFT_WIDTH/2,120,canvas->color24to16(0x383838));
    compositeBuffer->canvas->fillCircle(TFT_HEIGHT/2, TFT_WIDTH/2,116,CanvasWidget::MASK_COLOR);
    // eraser ;-P
    compositeBuffer->canvas->fillRect(0,0,118,TFT_HEIGHT,CanvasWidget::MASK_COLOR);
    compositeBuffer->canvas->fillRect(132,0,108,TFT_HEIGHT,CanvasWidget::MASK_COLOR);
    compositeBuffer->canvas->fillRect(110,120,30,120,CanvasWidget::MASK_COLOR);

    watchFacebackground->canvas->fillSprite(CanvasWidget::MASK_COLOR);
    // build the circle marks
    for (int16_t sphereAngle = 0;sphereAngle<360;sphereAngle+=24) {
        compositeBuffer->canvas->pushRotated(watchFacebackground->canvas,sphereAngle,CanvasWidget::MASK_COLOR);
    }

    // upper single border circle
    compositeBuffer->canvas->drawCircle(TFT_HEIGHT/2, TFT_WIDTH/2,150, canvas->color24to16(0x383838));

    // inner circle
    compositeBuffer->canvas->fillCircle(TFT_HEIGHT/2, TFT_WIDTH/2,80, canvas->color24to16(0x383838));
    compositeBuffer->canvas->fillCircle(TFT_HEIGHT/2, TFT_WIDTH/2,75,CanvasWidget::MASK_COLOR);
    // right side circle cut
    compositeBuffer->canvas->fillRect(180,84,120,74, TFT_BLACK);
    compositeBuffer->canvas->fillCircle(TFT_WIDTH/2,TFT_HEIGHT/2,75, CanvasWidget::MASK_COLOR);

//    compositeBuffer->canvas->fillRect(120,80,120,80, TFT_BLACK);
    // right side circle cut lines
    compositeBuffer->canvas->fillRect(185,80,55,5, compositeBuffer->canvas->color24to16(0x383838));
    compositeBuffer->canvas->fillRect(185,160,55,5, compositeBuffer->canvas->color24to16(0x383838));

    // Dump to buffer
    compositeBuffer->canvas->pushRotated(watchFacebackground->canvas,0,CanvasWidget::MASK_COLOR);

    const int32_t notificationRouletteAngle=35;
    circleNotifications = new CanvasWidget(TFT_HEIGHT, TFT_WIDTH);

    //circleNotifications->canvas->setColorDepth(4);
    //circleNotifications->canvas->createPalette(default_4bit_palette,4);
    // default_4bit_palette
    //circleNotifications->canvas->createPalette(uint16_t *palette = nullptr, uint8_t colors = 16); 
    // notification pane draw

    // notif 1 test
    circleNotifications->canvas->fillCircle(TFT_WIDTH/2,38,20,canvas->color24to16(0x0));
    circleNotifications->canvas->drawCircle(TFT_WIDTH/2,38,18,canvas->color24to16(0xffffff));
    circleNotifications->canvas->fillCircle(TFT_WIDTH/2,38,16,canvas->color24to16(0x00ff00));

    compositeBuffer->canvas->fillSprite(CanvasWidget::MASK_COLOR);

    circleNotifications->canvas->pushRotated(compositeBuffer->canvas,notificationRouletteAngle,CanvasWidget::MASK_COLOR);
    circleNotifications->canvas->fillSprite(CanvasWidget::MASK_COLOR);
    compositeBuffer->canvas->pushRotated(circleNotifications->canvas,0,CanvasWidget::MASK_COLOR);
    // notif 2 test
    circleNotifications->canvas->fillCircle(TFT_WIDTH/2,38,20,canvas->color24to16(0x0));
    circleNotifications->canvas->drawCircle(TFT_WIDTH/2,38,18,canvas->color24to16(0xffffff));
    circleNotifications->canvas->fillCircle(TFT_WIDTH/2,38,16,canvas->color24to16(0x0000ff));

    compositeBuffer->canvas->fillSprite(CanvasWidget::MASK_COLOR);
    circleNotifications->canvas->pushRotated(compositeBuffer->canvas,notificationRouletteAngle,CanvasWidget::MASK_COLOR);
    circleNotifications->canvas->fillSprite(CanvasWidget::MASK_COLOR);
    compositeBuffer->canvas->pushRotated(circleNotifications->canvas,0,CanvasWidget::MASK_COLOR);

    // notif 3 test
    circleNotifications->canvas->fillCircle(TFT_WIDTH/2,38,20,canvas->color24to16(0x0));
    circleNotifications->canvas->drawCircle(TFT_WIDTH/2,38,18,canvas->color24to16(0xffffff));
    circleNotifications->canvas->fillCircle(TFT_WIDTH/2,38,16,canvas->color24to16(0xff00ff));

    compositeBuffer->canvas->fillSprite(CanvasWidget::MASK_COLOR);
    circleNotifications->canvas->pushRotated(compositeBuffer->canvas,notificationRouletteAngle,CanvasWidget::MASK_COLOR);
    circleNotifications->canvas->fillSprite(CanvasWidget::MASK_COLOR);
    compositeBuffer->canvas->pushRotated(circleNotifications->canvas,0,CanvasWidget::MASK_COLOR);

    // notif 4 test
    circleNotifications->canvas->fillCircle(TFT_WIDTH/2,38,20,canvas->color24to16(0x0));
    circleNotifications->canvas->drawCircle(TFT_WIDTH/2,38,18,canvas->color24to16(0xffffff));
    circleNotifications->canvas->fillCircle(TFT_WIDTH/2,38,16,canvas->color24to16(0xff0ffa));
    
}

bool IATestApplication::Tick() {

    static unsigned long nextRedraw = 0;
    if ( nextRedraw < millis() ) {
        canvas->fillSprite(TFT_BLACK);
        canvas->setPivot(TFT_WIDTH/2,TFT_HEIGHT/2);
        

        // static int16_t notifAngle = 0;
       //  testButton[0]->x = 120;
       //  testButton[0]->y = 140;
         //testButton[0]->DrawTo(clockSphere->canvas);

        // notifAngle++;
        // if ( notifAngle > 359 ) { notifAngle = notifAngle-360; }

        
        // clockSphere->canvas->fillRect(testButton[0]->x-20,testButton[0]->y-20,45+40,45+40,MYTRANSPARENT);

        nextRedraw = millis()+(1000/3);
    }

    testButton[0]->Interact(touched,touchX,touchY);

    static unsigned long assistantRedraw = 0;
    if ( assistantRedraw < millis() ) {
        // clean the composite
        compositeBuffer150->canvas->fillSprite(CanvasWidget::MASK_COLOR);
        compositeBuffer150->canvas->setPivot(75,75);
        accelSphere->canvas->pushRotated(compositeBuffer150->canvas,0,CanvasWidget::MASK_COLOR);

        int32_t randX = 8 - random(0, 16);
        int32_t randY = 8 - random(0, 16);
        int32_t randZ = 8 - random(0, 16);

        uint16_t pcXinByte = pcX*2.55;
        uint16_t pcYinByte = pcY*2.55;
        uint16_t pcZinByte = pcZ*2.55;
        
        uint16_t colorX = canvas->alphaBlend(pcXinByte,TFT_WHITE, canvas->color24to16(0xbfbf40))+randX;
        //uint16_t colorX = canvas->alphaBlend(pcX,TFT_WHITE, canvas->color24to16(0x1c02a2))+randX;
        uint16_t colorY = canvas->alphaBlend(pcYinByte,TFT_WHITE, canvas->color24to16(0xff1649))+randY;
        
        //uint16_t colorY = canvas->alphaBlend(pcY,TFT_WHITE, canvas->color24to16(0x4266f6))+randY;
        //uint16_t colorZ = canvas->alphaBlend(pcZ,TFT_WHITE, canvas->color24to16(0xff1ecf))+randZ;
        uint16_t colorZ = canvas->alphaBlend(pcZinByte,TFT_WHITE, canvas->color24to16(0x1c72c5))+randZ;

        int32_t sizeX = (pcX*0.12); if ( sizeX < 2 ) { sizeX =1; }
        int32_t sizeY = (pcY*0.12); if ( sizeY < 2 ) { sizeY =1; }
        int32_t sizeZ = (pcZ*0.12); if ( sizeZ < 2 ) { sizeZ =1; }
        /*
        compositeBuffer150->canvas->fillCircle(75,pcX+eyeSize,sizeX,colorX);
        compositeBuffer150->canvas->fillCircle(pcY+eyeSize,75,sizeY,colorY);
        compositeBuffer150->canvas->fillCircle(75+(pcZ*0.33)+eyeSize,75+(pcZ*0.33)+eyeSize,sizeZ,colorZ);
        */
        compositeBuffer150->canvas->fillCircle(75,pcX+randX,sizeX,colorX);
        compositeBuffer150->canvas->fillCircle(75,pcY+randY,sizeY,colorY);
        compositeBuffer150->canvas->fillCircle(75,pcZ+randZ,sizeZ,colorZ);
        int32_t fakeHeight = random(0, 100);
        //int32_t dcolor = random(0, 16536);
        int32_t dsize = random(1, 4);
        compositeBuffer150->canvas->fillCircle(75,fakeHeight,dsize,canvas->color24to16(0x8840bf)+randZ);

        /*
        int32_t drandX = random(0, 100);
        int32_t drandY = random(0, 100);
        int32_t dsize = random(1, 4);
        int32_t dcolor = random(0, 16536);
        compositeBuffer150->canvas->fillCircle(drandX,drandY,dsize,canvas->color24to16(0x8840bf)+randZ);
        */

        // cleanup final buffer
        accelSphere->canvas->fillSprite(CanvasWidget::MASK_COLOR);
        compositeBuffer150->canvas->setPivot(75,75);
        compositeBuffer150->canvas->pushRotated(accelSphere->canvas,3,CanvasWidget::MASK_COLOR);
        compositeBuffer150->canvas->setPivot(75,75);
/*
        static int16_t accelSphereAngle = 0;
        accelSphereAngle++;
        accelSphereAngle = accelSphereAngle % 360;
*/
        assistantRedraw = millis()+(1000/10);
        return true;
    }


    
    testButton[1]->Interact(touched,touchX,touchY);
    testButton[2]->Interact(touched,touchX,touchY);
    testButton[3]->Interact(touched,touchX,touchY);
    testButton[4]->Interact(touched,touchX,touchY);

    // watch whole clock together
    canvas->fillSprite(TFT_BLACK);
    static int16_t eyeBufferAngle = 0;
    eyeBufferAngle++;
    eyeBufferAngle = eyeBufferAngle % 360;
    eyeLensBuffer->canvas->pushRotated(canvas,360-eyeBufferAngle,CanvasWidget::MASK_COLOR);

    accelSphere->canvas->pushRotated(canvas,0,CanvasWidget::MASK_COLOR); // BMA

    watchFacebackground->canvas->pushRotated(canvas,0,CanvasWidget::MASK_COLOR); // background

    // notifications
    static int16_t notificationsAngle = 0;
    notificationsAngle+=6;
    notificationsAngle = notificationsAngle % 360;  


    // Bounding box parameters
    int16_t min_x;
    int16_t min_y;
    int16_t max_x;
    int16_t max_y;
    
/*
    testButton[0]->x = 0;
    testButton[0]->y = 0;
    testButton[0]->canvas->setPivot( 0,0);

    testButton[0]->canvas->pushRotated(circleNotifications->canvas,0,MYTRANSPARENT);
     */




///caca    testButton[0]->canvas->pushSprite(circleNotifications->canvas,10,10,MYTRANSPARENT);




//    testButton[0]->x = min_x;
 //   testButton[0]->y = min_y;
    // Get the bounding box of this rotated source Sprite

/*
    if ( circleNotifications->canvas->getRotatedBounds(testButton[0]->canvas,notificationsAngle, &min_x, &min_y, &max_x, &max_y) ) {
        //Serial.printf("getRotatedBounds: minX: %d minY: %d maxX: %d maxY: %d\n",min_x,min_y,max_x,max_y);
        
        //testButton[0]->canvas->setPivot(min_x,min_y);
         //testButton[0]->x = min_x;
         //testButton[0]->y = min_y;

    }
*/
       //testButton[0]->canvas->pushRotated(circleNotifications->canvas,MYTRANSPARENT);

        testButton[0]->DrawTo(circleNotifications->canvas);

 // /////  testButton[0]->canvas->pushRotated(circleNotifications->canvas,0,MYTRANSPARENT);

  //circleNotifications->canvas->pushRotated(canvas,notificationsAngle,MYTRANSPARENT);




//circleNotifications->canvas->pushSprite(0,0);
    //int16_t anchorX = accelSphere->canvas->getPivotX();
    //int16_t anchorY = accelSphere->canvas->getPivotY();
    //Serial.println("@TODO use anchor to get the eye the impression of watch you");




    //canvas->fillCircle(120,120,eyeSize, TFT_BLACK);
    //canvas->fillCircle(120,120,eyeSize+(eyeSize/5), TFT_BLACK);
    canvas->fillCircle(120,120,eyeSize, TFT_WHITE); // use to hide the rotation glitches
    return true;

}

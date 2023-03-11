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
#include "LogView.hpp" // log capabilities
#include "../UI/AppTemplate.hpp"
#include "Engine3D.hpp"
#include "../UI/UI.hpp"
#include "../system/SystemEvents.hpp"
#include <sys/param.h>
#include <cmath>
#include <esp_task_wdt.h>
#include "../UI/base/Widget.hpp"
//#include "../static/plane3DSample.c"
#include <freertos/task.h>
#include <freertos/semphr.h>


//float *ccosCache = nullptr;
//float *csinCache = nullptr;

//volatile bool RenderDataInUse=false;
SemaphoreHandle_t RenderDataSemaphore =  xSemaphoreCreateMutex();
TaskHandle_t RenderTaskCore0 = NULL;
TaskHandle_t RenderTaskCore1 = NULL;
TFT_eSprite * RenderTask0Result = nullptr;
Clipping2D RenderTask0ResultViewClipping;
TFT_eSprite * RenderTask1Result = nullptr;
Clipping2D RenderTask1ResultViewClipping;

// https://github.com/vindar/tgx
void RenderTask(void *data);

int16_t Engine3DApplication::GetDeepForPoint(IN Point3D &point) {
    //int16_t deep=3*point.x-3*point.y+3*point.z;
    int16_t deep=2*point.x-2*point.y+2*point.z; // isometric deep :D
    //int16_t deep = (point.x+point.z)/2;
    return deep;
}


uint8_t Engine3DApplication::GetZBufferDeepForPoint(INOUT TFT_eSprite *zbuffer,IN Point3D &point) {
    Point2D pix;
    GetProjection(point,pix);
    return GetZBufferDeepForPoint(zbuffer,pix);
}
uint8_t Engine3DApplication::GetZBufferDeepForPoint(INOUT TFT_eSprite *zbuffer,IN Point2D &pix) {
    return GetZBufferDeepForPoint(zbuffer,pix.x,pix.y);
}

uint8_t Engine3DApplication::GetZBufferDeepForPoint(INOUT TFT_eSprite *zbuffer, IN int32_t &x, IN int32_t &y) {
    uint16_t colorzDeep = zbuffer->readPixel(x,y);
    double r = ((colorzDeep >> 11) & 0x1F) / 31.0; // red   0.0 .. 1.0
    double g = ((colorzDeep >> 5) & 0x3F) / 63.0;  // green 0.0 .. 1.0
    double b = (colorzDeep & 0x1F) / 31.0;         // blue  0.0 .. 1.0
    uint8_t zdeep = 255*((r+g+b)/3.0);
    return zdeep;
}

float Engine3DApplication::NormalFacing(IN Normal3D &normal) {
    /*
    float deep=nx[contn]-ny[contn]+nz[contn];  
    if(deep<0){ saltar=true; }
    */
    float deep=normal.vertexData[0]-normal.vertexData[1]+normal.vertexData[2];  
    //if(deep<0){ return true; }
    return deep;
} 
bool Engine3DApplication::IsClockwise(IN Point2D &pix0,IN Point2D &pix1,IN Point2D &pix2) {
    // backface-culling for dickheads like me! :D
    int val = (pix1.y - pix0.y) * (pix2.x - pix1.x)
              - (pix1.x - pix0.x) * (pix2.y - pix1.y);
    if (val == 0) { return false; } // collinear
    return (val > 0) ? false : true; // clock or counterclock wise
}
bool Engine3DApplication::IsClockwise(IN Face3D &face) {
    // quick and dirty x'D
    Point3D * p0 = face.vertexData[0];
    Point3D * p1 = face.vertexData[1];
    Point3D * p2 = face.vertexData[2];
    Point2D pix0;
    Point2D pix1;
    Point2D pix2;
    GetProjection(*p0,pix0);
    GetProjection(*p1,pix1);
    GetProjection(*p2,pix2);
    return IsClockwise(pix0,pix1,pix2);
}


void Engine3DApplication::DirectRender(IN Range MeshDimensions,
                                INOUT TFT_eSprite * alphaChannel,
                                INOUT TFT_eSprite * zbuffer,
                                INOUT TFT_eSprite * normalBuffer,
                                INOUT TFT_eSprite * polyBuffer,
                                INOUT TFT_eSprite * smoothBuffer,
                                INOUT TFT_eSprite * lightBuffer,
                                INOUT TFT_eSprite * buffer3d) {
    // painter algorythm
    const int32_t MaxDeep= MeshDimensions.Max; //+abs(MeshDimensions.Min);
    int32_t currentDepth=MeshDimensions.Min;
    size_t orderdFaces=0;
    while ( currentDepth < MaxDeep ) {
        for(int i=0;i<myFacesNumber;i++) {
            Face3D & face = myFaces[i]; 
            Point3D * p0 = face.vertexData[0];
            Point3D * p1 = face.vertexData[1];
            Point3D * p2 = face.vertexData[2];

            // get the 3 involved vertex deep to get planar deepth
            int16_t deep0 = GetDeepForPoint(*p0);
            int16_t deep1 = GetDeepForPoint(*p1);
            int16_t deep2 = GetDeepForPoint(*p2);
            int32_t medianDeep = (deep0+deep1+deep2)/3;
            if ( currentDepth != medianDeep ) { continue; } // is face in this deep? (painter algorythm, back to forth)

            // backface culling
            Point2D pix0;
            Point2D pix1;
            Point2D pix2;
            GetProjection(*p0,pix0);
            GetProjection(*p1,pix1);
            GetProjection(*p2,pix2);
            float distNormal = NormalFacing(myNormals[i]);
            //if ( distNormal<0 ) { continue; }
            if ( false == IsClockwise(pix0,pix1,pix2) ) { continue; } // backface-culing

            uint32_t tp0x = centerX+pix0.x;
            uint32_t tp0y = centerY+pix0.y;
            uint32_t tp1x = centerX+pix1.x;
            uint32_t tp1y = centerY+pix1.y;
            uint32_t tp2x = centerX+pix2.x;
            uint32_t tp2y = centerY+pix2.y;

            int16_t alpha=(255*(float(medianDeep+abs(MeshDimensions.Min))/float(MaxDeep+abs(MeshDimensions.Min))));
            //lLog("MD: %d ", medianDeep);
            // draw if not black
            if ( alpha > 0 ) {
                const uint16_t zcolor = zbuffer->color565(alpha,alpha,alpha);
                // fill the cache
                facesToRender[orderdFaces].normalFacing = distNormal;
                facesToRender[orderdFaces].faceOffset = i;
                facesToRender[orderdFaces].p0x=tp0x;
                facesToRender[orderdFaces].p0y=tp0y;
                facesToRender[orderdFaces].p1x=tp1x;
                facesToRender[orderdFaces].p1y=tp1y;
                facesToRender[orderdFaces].p2x=tp2x;
                facesToRender[orderdFaces].p2y=tp2y;
                facesToRender[orderdFaces].colorDeep=zcolor;
                facesToRender[orderdFaces].alpha=alpha;
                orderdFaces++;
            }
        }
        currentDepth++;
    }
    // here the faces are ordered

    // do the default light @LIGHT
    //lightBuffer->fillCircle(centerX-40,centerY-40,20,TFT_WHITE);
    uint8_t r=20;
    uint8_t c=1;
    uint8_t colorStep=255/r;
    for(;r>3;r--) {
        uint16_t color = lightBuffer->color565(c,c,c);
        lightBuffer->fillCircle(centerX-10,centerY-10,r,color);
        c+=colorStep;
        if ( c > 200 ) { c=255; }
    }
    // final rendering
    //currentDepth=MeshDimensions.Min;
    //static int i=0;
    //i++;
    //if ( i > orderdFaces ) { i = 0; }
    //size_t renderedFaces=0;
    int16_t polyCount=0;
    for(int i=0;i<orderdFaces;i++) {
        int32_t tp0x = facesToRender[i].p0x;
        int32_t tp0y = facesToRender[i].p0y;
        int32_t tp1x = facesToRender[i].p1x;
        int32_t tp1y = facesToRender[i].p1y;
        int32_t tp2x = facesToRender[i].p2x;
        int32_t tp2y = facesToRender[i].p2y;

        //@FACE
        // fill the alpha channel
        alphaChannel->fillTriangle(tp0x,tp0y,tp1x,tp1y,tp2x,tp2y,TFT_WHITE);

        // fill the zbuffer
        const uint16_t zcolor = facesToRender[i].colorDeep;
        //zbuffer->color565(facesToRender[i].alpha,facesToRender[i].alpha,facesToRender[i].alpha);
        zbuffer->fillTriangle(tp0x,tp0y,tp1x,tp1y,tp2x,tp2y,zcolor);

        // fill the normals
        float dist = facesToRender[i].normalFacing; // NormalFacing(normal); //fabs(NormalFacing(normal));
        if ( dist>0 ) {
            uint8_t baseCol=(128*dist);
            uint16_t nColor = normalBuffer->color565(baseCol,baseCol,baseCol);
            normalBuffer->fillTriangle(tp0x,tp0y,tp1x,tp1y,tp2x,tp2y,nColor);
        
            // fill the poly map
            polyCount++;
            if ( polyCount > 9 ) { polyCount=0; }
            uint16_t polyColor = AvailColors[polyCount];
            polyBuffer->fillTriangle(tp0x,tp0y,tp1x,tp1y,tp2x,tp2y,polyColor);

            // fill the smooth buffer
            uint16_t smoothColor = TFT_BLACK;
            if ( myFaces[facesToRender[i].faceOffset].smooth ) {
                smoothColor=TFT_WHITE;
            }
            smoothBuffer->fillTriangle(tp0x,tp0y,tp1x,tp1y,tp2x,tp2y,smoothColor);

            // fill the face
            buffer3d->fillTriangle(tp0x,tp0y,tp1x,tp1y,tp2x,tp2y,myFaces[facesToRender[i].faceOffset].color);

            //renderedFaces++;
        }
    }
//    lLog("Faces: %u Rendered: %u\n", myFacesNumber,renderedFaces);
}

extern bool UILongTapOverride; // don't allow long tap for task switcher
bool Engine3DApplication::Tick() {
    UILongTapOverride=true; // remember every time
    TemplateApplication::btnBack->Interact(touched,touchX,touchY); // backbutton on bottom-right

    if ( 0 != lastCoreUsed ) {
        if ( nullptr != RenderTask0Result ) {
            TFT_eSprite * current = RenderTask0Result;
            //Clipping2D RenderTask0ResultViewClipping;
            int32_t x = centerX+RenderTask0ResultViewClipping.xMin;
            int32_t y = centerY+RenderTask0ResultViewClipping.yMin;
            //lLog("PUSH0: %d %d\n",x,y);
            current->pushSprite(x,y);
            //current->pushImageDMA()
            RenderTask0Result = nullptr;
            current->deleteSprite();
            delete current;
            lastCoreUsed=0;
            return false;
        }
    }
    if ( 1 != lastCoreUsed ) {
        if ( nullptr != RenderTask1Result ) {
            TFT_eSprite * current = RenderTask1Result;
            int32_t x = centerX+RenderTask1ResultViewClipping.xMin;
            int32_t y = centerY+RenderTask1ResultViewClipping.yMin;
            //lLog("PUSH1: %d %d\n",x,y);
            current->pushSprite(x,y);
            lastPx = (renderSizeW-current->width())/2;
            lastPy = (renderSizeH-current->height())/2;
            RenderTask1Result = nullptr;
            current->deleteSprite();
            delete current;
            lastCoreUsed=1;
            return false;
        }
    }
    return false;
}

volatile uint8_t showStats=0;
void RenderTask(void *data) {
    RenderCore * where=(RenderCore *)data;
    Engine3DApplication * app = where->instance;

    TFT_eSprite *alphaChannel = new TFT_eSprite(ttgo->tft);  // have mesh on the area?
    alphaChannel->setColorDepth(1);
    alphaChannel->createSprite(app->renderSizeW,app->renderSizeH);

    TFT_eSprite *zbuffer = new TFT_eSprite(ttgo->tft);  // the deep of pixels
    zbuffer->setColorDepth(16);
    zbuffer->createSprite(app->renderSizeW,app->renderSizeH);

    TFT_eSprite *normalsBuffer = new TFT_eSprite(ttgo->tft); // where 3D are rendered
    normalsBuffer->setColorDepth(16);
    normalsBuffer->createSprite(app->renderSizeW,app->renderSizeH);

    TFT_eSprite *polyBuffer = new TFT_eSprite(ttgo->tft); // where 3D are rendered
    polyBuffer->setColorDepth(16);
    polyBuffer->createSprite(app->renderSizeW,app->renderSizeH);

    TFT_eSprite *buffer3d=new TFT_eSprite(ttgo->tft); // where 3D are rendered
    buffer3d->setColorDepth(16);
    buffer3d->createSprite(app->renderSizeW,app->renderSizeH);
    Range MeshDimensions;
    Clipping2D ViewClipping;    // used to know the updated area on the current rendered frame

    TFT_eSprite *smoothBuffer = new TFT_eSprite(ttgo->tft);  // have mesh on the area?
    smoothBuffer->setColorDepth(1);
    smoothBuffer->createSprite(app->renderSizeW,app->renderSizeH);

    TFT_eSprite *lightBuffer = new TFT_eSprite(ttgo->tft);  // lights
    lightBuffer->setColorDepth(16);
    lightBuffer->createSprite(app->renderSizeW,app->renderSizeH);

    // default first clip dimension
    ViewClipping.xMin = app->centerX-1;
    ViewClipping.yMin = app->centerY-1;
    ViewClipping.xMax = app->centerX+1;
    ViewClipping.yMax = app->centerY+1;
    //const uint8_t border4=app->border*4;
    const uint8_t border2=app->border*2;
    unsigned long bClean;
    unsigned long eClean;
    unsigned long bAnimation;
    unsigned long eAnimation;
    unsigned long bRender;
    unsigned long eRender;
    unsigned long bCompsite;
    unsigned long eCompsite;
    unsigned long bWait;
    unsigned long eWait;
//    int64_t bPush;
//    int64_t ePush;
    while(where->run) {
        TickType_t nextCheck = xTaskGetTickCount();     // get the current ticks
        xTaskDelayUntil( &nextCheck, (1 / portTICK_PERIOD_MS) ); // wait a ittle bit (freeRTOS must breath)

        int64_t bTask = millis();
        //esp_task_wdt_reset();

        if ( false == where->run ) { break; }
        bClean = millis();

        // update fp
        int32_t fpx = app->centerX+ViewClipping.xMin;
        int32_t fpy = app->centerY+ViewClipping.yMin;
        int32_t fpw = app->centerX+ViewClipping.xMax;
        int32_t fph = app->centerY+ViewClipping.yMax;

        // new clip
        int16_t clipWidth  = fpw-fpx;
        int16_t clipHeight  = fph-fpy;


        //lLog("X: %d Y: %d Clip: %d %d %d %d\n",px,py,ViewClipping.xMin,ViewClipping.yMin,
        //                ViewClipping.xMax,ViewClipping.yMax);
        //lLog("CRECT: %d %d %d %d CLIP: %d %d\n",fpx,fpy,fpw,fph,clipWidth,clipHeight);

        // clean alpha channel
        //alphaChannel->fillSprite(TFT_BLACK);
        alphaChannel->fillRect(fpx-app->border,fpy-app->border,clipWidth+border2,clipHeight+border2,TFT_BLACK);
 
        // clean lights
        lightBuffer->fillRect(fpx-app->border,fpy-app->border,clipWidth+border2,clipHeight+border2,TFT_BLACK);

        //zbuffer->fillSprite(TFT_BLACK);
        //normalsBuffer->fillSprite(TFT_BLACK);
        //buffer3d->fillSprite(TFT_BLUE);
        buffer3d->fillRect(fpx-app->border,fpy-app->border,clipWidth+border2,clipHeight+border2,TFT_BLACK);

        // clear the mesh size
        MeshDimensions.Max=0;
        MeshDimensions.Min=1;
        // clear the clip dimension
        ViewClipping.xMin = app->centerX-1;
        ViewClipping.yMin = app->centerY-1;
        ViewClipping.xMax = 1;
        ViewClipping.yMax = 1;
        eClean = millis();

        bAnimation = millis();
        
        if( xSemaphoreTake( RenderDataSemaphore, LUNOKIOT_EVENT_IMPORTANT_TIME_TICKS) != pdTRUE )  {
            // cannot get lock! discard this iteration
            lLog("Cpu: %u render timeout, retry\n",where->core);
            continue; 
        }

        // rotate vertex!
        for(int i=0;i<app->myPointsNumber;i++) {
            app->Rotate(app->myPoints[i], app->rot);
            Point2D pix;
            app->GetProjection(app->myPoints[i],pix);
            app->UpdateClipWith(ViewClipping,pix);
            int32_t deep = app->GetDeepForPoint(app->myPoints[i]);
            app->UpdateRange(MeshDimensions,deep);
            esp_task_wdt_reset();
        }
        // defined clip on screen
        //const int16_t bx = app->centerX+ViewClipping.xMin; 
        //const int16_t by = app->centerY+ViewClipping.yMin; 
        //const int16_t fx = app->centerX+ViewClipping.xMax; 
        //const int16_t fy = app->centerY+ViewClipping.yMax; 
        // new clip
        fpx = app->centerX+ViewClipping.xMin;
        fpy = app->centerY+ViewClipping.yMin;
        fpw = app->centerX+ViewClipping.xMax;
        fph = app->centerY+ViewClipping.yMax;
        clipWidth  = fpw-fpx;
        clipHeight  = fph-fpy;

        //lLog("NRECT: %d %d %d %d CLIP: %d %d\n",fpx,fpy,fpw,fph,clipWidth,clipHeight);

        // rotate normals
        for(int i=0;i<app->myNormalsNumber;i++) {
            app->Rotate(app->myNormals[i],app->rot);
            esp_task_wdt_reset();
        }
        eAnimation = millis();

        // render the layers
        bRender = millis();
        app->DirectRender(MeshDimensions,alphaChannel,zbuffer,normalsBuffer,polyBuffer,smoothBuffer,lightBuffer,buffer3d);
        eRender = millis();
        //DEBUG LAYERS
        //alphaChannel->pushSprite(0,0);
        //zbuffer->pushSprite(0,0);
        //normalsBuffer->pushSprite(0,0);
        //buffer3d->pushSprite(0,0);
        //return false;


        xSemaphoreGive( RenderDataSemaphore );

        bCompsite = millis();
        const int16_t rectWidth = clipWidth+border2;
        const int16_t rectHeight = clipHeight+border2;


        /*
        uint16_t lastPolyColor=TFT_BLACK;
        //PIXEL @SHADER ONE SHOOT ONLY
        for(int y=fpy;y<fph;y++) {
            for(int x=fpx;x<fpw;x++) {
                bool alpha = alphaChannel->readPixel(x,y);
                if ( false == alpha ) { continue; } // don't work out of mesh
                bool mustbeSmooth = smoothBuffer->readPixel(x,y);
                if ( false == mustbeSmooth ) { continue; } // only on smooth need shader
                { // convert zbuffer to PLAIN @TODO @DEBUG
                    uint16_t zc0 = zbuffer->readPixel(x,y);
                    uint16_t finalColor=zc0;
                    finalColor=TFT_WHITE; // @DEBUG disable channel
                    
                    //{
                    //    double r = ((finalColor >> 11) & 0x1F) / 31.0; // red   0.0 .. 1.0
                    //    double g = ((finalColor >> 5) & 0x3F) / 63.0;  // green 0.0 .. 1.0
                    //    double b = (finalColor & 0x1F) / 31.0;         // blue  0.0 .. 1.0
                    //    double med = (r+g+b)/3;
                    //    if ( med > 0.5 ) {
                    //        finalColor=TFT_WHITE;
                            //finalColor = zbuffer->alphaBlend(128,TFT_WHITE,finalColor);
                    //    }
                    //    else {
                     //       finalColor=TFT_WHITE;
                            //finalColor = zbuffer->alphaBlend(128,TFT_BLACK,finalColor);
                    //    }
                    }
                    zbuffer->drawPixel(x, y,finalColor);
                }
                { // modify normalbuffer to trigger bright ramp on iterative phase
                    uint16_t polyColor = polyBuffer->readPixel(x,y);
                    if ( polyColor != lastPolyColor ) {
                        uint16_t nc0 = normalsBuffer->readPixel(x,y);
                        uint16_t nc1 = normalsBuffer->readPixel(x+1,y);
                        //if (nc0 != nc1) {
                            uint16_t finalColor = normalsBuffer->alphaBlend(128,nc1,nc0);
                            normalsBuffer->drawPixel(x+1, y,finalColor);
                            //buffer3d->drawPixel(x, y,TFT_GREEN);
                        //}
                        lastPolyColor = polyColor;
                    }
                }
            }
        }*/

                    //@TODO PIXELSHADER WITH CALLBACK
 
        // work only with the relevant data
        TFT_eSprite * rectBuffer3d = new TFT_eSprite(ttgo->tft);
        if ( nullptr != rectBuffer3d ) {
            rectBuffer3d->setColorDepth(16);
            if ( NULL != rectBuffer3d->createSprite(rectWidth,rectHeight)) {
                // composite the image
                for(int y=fpy;y<fph;y++) {
                    for(int x=fpx;x<fpw;x++) {
                        //@COMPOSITION
                        bool alpha = alphaChannel->readPixel(x,y);
                        if ( false == alpha ) { continue; } // don't work out of mesh


                        // default composer
                        uint16_t plainColor = buffer3d->readPixel(x,y);
                        const int32_t px = (x+app->border)-fpx;
                        const int32_t py = (y+app->border)-fpy;
                        bool smoothMaterial = smoothBuffer->readPixel(x,y);
                        if ( smoothMaterial ) {
                            // smooth shader
                            uint16_t lightColor = lightBuffer->readPixel(x,y); // deep
                            if ( TFT_BLACK != lightColor ) {
                                double r = ((lightColor >> 11) & 0x1F) / 31.0; // red   0.0 .. 1.0
                                double g = ((lightColor >> 5) & 0x3F) / 63.0;  // green 0.0 .. 1.0
                                double b = (lightColor & 0x1F) / 31.0;         // blue  0.0 .. 1.0
                                uint8_t lintensity = 255*((r+g+b)/3.0);
                                uint16_t finalColor = rectBuffer3d->alphaBlend(lintensity,TFT_WHITE,plainColor);
                                rectBuffer3d->drawPixel(px, py,finalColor);
                            } else {
                                rectBuffer3d->drawPixel(px, py,plainColor);
                            }
                        } else { // flat shader
                            uint16_t colorZ = zbuffer->readPixel(x,y); // deep
                            uint16_t normalColor = normalsBuffer->readPixel(x,y); // volume
                            uint16_t shadowColor = rectBuffer3d->alphaBlend(128,normalColor,colorZ);
                            uint16_t finalColor = rectBuffer3d->alphaBlend(128,shadowColor,plainColor);
                            rectBuffer3d->drawPixel(px, py,finalColor);
                        }
    
                        esp_task_wdt_reset();
                    }
                }
                // @POST PROCESSING (direct on final sprite)
                /*
                // antialias
                for(int y=0;y<rectHeight;y++) {
                    for(int x=0;x<rectWidth-1;x++) {
                        uint16_t px = fpx+x;
                        uint16_t py = fpy+y;
                        bool alpha = alphaChannel->readPixel(px,py);
                        if ( false == alpha ) { continue; } // don't work out of mesh
                        uint16_t col0 = rectBuffer3d->readPixel(x,y);
                        uint16_t col1 = rectBuffer3d->readPixel(x+1,y);
                        uint16_t colMix = rectBuffer3d->alphaBlend(128,col0,col1);
                        rectBuffer3d->drawPixel(x,y,colMix);
                    }
                    esp_task_wdt_reset();
                }*/

                /*
                // borders
                for(int y=0;y<rectHeight;y++) {
                    for(int x=0;x<rectWidth-1;x++) {
                        uint16_t px = fpx+x;
                        uint16_t py = fpy+y;
                        bool alpha = alphaChannel->readPixel(px,py);
                        if ( false == alpha ) {
                            bool alpha2 = alphaChannel->readPixel(px+1,py);
                            if ( true == alpha2 ) {
                                rectBuffer3d->drawPixel(x+app->border,y+app->border,TFT_GREEN); // DEBUG PIXEL
                            }
                        } else {
                            bool alpha2 = alphaChannel->readPixel(px+1,py);
                            if ( false == alpha2 ) {
                                rectBuffer3d->drawPixel(x+app->border,y+app->border,TFT_GREEN); // DEBUG PIXEL
                            }
                        }
                        esp_task_wdt_reset();
                    }
                }*/

                /*
                // tint back
                for(int y=0;y<rectHeight;y++) {
                    for(int x=0;x<rectWidth-1;x++) {
                        uint16_t px = fpx+x;
                        uint16_t py = fpy+y;
                        bool alpha = alphaChannel->readPixel(px,py);
                        if ( true == alpha ) { continue; } // don't work out of mesh
                        rectBuffer3d->drawPixel(x,y,TFT_GREEN); // DEBUG PIXEL
                        esp_task_wdt_reset();
                    }
                }*/

                /*
                // Antialias test DEBUG
                for(int y=0;y<rectHeight;y++) {
                    for(int x=0;x<rectWidth-1;x++) {
                        uint16_t px = fpx+x;
                        uint16_t py = fpy+y;
                        bool alpha = alphaChannel->readPixel(px,py);
                        if ( false == alpha ) { continue; } // don't work out of mesh
                        uint16_t c0 = normalsBuffer->readPixel(px,py);
                        uint16_t c1 = normalsBuffer->readPixel(px+1,py);
                        if ( c0 != c1 ) {
                            rectBuffer3d->drawPixel(x+1,y,TFT_GREEN); // DEBUG PIXEL
                        }
                        esp_task_wdt_reset();
                    }
                }*/
                eCompsite = millis();
            }
        }
        //FreeSpace();
        showStats++;
        if ( 0 == (showStats % 25 )) {

            int64_t eTask = millis();
            int32_t cleanT = eClean-bClean;
            int32_t animT = eAnimation-bAnimation;
            int32_t renderT = eRender-bRender;
            int32_t compT = eCompsite-bCompsite;
            int32_t waitT = eWait-bWait;
            //int32_t pushT = ePush-bPush;

            int32_t taskT = eTask-bTask;
            int32_t fps = 1000/(eTask-bTask);
            uint8_t pcClean = (float(cleanT)/float(taskT))*100;
            uint8_t pcAnim = (float(animT)/float(taskT))*100;
            uint8_t pcRender = (float(renderT)/float(taskT))*100;
            uint8_t pcComp = (float(compT)/float(taskT))*100;
            //uint8_t pcPush = (float(pushT)/float(taskT))*100;
            lLog("Core %u: Clean: %d (%u%%%%) Anim: %d (%u%%%%) Render: %d (%u%%%%) Composite: %d (%u%%%%) Totals: %d MS FPS: %d\n",
                                where->core,
                                cleanT,pcClean,
                                animT,pcAnim,
                                renderT,pcRender,
                                compT,pcComp,
                                taskT,
                                fps);
            //FreeSpace();
        }
        esp_task_wdt_reset();

        if (where->task) {
            if ( 0 == where->core) {
                if (nullptr != RenderTask0Result) {
                    // Dropped frame!!
                    TFT_eSprite * oldframe = RenderTask0Result;
                    RenderTask0Result = nullptr;
                    oldframe->deleteSprite();
                    delete oldframe;
                    lLog("DROPPED FRAME on core0\n");
                }
                RenderTask0Result = rectBuffer3d;
                RenderTask0ResultViewClipping = ViewClipping;
            } else if ( 1 == where->core) {
                if (nullptr != RenderTask1Result) {
                    // Dropped frame!!
                    TFT_eSprite * oldframe = RenderTask1Result;
                    RenderTask1Result = nullptr;
                    oldframe->deleteSprite();
                    delete oldframe;
                    lLog("DROPPED FRAME on core1\n");
                }
                RenderTask1Result = rectBuffer3d;
                RenderTask1ResultViewClipping = ViewClipping;
            }
            continue;
        } else {
            RenderTask0Result = rectBuffer3d;
            RenderTask0ResultViewClipping = ViewClipping;
            break;
        }
    }
    // end the task!
    bool isTask=where->task;
    free(data);
    if ( nullptr != alphaChannel ) {
        alphaChannel->deleteSprite();
        delete alphaChannel;
        alphaChannel=nullptr;
    }
    if ( nullptr != zbuffer ) {
        zbuffer->deleteSprite();
        delete zbuffer;
        zbuffer=nullptr;
    }
    if ( nullptr != buffer3d ) {
        buffer3d->deleteSprite();
        delete buffer3d;
        buffer3d=nullptr;
    }
    if ( nullptr != normalsBuffer ) {
        normalsBuffer->deleteSprite();
        delete normalsBuffer;
        normalsBuffer=nullptr;
    }
    if ( nullptr != polyBuffer ) {
        polyBuffer->deleteSprite();
        delete polyBuffer;
        polyBuffer=nullptr;
    }
    if ( nullptr != smoothBuffer ) {
        smoothBuffer->deleteSprite();
        delete smoothBuffer;
        smoothBuffer=nullptr;
    }
    if ( nullptr != lightBuffer ) {
        lightBuffer->deleteSprite();
        delete lightBuffer;
        lightBuffer=nullptr;
    }
    if (isTask) {
        vTaskDelete(NULL);
    }
}

void Engine3DApplication::UpdateClipWith(INOUT Clipping2D &clip, IN Point2D &point) {
    UpdateClipWith(clip, point.x,point.y);
}

void Engine3DApplication::UpdateClipWith(INOUT Clipping2D &clip, IN int16_t x,IN int16_t y) {
    clip.xMin=MIN(clip.xMin,x);
    clip.yMin=MIN(clip.yMin,y);
    clip.xMax=MAX(clip.xMax,x);
    clip.yMax=MAX(clip.yMax,y);
}

void Engine3DApplication::UpdateRange(INOUT Range &range, IN int32_t value) {
    range.Min=MIN(range.Min,value);
    range.Max=MAX(range.Max,value);
}


// demo app entrypoint
Engine3DApplication::Engine3DApplication() {
    /*
    // precalculate angles
    ccosCache =(float*)calloc(3600,sizeof(float));
    csinCache =(float*)calloc(3600,sizeof(float));

    for(int i=0;i<3600;i++){ 
        float iii=i*0.01745;
        ccosCache[i]= cos(iii);
        csinCache[i]= sin(iii);
    }
    */
    // from generated meshdata blender python script
//@MESH
myPointsNumber = 114;
myFacesNumber = 224;
myNormalsNumber = 224;

    myVectorsNumber=0;
    rot.x=3.0; 
    rot.y=5.0;//1.5;
    rot.z=-2.0;
    /*
    rot.x=1.2;//0.75; 
    rot.y=2.9;//1.5;
    rot.z=4;//3.0;
    */
    lAppLog("Trying to allocate: %u byte...\n",sizeof(Point3D)*myPointsNumber);
//    myPoints=(Point3D *)ps_calloc(myPointsNumber,sizeof(Point3D));
    myPoints=(Point3D *)calloc(myPointsNumber,sizeof(Point3D));
    if ( nullptr == myPoints ) {
        lAppLog("Unable to allocate vertex data\n");
        return;
    }
    /*
    if ( myVectorsNumber > 0 ) {
        lAppLog("Trying to allocate: %u byte...\n",sizeof(Vector3D)*myVectorsNumber);
        myVectors=(Vector3D *)ps_calloc(myVectorsNumber,sizeof(Vector3D));
        if ( nullptr == myVectors ) {
            lAppLog("Unable to allocate vector data\n");
            return;
        }
    }*/
    lAppLog("Trying to allocate: %u byte...\n",sizeof(Face3D)*myFacesNumber);
    //myFaces=(Face3D *)ps_calloc(myFacesNumber,sizeof(Face3D));
    myFaces=(Face3D *)calloc(myFacesNumber,sizeof(Face3D));
    if ( nullptr == myFaces ) {
        lAppLog("Unable to allocate faces data\n");
        return;
    }

    lAppLog("Trying to allocate: %u byte...\n",sizeof(Normal3D)*myNormalsNumber);
    //myNormals=(Normal3D *)ps_calloc(myNormalsNumber,sizeof(Normal3D));
    myNormals=(Normal3D *)calloc(myNormalsNumber,sizeof(Normal3D));
    if ( nullptr == myFaces ) {
        lAppLog("Unable to allocate face normals data\n");
        return;
    }


    lAppLog("Loading mesh...\n");
    #include "../static/meshData.c"

    //facesToRender=(OrderedFace3D*)ps_calloc(myFacesNumber,sizeof(OrderedFace3D));
    facesToRender=(OrderedFace3D*)calloc(myFacesNumber,sizeof(OrderedFace3D));



    Point3D scaleVal;
    scaleVal.x=MeshSize;
    scaleVal.y=MeshSize;
    scaleVal.z=MeshSize;
    Point3D displace;
    displace.x=0.0;
    displace.y=0.0;
    displace.z=0.0;
    Angle3D rotation;
    rotation.x = 90; //random(0.0,360.0);
    rotation.y = 90; //random(0.0,360.0);
    rotation.z = 0; //random(0.0,360.0);

    lAppLog("Vertex re-process...\n");
    for(int i=0;i<myPointsNumber;i++) {
        Scale(myPoints[i],scaleVal); // do more big the blender output
        Translate(myPoints[i],displace);
        Rotate(myPoints[i],rotation);
        //myPoints[i].color=TFT_RED;
        //int16_t deep = GetDeepForPoint(myPoints[i]);
        //UpdateRange(MeshDimensions,deep);
    }
    /*
    TFT_eSprite * imgVertex = new TFT_eSprite(ttgo->tft);
    imgVertex->setColorDepth(16);
    imgVertex->createSprite(plane3DSample.width,plane3DSample.height);
    //imgVertex->setSwapBytes(true);
    imgVertex->pushImage(0,0,plane3DSample.width,plane3DSample.height,(uint16_t *)plane3DSample.pixel_data);
    */
    lAppLog("Face re-process...\n");
    int x=0;
    int y=0;
    for(int i=0;i<myFacesNumber;i++) {
        Rotate(myNormals[i],rotation);
    }
    /*
    imgVertex->deleteSprite();
    delete imgVertex;
    */

    // custom splash
    canvas->fillSprite(TFT_BLACK);
    TemplateApplication::btnBack->DrawTo(canvas);
    lAppLog("Mesh ready!\n");

    //core0Worker =(void *)ps_malloc(sizeof(RenderCore));
    core0Worker =(void *)malloc(sizeof(RenderCore));
    RenderCore * tmpCore0=(RenderCore *)core0Worker;
    tmpCore0->instance=this;
    tmpCore0->task=true;
    tmpCore0->run=true;
    tmpCore0->core=0;
    xTaskCreatePinnedToCore(RenderTask, "rendr0", LUNOKIOT_TASK_STACK_SIZE, tmpCore0, ENGINE_TASK_PRIORITY,&RenderTaskCore0,0);


    //core1Worker =(void *)ps_malloc(sizeof(RenderCore));
    core1Worker =(void *)malloc(sizeof(RenderCore));
    RenderCore * tmpCore1=(RenderCore *)core1Worker;
    tmpCore1->instance=this;
    tmpCore1->task=true;
    tmpCore1->run=true;
    tmpCore1->core=1;
    xTaskCreatePinnedToCore(RenderTask, "rendr1", LUNOKIOT_TASK_STACK_SIZE, tmpCore1, ENGINE_TASK_PRIORITY,&RenderTaskCore1,1);
}

Engine3DApplication::~Engine3DApplication() {
    if ( nullptr != myNormals ) { free(myNormals); }
    if ( nullptr != myFaces ) { free(myFaces); }
    if ( nullptr != myPoints ) { free(myPoints); }
    if ( nullptr != myVectors ) { free(myVectors); }

    if ( nullptr != core0Worker ) {
        ((RenderCore*)core0Worker)->run=false;
    }
    if ( nullptr != core1Worker ) {
        ((RenderCore*)core1Worker)->run=false;
    }

    if ( nullptr != RenderTask0Result ) {
        RenderTask0Result->deleteSprite();
        delete RenderTask0Result;
        RenderTask0Result=nullptr;
    }
    if ( nullptr != RenderTask1Result ) {
        RenderTask1Result->deleteSprite();
        delete RenderTask1Result;
        RenderTask1Result=nullptr;
    }
    if ( nullptr != facesToRender ) { free(facesToRender); }
    //if ( nullptr != ccosCache) { free(ccosCache); ccosCache=nullptr; }
    //if ( nullptr != csinCache) { free(csinCache); csinCache=nullptr; }

}

void Engine3DApplication::GetProjection(IN Light3D &vertex, OUT Point2D &pixel ) {
    pixel.x = (vertex.x-vertex.z)/sq2;
    pixel.y = (vertex.x+2*vertex.y+vertex.z)/sq6;
}

void Engine3DApplication::GetProjection(IN Point3D &vertex, OUT Point2D &pixel ) {
    // https://stackoverflow.com/questions/28607713/convert-3d-coordinates-to-2d-in-an-isometric-projection
    //*u=(x-z)/sqrt(2);
    //*v=(x+2*y+z)/sqrt(6);
    // u = x*cos(α) + y*cos(α+120°) + z*cos(α-120°)
    // v = x*sin(α) + y*sin(α+120°) + z*sin(α-120°)
    pixel.x = (vertex.x-vertex.z)/sq2;
    pixel.y = (vertex.x+2*vertex.y+vertex.z)/sq6;
}


//https://followtutorials.com/2012/03/3d-transformation-translation-rotation-and-scaling-in-c-c.html
void Engine3DApplication::RenderFace(INOUT TFT_eSprite *buffer, bool shaded,uint32_t overrideColor, bool borders,IN Point2D &one,IN Point2D &two,IN Point2D &tree) {
    /*
    if ( shaded ) {
        // this code must dissapear from here, shadows must be get from zbuffer
        int16_t deep0 = (p0->x+p0->z)/2;
        int16_t deep1 = (p1->x+p1->z)/2;
        int16_t deep2 = (p2->x+p2->z)/2;
        int medianDeep = (deep0+deep1+deep2)/3;
        uint8_t alpha=128+medianDeep;
        color = ttgo->tft->alphaBlend(alpha,face.color,TFT_BLACK); // color with shade
    } else {
        */
    uint32_t color=TFT_WHITE;
    if ( overrideColor != Drawable::MASK_COLOR) { color = overrideColor; }
    //}
    buffer->fillTriangle(centerX+one.x,centerY+one.y,
                            centerX+two.x,centerY+two.y,
                        centerX+tree.x,centerY+tree.y,color);
    if ( borders ) {
        buffer->drawTriangle(centerX+one.x,centerY+one.y,
                                centerX+two.x,centerY+two.y,
                            centerX+tree.x,centerY+tree.y,TFT_BLACK);
    }
}

void Engine3DApplication::RenderFace(INOUT TFT_eSprite *buffer, IN Face3D & face,bool shaded,uint32_t overrideColor, bool borders) {
    if ( face.vertexNum != 3 ) { lLog("WARNING Mesh must be triangulated before use!\n"); return; }
    // get the 3 involved vertex
    Point3D * p0 = face.vertexData[0];
    Point3D * p1 = face.vertexData[1];
    Point3D * p2 = face.vertexData[2];
    // get their 2d coordinates
    Point2D pix0;
    GetProjection(*p0,pix0);
    Point2D pix1;
    GetProjection(*p1,pix1);
    Point2D pix2;
    GetProjection(*p2,pix2);
    uint32_t color=face.color; // default face color (flat)
    if ( overrideColor != Drawable::MASK_COLOR) { color = overrideColor; }
    RenderFace(buffer,shaded,color,borders,pix0,pix1,pix2);
}
/*
float temp = 0; 
void Engine3DApplication::RenderPoint(INOUT TFT_eSprite *buffer,IN Point3D & point) {
    //if ( nullptr == point ) { return; }
    float x = point.x;
    float y = point.y;
    float z = point.z;
    // https://gamedev.stackexchange.com/questions/159434/how-to-convert-3d-coordinates-to-2d-isometric-coordinates
    Point2D pix;
    GetProjection(point,pix);
    //int16_t x2d = (x-z)/sq2;
    //int16_t y2d = (x +2*y + z)/sq6;
    // https://stackoverflow.com/questions/28607713/convert-3d-coordinates-to-2d-in-an-isometric-projection
    //*u=(x-z)/sqrt(2);
    //*v=(x+2*y+z)/sqrt(6);

    // ((x+z+y)/3)*-1; 
    //int16_t deep = (fabs(x)+fabs(y)+fabs(z))/3;
    int16_t deep = GetDeepForPoint(point);
    int32_t pointX=centerX+pix.x;
    int32_t pointY=centerY+pix.y;
    // only draw inside screen view
    if ( ActiveRect::InRect(pointX,pointY,0,0,canvas->height(),canvas->width()) ) {
        uint16_t alpha = 128-(deep*2);
        //lLog("deep: %d\n", deep);
        //if ( alpha > 60 ) {
        uint16_t mixColor = ttgo->tft->alphaBlend(alpha,point.color,TFT_BLACK);
        buffer3d->drawPixel(pointX,pointY,mixColor);
        //UpdateClipWith(ViewClipping,centerX+x2d,centerY+y2d);
    }

    // https://math.stackexchange.com/questions/118832/how-are-3d-coordinates-transformed-to-2d-coordinates-that-can-be-displayed-on-th
    // const int16_t Cz = 1;
    // int16_t x2d = x/TFT_WIDTH * ((TFT_WIDTH-0)/pow(2,z*Cz)) + (TFT_WIDTH- TFT_WIDTH/pow(2,z*Cz))/2;
    // int16_t y2d = y/TFT_HEIGHT * ((TFT_HEIGHT-0)/pow(2,z*Cz)) + (TFT_HEIGHT-TFT_HEIGHT/pow(2,z*Cz))/2;
    //canvas->fillCircle(x2d,y2d,5,TFT_WHITE);
    //lLog("Render Vertex: X: %f Y: %f Z: %f -> x: %d y: %d\n", x,y,z,x2d,y2d);

}
*/
void Engine3DApplication::Translate(INOUT Point3D & point, IN Point3D & translate) { //, float tx, float ty, float tz){
    point.x += translate.x;
    point.y += translate.y;
    point.z += translate.z;
}
/*
void Engine3DApplication::Rotate(INOUT Light3D & light, IN Angle3D & rotationAngles) {
    Point3D tmpPoint;
    tmpPoint.x=light.x;
    tmpPoint.y=light.y;
    tmpPoint.z=light.z;
    Rotate(tmpPoint,rotationAngles);
    light.x=tmpPoint.x;
    light.y=tmpPoint.y;
    light.z=tmpPoint.z;
}*/
void Engine3DApplication::Rotate(INOUT Normal3D & normal, IN Angle3D & rotationAngles) {
    float tempN;
    if ( 0 != rotationAngles.x ) {
        float anglex = (rotationAngles.x * M_PI) / 180.0;
        tempN = normal.vertexData[1];
        normal.vertexData[1] = normal.vertexData[1] * cos(anglex) - normal.vertexData[2] * sin(anglex);
        normal.vertexData[2] = tempN * sin(anglex) + normal.vertexData[2] * cos(anglex);
    }

    if ( 0 != rotationAngles.y ) {
        float angley = (rotationAngles.y * M_PI) / 180.0;
        tempN = normal.vertexData[2];
        normal.vertexData[2] = normal.vertexData[2] * cos(angley) - normal.vertexData[0] * sin(angley);
        normal.vertexData[0] = tempN * sin(angley) + normal.vertexData[0] * cos(angley);
    }

    if ( 0 != rotationAngles.y ) {
        float anglez = (rotationAngles.z * M_PI) / 180.0;
        tempN = normal.vertexData[0];
        normal.vertexData[0] = normal.vertexData[0] * cos(anglez) - normal.vertexData[1] * sin(anglez);
        normal.vertexData[1] = tempN * sin(anglez) + normal.vertexData[1] *cos(anglez);
    }
}
void Engine3DApplication::Rotate(INOUT Point3D & point, IN Angle3D & rotationAngles) {
    float temp;
    if ( 0 != rotationAngles.x ) {
        float anglex = (rotationAngles.x * M_PI) / 180.0;
        temp = point.y;
        //point.y = point.y * coscX - point.z * sincX;
        //point.z = temp * sincX + point.z * coscX;
        point.y = point.y * cos(anglex) - point.z * sin(anglex);
        point.z = temp * sin(anglex) + point.z * cos(anglex);
    }
    if ( 0 != rotationAngles.y ) {
        float angley = (rotationAngles.y * M_PI) / 180.0;
        temp = point.z;
        //point.z = point.z * coscY - point.x * sincY;
        //point.x = temp * sincY + point.x * coscY;
        point.z = point.z * cos(angley) - point.x * sin(angley);
        point.x = temp * sin(angley) + point.x * cos(angley);
    }
    if ( 0 != rotationAngles.z ) {
        float anglez = (rotationAngles.z * M_PI) / 180.0;
        temp = point.x;
        //point.x = point.x * coscZ - point.y * sincZ;
        //point.y = temp * sincZ + point.y *coscZ;
        point.x = point.x * cos(anglez) - point.y * sin(anglez);
        point.y = temp * sin(anglez) + point.y *cos(anglez);
    }    
    /*
    float sincX=csinCache[int(anglex*10)];
    float coscX=ccosCache[int(anglex*10)];
    float sincY=csinCache[int(angley*10)];
    float coscY=ccosCache[int(angley*10)];
    float sincZ=csinCache[int(anglez*10)];
    float coscZ=ccosCache[int(anglez*10)];
    */




    /*
    //https://stackoverflow.com/questions/2096474/given-a-surface-normal-find-rotation-for-3d-plane
    int imin = 0;
    for(int i=0; i<3; ++i) {
        if(abs(normal.vertexData[i]) < abs(normal.vertexData[imin])) {
            imin = i;
        }
    }

    float v2[3] = {0,0,0};
    float dt    = normal.vertexData[imin];

    v2[imin] = 1;
    for(int i=0;i<3;i++) {
        v2[i] -= dt*normal.vertexData[i];
    }
    for(int i=0;i<3;i++) {
        normal.vertexData[i]=v2[i];
    }*/
}

void Engine3DApplication::Scale(INOUT Point3D & point, IN Point3D & axes) {
    point.x *= axes.x;
    point.y *= axes.y;
    point.z *= axes.z;
    /*
    point.x = point.x * axes.x + (1 - axes.x) * point.x;
    point.y = point.y * axes.y + (1 - axes.y) * point.y;
    point.z = point.z * axes.z + (1 - axes.z) * point.z;
    */
} 

/*
void Engine3DApplication::RenderVector(IN Vector3D &vec) {
    float x0 = vec.from->x;
    float y0 = vec.from->y;
    float z0 = vec.from->z;

    float x1 = vec.to->x;
    float y1 = vec.to->y;
    float z1 = vec.to->z;
    // https://gamedev.stackexchange.com/questions/159434/how-to-convert-3d-coordinates-to-2d-isometric-coordinates
    int16_t x2d0 = (x0-z0)/ sq2;
    int16_t y2d0 = (x0 +2*y0 + z0) / sq6;
    int16_t x2d1 = (x1-z1)/ sq2;
    int16_t y2d1 = (x1 +2*y1 + z1) / sq6;
    // https://stackoverflow.com/questions/28607713/convert-3d-coordinates-to-2d-in-an-isometric-projection
    //*u=(x-z)/sqrt(2);
    //*v=(x+2*y+z)/sqrt(6);
    buffer3d->drawLine(centerX+x2d0,centerY+y2d0,centerX+x2d1,centerY+y2d1,vec.color);
    //lLog("Render Vector: (X: %f Y: %f Z: %f -> x: %d y: %d) --> (X: %f Y: %f Z: %f -> x: %d y: %d)\n", vec.from->x,vec.from->y,vec.from->z,centerX+x2d0,centerY+y2d0,vec.to->x,vec.to->y,vec.to->z,centerX+x2d1,centerY+y2d1);
    
    //UpdateClipWith(ViewClipping,centerX+x2d0,centerY+y2d0);
    //UpdateClipWith(ViewClipping,centerX+x2d1,centerY+y2d1);
}
*/
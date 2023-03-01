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

SemaphoreHandle_t Engine3DRenderChunk0Done =  xSemaphoreCreateMutex();
SemaphoreHandle_t Engine3DRenderChunk1Done =  xSemaphoreCreateMutex();

void Engine3DApplication::Render() {
    if( xSemaphoreTake( Engine3DRenderChunk0Done, portMAX_DELAY) != pdTRUE ) { return; }
    uint16_t faceColor = buffer3d->color565(255,255,255);
    const uint16_t shadowColor = buffer3d->color565(32,32,32);

    Point2D pixL0;
    GetProjection(Light0Point,pixL0);
    // conver to screen 
    pixL0.x=centerX+pixL0.x;
    pixL0.y=centerY+pixL0.y;
    Point2D pixL1;
    GetProjection(Light1Point,pixL1);
    // conver to screen 
    pixL1.x=centerX+pixL1.x;
    pixL1.y=centerY+pixL1.y;
    Point2D pixL2;
    GetProjection(Light2Point,pixL2);
    // conver to screen 
    pixL1.x=centerX+pixL2.x;
    pixL1.y=centerY+pixL2.y;

    for(int y=ViewClipping.yMin;y<ViewClipping.yMax;y++) {
        for(int x=ViewClipping.xMin;x<ViewClipping.xMax;x++) {
            // discard if nothing to draw
            bool mustDraw = alphaChannel->readPixel(x,y);
            if ( false == mustDraw ) { continue; }

            // get deep
            uint16_t colorzdeep = zbuffer->readPixel(x,y);
            double r = ((colorzdeep >> 11) & 0x1F) / 31.0; // red   0.0 .. 1.0
            double g = ((colorzdeep >> 5) & 0x3F) / 63.0;  // green 0.0 .. 1.0
            double b = (colorzdeep & 0x1F) / 31.0;         // blue  0.0 .. 1.0
            uint8_t zdeep = 255*((r+g+b)/3.0);

            // use deep to darken color
            uint16_t pixelColor=buffer3d->alphaBlend(zdeep,faceColor,shadowColor);

            // check lights
            bool light0Hit = ActiveRect::InRadius(x,y,pixL0.x,pixL0.y,Light0Point.radius);
            if ( light0Hit ) {
                // under the influence of light!!!
                
                // obtain vector from center
                int16_t tdvX = x-(pixL0.x+Light0Point.radius);
                int16_t tdvY = y-(pixL0.y+Light0Point.radius);
                // obtain distance between two points (radial active area)
                int16_t dist = sqrt(pow(tdvX, 2) + pow(tdvY, 2) * 1.0);

                // calculate the influence of light
                uint8_t alpha = 128*(dist/float(Light0Point.radius));
                pixelColor=buffer3d->alphaBlend(alpha,Light0Point.color,pixelColor); 
            }

            bool light1Hit = ActiveRect::InRadius(x,y,pixL1.x,pixL1.y,Light1Point.radius);
            if ( light1Hit ) {
                // under the influence of light!!!

                // obtain vector from center
                int16_t tdvX = pixL1.x-(x+Light1Point.radius);
                int16_t tdvY = pixL1.y-(y+Light1Point.radius);
                // obtain distance between two points (radial active area)
                int16_t dist = sqrt(pow(tdvX, 2) + pow(tdvY, 2) * 1.0);

                // calculate the influence of light
                uint8_t alpha = 128*(dist/float(Light1Point.radius));
                pixelColor=buffer3d->alphaBlend(alpha,Light1Point.color,pixelColor);                    
            }
            //GetProjection(Light2Point,pixL2);
            bool light2Hit = ActiveRect::InRadius(x,y,pixL2.x,pixL2.y,Light2Point.radius);
            if ( light2Hit ) {
                // under the influence of light!!!

                // obtain vector from center
                ///SWAAAAAAP!!!!!!! <==========================
                int16_t tdvX = x-(pixL2.x+Light2Point.radius);
                int16_t tdvY = y-(pixL2.y+Light2Point.radius);
                // obtain distance between two points (radial active area)
                float dist = sqrt(pow(tdvX, 2) + pow(tdvY, 2) * 1.0);
                // calculate the influence of light
                uint8_t alpha = 128*(dist/float(Light2Point.radius));
                pixelColor=buffer3d->alphaBlend(alpha,Light2Point.color,pixelColor);
            }

            buffer3d->drawPixel(x,y,pixelColor);
        }
    }
    buffer3d->drawCircle(pixL0.x,pixL0.y,5,Light0Point.color);
    buffer3d->drawCircle(pixL1.x,pixL1.y,5,Light1Point.color);
    buffer3d->drawCircle(pixL2.x,pixL2.y,5,Light2Point.color);
    /*
    // draw wire over draw
    const int16_t MaxDeep= MeshDimensions.Max+abs(MeshDimensions.Min);
    Point3D scaleFactor;
    scaleFactor.x=-3.0;
    scaleFactor.y=-3.0;
    scaleFactor.z=-3.0;
    for(int i=0;i<myFacesNumber;i++) {
        esp_task_wdt_reset();
        // get the 3 involved vertex

        Point3D * p0 = myFaces[i].vertexData[0];
        uint8_t zbuffer=GetZBufferDeepForPoint(*p0);
        int16_t deep0 = GetDeepForPoint(*p0);
        int32_t alpha=((deep0+abs(MeshDimensions.Min))*MaxDeep)/255;
        if ( zbuffer > alpha ) { continue; }

        Point3D * p1 = myFaces[i].vertexData[1];
        zbuffer=GetZBufferDeepForPoint(*p1);
        int16_t deep1 = GetDeepForPoint(*p1);
        alpha=((deep1+abs(MeshDimensions.Min))*MaxDeep)/255;
        if ( zbuffer > alpha ) { continue; }

        Point3D * p2 = myFaces[i].vertexData[2];
        zbuffer=GetZBufferDeepForPoint(*p2);
        int16_t deep2 = GetDeepForPoint(*p2);
        alpha=((deep2+abs(MeshDimensions.Min))*MaxDeep)/255;
        if ( zbuffer > alpha ) { continue; }
        // get their 2d coordinates
        Point2D pix0;
        GetProjection(*p0,pix0);
        Point2D pix1;
        GetProjection(*p1,pix1);
        Point2D pix2;
        GetProjection(*p2,pix2);

        buffer3d->drawTriangle(centerX+pix0.x,centerY+pix0.y,
                            centerX+pix1.x,centerY+pix1.y,
                        centerX+pix2.x,centerY+pix2.y,TFT_WHITE);
    }
    */
   xSemaphoreGive( Engine3DRenderChunk0Done );
}

int16_t Engine3DApplication::GetDeepForPoint(IN Point3D &point) {
    int16_t deep = (point.x+point.z)/2;
    return deep;
}

uint8_t Engine3DApplication::GetZBufferDeepForPoint(IN Point3D &point) {
    Point2D pix;
    GetProjection(point,pix);
    uint16_t colorzdeep = zbuffer->readPixel(pix.x,pix.y);
    double r = ((colorzdeep >> 11) & 0x1F) / 31.0; // red   0.0 .. 1.0
    double g = ((colorzdeep >> 5) & 0x3F) / 63.0;  // green 0.0 .. 1.0
    double b = (colorzdeep & 0x1F) / 31.0;         // blue  0.0 .. 1.0
    uint8_t zdeep = 255*((r+g+b)/3.0);
    return zdeep;

    /*
    const int16_t MaxDeep= MeshDimensions.Max+abs(MeshDimensions.Min);
    Point2D pix;
    GetProjection(point,pix);
    uint16_t pixDeep = zbuffer->readPixel(pix.x,pix.y);
    int32_t deep=(255*pixDeep)/MaxDeep;
    return deep;
    */
}

void Engine3DApplication::BuildZBuffer() {
    const int16_t MaxDeep= MeshDimensions.Max+abs(MeshDimensions.Min);
    // iterate all faces to determine their deep
    int16_t currentDepth=0;
    while ( currentDepth < 255 ) {
        for(int i=0;i<myFacesNumber;i++) {
            esp_task_wdt_reset();
            // get the 3 involved vertex
            Point3D * p0 = myFaces[i].vertexData[0];
            Point3D * p1 = myFaces[i].vertexData[1];
            Point3D * p2 = myFaces[i].vertexData[2];
            int16_t deep0 = GetDeepForPoint(*p0);
            int16_t deep1 = GetDeepForPoint(*p1);
            int16_t deep2 = GetDeepForPoint(*p2);
            int32_t medianDeep = (deep0+deep1+deep2)/3;
            int32_t alpha=((medianDeep+abs(MeshDimensions.Min))*MaxDeep)/255;
            if ( alpha < 0 ) { alpha = 0; }
            else if ( alpha > 255 ) { alpha = 255; }
            /*
            if (( alpha < 0 )||( alpha > 255 )) {
                lLog("WARNING! D0: %d D1: %d D2: %d med: %d res: %d\n", deep0, deep1,deep2,medianDeep,alpha);
                continue;
            }*/
            // only this line
            if ( currentDepth == alpha ) {
                //lLog("CUR: %d ALP: %d\n",currentDepth,alpha);
                RenderFace(zbuffer,myFaces[i],false,zbuffer->color565(alpha,alpha,alpha));
                //RenderFace(zbuffer,myFaces[i],false,alpha);
                //RenderFace(buffer3d,myFaces[i],false,TFT_WHITE);
                RenderFace(alphaChannel,myFaces[i],false,1);
                RenderPoint(zbuffer,*p0);
                RenderPoint(zbuffer,*p1);
                RenderPoint(zbuffer,*p2);

            }
        }
        currentDepth++;
    }
    //lLog("MESH HEIGHT: MIN: %d MAX: %d\n", MeshDimensions.Min,MeshDimensions.Max);
}
extern bool UILongTapOverride; // don't allow long tap for task switcher
bool Engine3DApplication::Tick() {
    UILongTapOverride=true; // remember every time
    TemplateApplication::btnBack->Interact(touched,touchX,touchY); // backbutton on bottom-right
    // clean last 3D buffer
    CleanBuffers();
    MeshDimensions.Max=0;
    MeshDimensions.Min=9000;
    if ( millis() > nextRefreshRotation ) {
        LampRotation.x=random(-2.0,2.0);
        LampRotation.y=random(-2.0,2.0);
        LampRotation.z=random(-2.0,2.0);
        nextRefreshRotation = millis()+3000;
    }


    // rotate animation!
    for(int i=0;i<myPointsNumber;i++) {
        Rotate(myPoints[i], rot);
        int16_t deep = GetDeepForPoint(myPoints[i]);
        UpdateRange(MeshDimensions,deep);
        esp_task_wdt_reset();
    }
    Rotate(Light0Point, LampRotation);
    Rotate(Light1Point, LampRotation);
    Rotate(Light2Point, LampRotation);

    // get the pixel depth
    BuildZBuffer();
    // begin the rendering!!
    Render();
    
    // draw faces
    //for(int i=0;i<myFacesNumber;i++) { RenderFace(buffer3d,myFaces[i],false,TFT_WHITE); esp_task_wdt_reset(); }
    //,false,TFT_PINK); esp_task_wdt_reset(); }

    // draw vectors (unused!)
    //for(int i=0;i<myVectorsNumber;i++) { RenderVector(myVectors[i]); }

    // draw vertex
    //for(int i=0;i<myPointsNumber;i++) { RenderPoint(myPoints[i]); }


    // use clip to know the need to render part
    TFT_eSprite * rectBuffer3d = new TFT_eSprite(ttgo->tft);
    if ( nullptr != rectBuffer3d ) {
        rectBuffer3d->setColorDepth(16);
        if ( NULL != rectBuffer3d->createSprite(ViewClipping.xMax-ViewClipping.xMin,
                                                ViewClipping.yMax-ViewClipping.yMin) ) {
            for(int y=0;y<rectBuffer3d->height();y++) {
                for(int x=0;x<rectBuffer3d->width();x++) {
                    uint16_t color = buffer3d->readPixel(ViewClipping.xMin+x,ViewClipping.yMin+y);
                    //uint16_t deep = zbuffer->readPixel(ViewClipping.xMin+x,ViewClipping.yMin+y);
                    rectBuffer3d->drawPixel(x,y,color);
                }
                esp_task_wdt_reset();
            }
            // push to screen
            rectBuffer3d->pushSprite(ViewClipping.xMin,ViewClipping.yMin);
            rectBuffer3d->deleteSprite();
        }
        delete rectBuffer3d;
    }
    return false;
}

void Engine3DApplication::UpdateClipWith(INOUT Clipping2D &clip, IN Point2D &point) {
    UpdateClipWith(clip, point.x,point.y);
}

void Engine3DApplication::UpdateClipWith(INOUT Clipping2D &clip, IN uint16_t x,IN uint16_t y) {
    clip.xMin=MIN(clip.xMin,x);
    clip.yMin=MIN(clip.yMin,y);
    clip.xMax=MAX(clip.xMax,x);
    clip.yMax=MAX(clip.yMax,y);
}

void Engine3DApplication::UpdateRange(INOUT Range &range, IN int32_t value) {
    range.Min=MIN(range.Min,value);
    range.Max=MAX(range.Max,value);
}

void Engine3DApplication::CleanBuffers() {
    if ( nullptr != buffer3d ) {
        // wipe only the necesary part
        buffer3d->fillRect(ViewClipping.xMin-5,
                           ViewClipping.yMin-5,
                           (ViewClipping.xMax-ViewClipping.xMin)+10,
                           (ViewClipping.yMax-ViewClipping.yMin)+10,TFT_BLACK); // cover old frame

    }
    // destroy zbuffer data
    if ( nullptr != zbuffer ) {
        zbuffer->fillRect(ViewClipping.xMin-5,
                           ViewClipping.yMin-5,
                           (ViewClipping.xMax-ViewClipping.xMin)+10,
                           (ViewClipping.yMax-ViewClipping.yMin)+10,TFT_BLACK); // cover old frame
    }
    if ( nullptr != alphaChannel ) {
        alphaChannel->fillRect(ViewClipping.xMin-5,
                           ViewClipping.yMin-5,
                           (ViewClipping.xMax-ViewClipping.xMin)+10,
                           (ViewClipping.yMax-ViewClipping.yMin)+10,TFT_BLACK); // cover old frame
    }
    if ( nullptr != zbufferFaceOrder ) {
        for (int i = 0; i < ZBufferFaceSize; i++) {
            free(zbufferFaceOrder[i]);
            zbufferFaceOrder[i]=nullptr;
        }
        free(zbufferFaceOrder);
        zbufferFaceOrder=nullptr;
    }
}
// demo app entrypoint
Engine3DApplication::Engine3DApplication() {
    // lets to initialize the image buffer (screen size) for draw with enough space
    lAppLog("Allocating TFT_eSPI Sprite as 3D buffer...\n");
    buffer3d = new TFT_eSprite(ttgo->tft);
    if ( nullptr != buffer3d ) {
        buffer3d->setColorDepth(16);
        if ( NULL == buffer3d->createSprite(canvas->width(), canvas->height()) ) {
            lAppLog("ERROR: unable to create buffer3d sprite! (app abort)\n");
            delete buffer3d;
            buffer3d=nullptr;
            return;
        }
        buffer3d->fillSprite(TFT_BLACK);
    }
    lAppLog("Buffer 3D generated\n");

    lAppLog("Allocating TFT_eSPI Sprite as ZBuffer...\n");
    zbuffer = new TFT_eSprite(ttgo->tft);
    if ( nullptr != zbuffer ) {
        zbuffer->setColorDepth(16);
        if ( NULL == zbuffer->createSprite(canvas->width(), canvas->height()) ) {
            lAppLog("ERROR: unable to create ZBuffer sprite! (app abort)\n");
            delete zbuffer;
            zbuffer=nullptr;
            return;
        }
        zbuffer->fillSprite(TFT_BLACK);
    }
    lAppLog("ZBuffer generated\n");


    lAppLog("Allocating TFT_eSPI Sprite as Alpha channel...\n");
    alphaChannel = new TFT_eSprite(ttgo->tft);
    if ( nullptr != alphaChannel ) {
        alphaChannel->setColorDepth(1);
        if ( NULL == alphaChannel->createSprite(canvas->width(), canvas->height()) ) {
            lAppLog("ERROR: unable to create Alpha channel sprite! (app abort)\n");
            delete alphaChannel;
            alphaChannel=nullptr;
            return;
        }
        alphaChannel->fillSprite(0);
    }
    lAppLog("Alpha channel generated\n");

    /*
    lAppLog("Allocating TFT_eSPI Sprite as lights...\n");
    light0 = new TFT_eSprite(ttgo->tft);
    if ( nullptr != light0 ) {
        light0->setColorDepth(1);
        if ( NULL == light0->createSprite(canvas->width(), canvas->height()) ) {
            lAppLog("ERROR: unable to create lights sprite! (app abort)\n");
            delete light0;
            light0=nullptr;
            return;
        }
        light0->fillSprite(0);
    }*/
    Light0Point.x=60;
    Light0Point.y=60;
    Light0Point.z=60;
    Light0Point.radius=120;
    Light0Point.color=TFT_RED;

    Light1Point.x=-60;
    Light1Point.y=60;
    Light1Point.z=60;
    Light1Point.radius=120;
    Light1Point.color=TFT_GREEN;

    Light2Point.x=60;
    Light2Point.y=-60;
    Light2Point.z=60;
    Light2Point.radius=120;
    Light2Point.color=TFT_BLUE;

    lAppLog("lights generated\n");


    // set the default valid values for renderclip (contains the size of the current frame)
    // calculated on the render step
    ViewClipping.xMin = centerX-1;
    ViewClipping.yMin = centerY-1;
    ViewClipping.xMax = centerX+1;
    ViewClipping.yMax = centerY+1;


    // from generated meshdata blender python script
//@MESH
myPointsNumber = 24;
myFacesNumber = 44;
    myVectorsNumber=0;

    lAppLog("Trying to allocate: %u byte...\n",sizeof(Point3D)*myPointsNumber);
    myPoints=(Point3D *)ps_calloc(myPointsNumber,sizeof(Point3D));
    if ( nullptr == myPoints ) {
        lAppLog("Unable to allocate vertex data\n");
        return;
    }
    if ( myVectorsNumber > 0 ) {
        lAppLog("Trying to allocate: %u byte...\n",sizeof(Vector3D)*myVectorsNumber);
        myVectors=(Vector3D *)ps_calloc(myVectorsNumber,sizeof(Vector3D));
        if ( nullptr == myVectors ) {
            lAppLog("Unable to allocate vector data\n");
            return;
        }
    }
    lAppLog("Trying to allocate: %u byte...\n",sizeof(unsigned int*)*myFacesNumber);
    myFaces=(Face3D *)ps_calloc(myFacesNumber,sizeof(Face3D));
    if ( nullptr == myFaces ) {
        lAppLog("Unable to allocate faces data\n");
        return;
    }

    lAppLog("Loading mesh...\n");
    #include "../static/meshData.c"

    Point3D scaleVal;
    scaleVal.x=MeshSize;
    scaleVal.y=MeshSize;
    scaleVal.z=MeshSize;
    Point3D displace;
    displace.x=0.0;
    displace.y=0.0;
    displace.z=0.0;
    Angle3D rotation;
    rotation.x = random(0.0,360.0);
    rotation.y = random(0.0,360.0);
    rotation.z = random(0.0,360.0);

    lAppLog("Vertex re-process...\n");
    for(int i=0;i<myPointsNumber;i++) {
        Scale(myPoints[i],scaleVal); // do more big the blender output
        Translate(myPoints[i],displace);
        Rotate(myPoints[i],rotation);
        myPoints[i].color=TFT_WHITE;
        int16_t deep = GetDeepForPoint(myPoints[i]);
        UpdateRange(MeshDimensions,deep);        
        esp_task_wdt_reset();
    }
    lAppLog("Face re-process...\n");
    for(int i=0;i<myFacesNumber;i++) { 
        myFaces[i].color=TFT_LIGHTGREY;
        esp_task_wdt_reset();
    }

    rot.x=1.0; 
    rot.y=1.0;
    rot.z=1.0;

    LampRotation.x=2;
    LampRotation.y=2;
    LampRotation.z=2;

    // custom splash
    canvas->fillSprite(TFT_BLACK);
    TemplateApplication::btnBack->DrawTo(canvas);
    lAppLog("Mesh ready!\n");
}

Engine3DApplication::~Engine3DApplication() {
    if ( nullptr != myFaces ) { free(myFaces); }
    if ( nullptr != myPoints ) { free(myPoints); }
    if ( nullptr != myVectors ) { free(myVectors); }
    if ( nullptr != zbuffer ) {
        zbuffer->deleteSprite();
        delete zbuffer;
    }
    if ( nullptr != alphaChannel ) {
        alphaChannel->deleteSprite();
        delete alphaChannel;
    }
    /*
    if ( nullptr != light0 ) {
        light0->deleteSprite();
        delete light0;
    }*/

    if ( nullptr != buffer3d ) {
        buffer3d->deleteSprite();
        delete buffer3d;
    }
    if ( nullptr != zbufferFaceOrder ) {
        for (int i = 0; i < ZBufferFaceSize; i++) {
            free(zbufferFaceOrder[i]);
        }
        free(zbufferFaceOrder);
    }
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

void Engine3DApplication::RenderFace(INOUT TFT_eSprite *buffer, IN Face3D & face,bool shaded,uint32_t overrideColor) {
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
    /*
    uint32_t maskColor=AvailColors[AvailColorsoffset];
    AvailColorsoffset++;
    if ( AvailColorsoffset > (sizeof(AvailColors)/sizeof(uint32_t)-1) ){
        AvailColorsoffset=0;
    }*/
    uint32_t color=face.color; // default face color (flat)
    if ( shaded ) {
        // this code must dissapear from here, shadows must be get from zbuffer
        int16_t deep0 = (p0->x+p0->z)/2;
        int16_t deep1 = (p1->x+p1->z)/2;
        int16_t deep2 = (p2->x+p2->z)/2;
        int medianDeep = (deep0+deep1+deep2)/3;
        uint8_t alpha=128+medianDeep;
        color = ttgo->tft->alphaBlend(alpha,face.color,TFT_BLACK); // color with shade
    } else {
        if ( overrideColor != Drawable::MASK_COLOR) { color = overrideColor; }
    }
    buffer->fillTriangle(centerX+pix0.x,centerY+pix0.y,
                            centerX+pix1.x,centerY+pix1.y,
                        centerX+pix2.x,centerY+pix2.y,color);
    // update render clip
    UpdateClipWith(ViewClipping,centerX+p0->x,centerY+p0->y);
    UpdateClipWith(ViewClipping,centerX+p1->x,centerY+p1->y);
    UpdateClipWith(ViewClipping,centerX+p2->x,centerY+p2->y);
}

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

void Engine3DApplication::Translate(INOUT Point3D & point, IN Point3D & translate) { //, float tx, float ty, float tz){
    point.x += translate.x;
    point.y += translate.y;
    point.z += translate.z;
}

void Engine3DApplication::Rotate(INOUT Light3D & light, IN Angle3D & rotationAngles) {
    Point3D tmpPoint;
    tmpPoint.x=light.x;
    tmpPoint.y=light.y;
    tmpPoint.z=light.z;
    Rotate(tmpPoint,rotationAngles);
    light.x=tmpPoint.x;
    light.y=tmpPoint.y;
    light.z=tmpPoint.z;
}

void Engine3DApplication::Rotate(INOUT Point3D & point, IN Angle3D & rotationAngles) {
    float anglex = rotationAngles.x * M_PI / 180.0;
    temp = point.y;
    point.y = point.y * cos(anglex) - point.z * sin(anglex);
    point.z = temp * sin(anglex) + point.z * cos(anglex);

    float angley = (rotationAngles.y * M_PI) / 180.0;
    temp = point.z;
    point.z = point.z * cos(angley) - point.x * sin(angley);
    point.x = temp * sin(angley) + point.x * cos(angley);

    float anglez = rotationAngles.z * M_PI / 180.0;
    temp = point.x;
    point.x = point.x * cos(anglez) - point.y * sin(anglez);
    point.y = temp * sin(anglez) + point.y *cos(anglez);

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
    
    UpdateClipWith(ViewClipping,centerX+x2d0,centerY+y2d0);
    UpdateClipWith(ViewClipping,centerX+x2d1,centerY+y2d1);
}

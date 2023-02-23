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
#include <cmath>
#include <esp_task_wdt.h>

//https://followtutorials.com/2012/03/3d-transformation-translation-rotation-and-scaling-in-c-c.html

// https://www.geeksforgeeks.org/boundary-fill-algorithm/
// Function for 4 connected Pixels
void Engine3DApplication::BoundaryFill4(int32_t x, int32_t y, uint16_t fill_color,uint16_t boundary_color) {
    int32_t ox=x;
    int32_t oy=y;

    uint32_t currCol = buffer3d->readPixel(x,y);
    if(currCol != boundary_color && currCol != fill_color) {
        buffer3d->drawPixel(x,y,fill_color);
        BoundaryFill4(x + 1, y, fill_color, boundary_color);
        //BoundaryFill4(x, y + 1, fill_color, boundary_color);
        //BoundaryFill4(x - 1, y, fill_color, boundary_color);
        //BoundaryFill4(x, y - 1, fill_color, boundary_color);
    }
}
/*
bool Engine3DApplication::FillWithColor(int16_t x,int16_t y,uint32_t color,uint32_t stopColor) {
    int16_t ox=x;
    int16_t oy=y;
    while(true) {
        esp_task_wdt_reset();
        uint32_t currCol = buffer3d->readPixel(x,y);
        if ( stopColor == currCol ) { break; }
        if ( color == currCol ) { break; }
        buffer3d->drawPixel(x,y,color);

        x+=1;
        //if ( x<0 ) { x=ox; y++; continue; }
        if ( x > buffer3d->width() ) {
            y++;
            x=ox;
            if ( y > buffer3d->height() ) {
                break;
            }
            continue;
        }
    }
    return true;
}
*/
//int engineLock=0;
void Engine3DApplication::RenderFace(Face3D & face) {
    //if ( engineLock > 10 ) { return; }
    if ( face.vertexNum != 3 ) { lLog("WARNING Mesh must be triangulated before use!\n"); return; }

    Point3D * p0 = face.vertexData[0];
    Point3D * p1 = face.vertexData[1];
    Point3D * p2 = face.vertexData[2];

    //int16_t deep0 = (p0->x+p0->z)/2;

    int16_t deep0 = (p0->z+p0->y)/2;
    int16_t deep1 = (p1->z+p1->y)/2;
    int16_t deep2 = (p2->z+p2->y)/2;
    int deep = (deep0+deep1+deep2)/3;

    //if ( deep < -10 ) { return; }
    int16_t x0 = (p0->x-p0->z)/sq2;
    int16_t y0 = (p0->x +2*p0->y + p0->z)/sq6;
    int16_t x1 = (p1->x-p1->z)/sq2;
    int16_t y1 = (p1->x +2*p1->y + p1->z)/sq6;
    int16_t x2 = (p2->x-p2->z)/sq2;
    int16_t y2 = (p2->x +2*p2->y + p2->z)/sq6;

    if ( centerX+x0 < minLastX ) { minLastX = centerX+x0; }
    if ( centerY+y0 < minLastY ) { minLastY = centerY+y0; }
    if ( centerX+x1 < minLastX ) { minLastX = centerX+x1; }
    if ( centerY+y1 < minLastY ) { minLastY = centerY+y1; }
    if ( centerX+x2 < minLastX ) { minLastX = centerX+x2; }
    if ( centerY+y2 < minLastY ) { minLastY = centerY+y2; }
    if ( centerX+x0 > maxLastX ) { maxLastX = centerX+x0; }
    if ( centerY+y0 > maxLastY ) { maxLastY = centerY+y0; }
    if ( centerX+x1 > maxLastX ) { maxLastX = centerX+x1; }
    if ( centerY+y1 > maxLastY ) { maxLastY = centerY+y1; }
    if ( centerX+x2 > maxLastX ) { maxLastX = centerX+x2; }
    if ( centerY+y2 > maxLastY ) { maxLastY = centerY+y2; }

    uint32_t maskColor=AvailColors[AvailColorsoffset];
    AvailColorsoffset++;
    if ( AvailColorsoffset > (sizeof(AvailColors)/sizeof(uint32_t)-1) ){
        AvailColorsoffset=0; //engineLock++;
    }

    buffer3d->fillTriangle(centerX+x0,centerY+y0,centerX+x1,centerY+y1,centerX+x2,centerY+y2,TFT_DARKGREY);
    buffer3d->drawTriangle(centerX+x0,centerY+y0,centerX+x1,centerY+y1,centerX+x2,centerY+y2,TFT_BLACK);
}

float temp = 0; 
void Engine3DApplication::RenderPoint(Point3D & point) {
    //if ( nullptr == point ) { return; }
    float x = point.x;
    float y = point.y;
    float z = point.z;
    // https://gamedev.stackexchange.com/questions/159434/how-to-convert-3d-coordinates-to-2d-isometric-coordinates
    int16_t x2d = (x-z)/sq2;
    int16_t y2d = (x +2*y + z)/sq6;
    // https://stackoverflow.com/questions/28607713/convert-3d-coordinates-to-2d-in-an-isometric-projection
    //*u=(x-z)/sqrt(2);
    //*v=(x+2*y+z)/sqrt(6);

    // ((x+z+y)/3)*-1; 
    //int16_t deep = (fabs(x)+fabs(y)+fabs(z))/3;
    int16_t deep = (x+z)/2;
    int32_t pointX=centerX+x2d;
    int32_t pointY=centerY+y2d;
    // only draw inside screen view
    if ( ActiveRect::InRect(pointX,pointY,0,0,canvas->height(),canvas->width()) ) {
        uint16_t alpha = 128-(deep*2);
        //lLog("deep: %d\n", deep);
        if ( alpha > 60 ) {
            uint16_t mixColor = ttgo->tft->alphaBlend(alpha,point.color,TFT_BLACK);
            buffer3d->drawPixel(pointX,pointY,mixColor);
        }
        /*
        if ( deep < -20 ) {
            // dont draw hidden side
            //buffer3d->drawPixel(pointX,pointY, ttgo->tft->alphaBlend(192,TFT_BLACK,point.color));
        } else if ( deep > -10 ) {
            uint16_t mixColor = ttgo->tft->alphaBlend(128+deep,TFT_BLACK,point.color);
            if ( deep > 20 ) { mixColor = point.color; }
            //buffer3d->fillCircle(pointX,pointY,2,mixColor);
            buffer3d->drawPixel(pointX,pointY,mixColor);
        }
        */
    }

    // https://math.stackexchange.com/questions/118832/how-are-3d-coordinates-transformed-to-2d-coordinates-that-can-be-displayed-on-th
    // const int16_t Cz = 1;
    // int16_t x2d = x/TFT_WIDTH * ((TFT_WIDTH-0)/pow(2,z*Cz)) + (TFT_WIDTH- TFT_WIDTH/pow(2,z*Cz))/2;
    // int16_t y2d = y/TFT_HEIGHT * ((TFT_HEIGHT-0)/pow(2,z*Cz)) + (TFT_HEIGHT-TFT_HEIGHT/pow(2,z*Cz))/2;
    //canvas->fillCircle(x2d,y2d,5,TFT_WHITE);
    //lLog("Render Vertex: X: %f Y: %f Z: %f -> x: %d y: %d\n", x,y,z,x2d,y2d);

    if ( centerX+x2d < minLastX ) { minLastX = centerX+x2d; }
    if ( centerY+y2d < minLastY ) { minLastY = centerY+y2d; }
    if ( centerX+x2d > maxLastX ) { maxLastX = centerX+x2d; }
    if ( centerY+y2d > maxLastY ) { maxLastY = centerY+y2d; }
}

void Engine3DApplication::TranslatePoint(Point3D & point, Point3D & translate) { //, float tx, float ty, float tz){
    point.x += translate.x;
    point.y += translate.y;
    point.z += translate.z;
} 

void Engine3DApplication::Rotate(Point3D & point, Angle3D & rotationAngles) {
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

void Engine3DApplication::Scale(Point3D & point, Point3D & axes) {
//void scale(float sf, float xf, float yf, float zf){
    point.x *= axes.x;
    point.y *= axes.y;
    point.z *= axes.z;
    /*
    point.x = point.x * axes.x + (1 - axes.x) * point.x;
    point.y = point.y * axes.y + (1 - axes.y) * point.y;
    point.z = point.z * axes.z + (1 - axes.z) * point.z;
    */
} 


Engine3DApplication::Engine3DApplication() {
    lAppLog("Allocating 3D buffer...\n");
    buffer3d = new TFT_eSprite(ttgo->tft);
    if ( nullptr != buffer3d ) { 
        buffer3d->setColorDepth(16);
        if ( NULL == buffer3d->createSprite(canvas->width(), canvas->height()) ) {
            lAppLog("ERROR: unable to create buffer3d sprite\n");
            delete buffer3d;
            buffer3d=nullptr;
            return;
        }
        buffer3d->fillSprite(TFT_BLACK);
    }
    lAppLog("Buffer 3D generated\n");
    
    // from generated meshdata blender python script
    myPointsNumber = 507;
    myFacesNumber = 967;
    myVectorsNumber=0;

    lAppLog("Trying to allocate: %u byte...\n",sizeof(Point3D)*myPointsNumber);
    myPoints=(Point3D *)ps_calloc(myPointsNumber,sizeof(Point3D));
    if ( nullptr == myPoints ) {
        lAppLog("Unable to allocate vertex data step 0\n");
        return;
    }
    if ( myVectorsNumber > 0 ) {
        lAppLog("Trying to allocate: %u byte...\n",sizeof(Vector3D)*myVectorsNumber);
        myVectors=(Vector3D *)ps_calloc(myVectorsNumber,sizeof(Vector3D));
        if ( nullptr == myVectors ) {
            lAppLog("Unable to allocate vector data step 1\n");
            return;
        }
    }
    lAppLog("Trying to allocate: %u byte...\n",sizeof(unsigned int*)*myFacesNumber);
    myFaces=(Face3D *)ps_calloc(myFacesNumber,sizeof(Face3D));
    if ( nullptr == myFaces ) {
        lAppLog("Unable to allocate vertex data step 2\n");
        return;
    }

    lAppLog("Loading mesh...\n");
    #include "../static/meshData.c"

    Point3D scaleVal;
    scaleVal.x=80.0;
    scaleVal.y=80.0;
    scaleVal.z=80.0;
    Point3D displace;
    displace.x=0.0;
    displace.y=0.0;
    displace.z=0.0;
    Angle3D rotation;
    rotation.x = random(0.0,360.0);
    rotation.y = random(0.0,360.0);
    rotation.z = random(0.0,360.0);

    lAppLog("Vertex process...\n");
    for(int i=0;i<myPointsNumber;i++) {
        Scale(myPoints[i],scaleVal); // do more big the blender output
        TranslatePoint(myPoints[i],displace);
        Rotate(myPoints[i],rotation);
        myPoints[i].color=TFT_WHITE;
        esp_task_wdt_reset();
    }
    //lAppLog("Tint vectors\n");
    //for(int i=0;i<myVectorsNumber;i++) { myVectors[i].color=TFT_WHITE; esp_task_wdt_reset(); }



    rot.x=1.0; 
    rot.y=1.0;
    rot.z=1.0;

    // custom splash
    canvas->fillSprite(TFT_BLACK);
    TemplateApplication::btnBack->DrawTo(canvas);
    lAppLog("Mesh ready!\n");
}

Engine3DApplication::~Engine3DApplication() {
    if ( nullptr != myFaces ) { free(myFaces); }
    if ( nullptr != myPoints ) { free(myPoints); }
    if ( nullptr != myVectors ) { free(myVectors); }
    if ( nullptr != buffer3d ) {
        buffer3d->deleteSprite();
        delete buffer3d;
        buffer3d = nullptr;
    }
}

void Engine3DApplication::RenderVector(Vector3D &vec) {
    //const int16_t Cz = 1;
    const int16_t centerX = canvas->width()/2;
    const int16_t centerY = canvas->height()/2;

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

    if ( centerX+x2d0 < minLastX ) { minLastX = centerX+x2d0; }
    if ( centerY+y2d0 < minLastY ) { minLastY = centerY+y2d0; }
    if ( centerX+x2d0 > maxLastX ) { maxLastX = centerX+x2d0; }
    if ( centerY+y2d0 > maxLastY ) { maxLastY = centerY+y2d0; }

    if ( centerX+x2d1 < minLastX ) { minLastX = centerX+x2d1; }
    if ( centerY+y2d1 < minLastY ) { minLastY = centerY+y2d1; }
    if ( centerX+x2d1 > maxLastX ) { maxLastX = centerX+x2d1; }
    if ( centerY+y2d1 > maxLastY ) { maxLastY = centerY+y2d1; }
}
extern bool UILongTapOverride;
bool Engine3DApplication::Tick() {
    UILongTapOverride=true;
    TemplateApplication::btnBack->Interact(touched,touchX,touchY);
    if ( millis() > nextRefresh ) { // redraw full canvas
        /*if (( nullptr == myVectors ) || ( nullptr == myPoints )) {
            lAppLog("ERROR: 3d data buffers empty\n");
            return false;
        }*/
        //engineLock=0;
        buffer3d->fillRect(minLastX-5,minLastY-5,(maxLastX-minLastX)+10,(maxLastY-minLastY)+10,TFT_BLACK); // cover old frame


        // draw faces
        for(int i=0;i<myFacesNumber;i++) { RenderFace(myFaces[i]); esp_task_wdt_reset(); }

        // draw vectors
        //for(int i=0;i<myVectorsNumber;i++) { RenderVector(myVectors[i]); }

        // draw vertex
        //for(int i=0;i<myPointsNumber;i++) { RenderPoint(myPoints[i]); }

        // animation!
        for(int i=0;i<myPointsNumber;i++) { Rotate(myPoints[i], rot); esp_task_wdt_reset(); }

        /*
        // animation with BMA!
        rot.x=(degX-lastRot.x)/2; // acceleromether degrees
        rot.y=(degY-lastRot.y)/2;
        rot.z=(degZ-lastRot.z)/2;
        for(int i=0;i<myPointsNumber;i++) { Rotate(myPoints[i], rot); }
        lastRot.x=degX;
        lastRot.y=degY;
        lastRot.z=degZ;
        */

//        nextStep=millis()+(1000/24);
//    }
        //lLog("DEBUG MINX: %d MINY: %d MAXX: %d MAXY: %d\n",minLastX,minLastY,maxLastX-minLastX,maxLastY-minLastY);
        TFT_eSprite * rectBuffer3d = new TFT_eSprite(ttgo->tft);
        if ( nullptr != rectBuffer3d ) {
            rectBuffer3d->setColorDepth(16);
            if ( NULL != rectBuffer3d->createSprite(maxLastX-minLastX, maxLastY-minLastY) ) {
                for(int y=0;y<rectBuffer3d->height();y++) {
                    for(int x=0;x<rectBuffer3d->width();x++) {
                        uint16_t color = buffer3d->readPixel(minLastX+x,minLastY+y);
                        rectBuffer3d->drawPixel(x,y,color);
                    }
                    esp_task_wdt_reset();
                }
                //unsigned long bPush = millis();
                rectBuffer3d->pushSprite(minLastX,minLastY);
                //unsigned long pushTime = millis()-bPush;
                //lLog("FPS: %lu\n",pushTime/60);
                rectBuffer3d->deleteSprite();
            }
            delete rectBuffer3d;
        }
        //buffer3d->pushSprite(0,0);
        //nextRefresh=millis()+(1000/24); // 8 FPS is enought for GUI
        ///return false;
    }
    return false;
}

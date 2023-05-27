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

#include "View3D.hpp"
#include "../../../app/LogView.hpp"
#include <sys/param.h>
#include <cmath>
#include <esp_task_wdt.h>
#include "Mesh3D.hpp"
using namespace LuI;

SemaphoreHandle_t View3D::renderMutex = xSemaphoreCreateMutex();
TaskHandle_t View3D::renderTask = NULL;

void View3D::AddMesh3D(INOUT Mesh3D * meshObject) {
    bool foundSlot=false;
    int offset=0;
    for(offset=0;offset<MAX_MESHES;offset++) {
        if ( nullptr == mesh[offset] ) {
            foundSlot=true;
            break;
        }
    }
    if ( false == foundSlot ) {
        lUILog("View3D %p View FULL, Cannot add more than %d meshes\n",this,MAX_MESHES);
        return;
    }
    mesh[offset] = meshObject;
    meshObject->dirty=true;
    dirty=true;
}

float View3D::OriginalNormalFacing(INOUT Normal3D &normal) {
    if ( ENGINE_INVALID_VALUE != normal._deepCache ) { return normal._deepCache; }
    normal._deepCache=normal.vertexData[0]-normal.vertexData[1]+normal.vertexData[2];  
    //if(deep<0){ return true; }
    return normal._deepCache;
}

float View3D::NormalFacing(INOUT Normal3D &normal) {
    if ( ENGINE_INVALID_VALUE != normal._deepCache ) { return normal._deepCache; }
    normal._deepCache=normal.vertexData[0]-normal.vertexData[1]+normal.vertexData[2];  
    //if(deep<0){ return true; }
    return normal._deepCache;
}


void View3D::Refresh(bool direct) {
    if ( false == dirty ) { return; }
    //lLog("View3D %p refresh canvas at %p direct: %s\n",this,canvas,(direct?"true":"false"));
    Control::Refresh();
    centerX = width/2;
    centerY = height/2;
    // iterate all meshes to apply their last transformations
    int offset=0;
    for(offset=0;offset<MAX_MESHES;offset++) {
        if ( nullptr == mesh[offset] ) { continue; }
        mesh[offset]->ApplyTransform(GlobalLocation,GlobalRotation,GlobalScale);
    }
    Render();
    if (( directDraw ) && ( direct )) {
        canvas->pushSprite(clipX,clipY);
    }
    if ( nullptr != stepCallback ) { (stepCallback)(stepCallbackParam); }
}

void View3D::SetAllDirty() {
    for(int offset=0;offset<MAX_MESHES;offset++) {
        if ( nullptr == mesh[offset] ) { continue; }
        mesh[offset]->dirty=true;
    }
    dirty=true;
}

void View3D::SetGlobalLocation(IN Vertex3D globLocation ) {
    GlobalLocation=globLocation;
    SetAllDirty();
}

void View3D::SetGlobalScale(IN float scale ) {
    GlobalScale= { scale,scale,scale };
    SetAllDirty();
}

void View3D::SetGlobalScale(IN Vertex3D globScale ) {
    GlobalScale=globScale;
    SetAllDirty();
}
void View3D::SetGlobalRotation(INOUT Angle3D globRotation ) {
    // apply global shit
    if ( globRotation.x > 359 ) { globRotation.x-=360; }
    else if ( globRotation.x < 0 ) { globRotation.x+=360; }
    if ( globRotation.y > 359 ) { globRotation.y-=360; }
    else if ( globRotation.y < 0 ) { globRotation.y+=360; }
    if ( globRotation.z > 359 ) { globRotation.z-=360; }
    else if ( globRotation.z < 0 ) { globRotation.z+=360; }
    GlobalRotation.x=globRotation.x;
    GlobalRotation.y=globRotation.y;
    GlobalRotation.z=globRotation.z;
    SetAllDirty();
}

void View3D::UpdateClipWith(INOUT Clipping2D &clip, IN Point2D &point) {
    UpdateClipWith(clip, point.x,point.y);
}

void View3D::UpdateClipWith(INOUT Clipping2D &clip, IN int16_t x,IN int16_t y) {
    clip.xMin=MIN(clip.xMin,x);
    clip.yMin=MIN(clip.yMin,y);
    clip.xMax=MAX(x,clip.xMax);
    clip.yMax=MAX(y,clip.yMax);
}

void View3D::GetProjection(INOUT Vertex3D &vertex, OUT Point2D &pixel ) {
    // https://stackoverflow.com/questions/28607713/convert-3d-coordinates-to-2d-in-an-isometric-projection
    //*u=(x-z)/sqrt(2);
    //*v=(x+2*y+z)/sqrt(6);
    // u = x*cos(α) + y*cos(α+120°) + z*cos(α-120°)
    // v = x*sin(α) + y*sin(α+120°) + z*sin(α-120°)
    if ( vertex._xCache == ENGINE_INVALID_VALUE ) {
        vertex._xCache=(vertex.x-vertex.z)/Mesh3D::sq2;
    }
    if ( vertex._yCache == ENGINE_INVALID_VALUE ) {
        vertex._yCache = (vertex.x+2*vertex.y+vertex.z)/Mesh3D::sq6;
    }
    pixel.x=vertex._xCache;
    pixel.y=vertex._yCache;
}

void View3D::UpdateRange(INOUT Range &range, IN int32_t value) {
    range.Min=MIN(range.Min,value);
    range.Max=MAX(range.Max,value);
}

int16_t View3D::GetDeepForPoint(INOUT Vertex3D &point) {
    if (point._deepCache != ENGINE_INVALID_VALUE ) { return point._deepCache; }
    //int16_t deep=3*point.x-3*point.y+3*point.z;
    int16_t deep=2*point.x-2*point.y+2*point.z; // isometric deep :D
    point._deepCache=deep;
    //int16_t deep = (point.x+point.z)/2;
    return deep;
}

bool View3D::IsClockwise(INOUT Face3D &face,uint16_t meshNumber) {
    if ( ENGINE_INVALID_VALUE != face._clockWiseCache ) {
        return (face._clockWiseCache?true:false);
    }
    Vertex3D & p0 = mesh[meshNumber]->vertexCache[face.vertex[0]];
    Vertex3D & p1 = mesh[meshNumber]->vertexCache[face.vertex[1]];
    Vertex3D & p2 = mesh[meshNumber]->vertexCache[face.vertex[2]];
    Point2D pix0;
    Point2D pix1;
    Point2D pix2;
    GetProjection(p0,pix0);
    GetProjection(p1,pix1);
    GetProjection(p2,pix2);

    int val = (pix1.y - pix0.y) * (pix2.x - pix1.x)
              - (pix1.x - pix0.x) * (pix2.y - pix1.y);
    //if (val == 0) { face._clockWiseCacheValue=false; } // collinear
    if (val > 0) {
        face._clockWiseCache=CLOCKWISE;
    } else { 
        face._clockWiseCache=COUNTER_CLOCKWISE;
    }
    return (face._clockWiseCache?true:false);
}
/*
//@TODO this must be thinked a little bit more
void _View3DThreadTask(void *data) {
    View3D *self=(View3D*)data;
    lLog("View3D render task begin\n");
    unsigned long announceThread=0;
    while(self->renderLoop ) {
        self->Render();
        if ( millis() > announceThread ) {
            announceThread=millis()+3000;
            lLog("View3D %p Render Task alive\n",self);
        }
    }
    lLog("View3D render task ended\n");
    vTaskDelete(NULL);
}   

void View3D::ThreadedRender() {
    //@TODO lauch ::Render() in a thread
    xTaskCreatePinnedToCore(_View3DThreadTask, "Rendr3D", LUNOKIOT_APP_STACK_SIZE, this, tskIDLE_PRIORITY, &this->renderTask,1);
}
// <======================================
*/
void View3D::DrawBilboards() {    
    for(uint16_t m=0;m<MAX_MESHES;m++) { // iterate meshes
        Mesh3D * currentMesh = mesh[m];
        if ( nullptr == currentMesh ) { continue; }
        if ( nullptr == currentMesh->bilboard ) { continue; } // is bilboarded one?
        // get the vertex coords
        Range dimensions = MeshDimensions;
        int16_t realRange = (dimensions.Max-dimensions.Min);
        for(uint16_t i=0;i< currentMesh->meshVertexCount;i++) {
            Vertex3D currVertex = currentMesh->vertexCache[i];

            int16_t deep = GetDeepForPoint(currVertex);
            int16_t realDeep = deep-dimensions.Min;
            float scaleValue = float(realDeep)/float(realRange); // scale from zbuffer

            /* bad idea, so small or soo huge bilboars dont pay
            // scale relative to mesh
            Vertex3D meshScale = currentMesh->MeshScale;
            float meshScaleMed=(meshScale.x+meshScale.y+meshScale.z)/3;
            scaleValue=meshScaleMed*scaleValue; // increment bilboard size with mesh scale
            */

            scaleValue=scaleValue*currentMesh->bilboardMultiplier; // increment with multiplier
            
            if ( scaleValue < 0.2) { continue; } // don't render at all: is too small to pay the effort

            Point2D pix;
            GetProjection(currVertex,pix);
            int32_t controlX = (centerX+pix.x);
            int32_t controlY = (centerY+pix.y);
            // take care of always show, including when out of view with margins
            int32_t bilboardMargin=(((currentMesh->bilboard->width()+currentMesh->bilboard->height())/2)*scaleValue)/2;

            if ( controlX < bilboardMargin*-1 ) { continue; }
            else if ( controlY < bilboardMargin*-1 ) { continue; }
            else if ( controlX > canvas->width()+bilboardMargin ) { continue; }
            else if ( controlY > canvas->height()+bilboardMargin ) { continue; }


            TFT_eSprite * scaledBillboard = ScaleSprite(currentMesh->bilboard,scaleValue);
            /* @TODO rotate bilboard from mesh rotation
            Angle3D currentRotation = currentMesh->MeshRotation;
            int16_t minX,minY,maxX,maxY;
            scaledBillboard->getRotatedBounds(currentRotation.y,&minX,&minY,&maxX,&maxY);
            TFT_eSprite * scaledAndRotated= new TFT_eSprite(tft);
            scaledAndRotated->setColorDepth(16);
            scaledAndRotated->setSwapBytes(true);
            scaledAndRotated->createSprite(maxX,maxY);
            scaledBillboard->pushRotated(scaledAndRotated,(int16_t)currentRotation.y);
            scaledBillboard->deleteSprite();
            delete scaledBillboard;
            */
            TFT_eSprite * scaledAndRotated=scaledBillboard;
            //lLog("SCALE: %p -> %p %f\n",currentMesh->bilboard,scaledBillboard,scaleValue);
            int32_t fx = controlX-(scaledAndRotated->width()/2);
            int32_t fy = controlY-(scaledAndRotated->height()/2);

            int16_t colorWeight = 255*scaleValue;
            if ( colorWeight>255 ) { colorWeight=255; }

            for(int y=0;y<scaledAndRotated->height();y++) {
                for(int x=0;x<scaledAndRotated->width();x++) {
                    uint16_t color = ByteSwap(scaledAndRotated->readPixel(x,y));
                    if ( currentMesh->bilboardMaskColor == color ) { continue; }
                    uint16_t finalColor = color; // flat by default
                    if ( RENDER::FULL == RenderMode ) { finalColor = tft->alphaBlend(colorWeight,color,TFT_BLACK); }
                    if ( RENDER::MASK == RenderMode ) { finalColor= MaskColor; }
                    //@TODO make sensible to FLATWIREFRAME (show borders) and WIREFRAME (with colors)
                    canvas->drawPixel(fx+x,fy+y,finalColor);
                }
            }
            scaledAndRotated->deleteSprite();
            delete scaledAndRotated;
        }
    }
}

void View3D::Render() {
    if( xSemaphoreTake( renderMutex, LUNOKIOT_EVENT_TIME_TICKS) != pdTRUE )  { return; }
    unsigned long bRender=millis();
    //lUILog("Clean...\n");
    /* THIS IS UNNECESARy, THE LAST CACHE IS VALID 
    // destroy caches
    for(uint16_t m=0;m<MAX_MESHES;m++) {
        if ( nullptr == mesh[m] ) { continue; }
        Mesh3D * currentMesh=mesh[m];

        for(uint16_t i=0;i<currentMesh->meshVertexCount;i++) {
            currentMesh->vertexCache[i]._xCache = ENGINE_INVALID_VALUE;
            currentMesh[i]._yCache = ENGINE_INVALID_VALUE;
            currentMesh[i]._deepCache = ENGINE_INVALID_VALUE;
        }
        for(uint16_t i=0;i<currentMesh->meshFaceCount[m];i++) {
            meshFaceData[m][i]._clockWiseCache=(int8_t)ENGINE_INVALID_VALUE;
        }
        for(uint16_t i=0;i<meshFaceCount[m];i++) {
            meshNormalsData[m][i]._deepCache=ENGINE_INVALID_VALUE;
        }
    }
    */

    // clear the mesh size
    MeshDimensions.Max=0;
    MeshDimensions.Min=1;
    ViewClipping.xMin=width;
    ViewClipping.xMax=0;
    ViewClipping.yMin=height;
    ViewClipping.yMax=0;

    esp_task_wdt_reset();

    // get mesh dimensions and ranges
    for(uint16_t m=0;m<MAX_MESHES;m++) {
        Mesh3D * currentMesh=mesh[m];
        if ( nullptr == currentMesh ) { continue; }
        for(uint16_t i=0;i<currentMesh->meshVertexCount;i++) {
            Point2D pix;
            GetProjection(currentMesh->vertexCache[i],pix);
            UpdateClipWith(ViewClipping,pix);
            int32_t deep = GetDeepForPoint(currentMesh->vertexCache[i]);
            UpdateRange(MeshDimensions,deep);
        }
    }
    //lUILog("Ordering...\n");
    // painter algorythm
    const int32_t MaxDeep=MeshDimensions.Max; //+abs(MeshDimensions.Min);
    int32_t currentDepth=MeshDimensions.Min;
    size_t orderdFaces=0;
    while ( currentDepth < MaxDeep ) {
        for(uint16_t m=0;m<MAX_MESHES;m++) {
            Mesh3D * currentMesh=mesh[m];
            if ( nullptr == currentMesh ) { continue; }
            //if ( nullptr != currentMesh->bilboard ) { continue; } // ignore bilboarded meshes

            for(uint16_t i=0;i<currentMesh->meshFaceCount;i++) {
                Face3D & face = currentMesh->faceCache[i];
                Vertex3D & p0 = currentMesh->vertexCache[face.vertex[0]];
                Vertex3D & p1 = currentMesh->vertexCache[face.vertex[1]];
                Vertex3D & p2 = currentMesh->vertexCache[face.vertex[2]];

                // get the 3 involved vertex deep to get planar deepth
                int16_t deep0 = GetDeepForPoint(p0);
                int16_t deep1 = GetDeepForPoint(p1);
                int16_t deep2 = GetDeepForPoint(p2);
                int32_t medianDeep = (deep0+deep1+deep2)/3;
                if ( currentDepth != medianDeep ) { continue; } // is face in this deep? (painter algorythm, back to forth)

                // backface culling
                //if ( false == IsClockwise(pix0,pix1,pix2) ) { continue; } // backface-culing
                if ( CLOCKWISE != IsClockwise(face,m) ) { continue; } // backface-culing
                                
                int16_t alpha=(255*(float(medianDeep+abs(MeshDimensions.Min))/float(MaxDeep+abs(MeshDimensions.Min))));
                if ( alpha > 0 ) {
                    Point2D pix0;
                    Point2D pix1;
                    Point2D pix2;
                    GetProjection(p0,pix0);
                    GetProjection(p1,pix1);
                    GetProjection(p2,pix2);

                    uint32_t tp0x = centerX+pix0.x;
                    uint32_t tp0y = centerY+pix0.y;
                    uint32_t tp1x = centerX+pix1.x;
                    uint32_t tp1y = centerY+pix1.y;
                    uint32_t tp2x = centerX+pix2.x;
                    uint32_t tp2y = centerY+pix2.y;

                    bool isVisible=false;
                    if ( ( tp0x > 0 ) && ( tp0x < width ) ) { isVisible = true; }
                    else if ( ( tp0y > 0 ) && ( tp0y < height ) ) { isVisible = true; }
                    else if ( ( tp1x > 0 ) && ( tp1x < width ) ) { isVisible = true; }
                    else if ( ( tp1y > 0 ) && ( tp1y < height ) ) { isVisible = true; }
                    else if ( ( tp2x > 0 ) && ( tp2x < width ) ) { isVisible = true; }
                    else if ( ( tp2y > 0 ) && ( tp2y < height ) ) { isVisible = true; }
                    if ( false == isVisible ) { continue; }
            
                    const uint16_t zcolor = tft->color565(alpha,alpha,alpha);
                    float distNormal = NormalFacing(currentMesh->normalCache[i]);
                    // discard from normals if no FULL (no backface normals needed)
                    if ( ( distNormal < 0 ) && ( RENDER::FULL == RenderMode ) ) { continue; }
                    //if ( ( distNormal < 0 ) ) { continue; }
                    meshOrderedFaces[orderdFaces].normalFacing = distNormal;
                    meshOrderedFaces[orderdFaces].faceOffset = i;
                    meshOrderedFaces[orderdFaces].p0x=tp0x;
                    meshOrderedFaces[orderdFaces].p0y=tp0y;
                    meshOrderedFaces[orderdFaces].p1x=tp1x;
                    meshOrderedFaces[orderdFaces].p1y=tp1y;
                    meshOrderedFaces[orderdFaces].p2x=tp2x;
                    meshOrderedFaces[orderdFaces].p2y=tp2y;
                    meshOrderedFaces[orderdFaces].colorDeep=zcolor;
                    meshOrderedFaces[orderdFaces].alpha=alpha;
                    meshOrderedFaces[orderdFaces].faceColor=face.color;
                    meshOrderedFaces[orderdFaces].smooth=face.smooth;
                    meshOrderedFaces[orderdFaces].fromMesh=m; // save the mesh offset
                    orderdFaces++;
                    if ( orderdFaces >= MAX_ORDERED_FACES ) {
                        lUILog("View3D %p MAX_ORDERED_FACES\n");
                        goto noMoreFacesPlease;
                    }
                }
            }
        }
        currentDepth++;
    }
    noMoreFacesPlease:

    // render here!
    //lUILog("Rendering...\n");
    
    // fill with background color or do beforerender callback?
    if ( nullptr == beforeRenderCallback ) { canvas->fillSprite(viewBackgroundColor); }
    else { (beforeRenderCallback)(beforeRenderCallbackParam,canvas); }

    for(int i=0;i<orderdFaces;i++) {
        // don't render faces from bilboards
        if ( mesh[meshOrderedFaces[i].fromMesh]->bilboard ) { continue; }
        if ( RENDER::NODRAW == RenderMode ) { continue; }

        int32_t tp0x = meshOrderedFaces[i].p0x;
        int32_t tp0y = meshOrderedFaces[i].p0y;
        int32_t tp1x = meshOrderedFaces[i].p1x;
        int32_t tp1y = meshOrderedFaces[i].p1y;
        int32_t tp2x = meshOrderedFaces[i].p2x;
        int32_t tp2y = meshOrderedFaces[i].p2y;

        if ( ( RENDER::FULL == RenderMode ) || ( RENDER::WIREFRAME == RenderMode ) ) {
            uint16_t finalColor = meshOrderedFaces[i].faceColor;
            bool smooth =  meshOrderedFaces[i].smooth;
            float dist = meshOrderedFaces[i].normalFacing;
            uint8_t mixProp=128;
            if ( smooth ) { mixProp=64; }
            if ( dist>0 ) { // normal faces view
                uint8_t baseCol=(128*dist);
                uint16_t nColor = tft->color565(baseCol,baseCol,baseCol);
                finalColor = tft->alphaBlend(mixProp,nColor,finalColor);
                if ( RENDER::FULL == RenderMode ) {
                    canvas->fillTriangle(tp0x,tp0y,tp1x,tp1y,tp2x,tp2y,finalColor);
                } else if ( RENDER::WIREFRAME == RenderMode ) {
                    canvas->drawTriangle(tp0x,tp0y,tp1x,tp1y,tp2x,tp2y,finalColor);
                }
            }
            /*} else { // normal is negated, must be darken or brighten? @:(
                finalColor = tft->alphaBlend(192,TFT_BLACK,finalColor);
            }*/

        } else if ( RENDER::FLAT == RenderMode ) {
            canvas->fillTriangle(tp0x,tp0y,tp1x,tp1y,tp2x,tp2y,meshOrderedFaces[i].faceColor);
        } else if ( RENDER::FLATWIREFRAME == RenderMode ) {
            canvas->drawTriangle(tp0x,tp0y,tp1x,tp1y,tp2x,tp2y,MaskColor);
        } else if ( RENDER::MASK == RenderMode ) {
            canvas->fillTriangle(tp0x,tp0y,tp1x,tp1y,tp2x,tp2y,MaskColor);
        } // else NODRAW
        if ( nullptr != polygonCallback ) { (polygonCallback)(polygonCallbackParam,canvas,&meshOrderedFaces[i]); }
    }
    DrawBilboards();
    if ( nullptr != renderCallback ) { (renderCallback)(renderCallbackParam,canvas); }
    unsigned long tRender=millis()-bRender;
    uint16_t fps = 1;
    if ( tRender > 0 ) { fps = 1000/tRender; }
    //lUILog("Render %p complete fps: %u ms: %lu faces: %u iteration: %u\n",this,fps,tRender,orderdFaces,renderCount);
    renderCount++;
    xSemaphoreGive( renderMutex );
}

View3D::~View3D() {
    lLog("View3D %p destroyed!\n",this);
    //renderLoop = false;
    if ( nullptr != meshOrderedFaces ) { free(meshOrderedFaces); }
    for(uint16_t m=0;m<MAX_MESHES;m++) {
        if ( nullptr == mesh[m] ) { continue; }
        delete mesh[m];
    }
}
View3D::View3D() { //}: Control() {
    lLog("View3D %p created!\n",this);
    meshOrderedFaces=(OrderedFace3D *)ps_calloc(MAX_ORDERED_FACES,sizeof(OrderedFace3D));
}


TFT_eSprite * View3D::GetCanvas() {
    //lLog("View3D %p GetCanvas()\n",this);
    if (directDraw) { return canvas; }
    return nullptr; // deny render without DirectDraw
}

    // @TODO ugly code
    /*
    if ( false == firstPush ) {
        firstPush=true;
        Render();
        return canvas;

    }*/
    // dont push image until directDraw is enabled
    /*
    if (false == directDraw) { // dont show nothing until directDraw available
        //canvas->fillSprite(viewBackgroundColor);
        canvas->fillSprite(TFT_BLACK);
        return canvas;
    }*/
    /*
    unsigned long bDirect=millis();
    // Children dirty?
    bool needRedraw=dirty;
    for(uint16_t m=0;m<MAX_MESHES;m++) {
        if ( nullptr == mesh[m]) { continue; }
        if ( mesh[m]->dirty ) {
            needRedraw=true;
            break;
        }
    }
    if ( needRedraw ) { Render(); }
    return canvas; // @TODO seems direct push is more efficient o_O'?
}*/
    /**
    // bugs with multi-view
    const uint8_t border = 8;
    // lets push a piece of myself to tft to get faster refresh
    int32_t bx0 = int(centerX+ViewClipping.xMin)+clipX;
    int32_t by0 = int(centerY+ViewClipping.yMin)+clipY;
    int32_t bx1 = int(centerX+ViewClipping.xMax)+clipX;
    int32_t by1 = int(centerY+ViewClipping.yMax)+clipY;
    // correct the points (@TODO can be improved with MIN/MAX)
    if ( bx0 < int(clipX+1) ) { bx0=clipX+1; }
    else if ( bx0 > int(clipX+width-1) ) { bx0=clipX+width-1; }
    if ( bx1 < int(clipX+1) ) { bx1=clipX+1; }
    else if ( bx1 > int(clipX+width-1) ) { bx1=clipX+width-1; }
    if ( by0 < int(clipY+1) ) { by0=clipY+1; }
    else if ( by0 > int(clipY+height) ) { by0=clipY+height-1; }
    if ( by1 < int(clipY+1) ) { by1=clipY+1; }
    else if ( by1 > int(clipY+height) ) { by1=clipY+height-1; }
    
    //lLog("BX0: %d BY0: %d BX1: %d BY1: %d\n", bx0, by0, bx1, by1);
    int32_t rw = (bx1-bx0);
    int32_t rh = (by1-by0);

    //@TODO sprite can be replaced with tft->setWindow and dump pixel data?
    TFT_eSprite finalRender=TFT_eSprite(tft);
    finalRender.setColorDepth(16);
    finalRender.createSprite(rw,rh);

    for (uint32_t y=by0;y<by1;y++) {
        if ( canvas->height() <= y ) { break; }
        for (uint32_t x=bx0;x<bx1;x++) {
            if ( canvas->width() <= x ) { break; }
            uint32_t color = canvas->readPixel(x,y);
            //if ( TFT_BLACK == color ) { color = TFT_PINK; }
            finalRender.drawPixel((x-bx0),(y-by0),color);
        }
    }
    finalRender.pushSprite(bx0,by0);
    finalRender.deleteSprite();

    //tft->drawRect(bx0,by0,rw,rh,TFT_WHITE);

    // fill rest of view
    // up
    tft->fillRect(clipX,clipY,width,by0-clipY,TFT_BLACK);
    // left
    tft->fillRect(clipX,by0,bx0-clipX,rh,TFT_BLACK);
    // down
    tft->fillRect(clipX,by1,width,(height-by1)+clipY,TFT_BLACK);
    // right
    tft->fillRect(bx1,by0,(width-bx1)+clipX,rh,TFT_BLACK);
    unsigned long tDirect=millis()-bDirect;
    uint16_t fps = 1000/tDirect;
    lUILog("Direct complete ms: %lu fps: %u\n", tDirect,fps);
    return nullptr;
    */



/*
// https://computergraphics.stackexchange.com/questions/1866/how-to-map-square-texture-to-triangle
// https://gamedev.stackexchange.com/questions/23743/whats-the-most-efficient-way-to-find-barycentric-coordinates
// Compute barycentric coordinates (u, v, w) for
// point p with respect to triangle (a, b, c)
void Mesh3D::Barycentric(Point p, Point a, Point b, Point c, float &u, float &v, float &w)
{
    Vector v0 = b - a, v1 = c - a, v2 = p - a;
    float d00 = Dot(v0, v0);
    float d01 = Dot(v0, v1);
    float d11 = Dot(v1, v1);
    float d20 = Dot(v2, v0);
    float d21 = Dot(v2, v1);
    float denom = d00 * d11 - d01 * d01;
    v = (d11 * d20 - d01 * d21) / denom;
    w = (d00 * d21 - d01 * d20) / denom;
    u = 1.0f - v - w;
}
*/
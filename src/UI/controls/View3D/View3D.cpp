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
/*
bool View3D::CacheIsFilled=false;
float View3D::AngleSinCache[360];
float View3D::AngleCosCache[360];
float View3D::AngleCache[360];
const double View3D::sq2 = sqrt(2);
const double View3D::sq6 = sqrt(6);
*/

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
        lUILog("View3D %p View FULL\n",this);
        return;
    }
    mesh[offset] = meshObject;
    dirty=true;
}

float View3D::OriginalNormalFacing(INOUT Normal3D &normal) {
    //AAAAAAA
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


void View3D::Refresh(bool direct,bool swap) {
    //lLog("View3D %p refresh canvas at %p swap: %s\n",this,canvas,(swap?"true":"false"));
    if ( nullptr == canvas  ) { Control::Refresh(direct,swap); }
    //if ( dirty ) { Control::Refresh(swap); }
    centerX = width/2;
    centerY = height/2;
    // iterate all meshes to apply their last transformations
    int offset=0;
    for(offset=0;offset<MAX_MESHES;offset++) {
        if ( nullptr == mesh[offset] ) { continue; }
        mesh[offset]->ApplyTransform();
    }
    if ( nullptr != stepCallback ) { (stepCallback)(stepCallbackParam); }
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

void View3D::Render() {
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
        if ( nullptr == mesh[m] ) { continue; }
        Mesh3D * currentMesh=mesh[m];
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
            if ( nullptr == mesh[m] ) { continue; }
            Mesh3D * currentMesh=mesh[m];
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
                    if ( ( distNormal < 0 ) && ( RENDER::FULL != RenderMode ) ) { continue; }
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
    canvas->fillSprite(viewBackgroundColor);
    for(int i=0;i<orderdFaces;i++) {
        int32_t tp0x = meshOrderedFaces[i].p0x;
        int32_t tp0y = meshOrderedFaces[i].p0y;
        int32_t tp1x = meshOrderedFaces[i].p1x;
        int32_t tp1y = meshOrderedFaces[i].p1y;
        int32_t tp2x = meshOrderedFaces[i].p2x;
        int32_t tp2y = meshOrderedFaces[i].p2y;

        uint16_t finalColor = meshOrderedFaces[i].faceColor;
        bool smooth =  meshOrderedFaces[i].smooth;
        float dist = meshOrderedFaces[i].normalFacing;
        // discard if no lights involved (already on painter algorythm)
        //if ( ( RENDER::FULL != RenderMode ) && ( dist <= 0 )) { continue; }
        uint8_t mixProp=128;
        if ( smooth ) { mixProp=64; }
        if ( dist>0 ) { // normal faces view
            uint8_t baseCol=(128*dist);
            uint16_t nColor = tft->color565(baseCol,baseCol,baseCol);
            finalColor = tft->alphaBlend(mixProp,nColor,finalColor);
        } else { // normal is negated, must be darken
            finalColor = tft->alphaBlend(192,TFT_BLACK,finalColor);
        }

        if ( RENDER::FULL == RenderMode ) {
            canvas->fillTriangle(tp0x,tp0y,tp1x,tp1y,tp2x,tp2y,finalColor);
        } else if ( RENDER::FLAT == RenderMode ) {
            canvas->fillTriangle(tp0x,tp0y,tp1x,tp1y,tp2x,tp2y,meshOrderedFaces[i].faceColor);
        } else if ( RENDER::WIREFRAME == RenderMode ) {
            canvas->drawTriangle(tp0x,tp0y,tp1x,tp1y,tp2x,tp2y,finalColor);
        } else if ( RENDER::FLATWIREFRAME == RenderMode ) {
            canvas->drawTriangle(tp0x,tp0y,tp1x,tp1y,tp2x,tp2y,TFT_WHITE);
        } else if ( RENDER::MASK == RenderMode ) {
            canvas->fillTriangle(tp0x,tp0y,tp1x,tp1y,tp2x,tp2y,TFT_WHITE);
        }
    }
    unsigned long tRender=millis()-bRender;
    uint16_t fps = 1000/tRender;
    //lUILog("Render %p complete fps: %u ms: %lu faces: %u iteration: %u\n",this,fps,tRender,orderdFaces,renderCount);
    renderCount++;
}

View3D::~View3D() {
    lLog("View3D %p destroyed!\n",this);
    if ( nullptr != meshOrderedFaces ) { free(meshOrderedFaces); }
    for(uint16_t m=0;m<MAX_MESHES;m++) {
        if ( nullptr == mesh[m] ) { continue; }
        delete mesh[m];
    }
}
View3D::View3D(): Control() {
    lLog("View3D %p created!\n",this);
    meshOrderedFaces=(OrderedFace3D *)ps_calloc(MAX_ORDERED_FACES,sizeof(OrderedFace3D));
}


TFT_eSprite * View3D::GetCanvas() {
    // dont push image until directDraw is enabled
    /*
    if (false == directDraw) { // dont show nothing until directDraw available
        //canvas->fillSprite(viewBackgroundColor);
        canvas->fillSprite(TFT_BLACK);
        return canvas;
    }*/
    unsigned long bDirect=millis();
    // Children dirty?
    bool needRedraw=false;
    for(uint16_t m=0;m<MAX_MESHES;m++) {
        if ( nullptr == mesh[m]) { continue; }
        if ( mesh[m]->dirty ) {
            needRedraw=true;
            break;
        }
    }
    if ( needRedraw ) {
        Render();
    }
    return canvas; // @TODO seems direct push is more efficient o_O'?
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
}

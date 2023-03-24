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
}

float View3D::NormalFacing(INOUT Normal3D &normal) {
    if ( ENGINE_INVALID_VALUE != normal._deepCache ) { return normal._deepCache; }
    normal._deepCache=normal.vertexData[0]-normal.vertexData[1]+normal.vertexData[2];  
    //if(deep<0){ return true; }
    return normal._deepCache;
}


void View3D::Refresh(bool swap) {
    //lLog("View3D %p refresh\n",this);
    Control::Refresh(swap);
    centerX = width/2;
    centerY = height/2;

    // iterate all meshes to apply their last transformations
    int offset=0;
    for(offset=0;offset<MAX_MESHES;offset++) {
        if ( nullptr == mesh[offset] ) { continue; }
        mesh[offset]->ApplyTransform();
    }
    if ( directDraw ) { Render(); }
    if ( stepEnabled ) {
        if ( nullptr != stepCallback ) { (stepCallback)(stepCallbackParam); }
    }

}

void View3D::UpdateClipWith(INOUT Clipping2D &clip, IN Point2D &point) {
    UpdateClipWith(clip, point.x,point.y);
}

void View3D::UpdateClipWith(INOUT Clipping2D &clip, IN int16_t x,IN int16_t y) {
    clip.xMin=MIN(clip.xMin,x);
    clip.yMin=MIN(clip.yMin,y);
    clip.xMax=MAX(clip.xMax,x);
    clip.yMax=MAX(clip.yMax,y);
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
    return (face._clockWiseCache?CLOCKWISE:COUNTER_CLOCKWISE);
}

void View3D::Render() {
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
                int16_t alpha=(255*(float(medianDeep+abs(MeshDimensions.Min))/float(MaxDeep+abs(MeshDimensions.Min))));
                if ( alpha > 0 ) {
                    const uint16_t zcolor = tft->color565(alpha,alpha,alpha);
                    // fill the cache
                    //float distNormal = NormalFacing(meshNormalsData[m][i]);
                    float distNormal = NormalFacing(currentMesh->normalCache[i]);
                    
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
                }

            }
        }
        currentDepth++;
    }

    // render here!
    //lUILog("Rendering...\n");
    canvas->fillSprite(TFT_BLACK);
    for(int i=0;i<orderdFaces;i++) {
        int32_t tp0x = meshOrderedFaces[i].p0x;
        int32_t tp0y = meshOrderedFaces[i].p0y;
        int32_t tp1x = meshOrderedFaces[i].p1x;
        int32_t tp1y = meshOrderedFaces[i].p1y;
        int32_t tp2x = meshOrderedFaces[i].p2x;
        int32_t tp2y = meshOrderedFaces[i].p2y;

        //@FACE
        uint16_t finalColor = meshOrderedFaces[i].faceColor;
        bool smooth =  meshOrderedFaces[i].smooth;

        uint8_t mixProp=128;
        //uint16_t deepColor = tft->alphaBlend(128,mixProp,meshOrderedFaces[i].colorDeep,finalColor);
        //finalColor = tft->alphaBlend(mixProp,deepColor,finalColor);
        if ( smooth ) {
            mixProp=32;
        }
        float dist = meshOrderedFaces[i].normalFacing; // NormalFacing(normal); //fabs(NormalFacing(normal));
        if ( dist>0 ) {
            uint8_t baseCol=(128*dist);
            uint16_t nColor = tft->color565(baseCol,baseCol,baseCol);
            finalColor = tft->alphaBlend(mixProp,nColor,finalColor);
        }
        canvas->fillTriangle(tp0x,tp0y,tp1x,tp1y,tp2x,tp2y,finalColor);
    }
    //lUILog("Render complete!\n");

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
/*
int16_t View3D::AddModel(IN Mesh3DDescriptor * meshData) {
    if ( meshCount >= MAX_MESHES ) {
        lUILog("View FULL!!\n");
        return INVALID_MESH;
    }
    lUILog("Adding mesh from %p...\n",meshData);
    //meshVertexData=(Vertex3D *)calloc(meshData->vetexNum,sizeof(Vertex3D));
    meshVertexData[meshCount]=(Vertex3D *)ps_calloc(meshData->vetexNum,sizeof(Vertex3D));
    if ( nullptr == meshVertexData) { 
        lUILog("Unable to allocate Vertex Data\n");
        return INVALID_MESH;
    }
    //meshFaceData=(Face3D *)calloc(meshData->faceNum,sizeof(Face3D));
    meshFaceData[meshCount]=(Face3D *)ps_calloc(meshData->faceNum,sizeof(Face3D));
    if ( nullptr == meshFaceData) {
        lUILog("Unable to allocate Face Data\n");
        return INVALID_MESH;
    }
    //meshNormalsData=(Normal3D *)calloc(meshData->faceNum,sizeof(Normal3D));
    meshNormalsData[meshCount]=(Normal3D *)ps_calloc(meshData->faceNum,sizeof(Normal3D));
    if ( nullptr == meshNormalsData) {
        lUILog("Unable to allocate Normal Data\n");
        return INVALID_MESH;
    }

    // build vertex
    for(uint16_t vc=0;vc<meshData->vetexNum;vc++) {
        const RawVertex3D vertex = meshData->vertex[vc];
        meshVertexData[meshCount][vc]= { 
            vertex.x,
            vertex.y,
            vertex.z,
            TFT_WHITE,
            ENGINE_INVALID_VALUE,
            ENGINE_INVALID_VALUE,
            ENGINE_INVALID_VALUE
        };
    }
    //build faces
    for(uint16_t fc=0;fc<meshData->faceNum;fc++) {
        const RawFace3D face = meshData->face[fc];
        meshFaceData[meshCount][fc]= {
            face.color,
            { face.vertex[0],face.vertex[1],face.vertex[2] },
            face.smooth,
            (int8_t)ENGINE_INVALID_VALUE
        };
    }
    //build normals
    for(uint16_t nc=0;nc<meshData->faceNum;nc++) {
        const RawNormal3D normal = meshData->normal[nc];
        meshNormalsData[meshCount][nc]= {
            { normal.vertexData[0],normal.vertexData[1],normal.vertexData[2] },
            ENGINE_INVALID_VALUE
        };
    }
    
    // don't set until the end
    meshVertexCount[meshCount]=meshData->vetexNum;
    meshFaceCount[meshCount]=meshData->faceNum;
    lUILog("Mesh %u loaded\n",meshCount);
    dirty=true; // force first refresh
    meshCount++;
    return meshCount-1;
}

void View3D::Rotate(INOUT Normal3D & normal, IN Angle3D & rotationAngles) {
    float tempN;
    if ( 0.0 != rotationAngles.x ) {
        int angleOffset=(int)rotationAngles.x;
        if ( angleOffset < 0 ) { angleOffset+=360; }
        float anglex=AngleCache[angleOffset];
        //float anglex = (rotationAngles.x * M_PI) / 180.0;
        tempN = normal.vertexData[1];
        //point.y = point.y * cos(anglex) - point.z * sin(anglex);
        //point.z = temp * sin(anglex) + point.z * cos(anglex);
        normal.vertexData[1] = normal.vertexData[1] * AngleCosCache[angleOffset] - normal.vertexData[2] * AngleSinCache[angleOffset];
        normal.vertexData[2] = tempN * AngleSinCache[angleOffset] + normal.vertexData[2] * AngleCosCache[angleOffset];
    }
    if ( 0.0 != rotationAngles.y ) {
        int angleOffset=(int)rotationAngles.y;
        if ( angleOffset < 0 ) { angleOffset+=360; }
        float angley=AngleCache[angleOffset];
        //float angley = (rotationAngles.y * M_PI) / 180.0;
        tempN = normal.vertexData[2];
        
        normal.vertexData[2] = normal.vertexData[2] * AngleCosCache[angleOffset] - normal.vertexData[0] * AngleSinCache[angleOffset];
        normal.vertexData[0] = tempN * AngleSinCache[angleOffset] + normal.vertexData[0] * AngleCosCache[angleOffset];
        //point.z = point.z * coscY - point.x * sincY;
        //point.x = temp * sincY + point.x * coscY;
    }
    if ( 0.0 != rotationAngles.z ) {
        int angleOffset=(int)rotationAngles.z;
        if ( angleOffset < 0 ) { angleOffset+=360; }
        float anglez=AngleCache[angleOffset];
        //float anglez = (rotationAngles.z * M_PI) / 180.0;
        tempN = normal.vertexData[0];
        //point.x = point.x * cos(anglez) - point.y * sin(anglez);
        //point.y = temp * sin(anglez) + point.y *cos(anglez);
        normal.vertexData[0] = normal.vertexData[0] * AngleCosCache[angleOffset] - normal.vertexData[1] * AngleSinCache[angleOffset];
        normal.vertexData[1] = tempN * AngleSinCache[angleOffset] + normal.vertexData[1] *AngleCosCache[angleOffset];
    }
}

void View3D::Rotate(INOUT Vertex3D & point, IN Angle3D & rotationAngles) {
    float temp;
    if ( 0.0 != rotationAngles.x ) {
        int angleOffset=(int)rotationAngles.x;
        if ( angleOffset < 0 ) { angleOffset+=360; }
        float anglex=AngleCache[angleOffset];
        //float anglex = (rotationAngles.x * M_PI) / 180.0;
        temp = point.y;
        //point.y = point.y * cos(anglex) - point.z * sin(anglex);
        //point.z = temp * sin(anglex) + point.z * cos(anglex);
        point.y = point.y * AngleCosCache[angleOffset] - point.z * AngleSinCache[angleOffset];
        point.z = temp * AngleSinCache[angleOffset] + point.z * AngleCosCache[angleOffset];
    }
    if ( 0.0 != rotationAngles.y ) {
        int angleOffset=(int)rotationAngles.y;
        if ( angleOffset < 0 ) { angleOffset+=360; }
        float angley=AngleCache[angleOffset];
        //float angley = (rotationAngles.y * M_PI) / 180.0;
        temp = point.z;
        
        point.z = point.z * AngleCosCache[angleOffset] - point.x * AngleSinCache[angleOffset];
        point.x = temp * AngleSinCache[angleOffset] + point.x * AngleCosCache[angleOffset];
        //point.z = point.z * coscY - point.x * sincY;
        //point.x = temp * sincY + point.x * coscY;
    }
    if ( 0.0 != rotationAngles.z ) {
        int angleOffset=(int)rotationAngles.z;
        if ( angleOffset < 0 ) { angleOffset+=360; }
        float anglez=AngleCache[angleOffset];
        //float anglez = (rotationAngles.z * M_PI) / 180.0;
        temp = point.x;
        //point.x = point.x * cos(anglez) - point.y * sin(anglez);
        //point.y = temp * sin(anglez) + point.y *cos(anglez);
        point.x = point.x * AngleCosCache[angleOffset] - point.y * AngleSinCache[angleOffset];
        point.y = temp * AngleSinCache[angleOffset] + point.y *AngleCosCache[angleOffset];
    }


    //https://stackoverflow.com/questions/2096474/given-a-surface-normal-find-rotation-for-3d-plane

}


TFT_eSprite * View3D::GetCanvas() {
    // dont push image until directDraw is enabled
    if (false == directDraw) {
        canvas->fillSprite(TFT_BLACK);
        return canvas;
    }
    return canvas; // @TODO TEST
    // lets push a piece of myself to tft to get faster refresh
    uint8_t border = 8;
    uint32_t bx = (centerX+ViewClipping.xMin);
    uint32_t by = (centerY+ViewClipping.yMin);
    uint32_t bw = (centerX+ViewClipping.xMax);
    uint32_t bh = (centerY+ViewClipping.yMax);

    uint32_t rw = (bw-bx)+(border*2);
    uint32_t rh = (bh-by)+(border*2);
    if (( rw >= width-border )||( rh >= height-border )) { // bigger buffer than canvas? Push via parent
        return canvas;
    } // if clip is bigger, send as usual
    //unsigned long bpushTime=millis();
    uint32_t sx = (clipX+bx)-border;
    uint32_t sy = (clipY+by)-border;
    if (( sx <= clipX )||( sy <= clipY )) { // bigger buffer than canvas? Push via parent
        return canvas;
    } // if clip is bigger, send as usual
    //tft->drawRect(sx,sy,rw,rh,TFT_GREEN);
    //lUILog("CLIP: X: %u Y: %u W: %u H: %u Control: X: %u Y: %u\n",bx,by,bw,bh,sx,sy);
    TFT_eSprite finalRender=TFT_eSprite(tft);
    finalRender.setColorDepth(16);

    finalRender.createSprite(rw,rh);
    for (uint32_t y=border;y<rh;y++) {
        for (uint32_t x=border;x<rw;x++) {
            int32_t rpx=bx+(x-border);
            if ( rpx < 0 ) { continue; }
            else if ( rpx >= width ) { continue; }
            int32_t rpy=by+(y-border);
            if ( rpy < 0 ) { continue; }
            else if ( rpy >= height ) { continue; }
            uint32_t color = canvas->readPixel(rpx,rpy);
            finalRender.drawPixel(x,y,color);
        }
    }
    finalRender.pushSprite(sx,sy);
    finalRender.deleteSprite();
    //unsigned long pushTime=millis()-bpushTime;
    //uint16_t fps=1000/pushTime;
    //lLog("Render %p time: %lu ms FPS: %u\n",this,pushTime,fps);
    //dirty=true; // eternal dirty
    return nullptr; // don't allow parent to push me
}



void View3D::Scale(INOUT Vertex3D & point, IN Vertex3D & axes) {
    point.x *= axes.x;
    point.y *= axes.y;
    point.z *= axes.z;
}


void View3D::Rotate(IN Angle3D &rotationAngles,int16_t meshId,bool save) {
    if ( nullptr == meshVertexData[meshId] ) { return; }
    if ( nullptr == meshNormalsData[meshId] ) { return; }
    
    if ( save ) {
        lLog("%d After rotation: X: %f  Y: %f Z: %f\n",meshId,RotationMesh[meshId].x,RotationMesh[meshId].y,RotationMesh[meshId].z);
        RotationMesh[meshId].x+=rotationAngles.x;
        RotationMesh[meshId].y+=rotationAngles.y;
        RotationMesh[meshId].z+=rotationAngles.z;
        if ( RotationMesh[meshId].x > 360.0 ) { RotationMesh[meshId].x-=360.0; }
        else if ( RotationMesh[meshId].x < -360.0 ) { RotationMesh[meshId].x+=360.0; }
        if ( RotationMesh[meshId].y > 360.0 ) { RotationMesh[meshId].y-=360.0; }
        else if ( RotationMesh[meshId].y < -360.0 ) { RotationMesh[meshId].y+=360.0; }
        if ( RotationMesh[meshId].z > 360.0 ) { RotationMesh[meshId].z-=360.0; }
        else if ( RotationMesh[meshId].z < -360.0 ) { RotationMesh[meshId].z+=360.0; }
        lLog("%d Current rotation: X: %f  Y: %f Z: %f\n",meshId,RotationMesh[meshId].x,RotationMesh[meshId].y,RotationMesh[meshId].z);
    } else {
        lLog("%d Shift rotation to: X: %f  Y: %f Z: %f\n",meshId,rotationAngles.x,rotationAngles.y,rotationAngles.z);
    }

    // put meshes to center (for rotations)
    for(uint16_t m=0;m<MAX_MESHES;m++) {
        if ( nullptr == meshVertexData[m] ) { continue; }
        Vertex3D negated =LocationMesh[m];
        negated.x*=-1;
        negated.y*=-1;
        negated.z*=-1;
        Translate(negated,m,false);
        lLog("%u Shift mesh to 0,0,0\n",m);
    }
    // rotate vertex!
    for(uint16_t i=0;i<meshVertexCount[meshId];i++) {
        Rotate(meshVertexData[meshId][i],rotationAngles);
    }
    // rotate normals!
    for(uint16_t i=0;i<meshFaceCount[meshId];i++) {
        Rotate(meshNormalsData[meshId][i],rotationAngles);
    }

    // return mesh to their location for rendering
    for(uint16_t m=0;m<MAX_MESHES;m++) {
        if ( nullptr == meshVertexData[m] ) { continue; }
        Translate(LocationMesh[m],m,false);
        lLog("Return mesh %u to their location\n",m);
    }
}

void View3D::Rotate(IN Angle3D &rotationAngles, bool save) {

    //if ( ( 0.0 != rotationAngles.x ) || ( 0.0 != rotationAngles.y ) || ( 0.0 != rotationAngles.z )) {

        for(uint16_t m=0;m<MAX_MESHES;m++) {
            Rotate(rotationAngles,m,save);
        }
    //}
}

void View3D::Scale(IN Vertex3D & axes) {
    if ( ( 1.0 != axes.x ) || ( 1.0 != axes.y ) || ( 1.0 != axes.z )) {
        for(uint16_t m=0;m<MAX_MESHES;m++) {
            if ( nullptr == meshVertexData[m] ) { continue; }
            for(uint16_t i=0;i<meshVertexCount[m];i++) {
                Scale(meshVertexData[m][i],axes);
            }
        }
    }
}
void View3D::RotateStep() {
    if ( ( FinalRotationMesh.x == RotationMesh.x ) &&  ( FinalRotationMesh.y == RotationMesh.y )
                                                && ( FinalRotationMesh.z == RotationMesh.z ) ) {
        dirty=false;
        return;
    }
    Angle3D current;
    // James Bruton smooth https://hackaday.com/2021/09/03/smooth-servo-motion-for-lifelike-animatronics/
    current.x=(RotationMesh.x*0.8)+(FinalRotationMesh.x*0.2);
    current.y=(RotationMesh.y*0.8)+(FinalRotationMesh.y*0.2);
    current.z=(RotationMesh.z*0.8)+(FinalRotationMesh.z*0.2);
    Angle3D displacement;
    displacement.x =RotationMesh.x-current.x;
    displacement.y =RotationMesh.y-current.y;
    displacement.z =RotationMesh.z-current.z;
    if ( ( abs(displacement.x) < 0.4 ) && ( abs(displacement.y) < 0.4 ) && ( abs(displacement.z) < 0.4 ) ) {
        RotationMesh.x = FinalRotationMesh.x;
        RotationMesh.y = FinalRotationMesh.y;
        RotationMesh.z = FinalRotationMesh.z;
        //lLog("ROTATION REACHED\n");
        FinalRotationMesh = { float(random(0,359)), float(random(0,359)), float(random(0,359)) };
        dirty=true;
        return;
    }
    //lLog("ROTATION STEP: X: %f Y: %f Z: %f\n", displacement.x, displacement.y, displacement.z);
    Rotate(displacement);
    RotationMesh.x=current.x;
    RotationMesh.y=current.y;
    RotationMesh.z=current.z;
    dirty=true;
}

void View3D::ScaleStep() {
    if ( ( FinalScaleMesh.x == ScaleMesh.x ) &&  ( FinalScaleMesh.y == ScaleMesh.y ) && ( FinalScaleMesh.z == ScaleMesh.z ) ) {
        return;
    }
    Vertex3D current;
    // James Bruton smooth https://hackaday.com/2021/09/03/smooth-servo-motion-for-lifelike-animatronics/
    current.x=((ScaleMesh.x*0.8)+(FinalScaleMesh.x*0.2));
    current.y=((ScaleMesh.y*0.8)+(FinalScaleMesh.y*0.2));
    current.z=((ScaleMesh.z*0.8)+(FinalScaleMesh.z*0.2));
    if ( ( fabs(current.x) == fabs(ScaleMesh.x) )
       && ( fabs(current.y) == fabs(ScaleMesh.y) )
       && ( fabs(current.z) == fabs(ScaleMesh.z) ) ) {
        ScaleMesh.x = FinalScaleMesh.x;
        ScaleMesh.y = FinalScaleMesh.y;
        ScaleMesh.z = FinalScaleMesh.z;
        lLog("SCALE REACHED\n");
        return;
    }
    lLog("SCALE: X: %f Y: %f Z: %f\n", current.x, current.y, current.z);
    Scale(current);
    ScaleMesh.x=current.x;
    ScaleMesh.y=current.y;
    ScaleMesh.z=current.z;
}


void View3D::Translate(INOUT Vertex3D & point, IN Vertex3D & translate) { //, float tx, float ty, float tz){
    point.x += translate.x;
    point.y += translate.y;
    point.z += translate.z;
}

void View3D::Translate(IN Vertex3D & translate,int16_t meshNumber, bool save) {
    if ( nullptr == meshVertexData[meshNumber] ) { return; }
    
    if ( save ) {
        lLog("%d After position: X: %f  Y: %f Z: %f\n",meshNumber,LocationMesh[meshNumber].x,LocationMesh[meshNumber].y,LocationMesh[meshNumber].z);
        Translate(LocationMesh[meshNumber],translate); //do the add
        lLog("%d Current position: X: %f  Y: %f Z: %f\n",meshNumber,LocationMesh[meshNumber].x,LocationMesh[meshNumber].y,LocationMesh[meshNumber].z);
    } else {
        lLog("%d Shift position to: X: %f  Y: %f Z: %f\n",meshNumber,translate.x,translate.y,translate.z);
    }
    for(uint16_t i=0;i<meshVertexCount[meshNumber];i++) {
        Translate(meshVertexData[meshNumber][i],translate);
    }
}

void View3D::Render() {
    //lUILog("Clean...\n");
    // destroy caches
    for(uint16_t m=0;m<MAX_MESHES;m++) {
        if ( nullptr == meshVertexData[m] ) { continue; }
        if ( nullptr == meshFaceData[m] ) { continue; }
        if ( nullptr == meshNormalsData[m] ) { continue; }
        for(uint16_t i=0;i<meshVertexCount[m];i++) {
            meshVertexData[m][i]._xCache = ENGINE_INVALID_VALUE;
            meshVertexData[m][i]._yCache = ENGINE_INVALID_VALUE;
            meshVertexData[m][i]._deepCache = ENGINE_INVALID_VALUE;
        }
        for(uint16_t i=0;i<meshFaceCount[m];i++) {
            meshFaceData[m][i]._clockWiseCache=(int8_t)ENGINE_INVALID_VALUE;
        }
        for(uint16_t i=0;i<meshFaceCount[m];i++) {
            meshNormalsData[m][i]._deepCache=ENGINE_INVALID_VALUE;
        }
    }
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
        if ( nullptr == meshVertexData[m] ) { continue; }
        for(uint16_t i=0;i<meshVertexCount[m];i++) {
            Point2D pix;
            GetProjection(meshVertexData[m][i],pix);
            UpdateClipWith(ViewClipping,pix);
            int32_t deep = GetDeepForPoint(meshVertexData[m][i]);
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
            if ( nullptr == meshVertexData[m] ) { continue; }
            if ( nullptr == meshFaceData[m] ) { continue; }
            for(uint16_t i=0;i<meshFaceCount[m];i++) {
                Face3D & face = meshFaceData[m][i]; 
                Vertex3D & p0 = meshVertexData[m][face.vertex[0]];
                Vertex3D & p1 = meshVertexData[m][face.vertex[1]];
                Vertex3D & p2 = meshVertexData[m][face.vertex[2]];
                // get the 3 involved vertex deep to get planar deepth
                int16_t deep0 = GetDeepForPoint(p0);
                int16_t deep1 = GetDeepForPoint(p1);
                int16_t deep2 = GetDeepForPoint(p2);
                int32_t medianDeep = (deep0+deep1+deep2)/3;
                if ( currentDepth != medianDeep ) { continue; } // is face in this deep? (painter algorythm, back to forth)

                // backface culling
                //if ( false == IsClockwise(pix0,pix1,pix2) ) { continue; } // backface-culing
                if ( CLOCKWISE != IsClockwise(face,m) ) { continue; } // backface-culing
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
                int16_t alpha=(255*(float(medianDeep+abs(MeshDimensions.Min))/float(MaxDeep+abs(MeshDimensions.Min))));
                if ( alpha > 0 ) {
                    const uint16_t zcolor = tft->color565(alpha,alpha,alpha);
                    // fill the cache
                    float distNormal = NormalFacing(meshNormalsData[m][i]);
                    
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
                }

            }
        }
        currentDepth++;
    }

    // render here!
    //lUILog("Rendering...\n");
    canvas->fillSprite(TFT_BLACK);
    for(int i=0;i<orderdFaces;i++) {
        int32_t tp0x = meshOrderedFaces[i].p0x;
        int32_t tp0y = meshOrderedFaces[i].p0y;
        int32_t tp1x = meshOrderedFaces[i].p1x;
        int32_t tp1y = meshOrderedFaces[i].p1y;
        int32_t tp2x = meshOrderedFaces[i].p2x;
        int32_t tp2y = meshOrderedFaces[i].p2y;

        //@FACE
        uint16_t finalColor = meshOrderedFaces[i].faceColor;
        bool smooth =  meshOrderedFaces[i].smooth;

        uint8_t mixProp=128;
        //uint16_t deepColor = tft->alphaBlend(128,mixProp,meshOrderedFaces[i].colorDeep,finalColor);
        //finalColor = tft->alphaBlend(mixProp,deepColor,finalColor);
        if ( smooth ) {
            mixProp=32;
        }
        float dist = meshOrderedFaces[i].normalFacing; // NormalFacing(normal); //fabs(NormalFacing(normal));
        if ( dist>0 ) {
            uint8_t baseCol=(128*dist);
            uint16_t nColor = tft->color565(baseCol,baseCol,baseCol);
            finalColor = tft->alphaBlend(mixProp,nColor,finalColor);
        }
        canvas->fillTriangle(tp0x,tp0y,tp1x,tp1y,tp2x,tp2y,finalColor);
    }
    //lUILog("Render complete!\n");

}
*/

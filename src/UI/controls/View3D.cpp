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
#include "../../app/LogView.hpp"
#include <sys/param.h>
#include <cmath>
#include <esp_task_wdt.h>

using namespace LuI;

bool View3D::CacheIsFilled=false;
float View3D::AngleSinCache[360];
float View3D::AngleCosCache[360];
float View3D::AngleCache[360];
const double View3D::sq2 = sqrt(2);
const double View3D::sq6 = sqrt(6);

View3D::~View3D() {
    lLog("View3D %p destroyed!\n",this);
    if ( nullptr != meshVertexData ) { free(meshVertexData); }
    if ( nullptr != meshFaceData ) { free(meshFaceData); }
    if ( nullptr != meshNormalsData ) { free(meshNormalsData); }
    if ( nullptr != meshOrderedFaces ) { free(meshOrderedFaces); }
}

View3D::View3D(): Control() {
    lLog("View3D %p created!\n",this);
    if ( false == CacheIsFilled ) {
        // precalculate angles
        lUILog("View3D precalculating shared cache sin/cos/angle...\n");
        for(int i=0;i<360;i++){ 
            float iii=i*0.01745;
            View3D::AngleCosCache[i]= cos(iii);
            View3D::AngleSinCache[i]= sin(iii);
            View3D::AngleCache[i]=(iii*M_PI)/180;
        }
        View3D::CacheIsFilled=true;
        lUILog("View3D Cache generated\n");
    }
}

void View3D::Refresh(bool swap) {
    //lLog("View3D %p refresh\n",this);
    Control::Refresh(swap);
    centerX = width/2;
    centerY = height/2;
    Render();
}

bool View3D::AddModel(IN Mesh3DDescriptor * meshData) {
    lUILog("Adding mesh from %p...\n",meshData);
    //meshVertexData=(Vertex3D *)calloc(meshData->vetexNum,sizeof(Vertex3D));
    meshVertexData=(Vertex3D *)ps_calloc(meshData->vetexNum,sizeof(Vertex3D));
    if ( nullptr == meshVertexData) { 
        lUILog("Unable to allocate Vertex Data\n");
        return false; }
    //meshFaceData=(Face3D *)calloc(meshData->faceNum,sizeof(Face3D));
    meshFaceData=(Face3D *)ps_calloc(meshData->faceNum,sizeof(Face3D));
    if ( nullptr == meshFaceData) {
        lUILog("Unable to allocate Face Data\n");
        return false;
    }
    //meshNormalsData=(Normal3D *)calloc(meshData->faceNum,sizeof(Normal3D));
    meshNormalsData=(Normal3D *)ps_calloc(meshData->faceNum,sizeof(Normal3D));
    if ( nullptr == meshNormalsData) {
        lUILog("Unable to allocate Normal Data\n");
        return false;
    }
    //meshOrderedFaces=(OrderedFace3D *)calloc(meshData->faceNum,sizeof(OrderedFace3D));
    meshOrderedFaces=(OrderedFace3D *)ps_calloc(meshData->faceNum,sizeof(OrderedFace3D));
    if ( nullptr == meshOrderedFaces) {
        lUILog("Unable to allocate Ordered faces Data\n");
        return false;
    }

    // build vertex
    for(uint16_t vc=0;vc<meshData->vetexNum;vc++) {
        const RawVertex3D vertex = meshData->vertex[vc];
        meshVertexData[vc]= { 
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
        meshFaceData[fc]= {
            face.color,
            { face.vertex[0],face.vertex[1],face.vertex[2] },
            face.smooth,
            (int8_t)ENGINE_INVALID_VALUE
        };
    }
    //build normals
    for(uint16_t nc=0;nc<meshData->faceNum;nc++) {
        const RawNormal3D normal = meshData->normal[nc];
        meshNormalsData[nc]= {
            { normal.vertexData[0],normal.vertexData[1],normal.vertexData[2] },
            ENGINE_INVALID_VALUE
        };
    }

    for(uint16_t vc=0;vc<meshData->vetexNum;vc++) {
        Scale(meshVertexData[vc],ScaleMesh);
    }

    // don't set until the end
    meshVertexCount=meshData->vetexNum;
    meshFaceCount=meshData->faceNum;
    lUILog("Mesh loaded\n");
    return true;
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

void View3D::GetProjection(INOUT Vertex3D &vertex, OUT Point2D &pixel ) {
    // https://stackoverflow.com/questions/28607713/convert-3d-coordinates-to-2d-in-an-isometric-projection
    //*u=(x-z)/sqrt(2);
    //*v=(x+2*y+z)/sqrt(6);
    // u = x*cos(α) + y*cos(α+120°) + z*cos(α-120°)
    // v = x*sin(α) + y*sin(α+120°) + z*sin(α-120°)
    if ( vertex._xCache == ENGINE_INVALID_VALUE ) {
        vertex._xCache=(vertex.x-vertex.z)/sq2;
    }
    if ( vertex._yCache == ENGINE_INVALID_VALUE ) {
        vertex._yCache = (vertex.x+2*vertex.y+vertex.z)/sq6;
    }
    pixel.x=vertex._xCache;
    pixel.y=vertex._yCache;
}

int16_t View3D::GetDeepForPoint(INOUT Vertex3D &point) {
    if (point._deepCache != ENGINE_INVALID_VALUE ) { return point._deepCache; }
    //int16_t deep=3*point.x-3*point.y+3*point.z;
    int16_t deep=2*point.x-2*point.y+2*point.z; // isometric deep :D
    point._deepCache=deep;
    //int16_t deep = (point.x+point.z)/2;
    return deep;
}

void View3D::UpdateRange(INOUT Range &range, IN int32_t value) {
    range.Min=MIN(range.Min,value);
    range.Max=MAX(range.Max,value);
}

bool View3D::IsClockwise(INOUT Face3D &face) {
    if ( ENGINE_INVALID_VALUE != face._clockWiseCache ) {
        return (face._clockWiseCache?true:false);
    }
    Vertex3D & p0 = meshVertexData[face.vertex[0]];
    Vertex3D & p1 = meshVertexData[face.vertex[1]];
    Vertex3D & p2 = meshVertexData[face.vertex[2]];
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
        face._clockWiseCache=0;
    } else { 
        face._clockWiseCache=1;
    }
    return (face._clockWiseCache?true:false);
}

float View3D::NormalFacing(INOUT Normal3D &normal) {
    if ( ENGINE_INVALID_VALUE != normal._deepCache ) { return normal._deepCache; }
    normal._deepCache=normal.vertexData[0]-normal.vertexData[1]+normal.vertexData[2];  
    //if(deep<0){ return true; }
    return normal._deepCache;
}

void View3D::Scale(INOUT Vertex3D & point, IN Vertex3D & axes) {
    point.x *= axes.x;
    point.y *= axes.y;
    point.z *= axes.z;
    /*
    point.x = point.x * axes.x + (1 - axes.x) * point.x;
    point.y = point.y * axes.y + (1 - axes.y) * point.y;
    point.z = point.z * axes.z + (1 - axes.z) * point.z;
    */
} 

void View3D::Render() {
    //lUILog("Clean...\n");
    // destroy caches
    for(uint16_t i=0;i<meshVertexCount;i++) {
        meshVertexData[i]._xCache = ENGINE_INVALID_VALUE;
        meshVertexData[i]._yCache = ENGINE_INVALID_VALUE;
        meshVertexData[i]._deepCache = ENGINE_INVALID_VALUE;
    }
    for(uint16_t i=0;i<meshFaceCount;i++) {
        meshFaceData[i]._clockWiseCache=(int8_t)ENGINE_INVALID_VALUE;
    }
    for(uint16_t i=0;i<meshFaceCount;i++) {
        meshNormalsData[i]._deepCache=ENGINE_INVALID_VALUE;
    }

    // clear the mesh size
    MeshDimensions.Max=0;
    MeshDimensions.Min=1;

    //lUILog("Rotate...\n");

    // rotate vertex!
    for(uint16_t i=0;i<meshVertexCount;i++) {
        Rotate(meshVertexData[i],RotateMesh);
        Point2D pix;
        GetProjection(meshVertexData[i],pix);
        //app->UpdateClipWith(ViewClipping,pix);
        int32_t deep = GetDeepForPoint(meshVertexData[i]);
        UpdateRange(MeshDimensions,deep);
    }

    // rotate normals!
    for(uint16_t i=0;i<meshFaceCount;i++) {
        Rotate(meshNormalsData[i],RotateMesh);
    }

    //lUILog("Ordering...\n");
    // painter algorythm
    const int32_t MaxDeep=MeshDimensions.Max; //+abs(MeshDimensions.Min);
    int32_t currentDepth=MeshDimensions.Min;
    size_t orderdFaces=0;
    while ( currentDepth < MaxDeep ) {
        for(uint16_t i=0;i<meshFaceCount;i++) {
            Face3D & face = meshFaceData[i]; 
            Vertex3D & p0 = meshVertexData[face.vertex[0]];
            Vertex3D & p1 = meshVertexData[face.vertex[1]];
            Vertex3D & p2 = meshVertexData[face.vertex[2]];
            // get the 3 involved vertex deep to get planar deepth
            int16_t deep0 = GetDeepForPoint(p0);
            int16_t deep1 = GetDeepForPoint(p1);
            int16_t deep2 = GetDeepForPoint(p2);
            int32_t medianDeep = (deep0+deep1+deep2)/3;
            if ( currentDepth != medianDeep ) { continue; } // is face in this deep? (painter algorythm, back to forth)

            // backface culling
            //if ( false == IsClockwise(pix0,pix1,pix2) ) { continue; } // backface-culing
            if ( false == IsClockwise(face) ) { continue; } // backface-culing
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
            if ( alpha > -0.1 ) {
                const uint16_t zcolor = tft->color565(alpha,alpha,alpha);
                // fill the cache
                float distNormal = NormalFacing(meshNormalsData[i]);
                
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
                orderdFaces++;
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
        uint16_t finalColor = meshFaceData[meshOrderedFaces[i].faceOffset].color;
        bool smooth =  meshFaceData[meshOrderedFaces[i].faceOffset].smooth;

        uint8_t mixProp=128;
        /*
        uint16_t deepColor = tft->alphaBlend(128,mixProp,meshOrderedFaces[i].colorDeep,finalColor);
        finalColor = tft->alphaBlend(mixProp,deepColor,finalColor);
        */
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
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

#include "Common.hpp"
#include "../../../app/LogView.hpp"
#include <sys/param.h>
#include <cmath>
#include <esp_task_wdt.h>
#include "Mesh3D.hpp"
#include <cstring>
using namespace LuI;

float Mesh3D::AngleSinCache[360];
float Mesh3D::AngleCosCache[360];
float Mesh3D::AngleCache[360];
const double Mesh3D::sq2 = sqrt(2);
const double Mesh3D::sq6 = sqrt(6);
bool Mesh3D::_CacheIsFilled=false;


Mesh3D::Mesh3D(IN Mesh3DDescriptor * meshData) {
    lUILog("Adding mesh from %p...\n",meshData);
    vertex=(Vertex3D *)ps_calloc(meshData->vetexNum,sizeof(Vertex3D));
    if ( nullptr == vertex) {
        lUILog("Unable to allocate Vertex Data\n");
    }
    face=(Face3D *)ps_calloc(meshData->faceNum,sizeof(Face3D));
    if ( nullptr == face) {
        lUILog("Unable to allocate Face Data\n");
    }
    normal=(Normal3D *)ps_calloc(meshData->faceNum,sizeof(Normal3D));
    if ( nullptr == normal) {
        lUILog("Unable to allocate Normal Data\n");
    }

    // initialize vertex
    for(uint16_t vc=0;vc<meshData->vetexNum;vc++) {
        const RawVertex3D rvertex = meshData->vertex[vc];
        vertex[vc]= { 
            rvertex.x,
            rvertex.y,
            rvertex.z,
            TFT_WHITE,
            ENGINE_INVALID_VALUE,
            ENGINE_INVALID_VALUE,
            ENGINE_INVALID_VALUE
        };
    }
    //build faces
    for(uint16_t fc=0;fc<meshData->faceNum;fc++) {
        const RawFace3D rface = meshData->face[fc];
        face[fc]= {
            rface.color,
            { rface.vertex[0],rface.vertex[1],rface.vertex[2] },
            rface.smooth,
            (int8_t)ENGINE_INVALID_VALUE
        };
    }
    //build normals
    for(uint16_t nc=0;nc<meshData->faceNum;nc++) {
        const RawNormal3D rnormal = meshData->normal[nc];
        normal[nc]= {
            { rnormal.vertexData[0],rnormal.vertexData[1],rnormal.vertexData[2] },
            ENGINE_INVALID_VALUE
        };
    }
    
    if ( false == _CacheIsFilled ) {
        // precalculate angles
        lUILog("Mesh3D precalculating shared cache sin/cos/angle...\n");
        for(int i=0;i<360;i++){ 
            float iii=i*0.01745;
            Mesh3D::AngleCosCache[i]= cos(iii);
            Mesh3D::AngleSinCache[i]= sin(iii);
            Mesh3D::AngleCache[i]=(iii*M_PI)/180;
        }
        Mesh3D::_CacheIsFilled=true;
        lUILog("Mesh3D Cache generated\n");
    }

    // don't set until the end
    meshVertexCount=meshData->vetexNum;
    meshFaceCount=meshData->faceNum;
    lUILog("Mesh: %p loaded from: %p (Vertex: %u Faces: %u)\n",this,meshData,meshVertexCount,meshFaceCount);
}

Mesh3D::~Mesh3D() {
    if ( nullptr != vertex ) { free(vertex); }
    if ( nullptr != face ) { free(face); }
    if ( nullptr != normal ) { free(normal); }
    if ( nullptr != vertexCache ) { free(vertexCache); }
    if ( nullptr != normalCache ) { free(normalCache); }

    if ( nullptr != bilboard ) {
        bilboard->deleteSprite();
        delete bilboard;
        bilboard=nullptr;
    }
    lLog("Mesh: %p destroyed\n",this);
}

void Mesh3D::RelativeRotate(Angle3D rotation) {
    //lLog("Mesh: %p After relative rotation: X: %f  Y: %f Z: %f\n",this,MeshRotation.x,MeshRotation.y,MeshRotation.z);
    MeshRotation.x+=rotation.x;
    MeshRotation.y+=rotation.y;
    MeshRotation.z+=rotation.z;
    //@TODO use modules instead of sums
    if ( MeshRotation.x > 359.0 ) { MeshRotation.x-=360.0; }
    else if ( MeshRotation.x < -359.0 ) { MeshRotation.x+=360.0; }
    if ( MeshRotation.y > 359.0 ) { MeshRotation.y-=360.0; }
    else if ( MeshRotation.y < -359.0 ) { MeshRotation.y+=360.0; }
    if ( MeshRotation.z > 359.0 ) { MeshRotation.z-=360.0; }
    else if ( MeshRotation.z < -359.0 ) { MeshRotation.z+=360.0; }
    //lLog("Mesh: %p Current relative rotation: X: %f  Y: %f Z: %f\n",this,MeshRotation.x,MeshRotation.y,MeshRotation.z);
    dirty=true;
}



Vertex3D Mesh3D::SumVectors(Vertex3D location0,Vertex3D location1) {
    Vertex3D result = {
        location0.x+location1.x,
        location0.y+location1.y,
        location0.z+location1.z
    };
    return result;
}

Angle3D Mesh3D::SumAngles(Angle3D angle0,Angle3D angle1) {
    //lLog("Mesh: %p After relative rotation: X: %f  Y: %f Z: %f\n",this,MeshRotation.x,MeshRotation.y,MeshRotation.z);
    angle0.x+=angle1.x;
    angle0.y+=angle1.y;
    angle0.z+=angle1.z;
    //@TODO use modules instead of sums
    if ( angle0.x > 359.0 ) { angle0.x-=360.0; }
    else if ( angle0.x < -359.0 ) { angle0.x+=360.0; }
    if ( angle0.y > 359.0 ) { angle0.y-=360.0; }
    else if ( angle0.y < -359.0 ) { angle0.y+=360.0; }
    if ( angle0.z > 359.0 ) { angle0.z-=360.0; }
    else if ( angle0.z < -359.0 ) { angle0.z+=360.0; }
    //lLog("Mesh: %p Current relative rotation: X: %f  Y: %f Z: %f\n",this,MeshRotation.x,MeshRotation.y,MeshRotation.z);
    return angle0;
}

void Mesh3D::RelativeScale(Vertex3D scale) {
    //lLog("Mesh: %p After relative scale: X: %f  Y: %f Z: %f\n",this,MeshScale.x,MeshScale.y,MeshScale.z);
    MeshScale.x=MeshScale.x*scale.x;
    MeshScale.y=MeshScale.y*scale.y;
    MeshScale.z=MeshScale.z*scale.z;
    //lLog("Mesh: %p Current relative scale: X: %f  Y: %f Z: %f\n",this,MeshScale.x,MeshScale.y,MeshScale.z);
    dirty=true;
}

void Mesh3D::RelativeTranslate(Vertex3D location) {
    //lLog("Mesh: %p After relative translate: X: %f  Y: %f Z: %f\n",this,MeshLocation.x,MeshLocation.y,MeshLocation.z);
    MeshLocation.x+=location.x;
    MeshLocation.y+=location.y;
    MeshLocation.z+=location.z;
    //lLog("Mesh: %p Current relative translate: X: %f  Y: %f Z: %f\n",this,MeshLocation.x,MeshLocation.y,MeshLocation.z);
    dirty=true;
}

void Mesh3D::Rotate(Angle3D rotation) {
    //lLog("Mesh: %p After rotation: X: %f  Y: %f Z: %f\n",this,MeshRotation.x,MeshRotation.y,MeshRotation.z);
    MeshRotation.x=rotation.x;
    MeshRotation.y=rotation.y;
    MeshRotation.z=rotation.z;
    //lLog("Mesh: %p Current rotation: X: %f  Y: %f Z: %f\n",this,MeshRotation.x,MeshRotation.y,MeshRotation.z);
    dirty=true;
}

void Mesh3D::Scale(float scale) {
    Vertex3D curr = { scale,scale,scale };
    Scale(curr);
}

void Mesh3D::Scale(Vertex3D scale) {
    //lLog("Mesh: %p After scale: X: %f  Y: %f Z: %f\n",this,MeshScale.x,MeshScale.y,MeshScale.z);
    MeshScale.x=scale.x;
    MeshScale.y=scale.y;
    MeshScale.z=scale.z;
    //lLog("Mesh: %p Current scale: X: %f  Y: %f Z: %f\n",this,MeshScale.x,MeshScale.y,MeshScale.z);
    dirty=true;
}
void Mesh3D::Translate(Vertex3D location) {
    lLog("Mesh: %p After translate: X: %f  Y: %f Z: %f\n",this,MeshLocation.x,MeshLocation.y,MeshLocation.z);
    MeshLocation.x=location.x;
    MeshLocation.y=location.y;
    MeshLocation.z=location.z;
    lLog("Mesh: %p Current translate: X: %f  Y: %f Z: %f\n",this,MeshLocation.x,MeshLocation.y,MeshLocation.z);
    dirty=true;
}

void Mesh3D::RotateVertex(INOUT Vertex3D & point, IN Angle3D & rotationAngles) {
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
}

void Mesh3D::RotateNormals(IN Angle3D & rotationAngles) {
    // rotate normals (lighting)
    for(uint16_t i=0;i<meshFaceCount;i++) {
        RotateNormal(normalCache[i],rotationAngles);
    }
    dirty=true;
}

void Mesh3D::RotateNormal(INOUT Normal3D & normal, IN Angle3D & rotationAngles) {
    float tempN;
    if ( 0.0 != rotationAngles.x ) {
        int angleOffset=(int)rotationAngles.x;
        if ( angleOffset < 0 ) { angleOffset+=360; }
        if ( angleOffset > 359 ) { angleOffset%=360; }
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
        if ( angleOffset > 359 ) { angleOffset%=360; }
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
        if ( angleOffset > 359 ) { angleOffset%=360; }
        float anglez=AngleCache[angleOffset];
        //float anglez = (rotationAngles.z * M_PI) / 180.0;
        tempN = normal.vertexData[0];
        //point.x = point.x * cos(anglez) - point.y * sin(anglez);
        //point.y = temp * sin(anglez) + point.y *cos(anglez);
        normal.vertexData[0] = normal.vertexData[0] * AngleCosCache[angleOffset] - normal.vertexData[1] * AngleSinCache[angleOffset];
        normal.vertexData[1] = tempN * AngleSinCache[angleOffset] + normal.vertexData[1] *AngleCosCache[angleOffset];
    }
}

void Mesh3D::ScaleVertex(INOUT Vertex3D & point, IN Vertex3D & axes) {
    point.x *= axes.x;
    point.y *= axes.y;
    point.z *= axes.z;
    /*
    point.x = point.x * axes.x + (1 - axes.x) * point.x;
    point.y = point.y * axes.y + (1 - axes.y) * point.y;
    point.z = point.z * axes.z + (1 - axes.z) * point.z;
    */
}

void Mesh3D::TranslateVertex(INOUT Vertex3D & point, IN Vertex3D & translate) { //, float tx, float ty, float tz){
    point.x += translate.x;
    point.y += translate.y;
    point.z += translate.z;
}

void Mesh3D::ApplyTransform(Vertex3D GlobalLocation, Angle3D GlobalRotation, Vertex3D GlobalScale) {
    // https://stackoverflow.com/questions/14607640/rotating-a-vector-in-3d-space
    // https://en.wikipedia.org/wiki/Rodrigues%27_rotation_formula

    if ( nullptr == vertexCache ) {
        vertexCache=(Vertex3D *)ps_calloc(meshVertexCount,sizeof(Vertex3D));
        if ( nullptr == vertexCache) {
            lUILog("Mesh3D:ApplyTransform %p Unable to allocate Vertex Data\n",this);
            return;
        }
    }
    if ( nullptr == faceCache ) {
        faceCache=(Face3D *)ps_calloc(meshFaceCount,sizeof(Face3D));
        if ( nullptr == faceCache) {
            lUILog("Mesh3D:ApplyTransform %p Unable to allocate Face Data\n",this);
            return;
        }
    }
    if ( nullptr == normalCache ) {
        normalCache=(Normal3D *)ps_calloc(meshFaceCount,sizeof(Normal3D));
        if ( nullptr == normalCache) {
            lUILog("Mesh3D:ApplyTransform %p Unable to allocate Normal Data\n",this);
            return;
        }
    }
    /*
    if ( false == dirty ) {
        //lUILog("Mesh3D:ApplyTransform %p isn't dirty\n",this);
        return;
    }*/
    //lUILog("Mesh3D:ApplyTransform %p copy mesh\n",this);
    memcpy(vertexCache,vertex,meshVertexCount*sizeof(Vertex3D));
    memcpy(faceCache,face,meshFaceCount*sizeof(Face3D));
    memcpy(normalCache,normal,meshFaceCount*sizeof(Normal3D));


    // rotate local vertex
    for(uint16_t i=0;i<meshVertexCount;i++) {
        RotateVertex(vertexCache[i],MeshRotation);
    }
    // rotate local normals!
    for(uint16_t i=0;i<meshFaceCount;i++) {
        RotateNormal(normalCache[i],MeshRotation);
    }

    // scale global+local vertex
    LuI::Vertex3D currentScale = {
        MeshScale.x*GlobalScale.x,
        MeshScale.y*GlobalScale.y,
        MeshScale.z*GlobalScale.z
    };
    for(uint16_t i=0;i<meshVertexCount;i++) {
        ScaleVertex(vertexCache[i],currentScale);
    }

    // translate local+global vertex
    Vertex3D sumLoc = SumVectors(GlobalLocation,MeshLocation);
    for(uint16_t i=0;i<meshVertexCount;i++) {
        TranslateVertex(vertexCache[i],sumLoc);
    }

    // global rotate vertex
    for(uint16_t i=0;i<meshVertexCount;i++) {
        RotateVertex(vertexCache[i],GlobalRotation);
    }
    // global rotate normals!
    for(uint16_t i=0;i<meshFaceCount;i++) {
        RotateNormal(normalCache[i],GlobalRotation);
    }

    //lUILog("Mesh3D:ApplyTransform %p end transformation\n",this);
    dirty=true;
}

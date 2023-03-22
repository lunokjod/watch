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

#ifndef __LUNOKIOT__CONTROL_3DIMAGE_CLASS__
#define __LUNOKIOT__CONTROL_3DIMAGE_CLASS__

#include "../../lunokIoT.hpp"
#include "base/Control.hpp"

namespace LuI {
    #define ENGINE_INVALID_VALUE -666
    #define CLOCKWISE 0
    #define COUNTER_CLOCKWISE 1

    typedef struct __attribute__((__packed__)) {
        const float x;
        const float y;
        const float z;
    } RawVertex3D;

    typedef struct __attribute__((__packed__)) {
        const uint16_t color;
        const bool smooth;
        const uint16_t vertex[3]; // remember, the mesh must be triangulated!!
    } RawFace3D;

    typedef struct __attribute__((__packed__)) {
        const float vertexData[3];
    } RawNormal3D;

    typedef struct __attribute__((__packed__)) {
        uint16_t vetexNum;
        uint16_t faceNum;
        const RawVertex3D * vertex;
        const RawFace3D * face;
        const RawNormal3D * normal;
    } Mesh3DDescriptor;

    typedef struct {
        float x;
        float y;
        float z;
        uint16_t color;
        int16_t _deepCache; // ENGINE_INVALID_VALUE // value
        int16_t _xCache; //  // ENGINE_INVALID_VALUE  // value
        int16_t _yCache; // ENGINE_INVALID_VALUE // value
    } Vertex3D;

    typedef struct {
        uint16_t color;
        uint16_t vertex[3]; // remember, the mesh must be triangulated!!
        bool smooth;
        int8_t _clockWiseCache; // ENGINE_INVALID_VALUE // CLOCKWISE // COUNTER_CLOCKWISE
    } Face3D;

    typedef struct {
        float vertexData[3];
        float _deepCache; // ENGINE_INVALID_VALUE // value
    } Normal3D;

    typedef struct {
        float x;
        float y;
        float z;
    } Angle3D;

    typedef struct {
        union {
            uint32_t x=0; // use as alias
            uint32_t u;
        };
        union {
            uint32_t y=0; // use as alias
            uint32_t v;
        };
        uint16_t color=TFT_WHITE;
    } Point2D;

    typedef struct {
        int32_t Min;
        int32_t Max;
    } Range;

    typedef struct {
        // 2d info about
        float normalFacing=0;
        uint32_t p0x=0;
        uint32_t p1x=0;
        uint32_t p2x=0;
        uint32_t p0y=0;
        uint32_t p1y=0;
        uint32_t p2y=0;
        size_t faceOffset=0;
        uint16_t colorDeep=0;
        int16_t alpha=0;
    //    int32_t deep=0;
    } OrderedFace3D;

    class View3D: public Control {
        protected:
            uint16_t meshVertexCount=0;
            Vertex3D *meshVertexData=nullptr;
            uint16_t meshFaceCount=0;
            Face3D * meshFaceData=nullptr;
            Normal3D * meshNormalsData=nullptr;
            OrderedFace3D * meshOrderedFaces=nullptr;
            Range MeshDimensions;
            uint16_t centerX;
            uint16_t centerY;
        public:
            Vertex3D ScaleMesh = { 0.5,0.5,0.5 };
            Angle3D RotateMesh = { 0,5,0 };
            ~View3D();
            View3D();
            void Render();
            void Rotate(INOUT Vertex3D & point, IN Angle3D & rotationAngles);
            void Rotate(INOUT Normal3D & point, IN Angle3D & rotationAngles);
            void Scale(INOUT Vertex3D & point, IN Vertex3D & axes);
            void GetProjection(INOUT Vertex3D &vertex, OUT Point2D &pixel );
            void UpdateRange(INOUT Range &range, IN int32_t value);
            int16_t GetDeepForPoint(INOUT Vertex3D &point);
            bool IsClockwise(INOUT Face3D &face);
            float NormalFacing(INOUT Normal3D &normal);
            bool AddModel(IN Mesh3DDescriptor * meshData);
            void Refresh(bool swap=false) override;
            static bool CacheIsFilled;
            static float AngleSinCache[360];
            static float AngleCosCache[360];
            static float AngleCache[360];
            static const double sq2;
            static const double sq6;
/*
            float angleSinCache[360] = { 0.0 };
float angleCosCache[360] = { 0.0 };
float angleCache[360] = { 0.0 };
*/
    };
};


#endif

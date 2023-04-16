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

#ifndef __LUNOKIOT__MESH_DEFINES___
#define __LUNOKIOT__MESH_DEFINES___
#include <Arduino.h>
#include <LilyGoWatch.h>

namespace LuI {
    #define ENGINE_INVALID_VALUE -666
    #define CLOCKWISE 1
    #define COUNTER_CLOCKWISE 0
    //#define INVALID_MESH -1
    #define MAX_MESHES 26
    #define MAX_ORDERED_FACES 2000

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
        uint32_t x=0; // use as alias
        uint32_t y=0; // use as alias
        uint16_t color=TFT_WHITE;
    } Point2D;

    typedef struct {
        int32_t Min;
        int32_t Max;
    } Range;

    // define screen area
    typedef struct {
        int16_t xMin;
        int16_t yMin;
        int16_t xMax;
        int16_t yMax;
    } Clipping2D;

    typedef struct {
        // 2d info about
        float normalFacing=0;
        int p0x=0;
        int p1x=0;
        int p2x=0;
        int p0y=0;
        int p1y=0;
        int p2y=0;
        /*
        uint32_t p0x=0;
        uint32_t p1x=0;
        uint32_t p2x=0;
        uint32_t p0y=0;
        uint32_t p1y=0;
        uint32_t p2y=0;
        */
        size_t faceOffset=0;
        uint16_t colorDeep=0;
        int16_t alpha=0;
        uint16_t faceColor=0;
        bool smooth=0;
        uint16_t fromMesh=0;
    } OrderedFace3D;

};

#endif


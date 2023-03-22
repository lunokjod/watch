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

#ifndef __LUNOKIOT__APPLICATION__ENGINE3D__
#define __LUNOKIOT__APPLICATION__ENGINE3D__

#include "../UI/AppTemplate.hpp"
#include "../UI/base/Widget.hpp"
//#include <cmath> 
#include "../lunokIoT.hpp"

#define ENGINE_INVALID_VALUE -666
#define ENGINE_TASK_PRIORITY tskIDLE_PRIORITY+2
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
    float x;
    float y;
    float z;
    uint16_t color=TFT_WHITE;
    int16_t _deepCache=ENGINE_INVALID_VALUE;
    uint32_t _xCache=ENGINE_INVALID_VALUE;
    uint32_t _yCache=ENGINE_INVALID_VALUE;
} Point3D;

typedef struct {
    float x;
    float y;
    float z;
} Angle3D;

typedef struct {
    float x;
    float y;
    float z;
    uint16_t color=TFT_WHITE;
    uint8_t radius=30;
} Light3D;

typedef struct {
    Point3D * from=nullptr;
    Point3D * to=nullptr;
    uint16_t color=TFT_BLACK;
} Vector3D;

typedef struct {
    size_t vertexNum=0;
    uint16_t color=TFT_DARKGREY;
    Point3D *vertexData[3]= { nullptr }; // remember, the mesh must be triangulated!!
    bool smooth=false;
    bool _clockWiseCache=false;
    bool _clockWiseCacheValue=false;
} Face3D;

typedef struct {
    size_t vertexNum=0;
    float vertexData[3]= { 0 };
    float _deepCache=ENGINE_INVALID_VALUE;
} Normal3D;

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

class Engine3DApplication : public TemplateApplication {
    public:

        //TFT_eSprite *light0 = nullptr;  // light calculation from  zbuffer
        Light3D Light0Point;
        ///Light3D Light1Point;
        //Light3D Light2Point;
        void * core0Worker=nullptr;
        void * core1Worker=nullptr;

        //const size_t ZBufferFaceSize = canvas->width()+canvas->height()/2;
        //size_t **zbufferFaceOrder = nullptr;
        /*
int** x;
x = malloc(dimension1_max * sizeof(*x));
for (int i = 0; i < dimension1_max; i++) {
  x[i] = malloc(dimension2_max * sizeof(x[0]));
}
*/
        //void BuildZBuffer();
        void DirectRender(IN Range MeshDimensions,INOUT TFT_eSprite * alphaChannel,
                                                  INOUT TFT_eSprite * zbuffer,
                                                  INOUT TFT_eSprite * normalBuffer,
                                                  INOUT TFT_eSprite * polyBuffer,
                                                  INOUT TFT_eSprite * smoothBuffer,
                                                  INOUT TFT_eSprite * lightBuffer,
                                                  INOUT TFT_eSprite * buffer3d);
        void UpdateClipWith(INOUT Clipping2D &clip, IN int16_t x,IN int16_t y);
        void UpdateClipWith(INOUT Clipping2D &clip, IN Point2D &point);
        void UpdateRange(INOUT Range &range, IN int32_t value);
        const uint8_t border=16;
        uint8_t lastCoreUsed=0;
        Angle3D rot;
        Angle3D LampRotation;
        Angle3D lastRot;
        const uint32_t AvailColors[9] = {TFT_RED,TFT_GREEN,TFT_BLUE,TFT_PURPLE,TFT_CYAN,TFT_MAGENTA,TFT_ORANGE,TFT_PINK,TFT_SKYBLUE};
        const double sq2 = sqrt(2);
        const double sq6 = sqrt(6);

        size_t myPointsNumber=0;
        Point3D * myPoints=nullptr;

        size_t myVectorsNumber=0;
        Vector3D * myVectors=nullptr;

        size_t myFacesNumber=0;
        Face3D *myFaces = nullptr;

        size_t myNormalsNumber=0;
        Normal3D *myNormals = nullptr;

        OrderedFace3D * facesToRender = nullptr;
        float lastDegX = 0.0; // degrees
        float lastDegY = 0.0;
        float lastDegZ = 0.0;
        unsigned long nextRefreshRotation=0;
        const float MeshSize = 0.6; //0.8;     // scale of mesh
        const uint16_t centerX = canvas->width()/2;
        const uint16_t centerY = canvas->height()/2;
        const int16_t renderSizeH = canvas->height();
        const int16_t renderSizeW = canvas->width();
        int32_t lastPx = 0;
        int32_t lastPy = 0;

        uint32_t selectedFace = 0;
        // lunokApp calls
        const char *AppName() override { return "Engine3D Test"; };
        Engine3DApplication();
        ~Engine3DApplication();
        bool Tick();
        //bool IsClockwise(IN Point2D &pix0,IN Point2D &pix1,IN Point2D &pix2);
        inline bool IsClockwise(INOUT Face3D &face);
        // tools
        inline float NormalFacing(INOUT Normal3D &normal);
        // convert 3D to 2D
        inline void GetProjection(INOUT Point3D &vertex, OUT Point2D &pixel);
        void GetProjection(IN Light3D &vertex, OUT Point2D &pixel ); 
        // obtain the point depth
        inline int16_t GetDeepForPoint(INOUT Point3D &point);
        // get zbuffer height for corresponding point
        //uint8_t GetZBufferDeepForPoint(INOUT TFT_eSprite *zbuffer,IN int32_t &x, IN int32_t &y);
        //uint8_t GetZBufferDeepForPoint(INOUT TFT_eSprite *zbuffer,INOUT Point2D &pix);
        //uint8_t GetZBufferDeepForPoint(INOUT TFT_eSprite *zbuffer,INOUT Point3D &point);

        // render mesh parts
        inline void RenderFace(INOUT TFT_eSprite *buffer, bool shaded,uint32_t overrideColor, bool borders,IN Point2D &one,IN Point2D &two,IN Point2D &tree);
        inline void RenderFace(INOUT TFT_eSprite *buffer, IN Face3D & face,bool shaded=true,uint32_t overrideColor=Drawable::MASK_COLOR,bool borders=false);
        //void RenderPoint(INOUT TFT_eSprite *buffer, IN Point3D & point);
        //void RenderVector(IN Vector3D &vec);

        // manipulate vertex
        void Translate(INOUT Point3D & point, IN Point3D & translate);
        inline void Rotate(INOUT Normal3D & normal, IN Angle3D & rotationAngles);
        inline void Rotate(INOUT Point3D & point, IN Angle3D & rotationAngles);
        //void Rotate(INOUT Light3D & light, IN Angle3D & rotationAngles);
        void Scale(INOUT Point3D & point, IN Point3D & axes);
};

typedef struct {
    bool task=false;
    uint8_t core=0;
    Engine3DApplication * instance;
    bool run=true;
} RenderCore;

#endif

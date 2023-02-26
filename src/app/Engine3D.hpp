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

// almost empty defines to clarify the paramethers direction (see declarations below)
#define IN const // clarify to developer if the data is input or output
#define OUT
#define INOUT

typedef struct {
    int32_t Min;
    int32_t Max;
} Range;

// define screen area
typedef struct {
    uint16_t xMin;
    uint16_t yMin;
    uint16_t xMax;
    uint16_t yMax;
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
} Face3D;

class Engine3DApplication : public TemplateApplication {
    private:
        const float MeshSize = 60.0;     // size of mesh
        TFT_eSprite *buffer3d = nullptr; // where 3D are rendered
        TFT_eSprite *zbuffer = nullptr;  // the deep of pixels
        TFT_eSprite *alphaChannel = nullptr;  // have mesh on the area?
        TFT_eSprite *light0 = nullptr;  // light calculation from  zbuffer
        Light3D Light0Point;
        Light3D Light1Point;
        Light3D Light2Point;
        const size_t ZBufferFaceSize = canvas->width()+canvas->height()/2;
        size_t **zbufferFaceOrder = nullptr;
        /*
int** x;
x = malloc(dimension1_max * sizeof(*x));
for (int i = 0; i < dimension1_max; i++) {
  x[i] = malloc(dimension2_max * sizeof(x[0]));
}
*/
        Range MeshDimensions;
        void BuildZBuffer();
        void Render();
        void CleanBuffers();
        Clipping2D ViewClipping;    // used to know the updated area on the current rendered frame
        void UpdateClipWith(INOUT Clipping2D &clip, IN uint16_t x,IN uint16_t y);
        void UpdateClipWith(INOUT Clipping2D &clip, IN Point2D &point);
        void UpdateRange(INOUT Range &range, IN int32_t value);

        Angle3D rot;
        Angle3D LampRotation;
        Angle3D lastRot;
        size_t AvailColorsoffset=0;
        const uint32_t AvailColors[9] = {TFT_RED,TFT_GREEN,TFT_BLUE,TFT_PURPLE,TFT_CYAN,TFT_MAGENTA,TFT_ORANGE,TFT_PINK,TFT_SKYBLUE};
        const double sq2 = sqrt(2);
        const double sq6 = sqrt(6);
        const uint16_t centerX = canvas->width()/2;
        const uint16_t centerY = canvas->height()/2;

        size_t myPointsNumber=0;
        Point3D * myPoints=nullptr;

        size_t myVectorsNumber=0;
        Vector3D * myVectors=nullptr;

        size_t myFacesNumber=0;
        Face3D *myFaces = nullptr;

    public:
        // lunokApp calls
        const char *AppName() override { return "Engine3D Test"; };
        Engine3DApplication();
        ~Engine3DApplication();
        bool Tick();

        // tools
        // convert 3D to 2D
        void GetProjection(IN Point3D &vertex, OUT Point2D &pixel);
        void GetProjection(IN Light3D &vertex, OUT Point2D &pixel);
        // obtain the point depth
        int16_t GetDeepForPoint(IN Point3D &point);
        // get zbuffer height for corresponding point
        uint8_t GetZBufferDeepForPoint(IN Point3D &point);

        // render mesh parts
        void RenderFace(INOUT TFT_eSprite *buffer, IN Face3D & face,bool shaded=true,uint32_t overrideColor=Drawable::MASK_COLOR);
        void RenderPoint(IN Point3D & point);
        void RenderVector(IN Vector3D &vec);

        // manipulate vertex
        void Translate(INOUT Point3D & point, IN Point3D & translate);
        void Rotate(INOUT Point3D & point, IN Angle3D & rotationAngles);
        void Rotate(INOUT Light3D & light, IN Angle3D & rotationAngles);
        void Scale(INOUT Point3D & point, IN Point3D & axes);
};

#endif

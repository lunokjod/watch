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
#include <cmath> 

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
    Point3D * from=nullptr;
    Point3D * to=nullptr;
    uint16_t color=TFT_DARKGREY;
} Vector3D;

typedef struct {
    size_t vertexNum=0;
    Point3D *vertexData[5]= { nullptr };
} Face3D;


class Engine3DApplication : public TemplateApplication {
    private:
        unsigned long nextRefresh=0;
        unsigned long nextStep=0;
        int16_t minLastX=119;
        int16_t minLastY=119;
        int16_t maxLastX=121;
        int16_t maxLastY=121;
        Angle3D rot;
        Angle3D lastRot;
        TFT_eSprite *buffer3d = nullptr;
    public:
        const char *AppName() override { return "Engine3D Test"; };
        Engine3DApplication();
        ~Engine3DApplication();
        void RenderFace(Face3D & face);
        void RenderPoint(Point3D & point);
        void RenderVector(Vector3D &vec);
        void TranslatePoint(Point3D & point, Point3D & translate);
        void Rotate(Point3D & point, Angle3D & rotationAngles);
        void Scale(Point3D & point, Point3D & axes);
        void BoundaryFill4(int32_t x, int32_t y, uint16_t fill_color,uint16_t boundary_color);
        //bool FillWithColor(int16_t x,int16_t y,uint32_t color,uint32_t stopColor);
        bool Tick();
        size_t myPointsNumber=0;
        Point3D * myPoints=nullptr;
        size_t myVectorsNumber=0;
        Vector3D * myVectors=nullptr;
        size_t myFacesNumber=0;
        Face3D *myFaces = nullptr;
        size_t AvailColorsoffset=0;
        uint32_t AvailColors[9] = {TFT_RED,TFT_GREEN,TFT_BLUE,TFT_PURPLE,TFT_CYAN,TFT_MAGENTA,TFT_ORANGE,TFT_PINK,TFT_SKYBLUE};

        const double sq2 = sqrt(2);
        const double sq6 = sqrt(6);
        const uint16_t centerX = canvas->width()/2;
        const uint16_t centerY = canvas->height()/2;
};

#endif

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

//#include "../../lunokIoT.hpp"
#include "../base/Control.hpp"
#include "Common.hpp"
#include "Mesh3D.hpp"

namespace LuI {

    class View3D: public Control {
        protected:
        /*
            uint16_t meshCount=0; // maintain the number of meshes on view
            uint16_t meshVertexCount[MAX_MESHES]= { 0 };
            Vertex3D *meshVertexData[MAX_MESHES]= { nullptr };
            uint16_t meshFaceCount[MAX_MESHES]= { 0 };
            Face3D * meshFaceData[MAX_MESHES]= { nullptr };
            Normal3D * meshNormalsData[MAX_MESHES]= { nullptr } ;
            */
        public:
            uint32_t viewBackgroundColor=Drawable::MASK_COLOR;
            OrderedFace3D * meshOrderedFaces= { nullptr };
            uint16_t centerX;
            uint16_t centerY;
            Clipping2D ViewClipping;
            Range MeshDimensions;
            Mesh3D * mesh[MAX_MESHES] = { nullptr };
            void AddMesh3D(INOUT Mesh3D * meshObject);
            void Refresh(bool swap=false) override;
            void Render();
            void GetProjection(INOUT Vertex3D &vertex, OUT Point2D &pixel );
            void UpdateClipWith(INOUT Clipping2D &clip, IN int16_t x,IN int16_t y);
            void UpdateClipWith(INOUT Clipping2D &clip, IN Point2D &point);
            void UpdateRange(INOUT Range &range, IN int32_t value);
            int16_t GetDeepForPoint(INOUT Vertex3D &point);
            bool IsClockwise(INOUT Face3D &face,uint16_t meshNumber=0);
            float NormalFacing(INOUT Normal3D &normal);
            ~View3D();
            View3D();
            // events
            UICallback stepCallback=nullptr; // where call when tap
            void * stepCallbackParam=nullptr; // what pass to callbac

            TFT_eSprite * GetCanvas();
            /*
            Vertex3D LocationMesh[MAX_MESHES] = {{0,0,0}};
            Angle3D RotationMesh[MAX_MESHES] = {{0,0,0}};
            Vertex3D ScaleMesh[MAX_MESHES] = {{1,1,1}};

            //Vertex3D ScaleMesh = { 1,1,1 }; // current scale
            //Angle3D RotationMesh = { 0,0,0 }; // current rotation
            //Vertex3D FinalScaleMesh = { 1,1,1 }; // desired scale
            //Angle3D FinalRotationMesh = { 0,0,0 }; // desired rotation
            //void RotateStep();
            void ScaleStep();
            void Rotate(INOUT Vertex3D & point, IN Angle3D & rotationAngles);
            void Rotate(INOUT Normal3D & point, IN Angle3D & rotationAngles);
            void Rotate(IN Angle3D &rotationAngles, bool save=true);
            void Rotate(IN Angle3D &rotationAngles,int16_t meshId,bool save=true);
            void Scale(IN Vertex3D & axes);
            void Scale(INOUT Vertex3D & point, IN Vertex3D & axes);
            void Translate(IN Vertex3D & translate,int16_t meshNumber=0,bool save=true);
            void Translate(INOUT Vertex3D & point, IN Vertex3D & translate);
            void TranslateMesh(IN Vertex3D & translate,int16_t meshNumber);
            int16_t AddModel(IN Mesh3DDescriptor * meshData);
            static bool CacheIsFilled;
            static float AngleSinCache[360];
            static float AngleCosCache[360];
            static float AngleCache[360];
            static const double sq2;
            static const double sq6;
            */
/*
            float angleSinCache[360] = { 0.0 };
float angleCosCache[360] = { 0.0 };
float angleCache[360] = { 0.0 };
*/
    };
};


#endif

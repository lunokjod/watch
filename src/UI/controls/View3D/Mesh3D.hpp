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

#ifndef __LUNOKIOT__MESH_3DIMAGE_CLASS__
#define __LUNOKIOT__MESH_3DIMAGE_CLASS__
#include "../../../lunokIoT.hpp"
#include "Common.hpp"

namespace LuI {
    class Mesh3D {
        protected:
            Vertex3D *vertex=nullptr;
            Face3D * face=nullptr;
            Normal3D * normal=nullptr;
        public:
            Vertex3D MeshLocation = { 0,0,0 };
            Angle3D MeshRotation = { 0,0,0 };
            Vertex3D MeshScale = { 1,1,1 };

            uint16_t meshVertexCount=0;
            uint16_t meshFaceCount=0;
            
            // copy to apply transformations
            Vertex3D *vertexCache=nullptr;
            Face3D * faceCache=nullptr;
            Normal3D * normalCache=nullptr;

            // bilboard related, @note add bilboard disables rendering of mesh
            TFT_eSprite * bilboard=nullptr;
            uint16_t bilboardMaskColor=TFT_GREEN;
            float bilboardMultiplier=1.0;

            bool dirty=true;
            static bool _CacheIsFilled;
            static float AngleSinCache[360];
            static float AngleCosCache[360];
            static float AngleCache[360];
            static const double sq2;
            static const double sq6;

            Mesh3D(IN Mesh3DDescriptor * meshData);
            ~Mesh3D();
            void RelativeRotate(Angle3D rotation);
            void RelativeScale(Vertex3D scale);
            void RelativeTranslate(Vertex3D location);
            static Vertex3D SumVectors(Vertex3D location0,Vertex3D location1);
            static Angle3D SumAngles(Angle3D angle0,Angle3D angle1);
            void Rotate(Angle3D rotation);
            void Scale(Vertex3D scale);
            void Scale(float scale);
            void Translate(Vertex3D location);
            void RotateNormals(IN Angle3D & rotationAngles);
            void ApplyTransform(Vertex3D GlobalLocation={0,0,0}, Angle3D GlobalRotation={0,0,0}, Vertex3D GlobalScale={1,1,1});
            static void RotateVertex(INOUT Vertex3D & point, IN Angle3D & rotationAngles);
            static void RotateNormal(INOUT Normal3D & normal, IN Angle3D & rotationAngles);
            static void ScaleVertex(INOUT Vertex3D & point, IN Vertex3D & axes);
            static void TranslateVertex(INOUT Vertex3D & point, IN Vertex3D & translate);
    };
};

#endif


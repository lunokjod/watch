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
            size_t renderCount=0;
            Vertex3D GlobalLocation = { 0,0,0 };
            Angle3D GlobalRotation = { 0,0,0 };
            Vertex3D GlobalScale = { 1,1,1 };
            bool firstPush=false;
            uint32_t viewBackgroundColor=Drawable::MASK_COLOR;
        public:
            //@TODO this must be thinked a little bit more
            static TaskHandle_t renderTask;
            static SemaphoreHandle_t renderMutex;
            //void ThreadedRender();
            //bool renderLoop=true;
            uint16_t MaskColor=TFT_WHITE;
            typedef enum { FULL = 0, FLAT, WIREFRAME, FLATWIREFRAME, MASK, NODRAW} RENDER;
            RENDER RenderMode = FULL; 
            void SetBackgroundColor(uint16_t color) { viewBackgroundColor=color; }
            OrderedFace3D * meshOrderedFaces= { nullptr };
            uint16_t centerX;
            uint16_t centerY;
            Clipping2D ViewClipping;
            Range MeshDimensions;
            void SetAllDirty();
            Vertex3D GetGlobalLocation() { return GlobalLocation; }
            Angle3D GetGlobalRotation() { return GlobalRotation; }
            Vertex3D GetGlobalScale() { return GlobalScale; }
            void SetGlobalLocation(IN Vertex3D globLocation );
            void SetGlobalScale(IN float scale );
            void SetGlobalScale(IN Vertex3D globScale );
            void SetGlobalRotation(INOUT Angle3D globRotation );

            Mesh3D * mesh[MAX_MESHES+1] = { nullptr };
            void AddMesh3D(INOUT Mesh3D * meshObject);
            void Refresh(bool direct=false) override;
            void Render();
            void GetProjection(INOUT Vertex3D &vertex, OUT Point2D &pixel );
            void UpdateClipWith(INOUT Clipping2D &clip, IN int16_t x,IN int16_t y);
            void UpdateClipWith(INOUT Clipping2D &clip, IN Point2D &point);
            void UpdateRange(INOUT Range &range, IN int32_t value);
            int16_t GetDeepForPoint(INOUT Vertex3D &point);
            bool IsClockwise(INOUT Face3D &face,uint16_t meshNumber=0);
            float NormalFacing(INOUT Normal3D &normal);
            float OriginalNormalFacing(INOUT Normal3D &normal);
            ~View3D();
            View3D();
            // events
            UICallback stepCallback=nullptr; // where call when engine do a step (geometry)
            void * stepCallbackParam=nullptr; // what pass to callback
            void DrawBilboards();
            
            UICallback2 beforeRenderCallback=nullptr; // where call when all is clear to start
            void * beforeRenderCallbackParam=nullptr; // what pass to callback

            UICallback2 renderCallback=nullptr; // where call when render is done
            void * renderCallbackParam=nullptr; // what pass to callback

            UICallback3 polygonCallback=nullptr; // where call when render is done
            void * polygonCallbackParam=nullptr; // what pass to callback


            TFT_eSprite * GetCanvas();

    };
};


#endif

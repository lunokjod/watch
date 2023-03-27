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

#ifndef __LUNOKIOT__APPLICATION__LUI_DEBUG__
#define __LUNOKIOT__APPLICATION__LUI_DEBUG__

#include "../UI/AppLuITemplate.hpp"
#include "../UI/controls/View3D/View3D.hpp"

class DebugLuIApplication : public TemplateLuIApplication {
    private:
        unsigned long nextTime=0;
    public:
        bool followCamera=true;
        bool showLights=true;
        LuI::View3D * view3DTest0;
        LuI::View3D * view3DTest1;
        LuI::View3D * view3DTest2;
        LuI::View3D * view3DTest3;
        LuI::View3D * view3DTest4;
        LuI::View3D * view3DTest5;
        LuI::View3D * view3DTest6;
        LuI::View3D * view3DTest7;
        LuI::View3D * view3DTest8;
        //bool Tick();
        bool GrowOrDie=false;
        DebugLuIApplication();
        void FollowCamera();
        void FixedCamera();
        void SwitchLights();
        const char *AppName() override { return "Playground LuI"; };
};

#endif

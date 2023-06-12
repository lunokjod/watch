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

#ifndef __LUNOKIOT__APPLICATION__CALENDAR_LOGDETAILS__
#define __LUNOKIOT__APPLICATION__CALENDAR_LOGDETAILS__

#include "../UI/AppLuITemplate.hpp"
#include "../system/Datasources/database.hpp"
#include "../UI/controls/Buffer.hpp"
#include "../UI/controls/base/Container.hpp"
#include "../UI/widgets/GraphWidget.hpp"

class CalendarLogDetailApplication : public TemplateLuIApplication {
    private:
        Database *currentDB=nullptr;
    public:
        GraphWidget * PowerGraph=nullptr;
        GraphWidget * ActivityGraph=nullptr;
        GraphWidget * AppsGraph=nullptr;
        GraphWidget * NetworkGraph=nullptr;
        LuI::Buffer * logView=nullptr;
        LuI::Container * viewContainer=nullptr;
        int entries=0;
        bool beginScrub=false;
        int32_t currentPowerWidth=0;
        int32_t currentActivityWidth=40;
        int32_t currentAppsWidth=80;
        int32_t currentNetworkWidth=120;
        int32_t currentBufferDisplacement=0;
        char * cacheFileName=nullptr;
        ~CalendarLogDetailApplication();
        CalendarLogDetailApplication(int year, int month, int day);
        const char *AppName() override { return "Calendar log detail"; };
        bool Tick() override;
};

#endif

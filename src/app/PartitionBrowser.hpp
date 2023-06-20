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

#ifndef __LUNOKIOT__APPLICATION__PARTITION_BROWSER__
#define __LUNOKIOT__APPLICATION__PARTITION_BROWSER__

#include "../UI/AppLuITemplate.hpp"
#include "../UI/controls/Text.hpp"
#include "../UI/controls/IconMenu.hpp"

class PartitionExplorerApplication : public TemplateLuIApplication {
    private:
        size_t partTableCSVSize=0;
        char * partTableCSV=nullptr;
        size_t appPartitionNumber=0;
        size_t dataPartitionNumber=0;
    public:
        unsigned long lastCheck=0;
        LuI::Text * devicesFoundText;
        LuI::Text * totalDBText;
        LuI::Text * statusText;
        LuI::IconMenu * locationMenu;
        ~PartitionExplorerApplication();
        PartitionExplorerApplication(const char * path="/");
        void RefreshTotalKnowDevices();
        const char *AppName() override { return "Partition explorer"; };
        bool Tick() override;
};

#endif

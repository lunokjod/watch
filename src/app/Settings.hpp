#ifndef __LUNOKIOT__SETTINGS_APP__
#define __LUNOKIOT__SETTINGS_APP__

#include "../UI/AppTemplate.hpp" // typical app design base with back button

// widgets used
#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../UI/widgets/SwitchWidget.hpp"

class SettingsApplication: public TemplateApplication {
    private:
        bool showOverlay=false;
        ButtonImageXBMWidget * btnHelp = nullptr;
        SwitchWidget * ntpCheck = nullptr;
        SwitchWidget * openweatherCheck = nullptr;
        SwitchWidget * wifiCheck = nullptr;
        SwitchWidget * bleCheck = nullptr;
    public:        
        SettingsApplication();
        virtual ~SettingsApplication();
        bool Tick();
};

#endif

#ifndef __LUNOKIOT__APPLICATION__THEME__
#define __LUNOKIOT__APPLICATION__THEME__

#include "../UI/AppTemplate.hpp"
#include "../UI/widgets/ButtonWidget.hpp"
#include "../UI/widgets/CanvasWidget.hpp"

class ThemeApplication : public TemplateApplication {
    private:
        CanvasWidget * imageBase=nullptr;
        unsigned long nextRefresh=0;
        const static size_t MAXThemes = 9;
        ButtonWidget *themeButton[MAXThemes] = { nullptr };
    public:
        const char *AppName() override { return "Themes"; };
        ThemeApplication();
        ~ThemeApplication();
        bool Tick();
};

#endif

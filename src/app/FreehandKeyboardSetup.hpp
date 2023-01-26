#ifndef __LUNOKIOT__APPLICATION__FREEHAND_KEYBOARD_SETUP__
#define __LUNOKIOT__APPLICATION__FREEHAND_KEYBOARD_SETUP__

#include "../UI/AppTemplate.hpp"
#include "../UI/widgets/ButtonTextWidget.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include "../UI/widgets/SwitchWidget.hpp"
#include "../system/Datasources/perceptron.hpp"

class FreeHandKeyboardSetupApplication : public TemplateApplication {
    private:
        unsigned long nextRefresh=0;
        ButtonTextWidget * trainButton=nullptr;
        ButtonTextWidget * LoadButton=nullptr;
        ButtonTextWidget * DumpButton=nullptr;
        //ButtonTextWidget * tryButton=nullptr;
        ButtonTextWidget * multiCheckButton=nullptr;
        ButtonImageXBMWidget * btnEraseTrain=nullptr;
        SwitchWidget * switchUseAltKeyb = nullptr;
    public:
        const char *AppName() override { return "FreeHandKeyboard Setup"; };
        FreeHandKeyboardSetupApplication();
        ~FreeHandKeyboardSetupApplication();
        Perceptron * LoadPerceptronFromSymbol(char symbol);
        void SavePerceptronWithSymbol(char symbol,Perceptron *item);
        bool Tick();
};

#endif

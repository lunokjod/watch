#ifndef __LUNOKIOT__APPLICATION__CALENDAR__
#define __LUNOKIOT__APPLICATION__CALENDAR__

#include "../UI/AppTemplate.hpp"
#include "../UI/widgets/ButtonTextWidget.hpp"

class CalendarApplication : public TemplateApplication {
    private:
        unsigned long nextRefresh=0;
        int year=0;
        int monthDay=0;
        int monthToShow=0;
        ButtonTextWidget *nextBtn=nullptr;
        ButtonTextWidget *lastBtn=nullptr;
    public:
        const char *AppName() override { return "Calendar"; };
        CalendarApplication();
        ~CalendarApplication();
        bool Tick();
};

#endif

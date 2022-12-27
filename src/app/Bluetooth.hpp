#ifndef __LUNOKIOT__BLUETOOTH_APP__
#define __LUNOKIOT__BLUETOOTH_APP__
#include <Arduino.h>
#include <LilyGoWatch.h>
#include "../UI/AppTemplate.hpp"

#include "../UI/widgets/ButtonImageXBMWidget.hpp"

class BluetoothApplication: public TemplateApplication {
    private:
        unsigned long nextRedraw=0;
        uint32_t generatedPin=-1;
    public:
        ButtonImageXBMWidget * btnGeneratePIN = nullptr;
        ButtonImageXBMWidget * btnRemoveBonding = nullptr;
        uint32_t SetBLERandomPin();
        BluetoothApplication();
        ~BluetoothApplication();
        bool Tick();
};

#endif

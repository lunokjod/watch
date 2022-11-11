#include <Arduino.h>
#include "LogView.hpp"
#include "../UI/AppTemplate.hpp"
#include "Theme.hpp"
#include "../lunokiot_config.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"

ThemeApplication::ThemeApplication() {
    // Init here any you need
    imageBase = new CanvasWidget(canvas->height(),canvas->width());
    defaultThemeBtn = new ButtonWidget(0,0,80,80,[](){
        currentColorPalette = DefaultThemeColorPalette;
        currentThemeScheme = DefaultThemeScheme;
        lAppLog("Default theme applied\n");
    },ThCol(button),false);
    blueThemeBtn = new ButtonWidget(80,0,80,80,[]() {
        currentColorPalette = BlueThemeColorPalette;
        currentThemeScheme = BlueThemeScheme;
        lAppLog("Blue theme applied\n");
    },ThCol(button),false);


    // Remember to dump anything to canvas to get splash screen
    Tick();
}

ThemeApplication::~ThemeApplication() {
    // please remove/delete/free all to avoid leaks!
    if ( nullptr != imageBase ) { delete imageBase; }
    if ( nullptr != defaultThemeBtn ) { delete defaultThemeBtn; }
    if ( nullptr != blueThemeBtn ) { delete blueThemeBtn; }
}

bool ThemeApplication::Tick() {
    // put your interacts here
    defaultThemeBtn->Interact(touched,touchX,touchY);
    blueThemeBtn->Interact(touched,touchX,touchY);
    btnBack->Interact(touched,touchX,touchY); // from template
    
    if ( millis() > nextRefresh ) {
        imageBase->canvas->fillSprite(CanvasWidget::MASK_COLOR);
        canvas->fillSprite(ThCol(background));

        // draw your interface here!!!
        defaultThemeBtn->canvas->fillSprite(CanvasWidget::MASK_COLOR);
        defaultThemeBtn->canvas->fillCircle(40,40,30,canvas->color24to16(DefaultThemeColorPalette.color0));
        defaultThemeBtn->canvas->fillCircle(20,20,9,canvas->color24to16(DefaultThemeColorPalette.color1));
        defaultThemeBtn->canvas->fillCircle(40,20,9,canvas->color24to16(DefaultThemeColorPalette.color2));
        defaultThemeBtn->canvas->fillCircle(60,20,9,canvas->color24to16(DefaultThemeColorPalette.color3));
        defaultThemeBtn->canvas->fillCircle(20,40,9,canvas->color24to16(DefaultThemeColorPalette.color4));
        defaultThemeBtn->canvas->fillCircle(20,60,9,canvas->color24to16(DefaultThemeColorPalette.color5));
        defaultThemeBtn->canvas->fillCircle(40,40,9,canvas->color24to16(DefaultThemeColorPalette.color6));
        defaultThemeBtn->canvas->fillCircle(40,60,9,canvas->color24to16(DefaultThemeColorPalette.color7));
        defaultThemeBtn->canvas->fillCircle(60,40,9,canvas->color24to16(DefaultThemeColorPalette.color8));
        defaultThemeBtn->canvas->fillCircle(60,60,9,canvas->color24to16(DefaultThemeColorPalette.color9));
        defaultThemeBtn->DrawTo(imageBase->canvas);

        blueThemeBtn->canvas->fillSprite(CanvasWidget::MASK_COLOR);
        blueThemeBtn->canvas->fillCircle(40,40,30,canvas->color24to16(BlueThemeColorPalette.color0));
        blueThemeBtn->canvas->fillCircle(20,20,9,canvas->color24to16(BlueThemeColorPalette.color1));
        blueThemeBtn->canvas->fillCircle(40,20,9,canvas->color24to16(BlueThemeColorPalette.color2));
        blueThemeBtn->canvas->fillCircle(60,20,9,canvas->color24to16(BlueThemeColorPalette.color3));
        blueThemeBtn->canvas->fillCircle(20,40,9,canvas->color24to16(BlueThemeColorPalette.color4));
        blueThemeBtn->canvas->fillCircle(20,60,9,canvas->color24to16(BlueThemeColorPalette.color5));
        blueThemeBtn->canvas->fillCircle(40,40,9,canvas->color24to16(BlueThemeColorPalette.color6));
        blueThemeBtn->canvas->fillCircle(40,60,9,canvas->color24to16(BlueThemeColorPalette.color7));
        blueThemeBtn->canvas->fillCircle(60,40,9,canvas->color24to16(BlueThemeColorPalette.color8));
        blueThemeBtn->canvas->fillCircle(60,60,9,canvas->color24to16(BlueThemeColorPalette.color9));
        blueThemeBtn->DrawTo(imageBase->canvas);

        imageBase->DrawTo(canvas,0,0,CanvasWidget::MASK_COLOR);
        btnBack->DrawTo(canvas); // from template

        nextRefresh=millis()+(1000/8); // 8 FPS is enought for GUI
        return true;
    }
    return false;
}

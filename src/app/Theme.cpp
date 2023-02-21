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

#include <Arduino.h>
#include "LogView.hpp"
#include "../UI/AppTemplate.hpp"
#include "Theme.hpp"
#include "../lunokiot_config.hpp"
#include "../UI/widgets/ButtonImageXBMWidget.hpp"
#include <ArduinoNvs.h>

#include "../UI/themes/all_themes.hpp"

extern const TemplateThemeScheme * currentThemeScheme;

ThemeApplication::ThemeApplication() {
    // Init here any you need
    imageBase = new CanvasWidget(canvas->height(),canvas->width());
    int x = 0;
    int y = 0;
    const size_t totalThemes = sizeof(AllThemes)/sizeof(AllThemes[0]);

    for (size_t i=0;i<MAXThemes;i++) {
        if ( i >= totalThemes ) { break; }
        themeButton[i] = new ButtonWidget(x,y,80,80,[&,i](void *unused){
            currentColorPalette = &AllColorPaletes[i];
            currentThemeScheme = &AllThemes[i];
            lAppLog("Theme %d applied\n", i);
            NVS.setInt("lWTheme",i,false);
            /**
            for (int a=0;a<currentColorPalette->size;a++) {
                lAppLog("Theme Color: %d is 0x%x\n", a, currentColorPalette->colors[a]);
            }
            lAppLog("ATheme Scheme: background is 0x%x\n", AllThemes[i].background);
            lAppLog("ATheme Scheme: min is 0x%x\n", AllThemes[i].min);
            lAppLog("Theme Scheme: background is 0x%x\n", currentThemeScheme->background);
            lAppLog("Theme Scheme: min is 0x%x\n", currentThemeScheme->min);
            */
        },ThCol(button),false);
        x+=80;
        if (x>160) {
            x=0;
            y+=80;
            if ( y>=160 ) { x+=80; } // dont block "back" button
        }
    }

    // Remember to dump anything to canvas to get splash screen
    Tick();
}

ThemeApplication::~ThemeApplication() {
    // please remove/delete/free all to avoid leaks!
    if ( nullptr != imageBase ) { delete imageBase; }
    for (int i=0;i<MAXThemes;i++) {
        if ( nullptr != themeButton[i] ) { delete themeButton[i]; }
    }
}

bool ThemeApplication::Tick() {
    // put your interacts here
    for (int i=0;i<MAXThemes;i++) {
        if ( nullptr != themeButton[i] ) { themeButton[i]->Interact(touched,touchX,touchY); }
    }
    btnBack->Interact(touched,touchX,touchY); // from template
    
    if ( millis() > nextRefresh ) {
        imageBase->canvas->fillSprite(CanvasWidget::MASK_COLOR);
        canvas->fillSprite(ThCol(background));

        // draw your interface here!!!
        for (int i=0;i<MAXThemes;i++) {
            if ( nullptr != themeButton[i] ) {
                themeButton[i]->Interact(touched,touchX,touchY);

                themeButton[i]->canvas->fillSprite(CanvasWidget::MASK_COLOR);
                themeButton[i]->canvas->fillCircle(40,40,30,canvas->color24to16(AllColorPaletes[i].colors[0]));
                int offColor = 0;
                for(int y=20;y<=60;y+=20) {
                    for(int x=20;x<=60;x+=20) {
                        offColor++;
                        if ( offColor >= AllColorPaletes[i].size ) { break; }
                        themeButton[i]->canvas->fillCircle(x,y,9,canvas->color24to16(AllColorPaletes[i].colors[offColor]));
                    }
                }

                themeButton[i]->DrawTo(imageBase->canvas);
            }
        }

        imageBase->DrawTo(canvas,0,0,CanvasWidget::MASK_COLOR);
        btnBack->DrawTo(canvas); // from template

        nextRefresh=millis()+(1000/8); // 8 FPS is enought for GUI
        return true;
    }
    return false;
}

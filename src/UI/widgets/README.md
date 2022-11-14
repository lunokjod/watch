# lunokIoTWatch Widgets

Description about lunokWatch widgets for app developers
This widgets bring to developer to use a conformed user interface
Adding rich interface for user interaction made easy, getting values and receiving events

## Canvas

Canvas is a base widget, their only function is show their contents on the screen, almost all widgets are based on this

Usage (on Application constructor):

```CanvasWidget *canvasCreated = new CanvasWidget(int16_t heightInPixels, int16_t widthInPixels);```

on Application::Tick must be call draw

```canvasCreated->DrawTo(canvas, int16_t xInPixels, int16_t yInPixels,int32_t maskColor=MASK_COLOR);```

on (Application destructor)

```delete canvasCreated;```

[Code](CanvasWidget.hpp)

## CanvasZ

Canvas with zoom capability (can be scaled)

Creation:

```CanvasZWidget *canvasZCreated = new CanvasZWidget(int16_t heightInPixels, int16_t widthInPixels, float scale=1.0);```

Draw:

```canvasZCreated->DrawTo(Canvas, int16_t xInPixels, int16_t yInPixels, float scale, bool centered, int32_t maskColor);```

on (Application destructor):

```delete canvasZCreated;```

[Code](CanvasZWidget.hpp)


## Button

This is the first widget that is reactive (do something when is pushed) in combination of [Canvas](CanvasWidget.hpp) and [ActiveRect](../activator/ActiveRect.hpp)

Creation:

```
ButtonWidget *buttonCreated = new ButtonWidget(int16_t xInPixels,int16_t yInPixels, int16_t heightInPixels, int16_t widthInPixels, []() {

    /* put here code to run when button is pushed */

}, uint32_t btnBackgroundColor, bool borders);
```

Draw:

```buttonCreated->DrawTo(canvas);```

on (Application destructor):

```delete buttonCreated;```

[Code](ButtonWidget.hpp)


## ButtonImage

Draw a button with a image centered on it (xbm format) more info: https://github.com/MichaelMure/gimp-plugins/blob/master/common/file-xbm.c

* note this can be exported easily from gimp using ending ".xbm" file name

```
ButtonImageXBMWidget *imgButton = ButtonImageXBMWidget(int16_t x,int16_t y, int16_t h, int16_t w, []() {

    /* put here code to run when button is pushed */

}, const uint8_t *XBMBitmapPtr, int16_t xbmH, int16_t xbmW, uint16_t xbmColor=ThCol(text), uint32_t btnBackgroundColor=ThCol(button), bool borders=true);
```

Draw:

```imgButton->DrawTo(canvas);```

on (Application destructor):

```delete imgButton;```

[Code](ButtonImageXBMWidget.hpp)


@TODO explain: Switch, Gauge, Graph, ValueSelector
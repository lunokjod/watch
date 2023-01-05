# lunokIoTWatch Widgets

Description about lunokWatch widgets for app developers
This widgets bring to developer to use a conformed user interface
Adding rich interface for user interaction made easy, getting values and receiving events

Application can hold widgets in their propertires, use private/protected/public in a class definition (.hpp file) to define different widgets

## Canvas

Canvas is a base widget, their only function is show their contents on the screen, almost all widgets are based on this. cannot receive events

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
ButtonWidget *buttonCreated = new ButtonWidget(int16_t xInPixels,int16_t yInPixels, int16_t heightInPixels, int16_t widthInPixels, [](void *caller) {

    /* put here code to run when button is pushed */

}, uint32_t btnBackgroundColor, bool borders);
```

Interact:

```buttonCreated->Interact(touched,touchX,touchY);```


Draw:

```buttonCreated->DrawTo(canvas);```

on (Application destructor):

```delete buttonCreated;```

[Code](ButtonWidget.hpp)


## ButtonImage

Draw a button with a image centered on it (xbm format) more info: https://github.com/MichaelMure/gimp-plugins/blob/master/common/file-xbm.c

* note this can be exported easily from gimp using ending ".xbm" file name

```
ButtonImageXBMWidget *imgButton = ButtonImageXBMWidget(int16_t x,int16_t y, int16_t h, int16_t w, [](void *caller) {

    /* put here code to run when button is pushed */

}, const uint8_t *XBMBitmapPtr, int16_t xbmH, int16_t xbmW, uint16_t xbmColor=ThCol(text), uint32_t btnBackgroundColor=ThCol(button), bool borders=true);
```

Interact:

```imgButton->Interact(touched,touchX,touchY);```


Draw:

```imgButton->DrawTo(canvas);```

on (Application destructor):

```delete imgButton;```

[Code](ButtonImageXBMWidget.hpp)


## Switch

Create a switch (true/false) value on screen

```
SwitchWidget *switchCreated = new SwitchWidget(int16_t x, int16_t y, [](void *caller) {
    // receive callback when switch pushed
}, uint32_t switchColor=ThCol(button));
```

Interact:

```switchCreated->Interact(touched,touchX,touchY);```


Draw:

```switchCreated->DrawTo(canvas);```

Get the current value are stored in:

```bool currentSwitchValue = switchCreated->switchEnabled;```


on (Application destructor):

```delete switchCreated;```


[Code](SwitchWidget.hpp)


## Gauge

This widget is a circular selector value (from 0 to 360) the activeArea for this widget are radial (not a rect)

Creation:

```
GaugeWidget *myGauge = new GaugeWidget(int16_t x, int16_t y, int16_t d=100, uint32_t wgaugeColor=ThCol(button), uint32_t markColor=ThCol(middle), uint32_t needleColor=ThCol(highlight), uint32_t incrementColor=ThCol(max));
```

Interact:

```myGauge->Interact(touched,touchX,touchY);```

Draw:

```myGauge->DrawTo(canvas);```

Get the current value are stored in:

```int16_t value = myGauge->selectedAngle;```


on (Application destructor):

```delete myGauge;```


[Code](GaugeWidget.hpp)


## Gaph

This widget allow to draw graphs with min/max value

Definition:

```
GraphWidget oneGraph = GraphWidget(int16_t h=100, int16_t w=220, int64_t minValue=0, int64_t maxValue=100, uint32_t markColor=TFT_YELLOW, uint32_t backgroundColor=CanvasWidget::MASK_COLOR, uint32_t outColor=TFT_RED);
```

Draw:

```oneGraph->DrawTo(canvas, int16_t x=0, int16_t y=0);```

Add values:

```bool oneGraph->PushValue(int64_t value);```

on (Application destructor):

```delete oneGraph;```

[Code](GraphWidget.hpp)


## ValueSelector

This widget allow to get a value from a range

```
ValueSelector * getMyValue = new ValueSelector(int16_t x,int16_t y, int16_t h, int16_t w,int32_t valMin=0,int32_t valMax=100,uint32_t backgroundColor=ThCol(background),bool showsign=false);
```

Interact:

```getMyValue->Interact(touched,touchX,touchY);```

Draw:

```getMyValue->DrawTo(canvas);```

Get the current value are stored in:

```int32_t value = getMyValue->selectedValue;```


on (Application destructor):

```delete getMyValue;```


[Code](ValueSelector.hpp)

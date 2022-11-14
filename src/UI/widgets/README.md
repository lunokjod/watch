# lunokIoTWatch Widgets

Description about lunokWatch widgets for app developers

## Canvas

Canvas is a base widget, their only function is show their contents on the screen

Usage (on Application constructor):

```CanvasWidget(int16_t heightInPixels, int16_t widthInPixels);```

on Application::Tick must be call draw

```DrawTo(canvas, int16_t X, int16_t Y,int32_t maskColor=MASK_COLOR);```

on (Application destructor)

```delete canvasCreated;```

[Code](CanvasWidget.hpp)


@TODO explain: CanvasZ, Button, ButtonImage, Switch, Gauge, Graph, ValueSelector
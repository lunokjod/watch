# lunokIoTWatch Widgets

Description about lunokWatch widgets for app developers
This widgets bring to developer to use a conformed user interface
Adding rich interface for user interaction made easy, getting values and receiving events

## Canvas

Canvas is a base widget, their only function is show their contents on the screen, almost all widgets are based on this

Usage (on Application constructor):

```new CanvasWidget(int16_t heightInPixels, int16_t widthInPixels);```

on Application::Tick must be call draw

```DrawTo(canvas, int16_t xInPixels, int16_t yInPixels,int32_t maskColor=MASK_COLOR);```

on (Application destructor)

```delete canvasCreated;```

[Code](CanvasWidget.hpp)

## CanvasZ

Canvas with zoom capability (can be scaled)

Creation:

```new CanvasZWidget(int16_t heightInPixels, int16_t widthInPixels, float scale=1.0);```

Draw:

```DrawTo(Canvas, int16_t xInPixels, int16_t yInPixels, float scale, bool centered, int32_t maskColor);```

on (Application destructor):

```delete canvasZCreated;```

[Code](CanvasZWidget.hpp)

@TODO explain: Button, ButtonImage, Switch, Gauge, Graph, ValueSelector
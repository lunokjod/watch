# Activators

Is a class that catch get events (and filter) to send to the upper objects

+ No visible, only reacts (touch, gyro, gpio events, mouse, joystick, keyboard...)
+ Must launch the callback when event occurs

ActiveRect can detect touch in a area or radius to launch callback (is the base for all reactable widgets)
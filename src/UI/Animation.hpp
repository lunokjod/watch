#ifndef __LUNOKIOT__ANIMATION__
#define __LUNOKIOT__ANIMATION__

/*
 * https://www.geeksforgeeks.org/2d-transformation-rotation-objects/ READ THIS!!!
 * https://www.geeksforgeeks.org/2d-transformation-in-computer-graphics-set-1-scaling-of-objects/?ref=lbp
 * https://www.geeksforgeeks.org/bresenhams-circle-drawing-algorithm/?ref=lbp
 * https://www.geeksforgeeks.org/mid-point-circle-drawing-algorithm/?ref=lbp
 */ 
#include <Arduino.h>
#include <LilyGoWatch.h>

#include "lunokiot_config.hpp"

class AnimationDescriptor {
    public:
        bool done = false;
        int32_t fromX = 0;
        int32_t fromY = 0;
        int32_t toX = 0;
        int32_t toY = 0;
        int32_t currentX = 0;
        int32_t currentY = 0;
        int32_t stepSize = 5;

        AnimationDescriptor(int32_t fromX,int32_t fromY,int32_t toX, int32_t toY, int32_t stepSize);
        void Step();
        void Draw();
};

#endif

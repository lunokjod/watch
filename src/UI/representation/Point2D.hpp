#ifndef ___LUNOKIOT__ARTIFACTS_2DPOINT___HEADER___
#define ___LUNOKIOT__ARTIFACTS_2DPOINT___HEADER___
/*
 * This class represents a 2D point on a plain surface (like screen)
 */
#include <Arduino.h>
//#include <LilyGoWatch.h>
#include "lunokiot_config.hpp"

class Point2D {
    private: //@TODO why private? this class must have getter, setter and callback in case of change (KVO stykle) 
        int16_t x=0;
        int16_t y=0;
        int16_t last_x=0;
        int16_t last_y=0;
    public:
        void GetDelta(int16_t &dx, int16_t &dy);
        void Set(int16_t px, int16_t py);
        void Get(int16_t &px, int16_t &py);
        Point2D(int16_t x=0,int16_t y=0);
};

#endif

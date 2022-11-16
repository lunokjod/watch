#include "Animation.hpp"
#include <libraries/TFT_eSPI/TFT_eSPI.h>
extern TFT_eSPI * tft;

AnimationDescriptor::AnimationDescriptor(int32_t fromX,int32_t fromY,int32_t toX, int32_t toY, int32_t stepSize)
                            : fromX(fromX), fromY(fromY),toX(toX),toY(toY),stepSize(stepSize)  {
                                currentX = fromX;
                                currentY = fromY;
}

void AnimationDescriptor::Step() {
    Serial.printf("DEBUG Step: ");
    int32_t incrementX = 0;
    int32_t incrementY = 0;

    if ( currentX < toX ) {
        incrementX=stepSize;
    } else if ( currentX > toX ) {
        incrementX=(stepSize*-1);
    }

    if ( currentY < toY ) {
        incrementY=stepSize;
    } else if ( currentY > toY ) {
        incrementY=(stepSize*-1);
    }
    currentX+=incrementX;
    currentY+=incrementY;
    Serial.printf("incrementX: %d", incrementX);
    Serial.printf(" incrementY: %d\n", incrementY);

    Serial.printf("  CurrentX: %d", currentX);
    Serial.printf(" CurrentY: %d\n", currentY);
}

void AnimationDescriptor::Draw() {
    tft->fillRoundRect(currentX,currentY,35,35,5,0x5DFC);
}

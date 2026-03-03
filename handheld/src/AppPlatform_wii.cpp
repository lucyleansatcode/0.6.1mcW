#include "AppPlatform_wii.h"

int AppPlatform_wii::getScreenWidth() {
    return 640;
}

int AppPlatform_wii::getScreenHeight() {
    return 480;
}

float AppPlatform_wii::getPixelsPerMillimeter() {
    return 4.0f;
}

bool AppPlatform_wii::supportsTouchscreen() {
    return false;
}

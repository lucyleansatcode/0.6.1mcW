#ifndef APPPLATFORM_WII_H__
#define APPPLATFORM_WII_H__

#include "AppPlatform.h"

class AppPlatform_wii : public AppPlatform {
public:
    AppPlatform_wii() = default;

    int getScreenWidth() override;
    int getScreenHeight() override;
    float getPixelsPerMillimeter() override;
    bool supportsTouchscreen() override;
};

#endif

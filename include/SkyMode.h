#ifndef SKYMODE_H
#define SKYMODE_H

#include <Mode.h>

class SkyMode : public Mode
{
private:
    bool edit_mode;
    int sunrise_point;
    int sunset_point;
    float speed;

    unsigned long sunrise_start_time;
    unsigned long last_time = 0UL;

    float SecondsSinceSunriseStart = 0.0F;

    void ShowEditMode(CRGB *);
    void ShowSunriseLight(float, CRGB *);
    void ShowSunriseSun(float, CRGB *);
    void TiltColors(float, CRGB *);

    CRGB SUN_COLOR;
    CRGB SKY_COLOR;

    float skyHue;
    float skyValue;

public:
    void update(CRGB *);
    void update_args(const char *);
    void update_arg(String arg, String value);
    SkyMode();
    SkyMode(const char *);
    ~SkyMode();
};

#endif
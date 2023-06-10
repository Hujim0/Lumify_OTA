#ifndef RAINBOWMODE_H
#define RAINBOWMODE_H

#include <Mode.h>

class RainbowMode : public Mode
{
private:
    float hue;
    bool reversed;
    float hueConst;

    float speed;
    int count;

public:
    void update(CRGB *);
    void update_args(const char *data);
    void update_arg(String arg, String value);
    RainbowMode();
    RainbowMode(const char *);
    ~RainbowMode();
};

#endif
#ifndef WAVEMODE_H
#define WAVEMODE_H

#include <Mode.h>

class WaveMode : public Mode
{
private:
    CRGB color;

    float offset;

    float speed;
    float length;
    float intensity;
    bool reversed;

public:
    void setup(CRGB *);
    void update(CRGB *);
    void update_args(const char *);
    void update_arg(String arg, String value);
    WaveMode();
    WaveMode(const char *);
    ~WaveMode();
};

#endif
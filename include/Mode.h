#ifndef MODE_H
#define MODE_H

#include <FastLED.h>
#include <main.h>
#include <ArduinoJson.h>

#define SPEED_ARG "speed"
#define INTENSITY_ARG "intensity"
#define LENGTH_ARG "length"
#define REVERSED_ARG "reversed"
#define COUNT_ARG "count"
#define EDIT_ARG "edit"
#define START_ARG "start"
#define END_ARG "end"
#define COLOR_ARG "color"

#define STATIC_DOCUMENT_MEMORY_SIZE 512

class Mode
{
public:
    // pure virtual method
    virtual void update(CRGB *) = 0;
    virtual void update_args(const char *data) = 0;
    virtual void update_arg(String arg, String value) = 0;

    static uint32_t toHex(String);
    static float removeNegatives(float);
    static int removeNegatives(int);
    static void printCRGB(CRGB);
    virtual ~Mode() {}
};

#endif
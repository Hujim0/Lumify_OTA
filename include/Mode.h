#pragma once

#include <FastLED.h>
#include <main.h>
#include <ArduinoJson.h>
#include <FileSystem.h>

class Mode
{
public:
    const char *SPEED_ARG = "speed";
    const char *INTENSITY_ARG = "intensity";
    const char *LENGTH_ARG = "length";
    const char *REVERSED_ARG = "reversed";
    const char *COUNT_ARG = "count";
    const char *EDIT_ARG = "edit";
    const char *START_ARG = "start";
    const char *END_ARG = "end";
    const char *COLOR_ARG = "color";

    // pure virtual method
    virtual void update(CRGB *) = 0;
    virtual void update_args(const char *data) = 0;
    virtual void update_arg(const char *arg, const char *value) = 0;

    static uint32_t toHex(const char *);
    static float removeNegatives(float);
    static int removeNegatives(int);
    static void printCRGB(CRGB);
    virtual ~Mode() {}
};
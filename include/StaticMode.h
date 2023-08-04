#pragma once

#include <Mode.h>

class StaticMode : public Mode
{
private:
    CRGB color;
    // StaticJsonDocument<STATIC_DOCUMENT_MEMORY_SIZE> args;

public:
    void update(CRGB *);
    void update_args(const char *data);
    void update_arg(String arg, String value);
    StaticMode();
    StaticMode(const char *);
    ~StaticMode();
};
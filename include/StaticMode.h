#ifndef STATICMODE_H
#define STATICMODE_H

#include <Mode.h>

class StaticMode : public Mode
{
private:
    CRGB color;
    // StaticJsonDocument<STATIC_DOCUMENT_MEMORY_SIZE> args;

public:
    void update(CRGB *);
    void update_args(const char *data);
    void update_arg(const char *arg, const char *value);
    StaticMode();
    StaticMode(const char *);
    ~StaticMode();
};

#endif
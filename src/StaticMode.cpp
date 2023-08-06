#include <StaticMode.h>

void StaticMode::update(CRGB *leds)
{
    for (int i = 0; i < NUMPIXELS; i++)
    {
        leds[i] = color;
    }
}

StaticMode::StaticMode()
{
    color = CRGB::White;
}

StaticMode::StaticMode(const char *data)
{
    update_args(data);
}

void StaticMode::update_args(const char *data)
{
    StaticJsonDocument<STATIC_DOCUMENT_MEMORY_SIZE> args;
    deserializeJson(args, data);

    // color = toHex(args[COLOR_ARG].as<const char *>());
    color = CRGB::White;

    args.garbageCollect();
}

void StaticMode::update_arg(const char *arg, const char *value)
{
    if (strcmp(arg, COLOR_ARG))
        color = toHex(value);
}
StaticMode::~StaticMode() {}
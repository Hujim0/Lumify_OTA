#include <WaveMode.h>

void WaveMode::update(CRGB *leds)
{
    for (int i = 0; i < NUMPIXELS; i++)
    {
        float multiplier = sin((i * (length)) + offset);

        if (multiplier <= 0.01F)
        {
            leds[i] = color;
            continue;
        }

        float intensity_const = 1.0F - (multiplier * intensity);

        leds[i] = CRGB(color.r * intensity_const,
                       color.g * intensity_const,
                       color.b * intensity_const);
    }
    if (abs(offset) >= TWO_PI * 10)
    {
        offset = 0;
        return;
    }
    offset += speed * (((length + 1.0F) * 2) / 255.0F); // to make it the same as in rainbow mode
}

WaveMode::WaveMode(const char *data)
{
    update_args(data);
}

void WaveMode::update_args(const char *data)
{
    StaticJsonDocument<STATIC_DOCUMENT_MEMORY_SIZE> args;
    deserializeJson(args, data);

    color = toHex(args[COLOR_ARG].as<String>());

    speed = args[SPEED_ARG].as<float>();
    intensity = args[INTENSITY_ARG].as<float>() * 0.01F;
    if (args[LENGTH_ARG].as<float>() == 0.0F)
    {
        length = 0.0F;
    }
    else
    {
        length = PI / (args[LENGTH_ARG].as<float>());
    }

    reversed = args[REVERSED_ARG].as<bool>();

    if (!reversed)
    {
        speed = abs(speed) * -1.0F;
    }

    args.garbageCollect();
}

void WaveMode::update_arg(String arg, String value)
{
    if (arg == SPEED_ARG)
    {
        speed = value.toFloat();
        if (!reversed)
        {
            speed *= -1.0F;
        }
    }
    else if (arg == INTENSITY_ARG)
        intensity = value.toFloat() * 0.01F;
    else if (arg == COLOR_ARG)
        color = toHex(value);
}

WaveMode::WaveMode()
{
    color = CRGB::White;

    speed = 10;
    intensity = 90;
    length = 10;

    reversed = false;
}

WaveMode::~WaveMode()
{
    free(&offset);
}

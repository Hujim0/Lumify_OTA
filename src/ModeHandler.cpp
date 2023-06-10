#include <ModeHandler.h>
#include <StaticMode.h>
#include <RainbowMode.h>
#include <WaveMode.h>
#include <SkyMode.h>

void ModeHandler::LightSwitch(bool state)
{
    if (led_state == state)
        return;

    led_state = state;

    if (!state)
    {
        last_brigthness = FastLED.getBrightness();

        FastLED.clearData();
        FastLED.setBrightness(0);
        return;
    }

    FastLED.setBrightness(last_brigthness);
}

void ModeHandler::ChangeMode(int id, const char *args)
{
    FastLED.clearData();

    current_mode_id = id;

    switch (id)
    {
    case 0:
        current_mode = new StaticMode(args);
        return;
    case 1:
        current_mode = new RainbowMode(args);
        return;
    case 2:
        current_mode = new WaveMode(args);
        return;
    case 3:
        current_mode = new SkyMode(args);
        return;

    default:
        ChangeMode(0);
        return;
    }
}
void ModeHandler::ChangeMode(int id)
{
    FastLED.clearData();

    current_mode_id = id;

    switch (id)
    {
    case 0:
        current_mode = new StaticMode();
        return;
    case 1:
        current_mode = new RainbowMode();
        return;
    case 2:
        current_mode = new WaveMode();
        return;
    case 3:
        current_mode = new SkyMode();
        return;

    default:
        ChangeMode(0);
        break;
    }
}

void ModeHandler::update(CRGB *leds)
{
    if (current_mode != NULL && led_state)
        current_mode->update(leds);
}
void ModeHandler::UpdateArgs(const char *data)
{
    if (current_mode != NULL)
        current_mode->update_args(data);
}
void ModeHandler::PushArg(String arg, String value)
{
    if (current_mode != NULL)
        current_mode->update_arg(arg, value);
}

void ModeHandler::ChangeBrightness(int value)
{
    FastLED.setBrightness(value);
}

ModeHandler::ModeHandler()
{
}

ModeHandler::~ModeHandler()
{
}
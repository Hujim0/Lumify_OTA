#pragma once

#include <Mode.h>

#define LIGHT_SWITCH "light_switch"
#define MODE_SWITCH "mode_switch"
#define MODE_UPDATE "mode_update"
#define BRIGHTNESS "brightness"
#define ARGS_REQUEST "mode_args_request"
#define STREAM_OPEN "open_stream"
#define STREAM_CLOSE "close_stream"
#define EPOCH_TIME "epoch_time"

class ModeHandler
{
private:
    Mode *current_mode = NULL;

    uint8_t last_brightness = 0;

public:
    int current_mode_id;
    bool led_state = true;
    void LightSwitch(bool);
    void ChangeBrightness(int);
    void ChangeMode(int, const char *);
    void UpdateArgs(const char *);
    void PushArg(const char *arg, const char *data);
    void update(CRGB *);
    ModeHandler();
    ~ModeHandler();
};
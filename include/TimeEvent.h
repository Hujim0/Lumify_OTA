#pragma once

#include <Arduino.h>

enum EventType
{
    Brightness = 0,
    ModeSwitch = 1
};

class TimeEvent
{
public:
    bool CheckTime(int /*, int*/);

    bool Equals(int epoch, EventType eventType);
    int epochTime;
    // bool *daysOfTheWeek;

    EventType eventType;
    int value;
    float transition;
    const char *args;

    const char *stringify();

    TimeEvent(int epoch, float transition, EventType type, int value, const char *args);
    TimeEvent();
};
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

    bool Equals(int epoch, int event_Type);
    bool Equals(int epoch, EventType eventType);
    int epochTime;
    // bool *daysOfTheWeek;

    EventType eventType;
    int value;
    float transition;
    String args;

    String stringify();

    TimeEvent(int epoch, float transition, EventType type, int value, String args);
    TimeEvent();
};
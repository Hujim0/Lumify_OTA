#ifndef TIMEEVENT_H
#define TIMEEVENT_H

#include <Arduino.h>
#include <ModeHandler.h>

enum EventType
{
    Brightness = 0,
    ModeSwitch = 1
};

class TimeEvent
{
public:
    bool CheckTime(ModeHandler *, int /*, int*/);

    bool Equals(int, int);
    bool Equals(int, EventType);
    int epochTime;
    // bool *daysOfTheWeek;

    int id = 0;

    EventType eventType;
    int value;
    const char *args;

    String stringify();

    TimeEvent(int, /*bool, */ EventType, int, String);
    TimeEvent();
};

#endif
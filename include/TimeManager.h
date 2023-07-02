#ifndef TIMEMANAGER_H
#define TIMEMANAGER_H

#include <Arduino.h>
#include <ModeHandler.h>
#include <TimeEvent.h>
#include <ESP_LinkedList.h>

typedef std::function<void()> FiveSecondTimer;

class TimeManager
{
private:
    ModeHandler *modeHandler;

    int epoch_time_day_seconds = 0;
    unsigned long MillisOffset = 0UL;
    int dayOfTheWeek = 0;
    ESP_LinkedList<TimeEvent> timeEvents;
    int timeEventsCounter = 0;

    int FiveSecondCounter = 0;
    bool isSetuped = false;

    int _seconds;
    void UpdateSeconds();
    int _minuts;
    void UpdateMinuts();
    int _hours;
    void UpdateHours();

public:
    void Setup(ModeHandler *, int epoch_time_day_seconds, int dayOfTheWeek);
    void Update();
    String GetFormattedTime();

    FiveSecondTimer timer;

    void AddTimeEvent(TimeEvent handler);
    void RemoveTimeEvent(int epoch_time, int event_type);
    void RemoveLastTimeEvent();

    TimeManager();

    static TimeManager *Instance;
};

#endif
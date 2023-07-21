#ifndef TIMEMANAGER_H
#define TIMEMANAGER_H

#include <Arduino.h>
#include <ModeHandler.h>
#include <TimeEvent.h>
#include <ESP_LinkedList.h>

typedef std::function<void()> OneMinuteTimer;
typedef std::function<void(float, EventType, int, String)> OnEventFired;

class TimeManager
{
private:
    int epoch_time_day_seconds = 0;
    unsigned long MillisOffset = 0UL;
    int dayOfTheWeek = 0;
    ESP_LinkedList<TimeEvent> timeEvents;
    int timeEventsCounter = 0;
    OnEventFired onEventFired;

    // int OneMinuteCounter = 0;
    bool isSetuped = false;

    int seconds;
    void UpdateSeconds();
    int minutes;
    void UpdateMinutes();
    int hours;
    void UpdateHours();

    

public:
    void setOnEventFiredEvent(OnEventFired);
    void InvokeOnEventFired(float, EventType, int, String);
    void Setup(int epoch_time_day_seconds, int dayOfTheWeek);
    void Setup(int epoch_time_seconds, int _dayOfTheWeek, StaticJsonDocument<STATIC_DOCUMENT_MEMORY_SIZE> &preferences);
    void Update();
    String GetCurrentFormattedTime();
    int GetEpochTime();
    static String FormatTime(int);

    OneMinuteTimer timer;

    void AddTimeEvent(TimeEvent handler);
    void CleanTimeEvents();

    TimeManager();

    static TimeManager *Instance;
};

#endif
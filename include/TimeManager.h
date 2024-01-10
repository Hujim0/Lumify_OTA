#ifndef TIMEMANAGER_H
#define TIMEMANAGER_H

#include <Arduino.h>
#include <ModeHandler.h>
#include <TimeEvent.h>

typedef std::function<void()> Timer;
typedef std::function<void(float, EventType, int, const char *)> OnEventFired;

class TimeManager
{
private:
    int epoch_time_day_seconds = 0;
    unsigned long MillisOffset = 0UL;
    int dayOfTheWeek = 0;
    TimeEvent *timeEvents = NULL;
    size_t timeEventsCounter = 0;
    OnEventFired onEventFired;

    // int OneMinuteCounter = 0;
    bool isReady = false;

    uint8_t seconds;
    void UpdateSeconds();
    uint8_t minutes;
    void UpdateMinutes();
    uint8_t hours;
    void UpdateHours();

public:
    void setOnEventFiredEvent(OnEventFired);
    void InvokeOnEventFired(float transition, EventType type, int value, const char *args);
    void Setup(int epoch_time_day_seconds, int dayOfTheWeek);
    void Setup(int epoch_time_seconds, int _dayOfTheWeek, StaticJsonDocument<STATIC_DOCUMENT_MEMORY_SIZE> &preferences);
    void Update();
    int GetEpochTime();

    String GetCurrentFormattedTime();
    static String FormatTime(int);

    Timer OneMinuteTimer;
    Timer TenSecondsTimer;

    void AddTimeEvents(TimeEvent *events, size_t count);
    void AddTimeEventsFromJson(JsonVariant doc);
    void CleanTimeEvents();

    TimeManager();

    static TimeManager *Instance;
};

#endif
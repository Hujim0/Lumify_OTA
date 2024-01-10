#include <global.h>

#include <TimeManager.h>
#include <TimeEvent.h>

#define LOG_PREFIX "[TimeManager] "

TimeManager *TimeManager::Instance = 0;

void TimeManager::Setup(int epoch_time_seconds, int _dayOfTheWeek, StaticJsonDocument<STATIC_DOCUMENT_MEMORY_SIZE> &preferences)
{

    preferences["last_epoch"] = epoch_time_seconds;
    preferences["last_day_of_the_week"] = _dayOfTheWeek;

    Setup(epoch_time_seconds, _dayOfTheWeek);
}

void TimeManager::Setup(int epoch_time_seconds, int _dayOfTheWeek)
{
    if (isReady)
    {
        MillisOffset = (unsigned long)(epoch_time_seconds * 1000) - millis();
        return;
    }

    Instance = this;

    dayOfTheWeek = _dayOfTheWeek;

    MillisOffset = (unsigned long)(epoch_time_seconds * 1000) - millis();

    hours = (epoch_time_seconds / 3600);
    minutes = (epoch_time_seconds % 3600) / 60;
    seconds = epoch_time_seconds % 60;

    isReady = true;

    {
        char msg[64];
        String time = GetCurrentFormattedTime();
        snprintf(msg, sizeof(msg), LOG_PREFIX "Got time! %s, day of the week: %i",
                 time,
                 dayOfTheWeek);
        time[0] = 0;

        sprintln(msg);
    }
}

void TimeManager::Update()
{
    if (!isReady)
        return;

    epoch_time_day_seconds = (int)(((MillisOffset + millis()) / 1000UL) % 86400UL);

    UpdateSeconds();
}

void TimeManager::UpdateHours()
{
    int timeHours = (epoch_time_day_seconds / 3600);

    if (hours == timeHours)
        return;

    hours = timeHours;

    // time events (hour update)
    // probably unnecessary
}

void TimeManager::UpdateMinutes()
{
    int timeMins = (epoch_time_day_seconds % 3600) / 60;

    if (minutes == timeMins)
        return;

    minutes = timeMins;

    UpdateHours();

    // time events (minute update)

    for (size_t i = 0; i < timeEventsCounter; i++)
    {
        bool result = timeEvents[i].CheckTime(epoch_time_day_seconds /*, dayOfTheWeek*/);

        if (!result)
            continue;

        {
            char msg[256];

            String event_stringified = timeEvents[i].stringify();

            snprintf(msg, sizeof(msg), LOG_PREFIX "TimeEvent executed: %s",
                     event_stringified.c_str());

            event_stringified[0] = 0;

            sprintln(msg);
        }
    }

    if (OneMinuteTimer != NULL)
    {
        OneMinuteTimer();
    }
}

void TimeManager::UpdateSeconds()
{
    int timeSecs = epoch_time_day_seconds % 60;

    if (timeSecs == seconds)
        return;

    seconds = timeSecs;

    if (seconds % 5 == 0)
    {
        if (TenSecondsTimer != NULL)
            TenSecondsTimer();
    }

    UpdateMinutes();
}

String TimeManager::GetCurrentFormattedTime()
{
    String res((char *)0);
    res.reserve(8);
    {
        char time[9];
        snprintf(time, sizeof(time), "%.2i:%.2i:%.2i", hours, minutes, seconds);

        res = time;
    }

    return res;
}

String TimeManager::FormatTime(int epoch)
{
    int _hours = epoch / 3600;
    int _minutes = (epoch % 3600) / 60;
    int _seconds = epoch % 60;

    String res((char *)0);
    res.reserve(8);
    {
        char time[9];
        snprintf(time, sizeof(time), "%.2i:%.2i:%.2i", _hours, _minutes, _seconds);

        res = time;
    }

    return res;
}

TimeManager::TimeManager() {}

void TimeManager::AddTimeEvents(TimeEvent *events, size_t count)
{
    timeEventsCounter = count;
    timeEvents = events;
}

void TimeManager::CleanTimeEvents()
{
    timeEvents = NULL;
}

void TimeManager::setOnEventFiredEvent(OnEventFired handler)
{
    onEventFired = handler;
}
void TimeManager::InvokeOnEventFired(float transition, EventType eventType, int value, const char *args)
{
    if (onEventFired != NULL)
        onEventFired(transition, eventType, value, args);
}

int TimeManager::GetEpochTime()
{
    return epoch_time_day_seconds;
}

void TimeManager::AddTimeEventsFromJson(JsonVariant json)
{
    JsonArray time_events_array_json = json["events"];

    size_t count = time_events_array_json.size();

    TimeEvent events[count];

    for (size_t i = 0; i < count; i++)
    {
        events[i] = TimeEvent(
            time_events_array_json[i]["epoch_time"].as<int>(),
            time_events_array_json[i]["transition"].as<float>(),
            (EventType)time_events_array_json[i]["event_type"].as<int>(),
            time_events_array_json[i]["value"].as<int>(),
            time_events_array_json[i]["args"].as<const char *>());

        char msg[256];

        {
            String event_stringified = events[i].stringify();
            snprintf(msg, sizeof(msg), LOG_PREFIX "Added %s", event_stringified.c_str());
            event_stringified[0] = 0;
        }

        sprintln(msg);
    }

    time_events_array_json.clear();

    AddTimeEvents(events, count);

    json.clear();
}

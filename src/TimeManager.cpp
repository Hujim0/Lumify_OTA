#include <global.h>

#include <TimeManager.h>
#include <TimeEvent.h>

const String LOG_PREFIX = "[TimeManager] ";

TimeManager *TimeManager::Instance = 0;

void TimeManager::Setup(ModeHandler *handler, int epoch_time_seconds, int _dayOfTheWeek)
{
    if (isSetuped)
        return;

    Instance = this;

    modeHandler = handler;

    dayOfTheWeek = _dayOfTheWeek;

    MillisOffset = (unsigned long)(epoch_time_seconds * 1000) - millis();

    hours = (epoch_time_seconds / 3600);
    minuts = (epoch_time_seconds % 3600) / 60;
    seconds = epoch_time_seconds % 60;

    isSetuped = true;
}

void TimeManager::Update()
{
    if (!isSetuped)
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

void TimeManager::UpdateMinuts()
{
    int timeMins = (epoch_time_day_seconds % 3600) / 60;

    if (minuts == timeMins)
        return;

    minuts = timeMins;

    UpdateHours();

    // time events (minute update)

    for (int i = 0; i < timeEvents.size(); i++)
    {
        bool result = timeEvents[i].CheckTime(modeHandler, epoch_time_day_seconds /*, dayOfTheWeek*/);

        if (!result)
            continue;

        sprintln(LOG_PREFIX + "TimeEvent executed: " + timeEvents[i].stringify());
    }

    if (timer != NULL)
    {
        timer();
    }
}

void TimeManager::UpdateSeconds()
{
    int timeSecs = epoch_time_day_seconds % 60;

    if (timeSecs == seconds)
        return;

    seconds = timeSecs;

    UpdateMinuts();

    // // time events (second update)

    // OneMinuteCounter++;

    // if (OneMinuteCounter == 5)
    // {
    //     OneMinuteCounter = 0;

    // }

    // sprintln(String(epoch_time_day_seconds) + " " + GetCurrentFormattedTime());
}

String TimeManager::GetCurrentFormattedTime()
{
    if (!isSetuped)
        return "unknown";

    String time = "";

    if (hours < 10)
        time += "0";
    time += String(hours) + ":";
    if (minuts < 10)
        time += "0";
    time += String(minuts) + ":";
    if (seconds < 10)
        time += "0";
    time += String(seconds);

    return time;
}

String TimeManager::FormatTime(int epoch)
{
    int _hours = epoch / 3600;
    int _minuts = (epoch % 3600) / 60;
    int _seconds = epoch % 60;

    String time = "";

    if (_hours < 10)
        time += "0";
    time += String(_hours) + ":";
    if (_minuts < 10)
        time += "0";
    time += String(_minuts) + ":";
    if (_seconds < 10)
        time += "0";
    time += String(_seconds);

    return time;
}

TimeManager::TimeManager() {}

void TimeManager::AddTimeEvent(TimeEvent event)
{
    timeEvents.add(event);

    sprintln(LOG_PREFIX + "Added " + event.stringify());
}

void TimeManager::RemoveTimeEvent(int epoch_time, int event_type)
{
    int delete_index = -1;

    for (int i = 0; i < timeEvents.size(); i++)
    {
        if (timeEvents[i].Equals(epoch_time, event_type))
        {
            delete_index = i;
        }
    }

    if (delete_index == -1)
    {
        sprintln(LOG_PREFIX + "Delete error: timeEvent not found at " + FormatTime(epoch_time) + ", type: " + String(event_type));
        return;
    }

    timeEvents.remove(delete_index);
}

void TimeManager::RemoveLastTimeEvent()
{
    timeEvents.remove(timeEvents.size() - 1);
}
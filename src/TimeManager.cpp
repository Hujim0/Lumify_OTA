#include <global.h>

#include <TimeManager.h>
#include <TimeEvent.h>

const String LOG_PREFIX = "[TimeManager] ";

TimeManager *TimeManager::Instance = 0;

void TimeManager::Setup(int epoch_time_seconds, int _dayOfTheWeek, StaticJsonDocument<STATIC_DOCUMENT_MEMORY_SIZE> &preferences)
{

    preferences["last_epoch"] = epoch_time_seconds;
    preferences["last_day_of_the_week"] = _dayOfTheWeek;

    Setup(epoch_time_seconds, _dayOfTheWeek);
}

void TimeManager::Setup(int epoch_time_seconds, int _dayOfTheWeek)
{
    if (isSetuped)
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

    isSetuped = true;

    sprintln(LOG_PREFIX + "Got time! " + GetCurrentFormattedTime() + ", day of the week: " + String(dayOfTheWeek));
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

void TimeManager::UpdateMinutes()
{
    int timeMins = (epoch_time_day_seconds % 3600) / 60;

    if (minutes == timeMins)
        return;

    minutes = timeMins;

    UpdateHours();

    // time events (minute update)

    for (int i = 0; i < timeEvents.size(); i++)
    {
        bool result = timeEvents[i].CheckTime(epoch_time_day_seconds /*, dayOfTheWeek*/);

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

    UpdateMinutes();
}

String TimeManager::GetCurrentFormattedTime()
{
    if (!isSetuped)
        return "unknown";

    String time = "";

    if (hours < 10)
        time += "0";
    time += String(hours) + ":";
    if (minutes < 10)
        time += "0";
    time += String(minutes) + ":";
    if (seconds < 10)
        time += "0";
    time += String(seconds);

    return time;
}

String TimeManager::FormatTime(int epoch)
{
    int _hours = epoch / 3600;
    int _minutes = (epoch % 3600) / 60;
    int _seconds = epoch % 60;

    String time = "";

    if (_hours < 10)
        time += "0";
    time += String(_hours) + ":";
    if (_minutes < 10)
        time += "0";
    time += String(_minutes) + ":";
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

void TimeManager::CleanTimeEvents()
{
    timeEvents.clear();
    sprintln(LOG_PREFIX + "Cleared list");
}

// void TimeManager::RemoveTimeEvent(int epoch_time, int event_type)
// {
//     int delete_index = -1;

//     for (int i = 0; i < timeEvents.size(); i++)
//     {
//         if (timeEvents[i].Equals(epoch_time, event_type))
//         {
//             delete_index = i;
//         }
//     }

//     if (delete_index == -1)
//     {
//         sprintln(LOG_PREFIX + "Delete error: timeEvent not found at " + FormatTime(epoch_time) + ", type: " + String(event_type));
//         return;
//     }

//     timeEvents.remove(delete_index);
// }

// void TimeManager::RemoveLastTimeEvent()
// {
//     timeEvents.remove(timeEvents.size() - 1);
// }

void TimeManager::setOnEventFiredEvent(OnEventFired handler)
{
    onEventFired = handler;
}
void TimeManager::InvokeOnEventFired(float transition, EventType eventType, int value, String args)
{
    if (onEventFired != NULL)
        onEventFired(transition, eventType, value, args);
}

int TimeManager::GetEpochTime()
{
    return epoch_time_day_seconds;
}
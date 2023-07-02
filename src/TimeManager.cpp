#include <TimeManager.h>
#include <TimeEvent.h>

TimeManager *TimeManager::Instance = 0;

void TimeManager::Setup(ModeHandler *handler, int epoch_time_seconds, int _dayOfTheWeek)
{
    if (isSetuped)
        return;

    Instance = this;

    modeHandler = handler;

    dayOfTheWeek = _dayOfTheWeek;

    MillisOffset = (unsigned long)(epoch_time_seconds * 1000) - millis();

    _hours = (epoch_time_seconds / 3600);
    _minuts = (epoch_time_seconds % 3600) / 60;
    _seconds = epoch_time_seconds % 60;

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

    if (_hours == timeHours)
        return;

    _hours = timeHours;

    // time events (hour update)
    // probably unnecessary
}

void TimeManager::UpdateMinuts()
{
    int timeMins = (epoch_time_day_seconds % 3600) / 60;

    if (_minuts == timeMins)
        return;

    _minuts = timeMins;

    UpdateHours();

    // time events (minute update)

    for (int i = 0; i < timeEvents.size(); i++)
    {
        bool result = timeEvents[i].CheckTime(modeHandler, epoch_time_day_seconds /*, dayOfTheWeek*/);

        Serial.println(result);

        if (result)
        {
            Serial.println(String("--[" + GetFormattedTime() + "]: " + timeEvents[i].stringify()));
        }
    }
}

void TimeManager::UpdateSeconds()
{
    int timeSecs = epoch_time_day_seconds % 60;

    if (timeSecs == _seconds)
        return;

    _seconds = timeSecs;

    UpdateMinuts();

    // time events (second update)

    FiveSecondCounter++;

    if (FiveSecondCounter == 5)
    {
        FiveSecondCounter = 0;

        if (timer != NULL)
        {
            timer();
        }
    }

    Serial.println(String(epoch_time_day_seconds) + " " + GetFormattedTime());
}

String TimeManager::GetFormattedTime()
{
    if (!isSetuped)
        return "unknown";

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

    Serial.println("--Added " + event.stringify());
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
        Serial.println("--Event not found: " + String(epoch_time) + " type: " + String(event_type));
        return;
    }

    timeEvents.remove(delete_index);
}

void TimeManager::RemoveLastTimeEvent()
{
    timeEvents.remove(timeEvents.size() - 1);
}
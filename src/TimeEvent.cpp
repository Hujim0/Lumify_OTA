#include <TimeEvent.h>
#include <TimeManager.h>

bool TimeEvent::CheckTime(int epoch_time /*, int _dayOfTheWeek*/)
{
    if (epochTime != epoch_time /* || daysOfTheWeek[_dayOfTheWeek] == false*/)
    {
        return false;
    }

    TimeManager::Instance->InvokeOnEventFired(transition, eventType, value, args);

    return true;
}

TimeEvent::TimeEvent(int epoch_time, float _transition, EventType type, int _value, String _args)
{
    epochTime = epoch_time;

    // daysOfTheWeek = _daysOfTheWeek;

    transition = _transition;

    eventType = type;

    value = _value;

    args = _args;
}

TimeEvent::TimeEvent() {}

String TimeEvent::stringify()
{
    return "TimeEvent at " + TimeManager::FormatTime(epochTime) + ", type: " + String(eventType) + ", value: " + String(value) + ", args: \"" + String(args) + "\"";
}

bool TimeEvent::Equals(int epoch_time, int event_type)
{
    return (epochTime == epoch_time && event_type == (int)eventType);
}

bool TimeEvent::Equals(int epoch_time, EventType event_type)
{
    return (epochTime == epoch_time && event_type == eventType);
}
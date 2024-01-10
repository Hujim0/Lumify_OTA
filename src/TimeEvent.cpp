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

TimeEvent::TimeEvent(int epoch_time, float _transition, EventType type, int _value, const char *_args)
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

    char msg[256];

    {
        String time = TimeManager::FormatTime(epochTime);
        snprintf(msg, sizeof(msg), "TimeEvent at %s, type: ", time.c_str());
        time[0] = 0;
    }

    if (eventType == EventType::Brightness)
    {
        snprintf(&msg[strlen(msg)],
                 sizeof(msg) - strlen(msg),
                 "Brightness, value: %i",
                 value);
    }
    else
    {
        snprintf(&msg[strlen(msg)],
                 sizeof(msg) - strlen(msg),
                 "Mode, id: %i, args: \"%s\"",
                 value,
                 args);
    }

    return String(msg);
}

bool TimeEvent::Equals(int epoch_time, EventType event_type)
{
    return (epochTime == epoch_time && event_type == eventType);
}
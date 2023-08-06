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

const char *TimeEvent::stringify()
{
    char *result = (char *)malloc(100);

    strcat(result, "TimeEvent at ");
    strcat(result, TimeManager::FormatTime(epochTime));
    strcat(result, ", type: ");
    if (eventType == EventType::Brightness)
    {
        strcat(result, "Brightness, value: ");
        itoa(value, strchr(result, 0), DEC);
    }
    else
    {
        strcat(result, "Mode, id: ");
        itoa(value, strchr(result, 0), DEC);
        strcat(result, ", args: \"");
        strcat(result, args);
        strcat(result, "\"");
    }

    return result;

    // return "TimeEvent at " +
    //        TimeManager::FormatTime(epochTime) + ", type: " + String(eventType) + ", value: " + String(value) + ", args: \"" + String(args) + "\"";
}

bool TimeEvent::Equals(int epoch_time, EventType event_type)
{
    return (epochTime == epoch_time && event_type == eventType);
}
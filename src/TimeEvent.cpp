#include <TimeEvent.h>
#include <ModeHandler.h>

bool TimeEvent::CheckTime(ModeHandler *modeHandler, int epoch_time /*, int _dayOfTheWeek*/)
{
    if (epochTime != epoch_time /* || daysOfTheWeek[_dayOfTheWeek] == false*/)
    {
        return false;
    }

    if (eventType == Brightness)
    {
        modeHandler->ChangeBrightness(value);
    }
    else
    {
        modeHandler->ChangeMode(value, args);
    }

    return true;
}

TimeEvent::TimeEvent(int epoch_time /*, bool _daysOfTheWeek*/, EventType type, int _value, String _args)
{
    epochTime = epoch_time;

    // daysOfTheWeek = _daysOfTheWeek;

    eventType = type;

    value = _value;

    args = _args.c_str();
}

TimeEvent::TimeEvent() {}

String TimeEvent::stringify()
{
    return "TimeEvent at " + String(epochTime) + ", type: " + String(eventType) + " value: " + String(value);
}

bool TimeEvent::Equals(int epoch_time, int event_type)
{
    return (epochTime == epoch_time && event_type == (int)eventType);
}

bool TimeEvent::Equals(int epoch_time, EventType event_type)
{
    return (epochTime == epoch_time && event_type == eventType);
}
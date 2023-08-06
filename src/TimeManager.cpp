#include <global.h>

#include <TimeManager.h>
#include <TimeEvent.h>

const char *LOG_PREFIX = "[TimeManager] ";

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

    char msg[50] = "";

    strcat(msg, LOG_PREFIX);
    strcat(msg, "Got time!");
    strcat(msg, GetCurrentFormattedTime());
    strcat(msg, ", day of the week: ");
    itoa(dayOfTheWeek, msg + strlen(msg), DEC);

    sprintln(msg);

    // sprintln(LOG_PREFIX + "Got time! " + GetCurrentFormattedTime() + ", day of the week: " + String(dayOfTheWeek));
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

    for (size_t i = 0; i < timeEventsCounter; i++)
    {
        bool result = timeEvents[i].CheckTime(epoch_time_day_seconds /*, dayOfTheWeek*/);

        if (!result)
            continue;

        char msg[100] = "";

        strcat(msg, LOG_PREFIX);
        strcat(msg, "TimeEvent executed: ");
        strcat(msg, timeEvents[i].stringify());

        sprintln(msg);
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

const char *TimeManager::GetCurrentFormattedTime()
{
    if (!isSetuped)
        return "unknown";

    char res[10] = "";

    if (hours < 10)
        strcat(res, "0");
    itoa(hours, strchr(res, NULL), DEC);
    strcat(res, ":");
    // time += hours + ":";
    if (minutes < 10)
        strcat(res, "0");
    itoa(minutes, strchr(res, NULL), DEC);
    strcat(res, ":");
    if (seconds < 10)
        strcat(res, "0");
    itoa(seconds, strchr(res, NULL), DEC);
    strcat(res, ":");

    return res;
}

const char *TimeManager::FormatTime(int epoch)
{
    int _hours = epoch / 3600;
    int _minutes = (epoch % 3600) / 60;
    int _seconds = epoch % 60;

    char res[10] = "";

    if (_hours < 10)
        strcat(res, "0");
    itoa(_hours, strchr(res, NULL), DEC);
    strcat(res, ":");
    // time += hours + ":";
    if (_minutes < 10)
        strcat(res, "0");
    itoa(_minutes, strchr(res, NULL), DEC);
    strcat(res, ":");
    if (_seconds < 10)
        strcat(res, "0");
    itoa(_seconds, strchr(res, NULL), DEC);
    strcat(res, ":");

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

    char msg_prefix[20] = "";

    strcat(msg_prefix, LOG_PREFIX);
    strcat(msg_prefix, "Added ");

    for (size_t i = 0; i < count; i++)
    {
        events[i] = TimeEvent(
            time_events_array_json[i]["epoch_time"].as<int>(),
            time_events_array_json[i]["transition"].as<float>(),
            (EventType)time_events_array_json[i]["event_type"].as<int>(),
            time_events_array_json[i]["value"].as<int>(),
            time_events_array_json[i]["args"].as<const char *>());

        char msg[100];

        strcpy(msg, msg_prefix);
        strcat(msg, events[i].stringify());

        sprintln(msg);

        // sprintln(LOG_PREFIX + "Added " + events[i].stringify());
    }

    time_events_array_json.clear();

    AddTimeEvents(events, count);

    json.clear();
}

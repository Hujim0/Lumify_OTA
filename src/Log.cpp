#include <Log.h>
#include <TimeManager.h>
#include <global.h>

Log *Log::Instance = 0;

void Log::Println(String msg)
{
    if (gotTime)
        msg = TimeManager::Instance->GetCurrentFormattedTime() + " " + msg;

#ifdef DEBUG_SERIAL
    Serial.println(msg);
#endif

#ifdef DEBUG_FILE
    if (!SaveLogs)
        return;

    currentFile = LittleFS.open(GetFileName(currentFileNumber), "a");
    currentFile.println(msg);
    currentFile.close();

#endif
}

void Log::Begin()
{
#ifdef DEBUG_SERIAL
    Serial.println("[ESP] Serial begin");
#endif
#ifdef DEBUG_FILE
    if (!SaveLogs)
        return;

    while (LittleFS.exists(GetFileName(currentFileNumber)))
        currentFileNumber += 1;

    sprintln("[DEBUG] Log File: " + GetFileName(currentFileNumber));
#endif
}

Log::Log()
{
    SaveLogs = true;
    gotTime = false;
    currentFileNumber = 0;

    Instance = this;
}

String Log::GetFileName(int id)
{
    return "/logs/log" + String(id) + ".txt";
}

String Log::GetFileName(String id)
{
    return "/logs/log" + id + ".txt";
}
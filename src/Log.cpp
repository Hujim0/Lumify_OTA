#include <Log.h>
#include <TimeManager.h>
#include <global.h>

Log *Log::Instance = 0;

void Log::Println(String msg)
{
    Serial.println(msg);

    if (!SaveLogs)
        return;

    currentFile = LittleFS.open(GetFileName(currentFileNumber), "a");
    if (gotTime)
        msg = "[" + TimeManager::Instance->GetFormattedTime() + "] " + msg;

    currentFile.println(msg);
    currentFile.close();
}

void Log::Begin()
{
    Serial.println("[ESP] Serial begin");

    if (!SaveLogs)
        return;

    while (LittleFS.exists(GetFileName(currentFileNumber)))
        currentFileNumber += 1;

    sprintln("Log File: " + GetFileName(currentFileNumber));
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
#include <Log.h>
#include <TimeManager.h>
#include <global.h>

Log *Log::Instance = 0;

void Log::Println(const char *toPrint)
{
    char msg[128] = "";

    if (gotTime)
    {
        strcat(msg, TimeManager::Instance->GetCurrentFormattedTime());
        strcat(msg, " ");
    }
    strcat(msg, toPrint);

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

    char *str = "[DEBUG] Log File: ";

    strcat(str, GetFileName(currentFileNumber));
    strcat(str, "\n");
    strcat(str, "[DEBUG] Reload cause: ");
    strcat(str, ESP.getResetReason().c_str());
    strcat(str, " ");
    strcat(str, ESP.getResetInfo().c_str());

    sprintln(str);
#endif
}

Log::Log()
{
    SaveLogs = true;
    gotTime = false;
    currentFileNumber = 0;

    Instance = this;
}

const char *Log::GetFileName(int &id)
{
    char result[32] = "";

    strcat(result, "/logs/log");
    itoa(id, result + strlen(result), DEC);
    strcat(result, ".txt");

    return result;
}
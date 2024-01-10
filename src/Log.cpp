#include <Log.h>
#include <TimeManager.h>
#include <global.h>

Log *Log::Instance = 0;

void Log::Println(const char *toPrint)
{
    char msg[128] = "";

    // if (gotTime)
    // {
    //     // strcpy(msg, TimeManager::Instance->GetCurrentFormattedTime());
    //     // strcat(msg, " ");
    //     strcat(msg, toPrint);
    // }
    // else
    // {
    strcpy(msg, toPrint);
    // }

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

    const char *path = GetFileName(currentFileNumber);

    while (LittleFS.exists(path))
    {
        currentFileNumber += 1;
        free((void *)path);
        path = GetFileName(currentFileNumber);
    }

    char str[128] = "[DEBUG] Log File: ";

    strcat(str, path);
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
    char *result = (char *)malloc(32);

    strcat(result, "/logs/log");
    itoa(id, result + strlen(result), DEC);
    strcat(result, ".txt");

    return result;
}
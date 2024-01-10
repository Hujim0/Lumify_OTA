#include <Log.h>
#include <TimeManager.h>
#include <global.h>

Log *Log::Instance = 0;

void Log::Println(const char *toPrint)
{
    char msg[256] = "";

    if (gotTime)
    {
        String time = TimeManager::Instance->GetCurrentFormattedTime();
        snprintf(msg, sizeof(msg), "%s %s", time.c_str(), toPrint);
        time[0] = 0;
    }
    else
    {
        strncpy(msg, toPrint, sizeof(msg));
    }

#ifdef DEBUG_SERIAL
    Serial.println(msg);
#endif

#ifdef DEBUG_FILE
    if (!SaveLogs)
        return;

    currentFile = LittleFS.open(currentFilePath, "a");
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

    int last_file_number = 0;

    char path[MAX_PATH_LENGTH];

    while (last_file_number < 100)
    {
        {
            snprintf(path, sizeof(path), "/logs/log%i.txt", last_file_number);

            if (!FileExists(path))
                break;
        }

        last_file_number += 1;
    }
    strncpy(currentFilePath, path, sizeof(currentFilePath));

    {
        char str[128] = "[DEBUG] Log File: ";

        snprintf(str, sizeof(str), "[DEBUG] Log File: %s\n[DEBUG] Reload cause: %s %s",
                 path,
                 ESP.getResetReason().c_str(),
                 ESP.getResetInfo().c_str());

        sprintln(str);
    }

#endif
}

Log::Log()
{
    SaveLogs = true;
    gotTime = false;

    Instance = this;
}

String Log::GetFileName(int id)
{
    String res((char *)0);
    res.reserve(MAX_PATH_LENGTH - 1);
    {
        char path[MAX_PATH_LENGTH];
        snprintf(path, sizeof(path), "/logs/log%i.txt", id);

        res = path;
    }

    return res;
}
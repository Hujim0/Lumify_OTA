#include <FileSystem.h>

void FSBegin()
{
    LittleFS.begin();
}

void GetWifiCredentials(String *data)
{
    File file = LittleFS.open(WIFI_SETTINGS_PATH, "r");

    data[0] = file.readStringUntil('\n');
    data[1] = file.readString();

    data[0].trim();
    data[1].trim();

    file.close();
}

void SaveWifiCredentials(const char *ssid, const char *pw)
{
    File file = LittleFS.open(WIFI_SETTINGS_PATH, "w");
    file.write(ssid);
    file.write("\n");
    file.write(pw);

    file.close();
}

void SavePreferences(const char *preferences_json)
{
    File file = LittleFS.open(PREFERENCES_PATH, "w");
    file.write(preferences_json);
    file.close();
}
void SavePreferences(StaticJsonDocument<STATIC_DOCUMENT_MEMORY_SIZE> *preferences_json)
{
    File file = LittleFS.open(PREFERENCES_PATH, "w");
    serializeJson(*preferences_json, file);

    file.close();

    preferences_json->garbageCollect();
}

String LoadPreferences()
{
    String data((char *)0);
    data.reserve(256);
    {
        File file = LittleFS.open(PREFERENCES_PATH, "r");
        data = file.readString();

        file.close();
    }

    return data;
}

String GetModeArgs(int id)
{
    String data((char *)0);
    data.reserve(MAX_ARGS_LENGTH);

    {
        String path = GetModeArgsFilePath(id);

        if (!LittleFS.exists(path))
            return data;

        File file = LittleFS.open(path, "r");

        path[0] = 0;

        data = file.readString();
        file.close();
    }

    return data;
}

String GetModeArgsFilePath(int id)
{
    String data((char *)0);
    data.reserve(MAX_PATH_LENGTH - 1);

    {
        char path[64];

        snprintf(path, sizeof(path), "/modes/mode%i.json", id);

        data = path;
    }

    return data;
}

String GetModeArgsFilePath(const char *id)
{
    String data((char *)0);
    data.reserve(MAX_PATH_LENGTH - 1);

    {
        char path[MAX_PATH_LENGTH];

        snprintf(path, sizeof(path), "/modes/mode%s.json", id);

        data = path;
    }

    return data;
}

void SaveModeArgs(int id, const char *json)
{
    String path = GetModeArgsFilePath(id);

    File file = LittleFS.open(path, "w");
    file.write(json);
    file.close();

    path[0] = 0;
}

String GetElementsFilePath(const char *lang, const char *id)
{
    String data((char *)0);
    data.reserve(MAX_PATH_LENGTH - 1);
    {
        char path[MAX_PATH_LENGTH];

        snprintf(path, sizeof(path), "/modes/elements/%s/elements%s.json", lang, id);

        data = path;
    }

    return data;
}

String GetTimeEvents()
{
    File file = LittleFS.open("/time_events.json", "r");
    String data = file.readString();
    file.close();

    return data;
}

void SaveTimeEvents(const char *data)
{
    File file = LittleFS.open("/time_events.json", "w");
    file.write(data);
    file.close();
}

String GetModeArgsDefault(int id)
{
    String data((char *)0);
    data.reserve(MAX_PATH_LENGTH - 1);

    {
        char path[MAX_PATH_LENGTH];
        snprintf(path, sizeof(path), "/modes/default/mode%i.json", id);

        if (!LittleFS.exists(path))
            return data;

        File file = LittleFS.open(path, "r");
        data = file.readString();
        file.close();
    }

    return data;
}

bool FileExists(const char *path)
{
    return LittleFS.exists(path);
}
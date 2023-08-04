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

const char *LoadPreferences()
{
    File file = LittleFS.open(PREFERENCES_PATH, "r");
    const char *data = file.readString().c_str();
    file.close();
    return data;
}

const char *GetModeArgs(int id)
{
    const char *path = GetModeArgsFilePath(id);

    if (!LittleFS.exists(path))
        return "";

    File file = LittleFS.open(path, "r");
    const char *data = file.readString().c_str();
    file.close();
    return data;
}

const char *GetModeArgsFilePath(int id)
{
    char path[32] = "/modes/mode";
    itoa(id, strchr(path, NULL), DEC);
    strcat(path, ".json");

    return path;
}

const char *GetModeArgsFilePath(const char *id)
{
    char path[32] = "/modes/mode";
    strcat(path, id);
    strcat(path, ".json");

    return path;
}

void SaveModeArgs(int id, const char *json)
{
    File file = LittleFS.open(GetModeArgsFilePath(id), "w");
    file.write(json);
    file.close();
}

const char *GetElementsFilePath(const char *lang, const char *id)
{
    char path[32] = "/modes/elements/";

    strcat(path, lang);
    strcat(path, "/elements");
    strcat(path, id);
    strcat(path, ".json");

    return path;
}

const char *GetTimeEvents()
{
    File file = LittleFS.open("/time_events.json", "r");
    const char *data = file.readString().c_str();
    file.close();
    return data;
}

void SaveTimeEvents(const char *data)
{
    File file = LittleFS.open("/time_events.json", "w");
    file.write(data);
    file.close();
}

const char *GetModeArgsDefault(int id)
{
    char path[32] = "/modes/default/mode";
    itoa(id, strchr(path, NULL), DEC);
    strcat(path, ".json");

    if (!LittleFS.exists(path))
        return "";

    File file = LittleFS.open(path, "r");
    const char *data = file.readString().c_str();
    file.close();
    return data;
}

bool FileExists(const char *path)
{
    return LittleFS.exists(path);
}
#include <FileSystem.h>

void FSBegin()
{
    LittleFS.begin();
}

void GetWifiSettings(String *data)
{
    File file = LittleFS.open(WIFI_SETTINGS_PATH, "r");

    data[0] = file.readStringUntil('\n');
    data[1] = file.readString();

    data[0].trim();
    data[1].trim();

    file.close();
}

void SavePreferences(String preferences_json)
{
    File file = LittleFS.open(PREFRENCES_PATH, "w");
    file.write(preferences_json.c_str());
    file.close();
}
void SavePreferences(StaticJsonDocument<STATIC_DOCUMENT_MEMORY_SIZE> *preferences_json)
{
    File file = LittleFS.open(PREFRENCES_PATH, "w");
    serializeJson(*preferences_json, file);

    file.close();

    preferences_json->garbageCollect();
}

String LoadPreferences()
{
    File file = LittleFS.open(PREFRENCES_PATH, "r");
    String data = file.readString();
    file.close();
    return data;
}

String GetModeArgs(int id)
{
    String path = "/modes/mode" + String(id) + ".json";

    if (!LittleFS.exists(path))
        return "";

    File file = LittleFS.open(path, "r");
    String data = file.readString();
    file.close();
    return data;
}

String GetModeArgsFilePath(String id)
{
    String path = "/modes/mode" + id + ".json";

    if (!LittleFS.exists(path))
        return "";
    return path;
}

void SaveModeArgs(int id, String json)
{
    File file = LittleFS.open("/modes/mode" + String(id) + ".json", "w");
    file.write(json.c_str());
    file.close();
}

// String GetElements(int id)
// {
//     String path = "/modes/elements/elements" + String(id) + ".json";

//     if (!LittleFS.exists(path))
//         return "";

//     File file = LittleFS.open(path, "r");
//     String data = file.readString();
//     file.close();
//     return data;
// }
String GetElementsFilePath(String lang, String id)
{
    String path = "modes/elements/" + lang + "/elements" + id + ".json";

    if (!LittleFS.exists(path))
        return "";
    return path;
}

String GetTimeEvents()
{
    File file = LittleFS.open("/time_events.json", "r");
    String data = file.readString();
    file.close();
    return data;
}

void SaveTimeEvents(String data)
{
    File file = LittleFS.open("/time_events.json", "w");
    file.write(data.c_str());
    file.close();
}

String GetModeArgsDefault(int id)
{
    String path = "/modes/default/mode" + String(id) + ".json";

    if (!LittleFS.exists(path))
        return "";

    File file = LittleFS.open(path, "r");
    String data = file.readString();
    file.close();
    return data;
}

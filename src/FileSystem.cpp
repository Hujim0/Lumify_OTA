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

String GetPreferences()
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
    {
        throw std::invalid_argument("File not found");
        return "";
    }

    File file = LittleFS.open(path, "r");
    String data = file.readString();
    file.close();
    return data;
}

void SaveModeArgs(int id, String json)
{
    File file = LittleFS.open("/modes/mode" + String(id) + ".json", "w");
    file.write(json.c_str());
    file.close();
}

String GetElements(int id)
{
    File file = LittleFS.open("/modes/elements/elements" + String(id) + ".json", "r");
    String data = file.readString();
    file.close();
    return data;
}

String GetTimeEvents()
{
    File file = LittleFS.open("/time_events.json", "r");
    String data = file.readString();
    file.close();
    return data;
}
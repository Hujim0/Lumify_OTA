#pragma once

#define WIFI_SETTINGS_PATH "/wifi_settings.txt"
#define PREFERENCES_PATH "/preferences.json"

#include <LittleFS.h>
#include <ArduinoJson.h>

#define STATIC_DOCUMENT_MEMORY_SIZE 512

void GetWifiCredentials(String *data_write_to);
void SaveWifiCredentials(const char *ssid, const char *pw);
const char *LoadPreferences();
void SavePreferences(const char *json_string);
void SavePreferences(StaticJsonDocument<STATIC_DOCUMENT_MEMORY_SIZE> *json);
void SaveTimeEvents(const char *json_string);
const char *GetModeArgs(int id);
const char *GetModeArgsDefault(int id);
const char *GetModeArgsFilePath(const char *id);
const char *GetModeArgsFilePath(int id);
void SaveModeArgs(int id, const char *args_string);
void FSBegin();
const char *GetTimeEvents();
bool FileExists(const char *path);
// void Save

const char *GetElementsFilePath(const char *lang, const char *id);
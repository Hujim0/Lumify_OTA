#ifndef FILESYSTEMMAHAGER_H
#define FILESYSTEMMAHAGER_H

#define WIFI_SETTINGS_PATH "/wifi_settings.txt"
#define PREFERENCES_PATH "/preferences.json"

#define MAX_PATH_LENGTH 64
#define MAX_ARGS_LENGTH 256

#include <LittleFS.h>
#include <ArduinoJson.h>

#define STATIC_DOCUMENT_MEMORY_SIZE 512

void SaveWifiCredentials(const char *ssid, const char *pw);
void SavePreferences(const char *json_string);
void SavePreferences(StaticJsonDocument<STATIC_DOCUMENT_MEMORY_SIZE> *json);
void SaveTimeEvents(const char *json_string);
void SaveModeArgs(int id, const char *args_string);

String GetElementsFilePath(const char *lang, const char *id);
String GetModeArgsFilePath(const char *id);
String GetModeArgsFilePath(int id);

void GetWifiCredentials(String *data_write_to);
String GetModeArgsDefault(int id);
String GetModeArgs(int id);
String GetTimeEvents();
String LoadPreferences();
// void Save

void FSBegin();
bool FileExists(const char *path);

#endif
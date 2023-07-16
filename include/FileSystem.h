#ifndef FILESYSTEMMAHAGER_H
#define FILESYSTEMMAHAGER_H

#define WIFI_SETTINGS_PATH "/wifi_settings.txt"
#define PREFRENCES_PATH "/preferences.json"

#include <LittleFS.h>
#include <ArduinoJson.h>

#define STATIC_DOCUMENT_MEMORY_SIZE 512

void GetWifiSettings(String *data_write_to);
String LoadPreferences();
void SavePreferences(String json_string);
void SavePreferences(StaticJsonDocument<STATIC_DOCUMENT_MEMORY_SIZE> *json);
void SaveTimeEvents(String json_string);
String GetModeArgs(int id);
String GetModeArgsDefault(int id);
void SaveModeArgs(int id, String args_string);
void FSBegin();
String GetTimeEvents();
// void Save

String GetModeArgsFilePath(String id);
String GetElementsFilePath(String lang, String id);

#endif
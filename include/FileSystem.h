#ifndef FILESYSTEMMAHAGER_H
#define FILESYSTEMMAHAGER_H

#define WIFI_SETTINGS_PATH "/wifi_settings.txt"
#define PREFRENCES_PATH "/preferences.json"

#include <LittleFS.h>
#include <ArduinoJson.h>

#define STATIC_DOCUMENT_MEMORY_SIZE 512

void GetWifiSettings(String *);
String LoadPreferences();
void SavePreferences(String);
void SavePreferences(StaticJsonDocument<STATIC_DOCUMENT_MEMORY_SIZE> *);
String GetModeArgs(int);
void SaveModeArgs(int, String);
void FSBegin();
String GetTimeEvents();
// void Save

String GetModeArgsFilePath(String id);
String GetElementsFilePath(String lang, String id);

#endif
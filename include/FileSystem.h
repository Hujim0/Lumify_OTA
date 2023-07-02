#ifndef FILESYSTEMMAHAGER_H
#define FILESYSTEMMAHAGER_H

#define WIFI_SETTINGS_PATH "/wifi_settings.txt"
#define PREFRENCES_PATH "/preferences.json"

#include <LittleFS.h>
#include <ArduinoJson.h>

void GetWifiSettings(String *);
String GetPreferences();
void SavePreferences(String);
String GetModeArgs(int);
void SaveModeArgs(int, String);
String GetElements(int);
void FSBegin();
String GetTimeEvents();
// void Save

#endif
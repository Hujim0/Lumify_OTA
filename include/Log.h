#pragma once

#include <ArduinoJson.h>
#include <LittleFS.h>

#define DEBUG

#ifdef DEBUG

#define DEBUG_FILE
#define DEBUG_SERIAL

#endif

class Log
{

public:
    bool SaveLogs = true;
    bool gotTime = false;

    int currentFileNumber = 0;

    File currentFile;

    // void Print(const char *);
    void Println(String);
    String GetFileName(int id);
    String GetFileName(String id);

    void Begin();

    static Log *Instance;

    Log();
};
#ifndef LOG_H
#define LOG_H

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

    char currentFilePath[64];

    File currentFile;

    // void Print(const char *);
    void Println(const char *);
    String GetFileName(int id);

    void Begin();

    static Log *Instance;

    Log();
};

#endif
#ifndef LOG_H
#define LOG_H

#include <ArduinoJson.h>
#include <LittleFS.h>

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

#endif
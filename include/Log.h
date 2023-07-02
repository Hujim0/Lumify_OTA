#ifndef LOG_H
#define LOG_H

#include <ArduinoJson.h>

class Log
{

public:
    static bool SaveLogs;

    static void Print(const char *);
    static void Print(String);
};

#endif
#ifndef CAPTIVE_REQUEST_HANDLER
#define CAPTIVE_REQUEST_HANDLER

#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

class CaptiveRequestHandler : public AsyncWebHandler
{
public:
    bool canHandle(AsyncWebServerRequest *request);
    void handleRequest(AsyncWebServerRequest *request);

    CaptiveRequestHandler();
    virtual ~CaptiveRequestHandler() {}
};

#endif
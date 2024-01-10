#ifndef MAIN_H
#define MAIN_H

#define NUMPIXELS 300

#include <TimeEvent.h>

#define STRIP WS2812B
#define STRIP_PIN D4
#define COLOR_ORDER GRB

void AddServerHandlers();
// void OnClientConnected(int);
void OnWebSocketMessage(const char *);
void ApplyPreferences(const char *);
void ChangeLanguage(const char *);
void SetupFastLED();
void minuteTimer();
void tenSecondTimer();
void ConnectToWifi();
void LoadTimeEvents();
void TryReconnect();
void onTimeEventFired(float, EventType eventType, int value, const char *args);
void SetupCaptivePortal();
void OnConnected();
void NewCredentials(const char *ssid, const char *pw);

#endif
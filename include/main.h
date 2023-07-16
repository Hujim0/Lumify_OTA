#ifndef MAIN_H
#define MAIN_H

#define NUMPIXELS 300

#include <TimeEvent.h>

#define STRIP WS2812B
#define STRIP_PIN D4
#define COLOR_ORDER GRB

void AddServerHandlers();
// void OnClientConnected(int);
void OnWebSocketMessage(String);
void ApplyPreferences(String);
void ChangeLanguage(String);
void SetupFastLED();
void minuteSecondTimer();
void ConnectToWifi();
void LoadTimeEvents();
void TryReconnect();
void onTimeEventFired(float, EventType eventType, int value, String args);

#endif
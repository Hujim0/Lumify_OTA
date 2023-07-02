#ifndef MAIN_H
#define MAIN_H

#define NUMPIXELS 300

#define STRIP WS2812B
#define STRIP_PIN D4
#define COLOR_ORDER GRB

void OnClientConnected(int);
void OnWebSocketMessage(String);
void LoadFromPreferences(String);
void ChangeLanguage(String);
void ledSetup();
void fiveSecondTimer();
void networkSetup();
void LoadTimeEvents();

#endif

#define FASTLED_ESP8266_RAW_PIN_ORDER
#define FASTLED_ESP8266_NODEMCU_PIN_ORDER
#define FASTLED_ESP8266_D1_PIN_ORDER
#define FASTLED_ALLOW_INTERRUPTS 0

#include <Arduino.h>
#include <FastLED.h>

#include <Log.h>

#define DEBUG_HEAP
#define INITIAL_DELAY 1500
//  #define DEBUG_PREFERENCES

#include <main.h>
#include <ModeHandler.h>
#include <NetworkManager.h>
#include <FileSystem.h>
#include <TimeManager.h>
#include <global.h>
#include <NetworkAP.h>

#include <ArduinoJson.h>

CRGB leds[NUMPIXELS];

Log _log = Log();

ModeHandler modeHandler;
NetworkManager network = NetworkManager();
NetworkAP network_ap = NetworkAP();
TimeManager timeManager;
char currentLanguage[10] = "en";

void setup()
{
  delay(INITIAL_DELAY);

  ESP.wdtDisable();

  Serial.begin(115200);

  FSBegin();

  _log.Begin();

  sprintln("[ESP] loaded");
  {
    String preferences_string = LoadPreferences();
    ApplyPreferences(preferences_string.c_str());
    preferences_string[0] = 0;
  }

  SetupFastLED();

  network.OnConnectionSuccessful(OnConnected);

  ConnectToWifi();

  LoadTimeEvents();
}

void OnConnected()
{
  AddServerHandlers();

  {
    String args = GetModeArgs(modeHandler.current_mode_id);
    modeHandler.ChangeMode(modeHandler.current_mode_id, args.c_str());

    args[0] = 0;
  }

  timeManager.OneMinuteTimer = minuteTimer;
  timeManager.TenSecondsTimer = tenSecondTimer;
}

void loop()
{
  if (modeHandler.led_state)
  {
    modeHandler.update(leds);
  }
  FastLED.show();

  timeManager.Update();

  // sprintln("update");

  ESP.wdtFeed();

  if (network_ap.isActive)
  {
    network_ap.update();
  }
  else
  {
    network.loop();
  }
}

void ConnectToWifi()
{
  sprintln(line);

  String wifi_data[2];
  GetWifiCredentials(wifi_data);

  if (wifi_data[0] != NULL && wifi_data[0] != "")
  {
    if (!network.BeginSTA(wifi_data[0].c_str(), wifi_data[1].c_str()))
      sprintln(LOG_PREFIX "Failed to connect! Falling back to AP...");
    else
      return;
  }
  else
  {
    sprintln(LOG_PREFIX "Credentials not found! Starting AP...");
  }

  SetupCaptivePortal();

  network.OnNewCredentials(NewCredentials);

  wifi_data[0][0] = 0;
  wifi_data[1][0] = 0;
}

void NewCredentials(const char *ssid, const char *pw)
{
  SaveWifiCredentials(ssid, pw);
  network_ap.CloseCaptivePortal();
  // network._server.removeHandler(new CaptiveRequestHandler());
  AddServerHandlers();

  // network.BeginSTA(ssid, pw);
}

void SetupCaptivePortal()
{
  if (!network_ap.StartCaptivePortal("Lumify "))
    return;

  // network._server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);

  network.ServeStatic("/data", LittleFS, "/", "max-age=600");

  network.AddWebPageHandler("/", [](AsyncWebServerRequest *request)
                            { if(request->hasArg("ssid")) {
                                SaveWifiCredentials(request->arg("ssid").c_str(),
                                                request->arg("pw").c_str()); 

                                ESP.reset();

                                // network.BeginSTA(request->arg("ssid").c_str(),
                                //                 request->arg("pw").c_str()); 
                                request->send(200);
                                return;
                            }
                              
                              request->send(request->beginResponse(
                                  LittleFS,
                                  "/web/configure/configure.html",
                                  "text/html")); });
  // network._server.

  network._server.begin();
}

void TryReconnect()
{
  if (WiFi.getMode() != WiFiMode_t::WIFI_AP)
    network.TryReconnect();
}

void AddServerHandlers()
{
  // network.OnNewClient(OnClientConnected);
  network.OnNewMessage(OnWebSocketMessage);
  network.OnConnectionLost(TryReconnect);

  network.ServeStatic("/data", LittleFS, "/", "max-age=600");

  // // global
  network.AddWebPageHandler("/", [](AsyncWebServerRequest *request)
                            { 
    char uri[64];

    snprintf(uri, sizeof(uri), "/%s/home.json", currentLanguage);

    request->redirect(uri); });

  network.AddWebPageHandler("/favicon.ico", [](AsyncWebServerRequest *request)
                            { request->send(
                                  request->beginResponse(LittleFS, "/web/favicon.png", "image/png")); });

  network.AddWebPageHandler("/localization", [](AsyncWebServerRequest *request)
                            {     
                              char uri[64];
                              snprintf(uri, 
                                      sizeof(uri), 
                                      "/modes/elements/%s/localization.json", 
                                      currentLanguage);

                              request->send(
                                  request->beginResponse(
                                    LittleFS, 
                                    uri, 
                                    "text/json")); });

  network.AddWebPageHandler("/mode", [](AsyncWebServerRequest *request)
                            {
                              int id = request->arg("id").toInt();

                              char args_string[MAX_ARGS_LENGTH] = "";

                              {
                                String args = GetModeArgs(id);

                                strncpy(args_string, args.c_str(), sizeof(args_string));
                                args[0] = 0;
                              }

                              if (strcmp(args_string, "") == 0)
                              {
                                {
                                  char msg[64];
                                  snprintf(msg, sizeof(msg), "[ERROR] Invalid mode id: %i", id);
                                  sprintln(msg);
                                }

                                request->send(404);
                                return;
                              }

                              if (request->hasArg("change"))
                              {
                                modeHandler.ChangeMode(id, args_string);
                              }
                              if (request->hasArg("save"))
                              {
                                SaveModeArgs(id, args_string);

                                StaticJsonDocument<STATIC_DOCUMENT_MEMORY_SIZE> preferences;
                                deserializeJson(preferences, LoadPreferences());
                                preferences["mode"] = id;
                                SavePreferences(&preferences);
                              }

                              request->send(
                                  request->beginResponse(
                                    HTTP_POST, 
                                    "text/json", 
                                    args_string)); });

  network.AddJSONBodyHandler("/time_events", [](AsyncWebServerRequest *request, JsonVariant &json)
                             {
                        
                              {
                                String json_string;
                                serializeJson(json, json_string);
                                SaveTimeEvents(json_string.c_str());
                                json_string[0] = 0;
                              }
                              
                              {
                                timeManager.AddTimeEventsFromJson(json);

                                json.clear();
                              }
                              
                              request->send(200); });

  network.AddJSONBodyHandler("/mode", [](AsyncWebServerRequest *request, JsonVariant &json)
                             {
                              int id = request->arg("id").toInt();

                              char json_string[256] = "";

                              {
                                String _json_string((char *)0);
                                serializeJson(json, _json_string);
                                json.clear();
                                strncpy(json_string, _json_string.c_str(), sizeof(json_string));
                                _json_string[0] = 0;
                              }                                    

                              if (request->hasArg("save"))
                              {
                                SaveModeArgs(id, json_string);
                              }

                              if (modeHandler.current_mode_id != id)
                              {
                                modeHandler.ChangeMode(id, json_string);

                                if (request->hasArg("save"))
                                {
                                  StaticJsonDocument<STATIC_DOCUMENT_MEMORY_SIZE> preferences;
                                  deserializeJson(preferences, LoadPreferences());
                                  preferences["mode"] = id;
                                  SavePreferences(&preferences);
                                }
                              }

                              modeHandler.UpdateArgs(json_string);

                              request->send(200); });

  network.AddWebPageHandler("/elements", [](AsyncWebServerRequest *request)
                            {
                              char path[64] = "";
                              {
                                String path_string = GetElementsFilePath(currentLanguage, request->arg("id").c_str());
                                strncpy(path, path_string.c_str(), sizeof(path));
                                path_string[0] = 0;
                              }

                              if (!FileExists(path))
                                {
                                {
                                  char msg[64];
                                  snprintf(msg, sizeof(msg), "[ERROR] Not found elements file! id: %s", request->arg("id").c_str());
                                  sprintln(msg);
                                }

                                request->send(404);
                                return;
                              }

                              request->send(
                                  request->beginResponse(LittleFS, path, "text/json")); });

  network.AddWebPageHandler("/preferences", [](AsyncWebServerRequest *request)
                            { request->send(
                                  request->beginResponse(LittleFS, "preferences.json", "text/json")); });

  network.AddWebPageHandler("/changelang", [](AsyncWebServerRequest *request)
                            {
                              char lang[10] = "";
                              {
                                strncpy(lang, request->arg("lang").c_str(), sizeof(lang));
                                ChangeLanguage(lang);
                              }

                              char uri[32] = "";
                              
                              snprintf(uri, sizeof(uri), "/%s/home", lang);

                              request->redirect(uri); });

  network.AddWebPageHandler("/log", [](AsyncWebServerRequest *request)
                            {
                              char file_name[64] = "";
                              
                              if (request->hasArg("id"))
                              {
                                String file_name_string = _log.GetFileName(request->arg("id").toInt());
                                strncpy(file_name, file_name_string.c_str(), sizeof(file_name));
                                file_name_string[0] = 0;
                              }
                              else
                              {
                                strncpy(file_name, _log.currentFilePath ,sizeof(file_name));
                              }

                              if (!FileExists(file_name))
                              {
                                request->send(404);
                                return;
                              }

                              request->send(
                                  request->beginResponse(
                                      LittleFS,
                                      file_name,
                                      "text/plain")); });

  network.AddWebPageHandler("/time", [](AsyncWebServerRequest *request)
                            {
                              StaticJsonDocument<STATIC_DOCUMENT_MEMORY_SIZE> preferences;
                              deserializeJson(preferences, LoadPreferences());

                              timeManager.Setup(
                                request->arg("epoch").toInt(), 
                                request->arg("dayoftheweek").toInt(), 
                                preferences);
                              _log.gotTime = true;

                              SavePreferences(&preferences);

                              request->send(200); });

  network.AddWebPageHandler("/brightness", [](AsyncWebServerRequest *request)
                            {
                              int value = request->arg("value").toInt();
                              FastLED.setBrightness(value);

                              if (request->hasArg("save"))
                              {
                                StaticJsonDocument<STATIC_DOCUMENT_MEMORY_SIZE> preferences;
                                deserializeJson(preferences, LoadPreferences());
                                preferences[BRIGHTNESS] = value;
                                SavePreferences(&preferences);
                              }
                              request->send(200); });

  network.AddWebPageHandler("/light_switch", [](AsyncWebServerRequest *request)
                            {
                              bool value = request->arg("value") == "true";
                              modeHandler.LightSwitch(value);

                              if (request->hasArg("save"))
                              {
                                StaticJsonDocument<STATIC_DOCUMENT_MEMORY_SIZE> preferences;
                                deserializeJson(preferences, LoadPreferences());
                                preferences[LIGHT_SWITCH] = value;
                                SavePreferences(&preferences);
                              }
                              request->send(200); });

  network.AddWebPageHandler("/fullreset", [](AsyncWebServerRequest *request)
                            { 
                              SaveWifiCredentials("", "");
                              request->send(200);
                              delay(100);
                              ESP.reset(); });

  //====================================================

  // pages
  network.AddWebPageHandler("/en/home", [](AsyncWebServerRequest *request)
                            { request->send(
                                  request->beginResponse(
                                      LittleFS,
                                      "web/home/home.html",
                                      "text/html")); });
  network.AddWebPageHandler("/ru/home", [](AsyncWebServerRequest *request)
                            { request->send(
                                  request->beginResponse(
                                      LittleFS,
                                      "web/home/home_ru.html",
                                      "text/html")); });

  //------------------------------------------------------
  network.AddWebPageHandler("/en/schedule", [](AsyncWebServerRequest *request)
                            { request->send(
                                  request->beginResponse(
                                      LittleFS,
                                      "web/schedule/schedule.html",
                                      "text/html")); });
  network.AddWebPageHandler("/ru/schedule", [](AsyncWebServerRequest *request)
                            { request->send(
                                  request->beginResponse(
                                      LittleFS,
                                      "web/schedule/schedule_ru.html",
                                      "text/html")); });

  network._server.begin();
}

void ChangeLanguage(const char *_lang)
{
  strncpy(currentLanguage, _lang, sizeof(currentLanguage));

  StaticJsonDocument<STATIC_DOCUMENT_MEMORY_SIZE> preferences;
  deserializeJson(preferences, LoadPreferences());

  preferences["lang"] = _lang;

  SavePreferences(&preferences);
}

void minuteTimer()
{

#ifdef DEBUG_HEAP
  {
    char msg[64];

    snprintf(msg, sizeof(msg), "[ESP] Available ram: %i bytes",
             (int)ESP.getFreeHeap());
    sprintln(msg);
  }

#endif

  network.CleanUp();

  StaticJsonDocument<STATIC_DOCUMENT_MEMORY_SIZE> preferences;
  deserializeJson(preferences, LoadPreferences());

  preferences["last_epoch"] = timeManager.GetEpochTime();

  SavePreferences(&preferences);
}

void tenSecondTimer()
{
  network.CheckStatus();
}

char current_stream[64] = "";

void OnWebSocketMessage(const char *data)
{
  if (strcmp(current_stream, "") == 0)
  {
    {
      char msg[64];

      snprintf(msg, sizeof(msg), "[Websocket] New stream: \"%s\"", data);

      sprintln(msg);
    }

    strcpy(current_stream, data);

    return;
  }

  if (strcmp(data, "]") == 0)
  {
    strcpy(current_stream, "");
    return;
  }

  if (strcmp(current_stream, BRIGHTNESS) == 0)
    FastLED.setBrightness(atoi(data));
  else
    modeHandler.PushArg(current_stream, data);
}

void SetupFastLED()
{
  pinMode(STRIP_PIN, OUTPUT);

  FastLED.addLeds<STRIP, STRIP_PIN, COLOR_ORDER>(leds, NUMPIXELS).setCorrection(TypicalLEDStrip);
  FastLED.setMaxRefreshRate(60);
  FastLED.setMaxPowerInVoltsAndMilliamps((uint8_t)5, (uint32_t)(60 * NUMPIXELS));

  FastLED.clearData();
  FastLED.clear();
  FastLED.show();
}

void ApplyPreferences(const char *data)
{
  StaticJsonDocument<STATIC_DOCUMENT_MEMORY_SIZE> preferences;
  deserializeJson(preferences, data);

#ifdef DEBUG_PREFERENCES
  msg char[256] = ;

  snprintf(msg, sizeof(msg), "[ESP] Loaded preferences: \n%s\n" line, data);

  sprintln(msg);

#endif

  modeHandler.LightSwitch(preferences["light_switch"].as<bool>());
  FastLED.setBrightness(preferences["brightness"].as<int>());

  modeHandler.current_mode_id = preferences["mode"].as<int>();

  strncpy(currentLanguage, preferences["lang"].as<const char *>(), sizeof(currentLanguage));

  _log.SaveLogs = preferences["debug"].as<bool>();

  timeManager.Setup(preferences["last_epoch"].as<int>(),
                    preferences["last_day_of_the_week"].as<int>());

  preferences.garbageCollect();
}

void LoadTimeEvents()
{
  timeManager.setOnEventFiredEvent(onTimeEventFired);

  StaticJsonDocument<STATIC_DOCUMENT_MEMORY_SIZE> doc;
  deserializeJson(doc, GetTimeEvents());

  timeManager.AddTimeEventsFromJson(doc.to<JsonVariant>());

  doc.garbageCollect();
}

void onTimeEventFired(float, EventType eventType, int value, const char *args)
{
  StaticJsonDocument<STATIC_DOCUMENT_MEMORY_SIZE> preferences;
  deserializeJson(preferences, LoadPreferences());

  if (eventType == Brightness)
  {
    preferences["brightness"] = value;
    modeHandler.ChangeBrightness(value);

    network.SendEvent("brightness_changed", String(value).c_str());
  }
  else
  {
    preferences["mode"] = value;
    modeHandler.ChangeMode(value, args);

    {
      char msg[128];
      snprintf(msg, sizeof(msg), "%i,%s", value, args);

      network.SendEvent("mode_changed", msg);
    }

    SaveModeArgs(value, args);
  }

  SavePreferences(&preferences);
}
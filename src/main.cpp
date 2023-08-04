
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
const char *currentLanguage = "en";

void setup()
{
  delay(INITIAL_DELAY);

  ESP.wdtDisable();

  Serial.begin(115200);

  FSBegin();

  _log.Begin();

  sprintln("[ESP] loaded");

  ApplyPreferences(LoadPreferences());
  SetupFastLED();

  network.OnConnectionSuccessful(OnConnected);

  ConnectToWifi();

  LoadTimeEvents();
}

void OnConnected()
{
  AddServerHandlers();

  modeHandler.ChangeMode(
      modeHandler.current_mode_id,
      GetModeArgs(modeHandler.current_mode_id));

  timeManager.timer = minuteSecondTimer;
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
    {
      char msg[128] = "";

      strcpy(msg, LOG_PREFIX);
      strcat(msg, "Failed to connect! Falling back to AP...");

      sprintln(msg);
    }
    else
      return;
  }
  else
  {
    char msg[128] = "";

    strcpy(msg, LOG_PREFIX);
    strcat(msg, "Credentials not found! Starting AP...");

    sprintln(msg);
  }

  SetupCaptivePortal();

  network.OnNewCredentials(NewCredentials);
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
    char uri[32] = "/";
    strcat(uri, currentLanguage);
    strcat(uri, "/home");

    request->redirect(uri); });

  network.AddWebPageHandler("/favicon.ico", [](AsyncWebServerRequest *request)
                            { request->send(
                                  request->beginResponse(LittleFS, "/web/favicon.png", "image/png")); });

  network.AddWebPageHandler("/localization", [](AsyncWebServerRequest *request)
                            {     
    char uri[32] = "/modes/elements/";
    strcat(uri, currentLanguage);
    strcat(uri, "/localization.json");

    request->send(
        request->beginResponse(LittleFS, uri, "text/json")); });

  network.AddWebPageHandler("/mode", [](AsyncWebServerRequest *request)
                            {
    int id = request->arg("id").toInt();
    const char* args = GetModeArgs(id);

    if (strcmp(args, ""))
    {
      {
        char msg[64] = "[ERROR] Invalid mode id: ";
        itoa(id, strchr(msg, 0), DEC);
        sprintln(msg);
      }

      request->send(404);
      return;
    }

    if (request->hasArg("change"))
    {
      modeHandler.ChangeMode(id, args);
    }
    if (request->hasArg("save"))
    {
      // SaveModeArgs(id, args);

      StaticJsonDocument<STATIC_DOCUMENT_MEMORY_SIZE> preferences;
      deserializeJson(preferences, LoadPreferences());
      preferences["mode"] = id;
      SavePreferences(&preferences);
    }

    request->send(
        request->beginResponse(HTTP_POST, "text/json", args)); });

  network.AddJSONBodyHandler("/time_events", [](AsyncWebServerRequest *request, JsonVariant &json)
                             {
    const char* json_string = "";
                        
    {
      String _json_string;
      serializeJson(json, _json_string);
      json_string = _json_string.c_str();
      _json_string[0] = 0;
    }

    SaveTimeEvents(json_string);

    timeManager.AddTimeEventsFromJson(json);

    json.clear();
    
    request->send(200); });

  network.AddJSONBodyHandler("/mode", [](AsyncWebServerRequest *request, JsonVariant &json)
                             {
    int id = request->arg("id").toInt();

    const char *json_string = "";

    {
      String _json_string;
      serializeJson(json, _json_string);
      json_string = _json_string.c_str();
      _json_string[0] = 0;
    }

    json.clear();
          

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
    const char* path = GetElementsFilePath(currentLanguage, request->arg("id").c_str());

    if (!FileExists(path))
    {
      {
        char msg[64] = "[ERROR] Not found elements file! id=";
        strcat(msg, request->arg("id").c_str());

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
    ChangeLanguage(request->arg("lang").c_str());
    request->redirect("/" + request->arg("lang") + "/home"); });

  network.AddWebPageHandler("/log", [](AsyncWebServerRequest *request)
                            {
    int id = 0;

    if (request->hasArg("id"))
      id = request->arg("id").toInt();
    else
      id = _log.currentFileNumber;

    const char* FileName = _log.GetFileName(id);

    if (!FileExists(FileName))
    {
      request->send(404);
      return;
    }

    request->send(
        request->beginResponse(LittleFS, FileName, "text/plain")); });

  network.AddWebPageHandler("/time", [](AsyncWebServerRequest *request)
                            {
    StaticJsonDocument<STATIC_DOCUMENT_MEMORY_SIZE> preferences;
    deserializeJson(preferences, LoadPreferences());

    timeManager.Setup(request->arg("epoch").toInt(), request->arg("dayoftheweek").toInt(), preferences);
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
                            { SaveWifiCredentials("", "");
                              delay(100);
                              request->send(200);
                              ESP.reset(); });

  //====================================================

  // pages
  network.AddWebPageHandler("/en/home", [](AsyncWebServerRequest *request)
                            { request->send(
                                  request->beginResponse(LittleFS, "web/home/home.html", "text/html")); });
  network.AddWebPageHandler("/ru/home", [](AsyncWebServerRequest *request)
                            { request->send(
                                  request->beginResponse(LittleFS, "web/home/home_ru.html", "text/html")); });

  //------------------------------------------------------
  network.AddWebPageHandler("/en/schedule", [](AsyncWebServerRequest *request)
                            { request->send(
                                  request->beginResponse(LittleFS, "web/schedule/schedule.html", "text/html")); });
  network.AddWebPageHandler("/ru/schedule", [](AsyncWebServerRequest *request)
                            { request->send(
                                  request->beginResponse(LittleFS, "web/schedule/schedule_ru.html", "text/html")); });

  network._server.begin();
}

void ChangeLanguage(const char *_lang)
{
  currentLanguage = _lang;

  StaticJsonDocument<STATIC_DOCUMENT_MEMORY_SIZE> preferences;
  deserializeJson(preferences, LoadPreferences());

  preferences["lang"] = _lang;

  SavePreferences(&preferences);
}

void minuteSecondTimer()
{
#ifdef DEBUG_HEAP
  {
    char msg[64] = "[ESP] Available ram: ";

    itoa(ESP.getFreeHeap(), strchr(msg, NULL), DEC);
    strcat(msg, " bytes");
    sprintln(msg);
  }

#endif

  network.CleanUp();

  network.CheckStatus();

  StaticJsonDocument<STATIC_DOCUMENT_MEMORY_SIZE> preferences;
  deserializeJson(preferences, LoadPreferences());

  preferences["last_epoch"] = timeManager.GetEpochTime();

  SavePreferences(&preferences);
}

const char *current_stream = "";

void OnWebSocketMessage(const char *data)
{
  if (!strcmp(current_stream, ""))
  {
    if (strcmp(data, "]"))
    {
      current_stream = "";
      return;
    }

    if (strcmp(current_stream, BRIGHTNESS))
      FastLED.setBrightness(atoi(data));
    else
      modeHandler.PushArg(current_stream, data);

    return;
  }

  {
    char msg[64] = "[Websocket] Got: \"";
    strcat(msg, data);
    strcat(msg, "\" --endln");
    sprintln(msg);
  }

  current_stream = data;
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
  msg char[256] = "[ESP] Loaded preferences: \n";

  strcat(msg, data);
  strcat(msg, "\n");
  strcat(msg, line);

  sprintln(msg);

#endif

  modeHandler.LightSwitch(preferences["light_switch"].as<bool>());
  FastLED.setBrightness(preferences["brightness"].as<int>());

  modeHandler.current_mode_id = preferences["mode"].as<int>();

  currentLanguage = preferences["lang"].as<const char *>();

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
      char msg[128] = "";
      itoa(value, msg, DEC);
      strcat(msg, ",");
      strcat(msg, args);

      network.SendEvent("mode_changed", msg);
    }

    SaveModeArgs(value, args);
  }

  SavePreferences(&preferences);
}

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
String currentLanguage = "en";

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
      GetModeArgs(modeHandler.current_mode_id).c_str());

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
      sprintln(LOG_PREFIX + "Failed to connect! Falling back to AP...");
    else
      return;
  }
  else
    sprintln(LOG_PREFIX + "Credentials not found! Starting AP...");

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

  network.AddWebPageHandler("/", [](AsyncWebServerRequest *request)
                            { if(request->hasArg("ssid")) {
                                network.BeginSTA(request->arg("ssid").c_str(),
                                                request->arg("pw").c_str()); 
                                request->send(200);
                                return;
                            }
                              
                              request->send(request->beginResponse(
                                  LittleFS,
                                  "/web/configure/configure.html",
                                  "text/html")); });

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
                            { request->redirect("/" + currentLanguage + "/home"); });
  network.AddWebPageHandler("/favicon.ico", [](AsyncWebServerRequest *request)
                            { request->send(
                                  request->beginResponse(LittleFS, "/web/favicon.png", "image/png")); });

  network.AddWebPageHandler("/localization", [](AsyncWebServerRequest *request)
                            { request->send(
                                  request->beginResponse(LittleFS, "/modes/elements/" + currentLanguage + "/localization.json", "text/json")); });

  network.AddWebPageHandler("/mode", [](AsyncWebServerRequest *request)
                            {
  int id = request->arg("id").toInt();
  String args = GetModeArgs(id);

  if (args == "")
  {
    sprintln("[ERROR] Invalid mode id: " + String(id));
    request->send(404);
    return;
  }

  if (request->hasArg("change"))
  {
    modeHandler.ChangeMode(id, args.c_str());
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
    timeManager.CleanTimeEvents();

    String json_string;

    serializeJson(json, json_string);

    JsonArray time_events_array = json["events"];
    json.clear();

    for (size_t i = 0; i < time_events_array.size(); i++)
    {
      timeManager.AddTimeEvent(TimeEvent(
        time_events_array[i]["epoch_time"].as<int>(),
        time_events_array[i]["transition"].as<float>(),
        (EventType)time_events_array[i]["event_type"].as<int>(),
        time_events_array[i]["value"].as<int>(),
        time_events_array[i]["args"].as<String>()
      ));
    }

    time_events_array.clear();

    SaveTimeEvents(json_string);
    
    request->send(200); });

  network.AddJSONBodyHandler("/mode", [](AsyncWebServerRequest *request, JsonVariant &json)
                             {
    int id = request->arg("id").toInt();

    String json_string = "";

    serializeJson(json, json_string);
    json.clear();

    if (request->hasArg("save"))
    {
      SaveModeArgs(id, json_string);
    }

    if (modeHandler.current_mode_id != id)
    {
      modeHandler.ChangeMode(id, json_string.c_str());

      if (request->hasArg("save"))
      {
        StaticJsonDocument<STATIC_DOCUMENT_MEMORY_SIZE> preferences;
        deserializeJson(preferences, LoadPreferences());
        preferences["mode"] = id;
        SavePreferences(&preferences);
      }
    }

    modeHandler.UpdateArgs(json_string.c_str());

    request->send(200); });

  network.AddWebPageHandler("/elements", [](AsyncWebServerRequest *request)
                            {
    String path = GetElementsFilePath(currentLanguage, request->arg("id"));

    if (path == "")
    {
      sprintln("[ERROR] Not found elements file! id=" + request->arg("id"));
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
    ChangeLanguage(request->arg("lang"));
    request->redirect("/" + request->arg("lang") + "/home"); });

  network.AddWebPageHandler("/log", [](AsyncWebServerRequest *request)
                            {
    String id = "";

    if (request->hasArg("id"))
    {
      id = request->arg("id");
    }
    else
    {
      id = _log.currentFileNumber;
    }

    String FileName = _log.GetFileName(id);

    if (!LittleFS.exists(FileName))
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

void ChangeLanguage(String _lang)
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
  sprintln("[ESP] Available ram: " + String(ESP.getFreeHeap()) + " bytes");
#endif

  network.CleanUp();

  network.CheckStatus();

  StaticJsonDocument<STATIC_DOCUMENT_MEMORY_SIZE> preferences;
  deserializeJson(preferences, LoadPreferences());

  preferences["last_epoch"] = timeManager.GetEpochTime();

  SavePreferences(&preferences);
}

String current_stream = "";

void OnWebSocketMessage(String data)
{
  if (current_stream != "")
  {
    if (data.endsWith("]"))
    {
      current_stream = "";
      return;
    }

    if (current_stream == BRIGHTNESS)
      FastLED.setBrightness(data.toInt());
    else
      modeHandler.PushArg(current_stream, data);

    return;
  }

  sprintln("[Websocket] Got: \"" + data + "\" --endln");

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

void ApplyPreferences(String data)
{
  StaticJsonDocument<STATIC_DOCUMENT_MEMORY_SIZE> preferences;
  deserializeJson(preferences, data);

#ifdef DEBUG_PREFERENCES
  sprintln("[ESP] Loaded preferences: ");

  sprintln(" settings: ");
  sprintln(data);

  sprintln(line);

#endif

  modeHandler.LightSwitch(preferences["light_switch"].as<bool>());
  FastLED.setBrightness(preferences["brightness"].as<int>());

  modeHandler.current_mode_id = preferences["mode"].as<int>();

  currentLanguage = preferences["lang"].as<String>();

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

  for (size_t i = 0; i < doc["events"].size(); i++)
  {
    JsonObject timeEvent = doc["events"][i].as<JsonObject>();

    int epoch_time = timeEvent["epoch_time"].as<int>();
    float transition = timeEvent["transition"].as<float>();
    int event_type = timeEvent["event_type"].as<int>();

    int value = timeEvent["value"].as<int>();

    String args = timeEvent["args"].as<String>();

    timeManager.AddTimeEvent(TimeEvent(
        epoch_time,
        transition,
        (EventType)event_type,
        value,
        args));

    timeEvent.clear();
  }

  doc.garbageCollect();
}

void onTimeEventFired(float, EventType eventType, int value, String args)
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
    args.trim();
    // args.replace('\n', '');
    preferences["mode"] = value;
    modeHandler.ChangeMode(value, args.c_str());

    network.SendEvent("mode_changed", (String(value) + "," + args).c_str());

    SaveModeArgs(value, args);
  }

  SavePreferences(&preferences);
}
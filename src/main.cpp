
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
// #include <ESPAsync_WiFiManager.h>
#include <FileSystem.h>
#include <TimeManager.h>

#include <ArduinoJson.h>

CRGB leds[NUMPIXELS];

Log _log = Log();

#define sprintln _log.Println

ModeHandler modeHandler;
NetworkManager network = NetworkManager();
TimeManager timeManager;
String currentLanguage = "en";

void setup()
{
  delay(INITIAL_DELAY);

  Serial.begin(115200);

  FSBegin();

  _log.Begin();

  sprintln("[ESP] loaded");

  ApplyPreferences(LoadPreferences());

  ConnectToWifi();
  AddServerHandlers();

  SetupFastLED();

  timeManager.timer = fiveSecondTimer;

  modeHandler.ChangeMode(
      modeHandler.current_mode_id,
      GetModeArgs(modeHandler.current_mode_id).c_str());

  LoadTimeEvents();

  // timeManager.AddTimeEvent(TimeEvent(72180 /*, days*/, Brightness, 255, ""));
}

void loop()
{
  if (modeHandler.led_state)
  {
    modeHandler.update(leds);
  }
  FastLED.show();

  timeManager.Update();

  network.loop();
}

void ConnectToWifi()
{
  String wifi_data[2];
  GetWifiSettings(wifi_data);

  while (!network.Begin(wifi_data[0].c_str(), wifi_data[1].c_str()))
  {
    // try again
  }
}

void TryReconnect()
{
  sprintln("[ERROR] Lost connection!!!");

  network.TryReconnect();
}

void AddServerHandlers()
{
  // network.OnNewClient(OnClientConnected);
  network.OnNewMessage(OnWebSocketMessage);
  network.OnConnectionLost(TryReconnect);

  network.ServeStatic("/data", LittleFS, "/", "max-age=600");

  // global
  network.AddWebPageHandler("http://local_lumify/", [](AsyncWebServerRequest *request)
                            { request->redirect(network.getUrl() + currentLanguage + "/home"); });

  network.AddWebPageHandler("/", [](AsyncWebServerRequest *request)
                            { request->redirect("/" + currentLanguage + "/home"); });

  network.AddWebPageHandler("/mode", [](AsyncWebServerRequest *request)
                            {
    int id = request->arg("id").toInt();
    String args = GetModeArgs(id);

    if (args == "") {
        sprintln("[ERROR] Invalid mode id: " + String(id));
        request->send(404);
        return;
    }

    if (!request->hasArg("request"))
    {
        modeHandler.ChangeMode(id, args.c_str());

        SaveModeArgs(id, args);

        StaticJsonDocument<STATIC_DOCUMENT_MEMORY_SIZE> preferences;
        deserializeJson(preferences, LoadPreferences());
        preferences["mode"] = id;
        SavePreferences(&preferences);
    }

    request->send(
        request->beginResponse(HTTP_POST, "text/json", args)); });

  network.AddWebPageHandler("/elements", [](AsyncWebServerRequest *request)
                            { request->send(
                                  request->beginResponse(LittleFS, "modes/elements/" + currentLanguage + "/elements" + request->arg("id") + ".json", "text/json")); });
  network.AddWebPageHandler("/preferences", [](AsyncWebServerRequest *request)
                            { request->send(
                                  request->beginResponse(LittleFS, "preferences.json", "text/json")); });

  network.AddWebPageHandler("/changelang", [](AsyncWebServerRequest *request)
                            {
        ChangeLanguage(request->arg("lang"));
        request->redirect("/" + request->arg("lang") + "/home"); });

  // network.AddWebPageGetter("/post_time", [](AsyncWebServerResponse *response) {
  //});

  network.AddWebPageHandler("/log", [](AsyncWebServerRequest *request)
                            {
    String FileName = "";
    if (request->hasArg("id"))
    {
      FileName = _log.GetFileName(request->arg("id"));
    }
    else
    {
      FileName = _log.GetFileName(_log.currentFileNumber);
    }

    request->send(
        request->beginResponse(LittleFS, FileName, "text/json")); });
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
}

// void AddWebResponce(const char *page_name, const char *file_path, const char *content_type)
// {
//     network.AddWebPageHandler(page_name, [](AsyncWebServerRequest *request)
//                               { request->send(
//                                     request->beginResponse(LittleFS, file_path, content_type)); });
// }
void ChangeLanguage(String _lang)
{
  currentLanguage = _lang;

  StaticJsonDocument<STATIC_DOCUMENT_MEMORY_SIZE> preferences;
  deserializeJson(preferences, LoadPreferences());

  preferences["lang"] = _lang;

  SavePreferences(&preferences);
}

void fiveSecondTimer()
{
#ifdef DEBUG_HEAP
  sprintln("Avalible ram: " + String(ESP.getFreeHeap()) + " bytes");
#endif

  network.CleanUp();

  network.CheckStatus();
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

  if (data[0] != '{')
    return;

  sprintln("[Websocket] Got: \"" + data + "\" --endln");

  StaticJsonDocument<STATIC_DOCUMENT_MEMORY_SIZE> doc;
  deserializeJson(doc, data.c_str());

  if (!doc.containsKey("event"))
  {
    doc.garbageCollect();

    modeHandler.UpdateArgs(data.c_str());

    SaveModeArgs(modeHandler.current_mode_id, data);

    return;
  }
  else if (doc["event"] == STREAM_OPEN)
  {
    current_stream = doc["value"].as<String>();
    return;
  }

  StaticJsonDocument<STATIC_DOCUMENT_MEMORY_SIZE> preferences;
  deserializeJson(preferences, LoadPreferences());

  if (doc["event"] == BRIGHTNESS)
  {
    int value = doc["value"].as<int>();
    FastLED.setBrightness(value);
    preferences[BRIGHTNESS] = value;
  }
  else if (doc["event"] == LIGHT_SWITCH)
  {
    bool value = doc["value"].as<bool>();
    modeHandler.LightSwitch(value);
    preferences[LIGHT_SWITCH] = value;
  }
  else if (doc["event"] == EPOCH_TIME)
  {
    timeManager.Setup(&modeHandler, doc["value"].as<int>(), doc["dayOfTheWeek"].as<int>());
    _log.gotTime = true;
  }

  SavePreferences(&preferences);

  doc.garbageCollect();
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

  preferences.garbageCollect();
}

void LoadTimeEvents()
{
  StaticJsonDocument<STATIC_DOCUMENT_MEMORY_SIZE> doc;
  deserializeJson(doc, GetTimeEvents());

  for (size_t i = 0; i < doc["events"].size(); i++)
  {
    JsonObject timeEvent = doc["events"][i].as<JsonObject>();

    int epoch_time = timeEvent["epoch_time"].as<int>();
    int event_type = timeEvent["event_type"].as<int>();

    int value = timeEvent["value"].as<int>();

    String args = timeEvent["args"].as<String>();

    timeManager.AddTimeEvent(TimeEvent(
        epoch_time,
        (EventType)event_type,
        value,
        args));
  }

  doc.garbageCollect();
}
#pragma region fastled include

#define FASTLED_ESP8266_RAW_PIN_ORDER
#define FASTLED_ESP8266_NODEMCU_PIN_ORDER
#define FASTLED_ESP8266_D1_PIN_ORDER
#define FASTLED_ALLOW_INTERRUPTS 0

#include <Arduino.h>
#include <FastLED.h>

#pragma endregion

// #define DEBUG_HEAP
//  #define DEBUG_PREFERENCES

#include <main.h>
#include <ModeHandler.h>
#include <NetworkManager.h>
#include <FileSystem.h>
#include <TimeManager.h>

#include <ArduinoJson.h>

CRGB leds[NUMPIXELS];

ModeHandler modeHandler;
NetworkManager network = NetworkManager();
TimeManager timeManager;
String lang = "en";
#define INITIAL_DELAY 1500

void setup()
{
    delay(INITIAL_DELAY);

    Serial.begin(115200);
    Serial.println("[ESP] loaded");

    FSBegin();

    LoadFromPreferences(GetPreferences());

    networkSetup();

    ledSetup();

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
}

void networkSetup()
{
    network.OnNewClient(OnClientConnected);
    network.OnNewMessage(OnWebSocketMessage);
    network.OnConnectionLost(networkSetup);

    String wifi_data[2];
    GetWifiSettings(wifi_data);

    if (!network.Begin(wifi_data[0].c_str(), wifi_data[1].c_str()))
    {
        networkSetup();
        return;
    }

    network.ServeStatic("/data", LittleFS, "/", "max-age=600");

    // global
    network.AddWebPageHandler("http://local_lumify/", [](AsyncWebServerRequest *request)
                              { request->redirect(network.getUrl() + lang + "/home"); });

    network.AddWebPageHandler("/", [](AsyncWebServerRequest *request)
                              { request->redirect("/" + lang + "/home"); });

    network.AddWebPageHandler("/mode", [](AsyncWebServerRequest *request)
                              {
                                int id = request->arg("id").toInt();
                                String args = "";

                                try
                                {
                                    args = GetModeArgs(id);
                                }
                                catch (const std::exception& ex) {
                                    Serial.println("[error]" + String(ex.what()));
                                    request->send(404);
                                    return;
                                }

                                if (!request->hasArg("request"))
                                {
                                    modeHandler.ChangeMode(id, args.c_str());

                                    SaveModeArgs(id, args);

                                    String json = GetPreferences();
                                    StaticJsonDocument<STATIC_DOCUMENT_MEMORY_SIZE> preferences;
                                    deserializeJson(preferences, json);
                                    preferences["mode"] = id;
                                    String json_save;
                                    serializeJsonPretty(preferences, json_save);
                                    SavePreferences(json_save);
                                    preferences.garbageCollect();
                                } 

                                request->send(
                                    request->beginResponse(HTTP_POST, "text/json", args)); });

    network.AddWebPageHandler("/elements", [](AsyncWebServerRequest *request)
                              { request->send(
                                    request->beginResponse(LittleFS, "modes/elements/" + lang + "/elements" + request->arg("id") + ".json", "text/json")); });
    network.AddWebPageHandler("/preferences", [](AsyncWebServerRequest *request)
                              { request->send(
                                    request->beginResponse(LittleFS, "preferences.json", "text/json")); });

    network.AddWebPageHandler("/changelang", [](AsyncWebServerRequest *request)
                              {
        ChangeLanguage(request->arg("lang"));
        request->redirect("/" + request->arg("lang") + "/home"); });

    // network.AddWebPageGetter("/post_time", [](AsyncWebServerResponse *response) {
    //});

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
    lang = _lang;

    String json = GetPreferences();

    StaticJsonDocument<STATIC_DOCUMENT_MEMORY_SIZE> preferences;
    deserializeJson(preferences, json);

    preferences["lang"] = _lang;

    String json_save;
    serializeJsonPretty(preferences, json_save);
    SavePreferences(json_save);

    preferences.garbageCollect();
}

void fiveSecondTimer()
{
#ifdef DEBUG_HEAP
    Serial.print("Avalible ram: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" bytes");
#endif

    network.CleanUp();

    network.CheckStatus();
}

unsigned long timer = millis();

void OnClientConnected(int id)
{
    // network.SentTextToClient(id, GetPreferences().c_str());
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
    deserializeJson(preferences, GetPreferences());

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
    }

    String json;
    serializeJsonPretty(preferences, json);
    SavePreferences(json);

    preferences.garbageCollect();
    doc.garbageCollect();
}

void ledSetup()
{
    pinMode(STRIP_PIN, OUTPUT);

    FastLED.addLeds<STRIP, STRIP_PIN, COLOR_ORDER>(leds, NUMPIXELS).setCorrection(TypicalLEDStrip);
    FastLED.setMaxRefreshRate(60);
    FastLED.setMaxPowerInVoltsAndMilliamps((uint8_t)5, (uint32_t)(60 * NUMPIXELS));

    FastLED.clearData();
    FastLED.clear();
    FastLED.show();
}

void LoadFromPreferences(String data)
{
    StaticJsonDocument<STATIC_DOCUMENT_MEMORY_SIZE> preferences;
    deserializeJson(preferences, data);

#ifdef DEBUG_PREFERENCES
    Serial.println("[ESP] loading preferences...");

    Serial.print("Loaded settings: ");
    Serial.println(data);

    Serial.println("------------------------------------------------------------------");

#endif

    modeHandler.LightSwitch(preferences["light_switch"].as<bool>());
    FastLED.setBrightness(preferences["brightness"].as<int>());

    modeHandler.current_mode_id = preferences["mode"].as<int>();

    lang = preferences["lang"].as<String>();

    preferences.garbageCollect();
}

void LoadTimeEvents()
{
    String json = GetTimeEvents();

    StaticJsonDocument<STATIC_DOCUMENT_MEMORY_SIZE> doc;
    deserializeJson(doc, json.c_str());

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
}
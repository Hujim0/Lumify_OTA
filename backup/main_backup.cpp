#pragma region fastled include

#define FASTLED_ESP8266_RAW_PIN_ORDER
#define FASTLED_ESP8266_NODEMCU_PIN_ORDER
#define FASTLED_ESP8266_D1_PIN_ORDER
#define FASTLED_ALLOW_INTERRUPTS 0

#include <Arduino.h>
#include <FastLED.h>

#pragma endregion

#include <main.h>
#include <ModeHandler.h>

#include <wifi_settings.h>

#pragma region fastled setup

CRGB leds[NUMPIXELS];

ModeHandler modeHandler;

void ledSetup()
{
    modeHandler = ModeHandler();

    //.setCorrection(TypicalLEDStrip);
    FastLED.addLeds<STRIP, STRIP_PIN, COLOR_ORDER>(leds, NUMPIXELS);
    FastLED.setBrightness(20);
}

#pragma endregion

#include <index_html.h>
#include <light_switch_html.h>
#include <mode_switch_html.h>
#include <brightness_html.h>

void IndexPageHandler();
void LightSwitchHandler();
void ModeSwitchHandler();
void BrightnessHandler();

#pragma region wifi

ESP8266WebServer server(HTTP_PORT);
WiFiClient client;

void networkSetup()
{
    Serial.begin(115200);
    Serial.println("[ESP01] loaded");
    // connection to wifi
    WiFi.begin(ssid, password);

    Serial.print("[ESP01] Connecting to ");
    Serial.print(ssid);

    while (WiFi.status() != WL_CONNECTED && millis() < INITIAL_DELAY + ATTEMPT_DURATION)
    {
        Serial.print(".");
        delay(500);
    }

    // if esp cant connect within 30 secs, reset and try again
    if (WiFi.status() != WL_CONNECTED)
    {
        ESP.restart();

        return;
    }

    Serial.println("success");

    // server setup
    server.begin();
    server.on("/", IndexPageHandler);
    server.on(pathToLightSwitchHttp, LightSwitchHandler);
    server.on(pathToModeSwitchHttp, ModeSwitchHandler);
    server.on(pathToBrightnessHttp, BrightnessHandler);

    // server url
    Serial.print("[ESP01] HTTP server started at \"http://");
    Serial.print(WiFi.localIP());
    Serial.print(":");
    Serial.println(HTTP_PORT);

    // send ready message
    Serial.println("!r");
}

#pragma endregion

void setup()
{
    delay(INITIAL_DELAY);

    networkSetup();

    ledSetup();

    // array<String, 3> args = {"1", "2", "reversed"};

    modeHandler.ChangeMode(0);
    modeHandler.setup(leds);
}

void loop()
{
    server.handleClient();

    if (modeHandler.led_state)
    {
        modeHandler.update(leds);
        FastLED.show();
    }
}

#pragma region http functions

void IndexPageHandler(void)
{
    server.send(200, "text/html", index_html);
}

void LightSwitchHandler(void)
{
    server.send(200, "text/html", light_switch_html);

    const String arg = server.arg("switch_light_state");

    Serial.print("!l=");
    Serial.println(arg);

    modeHandler.LedSwitch(arg.startsWith("true"));
}

void ModeSwitchHandler(void)
{
    server.send(200, "text/html", mode_switch_html);

    const int arg = server.arg("mode_switch_value").toInt();

    Serial.print("!m=");
    Serial.println(arg);

    if (arg == 0 && server.hasArg("r"))
    {
        array<String, 10> args;
        const String r = server.arg("r");
        args[0] = r;
        const String g = server.arg("g");
        args[1] = g;
        const String b = server.arg("b");
        args[2] = b;
        modeHandler.ChangeMode(arg, args);
        modeHandler.setup(leds);
        return;
    }

    modeHandler.ChangeMode(arg);
    modeHandler.setup(leds);
}

void BrightnessHandler(void)
{
    server.send(200, "text/html", brightness_html);

    const int arg = server.arg("b").toInt();

    Serial.print("!b=");
    Serial.println(arg);

    FastLED.setBrightness(arg);
}

#pragma endregion
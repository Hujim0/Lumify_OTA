#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#define DNS_SERVER_URL "lumify"
#include <ESP8266mDNS.h>

#pragma once

#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <Arduino.h>

#include <ESP8266WiFi.h>

#define DEFAULT_PORT 80

const String LOG_PREFIX = "[Network] ";

#define ATTEMPT_DURATION 5000

#define DEBUG_WIFI_SETTINGS

typedef std::function<void(String)> OnNewMessageHandler;
typedef std::function<void(int)> OnNewClientHandler;
typedef std::function<void()> OnConnectionLostHandler;
typedef std::function<void()> OnConnectionSuccessfulHandler;
typedef std::function<void(const char *ssid, const char *pw)> OnNewCredentialsHandler;

const IPAddress ip(192, 168, 0, 146);
const IPAddress gateway(192, 168, 0, 146);
const IPAddress subnet(255, 255, 255, 0);

class NetworkManager
{
private:
    OnNewMessageHandler onNewMessageHandler = NULL;
    OnConnectionLostHandler onConnectionLostHandler = NULL;
    OnConnectionSuccessfulHandler onConnectionSuccessfulHandler = NULL;
    OnNewCredentialsHandler onNewCredentialsHandler = NULL;

    String buffer;
    uint64_t buffer_size;

    String url;

public:
    OnNewClientHandler onNewClientHandler = NULL;

    AsyncWebServer _server = AsyncWebServer(DEFAULT_PORT);
    AsyncWebSocket _webSocket = AsyncWebSocket("/ws");
    AsyncEventSource _events = AsyncEventSource("/events");

    String stringPort;

    uint16_t NetworkPort = DEFAULT_PORT;

    void handleWebSocketMessage(void *, uint8_t *, size_t);

    void ResetServers(int port);
    void OnNewMessage(OnNewMessageHandler);
    void OnNewClient(OnNewClientHandler);
    void OnConnectionLost(OnConnectionLostHandler);
    void OnConnectionSuccessful(OnConnectionSuccessfulHandler);
    void OnNewCredentials(OnNewCredentialsHandler);
    void TryReconnect();
    void SentTextToClient(int client_id, const char *msg);
    void SentTextToAll(const char *msg);
    void CleanUp();

    void AddWebPageHandler(String uri, ArRequestHandlerFunction function);
    void AddWebPageHandler(const char *uri, ArRequestHandlerFunction function);
    void AddJSONBodyHandler(const String &uri, ArJsonRequestHandlerFunction func);

    void ServeStatic(const char *uri, fs::FS &fs, const char *path, const char *cache_control = (const char *)__null);
    void CheckStatus();
    void SendEvent(const char *event_name, const char *msg);

    static NetworkManager *Instance;
    String getUrl();

    bool BeginSTA(const char *ssid, const char *pw);
    void loop();

    NetworkManager();
};

#endif
#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#define DNS_SERVER_URL "lumify"
#include <ESP8266mDNS.h>

#pragma once

#include <NetworkDefines.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <ESPAsync_WiFiManager.hpp>

#include <ESP8266WiFi.h>
#include <ESPAsyncDNSServer.h>
#include <ESP8266WiFiMulti.h>

#define PORT 80

#define ATTEMPT_DURATION 30000

#define DEBUG_WIFI_SETTINGS

typedef std::function<void(String)> OnNewMessageHandler;
typedef std::function<void(int)> OnNewClientHandler;
typedef std::function<void()> OnConnectionLostHandler;

class NetworkManager
{
private:
    AsyncWebServer server = AsyncWebServer(PORT);
    AsyncWebSocket webSocket = AsyncWebSocket("/ws");
    AsyncEventSource events = AsyncEventSource("/events");
    AsyncDNSServer dnsServer;

    ESP8266WiFiMulti wifiMulti;

    String buffer;
    uint64_t buffer_size;

    String url;
    String stringPort;

public:
    void handleWebSocketMessage(void *, uint8_t *, size_t);

    OnNewMessageHandler onNewMessageHandler = nullptr;
    OnNewClientHandler onNewClientHandler = nullptr;
    OnConnectionLostHandler onConnectionLostHandler = nullptr;

    void OnNewMessage(OnNewMessageHandler);
    void OnNewClient(OnNewClientHandler);
    void OnConnectionLost(OnConnectionLostHandler);
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

    bool Begin(const char *ssid, const char *pw);
    void loop();
};

#endif
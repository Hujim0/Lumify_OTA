#include <global.h>

#include <NetworkManager.h>

#include <AsyncElegantOTA.h>

#define SERIAL_WEBSOCKET

const int MAX_ATTEMPT_COUNT = 3;

// singleton initializer
NetworkManager *NetworkManager::Instance = 0;

static void onNewEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
                       void *arg, uint8_t *data, size_t len)
{
    switch (type)
    {
    case WS_EVT_CONNECT:
        sprintln("[Websocket] client #" + String(client->id()) + " connected from " + client->remoteIP().toString());

        if (NetworkManager::Instance->onNewClientHandler != NULL)
            NetworkManager::Instance->onNewClientHandler(client->id());

        break;
    case WS_EVT_DISCONNECT:
        sprintln("[Websocket] client #" + String(client->id()) + " disconnected");
        NetworkManager::Instance->CleanUp();
        break;
    case WS_EVT_DATA:
        NetworkManager::Instance->handleWebSocketMessage(arg, data, len);
        break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
        break;
    }
}
// websocket stuff
void NetworkManager::handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
    if (onNewMessageHandler == NULL)
        return;

    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->opcode != WS_TEXT)
        return;

    data[len] = 0;
#ifdef SERIAL_WEBSOCKET
    Serial.println((char *)data);
#endif

    if (info->len != len)
    {
        buffer_size += len;
        buffer += (char *)data;

        sprintln("[Websocket] Partial message: " + String(buffer_size) + " / " + String(info->len) + " \"" + String((char *)data) + "\" --endln");

        if (info->len != buffer_size)
            return;

        onNewMessageHandler(buffer);

        buffer_size = (uint64_t)0;
        buffer = "";

        return;
    }

    onNewMessageHandler((String)(char *)data);

    return;
}

void NetworkManager::SentTextToClient(int id, const char *data)
{
    _webSocket.text(id, data);
}
void NetworkManager::SentTextToAll(const char *data)
{
    sprintln("[Websocket] Texted to all: " + String(data));
    _webSocket.textAll(data);
}

void NetworkManager::AddWebPageHandler(String uri, ArRequestHandlerFunction func)
{
    _server.on(uri.c_str(), HTTP_GET, func);
}
void NetworkManager::AddWebPageHandler(const char *uri, ArRequestHandlerFunction func)
{
    _server.on(uri, HTTP_GET, func);
}

void NetworkManager::AddJSONBodyHandler(const String &uri, ArJsonRequestHandlerFunction func)
{
    // server.on(uri, HTTP_POST, func, upload, body);
    AsyncCallbackJsonWebHandler *handler = new AsyncCallbackJsonWebHandler(uri, func);
    _server.addHandler(handler);
}

void NetworkManager::CheckStatus()
{
    wl_status_t status = WiFi.status();
    if (status == WL_CONNECTION_LOST || status == WL_DISCONNECTED)
    {
        if (onConnectionLostHandler != NULL)
            onConnectionLostHandler();
    }
}

void NetworkManager::OnNewClient(OnNewClientHandler handler)
{
    onNewClientHandler = handler;
}

void NetworkManager::OnNewMessage(OnNewMessageHandler handler)
{
    onNewMessageHandler = handler;
}
void NetworkManager::OnConnectionLost(OnConnectionLostHandler handler)
{
    onConnectionLostHandler = handler;
}
void NetworkManager::OnConnectionSuccessful(OnConnectionSuccessfulHandler handler)
{
    onConnectionSuccessfulHandler = handler;
}
void NetworkManager::OnNewCredentials(OnNewCredentialsHandler handler)
{
    onNewCredentialsHandler = handler;
}

void NetworkManager::CleanUp()
{
    _webSocket.cleanupClients();
}

void NetworkManager::ServeStatic(const char *uri, fs::FS &fs, const char *path, const char *cache_control)
{
    _server.serveStatic(uri, fs, path, cache_control);
}

String NetworkManager::getUrl()
{
    return url;
}

void NetworkManager::TryReconnect()
{
    if (WiFi.getMode() == WiFiMode_t::WIFI_AP)
        return;
    if (WiFi.reconnect())
    {
        sprintln(LOG_PREFIX + "Success!");
    }
    else
    {
        sprintln(LOG_PREFIX + "Cant reconnect!");
    }
}
bool NetworkManager::BeginSTA(const char *ssid, const char *pw)
{
    WiFi.mode(WiFiMode_t::WIFI_STA);

#ifdef DEBUG_WIFI_SETTINGS
    sprintln(LOG_PREFIX + "Wifi credentials: " + String(ssid) + " " + String(pw));
    // sprintln(line);
#endif

    sprintln(LOG_PREFIX + "Connecting to Wifi...");

    // WiFi.config(WiFi.localIP(), WiFi.localIP(), subnet);

    // delay(500);

    int attempt = 1;

    ESP.wdtDisable();

    // delay(200);
    while (attempt <= MAX_ATTEMPT_COUNT)
    {
        sprintln("Attempt " + String(attempt));

        WiFi.begin(ssid, pw);

        if (WiFi.waitForConnectResult(5000) == WL_CONNECTED)
        {
            break;
        }
        ESP.wdtFeed();
        attempt += 1;
        delay(200);
    }

    if (attempt >= MAX_ATTEMPT_COUNT)
    {
        sprintln("[ERROR] Cant connect!");

        return false;
    }

    sprintln(LOG_PREFIX + "success");
    // server setup

    _server.begin();
    AsyncElegantOTA.begin(&_server, "admin", "admin");

    // websocket setup

    _server.addHandler(&_webSocket);
    _server.addHandler(&_events);
    _webSocket.onEvent(onNewEvent);
    _webSocket.closeAll();
    // print server url
    url = "http://" + WiFi.localIP().toString() + stringPort;

    sprintln(LOG_PREFIX + "HTTP server started at \"" + url + "\"");

    if (!MDNS.begin(DNS_SERVER_URL))
    {
        sprintln(LOG_PREFIX + "Error setting up MDNS responder!");
        return false;
    }

    sprintln(LOG_PREFIX + "mDNS responder started: \"http://" + String(DNS_SERVER_URL) + ".local" + stringPort + "\"");
    MDNS.addService("http", "tcp", NetworkPort);

    sprintln(line);

    WiFi.enableAP(false);

    if (onConnectionSuccessfulHandler != NULL)
        onConnectionSuccessfulHandler();
    if (onNewCredentialsHandler != NULL)
        onNewCredentialsHandler(ssid, pw);

    return true;
}

void NetworkManager::loop()
{
    MDNS.update();
}

void NetworkManager::SendEvent(const char *event_name, const char *msg)
{
    _events.send(msg, event_name);
}

void NetworkManager::ResetServers(int port)
{
    _server.end();
    _server = AsyncWebServer(port);
    NetworkPort = port;
}

NetworkManager::NetworkManager()
{
    Instance = this;

    stringPort = ":" + String(NetworkPort);

    if (NetworkPort == 80)
        stringPort = "";

    WiFi.onWiFiModeChange([](const WiFiEventModeChange &event)
                          { sprintln(LOG_PREFIX + "Switched Wifi mode: " + event.oldMode + " -> " + event.newMode);
                          delay(200); });

    onLostWifiConnectionHandler = [](const WiFiEventStationModeDisconnected &event)
    {
        sprintln(LOG_PREFIX + "Disconnected from Wifi: " + event.ssid);
    };

    WiFi.onStationModeDisconnected(onLostWifiConnectionHandler);

    WiFi.setSleepMode(WiFiSleepType_t::WIFI_NONE_SLEEP);
}

void NetworkManager::OnLostWifiConnection(OnLostWifiConnectionHandler handler)
{
    onLostWifiConnectionHandler = handler;
}
#include <global.h>

#include <NetworkManager.h>

#include <AsyncElegantOTA.h>

#define SERIAL_WEBSOCKET

// singleton initializer
NetworkManager *NetworkManager::Instance = 0;

static void onNewEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
                       void *arg, uint8_t *data, size_t len)
{
    switch (type)
    {
    case WS_EVT_CONNECT:
    {
        char msg[64] = "";

        strcat(msg, "[Websocket] client #");
        itoa(client->id(), strchr(msg, NULL), DEC);
        strcat(msg, " connected from ");
        strcat(msg, client->remoteIP().toString().c_str());

        sprintln(msg);
    }

        if (NetworkManager::Instance->onNewClientHandler != NULL)
            NetworkManager::Instance->onNewClientHandler(client->id());

        break;
    case WS_EVT_DISCONNECT:
    {
        char msg[64] = "";

        strcat(msg, "[Websocket] client #");
        itoa(client->id(), strchr(msg, 0), DEC);
        strcat(msg, "disconnected");

        sprintln(msg);
    }
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
        {
            char msg[128] = "";

            strcat(msg, "[Websocket] Partial message: ");
            itoa(buffer_size, strchr(msg, NULL), DEC);
            strcat(msg, " / ");
            itoa(info->len, strchr(msg, NULL), DEC);
            strcat(msg, " \"");
            strcat(msg, (char *)data);
            strcat(msg, "\" --endln");

            sprintln(msg);
        }

        if (info->len != buffer_size)
            return;

        onNewMessageHandler(buffer.c_str());

        buffer_size = (uint64_t)0;
        buffer[0] = NULL;
        buffer = "";

        return;
    }

    onNewMessageHandler((char *)data);

    return;
}

void NetworkManager::SentTextToClient(int id, const char *data)
{
    _webSocket.text(id, data);
}
void NetworkManager::SentTextToAll(const char *data)
{
    {
        char msg[128] = "";

        strcat(msg, "[Websocket] Texted to all: ");
        strcat(msg, data);

        sprintln(data);
    }

    _webSocket.textAll(data);
}

void NetworkManager::AddWebPageHandler(const char *uri, ArRequestHandlerFunction func)
{
    _server.on(uri, HTTP_GET, func);
}

void NetworkManager::AddJSONBodyHandler(const char *uri, ArJsonRequestHandlerFunction func)
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

void NetworkManager::OnLostWifiConnection(OnLostWifiConnectionHandler handler)
{
    onLostWifiConnectionHandler = handler;
}

void NetworkManager::CleanUp()
{
    _webSocket.cleanupClients();
}

void NetworkManager::ServeStatic(const char *uri, fs::FS &fs, const char *path, const char *cache_control)
{
    _server.serveStatic(uri, fs, path, cache_control);
}

const char *NetworkManager::getUrl()
{
    return url;
}

void NetworkManager::TryReconnect()
{
    if (WiFi.getMode() == WiFiMode_t::WIFI_AP)
        return;

    {
        char msg[32] = "";

        strcpy(msg, LOG_PREFIX);
        strcat(msg, "Trying to reconnect...");

        sprintln(msg);
    }

    char msg[32] = "";

    strcpy(msg, LOG_PREFIX);

    if (WiFi.reconnect())
    {

        strcat(msg, "Success!");
    }
    else
    {
        strcat(msg, "Cant reconnect!");
    }

    sprintln(msg);
}
bool NetworkManager::BeginSTA(const char *ssid, const char *pw)
{
    WiFi.mode(WiFiMode_t::WIFI_STA);

#ifdef DEBUG_WIFI_SETTINGS
    {
        char msg[100] = "";

        strcpy(msg, LOG_PREFIX);
        strcat(msg, "Wifi credentials: ");
        strcat(msg, ssid);
        strcat(msg, pw);

        sprintln(msg);
    }
#endif
    {
        char msg[32] = "";

        strcpy(msg, LOG_PREFIX);
        strcat(msg, "Connecting to Wifi...");

        sprintln(msg);
    }

    int attempt = 1;

    ESP.wdtDisable();

    while (attempt <= MAX_ATTEMPT_COUNT)
    {
        {
            char msg[32] = "";

            strcpy(msg, LOG_PREFIX);
            strcat(msg, "Attempt ");
            itoa(attempt, strchr(msg, 0), DEC);
            strcat(msg, "...");

            sprintln(msg);
        }

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

    {
        char msg[32] = "";

        strcpy(msg, LOG_PREFIX);
        strcat(msg, "success");

        sprintln(msg);
    }
    // server setup

    _server.begin();
    AsyncElegantOTA.begin(&_server, "admin", "admin");

    // websocket setup

    _server.addHandler(&_webSocket);
    _server.addHandler(&_events);
    _webSocket.onEvent(onNewEvent);
    _webSocket.closeAll();
    // print server url

    {
        char _url[64] = "";

        strcpy(_url, "http://");
        strcat(_url, WiFi.localIP().toString().c_str());
        strcat(_url, stringPort);

        url = _url;
    }

    {
        char msg[128] = "";

        strcpy(msg, LOG_PREFIX);
        strcat(msg, "HTTP server started at \"");
        strcat(msg, url);
        strcat(msg, "\"");
        sprintln(msg);
    }

    if (!MDNS.begin(DNS_SERVER_URL))
    {
        sprintln("[ERROR] Can't set up MDNS responder!");
        return false;
    }

    {
        char msg[128] = "";

        strcpy(msg, LOG_PREFIX);
        strcat(msg, "mDNS responder started: \"http://");
        strcat(msg, DNS_SERVER_URL);
        strcat(msg, ".local");
        strcat(msg, stringPort);
        strcat(msg, "\"");

        sprintln(msg);
    }

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

    if (NetworkPort == 80)
        stringPort = "";
    else
    {
        char port[10] = "";

        strcpy(port, ":");
        itoa(NetworkPort, strchr(port, 0), DEC);

        stringPort = port;
    }

    WiFi.onWiFiModeChange([](const WiFiEventModeChange &event)
                          {
        char msg[128] = "";

        strcpy(msg, LOG_PREFIX);
        strcat(msg, "Switched Wifi mode: ");
        strcat(msg, stringifyWifiMode(event.oldMode));
        strcat(msg, " -> ");
        strcat(msg, stringifyWifiMode(event.newMode));

        sprintln(msg);
    
        delay(200); });

    // onLostWifiConnectionHandler = [](const WiFiEventStationModeDisconnected &event)
    // {
    //     sprintln(LOG_PREFIX + "Disconnected from Wifi: " + event.ssid);
    // };

    // WiFi.onStationModeDisconnected(onLostWifiConnectionHandler);

    WiFi.setSleepMode(WiFiSleepType_t::WIFI_NONE_SLEEP);
}

const char *NetworkManager::stringifyWifiMode(WiFiMode mode)
{
    switch (mode)
    {
    case WiFiMode::WIFI_AP:
        return "WIFI_AP";
    case WiFiMode::WIFI_AP_STA:
        return "WIFI_AP_STA";
    case WiFiMode::WIFI_OFF:
        return "WIFI_OFF";
    case WiFiMode::WIFI_STA:
        return "WIFI_STA";
    default:
        return "";
    }

    return "";
}
#include <global.h>

#include <NetworkManager.h>
// #include <AsyncElegantOTA.h>

const IPAddress ip(192, 168, 0, 146); // статический IP
const IPAddress gateway(192, 168, 0, 146);
const IPAddress subnet(255, 255, 255, 0);

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
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->opcode != WS_TEXT)
        return;

    data[len] = 0;

    if (info->len == len)
    {
        if (onNewMessageHandler != NULL)
            onNewMessageHandler((String)(char *)data);

        return;
    }

    buffer_size += len;
    buffer += (char *)data;

    sprintln("[Websocket] Partial message: " + String(buffer_size) + " / " + String(info->len) + " \"" + String((char *)data) + "\" --endln");

    if (info->len == buffer_size)
    {

        buffer_size = (uint64_t)0;

        if (onNewMessageHandler != NULL)
            onNewMessageHandler(buffer);

        buffer = "";
    }
}

void NetworkManager::SentTextToClient(int id, const char *data)
{
    webSocket.text(id, data);
}
void NetworkManager::SentTextToAll(const char *data)
{
    sprintln("Texted to all: " + String(data));
    webSocket.textAll(data);
}

void NetworkManager::AddWebPageHandler(String uri, ArRequestHandlerFunction func)
{
    server.on(uri.c_str(), HTTP_GET, func);
}
void NetworkManager::AddWebPageHandler(const char *uri, ArRequestHandlerFunction func)
{
    server.on(uri, HTTP_GET, func);
}

void NetworkManager::AddWebPageGetter(const char *uri, ArRequestHandlerFunction func)
{
    server.on(uri, HTTP_POST, func);
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

void NetworkManager::CleanUp()
{
    webSocket.cleanupClients();
}

void NetworkManager::ServeStatic(const char *uri, fs::FS &fs, const char *path, const char *cache_contol)
{
    server.serveStatic(uri, fs, path, cache_contol);
}

String NetworkManager::getUrl()
{
    return url;
}

void NetworkManager::TryReconnect()
{
    WiFi.reconnect();
}

bool NetworkManager::Begin(const char *ssid, const char *password)
{
    Instance = this;

    // connection to wifi
    // WiFi.config(ip, gateway, subnet);
    WiFi.begin(ssid, password);
    WiFi.setAutoReconnect(true);

#ifdef DEBUG_WIFI_SETTINGS
    sprintln("Wifi: " + String(ssid) + String(password));
    Serial.println("------------------------------------------------------------------");
#endif

    sprintln("[ESP] Connecting to " + String(ssid) + "...");

    if (WiFi.waitForConnectResult(ATTEMPT_DURATION) != WL_CONNECTED)
    {
        return false;
    }

    sprintln("success");
    // server setup

    server.begin();
    // AsyncElegantOTA.begin(&server, "admin", "admin");

    // websocket setup
    server.addHandler(&webSocket);
    webSocket.onEvent(onNewEvent);
    webSocket.closeAll();
    // print server url
    url = "http://" + WiFi.localIP().toString() + ":" + String(PORT);

    sprintln("[ESP] HTTP server started at \"" + url + "\"");
    sprintln("------------------------------------------------------------------");

    return true;
}
#include <global.h>

#include <NetworkManager.h>

#include <AsyncElegantOTA.h>
#include <ESPAsync_WiFiManager.h>

const IPAddress ip(192, 168, 0, 146);
const IPAddress gateway(192, 168, 0, 146);
const IPAddress subnet(255, 255, 255, 0);

const String LOG_PREFIX = "[Network] ";

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
    webSocket.text(id, data);
}
void NetworkManager::SentTextToAll(const char *data)
{
    sprintln("[Websocket] Texted to all: " + String(data));
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

void NetworkManager::AddJSONBodyHandler(const String &uri, ArJsonRequestHandlerFunction func)
{
    // server.on(uri, HTTP_POST, func, upload, body);
    server.addHandler(new AsyncCallbackJsonWebHandler(uri, func));
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

void NetworkManager::ServeStatic(const char *uri, fs::FS &fs, const char *path, const char *cache_control)
{
    server.serveStatic(uri, fs, path, cache_control);
}

String NetworkManager::getUrl()
{
    return url;
}

void NetworkManager::TryReconnect()
{
    WiFi.reconnect();
}
bool NetworkManager::Begin(const char *ssid, const char *pw)
{
    Instance = this;

    stringPort = ":" + String(PORT);

    if (PORT == 80)
        stringPort = "";

    WiFi.begin(ssid, pw);
    WiFi.mode(WiFiMode_t::WIFI_STA);
    WiFi.setAutoReconnect(true);

#ifdef DEBUG_WIFI_SETTINGS
    sprintln(LOG_PREFIX + "Wifi credentials: " + String(ssid) + " " + String(pw));
    sprintln(line);
#endif

    sprintln(LOG_PREFIX + "Connecting to Wifi...");

    if (WiFi.waitForConnectResult(ATTEMPT_DURATION) != WL_CONNECTED)
    {
        return false;
    }

    sprintln(LOG_PREFIX + "success");
    // server setup

    server.begin();
    AsyncElegantOTA.begin(&server, "admin", "admin");

    // websocket setup

    server.addHandler(&webSocket);
    webSocket.onEvent(onNewEvent);
    webSocket.closeAll();
    // print server url
    WiFi.config(ip, gateway, subnet);
    url = "http://" + WiFi.localIP().toString() + stringPort;

    sprintln(LOG_PREFIX + "HTTP server started at \"" + url + "\"");

    if (!MDNS.begin(DNS_SERVER_URL))
    {
        sprintln(LOG_PREFIX + "Error setting up MDNS responder!");
        return false;
    }

    sprintln(LOG_PREFIX + "mDNS responder started: \"http://" + String(DNS_SERVER_URL) + ".local" + stringPort + "\"");
    MDNS.addService("http", "tcp", PORT);

    sprintln(line);

    ESPAsync_WiFiManager wifiManager(&server, &dnsServer, DNS_SERVER_URL);
    wifiManager.setMinimumSignalQuality(-1);

    // ConfigureTZ(wifiManager);

    return true;
}

void NetworkManager::loop()
{
    MDNS.update();
}

void NetworkManager::ConfigureTZ(ESPAsync_WiFiManager &manager)
{

    String tempTZ = manager.getTimezoneName();

    // if (strlen(tempTZ.c_str()) < sizeof(WM_config.TZ_Name) - 1)
    //     strcpy(WM_config.TZ_Name, tempTZ.c_str());
    // else
    //     strncpy(WM_config.TZ_Name, tempTZ.c_str(), sizeof(WM_config.TZ_Name) - 1);

    const char *TZ_Result = manager.getTZ(tempTZ);

    // if (strlen(TZ_Result) < sizeof(WM_config.TZ) - 1)
    //     strcpy(WM_config.TZ, TZ_Result);
    // else
    //     strncpy(WM_config.TZ, TZ_Result, sizeof(WM_config.TZ_Name) - 1);

    // if (strlen(WM_config.TZ_Name) > 0)
    // {
    //     sprintln("Saving current TZ_Name =" + WM_config.TZ_Name + ", TZ = " + WM_config.TZ);
    // }

    configTime(TZ_Result, "pool.ntp.org");

    static time_t now;

    now = time(nullptr);

    if (now > 1451602800)
    {
        Serial.print("Local Date/Time: ");
        Serial.print(ctime(&now));
    }
}
#include <NetworkAP.h>
#include <global.h>

#define CAPTIVE_PORTAL_PORT 53

bool NetworkAP::StartCaptivePortal(const char *ssid)
{
    WiFi.softAPConfig(ip, gateway, subnet);

    WiFi.mode(WiFiMode_t::WIFI_AP);

    WiFi.softAP(String(ssid) + "http://" + WiFi.softAPIP().toString(), "", 1, 0, 16, 100);

    sprintln(LOG_PREFIX + "AP started!" + " http://" + WiFi.softAPIP().toString());

    // WiFi.softAPgetStationNum();

    WiFi.setOutputPower(10.0F);

    isActive = true;

    return true;
}

void NetworkAP::update()
{
    // ESP.wdtFeed();

    // dnsServer.processNextRequest();
}

void NetworkAP::CloseCaptivePortal()
{
    isActive = false;
    // dnsServer.stop();
}
#include <NetworkAP.h>
#include <global.h>

#define CAPTIVE_PORTAL_PORT 53

bool NetworkAP::StartCaptivePortal(const char *ssid)
{
    WiFi.mode(WiFiMode_t::WIFI_AP);

    WiFi.softAPConfig(ip, gateway, subnet);

    WiFi.softAP(String(ssid) + "http://" + ip.toString(), "", 1, 0, 16, 100);

    sprintln(LOG_PREFIX + "AP started!");

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
    dnsServer.stop();
}
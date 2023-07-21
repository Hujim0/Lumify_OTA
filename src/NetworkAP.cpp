#include <NetworkAP.h>
#include <global.h>

#define CAPTIVE_PORTAL_PORT 53

bool NetworkAP::StartCaptivePortal(const char *ssid)
{
    WiFi.mode(WiFiMode_t::WIFI_AP);

    WiFi.softAPConfig(ip, gateway, subnet);

    WiFi.softAP(ssid);
    // WiFi.onSoftAPModeStationConnected([](const WiFiEventSoftAPModeStationConnected &event)
    //                                   { sprintln("Connected to the AP: " + String(event.aid)); });

    dnsServer.setTTL(60 * 60 * 1000);

    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);

    if (!dnsServer.start(CAPTIVE_PORTAL_PORT, "*", ip))
    {
        sprintln(LOG_PREFIX + "Error setting up Captive Portal!");
        return false;
    }

    sprintln(LOG_PREFIX + "Captive Portal started!");

    isActive = true;

    // WiFi.softAPgetStationNum();

    WiFi.setOutputPower(10.0F);

    // WiFi.`

    return true;
}

void NetworkAP::update()
{
    ESP.wdtFeed();

    dnsServer.processNextRequest();
}

void NetworkAP::CloseCaptivePortal()
{
    isActive = false;
    dnsServer.stop();
}
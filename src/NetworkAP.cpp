#include <NetworkAP.h>
#include <global.h>

#define CAPTIVE_PORTAL_PORT 53

bool NetworkAP::StartCaptivePortal(const char *ssid)
{
    WiFi.mode(WiFiMode_t::WIFI_AP);

    WiFi.softAP(ssid);

    WiFi.softAPConfig(ip, gateway, subnet);

    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);

    if (!dnsServer.start(CAPTIVE_PORTAL_PORT, "*", ip))
    {
        sprintln(LOG_PREFIX + "Error setting up Captive Portal!");
        return false;
    }

    sprintln(LOG_PREFIX + "Captive Portal started!");

    isActive = true;

    return true;
}

void NetworkAP::update()
{
    dnsServer.processNextRequest();
}

void NetworkAP::CloseCaptivePortal()
{
    isActive = false;
    dnsServer.stop();
}
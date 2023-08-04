#include <NetworkAP.h>
#include <global.h>

#define CAPTIVE_PORTAL_PORT 53

bool NetworkAP::StartCaptivePortal(const char *ssid)
{
    WiFi.softAPConfig(ip, gateway, subnet);

    WiFi.mode(WiFiMode_t::WIFI_AP);

    char url[64] = "http://";
    strcat(url, WiFi.softAPIP().toString().c_str());

    {
        char new_ssid[64] = "";
        strcpy(new_ssid, ssid);
        strcat(new_ssid, " ");
        strcat(new_ssid, url);
        WiFi.softAP(new_ssid, "", 1, 0, 16, 100);
    }

    {
        char msg[128] = "";
        strcpy(msg, LOG_PREFIX);
        strcat(msg, "AP started! ");
        strcat(msg, url);

        sprintln(msg);
    }

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
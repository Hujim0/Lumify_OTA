#ifndef NETWORK_AP
#define NETWORK_AP

#include <NetworkManager.h>
#include <DNSServer.h>

class NetworkAP
{
private:
    DNSServer dnsServer;

public:
    bool StartCaptivePortal(const char *ssid);

    void CloseCaptivePortal();

    bool isActive = false;

    void update();
};

#endif
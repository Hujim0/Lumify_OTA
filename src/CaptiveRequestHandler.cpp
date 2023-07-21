#include <CaptiveRequestHandler.h>
#include <global.h>
#include <NetworkManager.h>

bool CaptiveRequestHandler::canHandle(AsyncWebServerRequest *request)
{
    return true;
}
void CaptiveRequestHandler::handleRequest(AsyncWebServerRequest *request)
{
    if (request->hasArg("ssid"))
    {
        // sprintln("ssid: " + request->arg("ssid") + " pw: " + request->arg("pw"));
        NetworkManager::Instance->BeginSTA(request->arg("ssid").c_str(), request->arg("pw").c_str());
        AsyncResponseStream *response = request->beginResponseStream("text/html");
        response->print(R"...(<!DOCTYPE html><html lang="en"><head><title>Configure</title></head>)...");
        response->print(R"...(<body><h1>You can leave this page now</h1></body></html>)...");

        request->send(response);
        return;
    }

    // Serial.println(request->arg(0));
    // Serial.print(request->host());
    // delay(100);

    ESP.wdtFeed();

    // AsyncResponseStream *response = request->beginResponseStream("text/html");
    // response->print(R"...(<!DOCTYPE html><html lang="en"><head><title>Configure</title></head>)...");
    // response->print(R"...(<body style="font-size: 25px; background-color: #252525">)...");
    // response->print(R"...(<h1 style="text-align: center; color: white; letter-spacing: 3px">Configure</h1>)...");
    // response->print(R"...(<div style=" border: 4px solid #191d21; background-color: #393939; border-radius: 5vw; width: 75vw; min-height: 75vh; margin: 5vw; padding: 5vw; flex-direction: column;">)...");
    // response->print(R"...(<h2 style="padding-top: 10px; color: white">Enter your home wifi data for lumify to connect to it</h2>)...");
    // response->print(R"...(<form action="/credentials" method="get" style="display: flex; flex-direction: column">)...");
    // response->print(R"...(<div style="padding-top: 20px">)...");
    // response->print(R"...(<label style="color: white" for="name">Enter your wifi name: </label>)...");
    // response->print(R"...(<input type="text" name="ssid" required /></div>)...");
    // response->print(R"...(<div style="padding-top: 20px">)...");
    // response->print(R"...(<label style="color: white" for="email">Enter your wifi password: </label>)...");
    // response->print(R"...(<input type="password" name="pw" /></div>)...");
    // response->print(R"...(<div style="padding-top: 20px; align-self: center">)...");
    // response->print(R"...(<input type="submit" value="Submit" style="width: 6rem; height: 2rem"/>)...");
    // // response->print(R"...(</form></div></div></body></html>)...");
    // response->print(R"...(<!DOCTYPE html><html lang="en"><head><title>Configure</title></head>)...");
    // response->print(R"...(<body><h1>You can leave this page now</h1></body></html>)...");

    // response->addHeader("Connection", "keep-alive");
    // response->addHeader("Cache-control", "max-age=0");

    request->redirect("https://www.google.com");

    // response->print("hello");

    // request->send(response);
    // request->send(
    //     request->beginResponse(LittleFS, "text/html", "/web/configure/configure.html"));
}

CaptiveRequestHandler::CaptiveRequestHandler()
{
    // put all your routes into captiveRoutes() function.
    // captiveRoutes();
    // Webserver_configure();
}
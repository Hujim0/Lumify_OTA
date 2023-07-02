#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#define pathToLightSwitchHttp "/light_switch"
#define pathToModeSwitchHttp "/mode_switch"

#define HTTP_PORT 10200

#define INITIAL_DELAY 3000

#define ATTEMPT_DURATION 30000

#pragma region html

#pragma region index

const char *index_html = R"(
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP LED</title>
</head>

<body>
    <p>
        <label>
            Light switch
            <input type="checkbox" id="light_switch_checkbox" />
            <script type="text/javascript">

                var checkbox = document.getElementById("light_switch_checkbox");

                checkbox.checked = (localStorage.getItem("switch_light_state") == "true") ? true : false;
                checkbox.addEventListener("change", (event) => {
                    localStorage.setItem("switch_light_state", event.currentTarget.checked);


                    var index_url = window.location.href.split("?")[0];
                    window.location.href = index_url + "light_switch?switch_light_state=" +
                        event.currentTarget.checked;
                });
            </script>
        </label>
    </p>
    <p>
        <label>
            Light mode
            <select id="mode_switch_select">
                <option>0 - static</option>
                <option>1 - wave</option>
                <option>2 - rainbow</option>
            </select>
            <script type="text/javascript">

                var select = document.getElementById("mode_switch_select");

                select.selectedIndex = parseInt(localStorage.getItem("mode_switch_value"));
                select.addEventListener("change", (event) => {
                    localStorage.setItem("mode_switch_value", event.currentTarget.selectedIndex);


                    var index_url = window.location.href.split("?")[0];
                    window.location.href = index_url + "mode_switch?mode_switch_value=" +
                        event.currentTarget.selectedIndex;
                });
            </script>
        </label>
    </p>
</body>

</html>
)";

#pragma endregion

#pragma region light_switch

const char *light_switch_html = R"(
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
</head>

<body>
    <script type="text/javascript">
        window.onload = function () {
            var url_raw = window.location.href;
            var index_url = url_raw.split('?')[0];
            window.location.href = index_url.split("/light_switch")[0];

        }
    </script>
</body>

</html>
)";

#pragma endregion

#pragma region mode_switch

const char *mode_switch_html = R"(
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
</head>

<body>
    <script type="text/javascript">
        window.onload = function () {
            var url_raw = window.location.href;
            var index_url = url_raw.split('?')[0];
            window.location.href = index_url.split("/mode_switch")[0];
        }
    </script>
</body>

</html>

)";

#pragma endregion

#pragma endregion

void IndexPageHandler();
void LightSwitchHandler();
void ModeSwitchHandler();

const char *ssid = "RT-GPON-E3F6";
const char *password = "8zMQmsVB";

ESP8266WebServer server(HTTP_PORT);
WiFiClient client;

void setup(void)
{
    delay(INITIAL_DELAY);
    Serial.begin(115200);
    Serial.println("[ESP01] loaded");
    Serial.println("!nr");

    // connection to wifi
    WiFi.begin(ssid, password);

    Serial.print("[ESP01] Connecting to ");
    Serial.print(ssid);

    while (WiFi.status() != WL_CONNECTED && millis() < INITIAL_DELAY + ATTEMPT_DURATION)
    {
        Serial.print(".");
        delay(500);
    }

    // if esp cant connect within 30 secs, reset and try again
    if (WiFi.status() != WL_CONNECTED)
    {
        ESP.restart();

        return;
    }

    Serial.println("success");

    // server setup
    server.begin();
    server.on("/", IndexPageHandler);
    server.on(pathToLightSwitchHttp, LightSwitchHandler);
    server.on(pathToModeSwitchHttp, ModeSwitchHandler);

    // server url
    Serial.print("[ESP01] HTTP server started at \"http://");
    Serial.print(WiFi.localIP());
    Serial.print(":");
    Serial.println(HTTP_PORT);

    // send ready message
    Serial.println("!r");
}
void loop(void)
{
    // if (WiFi.status() != WL_CONNECTED)
    // {
    //     newSerial.print("Server is down! network error...");
    //     delay(1000);
    //     ESP.restart();
    // }

    server.handleClient();
}

void IndexPageHandler(void)
{
    server.send(200, "text/html", index_html);
}

void LightSwitchHandler(void)
{
    server.send(200, "text/html", light_switch_html);

    const String arg = server.arg("switch_light_state");

    Serial.print("!l=");
    Serial.println(arg);
}

void ModeSwitchHandler(void)
{
    server.send(200, "text/html", mode_switch_html);

    const String arg = server.arg("mode_switch_value");

    Serial.print("!m=");
    Serial.println(arg);
}
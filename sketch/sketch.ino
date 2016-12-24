#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Ticker.h>
#include <WiFiClient.h> 

//#define SOFTAP
#ifdef SOFTAP
const char *ssid = "HotspotForAHotbox";
const char *password = "nebel-in-the-cloud";
#else
const char *ssid = "";
const char *password = "";
#endif

ESP8266WebServer server(80);
Ticker ticker;

const int PIN_NEBEL_AVAILABLE = D0;
const int PIN_LED = 13;
const int PIN_NEBEL_RELAY = D1;

bool nebelAvailable = false;
unsigned long nebelUntil = 0;

void setNebelRelay(bool closed) {
    digitalWrite(PIN_NEBEL_RELAY, closed);
}

void doNebel(unsigned long duration) {
    Serial.print("Attempting to do nebel for ");
    Serial.print(duration);
    Serial.print("ms... ");
    if (nebelAvailable) {
        Serial.println("enabled.");
        setNebelRelay(true);
        nebelUntil = millis() + duration;
    } else {
        Serial.println("but nebel is not available!");
    }
}

void update() {
    nebelAvailable = !digitalRead(PIN_NEBEL_AVAILABLE);
    digitalWrite(PIN_LED, nebelAvailable);
    if (millis() > nebelUntil || !nebelAvailable) {
        //Serial.println("Disabling nebel.");
        setNebelRelay(false);
    }
}

void handleIndex() {
    static const char* index =
        "<html> \
        <head> \
            <meta name='viewport' content='width=device-width, initial-scale=1.0, user-scalable=no'> \
            <script type='text/javascript'> \
            var UPDATE_INTERVAL = 2000; \
            var UPDATE_ERROR_INTERVAL = 5000; \
            var runUpdates; \
            var updater = null; \
            function blur() { \
                runUpdates = false; \
                console.log('blur'); \
                if (updater != null) { \
                    window.clearTimeout(updater); \
                    updater = null; \
                } \
            } \
            function focus() { \
                runUpdates = true; \
                console.log('focus'); \
                if (updater == null) { \
                    updater = window.setTimeout(update, 1); \
                } \
            } \
            function formatUptime(uptime) { \
                return '' + Math.round(uptime / 1000) + ' seconds'; \
            } \
            function update() { \
                var r = new XMLHttpRequest(); \
                r.onreadystatechange = function() { \
                    if (this.readyState == 4 && this.status == 200) { \
                        var data = JSON.parse(this.responseText); \
                        var status = document.getElementById('status'); \
                        status.innerHTML = data.available ? 'Nebel is available' : 'Nebel is unavailable'; \
                        status.style.color = data.available ? 'green' : 'red'; \
                        var uptime = document.getElementById('uptime'); \
                        uptime.innerHTML = formatUptime(data.uptime); \
                        console.log(data); \
                        if (runUpdates) { \
                            updater = window.setTimeout('update()', UPDATE_INTERVAL); \
                        } else { \
                            updater = null; \
                        } \
                    } else if (this.readyState == 4) { \
                        var status = document.getElementById('status'); \
                        status.innerHTML = 'Unable to update!'; \
                        status.style.color = 'red'; \
                        if (runUpdates) { \
                            updater = window.setTimeout('update()', UPDATE_ERROR_INTERVAL); \
                        } else { \
                            updater = null; \
                        } \
                    } \
                }; \
                r.timeout = 1000; \
                r.open('GET', 'status', true); \
                r.send(); \
            } \
            </script> \
        </head> \
        <body onload='focus(); window.addEventListener(\"focus\", focus); window.addEventListener(\"blur\", blur);'> \
            <h1 id='status'></h1> \
            <form action='/nebel'> \
                <input type='submit' value='Nebel!' /> \
            </form> \
            <div>Uptime: <span id='uptime'></span></div> \
        </body> \
        </html>";
    server.send(200, "text/html", index);
}

void handleStatus() {
    Serial.print("Status request ");
    Serial.println(millis());
    char buf[512];
    snprintf(buf, 512, "{\"available\" : %d, \"uptime\" : %u}", nebelAvailable, millis());
    server.send(200, "text/plain", buf);
}

void handleNebel() {
    doNebel(1000);
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
}

void setup() {
    Serial.begin(115200);
    pinMode(PIN_NEBEL_AVAILABLE, INPUT);
    pinMode(PIN_LED, OUTPUT);
    pinMode(PIN_NEBEL_RELAY, OUTPUT);
    setNebelRelay(false);
    ticker.attach_ms(200, update);

#ifdef SOFT_AP
    Serial.print("Creating access point ");
    Serial.println(ssid);
    WiFi.softAP(ssid, password);
#else
    Serial.print("Connecting to ");
    Serial.print(ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println(" Connected!");
#endif

    if (!MDNS.begin("nebel")) {
        Serial.println("Warning: Unable to announce service via MDNS.");
    }
    MDNS.addService("http", "tcp", 80);

#ifdef SOFT_AP
    IPAddress myIP = WiFi.softAPIP();
#else
    IPAddress myIP = WiFi.localIP();
#endif

    Serial.print("IP address: ");
    Serial.println(myIP);
    server.on("/", handleIndex);
    server.on("/status", handleStatus);
    server.on("/nebel", handleNebel);
    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    server.handleClient();
}


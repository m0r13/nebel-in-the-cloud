#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Ticker.h>
#include <WiFiClient.h> 

const char *ssid = "HotspotForAHotbox";
const char *password = "nebel-in-the-cloud";

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
        Serial.println("Disabling nebel.");
        setNebelRelay(false);
    }
}

void handleIndex() {
    char buf[512];
    snprintf(buf, 512,
        "<html>"
        "<head>"
            "<meta name='viewport' content='width=device-width, initial-scale=1.0, user-scalable=no'>"
        "</head>"
        "<body>"
            "<h1 style='color: %s'>Nebel is %s</h1>"
            "<form action='/nebel'>"
                "<input type='submit' value='Nebel!' />"
            "</form>"
        "</body>"
        "</html>",
        nebelAvailable ? "green" : "red", nebelAvailable ? "available" : "not available");
    server.send(200, "text/html", buf);
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
    ticker.attach_ms(100, update);

    Serial.println();
    Serial.print("Configuring access point...");
    /* You can remove the password parameter if you want the AP to be open. */
    WiFi.softAP(ssid, password);
    MDNS.begin ("nebel");

    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    server.on("/", handleIndex);
    server.on("/nebel", handleNebel);
    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    server.handleClient();
}

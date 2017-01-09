#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <Ticker.h>
#include <WiFiClient.h> 

#include "wificonfig.h"

ESP8266WebServer server(80);
Ticker ticker;

const int PIN_NEBEL_AVAILABLE = D0;
const int PIN_LED = 13;
const int PIN_NEBEL_RELAY = D1;

bool nebelAvailable = false;
bool nebelStatus = false;
unsigned long nebelUntil = 0;

void setNebelRelay(bool closed) {
    nebelStatus = closed;
    digitalWrite(PIN_NEBEL_RELAY, !closed);
}

void startNebel(unsigned long duration) {
    Serial.print("[nebel] Attempting to do nebel for ");
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

void stopNebel() {
    nebelUntil = 0;
    setNebelRelay(false);
}

void update() {
    nebelAvailable = !digitalRead(PIN_NEBEL_AVAILABLE);
//    nebelAvailable = true;
    digitalWrite(PIN_LED, nebelAvailable);
    if (nebelStatus && (millis() > nebelUntil || !nebelAvailable)) {
        Serial.println("[nebel] Disabling nebel.");
        setNebelRelay(false);
    }
}

void logRequest() {
    Serial.print("[access] ");
    Serial.print(server.method() == HTTP_GET ? "GET" : (server.method() == HTTP_POST ? "POST" : "OTHER"));
    Serial.print(" ");
    Serial.println(server.uri());
}

void handleStatic(const char* path, const char* contentType) {
    if (SPIFFS.exists(path)) {
        File file = SPIFFS.open(path, "r");
        server.streamFile(file, contentType);
        file.close();
    } else {
        server.send(200, "text/plain", "Error");
    }
}

void handleIndex() {
    logRequest();

    handleStatic("/index.html", "text/html");
    return;
}

void handleStatus() {
    logRequest();

    char buf[512];
    snprintf(buf, 512, "{\"available\" : %d, \"uptime\" : %u}", nebelAvailable, millis());
    server.send(200, "text/plain", buf);
}

void handleNebel() {
    logRequest();

    startNebel(1000);
    server.send(200, "text/plain", "");
    //server.sendHeader("Location", "/", true);
    //server.send(302, "text/plain", "");
}

void handleStopNebel() {
    logRequest();

    stopNebel();
    server.send(200, "text/plain", "");
}

void handleRedButton() {
    logRequest();

    handleStatic("/redbutton.svg", "image/svg+xml");
    return;
}

void handleGrayButton() {
    logRequest();

    handleStatic("/graybutton.svg", "image/svg+xml");
    return;
}

void setup() {
    Serial.begin(115200);
    delay(5000);
    pinMode(PIN_NEBEL_AVAILABLE, INPUT);
    pinMode(PIN_LED, OUTPUT);
    pinMode(PIN_NEBEL_RELAY, OUTPUT);
    setNebelRelay(false);
    ticker.attach_ms(500, update);

    SPIFFS.begin();

#ifdef WIFI_AP
    Serial.print("Creating access point ");
    Serial.println(WIFI_SSID);
    WiFi.softAP(WIFI_SSID, WIFI_PWD);
#else
    Serial.print("Connecting to ");
    Serial.print(WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.begin(WIFI_SSID, WIFI_PWD);
    WiFi.reconnect();
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        //Serial.print(".");
        Serial.println(WiFi.status());
    }
    Serial.println(" Connected!");
#endif

    if (!MDNS.begin("nebel")) {
        Serial.println("Warning: Unable to announce service via MDNS.");
    }
    MDNS.addService("http", "tcp", 80);

#ifdef WIFI_AP
    IPAddress myIP = WiFi.softAPIP();
#else
    IPAddress myIP = WiFi.localIP();
#endif

    Serial.print("IP address: ");
    Serial.println(myIP);
    server.on("/", handleIndex);
    server.on("/status", handleStatus);
    server.on("/nebel", handleNebel);
    server.on("/stopNebel", handleStopNebel);
    server.on("/redbutton.svg", handleRedButton);
    server.on("/graybutton.svg", handleGrayButton);
    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    server.handleClient();
}


#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
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

void handleIndex() {
    logRequest();

    static const char* index =
        "<html> \
        <head> \
            <meta name='viewport' content='width=device-width, initial-scale=1.0, user-scalable=no'> \
            <script type='text/javascript'> \
            var UPDATE_INTERVAL = 2000; \
            var UPDATE_ERROR_INTERVAL = 5000; \
            var UPDATE_NEBEL_INTERVAL = 400; \
            var runUpdates = true; \
            var updater = null; \
            var runNebelUpdates = false; \
            var nebelUpdater = null; \
            function load() { \
                focus(); \
                window.addEventListener('focus', focus); \
                window.addEventListener('blur', blur); \
                var button = document.getElementById('button'); \
                button.addEventListener('mousedown', mousedown); \
                button.addEventListener('mouseup', mouseup); \
                button.addEventListener('touchstart', mousedown); \
                button.addEventListener('touchend', mouseup); \
                document.body.addEventListener('keydown', keydown); \
                document.body.addEventListener('keyup', keyup); \
            }; \
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
            function mousedown() { \
                console.log('mousedown'); \
                runNebelUpdates = true; \
                if (nebelUpdater == null) { \
                    nebelUpdater = window.setTimeout(nebelUpdate, 1); \
                } \
            } \
            function mouseup() { \
                console.log('mouseup'); \
                runNebelUpdates = false; \
                if (nebelUpdater != null) { \
                    window.clearTimeout(nebelUpdater); \
                    nebelUpdater = null; \
                    var r = new XMLHttpRequest(); \
                    r.open('GET', 'stopNebel', true); \
                    r.send(); \
                } \
            } \
            function keydown(e) { \
                if (e.keyCode == 78) { \
                    mousedown(); \
                } \
            } \
            function keyup(e) { \
                if (e.keyCode == 78) { \
                    mouseup(); \
                } \
            } \
            function nebelUpdate() { \
                var r = new XMLHttpRequest(); \
                r.onreadystatechange = function() { \
                    if (this.readyState == 4 && this.status == 200) { \
                        if (runNebelUpdates) { \
                            nebelUpdater = window.setTimeout('nebelUpdate()', UPDATE_NEBEL_INTERVAL); \
                        } \
                    } else if (this.readyState == 4) { \
                    } \
                }; \
                r.timeout = 1000; \
                r.open('GET', 'nebel', true); \
                r.send(); \
            } \
            </script> \
        </head> \
        <body onload='load()' style='font-family: Arial'> \
            <h1 id='status'></h1> \
            <!--<button style='width: 100\%; font-size: 3em; padding: 30px' id='button'>Power Diggha!</button>-->  \
            <div id='button' style='width: 300px; height: 300px; margin-left: auto; margin-right: auto; background: url(redbutton.svg); -webkit-touch-callout:none;-webkit-user-select:none;-khtml-user-select:none;-moz-user-select:none;-ms-user-select:none;user-select:none;-webkit-tap-highlight-color:rgba(0,0,0,0);'></div> \
            <!--<form action='/nebel'> \
                <input type='submit' value='Nebel!' /> \
            </form>--> \
            <div style='margin-top: 10px'>ESP Uptime: <span id='uptime'></span></div> \
        </body> \
        </html>";
    server.send(200, "text/html", index);
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

void handleRedbutton() {
    logRequest();

    server.send(200, "image/svg+xml", "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?><!-- Created with Inkscape (http://www.inkscape.org/) --><svg   xmlns:dc=\"http://purl.org/dc/elements/1.1/\"   xmlns:cc=\"http://web.resource.org/cc/\"   xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"   xmlns:svg=\"http://www.w3.org/2000/svg\"   xmlns=\"http://www.w3.org/2000/svg\"   xmlns:xlink=\"http://www.w3.org/1999/xlink\"   xmlns:sodipodi=\"http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd\"   xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\"   width=\"299.99991\"   height=\"299.99991\"   id=\"svg2\"   sodipodi:version=\"0.32\"   inkscape:version=\"0.44.1\"   version=\"1.0\"   sodipodi:docbase=\"f:\wiki\graphics\"   sodipodi:docname=\"redbutton.svg\">  <defs     id=\"defs4\">    <linearGradient       id=\"linearGradient2782\">      <stop         id=\"stop2788\"         offset=\"0\"         style=\"stop-color:white;stop-opacity:0;\" />      <stop         id=\"stop2790\"         offset=\"1\"         style=\"stop-color:white;stop-opacity:1;\" />    </linearGradient>    <linearGradient       id=\"linearGradient2760\">      <stop         style=\"stop-color:#fbcaca;stop-opacity:1;\"         offset=\"0\"         id=\"stop2762\" />      <stop         id=\"stop2770\"         offset=\"0.35964912\"         style=\"stop-color:#f78e8e;stop-opacity:1;\" />      <stop         style=\"stop-color:#f57070;stop-opacity:1;\"         offset=\"0.79216683\"         id=\"stop2772\" />      <stop         style=\"stop-color:#f54d4d;stop-opacity:1;\"         offset=\"1\"         id=\"stop2764\" />    </linearGradient>    <radialGradient       inkscape:collect=\"always\"       xlink:href=\"#linearGradient2760\"       id=\"radialGradient2768\"       cx=\"-100\"       cy=\"-278.33899\"       fx=\"-100\"       fy=\"-278.33899\"       r=\"155.28572\"       gradientUnits=\"userSpaceOnUse\"       gradientTransform=\"matrix(1.266666,0,0,1.733336,26.6667,204.1162)\" />    <linearGradient       inkscape:collect=\"always\"       xlink:href=\"#linearGradient2782\"       id=\"linearGradient4577\"       x1=\"191.45982\"       y1=\"155.84966\"       x2=\"147.98438\"       y2=\"10\"       gradientUnits=\"userSpaceOnUse\" />  </defs>  <sodipodi:namedview     id=\"base\"     pagecolor=\"#ffffff\"     bordercolor=\"#666666\"     borderopacity=\"1.0\"     gridtolerance=\"10000\"     guidetolerance=\"10\"     objecttolerance=\"10\"     inkscape:pageopacity=\"0.0\"     inkscape:pageshadow=\"2\"     inkscape:zoom=\"1.4\"     inkscape:cx=\"138.21425\"     inkscape:cy=\"148.57143\"     inkscape:document-units=\"px\"     inkscape:current-layer=\"layer1\"     width=\"300px\"     height=\"300px\"     inkscape:window-width=\"1024\"     inkscape:window-height=\"667\"     inkscape:window-x=\"-4\"     inkscape:window-y=\"26\" />  <metadata     id=\"metadata7\">    <rdf:RDF>      <cc:Work         rdf:about=\"\">        <dc:format>image/svg+xml</dc:format>        <dc:type           rdf:resource=\"http://purl.org/dc/dcmitype/StillImage\" />      </cc:Work>    </rdf:RDF>  </metadata>  <g     inkscape:label=\"Layer 1\"     inkscape:groupmode=\"layer\"     id=\"layer1\"     transform=\"translate(255.2856,538.9997)\">    <path       sodipodi:type=\"arc\"       style=\"opacity:1;fill:url(#radialGradient2768);fill-opacity:1;stroke:#b01e1e;stroke-width:4.14135456;stroke-miterlimit:4;stroke-dasharray:none;stroke-opacity:1\"       id=\"path1872\"       sodipodi:cx=\"-100\"       sodipodi:cy=\"-394.28571\"       sodipodi:rx=\"154.28572\"       sodipodi:ry=\"154.28572\"       d=\"M 54.285721 -394.28571 A 154.28572 154.28572 0 1 1  -254.28572,-394.28571 A 154.28572 154.28572 0 1 1  54.285721 -394.28571 z\"       transform=\"matrix(0.949753,0,0,0.949753,-10.31027,-14.52566)\" />    <path       style=\"opacity:1;fill:url(#linearGradient4577);fill-opacity:1.0;stroke:none;stroke-width:4.14135456;stroke-miterlimit:4;stroke-dasharray:none;stroke-opacity:1\"       d=\"M 150,10 C 73.677722,10 11.559193,71.166428 10.03125,147.125 C 48.624417,181.81242 87.062309,183.29266 106.43304,174.29911 C 126.43304,165.01338 142.12947,135.71429 167.84375,129.28572 C 193.55803,122.85715 207.84822,124.28571 249.27679,140.71429 C 271.55223,149.54765 269.14726,118.59045 285.9375,116.46875 C 270.89584,55.370801 215.71909,9.9999999 150,10 z \"       transform=\"translate(-255.2856,-538.9997)\"       id=\"path2796\"       sodipodi:nodetypes=\"ccssscc\" />  </g></svg>");
}

void setup() {
    Serial.begin(115200);
    delay(5000);
    pinMode(PIN_NEBEL_AVAILABLE, INPUT);
    pinMode(PIN_LED, OUTPUT);
    pinMode(PIN_NEBEL_RELAY, OUTPUT);
    setNebelRelay(false);
    ticker.attach_ms(500, update);

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
    server.on("/redbutton.svg", handleRedbutton);
    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    server.handleClient();
}


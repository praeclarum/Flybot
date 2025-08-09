#define CONFIG_ASYNC_TCP_RUNNING_CORE 1
#include <ESPAsyncWebServer.h>

#include <algorithm>

using namespace std;

static AsyncWebServer server(80);
// static AsyncLoggingMiddleware requestLogger;
static AsyncWebSocketMessageHandler wsHandler;
static AsyncWebSocket ws("/ws", wsHandler.eventHandler());

static const char *cssContent PROGMEM = R"(
body {
    min-height: 100vh;
    margin: 0;
    padding: 0;
    font-family: 'Segoe UI', Arial, sans-serif;
    background: linear-gradient(180deg, #87ceeb 0%, #b3e0ff 50%, #f0f8ff 100%);
}
h1 {
    color: #1565c0;
    text-shadow: 0 2px 8px #b3e0ff;
}
input, button {
    font-size: 1.1em;
    padding: 0.4em 0.8em;
    border-radius: 6px;
    border: 1px solid #90caf9;
    margin: 0.5em 0;
}
button {
    background: #90caf9;
    color: #1565c0;
    cursor: pointer;
    box-shadow: 0 2px 6px #b3e0ff;
    border: none;
}
)";
static const size_t cssContentLength = strlen_P(cssContent);

static const char *jsContent PROGMEM = R"(

let state = {
    rollDegrees: 0.0,
    pitchDegrees: 0.0,
    yawDegrees: 0.0,
    throttlePercent: 0.0,
    armed: false,
};

//
// COMMUNICATIONS
//
var ws = new WebSocket('ws://flightcontroller.local/ws');
ws.onopen = function() {
    console.log("WebSocket connected");
};
ws.onmessage = function(event) {
    var data = JSON.parse(event.data);
    console.log("WebSocket message: ", data);
    if (data.type === "state") {
        state.rollDegrees = data.rd;
        state.pitchDegrees = data.pd;
        state.yawDegrees = data.yd;
        state.throttlePercent = data.th;
        state.armed = data.a;
        drawAll();
    }
};
ws.onclose = function() {
    console.log("WebSocket closed");
};
ws.onerror = function(error) {
    console.log("WebSocket error: " + error);
};
function sendCommand() {
    var command = document.getElementById("command").value;
    ws.send("command " + command);
    console.log("WebSocket sent: " + command);
}
function getState() {
    ws.send("state");
}

//
// HUD
//
function drawAll() {
    drawHUD();
}
function drawHUD() {
    const $hud = document.getElementById("hud");
    if (!$hud) return;
    const ctx = $hud.getContext("2d");
    const width = $hud.width;
    const height = $hud.height;
    ctx.clearRect(0, 0, width, height);
    ctx.fillStyle = "#ffffff";
    ctx.fillRect(0, 0, width, height);
    ctx.fillStyle = "#000000";
    ctx.font = "16px Arial";
    ctx.fillText("Roll: " + state.rollDegrees.toFixed(1) + "°", 10, 20);
    ctx.fillText("Pitch: " + state.pitchDegrees.toFixed(1) + "°", 10, 40);
    ctx.fillText("Yaw: " + state.yawDegrees.toFixed(1) + "°", 10, 60);
    ctx.fillText("Throttle: " + state.throttlePercent.toFixed(1) + "%", 10, 80);
    ctx.fillText("Armed: " + (state.armed ? "Yes" : "No"), 10, 100);
}

drawAll();
)";
static const size_t jsContentLength = strlen_P(jsContent);

static const char *htmlContent PROGMEM = R"(<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Flybot</title>
    <link rel="stylesheet" type="text/css" href="style.css" />
</head>
<body>
    <h1>Flybot</h1>
    <div>
        <canvas id="hud" width="480" height="320"></canvas>
        <button onclick='getState()'>Update</button>
    </div>
    <div>
    <input type="text" id="command" placeholder="Send a command">
    <button onclick='sendCommand()'>Send</button>
    </div>
    <script type="text/javascript" src="script.js"></script>
</body>
</html>
)";
static const size_t htmlContentLength = strlen_P(htmlContent);

void webServerBegin() {
    // requestLogger.setEnabled(true);
    // requestLogger.setOutput(Serial);
    // server.addMiddleware(&requestLogger);
    
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", (const uint8_t *)htmlContent, htmlContentLength);
    });
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/css", (const uint8_t *)cssContent, cssContentLength);
    });
    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "application/javascript", (const uint8_t *)jsContent, jsContentLength);
    });
    
    wsHandler.onConnect([](AsyncWebSocket *server, AsyncWebSocketClient *client) {
        server->text(client->id(), "{\"type\":\"hello\"}");
    });
    wsHandler.onDisconnect([](AsyncWebSocket *server, uint32_t clientId) {
        server->text(clientId, "{\"type\":\"goodbye\"}");
    });
    wsHandler.onError([](AsyncWebSocket *server, AsyncWebSocketClient *client, uint16_t errorCode, const char *reason, size_t len) {
    });
    wsHandler.onMessage([](AsyncWebSocket *server, AsyncWebSocketClient *client, const uint8_t *data, size_t len) {
        if (strncmp((const char *)data, "state", min((size_t)5, len)) == 0) {
            String stateData = "{\"type\":\"state\",\"rd\":0.0,\"pd\":0.0,\"yd\":0.0,\"th\":0.0,\"a\":false}";
            server->text(client->id(), stateData);
            return;
        }
        server->text(client->id(), "{\"type\":\"echo\",\"d\":\"" + String((const char *)data, len) + "\"}");
    });
    wsHandler.onFragment([](AsyncWebSocket *server, AsyncWebSocketClient *client, const AwsFrameInfo *frameInfo, const uint8_t *data, size_t len) {
    });

    server.addHandler(&ws);
    server.begin();
}

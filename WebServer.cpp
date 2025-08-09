#define CONFIG_ASYNC_TCP_RUNNING_CORE 1
#include <ESPAsyncWebServer.h>

static AsyncWebServer server(80);
// static AsyncLoggingMiddleware requestLogger;
static AsyncWebSocketMessageHandler wsHandler;
static AsyncWebSocket ws("/ws", wsHandler.eventHandler());

static const char *htmlContent PROGMEM = R"(<!DOCTYPE html>
<html>
<head>
  <title>WebSocket</title>
</head>
<body>
  <h1>WebSocket Example</h1>
  <p>Open your browser console!</p>
  <input type="text" id="message" placeholder="Type a message">
  <button onclick='sendMessage()'>Send</button>
  <script>
    var ws = new WebSocket('ws://flightcontroller.local/ws');
    ws.onopen = function() {
      console.log("WebSocket connected");
    };
    ws.onmessage = function(event) {
      console.log("WebSocket message: " + event.data);
    };
    ws.onclose = function() {
      console.log("WebSocket closed");
    };
    ws.onerror = function(error) {
      console.log("WebSocket error: " + error);
    };
    function sendMessage() {
      var message = document.getElementById("message").value;
      ws.send(message);
      console.log("WebSocket sent: " + message);
    }
  </script>
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
    
    wsHandler.onConnect([](AsyncWebSocket *server, AsyncWebSocketClient *client) {
        Serial.printf("Client %" PRIu32 " connected\n", client->id());
        server->textAll("New client: " + String(client->id()));
    });
    wsHandler.onDisconnect([](AsyncWebSocket *server, uint32_t clientId) {
        Serial.printf("Client %" PRIu32 " disconnected\n", clientId);
        server->textAll("Client " + String(clientId) + " disconnected");
    });
    wsHandler.onError([](AsyncWebSocket *server, AsyncWebSocketClient *client, uint16_t errorCode, const char *reason, size_t len) {
        Serial.printf("Client %" PRIu32 " error: %" PRIu16 ": %s\n", client->id(), errorCode, reason);
    });
    wsHandler.onMessage([](AsyncWebSocket *server, AsyncWebSocketClient *client, const uint8_t *data, size_t len) {
        Serial.printf("Client %" PRIu32 " data: %s\n", client->id(), (const char *)data);
        server->textAll("Right back at you: " + String((const char *)data, len));
    });
    wsHandler.onFragment([](AsyncWebSocket *server, AsyncWebSocketClient *client, const AwsFrameInfo *frameInfo, const uint8_t *data, size_t len) {
        Serial.printf("Client %" PRIu32 " fragment %" PRIu32 ": %s\n", client->id(), frameInfo->num, (const char *)data);
    });

    server.addHandler(&ws);
    server.begin();
}

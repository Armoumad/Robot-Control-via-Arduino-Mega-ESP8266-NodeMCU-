#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid ="ARMD";      // Replace with your WiFi name
const char* password ="12345678";   // Replace with your WiFi password

ESP8266WebServer server(80);

// UART Buffer
#define LINE_BUFFER_LENGTH 64
char lineBuffer[LINE_BUFFER_LENGTH];
int bufferIndex = 0;

void setup() {
  Serial.begin(115200);  // UART communication with Arduino Mega

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  // Setup web server routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/move", HTTP_GET, handleMove);
  server.begin();

  Serial.println("NodeMCU Ready");
}

void loop() {
  server.handleClient();

  // Handle incoming UART messages from Arduino
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (bufferIndex > 0) {
        lineBuffer[bufferIndex] = '\0';
        processSerialData(lineBuffer);
        bufferIndex = 0;
      }
    } else if (bufferIndex < LINE_BUFFER_LENGTH - 1) {
      lineBuffer[bufferIndex++] = c;
    }
  }
}

void handleRoot() {
  String html = "<html><head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  html += "body { font-family: Arial; text-align: center; margin-top: 50px; }";
  html += ".btn { padding: 20px 40px; margin: 5px; font-size: 20px; }";
  html += "</style></head><body>";
  html += "<h1>Robot Control</h1>";
  html += "<div><button class='btn' onclick='move(\"F\")'>Forward</button></div>";
  html += "<div>";
  html += "<button class='btn' onclick='move(\"L\")'>Left</button>";
  html += "<button class='btn' onclick='move(\"S\")'>Stop</button>";
  html += "<button class='btn' onclick='move(\"R\")'>Right</button>";
  html += "</div>";
  html += "<div><button class='btn' onclick='move(\"B\")'>Backward</button></div>";
  html += "<script>";
  html += "function move(dir) {";
  html += "  fetch('/move?dir=' + dir)";
  html += "    .then(response => console.log('Movement sent'))";
  html += "    .catch(error => console.error('Error:', error));";
  html += "}";
  html += "</script></body></html>";
  server.send(200, "text/html", html);
}

void handleMove() {
  if (server.hasArg("dir")) {
    String direction = server.arg("dir");
    Serial.println(direction);  // Send to Arduino through UART
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Missing direction parameter");
  }
}

void processSerialData(char* data) {
  // Process any responses from Arduino if needed
  if (strstr(data, "MEGA_MOTOR_V1.0") != NULL) {
    // Version information received
  }
}
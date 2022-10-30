#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>

#define SERIAL_BAUD_RATE 1200

const char* _ssid = "xxx";
const char* _password = "yyy";

ESP8266WebServer _httpServer(80);
ESP8266HTTPUpdateServer _httpUpdater;

#define SEND_IDLE_INTERVAL_MILLIS 2500
uint32_t _nextSendIDLEMillis = 0;

#define INCOMING_DATA_BUFFER_LENGTH 500
char _incomingData[INCOMING_DATA_BUFFER_LENGTH] = { 0 };
uint16_t _incomingDataLength = 0;
uint16_t _numOverflows = 0;

void setup(void)
{
  Serial.begin(SERIAL_BAUD_RATE);
  // Serial.println();
  // Serial.println("Starting");
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(_ssid, _password);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    WiFi.begin(_ssid, _password);
    // Serial.println("WiFi connect failed, retrying");
    delay(500);
  }

  _httpUpdater.setup(&_httpServer);

  _httpServer.on("/", handleRoot);

  _httpServer.begin();

  // Serial.println("WiFi connected");
  // Serial.print("IP address: ");
  // Serial.println(WiFi.localIP());

  _nextSendIDLEMillis = millis() + SEND_IDLE_INTERVAL_MILLIS;

  // Serial.println("Setup complete");
}

void handleRoot()
{
  _incomingData[_incomingDataLength] = 0;
  _httpServer.send(200, "text/html", "<html><body>" + String(_incomingData) + "<br>Length: " + String(strlen(_incomingData)) + "<br># overflows: " + String(_numOverflows) + "<br><br><a href=\"/update\">Update firmware</a></html></body>");
  _incomingDataLength = 0;
  _numOverflows = 0;
}

void loop(void)
{
  _httpServer.handleClient();

  if (_nextSendIDLEMillis <= millis())
  {
    _nextSendIDLEMillis = millis() + SEND_IDLE_INTERVAL_MILLIS;
    // Serial.println("IDLE");
  }

  if (Serial.available())
  {
    char incomingChar = Serial.read();

    uint8_t incomingCharASCIIValue = (uint8_t)incomingChar;
    if (incomingCharASCIIValue < 32 || incomingCharASCIIValue > 126) return;

    // Serial.println("[" + String(incomingChar) + "]");

    if (_incomingDataLength == INCOMING_DATA_BUFFER_LENGTH - 1)
    {
      _incomingDataLength = 0;
      _numOverflows++;
    }
    _incomingData[_incomingDataLength] = incomingChar;
    _incomingDataLength++;

    digitalWrite(LED_BUILTIN, LOW);
    delay(5);
    digitalWrite(LED_BUILTIN, HIGH);
  }
}

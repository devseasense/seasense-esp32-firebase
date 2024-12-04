#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFiS3.h>
#include <ArduinoHttpClient.h>

// WiFi credentials
const char* ssid = "SEASENSE";
const char* password = "12345678";

// Firebase project details
const char* server = "example.firebasedatabase.app";
const int port = 443;

// Temperature sensor pin
#define ONE_WIRE_BUS 2

// Initialize temperature sensor
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Time variables
unsigned long lastTime = 0;
unsigned long timerDelay = 800;

WiFiSSLClient wifi;
HttpClient client = HttpClient(wifi, server, port);

void setupWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
}

void sendToFirebase(float temperature) {
  String path = "/sensor_data.json";
  String contentType = "application/json";
  String postData = "{\"temperature\":" + String(temperature) + "}";

  client.post(path, contentType, postData);

  // Read the response
  int statusCode = client.responseStatusCode();
  String response = client.responseBody();

  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Renesas Temperature Monitor Starting...");

  // Start temperature sensor
  sensors.begin();

  // Connect to WiFi
  setupWiFi();
}

void loop() {
  unsigned long currentTime = millis();

  if (currentTime - lastTime >= timerDelay) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi disconnected. Reconnecting...");
      setupWiFi();
    }

    // Request temperature readings
    sensors.requestTemperatures();
    float tempC = sensors.getTempCByIndex(0);

    // Check if reading failed
    if (tempC == -127.00) {
      Serial.println("Error: Could not read temperature data");
      delay(1000);
      return;
    }

    // Print temperature readings
    Serial.print("Temperature: ");
    Serial.print(tempC, 1);
    Serial.println("°C");

    // Send data to Firebase
    sendToFirebase(tempC);

    lastTime = currentTime;
  }
}

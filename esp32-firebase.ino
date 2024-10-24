#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h> // Provides token status callback functions
#include <addons/RTDBHelper.h>  // Provides RTDB status callback functions

// WiFi credentials
const char* ssid = "BoyPablo";
const char* password = "_Wachitooo777";

// Firebase project credentials
#define API_KEY "g4EoSEaptOXyFrPpsJ1LKjCZzJBN5NMfr8Ibyxss" // Your Firebase project's Web API key
#define DATABASE_URL "https://geomapper-d2b26-default-rtdb.asia-southeast1.firebasedatabase.app/" // Firebase RTDB URL

// Initialize Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Temperature sensor pin
#define ONE_WIRE_BUS 2

// Initialize temperature sensor
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Time variables
unsigned long lastTime = 0;
unsigned long timerDelay = 800;

// Initialize Firebase and WiFi connection
void setupWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
}

void setupFirebase() {
  // Set the API key for Firebase project
  config.api_key = API_KEY;

  // Assign the RTDB URL
  config.database_url = DATABASE_URL;

  // Enable anonymous sign-in
  auth.user.email = "";
  auth.user.password = "";

  // Assign callback functions
  config.token_status_callback = tokenStatusCallback; // Get Firebase token info

  // Initialize Firebase
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void sendToFirebase(float temperature) {
  if (Firebase.ready()) {
    // Path in Firebase RTDB
    String path = "/sensor_data";

    // Send data to Firebase Realtime Database
    if (Firebase.RTDB.pushFloat(&fbdo, path.c_str(), temperature)) {
      Serial.println("Data sent successfully");
    } else {
      Serial.print("Error sending data: ");
      Serial.println(fbdo.errorReason());
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Renesas Temperature Monitor Starting...");

  // Start temperature sensor
  sensors.begin();

  // Connect to WiFi
  setupWiFi();

  // Initialize Firebase
  setupFirebase();
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

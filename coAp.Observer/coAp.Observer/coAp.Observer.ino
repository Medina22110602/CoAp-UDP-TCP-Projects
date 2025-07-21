#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <coap-simple.h>
#include <math.h>

// Wi-Fi credentials
const char* ssid = "Intelory WiFi";
const char* password = "LuTa6OF0@INT";

// Initialize Wi-Fi UDP and CoAP
WiFiUDP udp;
Coap coap(udp, 5683);  // 5683 is the standard CoAP port

// Grove Temperature Sensor Pin
#define TEMP_SENSOR_PIN A0  // Analog pin for temperature sensor

// Thermistor characteristics
const int B = 4275000;       // B value of the thermistor
const int R0 = 1000;         // R0 = 100k

// Number of readings to average for stable temperature
#define NUM_SAMPLES 10

// Function to get the average temperature from the thermistor
float getAverageTemperatureThermistor() {
  float sum = 0.0;

  for (int i = 0; i < NUM_SAMPLES; i++) {
    int a = analogRead(TEMP_SENSOR_PIN);
    float R = 1023.0 / a - 1.0;
    R = R0 * R;
    float temperature = 1.0 / (log(R / R0) / B + 1 / 298.15) - 273.15; // convert to temperature via datasheet

    sum += temperature;
    delay(100); // Small delay between readings
  }

  return sum / NUM_SAMPLES;
}

// Store observers
struct Observer {
  IPAddress ip;
  int port;
};

std::vector<Observer> observers;

void notifyObservers(float temperature) {
  char tempStr[8];
  dtostrf(temperature, 1, 2, tempStr);  // Convert float to string with 2 decimals

  for (const auto& observer : observers) {
    coap.sendResponse(observer.ip, observer.port, 0, tempStr, strlen(tempStr)); // Send temperature to each observer
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Connecting to WiFi...");

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);

  // Wait for connection (Max 20 seconds)
  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED && attempt < 20) {
    delay(1000);
    Serial.print(".");
    attempt++;
  }

  // Check if connected
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to WiFi!");
    Serial.print("ESP IP Address: ");
    Serial.println(WiFi.localIP()); // Print ESP8266 IP
  } else {
    Serial.println("\nFailed to connect to WiFi!");
  }

  // Start CoAP server if connected
  if (WiFi.status() == WL_CONNECTED) {
    coap.start();

    // Define CoAP server response for /temperature
    coap.server([](CoapPacket &packet, IPAddress ip, int port) {
      Serial.println("Received request for /temperature");

      // Check if it's a GET request
      if (packet.code == COAP_GET) {
        // Get the average temperature reading using the thermistor function
        float temperature = getAverageTemperatureThermistor();

        // Check for invalid temperature readings
        if (isnan(temperature)) {
          Serial.println("Error: Invalid temperature reading");
          coap.sendResponse(ip, port, packet.messageid, "Error reading temperature", 22);
        } else {
          // Convert temperature to string
          char tempStr[8];
          dtostrf(temperature, 1, 2, tempStr);  // Convert float to string with 2 decimals

          // Send the temperature as response
          Serial.print("Sending temperature: ");
          Serial.println(tempStr);
          coap.sendResponse(ip, port, packet.messageid, tempStr, strlen(tempStr));
        }
      } else if (packet.code == COAP_POST) {
        // Register observer
        observers.push_back({ip, port});
        Serial.print("Observer registered: ");
        Serial.print(ip);
        Serial.print(":");
        Serial.println(port);
      }
    }, "temperature");
  }
}

void loop() {
  coap.loop();

  // Periodically check temperature and notify observers
  static unsigned long lastNotify = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - lastNotify >= 5000) { // Notify every 5 seconds
    lastNotify = currentMillis;
    float temperature = getAverageTemperatureThermistor();
    if (!isnan(temperature)) {
      Serial.print("Notifying observers with temperature: ");
      Serial.println(temperature);
      notifyObservers(temperature);
    }
  }
}
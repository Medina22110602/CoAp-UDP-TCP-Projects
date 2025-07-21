#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>

#define WIFI_SSID "Intelory WiFi"
#define WIFI_PASSWORD "LuTa6OF0@INT"
#define UDP_SERVER_IP "192.168.50.251"
#define UDP_SERVER_PORT 474

Adafruit_BME680 bme;

void execute_at(const char *cmd, const char *expect = nullptr, int32_t timeout = 1000) {
    String response = "";
    Serial1.write(cmd);
    Serial1.flush();
    unsigned long start = millis();
    while (millis() - start < timeout) {
        while (Serial1.available()) {
            response += char(Serial1.read());
        }
    }
    Serial.println(response);
    if (expect && response.indexOf(expect) != -1) {
        Serial.println("Execute OK.");
    } else if (expect) {
        Serial.println("Execution failed");
    }
}

void wait_for_ready() {
    String response = "";
    unsigned long start = millis();
    while (millis() - start < 5000) {
        if (Serial1.available()) {
            response += char(Serial1.read());
            if (response.indexOf("ready") != -1 || response.indexOf("OK") != -1) {
                break;
            }
        }
        delay(100);
    }
}


void send_udp_data(const String &data) {
    char cmd[128];
    // Check connection status
    execute_at("AT+CIPSTATUS\r\n");
    
    // Start UDP connection
    snprintf(cmd, sizeof(cmd), "AT+CIPSTART=\"UDP\",\"%s\",%d\r\n", UDP_SERVER_IP, UDP_SERVER_PORT);
    execute_at(cmd, "OK", 5000);
    wait_for_ready();
    
    // Send the actual data
    snprintf(cmd, sizeof(cmd), "AT+CIPSEND=%d\r\n", data.length());
    execute_at(cmd, ">", 5000);
    Serial1.print(data); // Send data
    delay(10000);
    
    execute_at("AT+CIPCLOSE\r\n", "OK");
}

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 5000) delay(100);

    Serial.println("=== RAK2305 UDP Example with BME680 ===");

    Serial1.begin(115200);
    delay(1000);

    execute_at("AT+CWMODE=3\r\n", "OK");
    execute_at("AT+CWCOUNTRY=0,\"MK\",1,13\r\n", "OK");

    char cmd[128];
    snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"\r\n", WIFI_SSID, WIFI_PASSWORD);
    execute_at(cmd, "OK");

    if (!bme.begin(0x76)) {
        Serial.println("BME680 initialization failed!");
        while (1);
    }
}

void get_bme680_data(float &temperature, float &humidity, float &pressure, float &gas) {
    temperature = bme.readTemperature();
    humidity = bme.readHumidity();
    pressure = bme.readPressure() / 100.0F;
    gas = bme.readGas();

    Serial.printf("Temperature: %.2f C\nHumidity: %.2f %%\nPressure: %.2f hPa\nGas: %.2f\n", 
                  temperature, humidity, pressure, gas);
}

// void send_udp_data(const String &data) {
//     char cmd[128];
//     snprintf(cmd, sizeof(cmd), "AT+CIPSTART=\"UDP\",\"%s\",%d\r\n", UDP_SERVER_IP, UDP_SERVER_PORT);
//     execute_at(cmd, "OK");

//     snprintf(cmd, sizeof(cmd), "AT+CIPSEND=%d\r\n", data.length());
//     execute_at(cmd, ">");

//     execute_at(data.c_str(), "SEND OK");
//     execute_at("AT+CIPCLOSE\r\n", "OK");
//}

void loop() {
    float temperature, humidity, pressure, gas;
    get_bme680_data(temperature, humidity, pressure, gas);

    String data = String("Temperature: ") + temperature + " C, " +
                  "Humidity: " + humidity + " %, " +
                  "Pressure: " + pressure + " hPa, " +
                  "Gas: " + gas;

    send_udp_data(data);
    delay(5000);
}  
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>

#define WIFI_SSID "Intelory WiFi"
#define WIFI_PASSWORD "LuTa6OF0@INT"
#define TCP_SERVER_IP "192.168.50.251"
#define TCP_SERVER_PORT 473

Adafruit_BME680 bme;

void execute_at(const char *cmd, const char *expect = nullptr, int32_t timeout = 1000) {
    String response = "";
    Serial1.write(cmd);
    delay(10);

    while (timeout--) {
        if (Serial1.available()) response += Serial1.readString();
        delay(1);
    }

    Serial.println(response);
    if (expect && response.indexOf(expect) != -1) {
        Serial.println("Execute OK.");
    } else if (expect) {
        Serial.println("Execute Fail.");
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 5000) delay(100);

    Serial.println("=== RAK2305 TCP Example with BME680 ===");

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

void send_tcp_data(const String &data) {
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "AT+CIPSTART=\"TCP\",\"%s\",%d\r\n", TCP_SERVER_IP, TCP_SERVER_PORT);
    execute_at(cmd, "OK");

    snprintf(cmd, sizeof(cmd), "AT+CIPSEND=%d\r\n", data.length());
    execute_at(cmd, ">");

    execute_at(data.c_str(), "SEND OK");
    execute_at("AT+CIPCLOSE\r\n", "OK");
}

void loop() {
    float temperature, humidity, pressure, gas;
    get_bme680_data(temperature, humidity, pressure, gas);

    String data = String("Temperature: ") + temperature + " C, " +
                  "Humidity: " + humidity + " %, " +
                  "Pressure: " + pressure + " hPa, " +
                  "Gas: " + gas;

    send_tcp_data(data);
    delay(5000);
}  
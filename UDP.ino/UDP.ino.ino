//#include <Adafruit_TinyUSB.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>

#define WIFI_SSID "Intelory WiFi"
#define WIFI_PASSWORD "LuTa6OF0@INT"
#define UDP_SERVER_IP "192.168.50.251"
#define UDP_SERVER_PORT 474

Adafruit_BME680 bme;

void execute_at(const char *cmd, const char *expect = nullptr, int32_t timeout = 3000) {
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

void loop() {
    // Read sensor data
    float temperature = bme.readTemperature();
    float humidity = bme.readHumidity();
    float pressure = bme.readPressure() / 100.0F;
    float gas = bme.gas_resistance / 1000.0F;

    // Create a plain text string
    String plain_text_data = "Temperature: " + String(temperature, 2) + "C, ";
    plain_text_data += "Humidity: " + String(humidity, 2) + "%, ";
    plain_text_data += "Pressure: " + String(pressure, 2) + "hpa, ";
    plain_text_data += "Gas: " + String(gas, 2);

    // Print the plain text data to the serial monitor
    Serial.println(plain_text_data);

    // Send the plain text data via UDP
    send_udp_data(plain_text_data);

    // Wait 5 seconds before taking another reading
    delay(5000);
}









// #include <Wire.h>

// // #define WIFI_SSID "Room-905"
// // #define WIFI_PASSWORD "Mxjmxj_905"

// #define WIFI_SSID "Intelory WiFi"
// #define WIFI_PASSWORD "LuTa6OF0@INT"
// #define UDP_SERVER_IP "192.168.50.251"
// #define UDP_SERVER_PORT 474

// /**
//    @brief  execute at command
//    @param  at: the at command you want to execute
//    @param  expect: the respond you want to get
//    @param  timeout: the timout of receive respond
// */
// void execute_at(char *at, char *expect = NULL, int32_t timeout = 1000)
// {
//     String resp = "";

//     Serial1.write(at);
//     delay(10);

//     while (timeout--)
//     {
//         if (Serial1.available())
//         {
//             resp += Serial1.readString();
//         }
//         delay(1);
//     }

//     Serial.println(resp);
//     if (expect != NULL)
//     {
//         if (resp.indexOf(expect) != -1)
//         {
//             Serial.println("Execute OK.");
//         }
//         else
//         {
//             Serial.println("Execute Fail.");
//         }
//     }
//     resp = "";
// }

// /**
//    @brief Arduino setup function. Called once after power on or reset

// */
// void setup()
// {
//     char cmd[128] = "";
//     time_t timeout = millis();
//     // Open serial communications and wait for port to open:
//     Serial.begin(115200);
//     while (!Serial)
//     {
//       if ((millis() - timeout) < 5000)
//         {
//             delay(100);
//         }
//         else
//         {
//             break;
//         }
//     }

//     Serial.println("================================");
//     Serial.println("RAK2305 WiFI example");
//     Serial.println("================================");

//     Serial1.begin(115200);
//     delay(1000);

//     // Set RAK2305 as AP and STA role
//     execute_at("AT+CWMODE=3\r\n", "OK");

//     // Set contry code
//     execute_at("AT+CWCOUNTRY=0,\"CN\",1,13\r\n", "OK");

//     // Connect AP with ssid and password
//     snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"\r\n", WIFI_SSID, WIFI_PASSWORD);
//     execute_at(cmd, "OK");
    
//     snprintf(cmd, sizeof(cmd), "AT+CIPSTART=\"UDP\",\"%s\",%d\r\n", UDP_SERVER_IP, UDP_SERVER_PORT);
//     execute_at(cmd, "OK");

    

// }

// /**
//    @brief Arduino loop. Runs forever until power off or reset

// */
// void loop()
// {
//     Serial1.println("AT+PING=\"8.8.8.8\"");
//     // ping 8.8.8.8
//     execute_at("AT+QPING=1,\"8.8.8.8\"\r\n", "OK");
//     delay(5000);
//     execute_at("AT=1,\"hello\"\r\n", "OK");
//     delay(7000);
// }









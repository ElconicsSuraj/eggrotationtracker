#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid = "Elconics";
const char* password = "Elconics@123";

// ThingsBoard device credentials
const char* serverName = "http://devices.dev.antzsystems.com/api/v1/UI7havXAnRZfcqaipxwG/telemetry";  // Replace with your ThingsBoard server URL and device token

WiFiClient client;
Adafruit_MPU6050 mpu;

// Define I2C pins for ESP8266
#define SDA_PIN 4  // GPIO4 (D2)
#define SCL_PIN 5  // GPIO5 (D1)

void setup() {
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin(ssid, password);

  // Wait for WiFi connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize MPU6050
  Wire.begin(SDA_PIN, SCL_PIN);  // Use GPIO4 and GPIO5 as SDA and SCL
  if (!mpu.begin()) {
    Serial.println("MPU6050 not detected!");
    while (1) delay(10);
  }
  Serial.println("MPU6050 Initialized!");
}

void loop() {
  // Check if WiFi is connected
  if (WiFi.status() == WL_CONNECTED) {

    // Connect to the ThingsBoard server
    if (client.connect("devices.dev.antzsystems.com", 8080)) {
      
      // Read MPU6050 sensor data
      sensors_event_t a, g, temp;
      mpu.getEvent(&a, &g, &temp);

      // Create a JSON object to hold sensor data
      StaticJsonDocument<200> doc;
      doc["temperature"] = temp.temperature;  // MPU6050 internal temperature
      doc["x"] = a.acceleration.x;
      doc["y"] = a.acceleration.y;
      doc["z"] = a.acceleration.z;

      // Serialize the JSON object to a string
      String jsonString;
      serializeJson(doc, jsonString);

      // Send the HTTP POST request to ThingsBoard
      client.print("POST /api/v1/UI7havXAnRZfcqaipxwG/telemetry HTTP/1.1\r\n");
      client.print("Host: devices.dev.antzsystems.com\r\n");
      client.print("Content-Type: application/json\r\n");
      client.print("Content-Length: " + String(jsonString.length()) + "\r\n");
      client.print("\r\n");
      client.print(jsonString);

      // Wait for the response
      long timeout = millis();
      while (client.available() == 0) {
        if (millis() - timeout > 5000) {
          Serial.println(">>> Client Timeout!");
          client.stop();
          return;
        }
      }

      // Read the response from ThingsBoard
      String response = "";
      while (client.available()) {
        response += char(client.read());
      }

      Serial.println(response);  // Print the response from ThingsBoard
    } else {
      Serial.println("Connection failed!");
    }

    // Close the connection
    client.stop();
  } else {
    Serial.println("Error in WiFi connection");
  }

  // Wait 10 seconds before sending the next request
  delay(10000);
}

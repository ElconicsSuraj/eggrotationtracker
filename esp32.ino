#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* ssid = "Elconics";
const char* password = "Elconics@123";
const char* mqtt_server = "broker.emqx.io";  // MQTT Broker address

WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_MPU6050 mpu;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  client.setServer(mqtt_server, 1883);

  if (!mpu.begin()) {
    Serial.println("MPU6050 not detected!");
    while (1) delay(10);
  }

  Serial.println("MPU6050 Initialized!");
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32_Client")) {
      Serial.println("Connected to MQTT broker!");
    } else {
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Create a JSON object
  StaticJsonDocument<200> doc;
  doc["x"] = a.acceleration.x;
  doc["y"] = a.acceleration.y;
  doc["z"] = a.acceleration.z;

  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);  // Serialize the JSON to a buffer

  // Publish to the topic "downlink/rotation"
  client.publish("downlink/rotation", jsonBuffer);

  delay(1000);  // Publish data every 1 second
}

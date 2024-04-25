#include <M5StickCPlus.h> //for M5StickC Begin and macros
#include <WiFi.h> //for connecting to wifi
#include <vector> //for vector storage
#include <NTPClient.h> //for getting unix epoc time
#include <WiFiUdp.h> //dependency of NTPClient
#include "DHT.h" //to streamline DHT snesor reading


WiFiUDP ntpUDP; //for getting unix epoch time
NTPClient timeClient(ntpUDP); //register for unix epoc time

const char* ssid = "Test_SSID";
const char* password = "Password";
const char* host = "192.168.0.146"; // Server IP
const uint16_t port = 12345; // Server port

#define DHTPIN 0
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

std::vector<String> dataBuffer; // Buffer to store data when offline

void setup() {
  M5.begin();
  dht.begin();
  Serial.begin(74880);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  timeClient.update();
  long currentEpoch = timeClient.getEpochTime(); // Seconds since Jan 1, 1970
  long currentMillis = millis(); // Milliseconds since MCU turned on
  long currentTime = currentEpoch * 1000 + currentMillis % 1000; // Approximate current time in milliseconds
  delay(2100);
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  if (isnan(humidity) || isnan(temperature)) {Serial.println("Failed to read from DHT sensor!");return;
  }
  // Format data with timestamp
  String data = String(millis()) + "," + String(temperature) + "," + String(humidity);
  // Check WiFi connection
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;

      // Print the readings in the Serial Monitor
      Serial.print("Humidity: ");
      Serial.print(humidity);
      Serial.print("%\t");
      Serial.print("Temperature: ");
      Serial.print(temperature);
      Serial.println("°C ");

    if (client.connect(host, port)) {
      // Check if there's buffered data to send
      while (!dataBuffer.empty()) {
        client.println(dataBuffer.front());
        dataBuffer.erase(dataBuffer.begin()); // Remove sent data from buffer
      }
      // Send current data
      client.println(data);
      client.stop();
    } else {
      Serial.println("Connection to host failed");
        dataBuffer.push_back(data);
        Serial.println("Data buffered");
    }
  } else {
    // Store data in buffer if not connected
    dataBuffer.push_back(data);
    Serial.println("Data buffered");
  }
}

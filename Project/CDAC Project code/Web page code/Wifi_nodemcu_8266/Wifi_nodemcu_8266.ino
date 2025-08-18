#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>

// Replace with your WiFi network credentials
const char *ssid = "Nothing Phone";
const char *password = "Shiv2k24";

// Replace with the IP address and port of your server
// This IP must match the computer running your Flask or Node.js server.
const char* serverHost = "10.91.73.251"; 
const int serverPort = 4000;

// The endpoint on your server where data will be sent
const char* serverPath = "/sensors";

void setup() {
  // Initialize the serial port for debugging and communication with STM32
  // Make sure your STM32 TX connects to ESP8266 RX (GPIO3)
  Serial.begin(9600);
  delay(100);

  Serial.println("\nConnecting to WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  Serial.print("NodeMCU IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Check if there is data available from the serial port (from the STM32)
  if (Serial.available()) {
    // Read the entire line of data until a newline character is found
    String sensorData = Serial.readStringUntil('\n');

    // Print the received data for debugging
    Serial.print("Received from STM32: ");
    Serial.println(sensorData);

    // Find the positions of the commas to split the string
    int firstComma = sensorData.indexOf(',');
    int secondComma = sensorData.indexOf(',', firstComma + 1);

    // Ensure the data format is correct (e.g., has at least two commas)
    if (firstComma != -1 && secondComma != -1) {
      // Extract each of the three sensor values as strings
      String value1Str = sensorData.substring(0, firstComma);
      String value2Str = sensorData.substring(firstComma + 1, secondComma);
      String value3Str = sensorData.substring(secondComma + 1);

      // Convert the string values to floating-point numbers
      float value1 = value1Str.toFloat(); // Temperature
      float value2 = value2Str.toFloat(); // Speed
      float value3 = value3Str.toFloat(); // Level

      // Construct the JSON body as a string. This is what the server expects.
      String jsonBody = "{\"location\":\"Nira\", \"sensor1\": " + String(value1) + ", \"sensor2\": " + String(value2) + ", \"sensor3\": " + String(value3) + "}";
      
      WiFiClient wifiClient;
      HTTPClient httpClient;

      // Construct the full URL for the POST request
      String postUrl = "http://" + String(serverHost) + ":" + String(serverPort) + String(serverPath);

      Serial.println("Sending data to: " + postUrl);
      Serial.println("JSON Body: " + jsonBody);
      
      // Initialize the HTTP client with the server URL
      httpClient.begin(wifiClient, postUrl);
      httpClient.addHeader("Content-Type", "application/json");

      // Send the POST request with the JSON body
      int httpResponseCode = httpClient.POST(jsonBody);
      
      // Check the status code of the response
      if (httpResponseCode > 0) {
        Serial.printf("HTTP Response Code: %d\n", httpResponseCode);
        // Print the server's response for debugging
        String payload = httpClient.getString();
        Serial.println("Server Response: " + payload);
      } else {
        Serial.printf("HTTP request failed, error: %s\n", httpClient.errorToString(httpResponseCode).c_str());
      }
      
      // End the HTTP connection to free up resources
      httpClient.end();
    }
  }
  
  // Wait a moment to avoid sending data too quickly
  delay(1000);
}

#include <HCSR04.h>
#include <WiFiNINA.h>

// HC-SR04 Sensor pins
UltraSonicDistanceSensor distanceSensor(3, 2);  

// WiFi and ThingSpeak settings
char ssid[] = "";     //deleted
char pass[] = "";  //deleted
char apiKey[] = ""; //deleted

char server[] = "api.thingspeak.com";
int status = WL_IDLE_STATUS;
WiFiClient client;

void setup() {
  Serial.begin(9600);
  
  // Connect to WiFi
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
    
    // Wait 10 seconds for connection
    delay(10000);
  }
  
  Serial.println("Connected to WiFi");
  printWifiStatus();
}

void loop() {
  // Measure distance
  float distance = distanceSensor.measureDistanceCm();
  
  // Check if reading is valid
  if (distance > 0) {
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");
    
    // Send data to ThingSpeak
    sendToThingSpeak(distance);
  } else {
    Serial.println("Error reading sensor data");
  }
  
  // ThingSpeak needs minimum 15 sec delay between updates
  delay(15000);
}

void sendToThingSpeak(float distance) {
  if (client.connect(server, 80)) {
    Serial.println("Connected to ThingSpeak");
    
    // Construct API request
    String postStr = "api_key=";
    postStr += apiKey;
    postStr += "&field1=";
    postStr += String(distance);
    
    // Print the request details for debugging
    Serial.print("Sending: ");
    Serial.println(postStr);
    
    // Send HTTP POST request
    client.println("POST /update HTTP/1.1");
    client.println("Host: api.thingspeak.com");
    client.println("Connection: close");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(postStr.length());
    client.println();
    client.println(postStr);
    
    Serial.println("Waiting for response...");
    
    // Wait for server response
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println(">>> Client Timeout !");
        client.stop();
        return;
      }
    }
    
    // Read and display the response
    Serial.println("Response from ThingSpeak:");
    while (client.available()) {
      String line = client.readStringUntil('\r');
      Serial.println(line);
    }
  } else {
    Serial.println("Connection to ThingSpeak failed");
  }
  client.stop();
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
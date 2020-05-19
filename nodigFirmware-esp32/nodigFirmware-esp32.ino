/*
  NoDig Firmware
  The MIT License (MIT)
  Copyright (c) 2020 Simon Bogutzky
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
  
  v1.1.0
*/

#include <WiFi.h>
#include <DHT.h>

const char* wiFiSsid = "";
const char* wiFiPassword = "";
const char* host = "";
const int dhtType = DHT22;
const int hostPort = 80;
const int ledPin = 5;
const int soilMoistureSensorDataPin = 35;
const int soilMoistureSensorPowerPin = 25;
const int dhtDataPin = 33;
const int waitTime = 3600000;
const int sensorId = 1;

DHT dht(dhtDataPin, dhtType);

void setup() 
{
  Serial.begin(9600);
  pinMode(soilMoistureSensorPowerPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  pinMode(ledPin, OUTPUT);
  digitalWrite(soilMoistureSensorPowerPin, LOW);
  dht.begin();
  connectToWiFi();
}

void loop() 
{
  delay(waitTime);
  
  int soilMoisture = readSoilMoisture();
  Serial.print("Soil Moisture = ");
  Serial.println(soilMoisture);

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
   
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Error while reading DHT");
    return;
  }

  Serial.print("Humidity = ");
  Serial.println(humidity);
  Serial.print("Temperature = ");
  Serial.println(temperature);

  float dataArray[] = {soilMoisture, humidity, temperature};
  sendData(dataArray);
}

void connectToWiFi()
{
  int ledState = 0;
  Serial.println("Connecting to WiFi network: " + String(wiFiSsid));
  WiFi.begin(wiFiSsid, wiFiPassword);
  while (WiFi.status() != WL_CONNECTED) 
  {
    digitalWrite(ledPin, ledState);
    ledState = (ledState + 1) % 2;
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void sendData(float dataArray[]) {
  Serial.println("Connecting to: " + String(host));
  WiFiClient client;
  if (!client.connect(host, hostPort))
  {
    Serial.println("Connection failed");
    return;
  }
  Serial.println("Connected");
  client.println("POST /api/details.json HTTP/1.1");
  client.print("Host: "); client.println(host);
  client.println("Content-Type: application/json");
  String jsonString = (String)"{\"soil_moisture\":" + (int)dataArray[0] + ",\"humidity\":" + dataArray[1] + ",\"temperature\":" + dataArray[2] + ",\"sensor_id\":" + sensorId + "}";
  int contentLength = jsonString.length();
  client.print("Content-Length:"); client.println(contentLength);
  client.println();
  Serial.println(jsonString);
  client.println(jsonString);
  
  unsigned long timeout = millis();
  while (client.available() == 0) 
  {
    if (millis() - timeout > 5000) 
    {
      Serial.println("Client timeout");
      client.stop();
      return;
    }
  }

  while (client.available()) 
  {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  Serial.println();
  Serial.println("Closing connection");
  client.stop();
}

int readSoilMoisture()
{
  digitalWrite(ledPin, HIGH);
  digitalWrite(soilMoistureSensorPowerPin, HIGH);
  delay(10);
  int soilMoisture = analogRead(soilMoistureSensorDataPin);
  digitalWrite(soilMoistureSensorPowerPin, LOW);
  digitalWrite(ledPin, LOW);
  return soilMoisture;
}

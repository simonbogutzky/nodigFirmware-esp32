/*
  NoDig Firmware
  The MIT License (MIT)
  Copyright (c) 2020 Simon Bogutzky
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
  
  v1.3.2
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include "Config.h"

const int dhtType = DHT22;
const int ledPin = 5;
const int soilMoistureSensorDataPin = 35;
const int soilMoistureSensorPowerPin = 25;
const int dhtDataPin = 33;
const int dhtPowerPin = 26;

DHT dht(dhtDataPin, dhtType);

void setup()
{
  Serial.begin(9600);
  Serial.println();
  dht.begin();
  pinMode(soilMoistureSensorPowerPin, OUTPUT);
  pinMode(dhtPowerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  digitalWrite(soilMoistureSensorPowerPin, HIGH);
  digitalWrite(dhtPowerPin, HIGH);
  digitalWrite(ledPin, HIGH);
  bool connected = connectToWiFi();

  if (connected) {
    int soilMoisture = readSoilMoisture();
    Serial.print("Soil Moisture = ");
    Serial.println(soilMoisture);

    delay(1000);
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    if (isnan(humidity) || isnan(temperature))
    {
      Serial.println("Error while reading DHT");
      humidity = -1;
      temperature = -1;
    }

    Serial.print("Humidity = ");
    Serial.println(humidity);
    Serial.print("Temperature = ");
    Serial.println(temperature);

    sendData(soilMoisture, humidity, temperature);
  }

  esp_sleep_enable_timer_wakeup(sleepTimeInMicroSeconds);
  esp_deep_sleep_start();
}

void loop()
{
}

bool connectToWiFi()
{
  int ledState = 0;
  Serial.println("Connecting to WiFi network: " + String(wiFiSsid));
  WiFi.begin(wiFiSsid, wiFiPassword);

  unsigned long timeout = millis();
  while (WiFi.status() != WL_CONNECTED)
  {
    if (millis() - timeout > 10000)
    {
      Serial.println();
      Serial.println("WiFi connection timeout");
      return false;
    }

    digitalWrite(ledPin, ledState);
    ledState = (ledState + 1) % 2;
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  return true;
}

void sendData(int soilMoisture, float humidity, float temperature)
{
  Serial.println("Send to: " + String(host));

  HTTPClient http;
  
  http.begin(host);
  http.addHeader("Content-Type", "application/json");
  String jsonString;
  
  if (isnan(humidity) || isnan(temperature))
  {
    jsonString = (String)"{\"soil_moisture\":" + soilMoisture + "}";
  }
  else
  {
    jsonString = (String)"{\"soil_moisture\":" + soilMoisture + ",\"humidity\":" + humidity + ",\"temperature\":" + temperature + "}";
  }
  
  int httpResponseCode = http.POST(jsonString);
  Serial.println(httpResponseCode);
  http.end();
}

int readSoilMoisture()
{
  delay(10);
  int soilMoisture = analogRead(soilMoistureSensorDataPin);
  return soilMoisture;
}

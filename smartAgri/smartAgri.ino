#include <DHT.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
// PINS ----------------------------------------------------------------------
#define DHTPIN 19
#define DHTTYPE DHT11
#define waterLevelSensorPin 35
#define rainSensorPin 32
#define moistureSensorPin 33
#define smokeSensorPin 34
#define tempSensorPin 19
//____________________________________________________________________________


// WIFI ----------------------------------------------------------------------
const char* ssid = "POCO F3";
const char* password = "LE GAI SAVOIR";
const char* serverUrl = "https://shpserver.pythonanywhere.com/smartAgricole";
//____________________________________________________________________________


DHT dht(DHTPIN, DHTTYPE);

// variables -----------------------------------------------------------------
int waterLevel = 0;
int rain = 0;
int soilMoisture = 0;
int smoke = 0;
float tempCelsiusDHT = 0;
float outsideRoomHumidity = 0;
//____________________________________________________________________________


void connectToWifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

String sendJson(String json) {
  if (WiFi.status() != WL_CONNECTED) {
    connectToWifi();
  }

  HTTPClient http;
  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.POST(json);
  String response;

  if (httpResponseCode > 0) {
    Serial.print("\n---> HTTP Response code: ");
    Serial.println(httpResponseCode);
    response = http.getString();
  } else {
    Serial.print("\n---> HTTP Request failed, error: ");
    Serial.println(httpResponseCode);
    response = "Error " + String(httpResponseCode);
    http.end();
    sendJson(json);
  }

  http.end();
  return response;
}

void reconnect() {
  if (WiFi.status() != WL_CONNECTED) {
    connectToWifi();
  }
}

String prepareJson() {
  DynamicJsonDocument jsonDoc(200);
  jsonDoc["waterLevel"] = waterLevel;
  jsonDoc["rainDetection"] = rain;
  jsonDoc["soilMoisture"] = soilMoisture;
  jsonDoc["outsideRoomSmoke"] = smoke;
  jsonDoc["outsideRoomTemperature"] = tempCelsiusDHT;
  jsonDoc["outsideRoomHumidity"] = outsideRoomHumidity;

  String jsonData;
  serializeJson(jsonDoc, jsonData);
  return jsonData;
}

void readValues_assigns_prints() {
  waterLevel = analogRead(waterLevelSensorPin);
  Serial.print("\n\nWater Level: \t\t");
  Serial.println(waterLevel);

  rain = analogRead(rainSensorPin);
  Serial.print("Rain: \t\t\t");
  Serial.println(rain);

  soilMoisture = analogRead(moistureSensorPin);
  Serial.print("Soil Moisture: \t\t");
  Serial.println(soilMoisture);

  smoke = analogRead(smokeSensorPin);
  Serial.print("Smoke: \t\t\t");
  Serial.println(smoke);

  float tempCelsiusDHTtmp = dht.readTemperature();
  tempCelsiusDHT = isnan(tempCelsiusDHTtmp) ? tempCelsiusDHT : tempCelsiusDHTtmp;
  Serial.print("Temperature (DHT): \t");
  Serial.println(tempCelsiusDHT);

  float outsideRoomHumiditytmp = dht.readHumidity();
  outsideRoomHumidity = isnan(outsideRoomHumiditytmp) ? outsideRoomHumidity : outsideRoomHumiditytmp;
  Serial.print("Humidite (DHT): \t");
  Serial.println(outsideRoomHumidity);
}

void setup() {
  Serial.begin(115200);
  connectToWifi();
  dht.begin();
  pinMode(waterLevelSensorPin, INPUT);
  pinMode(rainSensorPin, INPUT);
  pinMode(moistureSensorPin, INPUT);
  pinMode(smokeSensorPin, INPUT);
}

void loop() {
  reconnect();
  readValues_assigns_prints();
  sendJson(prepareJson());
  delay(1000);
}

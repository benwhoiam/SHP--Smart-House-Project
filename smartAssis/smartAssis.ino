#include <DHT.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Pins
#define RELAYPIN 23
#define DHTPIN 19
#define SMOKEPIN 34
#define DHTTYPE DHT11
#define LIGHTPIN 18
#define BUTTONPIN 12
#define BUZZERPIN 21
//initiate

DHT dht(DHTPIN, DHTTYPE);
TFT_eSPI tft = TFT_eSPI();
#include "logo_aero.h"
#include "smoke_img.h"
// variables
const char* ssid = "POCO F3";
const char* password = "LE GAI SAVOIR";
const char* serverUrl = "https://shpserver.pythonanywhere.com/smartAssistant";
String Response;

float insideRoomTemperature = 0;  // to server
float insideRoomHumidity = 0;     // to server
bool insideRoomSmoke = LOW;       // to server
bool light = LOW;                 // to server
float roomBrightness;             // from server
bool fan;                         // to server
bool someoneInRoom;               // from server
bool confirmClosingNotification;  // from server
bool closeConfirmation;
float threshold = 3000;
// variables local
int smoke_value;
float seuilTemperature = 25;
int buttonState = 0;
int startTime = 0;
int windowDuration = 60000;
int smokeValue;
//functions
String buildJson() {
  // Créer un objet JSON
  DynamicJsonDocument doc(1024);

  // Ajouter des données au JSON

  doc["insideRoomTemperature"] = insideRoomTemperature;
  doc["insideRoomHumidity"] = insideRoomHumidity;
  doc["insideRoomSmoke"] = smokeValue;
  doc["closeConfirmation"] = closeConfirmation;
  


  // Convertir le JSON en chaîne
  String json;
  serializeJson(doc, json);

  return json;
}
void connectToWiFi() {
  WiFi.begin(ssid, password);
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
    if (i == 10) {
      Serial.println("Not Connected to WiFi");
      return;
    }
  }

  Serial.println("Connected to WiFi");
}

void sendJSONToServer(String data) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(data);

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);

      Response = http.getString();
      Serial.println(Response);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
      http.end();
      sendJSONToServer(data);
    }

    http.end();
  }
}
void processServerResponse(const String& Response) {
  DynamicJsonDocument doc(1024);

  // Parse the JSON response
  DeserializationError error = deserializeJson(doc, Response);

  // Check for parsing errors
  if (error) {
    Serial.print(F("Failed to parse JSON: "));
    Serial.println(error.c_str());
    return;
  }

  // Extract information from the JSON document
  String confirmClosingNotificationString = doc["confirmClosingNotification"].as<String>();
  confirmClosingNotification = (confirmClosingNotificationString.equalsIgnoreCase("True")) ? true : false;
  String someoneInRoomString = doc["someoneInRoom"].as<String>();
  someoneInRoom = (someoneInRoomString.equalsIgnoreCase("True")) ? true : false;
  roomBrightness = doc["roomBrightness"];
  threshold = doc["threshold"];

  // Use the extracted values as needed
  Serial.print("Confirm Closing Notification: ");
  Serial.println(confirmClosingNotification);

  Serial.print("Room Brightness: ");
  Serial.println(roomBrightness);

  Serial.print("someoneInRoom: ");
  Serial.println(someoneInRoom);

  Serial.print("threshold: ");
  Serial.println(threshold);
}
void light_on_off() {
  if (light == LOW) {
    // If the lights are off and a person is detected with low brightness, turn on the lights
    if (someoneInRoom == 1 && roomBrightness < 50) {
      digitalWrite(LIGHTPIN, HIGH);
      light = HIGH;
      Serial.print("ch3al do");
    }
  } else if (light == HIGH) {
    // If the lights are on and no person is detected or brightness is high, turn off the lights
    if (someoneInRoom == LOW) {
      digitalWrite(LIGHTPIN, LOW);
      light = LOW;
      Serial.print("tfi do");
    }
  }
}
void CloseNotification() {
  if (confirmClosingNotification == 1) {
    Serial.print("notif");
    display_notification();
    startTime = millis();

    // Attendre que la fenêtre de temps se ferme ou que le bouton soit pressé
    while ((millis() - startTime < windowDuration) && (closeConfirmation == 0)) {
      // Lire l'état du bouton poussoir
      buttonState = digitalRead(BUTTONPIN);

      // Si le bouton est pressé, OUVRIR la fenêtre
      if (buttonState == HIGH) {
        closeConfirmation = 0;
        Serial.println("return");
      }
    }
    closeConfirmation = 1;
  }
}
void fan_on_off() {

  if (insideRoomTemperature > 25 && someoneInRoom == HIGH) {
    digitalWrite(RELAYPIN, HIGH);
  } else {

    digitalWrite(RELAYPIN, LOW);
  }
}
void read_values() {
  float tempCelsiusDHTtmp = dht.readTemperature();
  insideRoomTemperature = isnan(tempCelsiusDHTtmp) ? insideRoomTemperature : tempCelsiusDHTtmp;

  insideRoomHumidity = dht.readHumidity();
}
void smoke_detect() {
   smokeValue = analogRead(SMOKEPIN);
  if (smokeValue > threshold) {
    insideRoomSmoke = 1;
    closeConfirmation = 0;
    smoke_alert();
    buzzerAlert(1000);
  } else insideRoomSmoke = 0;
  display_dht(insideRoomTemperature, insideRoomHumidity);
}
void buzzerAlert(int duration) {
  // Turn on the buzzer
  digitalWrite(BUZZERPIN, HIGH);

  // Wait for the specified duration
  delay(duration);

  // Turn off the buzzer
  digitalWrite(BUZZERPIN, LOW);

  delay(500);
}
void smoke_alert() {
  tft.fillScreen(TFT_RED);
  tft.drawXBitmap(0, 0, smoke_img_bits, smoke_img_width, smoke_img_height, TFT_BLACK);
}
void display_logo() {
  tft.fillScreen(TFT_WHITE);
  tft.drawXBitmap(0, 0, logo_aero_bits, logo_aero_width, logo_aero_height, TFT_BLUE);
}
void display_welcome() {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(50, 70, 4);  //x y fornt
  tft.setTextColor(TFT_WHITE);
  tft.print("Welcome...");
}
void display_notification() {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(70, 70, 4);  // x y font
  tft.setTextColor(TFT_WHITE);
  tft.print("Confirm");
  tft.setCursor(70, 100, 4);  // x y font
  tft.print("Closing");
  tft.setCursor(50, 130, 4);  // x y font
  tft.print("The window");
}

void display_dht(float temperature, float humidity) {
  tft.fillScreen(TFT_BLUE);
  tft.setCursor(50, 50, 4);
  tft.setTextColor(TFT_WHITE);
  tft.print("Temperature: ");
  tft.setCursor(50, 80, 4);
  tft.print(temperature, 1);
  tft.print("°C");
  tft.setCursor(50, 110, 4);
  tft.println("Humidity: ");
  tft.setCursor(50, 140, 4);
  tft.print(humidity, 1);
  tft.println("%");
}

void setup(void) {
  Serial.begin(115200);
  pinMode(RELAYPIN, OUTPUT);
  pinMode(SMOKEPIN, INPUT);
  pinMode(BUTTONPIN, INPUT_PULLUP);
  pinMode(BUZZERPIN, OUTPUT);
  pinMode(LIGHTPIN, OUTPUT);
  //initiating
  dht.begin();
  tft.init();

  //logo

  display_logo();
  delay(5000);

  //welcome

  display_welcome();
  connectToWiFi();
  delay(5000);
}

void loop() {
  read_values();
  String jsonData = buildJson();
  sendJSONToServer(jsonData);
  processServerResponse(Response);
  
  display_dht(insideRoomTemperature, insideRoomHumidity);
  fan_on_off();
  smoke_detect();
  light_on_off();

  delay(1000);
}

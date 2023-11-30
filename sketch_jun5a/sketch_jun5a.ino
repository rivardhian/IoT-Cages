#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include "HTTPSRedirect.h"
#include "DebugMacros.h"
#include <WiFiClient.h>
#include <BlynkSimpleEsp8266.h>
#include <Servo.h>
#include <DHT.h>
#include <HCSR04.h>
#include <NTPClient.h>
#include <WiFiUDP.h>
#include <WiFiManager.h>
BlynkTimer timer;
WidgetLED led1(V5);
WidgetLCD lcd(V4);

#define FAN_PIN D8
#define FAN_PIN1 D7
#define SERVO_PIN D5
#define BLYNK_TEMPLATE_ID "TMPL6D2tLspFL"
#define BLYNK_TEMPLATE_NAME "kandang cubby"
#define BLYNK_AUTH_TOKEN "69mb7nnRZadHDdwjxdCbu6ipfbsI7sWq"
#define BLYNK_PRINT Serial

const long utcOffsetInSeconds = 25200;
//data ssid dan password
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "ur ssid";
char pass[] = "ur pass";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);
const int maxDistance = 30;  // Jarak maksimum (dalam cm) yang dianggap sebagai 100%
Servo myservo;

// DHT sensor object
#define DHTPIN D3
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
HCSR04 hc(5, 4);
// Variables for distance and temperature
int distance;
int duration;
int temperature;

HTTPSRedirect* client = nullptr;

const char* EMAIL = "pasukanxmalang@ugnet";
const char* PASS = "Gunadarma2021";
const int httpsPort = 443;
const char* host = "blynk.cloud";

// Blynk virtual pins
int fanState = 0;
int servoPosition = 0;

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println(WiFi.softAPIP());
}

void Connecting() {
  static int error_count = 0;
  static int connect_count = 0;
  const unsigned int MAX_CONNECT = 20;
  static bool flag = false;
  if (!flag) {
    client = new HTTPSRedirect(httpsPort);
    client->setInsecure();
    flag = true;
    client->setPrintResponseBody(true);
    client->setContentTypeHeader("application/json");
    Serial.println("If !flag");
  }

  if (client != nullptr) {
    if (!client->connected()) {
      client->connect(host, httpsPort);
      //client->POST(url2, host, payload, false);
      Serial.println("!=nullptr");
    }
  }
  else {
    DPRINTLN("Error creating client object!");
    error_count = 5;
  }

  if (connect_count > MAX_CONNECT) {
    connect_count = 0;
    flag = false;
    delete client;
    return;
  }
}

void setup() {

  Serial.begin(115200);

  // Initialize Blynk
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
  pinMode(SERVO_PIN, OUTPUT);
  // Initialize servo
  myservo.attach(SERVO_PIN);
  myservo.write(0);
  Blynk.virtualWrite(V1, 0);

  Serial.begin(115200);
  // Initialize DHT sensor
  dht.begin();

  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(FAN_PIN, HIGH);
  pinMode(FAN_PIN1, OUTPUT);
  digitalWrite(FAN_PIN1, HIGH);
}

BLYNK_WRITE(V1) {

  int switchStatus = param.asInt();

  if (switchStatus == 1) {
    // Memutar servo ke posisi 90 derajat
    myservo.write(90);
  } else {
    // Memutar servo ke posisi awal (0 derajat)
    myservo.write(0);
  }
}


BLYNK_WRITE(V0) {
  int switchStatus = param.asInt();

  if (switchStatus == HIGH) {
    // Menghidupkan kipas
    digitalWrite(FAN_PIN1, HIGH);
    Blynk.virtualWrite(V5, 1);

  } else {
    // Mematikan kipas
    digitalWrite(FAN_PIN1, LOW);
    Blynk.virtualWrite(V5, 0);
  }
}

void loop() {

  Blynk.run();
  timer.run();
  delay(1000);

  // Get distance
  distance = hc.dist();

  if (distance > 30) {
    lcd.clear();
    lcd.print(0, 0, "Makanan Habis");
    delay(60);
  } else if (distance <= 30 && distance >= 25) {
    lcd.clear();
    lcd.print(0, 0, "tinggal sedikit");
    delay(60);
  } else {
    lcd.clear();
    lcd.print(0, 0, "Makanan Penuh");
    delay(60);
  }
  Serial.println(distance);
  delay(60);

  // Get temperature
  temperature = dht.readTemperature();

  // Update Blynk Label Value widget for temperature
  Blynk.virtualWrite(V3, temperature);

  // Check if temperature is above 32 degrees Celsius
  if (temperature > 32) {
    // Turn on fan
    fanState = 1;
    digitalWrite(FAN_PIN, HIGH);
    Blynk.virtualWrite(V5, 1);  // Turn on Blynk LED widget
    lcd.print(0, 1, "Suhu sudah: ");
    lcd.print(13, 1, temperature);
  } else {
    // Turn off fan
    fanState = 0;
    digitalWrite(FAN_PIN, LOW);
    Blynk.virtualWrite(V5, 0);  // Turn off Blynk LED widget
    lcd.print(0, 1, "Suhu: ");
    lcd.print(6, 1, temperature);
  }
  Serial.println(distance);  // return curent distance in serial
  delay(60);
}
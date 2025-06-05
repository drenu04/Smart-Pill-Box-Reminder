#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>

// Network credentials
const char* ssid = "iQOO Neo6";
const char* password = "tejasasi1234";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
ESP8266WebServer server(80);
SoftwareSerial gsm(0, 1);

const String PHONE_1 = "+917396801423";
const int buttonPins[] = {D7, D6, D5, D4};  // Array of GPIO pins for the buttons
const int numButtons = 4;  // Number of buttons
volatile int counters[numButtons] = {100, 100, 100, 100};  // Array of counters for each button
int validButtons[7][2] = {{0, 1}, {1, 2}, {2, 3}, {0, 3}, {1, 3}, {0, 2}, {3, 4}};  // Valid buttons for each day of the week (Sun = 0)
int buzz = D3;

// Individual interrupt service routines for each button
void IRAM_ATTR handleButtonPress0() { commonButtonHandler(0); }
void IRAM_ATTR handleButtonPress1() { commonButtonHandler(1); }
void IRAM_ATTR handleButtonPress2() { commonButtonHandler(2); }
void IRAM_ATTR handleButtonPress3() { commonButtonHandler(3); }

void setup() {
  Serial.begin(9600);

  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  delay(1000);
  Serial.println(WiFi.localIP());
  delay(1000);
  server.on("/", HTTP_GET, []() {
    String html = "<html><body>";
    html += "<h1>ESP8266 Counters</h1>";
    html += "<p>Counter 1: " + String(counters[0]) + "</p>";
    html += "<p>Counter 2: " + String(counters[1]) + "</p>";
    html += "<p>Counter 3: " + String(counters[2]) + "</p>";
    html += "<p>Counter 4: " + String(counters[3]) + "</p>";
    html += "</body></html>";
    server.send(200, "text/html", html);
  });
  delay(1000);
  server.begin();
  delay(1000);
  gsm.begin(9600);   // Setting the baud rate of GSM Module  
  delay(100);

  gsm.println("AT");
  delay(1000);
  gsm.println("AT+CMGF=1"); // Set SMS mode to text
  delay(1000);
  gsm.println("AT+CNMI=1,2,0,0,0");

  timeClient.begin();
  timeClient.setTimeOffset(3600);  // Set the correct time offset in seconds (example for GMT+1)

  pinMode(buttonPins[0], INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(buttonPins[0]), handleButtonPress0, FALLING);
  pinMode(buttonPins[1], INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(buttonPins[1]), handleButtonPress1, FALLING);
  pinMode(buttonPins[2], INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(buttonPins[2]), handleButtonPress2, FALLING);
  pinMode(buttonPins[3], INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(buttonPins[3]), handleButtonPress3, FALLING);
  pinMode(buzz, OUTPUT);
}

void loop() {
  server.handleClient();
  timeClient.update();
}

void commonButtonHandler(int buttonIndex) {
  if (checkButtonValidity(buttonIndex)) {
    counters[buttonIndex]--;
    delay(1000);

    if (counters[buttonIndex] <= 95) {
      Serial.print("Threshold alert with button number: ");
      Serial.println(buttonIndex + 1);
      send_sms(buttonIndex);
    }
  }
}

bool checkButtonValidity(int buttonIndex) {
  int dayOfWeek = 2;//timeClient.getDay();  // Get the current day of the week, where 1 is Monday and 7 is Sunday

  int *allowedButtons = validButtons[dayOfWeek - 1];  // Get the allowed buttons for today
  if (allowedButtons[0] == buttonIndex || allowedButtons[1] == buttonIndex) {
    return true;
  } else {
    Serial.print("Alert: Wrong button pressed for today! Button: ");
    Serial.println(buttonIndex + 1);
    digitalWrite(buzz,HIGH);
    delay(5000);
    digitalWrite(buzz,LOW);
    return false;
  }
}

void send_sms(int buttonIndex)
{
    Serial.println("sending sms....");
    gsm.print("AT+CMGF=1\r");
    delay(1000);
    gsm.print("AT+CMGS=\"+917396801423\"");
    delay(1000);
    gsm.print("medicine are about to complete");
    delay(100);
    gsm.print(buttonIndex);
    delay(100);
    gsm.write(26); 
    delay(1000);
    Serial.println("SMS sent");
}
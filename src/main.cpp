#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SevSeg.h>

// Pin Definitions
#define SERVO_PIN 18
#define TRIG_PIN 10
#define ECHO_PIN 11
#define POT_PIN 1 
#define BUTTON_PIN 4
#define BUZZER_PIN 5
#define TOUCH_PIN 2 

// 7-Segment Pins (4-digit, multiplexed, standard non-TM1637)
#define SEG_A 16
#define SEG_B 17
#define SEG_C 21
#define SEG_D 38
#define SEG_E 39
#define SEG_F 40
#define SEG_G 41
#define SEG_DP 42
#define DIGIT1 12
#define DIGIT2 13
#define DIGIT3 14
#define DIGIT4 15

SevSeg sevseg;

// Objects
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo myServo;
WebServer server(80);

// Configuration
const int SERVO_CLOSED = 0;
const int SERVO_OPEN = 90;
const int DISTANCE_THRESHOLD = 15;
const int TOUCH_THRESHOLD = 100000;
const unsigned long GAME_TIME_LIMIT = 60000;

// Game State
enum GameState { IDLE, PLAYING, WIN, GAME_OVER_ALARM };
GameState currentState = IDLE;

// Game Logic Variables
int targetCode[4] = {1, 2, 3, 4};
int currentDigitIndex = 0;
unsigned long gameStartTime = 0;
int lastButtonState = HIGH;
String prankMsg = "";
unsigned long prankMsgTime = 0;

void openSafe() { myServo.write(SERVO_OPEN); Serial.println("Web: Safe Opened"); }
void closeSafe() { myServo.write(SERVO_CLOSED); Serial.println("Web: Safe Closed"); }

void triggerAlarm() {
  tone(BUZZER_PIN, 500);
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("   GAME OVER!   ");
  lcd.setCursor(0, 1); lcd.print("  ALARM ACTIVE  ");
  currentState = GAME_OVER_ALARM;
}

long getDistance() {
  static unsigned long lastMeasure = 0;
  static long lastDist = 999;
  if (millis() - lastMeasure < 60) return lastDist; 
  lastMeasure = millis();

  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 20000); 
  if (duration == 0) return 999;
  lastDist = duration * 0.034 / 2;
  return lastDist;
}

// --- WEB SERVER HANDLERS ---
void handleRoot() {
  String html = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'><style>body{font-family:sans-serif;text-align:center;background:#222;color:white}button{padding:15px;margin:10px;font-size:18px;width:200px;cursor:pointer;border:none;border-radius:5px} .btn-red{background:#d33} .btn-green{background:#28a745} .btn-blue{background:#007bff} input{padding:10px;font-size:18px;width:150px}</style></head><body>";
  html += "<h1>Safe Cracker Prank Panel</h1>";
  html += "<h3>Current Code: " + String(targetCode[0]) + String(targetCode[1]) + String(targetCode[2]) + String(targetCode[3]) + "</h3>";
  html += "<form action='/setcode'>New Code: <input type='number' name='code' maxlength='4'><input type='submit' value='Set'></form><hr>";
  html += "<button class='btn-red' onclick=\"location.href='/alarm'\">TRIGGER ALARM</button><br>";
  html += "<button class='btn-green' onclick=\"location.href='/open'\">OPEN DOOR</button>";
  html += "<button class='btn-blue' onclick=\"location.href='/close'\">CLOSE DOOR</button><hr>";
  html += "<form action='/msg'>LCD Prank Message: <input type='text' name='text'><input type='submit' value='Send'></form>";
  html += "<button onclick=\"location.href='/reset'\">RESET GAME</button>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleSetCode() {
  String newCode = server.arg("code");
  if (newCode.length() == 4) {
    for (int i = 0; i < 4; i++) targetCode[i] = newCode[i] - '0';
  }
  server.sendHeader("Location", "/"); server.send(302, "text/plain", "");
}

void handleMessage() {
  prankMsg = server.arg("text");
  prankMsgTime = millis();
  server.sendHeader("Location", "/"); server.send(302, "text/plain", "");
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT); pinMode(ECHO_PIN, INPUT); pinMode(POT_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP); pinMode(BUZZER_PIN, OUTPUT);

  Wire.begin(8, 9); lcd.init(); lcd.backlight();

  // 7-Segment Initialization 
  byte numDigits = 4;
  byte digitPins[] = {DIGIT1, DIGIT2, DIGIT3, DIGIT4};
  byte segmentPins[] = {SEG_A, SEG_B, SEG_C, SEG_D, SEG_E, SEG_F, SEG_G, SEG_DP};
  bool resistorsOnSegments = false;
  byte hardwareConfig = COMMON_CATHODE; 
  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments, false, false, false);
  sevseg.setBrightness(90);

  ESP32PWM::allocateTimer(0); myServo.setPeriodHertz(50);
  myServo.attach(SERVO_PIN, 500, 2400); closeSafe();

  // WiFi Access Point
  WiFi.softAP("SafeCracker_Prank", "password123");
  Serial.println("WiFi AP Started: SafeCracker_Prank");
  Serial.print("IP Address: "); Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/setcode", handleSetCode);
  server.on("/alarm", []() { triggerAlarm(); server.sendHeader("Location", "/"); server.send(302); });
  server.on("/open", []() { openSafe(); server.sendHeader("Location", "/"); server.send(302); });
  server.on("/close", []() { closeSafe(); server.sendHeader("Location", "/"); server.send(302); });
  server.on("/msg", handleMessage);
  server.on("/reset", []() { currentState = IDLE; noTone(BUZZER_PIN); server.sendHeader("Location", "/"); server.send(302); });
  server.begin();
}

void loop() {
  server.handleClient();
  sevseg.refreshDisplay();
  
  int touchVal = touchRead(TOUCH_PIN);
  bool isTouching = touchVal > TOUCH_THRESHOLD;
  int buttonState = digitalRead(BUTTON_PIN);
  long distance = getDistance();

  // Prank Message Override
  if (prankMsg != "" && millis() - prankMsgTime < 5000) {
    lcd.setCursor(0, 0); lcd.print("PRANK INBOUND:  ");
    lcd.setCursor(0, 1); lcd.print(prankMsg + "                ");
    return;
  } else if (prankMsg != "") { prankMsg = ""; lcd.clear(); }

  switch (currentState) {
    case IDLE:
      noTone(BUZZER_PIN); closeSafe();
      lcd.setCursor(0, 0); lcd.print("HOLD TOUCH &   ");
      lcd.setCursor(0, 1); lcd.print("PRESS BTN START ");
      sevseg.setChars("----");
      if (isTouching && buttonState == LOW && lastButtonState == HIGH) {
        currentState = PLAYING; gameStartTime = millis();
        currentDigitIndex = 0; lcd.clear();
        tone(BUZZER_PIN, 1500, 200);
      }
      break;

    case PLAYING:
      {
        if (!isTouching || distance > DISTANCE_THRESHOLD || (millis() - gameStartTime >= GAME_TIME_LIMIT)) {
          triggerAlarm(); return;
        }
        int potValue = analogRead(POT_PIN);
        int mappedValue = map(potValue, 0, 4095, 0, 9);
        
        // Update 7-Segment Display with PIN progress
        char codeChars[5] = "    ";
        for(int i=0; i<4; i++) {
          if (i < currentDigitIndex) codeChars[i] = targetCode[i] + '0';
          else if (i == currentDigitIndex) codeChars[i] = mappedValue + '0';
          else codeChars[i] = '-';
        }
        sevseg.setChars(codeChars);

        lcd.setCursor(0, 0); lcd.print("CRACK THE SAFE! ");
        lcd.setCursor(0, 1);
        int timeLeft = (GAME_TIME_LIMIT - (millis() - gameStartTime)) / 1000;
        lcd.printf("Time:%2ds Dist:%2d", timeLeft, (int)distance);

        if (buttonState == LOW && lastButtonState == HIGH) {
          if (mappedValue == targetCode[currentDigitIndex]) {
            currentDigitIndex++; tone(BUZZER_PIN, 1200, 100);
            if (currentDigitIndex >= 4) { currentState = WIN; openSafe(); }
          } else { tone(BUZZER_PIN, 400, 300); }
        }
      }
      break;

    case WIN:
      lcd.setCursor(0, 0); lcd.print("  SAFE OPENED!  ");
      lcd.setCursor(0, 1); lcd.print("   YOU CRACKED IT  ");
      sevseg.setChars("OPEN");
      if ((millis() / 500) % 2 == 0) tone(BUZZER_PIN, 2000, 100);
      if (buttonState == LOW && lastButtonState == HIGH) currentState = IDLE;
      break;

    case GAME_OVER_ALARM:
      sevseg.setChars("FAIL");
      if (buttonState == LOW && lastButtonState == HIGH) {
        noTone(BUZZER_PIN); currentState = IDLE; lcd.clear();
      }
      break;
  }
  lastButtonState = buttonState;
}

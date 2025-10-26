#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>

#define ALCOHOL_SENSOR A0
#define GREEN_LED 8
#define RED_LED 7
#define BUZZER 6
#define START_BTN 4

LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS1307 rtc;

bool measuring = false;
int maxValue = 0;

// custom icons (MADE USING CHATGPT)
byte bar[8] = {B11111,B11111,B11111,B11111,B11111,B11111,B11111,B11111};
byte bottle[8] = {B00100,B01110,B01110,B01110,B01110,B11111,B11111,B01110};
byte smile[8]  = {B00000,B00000,B00000,B00000,B10001,B01110,B00000,B00000};
byte wave[8]   = {B00000,B10001,B01010,B00100,B01010,B10001,B00000,B00000};

void setup() {
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(START_BTN, INPUT_PULLUP);
  pinMode(ALCOHOL_SENSOR, INPUT);

  lcd.init();
  lcd.backlight();
  lcd.createChar(0, bar);
  lcd.createChar(1, bottle);
  lcd.createChar(2, smile);
  lcd.createChar(3, wave);

  Wire.begin();
  rtc.begin();

  if (rtc.isrunning()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  Serial.begin(9600);
  Serial.println("Alcohol Tester Ready");

  lcd.setCursor(2, 0);
  lcd.print("ALCOHOL TESTER");
  lcd.setCursor(7, 1);
  lcd.write(1);
  delay(2000);
  lcd.clear();
}

void loop() {
  if (!measuring && digitalRead(START_BTN) == LOW) {
    delay(50);
    if (digitalRead(START_BTN) == LOW) {
      measuring = true;
      maxValue = 0;
      Serial.println("Measurement started");

      lcd.clear();
      lcd.setCursor(3, 0);
      lcd.print("Measuring...");
      lcd.setCursor(0, 1);

      // progress bar animation
      for (int i = 0; i < 16; i++) {
        lcd.write(byte(0));
        delay(250);
      }

      unsigned long startTime = millis();
      while (millis() - startTime < 5000) {
        int value = analogRead(ALCOHOL_SENSOR);
        Serial.print("Flying Fish = ");
        Serial.println(value);
        if (value > maxValue) maxValue = value;
        delay(100);
      }

      showResult();
      measuring = false;
    }
  }

  if (!measuring) {
    DateTime now = rtc.now();
    lcd.setCursor(0, 0);
    lcd.print("Time ");
    if (now.hour() < 10) lcd.print("0");
    lcd.print(now.hour());
    lcd.print(":");
    if (now.minute() < 10) lcd.print("0");
    lcd.print(now.minute());
    lcd.print(":");
    if (now.second() < 10) lcd.print("0");
    lcd.print(now.second());

    lcd.setCursor(0, 1);
    lcd.write(3);
    lcd.print(" READY ");
    lcd.write(2);
    lcd.print("     ");
    delay(500);
  }
}

void showResult() {
  lcd.clear();

  // 400 = clean air (~0.000%), 700 = strong alcohol (~0.080%)
  float bac = map(maxValue, 400, 700, 0, 80) / 1000.0;
  if (bac < 0) bac = 0;
  if (bac > 0.2) bac = 0.2;

  lcd.setCursor(0, 0);
  lcd.print("BAC: ");
  lcd.print(bac, 3);
  lcd.print("%");
  lcd.setCursor(11, 0);
  lcd.write(1);

  int threshold = 600;

  if (maxValue > threshold) {
    digitalWrite(RED_LED, HIGH);
    digitalWrite(BUZZER, HIGH);
    lcd.setCursor(0, 1);
    lcd.print("Too drunk! >_<");
    delay(2000);
    digitalWrite(BUZZER, LOW);
    digitalWrite(RED_LED, LOW);
  } else {
    digitalWrite(GREEN_LED, HIGH);
    lcd.setCursor(0, 1);
    lcd.print("OK to drive :)");
    delay(2000);
    digitalWrite(GREEN_LED, LOW);
  }

  delay(1000);
  lcd.clear();
  maxValue = 400; // Baseline of test
}

#include <Arduino.h>
#include <stdlib.h>
#include <time.h>
#include <LiquidCrystal_I2C.h>

#define SETUP1 1
#define SETUP2 2
#define GAMING1 3
#define GAMING2 4
#define GAMING3 5
#define GAMING4 6

#define RED_LED_PIN 11
#define POTENTIOMETER_PIN A0

#define RED_LED_BRIGHTNESS_STEP 5

#define MAX_RANDOM 15
#define TIME_DELTA 0.5
#define MAX_TIME_WINDOW 10

int state;
int difficulty;
int score;

int currentRandomValue;
int currentMaxTime;

int redLedBrightness;
int redLedBrightnessStep;

LiquidCrystal_I2C lcd(0x27, 16, 2); // 0x27 is the address for most I2C LCD displays

void setup() {
  srand(time(NULL));
  lcd.init();
  lcd.backlight();
  redLedBrightnessStep = RED_LED_BRIGHTNESS_STEP;
  state = SETUP1;
}

void loop() {
  switch (state) {
  case SETUP1:
    break;
  case SETUP2:
    break;
  case GAMING1:
    break;
  case GAMING2:
    break;
  case GAMING3:
    break;
  case GAMING4:
    break;
  default:
    break;
  }
}

void setup1() {
  score = 0;
  difficulty = 1;
  lcd.setCursor(0,0);
  lcd.print("Welcome");
  delay(1000);
  //start timer and transition to next state
  state = SETUP2; 
}

void setup2() {
  int potentiometerValue = analogRead(POTENTIOMETER_PIN);
  difficulty = mapToDifficulty(potentiometerValue);
  currentMaxTime = MAX_TIME_WINDOW - TIME_DELTA * difficulty;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Difficulty: X Turn Time: Y");
  lcd.setCursor(0,1);
  lcd.print("Press B1 to Start");
  if (redLedBrightness < 0 || redLedBrightness > 255) {
    redLedBrightnessStep = -redLedBrightnessStep;
  }
  redLedBrightness = redLedBrightness + redLedBrightnessStep;
  analogWrite(RED_LED_PIN, redLedBrightness);
  //if b1 is pressed in time go to next state else force sleep.
}

void wake() {
  state = SETUP1;
}

void gaming1() {
  lcd.setCursor(0,0);
  lcd.print("Go!");
  delay(1000);
  currentRandomValue = rand() % MAX_RANDOM + 1;
  //start timer and transition to the next state.
  state = GAMING2;
}

void gaming2() {
  //read button 1 2 3 4
  //power them on/off accordingly
  //read LED 1 2 3 4
  //if leds corresponding binary value sum equals to the random value go to gaming3 else go to gaming4
}

void gaming3() {
  lcd.clear();
  score++;
  lcd.setCursor(0,0);
  lcd.print("Score " + score);
  currentMaxTime = currentMaxTime - TIME_DELTA;
  delay(2000);
  state = GAMING2;
}

void gaming4() {
  lcd.clear();
  //turn red light on and off in the span of 1 sec
  digitalWrite(RED_LED_PIN, HIGH);
  delay(1000);
  digitalWrite(RED_LED_PIN, LOW);
  //print results
  lcd.setCursor(0,0);
  lcd.print("Game Over");
  lcd.setCursor(0,1);
  lcd.print("Score " + score);
  delay(10000);
  state = SETUP1;
}

int mapToDifficulty(int value) {
    if (value <= 255) {
        return 0;  // Difficulty Level 0
    } else if (value <= 511) {
        return 1;  // Difficulty Level 1
    } else if (value <= 767) {
        return 2;  // Difficulty Level 2
    } else {
        return 3;  // Difficulty Level 3
    }
}
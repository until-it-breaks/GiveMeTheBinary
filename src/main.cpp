#include <Arduino.h>
#include <stdlib.h>
#include <time.h>
#include <LiquidCrystal_I2C.h>

/*
  Give Me The Binary!
  Reference circuit https://www.tinkercad.com/things/epSlpAZTefP-givemethebinary?sharecode=rppd_5WtQ8WaYqnvDhg68Ee-5P04I2m4gol8a1v37rE
*/

//States
#define SETUP1 1
#define SETUP2 2
#define GAMING1 3
#define GAMING2 4
#define GAMING3 5
#define GAMING4 6

#define LED_COUNT 4
#define BUTTON_COUNT 4

//Pins
#define BUTTON1_PIN 4
#define BUTTON2_PIN 5
#define BUTTON3_PIN 6
#define BUTTON4_PIN 7

#define GREEN_LED1_PIN 10
#define GREEN_LED2_PIN 12
#define GREEN_LED3_PIN 13
#define GREEN_LED4_PIN 14

#define RED_LED_PIN 9 //PWM
#define POTENTIOMETER_PIN A0

//Game constants
#define MAX_RANDOM 15
#define TIME_DELTA 0.5
#define MAX_TIME_WINDOW 10
#define RED_LED_BRIGHTNESS_STEP 5

//Game variables
int state;
int difficulty;
int score;
int currentRandomValue;
int currentMaxTime;

//Red led variables
int redLedBrightness;
int redLedBrightnessStep;

//Arrays
int leds[LED_COUNT - 1];
bool ledStates[LED_COUNT - 1];
int buttons[BUTTON_COUNT - 1];

LiquidCrystal_I2C lcd(0x27, 16, 2); // 0x27 is the address for most I2C LCD displays

void setup() {
  srand(time(NULL));
  redLedBrightnessStep = RED_LED_BRIGHTNESS_STEP;
  
  leds[0] = GREEN_LED1_PIN;
  leds[1] = GREEN_LED2_PIN;
  leds[2] = GREEN_LED3_PIN;
  leds[3] = GREEN_LED4_PIN;

  buttons[0] = BUTTON1_PIN;
  buttons[1] = BUTTON2_PIN;
  buttons[2] = BUTTON3_PIN;
  buttons[3] = BUTTON4_PIN;

  for (auto &&led: leds)
  {
    pinMode(led, OUTPUT);
  }

  for (auto &&ledState : ledStates)
  {
    ledState = false;
  }

  for (auto &&button: buttons)
  {
    pinMode(button, INPUT);
  }

  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(POTENTIOMETER_PIN, INPUT);

  lcd.init();
  lcd.backlight();
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
  if (digitalRead(buttons[0] == HIGH)) {
    state = GAMING1;
  }
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
  for (size_t i = 0; i < BUTTON_COUNT - 1; i++)
  {
    if(digitalRead(buttons[i]) == HIGH) {
      digitalWrite(leds[i], HIGH);
      ledStates[i] = true;
    } else {
      digitalWrite(leds[i], LOW);
      ledStates[i] = false;
    }
  }
  int sum = 0;
  for (size_t i = 0; i < LED_COUNT - 1; i++)
  {
    if (ledStates[i] == true) {
      switch (i) {
        case 0:
          sum = sum + 1;
          break;
        case 1:
          sum = sum + 2;
          break;
        case 2:
          sum = sum + 4;
          break;
        case 3:
          sum = sum + 8;
          break;
        default:
          sum = sum + 0;
          break;
      }
    }
  }
  if (sum == currentRandomValue) {
    //Stop timer and transition to next stage
    state = GAMING3;
  }
  //Timer runs out, transition to game over
  state = GAMING4;
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
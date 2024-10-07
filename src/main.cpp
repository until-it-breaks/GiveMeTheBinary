#include <Arduino.h>
#include <stdlib.h>
#include <time.h>
#include <LiquidCrystal_I2C.h>
#include <TimerOne.h>
#include <avr/sleep.h>

/*
  Give Me The Binary!
  Reference circuit https://www.tinkercad.com/things/epSlpAZTefP-givemethebinary?sharecode=rppd_5WtQ8WaYqnvDhg68Ee-5P04I2m4gol8a1v37rE
*/

//States
#define STATE_SETUP_1 1
#define STATE_SETUP_2 2
#define STATE_GAMING_1 3
#define STATE_GAMING_2 4
#define STATE_GAMING_3 5
#define STATE_GAMING_4 6

#define LED_COUNT 4
#define BUTTON_COUNT 4

//Pins
#define BUTTON1_PIN 2 //this pin is compatible with interrupts
#define BUTTON2_PIN 5
#define BUTTON3_PIN 6
#define BUTTON4_PIN 7

#define GREEN_LED1_PIN 10
#define GREEN_LED2_PIN 11
#define GREEN_LED3_PIN 12
#define GREEN_LED4_PIN 13

#define RED_LED_PIN 9 //PWM
#define POTENTIOMETER_PIN A0

//Game constants
#define MAX_RANDOM 15
#define MAX_TIME_WINDOW 10
#define TIME_DELTA 0.5
#define RED_LED_BRIGHTNESS_STEP 1

//Difficulties
#define VERY_EASY 0
#define EASY 1
#define NORMAL 2
#define HARD 3

//Game variables
int state;
int difficulty;
int score;
int currentRandomValue;
int currentMaxTime;
bool timeWindowElapsed;

//Red led variables
int redLedBrightness;
int redLedBrightnessStep;

//Arrays
int leds[LED_COUNT];
bool ledStates[LED_COUNT];
int buttons[BUTTON_COUNT];

void setupTask1();
void setupTask2();
void gamingTask1();
void gamingTask2();
void gamingTask3();
void gamingTask4();
int mapToDifficulty(int value);
void startTimer(unsigned long seconds);
void timerISR();
void wakeUp();
void sleepNow();

LiquidCrystal_I2C lcd(0x27, 16, 2); // 0x27 is the address for most I2C LCD displays

void setup() {
  srand(time(NULL));
  Serial.begin(9600);
  redLedBrightness = 0;
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

  timeWindowElapsed = false;
  state = STATE_SETUP_1;
}

void loop() {
  switch (state) {
  case STATE_SETUP_1:
    setupTask1();
    break;
  case STATE_SETUP_2:
    setupTask2();
    break;
  case STATE_GAMING_1:
    gamingTask1();
    break;
  case STATE_GAMING_2:
    gamingTask2();
    break;
  case STATE_GAMING_3:
    gamingTask3();
    break;
  case STATE_GAMING_4:
    gamingTask4();
    break;
  default:
    break;
  }
}

void setupTask1() {
  score = 0;
  difficulty = VERY_EASY;
  currentMaxTime = MAX_TIME_WINDOW;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Welcome to GMB!");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Choose the");
  lcd.setCursor(0,1);
  lcd.print("difficulty");
  delay(5000);
  //start timer and transition to next state
  state = STATE_SETUP_2;
  startTimer(5000000);
}

void setupTask2() {
  //if the timer this time around has fully elapsed then go into deep sleep
  if (timeWindowElapsed) {
    sleepNow();
  } else {
    int potentiometerValue = analogRead(POTENTIOMETER_PIN);
    int newDifficulty = mapToDifficulty(potentiometerValue);
    if (difficulty != newDifficulty) {
      difficulty = newDifficulty;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Difficulty: " + String(difficulty));
      lcd.setCursor(0,1);
      lcd.print("Tap B1 to start");
      currentMaxTime = MAX_TIME_WINDOW - TIME_DELTA * difficulty;
    }
  
    analogWrite(RED_LED_PIN, redLedBrightness);
    redLedBrightness = redLedBrightness + redLedBrightnessStep;
    if (redLedBrightness == 0 || redLedBrightness == 255) {
      redLedBrightnessStep = -redLedBrightnessStep ; 
    }  
    
    //if b1 is pressed in time go to next state
    if (digitalRead(buttons[0]) == HIGH) {
      digitalWrite(RED_LED_PIN, LOW);
      state = STATE_GAMING_1;
    }
  }
}

void gamingTask1() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Go!");
  delay(1000);
  currentRandomValue = rand() % MAX_RANDOM + 1;
  lcd.setCursor(0,1);
  lcd.print("Value: " + String(currentRandomValue));
  //start timer and transition to the next state.
  state = STATE_GAMING_2;
  //startTimer(currentMaxTime * 1000000);
}

void gamingTask2() {
  if (timeWindowElapsed) {
    state = STATE_GAMING_4;
  } else {
    for (size_t i = 0; i < BUTTON_COUNT; i++)
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
      for (size_t i = 0; i < LED_COUNT; i++)
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
      if (sum == currentRandomValue) {
          state = STATE_GAMING_3;
      }
    }
  } 
}

void gamingTask3() {
  score++;
  currentMaxTime = currentMaxTime - TIME_DELTA;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("You won!");
  lcd.setCursor(0,1);
  lcd.print("Score: " + String(score));
  delay(2000);
  state = STATE_GAMING_1;
}

void gamingTask4() {
  lcd.clear();
  digitalWrite(RED_LED_PIN, HIGH);
  delay(1000);
  digitalWrite(RED_LED_PIN, LOW);
  lcd.setCursor(0,0);
  lcd.print("Game Over!");
  lcd.setCursor(0,1);
  lcd.print("Score: " + String(score));
  delay(10000);
  state = STATE_SETUP_1;
}

int mapToDifficulty(int value) {
    if (value <= 255) {
        return VERY_EASY;
    } else if (value <= 511) {
        return EASY;
    } else if (value <= 767) {
        return NORMAL;
    } else {
        return HARD;
    }
}

void startTimer(unsigned long seconds) {
  Timer1.stop();
  Timer1.detachInterrupt();
  timeWindowElapsed = false;
  Timer1.initialize(seconds);
  Timer1.attachInterrupt(timerISR);
  Timer1.start();
}

void timerISR() {
  timeWindowElapsed = true;
  Timer1.stop();
  Timer1.detachInterrupt();
}

void wakeUp() {
  state = STATE_SETUP_1;
}

void sleepNow() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  attachInterrupt(digitalPinToInterrupt(BUTTON1_PIN), wakeUp, HIGH); //When b1 is pressed
  sleep_mode();
  sleep_disable();
  detachInterrupt(digitalPinToInterrupt(BUTTON1_PIN));
}
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <avr/sleep.h>
#include <EnableInterrupt.h>

// Constants
#define LED_COUNT 4
#define BUTTON_COUNT 4
#define MAX_RANDOM 15
#define MAX_TIME_WINDOW 15000 //Max time window to light up the correct leds
#define TIME_DELTA 0.5
#define RED_LED_BRIGHTNESS_STEP 1

// Pins
#define BUTTON1_PIN 4
#define BUTTON2_PIN 5
#define BUTTON3_PIN 6
#define BUTTON4_PIN 7
#define GREEN_LED1_PIN 10
#define GREEN_LED2_PIN 11
#define GREEN_LED3_PIN 12
#define GREEN_LED4_PIN 13
#define RED_LED_PIN 9
#define POTENTIOMETER_PIN A0
#define UNCONNECTED_ANALOG_PIN A3

// Difficulty Levels
enum Difficulty { VERY_EASY, EASY, NORMAL, HARD };

// Game States
enum GameState {
    STATE_SETUP_1,
    STATE_SETUP_2,
    STATE_GAMING_1,
    STATE_GAMING_2,
    STATE_GAMING_3,
    STATE_GAMING_4
};

// Game variables
GameState state;
Difficulty difficulty;
int score;
int currentRandomValue;
int currentMaxTime;
bool timeWindowElapsed;

// Red LED variables
int redLedBrightness;
int redLedBrightnessStep;

// Arrays
int leds[LED_COUNT] = { GREEN_LED1_PIN, GREEN_LED2_PIN, GREEN_LED3_PIN, GREEN_LED4_PIN };
bool ledStates[LED_COUNT] = { false, false, false, false };
int buttons[BUTTON_COUNT] = { BUTTON1_PIN, BUTTON2_PIN, BUTTON3_PIN, BUTTON4_PIN };

// Timing variables
unsigned long previousMillis;
unsigned long timerInterval;

LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD initialization

void setup();
void loop();
void setupGame();
void updateDifficulty();
void gameStart();
void processGame();
void checkGameOutcome();
void showGameOver();
int readButtons();
void updateLEDs();
void manageRedLED();
void enterSleepMode();
void wakeUp();

void setup() {
    randomSeed(analogRead(UNCONNECTED_ANALOG_PIN));
    Serial.begin(9600);
    redLedBrightness = 0;
    redLedBrightnessStep = RED_LED_BRIGHTNESS_STEP;

    for (int i = 0; i < LED_COUNT; i++) {
        pinMode(leds[i], OUTPUT);
    }

    for (int i = 0; i < BUTTON_COUNT; i++) {
        pinMode(buttons[i], INPUT);
    }

    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(POTENTIOMETER_PIN, INPUT);

    lcd.init();
    lcd.backlight();

    state = STATE_SETUP_1;
}

void loop() {
    switch (state) {
    case STATE_SETUP_1:
        setupGame();
        break;
    case STATE_SETUP_2:
        updateDifficulty();
        break;
    case STATE_GAMING_1:
        gameStart();
        break;
    case STATE_GAMING_2:
        processGame();
        break;
    case STATE_GAMING_3:
        checkGameOutcome();
        break;
    case STATE_GAMING_4:
        showGameOver();
        break;
    default:
        break;
    }
}

void setupGame() {
    score = 0;
    difficulty = VERY_EASY;
    currentMaxTime = MAX_TIME_WINDOW;
    timeWindowElapsed = false;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Welcome to GMB!");
    delay(1000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Choose the");
    lcd.setCursor(0, 1);
    lcd.print("difficulty");
    delay(5000);

    previousMillis = millis();
    timerInterval = 10000; // 10 seconds
    state = STATE_SETUP_2;
}

void updateDifficulty() {
    if (timeWindowElapsed) {
        enterSleepMode();
    } else {
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= timerInterval) {
            timeWindowElapsed = true;
        }

        int potentiometerValue = analogRead(POTENTIOMETER_PIN);
        int newDifficulty = map(potentiometerValue, 0, 1023, 0, 3);
        if (difficulty != newDifficulty) {
            difficulty = static_cast<Difficulty>(newDifficulty);
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Difficulty: " + String(difficulty));
            lcd.setCursor(0, 1);
            lcd.print("Tap B1 to start");
            currentMaxTime = MAX_TIME_WINDOW - TIME_DELTA * difficulty;
        }

        manageRedLED();

        if (digitalRead(buttons[0]) == HIGH) {
            digitalWrite(RED_LED_PIN, LOW);
            state = STATE_GAMING_1;
        }
    }
}

void gameStart() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Go!");
    delay(1000);
    currentRandomValue = random(0, MAX_RANDOM + 1);
    lcd.setCursor(0, 1);
    lcd.print("Value: " + String(currentRandomValue));
    previousMillis = millis();
    timerInterval = currentMaxTime;
    state = STATE_GAMING_2;
}

void processGame() {
    if (timeWindowElapsed) {
        state = STATE_GAMING_4;
    } else {
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= timerInterval) {
            timeWindowElapsed = true;
        }

        updateLEDs();
        
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
      }
      if (sum == currentRandomValue) {
          state = STATE_GAMING_3;
      }
    }
}

void checkGameOutcome() {
    score++;
    currentMaxTime = currentMaxTime - TIME_DELTA;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("You won!");
    lcd.setCursor(0, 1);
    lcd.print("Score: " + String(score));
    delay(2000);
    state = STATE_GAMING_1;
}

void showGameOver() {
    lcd.clear();
    digitalWrite(RED_LED_PIN, HIGH);
    delay(1000);
    digitalWrite(RED_LED_PIN, LOW);
    lcd.setCursor(0, 0);
    lcd.print("Game Over!");
    lcd.setCursor(0, 1);
    lcd.print("Score: " + String(score));
    delay(10000);
    state = STATE_SETUP_1;
}

void updateLEDs() {
    for (size_t i = 0; i < BUTTON_COUNT; i++) {
        if (digitalRead(buttons[i]) == HIGH) {
            digitalWrite(leds[i], HIGH);
            ledStates[i] = true;
        } else {
            digitalWrite(leds[i], LOW);
            ledStates[i] = false;
        }
    }
}

void manageRedLED() {
    analogWrite(RED_LED_PIN, redLedBrightness);
    redLedBrightness += redLedBrightnessStep;
    if (redLedBrightness <= 0 || redLedBrightness >= 255) {
        redLedBrightnessStep = -redLedBrightnessStep;
    }
}

void enterSleepMode() {
    lcd.noBacklight();
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    //attachInterrupt(digitalPinToInterrupt(BUTTON1_PIN), wakeUp, HIGH); // When B1 is pressed
    sleep_mode();
    sleep_disable();
    //detachInterrupt(digitalPinToInterrupt(BUTTON1_PIN));
    lcd.backlight();
}

void wakeUp() {
    state = STATE_SETUP_1;
}
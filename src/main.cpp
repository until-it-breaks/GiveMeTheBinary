#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <avr/sleep.h>
#include <EnableInterrupt.h>

//TODO Improve Red LED fade, state cast, process game inputs, delay needed after value is shown, disable rogue red LED

/**
 * A simple binary numbers game for the Arduino Uno R3
 * Circuit reference: https://www.tinkercad.com/things/epSlpAZTefP-givemethebinary?sharecode=rppd_5WtQ8WaYqnvDhg68Ee-5P04I2m4gol8a1v37rE
 */

// Constants
#define LED_COUNT 4
#define BUTTON_COUNT 4
#define MAX_RANDOM 15
#define TIME_DELTA 0.5
#define RED_LED_BRIGHTNESS_STEP 1

// Time windows in milliseconds
#define INITIALIZATION_DELAY 5000
#define GAME_OVER_DELAY 10000
#define SET_DIFFICULTY_WINDOW 10000
#define ROUND_TIME_WINDOW 15000
#define ROUND_RESOLUTION_DELAY 2000
#define RED_LIGHT_DELAY 1000

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
    STATE_INITIALIZE,
    STATE_SETTINGS,
    STATE_ROUND_SETUP,
    STATE_PROCESS_GAME,
    STATE_RESOLVE_ROUND,
    STATE_GAME_OVER
};

// Game variables
GameState gameState;
Difficulty difficulty;
int score;
int currentRandomValue;
int currentMaxTime;
bool timeWindowHasElapsed;

// Red LED variables
int redLedBrightness;
int redLedBrightnessStep;

// Arrays
int leds[LED_COUNT] = { GREEN_LED1_PIN, GREEN_LED2_PIN, GREEN_LED3_PIN, GREEN_LED4_PIN };
int buttons[BUTTON_COUNT] = { BUTTON1_PIN, BUTTON2_PIN, BUTTON3_PIN, BUTTON4_PIN };

// Timing variables
unsigned long previousMillis;
unsigned long timerInterval;

LiquidCrystal_I2C lcd(0x27, 16, 2); // 0x27 is the most common address for I2C LCDs

// One time setup instructions
void setup();

// Executes tasks based on the current game state
void loop();

// Shows a welcome page while also setting things up
void initializeGame();

// Sets the difficulty choosen. Will either go to next state or put the device to sleep
void setupDifficulty();

// Sets up a new round
void setupRound();

// Reads user input and checks the win condition
void processGame();

// Successful round scenario
void resolveRound();

// Unsuccesful round scenario
void showGameOver();

// Powers on/ff the leds accordingly
void updateLEDs();

// Fades the red led
void manageRedLED();

// Puts the device to sleep
void enterSleepMode();

// Instructions to be executed upon wake
void wakeUp();

void setup() {
    randomSeed(analogRead(UNCONNECTED_ANALOG_PIN));

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

    gameState = STATE_INITIALIZE;
}

void loop() {
    switch (gameState) {
    case STATE_INITIALIZE:
        initializeGame();
        break;
    case STATE_SETTINGS:
        setupDifficulty();
        break;
    case STATE_ROUND_SETUP:
        setupRound();
        break;
    case STATE_PROCESS_GAME:
        processGame();
        break;
    case STATE_RESOLVE_ROUND:
        resolveRound();
        break;
    case STATE_GAME_OVER:
        showGameOver();
        break;
    default:
        break;
    }
}

void initializeGame() {
    // Resets game variables
    redLedBrightness = 0;
    redLedBrightnessStep = RED_LED_BRIGHTNESS_STEP;
    score = 0;
    difficulty = VERY_EASY;
    currentMaxTime = ROUND_TIME_WINDOW;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Welcome to GMB!");
    delay(1000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Choose the");
    lcd.setCursor(0, 1);
    lcd.print("difficulty");
    delay(INITIALIZATION_DELAY);
    // Sets up time window for difficulty setup
    gameState = STATE_SETTINGS;
    timerInterval = SET_DIFFICULTY_WINDOW;
    timeWindowHasElapsed = false;
    previousMillis = millis();
}

void setupDifficulty() {
    if (timeWindowHasElapsed) {
        enterSleepMode();
    } else {
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= timerInterval) {
            timeWindowHasElapsed = true;
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
            currentMaxTime = ROUND_TIME_WINDOW - TIME_DELTA * difficulty;
        }

        manageRedLED();

        if (digitalRead(buttons[0]) == HIGH) {
            digitalWrite(RED_LED_PIN, LOW);
            gameState = STATE_ROUND_SETUP;
        }
    }
}

void setupRound() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Go!");
    currentRandomValue = random(0, MAX_RANDOM + 1);
    lcd.setCursor(0, 1);
    lcd.print("Value: " + String(currentRandomValue));
    // Sets up the time window for the round
    gameState = STATE_PROCESS_GAME;
    timerInterval = currentMaxTime;
    previousMillis = millis();
}

void processGame() {
    if (timeWindowHasElapsed) {
        gameState = STATE_GAME_OVER;
    } else {
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= timerInterval) {
            timeWindowHasElapsed = true;
        }
        updateLEDs();
        int sum = 0;
        for (int i = 0; i < LED_COUNT; i++)
        {
            if (digitalRead(leds[i]) == HIGH) {
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
            gameState = STATE_RESOLVE_ROUND;
        }
    }
}

void resolveRound() {
    score++;
    currentMaxTime = currentMaxTime - TIME_DELTA;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("You won!");
    lcd.setCursor(0, 1);
    lcd.print("Score: " + String(score));
    delay(ROUND_RESOLUTION_DELAY);
    gameState = STATE_ROUND_SETUP;
}

void showGameOver() {
    digitalWrite(RED_LED_PIN, HIGH);
    delay(RED_LIGHT_DELAY);
    digitalWrite(RED_LED_PIN, LOW);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Game Over!");
    lcd.setCursor(0, 1);
    lcd.print("Score: " + String(score));
    delay(GAME_OVER_DELAY);
    gameState = STATE_INITIALIZE;
}

void updateLEDs() {
    for (int i = 0; i < BUTTON_COUNT; i++) {
        if (digitalRead(buttons[i]) == HIGH) {
            digitalWrite(leds[i], HIGH);
        } else {
            digitalWrite(leds[i], LOW);
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
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);    // set the sleep mode
    sleep_enable();                         // removes the safety pin
    enableInterrupt(BUTTON1_PIN, wakeUp, HIGH);
    enableInterrupt(BUTTON2_PIN, wakeUp, HIGH);
    enableInterrupt(BUTTON3_PIN, wakeUp, HIGH);
    enableInterrupt(BUTTON4_PIN, wakeUp, HIGH);
    sleep_mode();                           // put to sleep here
    sleep_disable();                        // enables all function
    disableInterrupt(BUTTON1_PIN);
    disableInterrupt(BUTTON2_PIN);
    disableInterrupt(BUTTON3_PIN);
    disableInterrupt(BUTTON4_PIN);
    lcd.backlight();
}

void wakeUp() {
    gameState = STATE_INITIALIZE;
}
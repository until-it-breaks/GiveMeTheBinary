#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <avr/sleep.h>
#include <EnableInterrupt.h>

//TODO Improve Red LED fade, process game inputs, disable rogue red LED

/**
 * A simple binary numbers game for the Arduino Uno R3
 * Circuit reference: https://www.tinkercad.com/things/epSlpAZTefP-givemethebinary?sharecode=rppd_5WtQ8WaYqnvDhg68Ee-5P04I2m4gol8a1v37rE
 **/

// Constants
#define LED_COUNT 4
#define BUTTON_COUNT 4
#define MAX_RANDOM 15
#define RED_LED_BRIGHTNESS_STEP 1
#define VERY_EASY 1
#define EASY 2
#define NORMAL 3
#define HARD 4

// Time windows in milliseconds
#define INITIALIZATION_DELAY 5000
#define GAME_OVER_DELAY 10000
#define SET_DIFFICULTY_WINDOW 10000
#define ROUND_SETUP_DELAY 1000
#define ROUND_RESOLUTION_DELAY 2000
#define ROUND_MAX_TIME_WINDOW 15000
#define ROUND_MIN_TIME_WINDOW 3000
#define ROUND_TIME_DELTA 1000
#define RED_LIGHT_DELAY 1000

// Pins
/**
 * Pin 4, 5, 6, 7 are capable of interrupts thanks to EnableInterrupt
 **/ 
#define BUTTON1_PIN 4
#define BUTTON2_PIN 5
#define BUTTON3_PIN 6
#define BUTTON4_PIN 7
#define GREEN_LED1_PIN 10
#define GREEN_LED2_PIN 11
#define GREEN_LED3_PIN 12
#define GREEN_LED4_PIN 13
#define RED_LED_PIN 9               // Must be a PWM capable pin
#define POTENTIOMETER_PIN A0
#define UNCONNECTED_ANALOG_PIN A3   // Must be unused for better randomness

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
int difficulty;
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

// Unsuccessful round scenario
void showGameOver();

// Powers on/off the LEDs accordingly
void updateLEDs();

// Fades the red LED
void fadeRedLED();

// Puts the device to sleep
void enterSleepMode();

// Instructions to be executed upon wake
void wakeUp();

// Maps input value to the corresponding difficulty
int getDifficulty(int value);

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
    currentMaxTime = ROUND_MAX_TIME_WINDOW;
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
        digitalWrite(RED_LED_PIN, LOW);
        enterSleepMode();
    } else {
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= timerInterval) {
            timeWindowHasElapsed = true;
        }

        int potentiometerValue = analogRead(POTENTIOMETER_PIN);
        int newDifficulty = getDifficulty(potentiometerValue);
        if (difficulty != newDifficulty) {
            difficulty = newDifficulty;
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Difficulty: " + String(difficulty));
            lcd.setCursor(0, 1);
            lcd.print("Tap B1 to start");
        }

        fadeRedLED();

        if (digitalRead(buttons[0]) == HIGH) {
            digitalWrite(RED_LED_PIN, LOW);
            gameState = STATE_ROUND_SETUP;
        }
    }
}

void setupRound() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Go! Time: " + String(currentMaxTime/1000) + "s ");
    currentRandomValue = random(0, MAX_RANDOM + 1);
    lcd.setCursor(0, 1);
    lcd.print("Value: " + String(currentRandomValue));
    delay(ROUND_SETUP_DELAY);
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
    if (currentMaxTime - (ROUND_TIME_DELTA * difficulty) >= ROUND_MIN_TIME_WINDOW) {
        currentMaxTime -= ROUND_TIME_DELTA * difficulty;
    }
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
    lcd.print("Final Score: " + String(score));
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

void fadeRedLED() {
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

int getDifficulty(int value) {
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
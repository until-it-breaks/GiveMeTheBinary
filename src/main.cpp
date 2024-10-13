#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <avr/sleep.h>
#include <EnableInterrupt.h>
#include "gmbFunctions.h"
#include "gameStates.h"
#include "defines.h"
/**
 * A simple binary numbers game for the Arduino Uno R3
 * Circuit reference: https://www.tinkercad.com/things/epSlpAZTefP-givemethebinary?sharecode=rppd_5WtQ8WaYqnvDhg68Ee-5P04I2m4gol8a1v37rE
 **/

// TODO Better win condition logic (invert), handle possible red led edge case, modularize into main.cpp, functions.c, function.h, defines.h, LCD still has text with lcd off

// Game variables
int difficulty;
int score;
int currentRandomValue;
int currentMaxTime;

// Red LED variables
int redLedBrightness;
int redLedBrightnessStep;
unsigned long previousFadeMillis; 

// Timing variables
unsigned long previousMillis;
unsigned long timerInterval;
bool timeWindowHasElapsed;

// Arrays
int leds[LED_COUNT] = { GREEN_LED1_PIN, GREEN_LED2_PIN, GREEN_LED3_PIN, GREEN_LED4_PIN };
int buttons[BUTTON_COUNT] = { BUTTON1_PIN, BUTTON2_PIN, BUTTON3_PIN, BUTTON4_PIN };

LiquidCrystal_I2C lcd(0x27, 16, 2); // 0x27 is the most common address for I2C LCDs 
GameState gameState = STATE_INITIALIZE;

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
    previousFadeMillis = 0;
    score = 0;
    difficulty = VERY_EASY;
    currentMaxTime = ROUND_MAX_TIME_WINDOW;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Welcome to GMB!");
    delay(INITIALIZATION_DELAY);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Choose the");
    lcd.setCursor(0, 1);
    lcd.print("difficulty");
    delay(INITIALIZATION_DELAY);
    // Sets up time window for difficulty setup
    gameState = STATE_SETTINGS;
    timerInterval = SET_DIFFICULTY_TIME_WINDOW;
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
    lcd.print("Go! Time: " + String(currentMaxTime/1000) + "s");
    currentRandomValue = random(0, MAX_RANDOM + 1);
    lcd.setCursor(0, 1);
    lcd.print("Value: [" + String(currentRandomValue) + "]");
    delay(ROUND_SETUP_DELAY);
    // Sets up the time window for the upcoming round
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
        // This next section could be optimized
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
    // The time window decreases each successful round by an amount multiplied by the difficulty scale
    if (currentMaxTime - (ROUND_TIME_DELTA * difficulty) >= ROUND_MIN_TIME_WINDOW) {
        currentMaxTime -= ROUND_TIME_DELTA * difficulty;
    }
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("You won!");
    lcd.setCursor(0, 1);
    lcd.print("Score: " + String(score));
    delay(ROUND_RESOLUTION_DELAY);
    turnOffLEDs();
    gameState = STATE_ROUND_SETUP;
}

void showGameOver() {
    digitalWrite(RED_LED_PIN, HIGH);
    delay(RED_LED_DELAY);
    digitalWrite(RED_LED_PIN, LOW);
    turnOffLEDs();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Game Over!");
    lcd.setCursor(0, 1);
    lcd.print("Final Score: " + String(score));
    delay(GAME_OVER_DELAY);
    gameState = STATE_INITIALIZE;
}


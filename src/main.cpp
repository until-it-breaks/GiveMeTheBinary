#include <Arduino.h>
#include <functions.h>

/**
 * A simple binary numbers game for the Arduino Uno R3
 * Authors: Jiekai Sun, Weijie Fu
 * Circuit reference: https://www.tinkercad.com/things/epSlpAZTefP-givemethebinary?sharecode=rppd_5WtQ8WaYqnvDhg68Ee-5P04I2m4gol8a1v37rE
 **/

// Game variables
GameState gameState;
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
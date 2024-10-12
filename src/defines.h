#ifndef DEFINES_H   // If DEFINES_H is not defined
#define DEFINES_H 
// Constants
#define LED_COUNT 4
#define BUTTON_COUNT 4
#define MAX_RANDOM 15
#define RED_LED_BRIGHTNESS_STEP 5
#define VERY_EASY 1
#define EASY 2
#define NORMAL 3
#define HARD 4

// Time windows in milliseconds
#define INITIALIZATION_DELAY 2000
#define GAME_OVER_DELAY 10000
#define SET_DIFFICULTY_TIME_WINDOW 10000
#define ROUND_SETUP_DELAY 1000
#define ROUND_RESOLUTION_DELAY 2000
#define ROUND_MAX_TIME_WINDOW 15000
#define ROUND_MIN_TIME_WINDOW 3000
#define ROUND_TIME_DELTA 1000
#define RED_LED_DELAY 1000
#define RED_LED_FADE_DELAY 30

// Pins
#define BUTTON1_PIN 4               //Pin 4, 5, 6, 7 are interrupt capable thanks to EnableInterrupt
#define BUTTON2_PIN 5
#define BUTTON3_PIN 6
#define BUTTON4_PIN 7
#define GREEN_LED1_PIN 10
#define GREEN_LED2_PIN 11
#define GREEN_LED3_PIN 12
#define GREEN_LED4_PIN 13
#define RED_LED_PIN 9               // Must be a PWM capable pin
#define POTENTIOMETER_PIN A0
#define UNCONNECTED_ANALOG_PIN A3  

int leds[LED_COUNT] = { GREEN_LED1_PIN, GREEN_LED2_PIN, GREEN_LED3_PIN, GREEN_LED4_PIN };
int buttons[BUTTON_COUNT] = { BUTTON1_PIN, BUTTON2_PIN, BUTTON3_PIN, BUTTON4_PIN };

enum GameState {
    STATE_INITIALIZE,
    STATE_SETTINGS,
    STATE_ROUND_SETUP,
    STATE_PROCESS_GAME,
    STATE_RESOLVE_ROUND,
    STATE_GAME_OVER
};

#include <LiquidCrystal_I2C.h>
extern LiquidCrystal_I2C lcd;

GameState gameState;

#endif
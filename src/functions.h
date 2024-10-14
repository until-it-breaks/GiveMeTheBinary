#ifndef __FUNCTIONS_H__
#define __FUNCTIONS_H__

#include <defines.h>
#include <LiquidCrystal_I2C.h>

extern GameState gameState;
extern int difficulty;
extern int score;
extern int currentRandomValue;
extern int currentMaxTime;
extern int redLedBrightness;
extern int redLedBrightnessStep;
extern unsigned long previousFadeMillis;
extern unsigned long previousMillis;
extern unsigned long timerInterval;
extern bool timeWindowHasElapsed;
extern int leds[];
extern int buttons[];
extern LiquidCrystal_I2C lcd;

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

// Turns off all LEDs;
void turnOffLEDs();

// Fades the red LED
void fadeRedLED();

// Puts the device to sleep
void enterSleepMode();

// Instructions to be executed upon wake
void wakeUp();

// Maps an input value to the corresponding difficulty
int getDifficulty(int value);

#endif
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <avr/sleep.h>
#include <EnableInterrupt.h>
#include "gmbFunctions.h"
#include "defines.h"

extern int leds[LED_COUNT];
extern int buttons[BUTTON_COUNT];

// Red LED variables
int redLedBrightness;
int redLedBrightnessStep;
unsigned long previousFadeMillis; 
extern LiquidCrystal_I2C lcd;

void updateLEDs() {
    for (int i = 0; i < BUTTON_COUNT; i++) {
        if (digitalRead(buttons[i]) == HIGH) {
            digitalWrite(leds[i], HIGH);
        } else {
            digitalWrite(leds[i], LOW);
        }
    }
}

void turnOffLEDs() {
    for (int i = 0; i < LED_COUNT; i++) {
        digitalWrite(leds[i], LOW);
    }
}

void fadeRedLED() {
    unsigned long currentMillis = millis();
    if (currentMillis - previousFadeMillis >= RED_LED_FADE_DELAY) {
        previousFadeMillis = currentMillis;
        // Possible edge case where the an out of bound value is written
        analogWrite(RED_LED_PIN, redLedBrightness);
        redLedBrightness += redLedBrightnessStep;
        if (redLedBrightness <= 0 || redLedBrightness >= 255) {
            redLedBrightnessStep = -redLedBrightnessStep;
        }
    }
}

void enterSleepMode() {
    lcd.noBacklight();
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);            // Sets the sleep mode
    sleep_enable();                                 // Removes the safety pin
    enableInterrupt(BUTTON1_PIN, wakeUp, RISING);   // RISING is preferrable to HIGH since it only triggers once on press
    enableInterrupt(BUTTON2_PIN, wakeUp, RISING);
    enableInterrupt(BUTTON3_PIN, wakeUp, RISING);
    enableInterrupt(BUTTON4_PIN, wakeUp, RISING);
    sleep_mode();                                   // Actually put to sleep here
    sleep_disable();                                // Re-enables all functions
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
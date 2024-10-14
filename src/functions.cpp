#include <functions.h>
#include <EnableInterrupt.h>
#include <avr/sleep.h>

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
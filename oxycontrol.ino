/*
 * OxyControl - opioid abuse prevention device
 *
 * Copyright (C) 2020 Marc Chmielewski
 *                    Chloe White,
 *                    Juan Lasso Velasco, and
 *                    Franklin Wei
 */

#include <LiquidCrystal.h>
#include <Servo.h>

#define READY_LED 8
#define DISPENSE_BUTTON 9

//#define NDEBUG
#ifdef NDEBUG
#define DOSAGE_INTERVAL_MILLIS (12UL * 3600 * 1000)
#define DISABLE_TIMEOUT_MILLIS (24UL * 3600 * 1000)
#else
#define DOSAGE_INTERVAL_MILLIS 5000
#define DISABLE_TIMEOUT_MILLIS 10000
#define SIMULATION_SPEEDUP 9 /* compensate for tinkercad's slow speed */
#define millis() (millis() * SIMULATION_SPEEDUP)
#endif

#define DEGREES_PER_PILL 24
#define SCREEN_UPDATE_INTERVAL 1000

enum { STATE_READY, STATE_BLOCKED, STATE_EMPTY, STATE_DISABLED } currentState;

union {
    struct {
        unsigned long disableTimeMillis;
    } readyData;
    struct {
        unsigned long unlockTimeMillis;
    } blockedData;
} stateData;

int lastButtonState = HIGH;
int dispensedPills = 0;
const int totalPills = 7;

Servo dispenseServo;

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
unsigned long lastScreenUpdate = 0;

void setLED(bool state) {
    digitalWrite(8, state ? HIGH: LOW);
}

char *formatHMS(char *buf, long time_ms) {
    if(time_ms < 0) {
        strcpy(buf, "*error*");
        return buf;
    }

    long seconds_total = time_ms / 1000 + 1;

    long disp_s = seconds_total % 60;
    long disp_m = (seconds_total / 60) % 60;
    long disp_h = seconds_total / 3600;

    /* work around crashing sprintf() in TinkerCAD */
    char *ptr = buf;
    ltoa(disp_h, ptr, 10);
    ptr += strlen(ptr);
    *ptr++ = ':';
    *ptr++ = disp_m / 10 + '0';
    *ptr++ = disp_m % 10 + '0';
    *ptr++ = ':';
    *ptr++ = disp_s / 10 + '0';
    *ptr++ = disp_s % 10 + '0';
    *ptr++ = 0;

    return buf;
}

void updateIndicators() {
    setLED(currentState == STATE_READY);
    lcd.clear();

    char buf[128], hmsbuf[128];
    char *ptr = buf;
    switch(currentState) {
    case STATE_READY:
    case STATE_EMPTY:
        lcd.print(currentState == STATE_READY ? "Dispenser ready." : "Out of pills.");
        lcd.setCursor(0, 1);
        lcd.print(totalPills - dispensedPills);
        lcd.print('/');
        lcd.print(totalPills);
        lcd.print(" remaining");
        break;
    case STATE_DISABLED:
        lcd.print("Device locked.");
        break;
    case STATE_BLOCKED:
        formatHMS(hmsbuf, stateData.blockedData.unlockTimeMillis - millis());
        lcd.print("Locked for:");
        lcd.setCursor(0, 1);
        lcd.print(hmsbuf);
        break;
    default:
        lcd.print("Internal error");
        break;
    }

    lastScreenUpdate = millis();
}

void becomeReady() {
    currentState = STATE_READY;
    stateData.readyData.disableTimeMillis = millis() + DISABLE_TIMEOUT_MILLIS;
}

void setup() {
    becomeReady();

    pinMode(READY_LED, OUTPUT); // dose ready LED
    pinMode(DISPENSE_BUTTON, INPUT_PULLUP); // dispense button
    dispenseServo.attach(10);
    dispenseServo.write(0);

    // set up the LCD's number of columns and rows:
    lcd.begin(16, 2);
    updateIndicators();

    dispensedPills = 0;

    Serial.begin(9600);
}

void advanceMotor() {
    dispenseServo.write(dispensedPills * DEGREES_PER_PILL);
}

void doDispense() {
    dispensedPills++; // order matters here -- advanceMotor() depends
                      // on this value.
    advanceMotor();

    if(dispensedPills == totalPills) {
        currentState = STATE_EMPTY;
        return;
    }

    currentState = STATE_BLOCKED;
    stateData.blockedData.unlockTimeMillis = millis() + DOSAGE_INTERVAL_MILLIS;
}

void loop() {
    int currentButtonState = digitalRead(DISPENSE_BUTTON);

    // high is not pressed, low is pressed
    bool dispenseRequested = (currentButtonState == HIGH && lastButtonState == LOW);

    bool changedState = false;

    switch(currentState) {
    case STATE_READY:
        if(dispenseRequested) {
            doDispense();
            changedState = true;
        }
        if(millis() > stateData.readyData.disableTimeMillis) {
            currentState = STATE_DISABLED;
            changedState = true;
        }
        break;
    case STATE_BLOCKED:
        if(millis() > stateData.blockedData.unlockTimeMillis) {
            becomeReady();
            changedState = true;
        }
        break;
    case STATE_EMPTY:
    case STATE_DISABLED:
    default:
        break;
    }

    if(changedState || (currentState == STATE_BLOCKED && millis() > lastScreenUpdate + SCREEN_UPDATE_INTERVAL)) {
        updateIndicators();
    }

    lastButtonState = currentButtonState;
}

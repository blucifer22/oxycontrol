/*
 * OxyControl - opioid abuse prevention device
 *
 * Copyright (C) 2020 Caroline Anderson,
 *                    Marc Chmielewski,
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
#define MIN_DELAY_MILLIS (12UL * 3600 * 1000)
#else
#define MIN_DELAY_MILLIS 5000
#endif

#define DEGREES_PER_PILL 24
#define SCREEN_UPDATE_INTERVAL 1000

enum { STATE_READY, STATE_BLOCKED, STATE_EMPTY } currentState;

union {
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
    Serial.println("point 2");

    if(time_ms < 0) {
        strcpy(buf, "*error*");
        return buf;
    }
    Serial.println("point 3");

    long seconds_total = time_ms / 1000;

    long disp_s = seconds_total % 60;
    long disp_m = (seconds_total / 60) % 60;
    long disp_h = seconds_total / 3600;
    Serial.println(disp_h);
    Serial.println(disp_m);
    Serial.println(disp_s);

    Serial.println("point 4");

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

    Serial.println(buf);

    Serial.println("point 5");

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

void setup() {
    currentState = STATE_READY;

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
    stateData.blockedData.unlockTimeMillis = millis() + MIN_DELAY_MILLIS;
}

void loop() {
    int currentButtonState = digitalRead(DISPENSE_BUTTON);

    Serial.println(currentButtonState);

    // high is not pressed, low is pressed
    bool dispenseRequested = (currentButtonState == HIGH && lastButtonState == LOW);

    bool changedState = false;

    switch(currentState) {
    case STATE_READY:
        if(dispenseRequested) {
            doDispense();
            changedState = true;
        }
        break;
    case STATE_BLOCKED:
        if(millis() > stateData.blockedData.unlockTimeMillis) {
            currentState = STATE_READY;
            changedState = true;
        }
        break;
    case STATE_EMPTY:
    default:
        break;
    }

    if(changedState || (currentState == STATE_BLOCKED && millis() > lastScreenUpdate + SCREEN_UPDATE_INTERVAL)) {
        updateIndicators();
    }

    lastButtonState = currentButtonState;
}

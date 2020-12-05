#include <LiquidCrystal.h>
#include <Servo.h>

// DO NOT LEAVE ON FOR MORE THAN 50 DAYS CONTINUOUSLY!!!!
// Counter will overflow

// Initialize the LCD ibrary with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
Servo servo;
unsigned int pill_number, time, count_up;


// This runs at power-on or when the board is reset
void setup() {
  // Set up pinmodes
  pinMode(9, INPUT); // Pushbutton INPUT
  pinMode(8, OUTPUT); // LED for pushbutton
  
  // Set up the LCD's number of columns and rows:
  // And print a friendly welcome message
  lcd.begin(16, 2);
  lcd.print("OxyControl v1.0");
  
  // Attach servo and initialize
  // to starting location
  servo.attach(10); // Attach the servo
  servo.write(0); // Set to initial location
  digitalWrite(8, LOW);
  pill_number = 7;
  time = 0;
  count_up = 0;
}

// This runs continuously while the board is powered
void loop() {
  while(pill_number > 0) {
    if(time != 0) {
      
    }
  }
    // While pill number > 0
    // if time is !4.32e+7
    // Update time
    // Display time
    // else
    // Light button
    // Dispense now on LCD
    // Start count-up time
    // if button is pressed && count-up is less than 24
    // Reset count up
    // Increment servo
    // Decrement pill count
    // Reset count down
    // Turn off light
    // elif count-up >= 24 BREAK

    // LOCKOUT and display take to pharm
}

#include <Arduino.h>

#include "DollhousePanel.h"

#include <Adafruit_TLC59711.h>
#include <Adafruit_NeoPixel.h>
#include <LiquidCrystal.h>
#include <SPI.h>

const char NUM_TLC59711=1;
const char TLC_DATA=12;
const char TLC_CLK=13;

const char PIXEL_COUNT = 3;
const char PIXEL_PIN = 8;

enum lightingModes {
  MODE_LIGHTING,
  MODE_PARTY,
  MODE_NITELIGHT,
  MODE_OFF
};

int currentMode = 0;

// The button interface is a Smartmaker 5A5 (annoying, but it works)
enum analogButtons {
  BUTTON_ONE,
  BUTTON_TWO,
  BUTTON_THREE,
  BUTTON_FOUR,
  BUTTON_FIVE
};

// NeoPixels (for the attic & !!!PARTY MODE!!!)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

// PWM board (controls the room lights)
Adafruit_TLC59711 tlc = Adafruit_TLC59711(NUM_TLC59711, TLC_CLK, TLC_DATA);

// 16x2 LCD display
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

// Panel RGB LED pins
const char RED_PIN = 9;
const char GREEN_PIN = 10;
const char BLUE_PIN = 11;

int brightness = 90;
int roomBrightness[] = {brightness * 180, brightness * 180};
int numberOfRooms = sizeof(roomBrightness);
int currentRoom = 0;

// Attic lights are Adafruit NeoPixels!


const int ENABLED = 0;
const int NOT_ENABLED = 1023; // use an explicit value to avoid triggering on floating input

int buttonOneState = 1023;
int buttonTwoState = 1023;
int buttonThreeState = 1023;
int buttonFourState = 1023;
int buttonFiveState = 1023;

int buttonOnePrevState = 1023;
int buttonTwoPrevState = 1023;
int buttonThreePrevState = 1023;
int buttonFourPrevState = 1023;
int buttonFivePrevState = 1023;

void setup() {
  // Fire up the LCD display
  lcd.begin(16, 2);
  lcd.print("Doll house");
  lcd.setCursor(0,1);
  lcd.print("lighting!");

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  // initialize the NeoPixel strand
  strip.begin();
  strip.show();

  // Initialize the PWM board
  tlc.begin();
  tlc.write();
}

void readButtonStates() {
  buttonOneState = analogRead(BUTTON_ONE);
  buttonTwoState = analogRead(BUTTON_TWO);
  buttonThreeState = analogRead(BUTTON_THREE);
  buttonFourState = analogRead(BUTTON_FOUR);
  buttonFiveState = analogRead(BUTTON_FIVE);
  delay(100);
}

void buttonHandler(int button, int &state, int &prevState, void(*handler)()) {
  if (state == ENABLED && state != prevState) {
    (*handler)();
    prevState = state;
  } else if (state == NOT_ENABLED && state != prevState) {
    prevState = state;
    lcd.clear();
  }
}

void handleButtonOne() {
  lcd.clear();
  lcd.print("modeset!");
}

void handleButtonTwo() {
  lcd.clear();
  lcd.print("Brightness++");
  lcd.setCursor(0,1);
  brightness = min(brightness + 30, 180);
  lcd.print(brightness);
  setRGBColor(0,0,brightness);
  setRoomBrightness(currentRoom,brightness * 180);
  delay(200);
}

void handleButtonThree() {
  lcd.clear();
  currentRoom = !currentRoom;
  lcd.print("Prev room");
}

void handleButtonFour() {
  lcd.clear();
  lcd.print("Brightness--");
  lcd.setCursor(0,1);
  brightness = max(brightness - 30, 0);
  lcd.print(brightness);
  setRGBColor(0,0,brightness);
  setRoomBrightness(currentRoom,brightness * 180);
  delay(200);
}

void handleButtonFive() {
  lcd.clear();
  currentRoom = !currentRoom;
  lcd.print("Next room");
}

void setRGBColor(int red, int green, int blue) {
  int myRed = constrain(red, 0, 255);
  int myGreen = constrain(green, 0, 255);
  int myBlue = constrain(blue, 0, 255);

  analogWrite(RED_PIN, myRed);
  analogWrite(GREEN_PIN, myGreen);
  analogWrite(BLUE_PIN, myBlue);
}

void setRoomBrightness(int room, int level) {
  roomBrightness[room] = level;
  tlc.setPWM(room * 3, roomBrightness[room]);
  tlc.write();
}

void lightRooms() {
  for (int i = 0; i < 6; i++) {

  }
}

void loop() {
  setRGBColor(0,0,brightness);
  readButtonStates();
  buttonHandler(BUTTON_ONE, buttonOneState, buttonOnePrevState, handleButtonOne);
  buttonHandler(BUTTON_TWO,buttonTwoState, buttonTwoPrevState, handleButtonTwo);
  buttonHandler(BUTTON_THREE, buttonThreeState, buttonThreePrevState, handleButtonThree);
  buttonHandler(BUTTON_FOUR, buttonFourState, buttonFourPrevState, handleButtonFour);
  buttonHandler(BUTTON_FIVE, buttonFiveState, buttonFivePrevState, handleButtonFive);
}

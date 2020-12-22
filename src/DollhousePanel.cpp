#include <Arduino.h>

#include "DollhousePanel.h"
#include "TimingHelpers.h"

#include <Adafruit_TLC59711.h>
#include <Adafruit_NeoPixel.h>
#include <LiquidCrystal.h>
#include <SPI.h>
#include <Fsm.h>
#include <EnableInterrupt.h>

const char NUM_TLC59711 = 1;
const char TLC_DATA = 12;
const char TLC_CLK = 13;

const char PIXEL_COUNT = 3;
const char PIXEL_PIN = 8;

enum events {
  CHANGE_LIGHT_MODE,
  NEXT_ROOM,
  PREVIOUS_ROOM,
  RESET_ROOMS
};

// Lighting modes finite state machine
State state_lighting_mode(on_lighting_mode_enter, NULL, &on_lighting_mode_exit);
State state_party_mode(on_party_mode_enter, NULL, &on_party_mode_exit);
State state_nitelite_mode(on_nitelite_mode_enter, NULL, &on_nitelite_mode_exit);
State state_off_mode(on_off_mode_enter, NULL, &on_off_mode_exit);
Fsm modes(&state_off_mode);

enum Modes {
  LIGHTING_MODE,
  PARTY_MODE,
  NITELITE_MODE,
  OFF_MODE
};

String modeNames[] = {"Lighting", "Party", "Nitelite", "Off"};

// Rooms finite state machine
State state_all_rooms(on_all_enter, NULL, &on_all_exit);
State state_hall(on_hall_enter, NULL, &on_hall_exit);
State state_living_room(on_living_room_enter, NULL, &on_living_room_exit);
State state_kitchen(on_kitchen_enter, NULL, &on_kitchen_exit);
State state_bedroom(on_bedroom_enter, NULL, &on_bedroom_exit);
State state_bathroom(on_bathroom_enter, NULL, &on_bathroom_exit);
State state_attic(on_attic_enter, NULL, &on_attic_exit);
Fsm rooms(&state_all_rooms);

// LastROOM is included to make it easier to figure out the size of the enum
// for things like sizing the brightness state array
enum Rooms {
  ALL_ROOMS,
  LIVING_ROOM,
  HALL,
  KITCHEN,
  BEDROOM,
  BATHROOM,
  ATTIC,
  LastROOM
};

String roomNames[] = {"All", "Living", "Hall", "Kitchen", "Bedroom", "Bathroom", "Attic"};

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
int deltaLevel = 30;
int minLevel = 0;
int maxLevel = 180;
int roomBrightness[LastROOM];
int currentRoom = ALL_ROOMS;
int currentMode = OFF_MODE;

int debounceDelay = 250;
long timerDebounce = 0;

int niteliteTimeout = 15000;
long timeNitelite = 0;

void setup() {
  // Fire up the LCD display
  lcd.begin(16, 2);

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  // initialize the NeoPixel strand
  strip.begin();
  strip.show();

  // Initialize the PWM board
  tlc.begin();
  tlc.write();

  // set default room brightness
  setDefaultLightLevel();

  // enable interrupts on buttons
  // The button interface is a Smartmaker 5A5 (annoying, but it works)
  enableInterrupt(A0, handleButtonOne, FALLING);
  enableInterrupt(A1, handleButtonTwo, FALLING);
  enableInterrupt(A2, handleButtonThree, FALLING);
  enableInterrupt(A3, handleButtonFour, FALLING);
  enableInterrupt(A4, handleButtonFive, FALLING);
  
  // mode FSM transitions
  modes.add_transition(&state_off_mode, &state_lighting_mode, CHANGE_LIGHT_MODE, NULL);
  modes.add_transition(&state_lighting_mode, &state_party_mode, CHANGE_LIGHT_MODE, NULL);
  modes.add_transition(&state_party_mode, &state_nitelite_mode, CHANGE_LIGHT_MODE, NULL);
  modes.add_transition(&state_nitelite_mode, &state_off_mode, CHANGE_LIGHT_MODE, NULL);

  // rooms FSM transitions
  // looping "forward" through the rooms
  rooms.add_transition(&state_all_rooms, &state_hall, NEXT_ROOM, NULL);
  rooms.add_transition(&state_hall, &state_living_room, NEXT_ROOM, NULL);
  rooms.add_transition(&state_living_room, &state_kitchen, NEXT_ROOM, NULL);
  rooms.add_transition(&state_kitchen, &state_bedroom, NEXT_ROOM, NULL);
  rooms.add_transition(&state_bedroom, &state_bathroom, NEXT_ROOM, NULL);
  rooms.add_transition(&state_bathroom, &state_attic, NEXT_ROOM, NULL);
  rooms.add_transition(&state_attic, &state_all_rooms, NEXT_ROOM, NULL);

  // looping "backward" through the rooms
  rooms.add_transition(&state_all_rooms, &state_attic, PREVIOUS_ROOM, NULL);
  rooms.add_transition(&state_attic, &state_bathroom, PREVIOUS_ROOM, NULL);
  rooms.add_transition(&state_bathroom, &state_bedroom, PREVIOUS_ROOM, NULL);
  rooms.add_transition(&state_bedroom, &state_kitchen, PREVIOUS_ROOM, NULL);
  rooms.add_transition(&state_kitchen, &state_living_room, PREVIOUS_ROOM, NULL);
  rooms.add_transition(&state_living_room, &state_hall, PREVIOUS_ROOM, NULL);
  rooms.add_transition(&state_hall, &state_all_rooms, PREVIOUS_ROOM, NULL);

  // resetting to the default room (all rooms)
  rooms.add_transition(&state_hall, &state_all_rooms, RESET_ROOMS, NULL);
  rooms.add_transition(&state_living_room, &state_all_rooms, RESET_ROOMS, NULL);
  rooms.add_transition(&state_kitchen, &state_all_rooms, RESET_ROOMS, NULL);
  rooms.add_transition(&state_bedroom, &state_all_rooms, RESET_ROOMS, NULL);
  rooms.add_transition(&state_bathroom, &state_all_rooms, RESET_ROOMS, NULL);
  rooms.add_transition(&state_attic, &state_all_rooms, RESET_ROOMS, NULL);
  
  // run each state machine once to initialize them; this is basically a NOOP
  // thanks the default state
  rooms.run_machine();
  modes.run_machine(); 
  
  lcd.clear();
  lcd.print("Doll house");
  lcd.setCursor(0,1);
  lcd.print("lighting!");
}

// ***** Button event handlers ***** //

// Use button one to set the light mode for all rooms
void handleButtonOne() {
  if (!still_bouncing()) {
    lcd.clear();
    rooms.trigger(RESET_ROOMS);
    modes.trigger(CHANGE_LIGHT_MODE);
  }
}

// Use button two to increase brightness for the current room
void handleButtonTwo() {
  lcd.display();
  if (!still_bouncing()) {
    setRoomBrightness(currentRoom, min(roomBrightness[currentRoom] + deltaLevel, maxLevel));
    printCurrentRoom();
  }
}

// Use button three to select the previous room
void handleButtonThree() {
  lcd.display();
  if (!still_bouncing()) {
    lcd.clear();
    rooms.trigger(PREVIOUS_ROOM);
  }
}

// Use button four to decrease brightness for the current room
void handleButtonFour() {
  lcd.display();
  if (!still_bouncing()) {
    setRoomBrightness(currentRoom, max(roomBrightness[currentRoom] - deltaLevel, minLevel));
    printCurrentRoom();
  }
}

// Use button five to select the next room
void handleButtonFive() {
  lcd.display();
  if (!still_bouncing()) {
    lcd.clear();
    rooms.trigger(NEXT_ROOM);
  }
}

// ***** helpers ***** //

void setRGBColor(int red, int green, int blue) {
  int myRed = constrain(red, 0, 255);
  int myGreen = constrain(green, 0, 255);
  int myBlue = constrain(blue, 0, 255);

  analogWrite(RED_PIN, myRed);
  analogWrite(GREEN_PIN, myGreen);
  analogWrite(BLUE_PIN, myBlue);
}

void setRoomBrightness(int room, int level) {
  setRGBColor(0,0,level);
  roomBrightness[room] = level;
  tlc.setPWM(room * 3, roomBrightness[room] * maxLevel);
  tlc.write();
}

void setDefaultLightLevel() {
  setRGBColor(0,0,brightness);
  for (int i = 0; i != LastROOM; i++) {
    roomBrightness[i] = brightness;
  }
}

void setCurrentMode(int mode) {
  currentMode = mode;
  printCurrentMode();
}

void printCurrentMode() {
  lcd.clear();
  lcd.print("Mode: ");
  lcd.print(modeNames[currentMode]);
}

void setCurrentRoom(int room) {
  currentRoom = room;
  setRGBColor(0,0,roomBrightness[room]);
  printCurrentRoom();
}

void printCurrentRoom() {
  lcd.clear();
  lcd.print("room: ");
  lcd.print(roomNames[currentRoom]);
  lcd.setCursor(0,1);
  lcd.print("brightness: ");
  lcd.print(roomBrightness[currentRoom]);
}

// ***** FSM event handlers ***** //

// ---- lighting mode states ---- //

void on_lighting_mode_enter(){
  setCurrentMode(LIGHTING_MODE);
}

void on_lighting_mode_exit(){
  
}

void on_party_mode_enter(){
  setCurrentMode(PARTY_MODE);
}

void on_party_mode_exit(){
  
}

void on_nitelite_mode_enter(){
  setCurrentMode(NITELITE_MODE);
  startTimer(timerNitelite);
}

void on_nitelite_mode_run(){
  if (isTimerExpired(timerNitelite, niteliteTimeout) {
    lcd.noDisplay();
    clearTimer(timerNitelite);
  }
}

void on_nitelite_mode_exit(){
  clearTimer(timerNitelite);
}

void on_off_mode_enter(){
  setCurrentMode(OFF_MODE);  
}

void on_off_mode_exit(){

}

// ---- room selection states ---- //
void on_all_enter() {
  setCurrentRoom(ALL_ROOMS);
}

void on_all_exit() {
  
}

void on_hall_enter() {
  setCurrentRoom(HALL);
}

void on_hall_exit() {
  
}

void on_living_room_enter() {
  setCurrentRoom(LIVING_ROOM);
}

void on_living_room_exit() {
  
}

void on_kitchen_enter() {
  setCurrentRoom(KITCHEN);
}

void on_kitchen_exit() {
  
}

void on_bathroom_enter() {
  setCurrentRoom(BATHROOM);
}

void on_bathroom_exit() {
  
}

void on_bedroom_enter() {
  setCurrentRoom(BEDROOM);
}

void on_bedroom_exit() {
  
}

void on_attic_enter() {
  setCurrentRoom(ATTIC);
}

void on_attic_exit() {
  
}

// Debounce timer
boolean still_bouncing() {
  // If the debounce timer is not running, then we can assume the buttons
  // aren't bouncing because nothing has been pressed recently
  if (timerDebounce == 0) {
    startTimer(timerDebounce);
    return false;
  }
  
  if (isTimerExpired(timerDebounce, debounceDelay)) {
    clearTimer(timerDebounce);
    startTimer(timerDebounce);
    return false;
  }
  
  return true;
}

void loop() {
  // do nothing; everything is handled via FSM events.
  // We also don't need to call the ".run_machine" methods of
  // the FSMs as there are no "on_state" handlers or timed transitions
}

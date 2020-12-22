void handleButtonOne();
void handleButtonTwo();
void handleButtonThree();
void handleButtonFour();
void handleButtonFive();
void setRGBColor(int red, int green, int blue);
void setRoomBrightness(int room, int level);
void lightRooms();

void setDefaultLightLevel();

void printCurrentMode();
void setCurrentMode(int mode);
void printCurrentRoom();
void setCurrentRoom(int room);

// ***** FSM event handlers ***** 

// ---- lighting mode states ---- 
void on_lighting_mode_enter();
void on_lighting_mode_exit();
void on_party_mode_enter();
void on_party_mode_exit();
void on_nitelite_mode_enter();
void on_nitelite_mode_exit();
void on_off_mode_enter();
void on_off_mode_exit();

// ---- room selection states ---- 
void on_all_enter();
void on_all_exit();
void on_hall_enter();
void on_hall_exit();
void on_living_room_enter();
void on_living_room_exit();
void on_kitchen_enter();
void on_kitchen_exit();
void on_bathroom_enter();
void on_bathroom_exit();
void on_bedroom_enter();
void on_bedroom_exit();
void on_attic_enter();
void on_attic_exit();

boolean still_bouncing();
void debounce_on_rise();

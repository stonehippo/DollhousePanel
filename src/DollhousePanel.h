void readButtonStates();
void buttonHandler(int button, int &state, int &prevState, void(*handler)());
void handleButtonOne();
void handleButtonTwo();
void handleButtonThree();
void handleButtonFour();
void handleButtonFive();
void setRGBColor(int red, int green, int blue);
void setRoomBrightness(int room, int level);
void lightRooms();

// FSM event handlers
void on_lighting_mode_enter();
void on_lighting_mode_exit();
void on_party_mode_enter();
void on_party_mode_exit();
void on_nitelite_mode_enter();
void on_nitelite_mode_exit();
void on_off_mode_enter();
void on_off_mode_exit();
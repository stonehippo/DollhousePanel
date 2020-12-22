/*
Copyright (C) 2015 George White <stonehippo@gmail.com>, All rights reserved.

See https://raw.githubusercontent.com/stonehippo/sploder/master/LICENSE.txt for license details. 
*/

// ******************* Timing helpers ******************* 
void startTimer(long &timer) {
  timer = millis(); 
}

boolean isTimerExpired(long &timer, long expiration) {
  long current = millis() - timer;
  return current > expiration;
}

void clearTimer(long &timer) {
  timer = 0; 
}

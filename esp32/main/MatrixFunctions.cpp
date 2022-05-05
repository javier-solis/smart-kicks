#include "Arduino.h"
#include "MatrixFunctions.h"
#include "LedControl.h"

// === Time Variables ===
// Note, these are all represented by their millisecond value, unless otherwise specified

// Constants
const int HALF_SEC = 500;
const int TENTH_SEC = 100;

// Timers

uint32_t blink_timer = 0; // Used for all animations, for now

// === Initiation & Config Variables ===

LedControl lc = LedControl(35,36,15,1);

void initialize_matrix(){
  lc.shutdown(0,false);
  lc.setIntensity(0,8); // Set brightness to a medium value
  lc.clearDisplay(0);
}

const int num_matrix_rows = 8;


// === Byte Arrays, Variables, and "Bitmaps" Used to Represent Content on the Matrix ===

byte center_bitmap[num_matrix_rows]= {B00000000,B00000000,B00000000,B00011000,B00011000,B00000000,B00000000,B00000000};

byte off_value = B00000000;


byte throbber_left = B00010000;
byte throbber_right = B00001000;


// === Static Displays ===
void show_center(){
  for(int row=0; row<num_matrix_rows; row++){
      lc.setRow(0,row, center_bitmap[row]);
  }
}

void clear_matrix(){
  for(int row=0; row<num_matrix_rows; row++){
      lc.setRow(0,row, off_value);
  }
}

// === Animations ===

int throbber_state = 0;
void throbber_animation(){
  clear_matrix();
  if(millis()-blink_timer>=TENTH_SEC){
    switch(throbber_state){
      case 0:
        lc.setRow(0, 3, throbber_left);
      break;
      case 1:
        lc.setRow(0, 3, throbber_right);
      break;
      case 2:
        lc.setRow(0, 4, throbber_right);
      break;
      case 3:
        lc.setRow(0, 4, throbber_left);
      	throbber_state=-1; // offset for the ++ later
      break;
    }
    throbber_state++;
    blink_timer=millis();
  }
}


void blinking_center_animation(){
  if(millis()-blink_timer>HALF_SEC*2){
    show_center();
    blink_timer=millis();
  }
  else if(millis() - blink_timer >HALF_SEC){
    clear_matrix();
  }
}

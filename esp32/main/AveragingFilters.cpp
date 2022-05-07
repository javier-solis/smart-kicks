#include "Arduino.h"
#include "AveragingFilters.h"

//enum for button states
enum button_state {S0,S1,S2,S3,S4};

IIR::IIR(float a, float y0){
  alpha = a;
  y_prev = y0;
}

float IIR::step(float input) {
  float y_n = alpha * y_prev + (1 - alpha) * input;
  y_prev = y_n;
  return y_n;
}

void IIR::set(float new_val) {
  y_prev = new_val;
}

void IIR::reset() {
  y_prev = 0;
}

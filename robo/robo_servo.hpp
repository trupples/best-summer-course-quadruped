#pragma once

namespace robo
{

#define SERVO_DUTY_BITS 14

static void ledc_init() {
  static bool ledc_initialized = false;
  if(ledc_initialized) return;
  ledc_initialized = true;

  analogWriteFreq(50);
  analogWriteResolution(SERVO_DUTY_BITS);
}

int servo_attach(int pin) {
  ledc_init();
  pinMode(pin, OUTPUT);
  return pin;
}

void servo_write(int channel, int duty) {
  analogWrite(channel, duty);
}

}

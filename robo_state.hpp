#pragma once

#include "robo_log.hpp"

namespace robo {
  
struct state_item {
  String name;
  enum { INT, FLOAT } type;
  void *addr;
};

// mechanical state: 8 angles, 4 x, 4 y, 4 tx, 4 ty
// ik state: height, pitch, yaw, roll
static state_item state_items[64]; // gonna need to bump up this 64 sometime soon
static int num_state_items = 0;

void state(const int *addr, const String name) {
  state_items[num_state_items++] = { name, state_item::INT, (void*)addr };
}

void state(const float *addr, const String name) {
  state_items[num_state_items++] = { name, state_item::FLOAT, (void*)addr };
}

void state_print() {
  for(int i = 0; i < num_state_items; i++) {
    if(state_items[i].type == state_item::INT) {
      Serial.printf("state %s %d\n", state_items[i].name.c_str(), *((int*)state_items[i].addr));
    } else if(state_items[i].type == state_item::FLOAT) {
      Serial.printf("state %s %f\n", state_items[i].name.c_str(), *((float*)state_items[i].addr));
    }
  }
}

void state_set(const String name, const String value) {
  for(int i = 0; i < num_state_items; i++) {
    if(state_items[i].name == name) {
      if(state_items[i].type == state_item::INT) {
        *((int*)state_items[i].addr) = std::atoi(value.c_str());
      } else if(state_items[i].type == state_item::FLOAT) {
        *((float*)state_items[i].addr) = std::atof(value.c_str());
      }
    }
  }
}

void state_set(const String name, const int value) {
  for(int i = 0; i < num_state_items; i++) {
    if(state_items[i].name == name) {
      *((int*)state_items[i].addr) = value;
    }
  }
}

void state_set(const String name, const float value) {
  for(int i = 0; i < num_state_items; i++) {
    if(state_items[i].name == name) {
      *((float*)state_items[i].addr) = value;
    }
  }
}

}

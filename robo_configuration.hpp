#pragma once
#include <Preferences.h>
#include "robo_log.hpp"

namespace robo {

Preferences preferences;

struct config_item {
  String name;
  enum { INT, FLOAT } type;
  void *addr;
};

static config_item config_items[64] = {};
static int num_config_items = 0;

void config(int *addr, const String name, const int default_value) {
  config_items[num_config_items++] = { name, config_item::INT, (void*) addr };

  preferences.begin("robo", true);

  if(preferences.isKey(name.c_str())) {
    *addr = preferences.getInt(name.c_str());
    LOG("robo_conf", "%s = %d (found)", name.c_str(), *addr);
  } else {
    *(int*)addr = default_value;
    preferences.end();
    preferences.begin("robo", false);
    preferences.putInt(name.c_str(), default_value);
    LOG("robo_conf", "%s = %d (defaulted)", name.c_str(), *addr);
  }

  preferences.end();
}

void config(float *addr, const String name, const float default_value) {
  config_items[num_config_items++] = { name, config_item::FLOAT, (void*) addr };
  
  preferences.begin("robo", true);

  if(preferences.isKey(name.c_str())) {
    *addr = preferences.getFloat(name.c_str());
    LOG("robo_conf", "%s = %f (found)", name.c_str(), *addr);
  } else {
    *(float*)addr = default_value;
    preferences.end();
    preferences.begin("robo", false);
    preferences.putFloat(name.c_str(), default_value);
    LOG("robo_conf", "%s = %f (defaulted)", name.c_str(), *addr);
  }

  preferences.end();
}

void config_print() {
  for(int i = 0; i < num_config_items; i++) {
    if(config_items[i].type == config_item::INT) {
      Serial.printf("config %s %d\n", config_items[i].name.c_str(), *((int*)config_items[i].addr));
    } else if(config_items[i].type == config_item::FLOAT) {
      Serial.printf("config %s %f\n", config_items[i].name.c_str(), *((float*)config_items[i].addr));
    }
  }
}

bool config_set(const String name, const int value) {
  for(int i = 0; i < num_config_items; i++) {
    if(config_items[i].name == name) {
      LOG("robo_conf", "%s = %d (set)", name.c_str(), value);
      *((int*)config_items[i].addr) = value;
      return true;
    }
  }
  return false;
}

bool config_set(const String name, const float value) {
  for(int i = 0; i < num_config_items; i++) {
    if(config_items[i].name == name) {
      LOG("robo_conf", "%s = %f (set)", name.c_str(), value);
      *((float*)config_items[i].addr) = value;
      return true;
    }
  }
  return false;
}

bool config_set(const String name, const String value_str) {
  for(int i = 0; i < num_config_items; i++) {
    if(config_items[i].name == name) {
      if(config_items[i].type == config_item::INT) {
        int value = std::atoi(value_str.c_str());
        LOG("robo_conf", "%s = %d (set)", name.c_str(), value);
        *((int*)config_items[i].addr) = value;
        return true;
      } else if(config_items[i].type == config_item::FLOAT) {
        float value = std::atof(value_str.c_str());
        LOG("robo_conf", "%s = %f (set)", name.c_str(), value);
        *((float*)config_items[i].addr) = value;
        return true;
      }
    }
  }
  return false;
}

void config_flush() {
  preferences.begin("robo", false);
  for(int i = 0; i < num_config_items; i++) {
      if(config_items[i].type == config_item::INT) {
        preferences.putInt(config_items[i].name.c_str(), *((int*)config_items[i].addr));
      } else if(config_items[i].type == config_item::FLOAT) {
        preferences.putFloat(config_items[i].name.c_str(), *((float*)config_items[i].addr));
      }
  }
  preferences.end();
}

}
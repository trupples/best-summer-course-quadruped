#pragma once

#include <LeanTask.h>

#include "robo_configuration.hpp"
#include "robo_state.hpp"
#include "robo_kinematics.hpp"
#include "robo_log.hpp"

namespace robo {

class SerialTask : public LeanTask {
public:
  SerialTask() : LeanTask() {}

  bool shouldRun() {
    return Serial.available() != 0;
  }

private:
  String cmd;

public:
  void setup() {
	  LOG("robo_serial", "Hello!");
  }

  void loop() {
    if(!shouldRun()) return;
    static String cmd;
    static bool got_line = false;
    while(Serial.available()) {
      int c = Serial.read();
      if(c == -1) { got_line = true; break; }
      if(c == '\n') { got_line = true; break; }
      cmd += (char) c;
    }
    if(!got_line) { // Read everything in the buffer. We don't want to hold up the CPU waiting on IO, so just yield
      delay(1);
      return;
    }

    if(cmd == "ping") {
        Serial.println("PONG!");
      } else if(cmd == "config") {
        config_print();
      } else if(cmd.startsWith("config ")) {
        int i = cmd.indexOf(' ', 7);
        String name = cmd.substring(7, i);
        i = cmd.indexOf(' ', i) + 1;
        String value = cmd.substring(i);
        
        LOG("serial", "Set configuration <%s> to <%s>\n", name.c_str(), value.c_str());
        config_set(name.c_str(), value.c_str());
      } else if(cmd == "config_flush") {
        LOG("serial", "Saved configuration");
        config_flush();
      } else if(cmd == "state") {
        state_print();
      } else if(cmd.startsWith("state ")) {
        int i = cmd.indexOf(' ', 6);
        String name = cmd.substring(6, i);
        i = cmd.indexOf(' ', i) + 1;
        String value = cmd.substring(i);
        
        LOG("serial", "Set state <%s> to <%s>\n", name.c_str(), value.c_str());
        state_set(name.c_str(), value.c_str());
      } else if(cmd == "calib1") {
        state_set("state", 0);
        state_set("fl-h", (float) PI/2);
        state_set("fl-k", (float) PI/2);
        state_set("fr-h", (float) PI/2);
        state_set("fr-k", (float) PI/2);
        state_set("bl-h", (float) PI/2);
        state_set("bl-k", (float) PI/2);
        state_set("br-h", (float) PI/2);
        state_set("br-k", (float) PI/2);
      } else if(cmd.startsWith("debug ")) {
        debug_feature = cmd.substring(6);
      } else {
        Serial.printf("Invalid command `%s`\n", cmd.c_str());
      }

    // reset
    cmd.clear();
    got_line = false;

    delay(1);
  }
};

SerialTask serialTask;

}

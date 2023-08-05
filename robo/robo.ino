#include <Scheduler.h>

namespace robo {
  String debug_feature;
};

#include "robo_log.hpp"
#include "robo_serial.hpp"
#include "robo_kinematics.hpp"
#include "robo_web.hpp"

void setup() {
  Serial.begin(115200);

  Serial.println("Hello!");
  LOG("main", "World!");

  Scheduler.start(&robo::serialTask);
  Scheduler.start(&robo::kinematicsTask);
  Scheduler.start(&robo::webTask);

  Scheduler.begin();
}

void loop() {
   /* Empty. Do everything using tasks, please! */
}

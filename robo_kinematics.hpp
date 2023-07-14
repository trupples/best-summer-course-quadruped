#pragma once

#include <LeanTask.h>

#include "robo_servo.hpp"
#include "robo_configuration.hpp"
#include "robo_state.hpp"
#include "robo_log.hpp"

namespace robo
{

#define DUTY_CYCLE_RESOLUTION 14 // 14 bits => 16384 levels per 20ms => 819 levels per 1ms = 90deg
#define PWM_LEVELS_PER_90DEG ((1 << DUTY_CYCLE_RESOLUTION) / 20)

const float l1 = 9.779; // femur length (axis to axis)
const float l2 = 8.89;  // tibia length (axis to middle of paw)

float feet_speed = 10; // cm / s

float mapf(const float x, const float fromLow, const float fromHigh, const float toLow, const float toHigh) {
  const float t = (x - fromLow) / (fromHigh - fromLow);
  return toLow + t * (toHigh - toLow);
}

struct Joint {
  String name;
  int servo_channel;
	int calibrate_0, calibrate_90; // duty cycle for 0 and 90 degrees
	float limit_low, limit_high;
	int pin; float angle;

  int written_duty = -1;

	Joint(String name, int pin) :
			name(name),
			pin(pin) {

    bool flip = (name == "bl-k") || (name == "fl-k") || (name == "br-h") || (name == "fr-h");
    bool knee = (name == "bl-k") || (name == "fl-k") || (name == "br-k") || (name == "fr-k");

    config(&calibrate_0,  name + "-c0",  flip ? PWM_LEVELS_PER_90DEG : 2*PWM_LEVELS_PER_90DEG);
    config(&calibrate_90, name + "-c90", flip ? 2*PWM_LEVELS_PER_90DEG : PWM_LEVELS_PER_90DEG);
    config(&limit_low,    name + "-ll",  0);
    config(&limit_high,   name + "-lh",  knee ? 5*PI/6 : PI/2); /* Knees will have an extra 60deg of movement */

    state(&angle, name);

    servo_channel = servo_attach(pin);
	}

	void write(float angle) {	
		if(angle < limit_low) angle = limit_low;
		if(angle > limit_high) angle = limit_high;
		this->angle = angle;

    int duty = mapf(angle, 0, PI / 2, calibrate_0, calibrate_90);
    if(written_duty != -1) {
      // Limit rotation
      const int max_degrees_per_second = 30;
      const int limit = abs((calibrate_90 - calibrate_0) * max_degrees_per_second / 90);
      if(duty > written_duty + limit) {
        duty = written_duty + limit;
      }
      if(duty < written_duty - limit) {
        duty = written_duty - limit;
      }
    }
    servo_write(servo_channel, duty);
    written_duty = duty;
	}
};

String to_string(const Joint &x) {
  return x.name + "\t" + ((int)(x.angle * 180 / PI)) + "\tC:" + (x.calibrate_0) + ":" + (x.calibrate_90) + "\tL:" + ((int)(x.limit_low * 180 / PI)) + ":" + ((int)(x.limit_high * 180 / PI));
}

struct Leg {
	String name;
	Joint hip, knee;
	float x, y;
  float target_x, target_y;

	Leg(String name, int hipPin, int kneePin) :
			name(name),
			hip(name + "-h", hipPin),
			knee(name + "-k", kneePin),
      x(l2), y(-l1),
      target_x(l2), target_y(-l1) {
    state(&x, name + "-x");
    state(&y, name + "-y");
    state(&target_x, name + "-tx");
    state(&target_y, name + "-ty");
  }

  void moveIK(float dt) {
    // Advance position towards target, limiting speed to `feet_speed`
    const float prev_x = x, prev_y = y; // To roll back in case the next position is invalid
    float dx = target_x - x, dy = target_y - y;
    const float remaining_distance = hypot(dx, dy);
    if(remaining_distance > feet_speed * dt) {
      dx *= feet_speed * dt / remaining_distance;
      dy *= feet_speed * dt / remaining_distance;
      //LOG("robo_kinematics", "Speed limit! d=%f dx=%f v*dt=%f", remaining_distance, dx, feet_speed * dt);
    }
    x += dx;
    y += dy;

    // Compute IK angles (Alpha, Beta)
    const float D = hypot(x, y); // Distance between hip and toe

		const float phi = atan2(y, x); // Angle from hip to toe

		const float cosGamma = (l1/D + D/l1 - l2*l2/l1/D) / 2;
		if(!(cosGamma > -1 && cosGamma < 1)) {
			LOG("robo_kinematics", "Impossible femur position");
      // roll back
      x = prev_x; y = prev_y;
			return;
		}
		const float gamma = -acos(cosGamma); // Angle between femur and line from hip to toe
		const float alpha = gamma + phi; // Absolute angle of femur
		const float Alpha = alpha + PI; // Angle of femur relative to femur zero (-X)

		const float kneeX = l1 * cos(alpha);
		const float kneeY = l1 * sin(alpha);

		const float beta = atan2(y - kneeY, x - kneeX); // Absolute angle of tibia
		const float Beta = beta - alpha; // Angle of tibia relative to "straight leg"


  /*// Old beta code, might need revisiting
    const float cosBeta = (l1/l2 + l2/l1 - D*D/l1/l2) / 2;
		if(!(cosBeta > -1 && cosBeta < 1)) {
			LOG("robo_kinematics", "Impossible tibia position");
			return false;
		}
		const float beta = acos(cosBeta); // Angle between femur and tibia (0 = closed)
		const float Beta = beta */

    // Set servos
		hip.write(Alpha);
		knee.write(Beta);

    if(debug_feature == "IK") {
      LOG("kinematics", "x,y = %f,%f => D = %f, phi = %f", x, y, D, phi);
      LOG("kinematics", "cos(gamma) = %f => gamma = %f", cosGamma, gamma);
      LOG("kinematics", "alpha = %f, Alpha = %f", alpha, Alpha);
      LOG("kinematics", "knee @ %f,%f", kneeX, kneeY);
      LOG("kinematics", "beta = %f, Beta = %f", beta, Beta);

      debug_feature.clear();
    }
  }

	void home() {
    this->target_x = 0;
    this->target_y = -5;
    moveIK(5);
		/*hip.write(PI/2);
		knee.write(PI/2);
		this->x = l2;
		this->y = -l1;*/
	}
};

String to_string(const Leg &x) {
  return x.name + "\t" + (x.target_x) + "," + (x.target_y) + "\t" + (x.x) + "," + (x.y);
}

struct Pose {
  float x = 0, y = 10;
  float pitch = 0, yaw = 0, roll = 0;
};

enum KinematicsState : int {
  MANUAL_FK = 0,
  MANUAL_IK = 1,
  IDLE = 2
};

class KinematicsTask : public LeanTask {
public:
  KinematicsTask() :
    LeanTask(true, 20), // Run kinematics loop at 50Hz <-> in sync with the servos
    leg_fl("fl", 5, 4),
    leg_fr("fr", 0, 2),
    leg_bl("bl", 13, 15),
    leg_br("br",  0, 16),
    legs { &leg_fl, &leg_fr, &leg_bl, &leg_br }
  {}

private:
  KinematicsState kinematics_state;
  Leg leg_fl;
  Leg leg_fr;
  Leg leg_bl;
  Leg leg_br;
  Leg *legs[4];

  Pose pose;

protected:
  void setup() {
    LOG("kinematics", "Hello!");
    state((int*)&kinematics_state, "state");

    state((float*)&pose.x, "pose-x");
    state((float*)&pose.y, "pose-y");
    
    for(int i = 0; i < 4; i++) {
      legs[i]->home(); LOG("kinematics", "%s", to_string(*legs[i]).c_str());
    }
  }

  void loop() {
    if(kinematics_state == MANUAL_FK) {
      for(int i = 0; i < 4; i++) {
        legs[i]->hip.write(legs[i]->hip.angle);
        legs[i]->knee.write(legs[i]->knee.angle);
      }

      return;
    }
    
    if(kinematics_state == IDLE) {
      for(int i = 0; i < 4; i++) {
        legs[i]->target_x = -pose.x;
        legs[i]->target_y = -pose.y;
      }
    }
    
    static int last = millis();
    int now = millis();
    for(int i = 0; i < 4; i++) {
      legs[i]->moveIK((now - last) * 0.001);
    }
    last = now;
  }
};

KinematicsTask kinematicsTask;

}

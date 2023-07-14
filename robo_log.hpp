#pragma once

namespace robo {

#define LOG(tag, format, ...) Serial.printf("%8lu [%s]: " format "\n", millis(), tag __VA_OPT__(,) __VA_ARGS__)

}

#pragma once

#include "globals.hpp"

#ifdef ESP_PLATFORM
#include "esphome.h"
#else
#include "../src-pc/mock_esphome.hpp"
#endif

class GameSettingsTestMode {
private:
  static constexpr int32_t TEST_TIME_MS = 5000;

public:
  AntGlobals &antg;
  GameSettingsTestMode(AntGlobals &antg) : antg(antg) {}

  bool active = false; // whether the user has entered ota settings

  char input = ' ';
  int32_t test_time_remaining = 0;

  void init() {
    active = true;
    test_time_remaining = TEST_TIME_MS;
  }

  void display_update(esphome::lcd_base::LCDDisplay &disp) {
    disp.printf(0, 0, "   TEST  MENU   ");
    if (input != ' ') {
      disp.printf(0, 1, "Key: %c", input);
    } else {
      disp.printf(0, 1, "");
    }
  }

  void handle_key(unsigned char key) {
    input = key;
    test_time_remaining = TEST_TIME_MS;
  }

  void clock(uint32_t now, uint32_t delta) {
    test_time_remaining -= delta;
    if (test_time_remaining <= 0) {
      active = false;
    }
  }
};

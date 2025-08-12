#pragma once

#include "globals.hpp"

#ifdef ESP_PLATFORM
#include "esphome.h"
#else
#include "../src-pc/mock_esphome.hpp"
#endif

class GameModeRespawnTimer {
private:
public:
  AntGlobals &antg;
  GameModeRespawnTimer(AntGlobals &antg) : antg(antg) {}

  void init() {}

  void display_update(esphome::lcd_base::LCDDisplay &disp) {
    disp.print(0, 0, "Soon TM");
    disp.print(0, 1, "");
  }

  void handle_key(unsigned char key) {}

  void clock(uint32_t now, uint32_t delta) {}
};

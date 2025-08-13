#pragma once

#include "globals.hpp"
#include "utilities.hpp"

#ifdef ESP_PLATFORM
#include "esphome.h"
#else
#include "../src-pc/mock_esphome.hpp"
#endif

class GameSettingsSiren {
private:
  enum class MENU { LEVEL, TEST, BACK, COUNT };
  MENU menu = MENU::LEVEL;

  int siren_test_delay_sec = 5; // 1-9
  int siren_test_delay_original = 5;
  uint8_t siren_level = 0;
  uint32_t siren_test_start_at = 0;

public:
  AntGlobals &antg;
  GameSettingsSiren(AntGlobals &antg) : antg(antg) {}

  bool active = false; // whether the user has entered siren settings

  void init() {
    menu = MENU::LEVEL;
    active = true;
    siren_level = antg.settings.siren_level_user;
  }

  void display_update(esphome::lcd_base::LCDDisplay &disp) {
    switch (menu) {
    case MENU::LEVEL:
      disp.printf(0, 0, "> Siren level: %d", siren_level);
      disp.printf(0, 1, "  Test (wait %ds)", siren_test_delay_sec);
      break;
    case MENU::TEST:
      disp.printf(0, 0, "  Siren level: %d", siren_level);
      disp.printf(0, 1, "> Test (wait %ds)", siren_test_delay_sec);
      break;
    case MENU::BACK:
      disp.printf(0, 0, "  Test (wait %ds)", siren_test_delay_sec);
      disp.printf(0, 1, "> Back");
      break;
    case MENU::COUNT: break;
    }
  }

  void handle_key(unsigned char key) {
    switch (key) {
    case KEY_A: menu = enum_prev(menu); break;
    case KEY_B: menu = enum_next(menu); break;
    case KEY_D:
      menu = MENU::LEVEL;
      active = false;
      break;
    case KEY_C:
      switch (menu) {
      case MENU::LEVEL: break;
      case MENU::TEST:
        // start siren test
        antg.action_siren(SIREN_DURATION_TEST, siren_test_delay_sec * 1000);
        siren_test_start_at = esphome::millis();
        siren_test_delay_original = siren_test_delay_sec;
        break;
      case MENU::BACK:
        menu = MENU::LEVEL;
        active = false;
        break;
      case MENU::COUNT: break;
      }
      break;
    case KEY_STAR:
      if (menu == MENU::TEST) {
        siren_test_delay_sec = 5;
      }
      break;
    case KEY_0:
    case KEY_1:
    case KEY_2:
    case KEY_3:
    case KEY_4:
    case KEY_5:
    case KEY_6:
    case KEY_7:
    case KEY_8:
    case KEY_9:
      switch (menu) {
      case MENU::LEVEL:
        if (key != KEY_0) {
          siren_level = append_digit(key, siren_level, 1);
          antg.action_set_siren_level(siren_level);
        }
        break;
      case MENU::TEST:
        if (key != KEY_0) {
          siren_test_delay_sec = append_digit(key, siren_test_delay_sec, 1);
        }
        break;
      case MENU::BACK:  break;
      case MENU::COUNT: break;
      }
      break;
    }
  }

  void clock(uint32_t now, uint32_t delta) {
    if (siren_test_start_at) {
      siren_test_delay_sec = siren_test_delay_original - (now - siren_test_start_at) / 1000;
      if (siren_test_delay_sec <= 0) {
        siren_test_delay_sec = siren_test_delay_original;
        siren_test_start_at = 0;
      }
    }
  }
};

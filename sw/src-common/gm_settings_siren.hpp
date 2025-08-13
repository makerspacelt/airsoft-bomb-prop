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
  enum class STATE { MENU, TEST_DELAY, TEST };
  STATE state = STATE::MENU;

  enum class MENU { LEVEL, TEST, BACK, COUNT };
  MENU menu = MENU::LEVEL;

  int siren_test_delay_sec = 5; // 1-9
  uint8_t siren_level = 0;
  uint32_t siren_test_start_at = 0;
  uint32_t siren_test_end_at = 0;

  std::string fmt_siren_level(uint8_t level) {
    switch (level) {
    case 1:  return "LOW";
    case 2:  return "MEDIUM";
    default: return "HIGH";
    }
  }

  void reset_test() {
    state = STATE::MENU;
    siren_test_start_at = 0;
    siren_test_end_at = 0;
  }

  void disp_test_line(esphome::lcd_base::LCDDisplay &disp, uint8_t row, bool selected) {
    std::string line = selected ? "> " : "  ";
    switch (state) {
    case STATE::MENU:
      line += "Test (wait %ds)";
      disp.printf(0, row, line.c_str(), siren_test_delay_sec);
      break;
    case STATE::TEST_DELAY: {
      float delay_sec = (siren_test_start_at - esphome::millis()) / 1000.0;
      line += "Test in %.1fs";
      disp.printf(0, row, line.c_str(), delay_sec);
      break;
    }
    case STATE::TEST:
      line += "SIREN ACTIVE";
      disp.printf(0, row, line.c_str(), SIREN_DURATION_TEST / 1000);
      break;
    }
  }

public:
  AntGlobals &antg;
  GameSettingsSiren(AntGlobals &antg) : antg(antg) {}

  bool active = false; // whether the user has entered siren settings

  void init() {
    menu = MENU::LEVEL;
    active = true;
    siren_level = antg.settings.siren_level_user;
    reset_test();
  }

  void display_update(esphome::lcd_base::LCDDisplay &disp) {
    switch (menu) {
    case MENU::LEVEL:
      disp.printf(0, 0, "> Level: %s", fmt_siren_level(siren_level).c_str());
      disp_test_line(disp, 1, false);
      break;
    case MENU::TEST:
      disp.printf(0, 0, "  Level: %s", fmt_siren_level(siren_level).c_str());
      disp_test_line(disp, 1, true);
      break;
    case MENU::BACK:
      disp_test_line(disp, 0, false);
      disp.printf(0, 1, "> Back");
      break;
    case MENU::COUNT: break;
    }
  }

  void handle_key(unsigned char key) {
    if (state == STATE::TEST_DELAY) { // any key press stops the active siren test delay
      reset_test();
      return;
    } else if (state == STATE::TEST) {
      // cannot press anything while siren is active
      return;
    }

    switch (key) {
    case KEY_A: menu = enum_prev(menu); break;
    case KEY_B: menu = enum_next(menu); break;
    case KEY_D:
      menu = MENU::LEVEL;
      active = false;
      break;
    case KEY_C:
      switch (menu) {
      case MENU::LEVEL:
        siren_level++;
        if (siren_level > 3) {
          siren_level = 1;
        }
        antg.action_set_siren_level(siren_level);
        break;
      case MENU::TEST:
        siren_test_start_at = esphome::millis() + siren_test_delay_sec * 1000;
        siren_test_end_at = siren_test_start_at + SIREN_DURATION_TEST;
        state = STATE::TEST_DELAY;
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
        if (key >= KEY_1 && key <= KEY_3) {
          siren_level = key - KEY_0;
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
    switch (state) {
    case STATE::MENU: break;
    case STATE::TEST_DELAY:
      if (now >= siren_test_start_at) {
        state = STATE::TEST;
        antg.action_siren(SIREN_DURATION_TEST);
      }
      break;
    case STATE::TEST:
      if (now >= siren_test_end_at) {
        reset_test();
      }
      break;
    }
  }
};

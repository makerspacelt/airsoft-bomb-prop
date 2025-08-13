#pragma once

#include "globals.hpp"
#include "utilities.hpp"

#ifdef ESP_PLATFORM
#include "esphome.h"
#else
#include "../src-pc/mock_esphome.hpp"
#endif

class GameModeRespawnTimer {
private:
  static constexpr uint16_t STANDBY_MAX_LEN = 3;
  static constexpr uint8_t RESPAWN_MAX_LEN = 2;

  enum class STATE { SETUP, INVALID_INPUT_STANDBY, INVALID_INPUT_RESPAWN, GAME_STANDBY, GAME_RESPAWN };
  enum class MENU { STANDBY_MIN, RESPAWN_SEC, USE_SIREN, START, BACK, COUNT };

  STATE state = STATE::SETUP;
  MENU menu = MENU::STANDBY_MIN;

  bool use_siren = false;
  uint16_t standby_min = 0;
  uint8_t respawn_sec = 0;

  int standby_time_remaining = 0;
  int go_time_remaining = 0;

  void start_game_standby() {
    state = STATE::GAME_STANDBY;
    standby_time_remaining = standby_min * 60 * 1000;
  }

  void start_game_respawn() {
    state = STATE::GAME_RESPAWN;
    go_time_remaining = respawn_sec * 1000;
    if (use_siren) {
      antg.action_siren(go_time_remaining);
    } else {
      antg.action_buzzer(BUZZER_TONE, go_time_remaining);
    }
  }

  // === SETUP STATE ===
  void display_setup(esphome::lcd_base::LCDDisplay &disp) {
    switch (menu) {
    case MENU::STANDBY_MIN:
      disp.printf(0, 0, ">Standby min %d", standby_min);
      disp.printf(0, 1, " Respawn sec %d", respawn_sec);
      break;
    case MENU::RESPAWN_SEC:
      disp.printf(0, 0, " Standby min %d", standby_min);
      disp.printf(0, 1, ">Respawn sec %d", respawn_sec);
      break;
    case MENU::USE_SIREN:
      disp.printf(0, 0, " Respawn sec %d", respawn_sec);
      disp.printf(0, 1, ">Use siren?: %s", use_siren ? "Y" : "N");
      break;
    case MENU::START:
      disp.printf(0, 0, " Use siren?: %s", use_siren ? "Y" : "N");
      disp.printf(0, 1, ">START");
      break;
    case MENU::BACK:
      disp.printf(0, 0, " START");
      disp.printf(0, 1, ">Back");
      break;
    case MENU::COUNT: break;
    }
  }

  void handle_key_setup(unsigned char key) {
    switch (key) {
    case KEY_A: menu = enum_prev(menu); break;
    case KEY_B: menu = enum_next(menu); break;
    case KEY_C:
      switch (menu) {
      case MENU::START:
        if (!standby_min) {
          state = STATE::INVALID_INPUT_STANDBY;
        } else if (!respawn_sec) {
          state = STATE::INVALID_INPUT_RESPAWN;
        } else {
          start_game_standby();
        }
        break;
      case MENU::USE_SIREN: //
        use_siren = !use_siren;
        break;
      case MENU::BACK: //
        antg.action_exit_game();
        break;
      default: //
        break;
      }
      break;
    case KEY_D: antg.action_exit_game(); break;
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
      case MENU::STANDBY_MIN: standby_min = append_digit(key, standby_min, STANDBY_MAX_LEN); break;
      case MENU::RESPAWN_SEC: respawn_sec = append_digit(key, respawn_sec, RESPAWN_MAX_LEN); break;
      default:                break;
      }
      break;
    case KEY_STAR:
      switch (menu) {
      case MENU::STANDBY_MIN: standby_min = 0; break;
      case MENU::RESPAWN_SEC: respawn_sec = 0; break;
      default:                break;
      }
      break;
    }
  }

  // === INVALID INPUT STATE ===
  void display_invalid_input_standby(esphome::lcd_base::LCDDisplay &disp) {
    disp.printf(0, 0, "*INVALID INPUT*");
    disp.printf(0, 1, "*STANDBY TIME*");
  }

  void display_invalid_input_respawn(esphome::lcd_base::LCDDisplay &disp) {
    disp.printf(0, 0, "*INVALID INPUT*");
    disp.printf(0, 1, "*RESPAWN TIME*");
  }

  void handle_key_invalid_input(unsigned char key) { state = STATE::SETUP; }

  // === GAME STANDBY STATE ===
  void display_game_standby(esphome::lcd_base::LCDDisplay &disp) {
    disp.printf(0, 0, "    STAND BY    ");
    disp.printf(0, 1, "      %s", format_time_remaining(standby_time_remaining).c_str());
  }

  void handle_key_game_standby(unsigned char key) {
    switch (key) {
    case KEY_RED:
    case KEY_YELLOW: start_game_respawn(); break;
    }
  }

  void clock_game_standby(uint32_t now, uint32_t delta) {
    standby_time_remaining -= delta;
    if (standby_time_remaining <= 0) {
      start_game_respawn();
    }
  }

  // === GAME RESPAWN STATE ===
  void display_game_respawn(esphome::lcd_base::LCDDisplay &disp) {
    disp.printf(0, 0, "   GO! GO! GO!  ");
    disp.printf(0, 1, "      %s", format_time_remaining(go_time_remaining).c_str());
  }

  void handle_key_game_respawn(unsigned char key) {}

  void clock_game_respawn(uint32_t now, uint32_t delta) {
    go_time_remaining -= delta;
    if (go_time_remaining <= 0) {
      start_game_standby();
    }
  }

public:
  AntGlobals &antg;
  GameModeRespawnTimer(AntGlobals &antg) : antg(antg) {}

  void init() {
    state = STATE::SETUP;
    menu = MENU::STANDBY_MIN;

    standby_time_remaining = 0;
    go_time_remaining = 0;
  }

  void display_update(esphome::lcd_base::LCDDisplay &disp) {
    switch (state) {
    case STATE::SETUP:                 display_setup(disp); break;
    case STATE::INVALID_INPUT_STANDBY: display_invalid_input_standby(disp); break;
    case STATE::INVALID_INPUT_RESPAWN: display_invalid_input_respawn(disp); break;
    case STATE::GAME_STANDBY:          display_game_standby(disp); break;
    case STATE::GAME_RESPAWN:          display_game_respawn(disp); break;
    }
  }

  void handle_key(unsigned char key) {
    switch (state) {
    case STATE::SETUP:                 handle_key_setup(key); break;
    case STATE::INVALID_INPUT_STANDBY:
    case STATE::INVALID_INPUT_RESPAWN: handle_key_invalid_input(key); break;
    case STATE::GAME_STANDBY:          handle_key_game_standby(key); break;
    case STATE::GAME_RESPAWN:          handle_key_game_respawn(key); break;
    }
  }

  void clock(uint32_t now, uint32_t delta) {
    switch (state) {
    case STATE::GAME_STANDBY: clock_game_standby(now, delta); break;
    case STATE::GAME_RESPAWN: clock_game_respawn(now, delta); break;
    default:                  break;
    }
  }
};

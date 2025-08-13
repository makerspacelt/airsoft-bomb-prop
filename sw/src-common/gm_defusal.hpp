#pragma once

#include "globals.hpp"
#include "utilities.hpp"
#include "gm_defusal_buttons.hpp"
#include "gm_defusal_code.hpp"

#ifdef ESP_PLATFORM
#include "esphome.h"
#else
#include "../src-pc/mock_esphome.hpp"
#endif

class GameModeDefusal {
  // Handles game setup & pre-start, then hands game control to:
  // * GameModeDefusalCode, if bomb code was entered
  // * GameModeDefusalButtons, if no bomb code was entered
private:
  static constexpr uint32_t MINUTES_MAX_LEN = 3;

  enum class STATE { SETUP, INVALID_INPUT, PRE_START, DEFUSAL_CODE, DEFUSAL_BUTTONS };
  enum class MENU { DELAY_MIN, BOMB_MIN, BOMB_CODE, START, BACK, COUNT };

  STATE state = STATE::SETUP;
  MENU menu = MENU::DELAY_MIN;

  AntGlobals &antg;
  GameModeDefusalCode gm_defusal_code;
  GameModeDefusalButtons gm_defusal_buttons;

  int delay_min = 0;
  int bomb_min = 0;
  int32_t delay_ms_remaining = 0;
  int32_t bomb_ms_total = 0;
  uint32_t last_bomb_buzzer_at = 0;
  std::string bomb_code = "";

  // === SETUP STATE ===
  void display_setup_menu(esphome::lcd_base::LCDDisplay &disp) {
    if (menu == MENU::DELAY_MIN) {
      disp.printf(0, 0, "> Delay min: %d", delay_min);
      disp.printf(0, 1, "  Bomb  min: %d", bomb_min);
    } else if (menu == MENU::BOMB_MIN) {
      disp.printf(0, 0, "  Delay min: %d", delay_min);
      disp.printf(0, 1, "> Bomb  min: %d", bomb_min);
    } else if (menu == MENU::BOMB_CODE) {
      disp.printf(0, 0, "  Bomb  min: %d", bomb_min);
      disp.printf(0, 1, "> Code: %s", bomb_code.c_str());
    } else if (menu == MENU::START) {
      disp.printf(0, 0, "  Code: %s", bomb_code.c_str());
      disp.printf(0, 1, "> START");
    } else if (menu == MENU::BACK) {
      disp.printf(0, 0, "  START");
      disp.printf(0, 1, "> Back");
    }
  }

  void handle_key_setup(unsigned char key) {
    if (key >= KEY_0 && key <= KEY_9) {
      switch (menu) {
      case MENU::DELAY_MIN: delay_min = append_digit(key, delay_min, MINUTES_MAX_LEN); break;
      case MENU::BOMB_MIN:  bomb_min = append_digit(key, bomb_min, MINUTES_MAX_LEN); break;
      case MENU::BOMB_CODE:
        if (bomb_code.length() >= GM_DEFUSAL_MAX_CODE_LEN) {
          bomb_code = "";
        }
        bomb_code += key;
        break;
      default: break;
      }
    }

    switch (key) {
    case KEY_A: menu = enum_prev(menu); break;
    case KEY_B: menu = enum_next(menu); break;
    case KEY_STAR:
      switch (menu) {
      case MENU::DELAY_MIN: delay_min = 0; break;
      case MENU::BOMB_MIN:  bomb_min = 0; break;
      case MENU::BOMB_CODE: bomb_code = ""; break;
      case MENU::BACK:      break;
      case MENU::START:     break;
      case MENU::COUNT:     break;
      }
      break;
    case KEY_C:
      if (menu == MENU::START) {
        if (!bomb_min) {
          state = STATE::INVALID_INPUT;
        } else {
          ESP_LOGI("GM_defusal", "Starting the game");
          start_game();
        }
      } else if (menu == MENU::BACK) {
        antg.action_exit_game();
      }
      break;
    case KEY_D: antg.action_exit_game(); break;
    }
  }

  // === INVALID_INPUT STATE ===
  void display_invalid_input(esphome::lcd_base::LCDDisplay &disp) {
    disp.printf(0, 0, "*INVALID INPUT*");
    disp.printf(0, 1, " * BOMB TIME * ");
  }

  void handle_key_invalid_input() {
    ESP_LOGI("GM_defusal_buttons", "INVALID INPUT -> SETUP");
    state = STATE::SETUP;
  }

  // === PRE_START STATE ===
  void display_pre_start(esphome::lcd_base::LCDDisplay &disp) {
    disp.printf(0, 0, " PREP FOR GAME");
    disp.printf(0, 1, "     %s", format_time_remaining(delay_ms_remaining).c_str());
  }

  void clock_pre_start(uint32_t now, uint32_t delta) {
    delay_ms_remaining -= delta;
    if (delay_ms_remaining <= 0) {
      antg.action_siren(SIREN_DURATION_GAME_START);
      start_subgame();
    }
  }

  // === SUB-GAME FINISHED STATE ===

  void handle_key_finished(unsigned char key) {
    switch (key) {
    case KEY_C_LONG:
      antg.action_buzzer(BUZZER_TONE_SPECIAL, BUZZER_DURATION_SPECIAL);
      ESP_LOGI("GM_defusal", "Restarting the game");
      start_game();
      break;
    }
  }

  // === Common ===
  void start_game() {
    delay_ms_remaining = delay_min * 60 * 1000;
    if (delay_ms_remaining > 0) {
      ESP_LOGI("GM_defusal_buttons", "-> PRE START");
      state = STATE::PRE_START;
    } else {
      start_subgame();
    }
  }

  void start_subgame() {
    last_bomb_buzzer_at = 0;
    bomb_ms_total = bomb_min * 60 * 1000;
    if (bomb_code.empty()) {
      ESP_LOGI("GM_defusal_buttons", "-> DEFUSAL (BUTTONS)");
      state = STATE::DEFUSAL_BUTTONS;
      gm_defusal_buttons.start_game(bomb_ms_total);
    } else {
      ESP_LOGI("GM_defusal_buttons", "-> DEFUSAL (CODE)");
      state = STATE::DEFUSAL_CODE;
      gm_defusal_code.start_game(bomb_code, bomb_ms_total);
    }
  }

  void bomb_buzzer(int8_t percent, uint32_t now) {
    if (percent < 0)
      percent = 0;
    uint32_t period = 10000;
    if (percent <= 10)
      period = 200;
    else if (percent <= 20)
      period = 1000;
    else if (percent <= 40)
      period = 3000;
    else if (percent <= 60)
      period = 5000;
    if (now - last_bomb_buzzer_at >= period) {
      antg.action_buzzer(BUZZER_TONE_BOMB);
      last_bomb_buzzer_at = now;
    }
  }

public:
  GameModeDefusal(AntGlobals &antg) : antg(antg), gm_defusal_code(antg), gm_defusal_buttons(antg) {}

  void init() {
    state = STATE::SETUP;
    menu = MENU::DELAY_MIN;
  }

  void display_update(esphome::lcd_base::LCDDisplay &disp) {
    switch (state) {
    case STATE::INVALID_INPUT:   display_invalid_input(disp); break;
    case STATE::SETUP:           display_setup_menu(disp); break;
    case STATE::PRE_START:       display_pre_start(disp); break;
    case STATE::DEFUSAL_CODE:    gm_defusal_code.display_update(disp); break;
    case STATE::DEFUSAL_BUTTONS: gm_defusal_buttons.display_update(disp); break;
    }
  }

  void handle_key(unsigned char key) {
    switch (state) {
    case STATE::SETUP:         handle_key_setup(key); break;
    case STATE::INVALID_INPUT: handle_key_invalid_input(); break;
    case STATE::PRE_START:     break;
    case STATE::DEFUSAL_CODE:
      gm_defusal_code.handle_key(key);
      if (gm_defusal_code.finished) {
        handle_key_finished(key);
      }
      break;
    case STATE::DEFUSAL_BUTTONS:
      gm_defusal_buttons.handle_key(key);
      if (gm_defusal_buttons.finished) {
        handle_key_finished(key);
      }
      break;
    }
  }

  void clock(uint32_t now, uint32_t delta) {
    switch (state) {
    case STATE::SETUP:         break;
    case STATE::INVALID_INPUT: break;
    case STATE::PRE_START:     clock_pre_start(now, delta); break;
    case STATE::DEFUSAL_CODE:
      gm_defusal_code.clock(now, delta);
      if (gm_defusal_code.armed && !gm_defusal_code.finished) {
        bomb_buzzer(100 * (float)gm_defusal_code.bomb_ms_remaining / bomb_ms_total, now);
      }
      break;
    case STATE::DEFUSAL_BUTTONS:
      gm_defusal_buttons.clock(now, delta);
      if (gm_defusal_code.armed && !gm_defusal_buttons.finished) {
        bomb_buzzer(100 * (float)gm_defusal_buttons.bomb_ms_remaining / bomb_ms_total, now);
      }
      break;
    }
  }
};

#pragma once

#include "globals.hpp"
#include "utilities.hpp"

#ifdef ESP_PLATFORM
#include "esphome.h"
#else
#include "../pc-build/mock_esphome.hpp"
#endif

class GameModeDomination {
private:
  static constexpr uint32_t CAPTURE_TIME = 5000;
  static constexpr uint8_t MINUTES_MAX_LEN = 3;

  enum class STATE { SETUP, INVALID_INPUT, PRE_START, RUNNING, FINISHED };
  enum class MENU { DELAY_MIN, GAME_MIN, START, COUNT };

  STATE state = STATE::SETUP;
  MENU menu = MENU::DELAY_MIN;

  int delay_min = 0;
  int game_min = 0;

  int delay_ms_remaining = 0;
  int game_ms_remaining = 0;

  int8_t team_active = 0; // 0=no team, 1=red, 2=yellow
  uint32_t team_red_time = 0;
  uint32_t team_yellow_time = 0;
  uint32_t capture_start = 0;

  // === SETUP STATE ===
  void display_setup_menu(esphome::lcd_base::LCDDisplay &disp) {
    if (menu == MENU::DELAY_MIN) {
      disp.printf(0, 0, "> Delay min: %d", delay_min);
      disp.printf(0, 1, "  Game  min: %d", game_min);
    } else if (menu == MENU::GAME_MIN) {
      disp.printf(0, 0, "  Delay min: %d", delay_min);
      disp.printf(0, 1, "> Game  min: %d", game_min);
    } else if (menu == MENU::START) {
      disp.printf(0, 0, "  Game  min: %d", game_min);
      disp.printf(0, 1, "> START");
    }
  }

  void handle_key_setup(unsigned char key) {
    switch (key) {
    case KEY_A: menu = enum_prev(menu); break;
    case KEY_B: menu = enum_next(menu); break;
    case KEY_STAR:
      if (menu == MENU::DELAY_MIN) {
        delay_min = 0;
      } else if (menu == MENU::GAME_MIN) {
        game_min = 0;
      }
      break;
    case KEY_C:
      if (menu == MENU::START) {
        if (!game_min) {
          state = STATE::INVALID_INPUT;
        } else {
          ESP_LOGI("GM_domination", "Starting the game");
          start();
        }
      }
      break;
    case KEY_D:
      // Go back to main menu
      antg.action_exit_game();
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
      case MENU::DELAY_MIN: delay_min = append_digit(key, delay_min, MINUTES_MAX_LEN); break;
      case MENU::GAME_MIN:  game_min = append_digit(key, game_min, MINUTES_MAX_LEN); break;
      default:              break;
      }
      break;
    }
  }

  // === INVALID_INPUT STATE ===
  void display_invalid_input(esphome::lcd_base::LCDDisplay &disp) {
    disp.printf(0, 0, "*INVALID INPUT*");
    disp.printf(0, 1, " * GAME TIME * ");
  }

  void handle_key_invalid_input() { state = STATE::SETUP; }

  // === PRE_START STATE ===
  void display_pre_start(esphome::lcd_base::LCDDisplay &disp) {
    disp.printf(0, 0, " PREP FOR GAME");
    disp.printf(0, 1, "     %s", format_time_remaining(delay_ms_remaining).c_str());
  }

  void clock_pre_start(uint32_t now, uint32_t delta) {
    delay_ms_remaining -= delta;
    if (delay_ms_remaining <= 0) {
      state = STATE::RUNNING;
      antg.action_siren(SIREN_DURATION_GAME_START);
    }
  }

  // === RUNNING STATE ===
  void display_running(esphome::lcd_base::LCDDisplay &disp) {
    if (capture_start) {
      float ratio = (esphome::millis() - capture_start) / (float)CAPTURE_TIME;
      disp.printf(0, 0, "TIME LEFT:% 6s", format_time_remaining(game_ms_remaining).c_str());
      disp.printf(0, 1, "%s", format_progress_bar(ratio).c_str());
    } else {
      disp.printf(0, 0, "TIME LEFT:% 6s", format_time_remaining(game_ms_remaining).c_str());
      disp.printf(0, 1, "T1:%-4d  T2:%-4d", team_red_time / 1000, team_yellow_time / 1000);
    }
  }

  void handle_key_running(unsigned char key) {
    if (!capture_start) {
      if (key == KEY_RED && team_active != 1)
        capture_start = esphome::millis();
      else if (key == KEY_YELLOW && team_active != 2)
        capture_start = esphome::millis();
    }
  }

  void clock_running(uint32_t now, uint32_t delta) {
    game_ms_remaining -= delta;
    if (game_ms_remaining <= 0) {
      state = STATE::FINISHED;
      antg.action_siren(SIREN_DURATION_GAME_END);
      return;
    }

    switch (team_active) {
    case 1: team_red_time += delta; break;
    case 2: team_yellow_time += delta; break;
    }

    if (capture_start) {
      if (antg.btn_red_duration >= CAPTURE_TIME) {
        capture_start = 0;
        team_active = 1;
        antg.action_buzzer(BUZZER_TONE, BUZZER_DURATION_TEAM_SWITCH);
      } else if (antg.btn_yellow_duration >= CAPTURE_TIME) {
        capture_start = 0;
        team_active = 2;
        antg.action_buzzer(BUZZER_TONE, BUZZER_DURATION_TEAM_SWITCH);
      } else if (!antg.btn_red_pressed && !antg.btn_yellow_pressed) {
        capture_start = 0;
      }
    }
  }

  // === FINISHED STATE ===
  void display_finished(esphome::lcd_base::LCDDisplay &disp) {
    disp.printf(0, 0, "DOMINATION ENDED");
    disp.printf(0, 1, "T1:%-4d  T2:%-4d", team_red_time / 1000, team_yellow_time / 1000);
  }

  void handle_key_finished(unsigned char key) {
    if (key == KEY_C_LONG) {
      ESP_LOGI("GM_domination", "Restarting the game");
      start();
    }
  }

  // === Common ===
  void start() {
    delay_ms_remaining = delay_min * 60 * 1000;
    game_ms_remaining = game_min * 60 * 1000;
    if (delay_ms_remaining > 0)
      state = STATE::PRE_START;
    else
      state = STATE::RUNNING;
  }

public:
  AntGlobals &antg;
  GameModeDomination(AntGlobals &antg) : antg(antg) {}

  void init() {
    state = STATE::SETUP;
    menu = MENU::DELAY_MIN;
    team_red_time = 0;
    team_yellow_time = 0;
  }

  void display_update(esphome::lcd_base::LCDDisplay &disp) {
    switch (state) {
    case STATE::INVALID_INPUT: display_invalid_input(disp); break;
    case STATE::SETUP:         display_setup_menu(disp); break;
    case STATE::PRE_START:     display_pre_start(disp); break;
    case STATE::RUNNING:       display_running(disp); break;
    case STATE::FINISHED:      display_finished(disp); break;
    }
  }

  void handle_key(unsigned char key) {
    switch (state) {
    case STATE::PRE_START:     break;
    case STATE::SETUP:         handle_key_setup(key); break;
    case STATE::INVALID_INPUT: handle_key_invalid_input(); break;
    case STATE::RUNNING:       handle_key_running(key); break;
    case STATE::FINISHED:      handle_key_finished(key); break;
    }
  }

  void clock(uint32_t now, uint32_t delta) {
    switch (state) {
    case STATE::PRE_START:     clock_pre_start(now, delta); break;
    case STATE::SETUP:         break;
    case STATE::INVALID_INPUT: break;
    case STATE::RUNNING:       clock_running(now, delta); break;
    case STATE::FINISHED:      break;
    }
  }
};

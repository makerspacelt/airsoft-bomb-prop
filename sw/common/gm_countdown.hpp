#pragma once

#include "globals.hpp"
#include "utilities.hpp"

#ifdef ESP_PLATFORM
#include "esphome.h"
#else
#include "../pc-build/mock_esphome.hpp"
#endif

class GameModeCountdown {
private:
  static constexpr uint8_t MINUTES_MAX_LEN = 3;

  enum class STATE { SETUP, INVALID_INPUT, PRE_START, RUNNING, FINISHED };
  enum class MENU { DELAY_MIN, GAME_MIN, START, COUNT };

  STATE state = STATE::SETUP;
  MENU menu = MENU::DELAY_MIN;

  int delay_min = 0;
  int game_min = 0;

  int delay_ms_remaining = 0;
  int game_ms_remaining = 0;

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
      if (menu == MENU::DELAY_MIN)
        delay_min = 0;
      else if (menu == MENU::GAME_MIN)
        game_min = 0;
      break;
    case KEY_C:
      if (menu == MENU::START) {
        if (!game_min) {
          state = STATE::INVALID_INPUT;
        } else {
          ESP_LOGI("GM_countdown", "Starting the game");
          start();
        }
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
  void display_started(esphome::lcd_base::LCDDisplay &disp) {
    disp.printf(0, 0, "  GAME STARTED");
    disp.printf(0, 1, "     %s", format_time_remaining(game_ms_remaining).c_str());
  }

  void clock_started(uint32_t now, uint32_t delta) {
    game_ms_remaining -= delta;
    if (game_ms_remaining <= 0) {
      state = STATE::FINISHED;
      antg.action_siren(SIREN_DURATION_GAME_END);
    }
  }

  // === FINISHED STATE ===
  void display_finished(esphome::lcd_base::LCDDisplay &disp) {
    disp.printf(0, 0, "   GAME ENDED");
    disp.printf(0, 1, "");
  }

  void handle_key_finished(unsigned char key) {
    if (key == KEY_C_LONG) {
      ESP_LOGI("GM_countdown", "Restarting the game");
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
  GameModeCountdown(AntGlobals &antg) : antg(antg) {}

  void init() {
    state = STATE::SETUP;
    menu = MENU::DELAY_MIN;
  }

  void display_update(esphome::lcd_base::LCDDisplay &disp) {
    switch (state) {
    case STATE::SETUP:         display_setup_menu(disp); break;
    case STATE::INVALID_INPUT: display_invalid_input(disp); break;
    case STATE::PRE_START:     display_pre_start(disp); break;
    case STATE::RUNNING:       display_started(disp); break;
    case STATE::FINISHED:      display_finished(disp); break;
    }
  }

  void handle_key(unsigned char key) {
    switch (state) {
    case STATE::SETUP:         handle_key_setup(key); break;
    case STATE::INVALID_INPUT: handle_key_invalid_input(); break;
    case STATE::PRE_START:     break;
    case STATE::RUNNING:       break;
    case STATE::FINISHED:      handle_key_finished(key); break;
    }
  }

  void clock(uint32_t now, uint32_t delta) {
    switch (state) {
    case STATE::SETUP:         break;
    case STATE::INVALID_INPUT: break;
    case STATE::PRE_START:     clock_pre_start(now, delta); break;
    case STATE::RUNNING:       clock_started(now, delta); break;
    case STATE::FINISHED:      break;
    }
  }
};

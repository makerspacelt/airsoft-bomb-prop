#pragma once

#include "globals.hpp"
#include "utilities.hpp"

#ifdef ESP_PLATFORM
#include "esphome.h"
#else
#include "../src-pc/mock_esphome.hpp"
#endif

class GameModeDefusalCode {
private:
  enum class STATE { ARM, BAD_CODE, ARMED, BAD_CODE_ARMED, DISARMED, EXPLODED };

  // How long to display the bad code warning
  static constexpr uint32_t BAD_CODE_DISPLAY_MS = 1000;

  STATE state = STATE::ARM;
  std::string bomb_code = "";
  std::string bomb_code_user = "";
  int failed_code_count = 0;
  int32_t bad_code_ms_remaining = 0;

  void disp_time_left(esphome::lcd_base::LCDDisplay &disp, int32_t bomb_ms_remaining) {
    disp.printf(0, 1, "TIME LEFT:% 6s", format_time_remaining(bomb_ms_remaining).c_str());
  }

  // === ARM STATE ===
  void display_arm(esphome::lcd_base::LCDDisplay &disp) {
    disp.printf(0, 0, "ARM CODE: %s", bomb_code_user.c_str());
    disp_time_left(disp, bomb_ms_remaining);
  }

  void handle_key_arm(unsigned char key) {
    if (key >= KEY_0 && key <= KEY_9) {
      // append bomb code digit
      if (bomb_code_user.length() >= GM_DEFUSAL_MAX_CODE_LEN) {
        bomb_code_user = "";
      }
      bomb_code_user += key;
    } else {
      switch (key) {
      case KEY_STAR:
        // clear entry
        bomb_code_user = "";
        break;
      case KEY_HASH:
        // confirm code
        if (bomb_code == bomb_code_user) {
          armed = true;
          bomb_code_user = "";
          ESP_LOGI("GM_defusal_code", "ARM -> ARMED");
          state = STATE::ARMED;
        } else {
          bomb_code_user = "";
          bad_code_ms_remaining = BAD_CODE_DISPLAY_MS;
          ESP_LOGI("GM_defusal_code", "ARM -> BAD CODE");
          state = STATE::BAD_CODE;
        }
        break;
      }
    }
  }

  // === BAD CODE STATE ===
  void display_bad_code(esphome::lcd_base::LCDDisplay &disp) {
    disp.printf(0, 0, "    BAD CODE    ");
    disp_time_left(disp, bomb_ms_remaining);
  }

  void clock_bad_code(uint32_t now, uint32_t delta) {
    bad_code_ms_remaining -= delta;
    if (bad_code_ms_remaining <= 0) {
      ESP_LOGI("GM_defusal_code", "BAD_CODE -> ARM");
      state = STATE::ARM;
    }
  }

  void clock_bad_code_armed(uint32_t now, uint32_t delta) {
    bad_code_ms_remaining -= delta;
    if (bad_code_ms_remaining <= 0) {
      ESP_LOGI("GM_defusal_code", "BAD_CODE_ARMED -> ARMED");
      state = STATE::ARMED;
    }
  }

  // === ARMED STATE ===
  void display_armed(esphome::lcd_base::LCDDisplay &disp) {
    disp.printf(0, 0, "ARMED: %s", bomb_code_user.c_str());
    disp_time_left(disp, bomb_ms_remaining);
  }

  void handle_key_armed(unsigned char key) {
    if (key >= KEY_0 && key <= KEY_9) {
      // append bomb code digit
      if (bomb_code_user.length() >= GM_DEFUSAL_MAX_CODE_LEN) {
        bomb_code_user = "";
      }
      bomb_code_user += key;
    } else {
      switch (key) {
      case KEY_STAR:
        // clear entry
        bomb_code_user = "";
        break;
      case KEY_HASH:
        // confirm code
        if (bomb_code == bomb_code_user) {
          bomb_code_user = "";
          ESP_LOGI("GM_defusal_code", "ARMED -> DISARMED");
          state = STATE::DISARMED;
          armed = false;
          finished = true;
          antg.action_siren(SIREN_DURATION_GAME_END, SIREN_GAME_END_DELAY);
        } else {
          bomb_code_user = "";
          bad_code_ms_remaining = BAD_CODE_DISPLAY_MS;
          ESP_LOGI("GM_defusal_code", "ARMED -> ARMED");
          state = STATE::BAD_CODE_ARMED;
          failed_code_count++;
          switch (failed_code_count) {
          case 1: bomb_ms_remaining /= 2; break;
          case 2:
            if (bomb_ms_remaining > 15000) {
              bomb_ms_remaining = 15000;
            }
            break;
          default:
            // third failed attempt = bomb explodes
            ESP_LOGI("GM_defusal_code", "ARMED -> EXPLODED");
            state = STATE::EXPLODED;
            armed = false;
            finished = true;
            antg.action_siren(SIREN_DURATION_GAME_END, SIREN_GAME_END_DELAY);
            break;
          }
        }
        break;
      }
    }
  }

  // === DISARMED STATE ===

  void display_disarmed(esphome::lcd_base::LCDDisplay &disp) {
    disp.printf(0, 0, "    DISARMED    ");
    disp_time_left(disp, bomb_ms_remaining);
  }

  // === EXPLODED STATE ===

  void display_exploded(esphome::lcd_base::LCDDisplay &disp) {
    disp.printf(0, 0, "    EXPLODED    ");
    disp_time_left(disp, bomb_ms_remaining);
  }

public:
  AntGlobals &antg;
  GameModeDefusalCode(AntGlobals &antg) : antg(antg) {}

  bool armed = false;
  bool finished = false;

  int32_t bomb_ms_remaining = 0;

  void start_game(std::string code, int32_t bomb_time_ms) {
    armed = false;
    finished = false;
    ESP_LOGI("GM_defusal_code", "START");
    state = STATE::ARM;
    bomb_code = code;
    bomb_code_user = "";
    failed_code_count = 0;
    bomb_ms_remaining = bomb_time_ms;
    bad_code_ms_remaining = 0;
  }

  void display_update(esphome::lcd_base::LCDDisplay &disp) {
    switch (state) {
    case STATE::ARM:            display_arm(disp); break;
    case STATE::BAD_CODE:       display_bad_code(disp); break;
    case STATE::ARMED:          display_armed(disp); break;
    case STATE::BAD_CODE_ARMED: display_bad_code(disp); break;
    case STATE::DISARMED:       display_disarmed(disp); break;
    case STATE::EXPLODED:       display_exploded(disp); break;
    }
  }

  void handle_key(unsigned char key) {
    switch (state) {
    case STATE::ARM:            handle_key_arm(key); break;
    case STATE::BAD_CODE:       break;
    case STATE::ARMED:          handle_key_armed(key); break;
    case STATE::BAD_CODE_ARMED: break;
    case STATE::DISARMED:       break;
    case STATE::EXPLODED:       break;
    }
  }

  void clock(uint32_t now, uint32_t delta) {
    if (armed && !finished) { // bomb timer
      bomb_ms_remaining -= delta;
      if (bomb_ms_remaining <= 0) {
        ESP_LOGI("GM_defusal_code", "EXPLODED");
        state = STATE::EXPLODED;
        armed = false;
        finished = true;
        antg.action_siren(SIREN_DURATION_GAME_END, SIREN_GAME_END_DELAY);
      }
    }
    switch (state) {
    case STATE::ARM:            break;
    case STATE::BAD_CODE:       clock_bad_code(now, delta); break;
    case STATE::ARMED:          break;
    case STATE::BAD_CODE_ARMED: clock_bad_code_armed(now, delta); break;
    case STATE::DISARMED:       break;
    case STATE::EXPLODED:       break;
    }
  }
};

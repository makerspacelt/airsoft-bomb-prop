#pragma once

#include "globals.hpp"
#include "utilities.hpp"

#ifdef ESP_PLATFORM
#include "esphome.h"
#else
#include "../src-pc/mock_esphome.hpp"
#endif

static constexpr uint32_t GM_DEFUSAL_MAX_CODE_LEN = 6;

class GameModeDefusalCode {
private:
  enum class STATE { ARM, BAD_CODE, ARMED, BAD_CODE_ARMED, DISARMED, EXPLODED };

  // How long to display the bad code warning
  static constexpr uint32_t BAD_CODE_DISPLAY_MS = 1000;

  STATE state = STATE::ARM;
  std::string bomb_code = "";
  std::string bomb_code_user = "";
  int failed_code_count = 0;
  uint32_t bad_code_ms_remaining = 0;

  void disp_time_left(esphome::lcd_base::LCDDisplay &disp, uint32_t bomb_ms_remaining) {
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

  uint32_t bomb_ms_remaining = 0;

  void start_game(std::string code, uint32_t bomb_time_ms) {
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
    if (!finished) {
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

class GameModeDefusalButtons {
private:
  enum class STATE { READY, ARMING, DISARMING, ARMED, DISARMED, EXPLODED };
  static constexpr uint32_t ARM_TIME = 5000;
  static constexpr uint32_t DISARM_TIME = 10000;

  STATE state = STATE::READY;

  uint32_t key_press_at = 0;

  void disp_time_left(esphome::lcd_base::LCDDisplay &disp, uint32_t bomb_ms_remaining) {
    disp.printf(0, 1, "TIME LEFT:% 6s", format_time_remaining(bomb_ms_remaining).c_str());
  }

  // === READY STATE ===

  void display_ready(esphome::lcd_base::LCDDisplay &disp) {
    disp.printf(0, 0, "     READY      ");
    disp_time_left(disp, bomb_ms_remaining);
  }

  void handle_key_ready(unsigned char key) {
    if (key == KEY_RED || key == KEY_YELLOW) {
      key_press_at = esphome::millis();
      ESP_LOGI("GM_defusal_buttons", "READY -> ARMING");
      state = STATE::ARMING;
    }
  }

  // === ARMING STATE ===

  void handle_key_arming(unsigned char key) {
    switch (key) {
    case KEY_RED_RELEASE:
    case KEY_YELLOW_RELEASE:
      ESP_LOGI("GM_defusal_buttons", "ARMING -> READY");
      key_press_at = 0;
      state = STATE::READY;
      break;
    }
  }

  void display_arming(esphome::lcd_base::LCDDisplay &disp) {
    float ratio = (esphome::millis() - key_press_at) / (float)ARM_TIME;
    disp.printf(0, 0, "ARMING % 6s", format_time_remaining(bomb_ms_remaining).c_str());
    disp.printf(0, 1, "%s", format_progress_bar(ratio).c_str());
  }

  void clock_arming(uint32_t now, uint32_t delta) {
    if (now - key_press_at >= ARM_TIME) {
      armed = true;
      ESP_LOGI("GM_defusal_buttons", "ARMING -> ARMED");
      state = STATE::ARMED;
    }
  }

  // === DISARMING STATE ===

  void handle_key_disarming(unsigned char key) {
    switch (key) {
    case KEY_RED_RELEASE:
    case KEY_YELLOW_RELEASE:
      ESP_LOGI("GM_defusal_buttons", "DISARMING -> ARMED");
      key_press_at = 0;
      state = STATE::ARMED;
      break;
    }
  }

  void display_disarming(esphome::lcd_base::LCDDisplay &disp) {
    float ratio = (esphome::millis() - key_press_at) / (float)DISARM_TIME;
    disp.printf(0, 0, "DISARMING% 6s", format_time_remaining(bomb_ms_remaining).c_str());
    disp.printf(0, 1, "%s", format_progress_bar(ratio).c_str());
  }

  void clock_disarming(uint32_t now, uint32_t delta) {
    if (now - key_press_at >= DISARM_TIME) {
      ESP_LOGI("GM_defusal_buttons", "DISARMING -> DISARMED");
      state = STATE::DISARMED;
      armed = false;
      finished = true;
      antg.action_siren(SIREN_DURATION_GAME_END, SIREN_GAME_END_DELAY);
    }
  }

  // === ARMED STATE ===

  void display_armed(esphome::lcd_base::LCDDisplay &disp) {
    disp.printf(0, 0, "     ARMED      ");
    disp_time_left(disp, bomb_ms_remaining);
  }

  void handle_key_armed(unsigned char key) {
    if (key == KEY_RED || key == KEY_YELLOW) {
      key_press_at = esphome::millis();
      ESP_LOGI("GM_defusal_buttons", "ARMED -> DISARMING");
      state = STATE::DISARMING;
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
  GameModeDefusalButtons(AntGlobals &antg) : antg(antg) {}

  bool armed = false;
  bool finished = false;

  uint32_t bomb_ms_remaining = 0;

  void start_game(uint32_t bomb_time_ms) {
    armed = false;
    finished = false;
    ESP_LOGI("GM_defusal_buttons", "START");
    state = STATE::READY;
    bomb_ms_remaining = bomb_time_ms;
    key_press_at = 0;
  }

  void display_update(esphome::lcd_base::LCDDisplay &disp) {
    switch (state) {
    case STATE::READY:     display_ready(disp); break;
    case STATE::ARMING:    display_arming(disp); break;
    case STATE::DISARMING: display_disarming(disp); break;
    case STATE::ARMED:     display_armed(disp); break;
    case STATE::DISARMED:  display_disarmed(disp); break;
    case STATE::EXPLODED:  display_exploded(disp); break;
    }
  }

  void handle_key(unsigned char key) {
    switch (state) {
    case STATE::READY:     handle_key_ready(key); break;
    case STATE::ARMING:    handle_key_arming(key); break;
    case STATE::DISARMING: handle_key_disarming(key); break;
    case STATE::ARMED:     handle_key_armed(key); break;
    case STATE::DISARMED:  break;
    case STATE::EXPLODED:  break;
    }
  }

  void clock(uint32_t now, uint32_t delta) {
    if (!finished) {
      bomb_ms_remaining -= delta;
      if (bomb_ms_remaining <= 0) {
        ESP_LOGI("GM_defusal_buttons", "EXPLODED");
        state = STATE::EXPLODED;
        armed = false;
        finished = true;
        antg.action_siren(SIREN_DURATION_GAME_END, SIREN_GAME_END_DELAY);
      }
    }
    switch (state) {
    case STATE::READY:     break;
    case STATE::ARMING:    clock_arming(now, delta); break;
    case STATE::DISARMING: clock_disarming(now, delta); break;
    case STATE::ARMED:     break;
    case STATE::DISARMED:  break;
    case STATE::EXPLODED:  break;
    }
  }
};

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
  uint32_t delay_ms_remaining = 0;
  uint32_t bomb_ms_total = 0;
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

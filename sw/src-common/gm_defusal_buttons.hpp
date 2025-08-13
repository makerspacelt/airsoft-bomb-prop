#pragma once

#include "globals.hpp"
#include "utilities.hpp"

#ifdef ESP_PLATFORM
#include "esphome.h"
#else
#include "../src-pc/mock_esphome.hpp"
#endif

class GameModeDefusalButtons {
private:
  enum class STATE { READY, ARMING, DISARMING, ARMED, DISARMED, EXPLODED };
  static constexpr uint32_t ARM_TIME = 5000;
  static constexpr uint32_t DISARM_TIME = 10000;

  STATE state = STATE::READY;

  uint32_t key_press_at = 0;

  void disp_time_left(esphome::lcd_base::LCDDisplay &disp, int32_t bomb_ms_remaining) {
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

  int32_t bomb_ms_remaining = 0;

  void start_game(int32_t bomb_time_ms) {
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
    if (armed && !finished) { // bomb timer
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

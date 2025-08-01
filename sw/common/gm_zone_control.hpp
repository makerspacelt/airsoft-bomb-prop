#pragma once

#include "globals.hpp"
#include "utilities.hpp"

#ifdef ESP_PLATFORM
#include "esphome.h"
#else
#include "../pc-build/mock_esphome.hpp"
#endif

class GameModeZoneControl {
private:
  static constexpr uint32_t CAPTURE_TIME = 5000;

  enum class STATE { SCOREBOARD, CAPTURING };
  enum class TEAM { NONE, RED, YELLOW };

  STATE state = STATE::SCOREBOARD;
  TEAM team_active = TEAM::NONE; // Use enum instead of magic numbers

  uint32_t team_red_time = 0;
  uint32_t team_yellow_time = 0;
  uint32_t capture_start = 0;

  // === SCOREBOARD STATE ===
  void display_scoreboard(esphome::lcd_base::LCDDisplay &disp) {
    disp.printf(0, 0, "TEAM 1:  TEAM 2:");
    disp.printf(0, 1, "%-7d  %-7d", team_red_time / 1000, team_yellow_time / 1000);
  }

  void handle_key_scoreboard(unsigned char key) {
    if (key == KEY_RED && team_active != TEAM::RED) {
      state = STATE::CAPTURING;
      capture_start = esphome::millis();
    } else if (key == KEY_YELLOW && team_active != TEAM::YELLOW) {
      state = STATE::CAPTURING;
      capture_start = esphome::millis();
    }
  }

  // === CAPTURING STATE ===
  void display_capturing(esphome::lcd_base::LCDDisplay &disp) {
    float ratio = (esphome::millis() - capture_start) / (float)CAPTURE_TIME;
    disp.printf(0, 0, "   CAPTURING   ");
    disp.printf(0, 1, "%s", format_progress_bar(ratio).c_str());
  }

  void clock_capturing(uint32_t now, uint32_t delta) {
    if (antg.btn_red_duration >= CAPTURE_TIME) {
      state = STATE::SCOREBOARD;
      team_active = TEAM::RED;
      antg.action_buzzer(BUZZER_TONE, BUZZER_DURATION_TEAM_SWITCH);
    } else if (antg.btn_yellow_duration >= CAPTURE_TIME) {
      state = STATE::SCOREBOARD;
      team_active = TEAM::YELLOW;
      antg.action_buzzer(BUZZER_TONE, BUZZER_DURATION_TEAM_SWITCH);
    } else if (!antg.btn_red_pressed && !antg.btn_yellow_pressed) {
      state = STATE::SCOREBOARD;
    }
  }

  // === COMMON ===
  void update_team_time(uint32_t delta) {
    switch (team_active) {
    case TEAM::RED:    team_red_time += delta; break;
    case TEAM::YELLOW: team_yellow_time += delta; break;
    case TEAM::NONE:   break;
    }
  }

public:
  AntGlobals &antg;
  GameModeZoneControl(AntGlobals &antg) : antg(antg) {}

  void init() {
    state = STATE::SCOREBOARD;
    team_active = TEAM::NONE;
    team_red_time = 0;
    team_yellow_time = 0;
  }

  void display_update(esphome::lcd_base::LCDDisplay &disp) {
    switch (state) {
    case STATE::SCOREBOARD: display_scoreboard(disp); break;
    case STATE::CAPTURING:  display_capturing(disp); break;
    }
  }

  void handle_key(unsigned char key) {
    switch (state) {
    case STATE::SCOREBOARD: handle_key_scoreboard(key); break;
    case STATE::CAPTURING:  break;
    }
  }

  void clock(uint32_t now, uint32_t delta) {
    update_team_time(delta);

    switch (state) {
    case STATE::CAPTURING:  clock_capturing(now, delta); break;
    case STATE::SCOREBOARD: break;
    }
  }
};

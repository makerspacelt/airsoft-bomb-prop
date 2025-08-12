#pragma once

#include "globals.hpp"
#include "gm_countdown.hpp"
#include "gm_defusal.hpp"
#include "gm_domination.hpp"
#include "gm_respawn_timer.hpp"
#include "gm_settings.hpp"
#include "gm_zone_control.hpp"

#ifdef ESP_PLATFORM
#include "esphome.h"
#else
#include "../src-pc/mock_esphome.hpp"
#endif

class GameManager {
private:
  enum class MODE { DEFUSAL, DOMINATION, ZONE_CONTROL, COUNTDOWN, RESPAWN_TIMER, SETTINGS, COUNT };
  // COUNT is used as a placeholder for the last value and also as an unselected gamemode.
  static constexpr MODE MODE_NONE = MODE::COUNT;

  enum class STATE { SPLASH, MENU };
  STATE state = STATE::SPLASH;
  uint32_t splash_start_time = 0;

  MODE menu = MODE::DEFUSAL;
  MODE current_game = MODE_NONE;

  AntGlobals &antg;
  GameModeDefusal gm_defusal;
  GameModeDomination gm_domination;
  GameModeZoneControl gm_zone_control;
  GameModeCountdown gm_countdown;
  GameModeRespawnTimer gm_respawn_timer;
  GameSettings gm_settings;

  void handle_actions() {
    // NOTE
    // This function directly references the `s_handle_actions` esphome script defined in config.yaml.
    // This is unsupported by esphome. If it stops working at some point, check the generated
    // .esphome/build/kms-ant-v2/src/main.cpp for the new signature/name.
    // Also the esphome definition must be kept in sync with mock_esphome.hpp for the PC build to work.

    uint32_t actions = antg.actions;
    if (actions == 0) {
      return;
    }

    if (actions & ACTION_EXIT_GAME) {
      ESP_LOGI("GameManager", "Exiting game");
      current_game = MODE_NONE;
    }
    if (actions & ACTION_START_SIREN) {
      ant_siren_t *s = &antg.siren_params;
      ESP_LOGI("GameManager", "Siren for %dms at %dHz level %f with %dms delay", s->duration, s->tone, s->level,
               s->delay);
    }
    if (actions & ACTION_START_BUZZER) {
      ant_buzzer_t *b = &antg.buzzer_params;
      ESP_LOGI("GameManager", "Buzzer for %dms at %dHz", b->duration, b->tone);
    }
    if (actions & ACTION_SAVE_SIREN_LEVEL) {
      ESP_LOGI("GameManager", "Saving siren level: %f (user level %d)", antg.settings.siren_level,
               antg.settings.siren_level_user);
    }
    s_handle_actions->execute();
    antg.clear_actions(); // all actions have been handled.
  }

  void display_splash(esphome::lcd_base::LCDDisplay &disp) {
    disp.print(0, 0, "   KMS ANT V2   ");
    disp.print(0, 1, "  makerspace.lt ");
  }

  void display_menu(esphome::lcd_base::LCDDisplay &disp) {
    switch (current_game) {
    case MODE_NONE:
      // No game activated, render menu
      switch (menu) {
      case MODE::DEFUSAL:
        disp.print(0, 0, "> Defusal");
        disp.print(0, 1, "  Domination");
        break;
      case MODE::DOMINATION:
        disp.print(0, 0, "  Defusal");
        disp.print(0, 1, "> Domination");
        break;
      case MODE::ZONE_CONTROL:
        disp.print(0, 0, "  Domination");
        disp.print(0, 1, "> Zone control");
        break;
      case MODE::COUNTDOWN:
        disp.print(0, 0, "  Zone control");
        disp.print(0, 1, "> Timer");
        break;
      case MODE::RESPAWN_TIMER:
        disp.print(0, 0, "  Timer");
        disp.print(0, 1, "> Respawn timer");
        break;
      case MODE::SETTINGS:
        disp.print(0, 0, "  Respawn timer");
        disp.print(0, 1, "> Settings");
        break;
      case MODE_NONE: ESP_LOGW("GameManager", "COUNT menu item should never be selected."); break;
      }
      break;
    case MODE::DEFUSAL:       gm_defusal.display_update(disp); break;
    case MODE::DOMINATION:    gm_domination.display_update(disp); break;
    case MODE::ZONE_CONTROL:  gm_zone_control.display_update(disp); break;
    case MODE::COUNTDOWN:     gm_countdown.display_update(disp); break;
    case MODE::RESPAWN_TIMER: gm_respawn_timer.display_update(disp); break;
    case MODE::SETTINGS:      gm_settings.display_update(disp); break;
    }
  }

  void handle_key_splash(unsigned char key) {
    // Immediate switch to MENU on any key press
    state = STATE::MENU;
  }

  void handle_key_menu(unsigned char key) {
    // Activate buzzer for key presses
    switch (key) {
    case KEY_C: antg.action_buzzer(BUZZER_TONE_C); break;
    case KEY_D: antg.action_buzzer(BUZZER_TONE_D); break;
    case KEY_RED:
    case KEY_RED_RELEASE:
    case KEY_YELLOW:
    case KEY_YELLOW_RELEASE:
      // Do not buzz for short RED/YELLOW button press.
      break;
    default: antg.action_buzzer(BUZZER_TONE);
    }

    // Long press timings
    switch (key) {
    case KEY_RED:    antg.btn_red_pressed = esphome::millis(); break;
    case KEY_YELLOW: antg.btn_yellow_pressed = esphome::millis(); break;
    case KEY_RED_RELEASE:
      antg.btn_red_pressed = 0;
      antg.btn_red_duration = 0;
      break;
    case KEY_YELLOW_RELEASE:
      antg.btn_yellow_pressed = 0;
      antg.btn_yellow_duration = 0;
      break;
    }

    // Handle keys
    switch (current_game) {
    case MODE_NONE:
      switch (key) {
      case KEY_A: menu = enum_prev(menu); break;
      case KEY_B: menu = enum_next(menu); break;
      case KEY_C:
        // Activate the selected mode
        current_game = menu;
        switch (current_game) {
        case MODE::DEFUSAL:       gm_defusal.init(); break;
        case MODE::DOMINATION:    gm_domination.init(); break;
        case MODE::ZONE_CONTROL:  gm_zone_control.init(); break;
        case MODE::COUNTDOWN:     gm_countdown.init(); break;
        case MODE::RESPAWN_TIMER: gm_respawn_timer.init(); break;
        case MODE::SETTINGS:      gm_settings.init(); break;
        case MODE_NONE:           break;
        }
        break;
      }
      break;
    case MODE::DEFUSAL:       gm_defusal.handle_key(key); break;
    case MODE::DOMINATION:    gm_domination.handle_key(key); break;
    case MODE::ZONE_CONTROL:  gm_zone_control.handle_key(key); break;
    case MODE::COUNTDOWN:     gm_countdown.handle_key(key); break;
    case MODE::RESPAWN_TIMER: gm_respawn_timer.handle_key(key); break;
    case MODE::SETTINGS:      gm_settings.handle_key(key); break;
    }

    if (key == KEY_RESET) {
      antg.action_buzzer(BUZZER_TONE_SPECIAL, BUZZER_DURATION_SPECIAL);
      ESP_LOGI("GameManager", "Hard reset");
      current_game = MODE_NONE;
    }

    handle_actions();
  }

  void clock_splash(uint32_t now, uint32_t delta) {
    if (now - splash_start_time >= 2000) {
      state = STATE::MENU;
    }
  }

  void clock_menu(uint32_t now, uint32_t delta) {
    if (hard_reset_press_at && now - hard_reset_press_at > HARD_RESET_KEY_HOLD_DURATION) {
      handle_key(KEY_RESET);
      hard_reset_press_at = 0;
      return;
    }
    if (key_c_press_at && now - key_c_press_at > KEY_C_LONG_HOLD_DURATION) {
      handle_key(KEY_C_LONG);
      key_c_press_at = 0;
    }

    if (antg.btn_red_pressed) {
      antg.btn_red_duration = now - antg.btn_red_pressed;
    }
    if (antg.btn_yellow_pressed) {
      antg.btn_yellow_duration = now - antg.btn_yellow_pressed;
    }

    switch (current_game) {
    case MODE::DEFUSAL:       gm_defusal.clock(now, delta); break;
    case MODE::DOMINATION:    gm_domination.clock(now, delta); break;
    case MODE::ZONE_CONTROL:  gm_zone_control.clock(now, delta); break;
    case MODE::COUNTDOWN:     gm_countdown.clock(now, delta); break;
    case MODE::RESPAWN_TIMER: gm_respawn_timer.clock(now, delta); break;
    case MODE::SETTINGS:      gm_settings.clock(now, delta); break;
    case MODE_NONE:           break;
    }

    handle_actions();
    clock_last_update_ms = now;
  }

public:
  int hard_reset_press_at = 0;
  int key_c_press_at = 0;

  int clock_last_update_ms = 0;

  GameManager(AntGlobals &antg)
      : antg(antg), gm_defusal(antg), gm_domination(antg), gm_zone_control(antg), gm_countdown(antg),
        gm_respawn_timer(antg), gm_settings(antg) {
    splash_start_time = esphome::millis();
  }

  void display_update(esphome::lcd_base::LCDDisplay &disp) {
    switch (state) {
    case STATE::SPLASH: display_splash(disp); break;
    case STATE::MENU:   display_menu(disp); break;
    }
  }

  void handle_key(unsigned char key) {
    switch (state) {
    case STATE::SPLASH: handle_key_splash(key); break;
    case STATE::MENU:   handle_key_menu(key); break;
    }
  }

  void clock(uint32_t now, uint32_t delta) {
    switch (state) {
    case STATE::SPLASH: clock_splash(now, delta); break;
    case STATE::MENU:   clock_menu(now, delta); break;
    }
  }
};

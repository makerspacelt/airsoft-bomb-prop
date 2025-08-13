#pragma once

#include "globals.hpp"
#include "utilities.hpp"
#include "gm_settings_ota.hpp"
#include "gm_settings_siren.hpp"

#ifdef ESP_PLATFORM
#include "esphome.h"
#else
#include "../src-pc/mock_esphome.hpp"
#endif

class GameSettings {
private:
  enum class STATE { TOP, SIREN, OTA };
  STATE state = STATE::TOP;

  enum class MENU { SIREN, OTA, BACK, COUNT };
  MENU menu = MENU::SIREN;

  AntGlobals &antg;
  GameSettingsSiren siren_settings;
  GameSettingsOTA ota_update;

public:
  GameSettings(AntGlobals &antg) : antg(antg), siren_settings(antg), ota_update(antg) {}

  void init() {
    state = STATE::TOP;
    menu = MENU::SIREN;
  }

  void display_update(esphome::lcd_base::LCDDisplay &disp) {
    switch (state) {
    case STATE::TOP:
      switch (menu) {
      case MENU::SIREN:
        disp.printf(0, 0, "> Siren settings");
        disp.printf(0, 1, "  OTA update");
        break;
      case MENU::OTA:
        disp.printf(0, 0, "  Siren settings");
        disp.printf(0, 1, "> OTA update");
        break;
      case MENU::BACK:
        disp.printf(0, 0, "  OTA Update");
        disp.printf(0, 1, "> Back");
        break;
      case MENU::COUNT: break;
      }
      break;
    case STATE::OTA:   ota_update.display_update(disp); break;
    case STATE::SIREN: siren_settings.display_update(disp); break;
    }
  }

  void handle_key(unsigned char key) {
    switch (state) {
    case STATE::TOP:
      switch (key) {
      case KEY_A: menu = enum_prev(menu); break;
      case KEY_B: menu = enum_next(menu); break;
      case KEY_C:
        switch (menu) {
        case MENU::SIREN:
          state = STATE::SIREN;
          siren_settings.init();
          break;
        case MENU::OTA:
          state = STATE::OTA;
          ota_update.init();
          break;
        case MENU::BACK:  antg.action_exit_game(); break;
        case MENU::COUNT: break;
        }
        break;
      case KEY_D: antg.action_exit_game(); break;
      }
      break;
    case STATE::OTA:
      ota_update.handle_key(key);
      if (!ota_update.active) {
        state = STATE::TOP;
      }
      break;
    case STATE::SIREN:
      siren_settings.handle_key(key);
      if (!siren_settings.active) {
        state = STATE::TOP;
      }
      break;
    }
  }

  void clock(uint32_t now, uint32_t delta) {
    switch (state) {
    case STATE::TOP:   break;
    case STATE::OTA:   ota_update.clock(now, delta); break;
    case STATE::SIREN: siren_settings.clock(now, delta); break;
    }
  }
};

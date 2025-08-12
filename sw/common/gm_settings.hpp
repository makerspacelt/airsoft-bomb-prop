#pragma once

#include "globals.hpp"
#include "utilities.hpp"

#ifdef ESP_PLATFORM
#include "esphome.h"
#else
#include "../pc-build/mock_esphome.hpp"
#endif

class GameSettingsSiren {
private:
  enum class MENU { LEVEL, TEST, BACK, COUNT };
  MENU menu = MENU::LEVEL;

  int siren_test_delay_sec = 5; // 1-9
  int siren_test_delay_original = 5;
  uint8_t siren_level = 0;
  uint32_t siren_test_start_at = 0;

public:
  AntGlobals &antg;
  GameSettingsSiren(AntGlobals &antg) : antg(antg) {}

  bool active = false; // whether the user has entered siren settings

  void init() {
    menu = MENU::LEVEL;
    active = true;
    siren_level = antg.settings.siren_level_user;
  }

  void display_update(esphome::lcd_base::LCDDisplay &disp) {
    switch (menu) {
    case MENU::LEVEL:
      disp.printf(0, 0, "> Siren level: %d", siren_level);
      disp.printf(0, 1, "  Test (wait %ds)", siren_test_delay_sec);
      break;
    case MENU::TEST:
      disp.printf(0, 0, "  Siren level: %d", siren_level);
      disp.printf(0, 1, "> Test (wait %ds)", siren_test_delay_sec);
      break;
    case MENU::BACK:
      disp.printf(0, 0, "  Test (wait %ds)", siren_test_delay_sec);
      disp.printf(0, 1, "> Back");
      break;
    case MENU::COUNT: break;
    }
  }

  void handle_key(unsigned char key) {
    switch (key) {
    case KEY_A: menu = enum_prev(menu); break;
    case KEY_B: menu = enum_next(menu); break;
    case KEY_D:
      menu = MENU::LEVEL;
      active = false;
      break;
    case KEY_C:
      switch (menu) {
      case MENU::LEVEL: break;
      case MENU::TEST:
        // start siren test
        antg.action_siren(SIREN_DURATION_TEST, siren_test_delay_sec * 1000);
        siren_test_start_at = esphome::millis();
        siren_test_delay_original = siren_test_delay_sec;
        break;
      case MENU::BACK:
        menu = MENU::LEVEL;
        active = false;
        break;
      case MENU::COUNT: break;
      }
      break;
    case KEY_STAR:
      if (menu == MENU::TEST) {
        siren_test_delay_sec = 5;
      }
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
      case MENU::LEVEL:
        if (key != KEY_0) {
          siren_level = append_digit(key, siren_level, 1);
          antg.action_set_siren_level(siren_level);
        }
        break;
      case MENU::TEST:
        if (key != KEY_0) {
          siren_test_delay_sec = append_digit(key, siren_test_delay_sec, 1);
        }
        break;
      case MENU::BACK:  break;
      case MENU::COUNT: break;
      }
      break;
    }
  }

  void clock(uint32_t now, uint32_t delta) {
    if (siren_test_start_at) {
      siren_test_delay_sec = siren_test_delay_original - (now - siren_test_start_at) / 1000;
      if (siren_test_delay_sec <= 0) {
        siren_test_delay_sec = siren_test_delay_original;
        siren_test_start_at = 0;
      }
    }
  }
};

class GameSettingsOTA {
private:
  enum class MENU { SSID, IP, BACK, COUNT };
  MENU menu = MENU::SSID;

public:
  AntGlobals &antg;
  GameSettingsOTA(AntGlobals &antg) : antg(antg) {}

  bool active = false; // whether the user has entered ota settings

  void init() {
    menu = MENU::SSID;
    active = true;
    antg.action_start_ota();
  }

  void display_update(esphome::lcd_base::LCDDisplay &disp) {
    switch (menu) {
    case MENU::SSID:
      disp.printf(0, 0, "SSID: %s", antg.ota_info.ssid.c_str());
      disp.printf(0, 1, "PASS: %s", antg.ota_info.psk.c_str());
      break;
    case MENU::IP:
      disp.printf(0, 0, "Browse to IP:");
      disp.printf(0, 1, "%s", antg.ota_info.ip.c_str());
      break;
    case MENU::BACK:
      disp.printf(0, 0, "Version: %s", ANT_VERSION);
      disp.printf(0, 1, "> Back");
      break;
    case MENU::COUNT: break;
    }
  }

  void handle_key(unsigned char key) {
    switch (key) {
    case KEY_A: menu = enum_prev(menu); break;
    case KEY_B: menu = enum_next(menu); break;
    case KEY_D:
      menu = MENU::SSID;
      active = false;
      antg.action_stop_ota();
      break;
    case KEY_C:
      switch (menu) {
      case MENU::BACK:
        menu = MENU::SSID;
        active = false;
        antg.action_stop_ota();
        break;
      default: menu = enum_next(menu); break;
      }
      break;
    }
  }

  void clock(uint32_t now, uint32_t delta) {}
};

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

#pragma once

#include "globals.hpp"
#include "utilities.hpp"

#ifdef ESP_PLATFORM
#include "esphome.h"
#else
#include "../src-pc/mock_esphome.hpp"
#endif

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

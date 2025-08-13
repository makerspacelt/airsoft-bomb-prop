#pragma once

#include <cstdint>
#include <string>

constexpr unsigned char KEY_0 = '0';
constexpr unsigned char KEY_1 = '1';
constexpr unsigned char KEY_2 = '2';
constexpr unsigned char KEY_3 = '3';
constexpr unsigned char KEY_4 = '4';
constexpr unsigned char KEY_5 = '5';
constexpr unsigned char KEY_6 = '6';
constexpr unsigned char KEY_7 = '7';
constexpr unsigned char KEY_8 = '8';
constexpr unsigned char KEY_9 = '9';
constexpr unsigned char KEY_A = 'A';
constexpr unsigned char KEY_B = 'B';
constexpr unsigned char KEY_C = 'C';
constexpr unsigned char KEY_D = 'D';
constexpr unsigned char KEY_STAR = '*';
constexpr unsigned char KEY_HASH = '#';
constexpr unsigned char KEY_RED = 'r';
constexpr unsigned char KEY_RED_RELEASE = 'R';
constexpr unsigned char KEY_YELLOW = 'y';
constexpr unsigned char KEY_YELLOW_RELEASE = 'Y';
constexpr unsigned char KEY_RESET = 'X';  // e(x)it / reset
constexpr unsigned char KEY_C_LONG = 'N'; // (n)ew / restart game

#define ANT_VERSION "2.0.0"

#define GM_DEFUSAL_MAX_CODE_LEN 6 // max defusal bomb code length

#define HARD_RESET_KEY_HOLD_DURATION 10000
#define KEY_C_LONG_HOLD_DURATION 10000
#define BTN_TEAM_SHORT_HOLD_DURATION 5000
#define BTN_TEAM_LONG_HOLD_DURATION 10000

#define SIREN_DURATION_GAME_START 8000
#define SIREN_DURATION_GAME_END 12000
#define SIREN_DURATION_TEST 5000
#define SIREN_GAME_END_DELAY 5000

#define BUZZER_LEVEL 0.5

#define BUZZER_DURATION 100
#define BUZZER_DURATION_TEAM_SWITCH 5000
#define BUZZER_DURATION_SPECIAL 400

#define BUZZER_TONE 1000
#define BUZZER_TONE_C 1400
#define BUZZER_TONE_D 400
#define BUZZER_TONE_BOMB 1500
#define BUZZER_TONE_SPECIAL 2200 // reset & long hold

#define ACTION_EXIT_GAME 1
#define ACTION_SAVE_SIREN_LEVEL 2
#define ACTION_START_SIREN 4
#define ACTION_STOP_SIREN 8
#define ACTION_START_BUZZER 16
#define ACTION_START_OTA 32
#define ACTION_STOP_OTA 64

struct ant_siren_t {
  int delay = 0;
  int duration = SIREN_DURATION_GAME_START;
  float level = 0.0004;
  int tone = 1220;
};

struct ant_buzzer_t {
  int duration = BUZZER_DURATION;
  int tone = BUZZER_TONE;
  float level = BUZZER_LEVEL;
};

struct ant_ota_t {
  std::string ssid;
  std::string psk;
  std::string ip;
};

struct ant_settings_t {
  int siren_level_user = 9; // overwritten on boot from flash by esphome
  float siren_level = 1.0;
};

class AntGlobals {
public:
  uint32_t actions = 0; // bitwise of ACTION_ states

  uint32_t btn_red_pressed = 0;
  uint32_t btn_red_duration = 0;
  uint32_t btn_yellow_pressed = 0;
  uint32_t btn_yellow_duration = 0;

  ant_siren_t siren_params = {};
  ant_buzzer_t buzzer_params = {};
  ant_ota_t ota_info = {};
  ant_settings_t settings = {};

  void clear_actions() { actions = 0; }

  void action_exit_game() { actions |= ACTION_EXIT_GAME; }

  void action_siren(int duration = SIREN_DURATION_GAME_START, int delay = 0) {
    siren_params.delay = delay;
    siren_params.duration = duration;
    siren_params.level = settings.siren_level;
    siren_params.tone = 1220;
    actions |= ACTION_START_SIREN;
  }

  void action_stop_siren() { actions |= ACTION_STOP_SIREN; }

  void action_buzzer(int tone = BUZZER_TONE, int duration = BUZZER_DURATION) {
    buzzer_params.duration = duration;
    buzzer_params.tone = tone;
    actions |= ACTION_START_BUZZER;
  }

  void action_set_siren_level(uint8_t level_user, bool save = true) {
    switch (level_user) {
    case 1: // low
      settings.siren_level = 0.001;
      settings.siren_level_user = 1;
      break;
    case 2: // medium
      settings.siren_level = 0.004;
      settings.siren_level_user = 2;
      break;
    default: // high
      settings.siren_level = 1.0;
      settings.siren_level_user = 3;
    }
    if (save) {
      actions |= ACTION_SAVE_SIREN_LEVEL;
    }
  }

  void action_start_ota() { actions |= ACTION_START_OTA; }

  void action_stop_ota() { actions |= ACTION_STOP_OTA; }
};

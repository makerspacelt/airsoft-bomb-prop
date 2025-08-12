#ifndef ESPHOME_VERSION

#include <algorithm>
#include <sstream>
#include <string>
#include <termios.h>
#include <thread>
#include <unistd.h>

#include "../src-common/gm_manager.hpp"
#include "mock_esphome.hpp"

#define TICK_INTERACTIVE 50

// --- Display Mock ---

esphome::lcd_base::LCDDisplay my_display;

// --- Time Mock ---

uint32_t cur_millis = 1;
uint32_t esphome::millis() { return cur_millis; }

// --- Non-blocking keyboard input ---
struct termios oldt, newt;

void set_unbuffered_input() {
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  newt.c_cc[VMIN] = 0;
  newt.c_cc[VTIME] = 0;
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

void restore_input_buffering() { tcsetattr(STDIN_FILENO, TCSANOW, &oldt); }

// --- Main Application ---

AntGlobals antg;
GameManager game_manager(antg);

void update_display() {
  game_manager.display_update(my_display);
  my_display.present();
}

void process_test_sequence(const std::string &sequence) {
  std::stringstream ss(sequence);
  std::string token;

  update_display();

  while (std::getline(ss, token, ',')) {
    // Convert token to uppercase for case-insensitivity
    std::transform(token.begin(), token.end(), token.begin(), [](unsigned char c) { return std::toupper(c); });

    if (token.rfind("DELAY=", 0) == 0) {
      int delay_ms = std::stoi(token.substr(6));
      printf("[DELAY %d]\n", delay_ms);
      cur_millis += delay_ms;
      game_manager.clock(cur_millis, delay_ms);
    } else if (token.length() == 1) {
      printf("[KEY %s]\n", token.c_str());
      game_manager.handle_key(token[0]);
    } else {
      printf("[KEY %s]\n", token.c_str());
      if (token == "RED") {
        game_manager.handle_key(KEY_RED);
      } else if (token == "RED_RELEASE") {
        game_manager.handle_key(KEY_RED_RELEASE);
      } else if (token == "YELLOW") {
        game_manager.handle_key(KEY_YELLOW);
      } else if (token == "YELLOW_RELEASE") {
        game_manager.handle_key(KEY_YELLOW_RELEASE);
      } else if (token == "RESET") {
        game_manager.handle_key(KEY_RESET);
      } else if (token == "C_LONG") {
        game_manager.handle_key(KEY_C_LONG);
      } else {
        printf("ERROR: Unknown test token: %s\n", token.c_str());
      }
    }
    update_display();
  }
}

int main(int argc, char *argv[]) {
  set_unbuffered_input();
  atexit(restore_input_buffering);

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--test" && i + 1 < argc) {
      std::string sequence = argv[++i];
      process_test_sequence(sequence);
      return 0; // Exit after sequence
    }
  }

  ESP_LOGI("main", "Starting interactive mode... Press 'q' to quit.");

  // Interactive loop
  while (true) {
    unsigned char key = 0;
    if (read(STDIN_FILENO, &key, 1) > 0) {
      switch (key) {
      case 'q':
      case 'Q': ESP_LOGI("main", "Exiting interactive mode."); return 0;
      case 'r': game_manager.handle_key(KEY_RED); break;
      case 'R': game_manager.handle_key(KEY_RED_RELEASE); break;
      case 'y': game_manager.handle_key(KEY_YELLOW); break;
      case 'Y': game_manager.handle_key(KEY_YELLOW_RELEASE); break;
      case 'x': game_manager.handle_key(KEY_RESET); break;
      case 'n': game_manager.handle_key(KEY_C_LONG); break;
      default:  game_manager.handle_key(std::toupper(key)); break;
      }
    }

    cur_millis += TICK_INTERACTIVE;
    game_manager.clock(esphome::millis(), TICK_INTERACTIVE);
    update_display();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  return 0;
}

#endif

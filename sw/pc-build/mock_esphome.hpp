#pragma once

#include <cstdarg>
#include <cstdint>
#include <iostream>
#include <string>

//
// Scripts & globals defined in config.yaml that the game manager needs to access.
//
// NOLINTBEGIN(misc-definitions-in-headers)
class ScriptHandleActions {
public:
  void execute() {
  };
} *s_handle_actions;
// NOLINTEND(misc-definitions-in-headers)

//
// Mocked esphome namespace parts that we need to access in our PC build
//
namespace esphome {
uint32_t millis();

namespace lcd_base {
class LCDDisplay {
private:
  std::string rows_prev[2];
  std::string rows[2];
  static const int height = 2;
  static const int width = 16;

  void clear() {
    rows_prev[0] = rows[0];
    rows_prev[1] = rows[1];
    rows[0] = std::string(width, ' ');
    rows[1] = std::string(width, ' ');
  }

public:
  LCDDisplay() { clear(); }

  void present() {
    if (rows[0] != rows_prev[0] || rows[1] != rows_prev[1]) {
      std::cout << "[LCD] |----------------|\n";
      std::cout << "[LCD] |" << rows[0] << "|\n";
      std::cout << "[LCD] |" << rows[1] << "|\n";
      clear(); // Clear buffer for next update cycle
    }
  }

  void print(int col, int row, const char *text) {
    if (row < 0 || row >= height || col >= width)
      return;

    std::string str_text = text;
    // Make str_text take the full rows[row] starting at col.
    // Make sure it is exactly `width - col` long, right-pad with spaces if
    // needed.
    unsigned long int w = width - col;
    str_text = str_text.substr(0, w);
    while (str_text.length() < w) {
      str_text += ' ';
    }
    rows[row].replace(col, w, str_text);
  }

  void printf(int col, int row, const char *format, ...) {
    if (row < 0 || row >= height || col >= width)
      return;

    va_list args;
    va_start(args, format);
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    print(col, row, buffer);
  }
};
} // namespace lcd_base
} // namespace esphome

// Mock ESPHome logging
#define ESP_LOGI(tag, format, ...) printf("[%s] " format "\n", tag, ##__VA_ARGS__)
#define ESP_LOGW(tag, format, ...) printf("[%s] " format "\n", tag, ##__VA_ARGS__)

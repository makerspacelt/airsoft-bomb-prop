#include <iomanip>

#include "utilities.hpp"

int append_digit(char digit_to_append, int current_value, long unsigned int max_len) {
  // Appends digit to the current value.
  // If value is 0 or exceeds max length, overrides it with digit.
  if (current_value == 0 || std::to_string(current_value).length() >= max_len) {
    return digit_to_append - '0';
  }
  return current_value * 10 + (digit_to_append - '0');
}

std::string format_time_remaining(int milliseconds) {
  if (milliseconds < 0)
    milliseconds = 0;
  int totalSeconds = milliseconds / 1000;
  int minutes = totalSeconds / 60;
  int seconds = totalSeconds % 60;

  std::ostringstream oss;
  oss << std::setw(2) << std::setfill('0') << minutes << ":" << std::setw(2) << std::setfill('0') << seconds;
  return oss.str();
}

// Given a ratio 0.1-1.0, return 16 characters, where each character consists of one of:
// * " " - empty
// * \x01 to \x05 - special character which fills 1 to 5 columns of the character width.
std::string format_progress_bar(float ratio) {
  if (ratio < 0)
    ratio = 0;
  if (ratio > 1)
    ratio = 1;

  static const int num_chars = 16;
#ifdef ESP_PLATFORM
  static const std::string chars = "\x01\x01\x02\x02\x03\x03\x04\x04\x05\x05";
  static const char block = '\x05';
#else
  static const std::string chars = "1122334455";
  static const char block = 'X';
#endif

  const float filled_ratio = ratio * num_chars;
  const int filled_chars = (int)filled_ratio;
  const int last_col_char_idx = 10 * (filled_ratio - filled_chars); // 0-9
                                                                    //
  std::string result(num_chars, ' ');

  // Fill complete characters
  for (int i = 0; i < filled_chars; ++i) {
    result[i] = block;
  }

  // Fill partial character if needed
  if (filled_chars < num_chars) {
    result[filled_chars] = chars[last_col_char_idx];
  }

  return result;
}

int scale(int value, int in_min, int in_max, int out_min, int out_max) {
  return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float scale(float value, float in_min, float in_max, float out_min, float out_max) {
  return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

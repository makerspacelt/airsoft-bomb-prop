#pragma once

#include <string>

int append_digit(char digit_to_append, int current_value, long unsigned int max_len);
std::string format_time_remaining(int milliseconds);
std::string format_progress_bar(float percent);
int scale(int value, int in_min, int in_max, int out_min, int out_max);
float scale(float value, float in_min, float in_max, float out_min, float out_max);

template <typename Enum> Enum enum_next(Enum current) {
  int val = static_cast<int>(current) + 1;
  int last = static_cast<int>(Enum::COUNT);
  return static_cast<Enum>(val >= last ? 0 : val);
}

template <typename Enum> Enum enum_prev(Enum current) {
  int val = static_cast<int>(current) - 1;
  int last = static_cast<int>(Enum::COUNT);
  return static_cast<Enum>(val < 0 ? last - 1 : val);
}

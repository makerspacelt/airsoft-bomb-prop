#pragma once

#include <string>

int append_digit(char digit_to_append, int current_value, long unsigned int max_len);
std::string format_time_remaining(int milliseconds);
std::string format_progress_bar(float percent);

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

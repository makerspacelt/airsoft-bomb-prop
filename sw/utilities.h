#include <iomanip>    // <-- Needed for std::setw and std::setfill
#include <sstream>    // <-- Needed for std::ostringstream

int time_to_arm_s = 5;
int time_to_defuse_s = 5;

int append_digit(std::string digit_to_append, int current_value) {
    // Appends digit to the current value. If value is 0, overrides it with digit. If value exceeds max length, does not append digit
    
    int num = std::stoi(digit_to_append);
    int next_value = current_value;
    if (current_value == 0) {
        next_value = num;
    } else {
        std::string next_value_str = std::to_string(current_value) + digit_to_append;
        if (next_value_str.length() <= 3) {
            next_value = std::stoi(next_value_str);
        }
    }
    return next_value;
}


std::string format_time_remaining(int milliseconds) {
    int totalSeconds = milliseconds / 1000;
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;

    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << minutes
        << ":"
        << std::setw(2) << std::setfill('0') << seconds;
    return oss.str();
}


#pragma once
#include <string>

namespace intercept {
    bool init();
    void cleanup();
    void reset();

    void key_down(const std::string& key);
    void key_up(const std::string& key);

    void mouse_down(const std::string& button);
    void mouse_up(const std::string& button);
    void move_relative(int dx, int dy);

    int get_keyboard_count();
    int get_mouse_count();
    std::string get_device_list_report();
    
    // Internal use or for status
    bool is_initialized();
}

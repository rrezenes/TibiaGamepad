#pragma once

#include <string>
#include <map>
#include <vector>
#include <atomic>

// nlohmann::json is forward-declared or included in cpp, but since we don't return it here,
// we just need standard library types.

class Config {
public:
    static std::atomic<bool> RUNNING;
    static std::atomic<bool> LOG_INPUT;
    static std::atomic<int> MOUSE_SENSITIVITY;
    static std::atomic<float> DEADZONE_STICK;
    static std::string MOUSE_STICK;

    static std::map<std::string, int> MOVE_AXIS;
    static std::map<std::string, std::vector<std::string>> MAP;

    static void save();
    static bool load();

private:
    static const std::string CONFIG_FILE;
};

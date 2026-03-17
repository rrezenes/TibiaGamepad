#include "ActionExecutor.h"
#include "Config.h"
#include "InterceptionWrapper.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <algorithm>
#include <atomic>
#include <cmath>

using namespace std::chrono_literals;

namespace {
    std::thread movement_thread;
    std::thread mouse_thread;
    std::atomic<bool> exit_threads{false};
    std::atomic<float> g_mouse_x{0.0f};
    std::atomic<float> g_mouse_y{0.0f};

    std::string to_lower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
        return s;
    }

    std::string strip(std::string s) {
        s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());
        return s;
    }
}

void ActionExecutor::init() {
    if (!intercept::init()) {
         std::cerr << "[ERRO] Falha ao inicializar o driver Interception!" << std::endl;
    }
}

void ActionExecutor::cleanup() {
    stop_threads();
    intercept::cleanup();
}

void ActionExecutor::start_threads() {
    exit_threads = false;
    movement_thread = std::thread(movement_loop);
    mouse_thread = std::thread(mouse_runner);
}

void ActionExecutor::stop_threads() {
    exit_threads = true;
    if (movement_thread.joinable()) movement_thread.join();
    if (mouse_thread.joinable()) mouse_thread.join();
}

void ActionExecutor::update_mouse_stick_pos(float mx, float my) {
    g_mouse_x = mx;
    g_mouse_y = my;
}

void ActionExecutor::execute_action(const std::string& btn_name, const std::vector<std::string>& action_list) {
    if (!Config::RUNNING) return;
    if (action_list.size() < 2) return;
    
    std::string mod = strip(to_lower(action_list[0]));
    std::string key = strip(to_lower(action_list[1]));
    
    if (mod.empty() && key.empty()) return;

    std::cout << "[DEBUG] " << btn_name << " -> " << (!mod.empty() ? "(" + mod + ") + " : "") << key << std::endl;

    if (mod == "alt" || mod == "ctrl" || mod == "shift") {
        intercept::key_down(mod);
        std::this_thread::sleep_for(10ms);
    }

    if (key == "mouse_left") {
        intercept::mouse_down("left");
        std::this_thread::sleep_for(10ms);
        intercept::mouse_up("left");
    } else if (key == "mouse_right") {
        intercept::mouse_down("right");
        std::this_thread::sleep_for(10ms);
        intercept::mouse_up("right");
    } else if (key == "mouse_middle") {
        intercept::mouse_down("middle");
        std::this_thread::sleep_for(10ms);
        intercept::mouse_up("middle");
    } else if (!key.empty()) {
        intercept::key_down(key);
        std::this_thread::sleep_for(20ms);
        intercept::key_up(key);
    }

    if (mod == "alt" || mod == "ctrl" || mod == "shift") {
        std::this_thread::sleep_for(10ms);
        intercept::key_up(mod);
    }
}

void ActionExecutor::movement_loop() {
    while (!exit_threads) {
        if (Config::RUNNING) {
            // Read from configuration
            for (auto const& [key, active] : Config::MOVE_AXIS) {
                if (active > 0) {
                    intercept::key_down(key);
                }
            }
            std::this_thread::sleep_for(30ms);
        } else {
            std::this_thread::sleep_for(500ms);
        }
    }
}

void ActionExecutor::mouse_runner() {
    while (!exit_threads) {
        if (Config::RUNNING) {
            float mx = g_mouse_x.load();
            float my = g_mouse_y.load();
            if (std::abs(mx) > 0.15f || std::abs(my) > 0.15f) {
                int dx = static_cast<int>(mx * Config::MOUSE_SENSITIVITY);
                int dy = static_cast<int>(-my * Config::MOUSE_SENSITIVITY);
                intercept::move_relative(dx, dy);
            }
        }
        std::this_thread::sleep_for(1ms);
    }
}

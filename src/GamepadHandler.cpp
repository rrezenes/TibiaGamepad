#include "GamepadHandler.h"
#include "Config.h"
#include "ActionExecutor.h"
#include "InterceptionWrapper.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <cmath>
#include <atomic>

using namespace std::chrono_literals;

namespace {
    std::thread gamepad_thread;
    std::atomic<bool> exit_gamepad{false};
    XINPUT_STATE last_state = {};
    bool controller_connected = false;
}

void GamepadHandler::init() {
    // Basic initialization if necessary
}

void GamepadHandler::start() {
    exit_gamepad = false;
    gamepad_thread = std::thread(poll_loop);
}

void GamepadHandler::stop() {
    exit_gamepad = true;
    if (gamepad_thread.joinable()) {
        gamepad_thread.join();
    }
}

float GamepadHandler::normalize_stick(SHORT value) {
    if (value > 0) return static_cast<float>(value) / 32767.0f;
    return static_cast<float>(value) / 32768.0f;
}

bool GamepadHandler::is_connected() {
    return controller_connected;
}

void GamepadHandler::poll_loop() {
    while (!exit_gamepad) {
        XINPUT_STATE state;
        ZeroMemory(&state, sizeof(XINPUT_STATE));

        DWORD dwResult = XInputGetState(0, &state);

        if (dwResult == ERROR_SUCCESS) {
            if (!controller_connected) {
                std::cout << "[LOG] Controle CONECTADO" << std::endl;
                controller_connected = true;
            }

            if (Config::RUNNING) {
                if (state.dwPacketNumber != last_state.dwPacketNumber) {
                    process_button_event(state.Gamepad.wButtons, last_state.Gamepad.wButtons);
                    process_trigger_event(state.Gamepad.bLeftTrigger, state.Gamepad.bRightTrigger, 
                                          last_state.Gamepad.bLeftTrigger, last_state.Gamepad.bRightTrigger);
                }
                
                process_stick_event(state.Gamepad.sThumbLX, state.Gamepad.sThumbLY,
                                    state.Gamepad.sThumbRX, state.Gamepad.sThumbRY);
            }
            
            last_state = state;
        } else {
            if (controller_connected) {
                std::cout << "[LOG] Controle DESCONECTADO" << std::endl;
                controller_connected = false;
            }
        }
        
        std::this_thread::sleep_for(10ms);
    }
}

void GamepadHandler::process_button_event(WORD buttons, WORD last_buttons) {
    auto check_button = [&](WORD mask, const std::string& name) {
        bool is_down = (buttons & mask) != 0;
        bool was_down = (last_buttons & mask) != 0;
        if (is_down && !was_down) {
            if (Config::LOG_INPUT) std::cout << "[LOG] Botão PRESSIONADO: " << name << std::endl;
            if (Config::MAP.find(name) != Config::MAP.end()) {
                ActionExecutor::execute_action(name, Config::MAP[name]);
            }
        } else if (!is_down && was_down) {
            if (Config::LOG_INPUT) std::cout << "[LOG] Botão SOLTO: " << name << std::endl;
        }
    };

    check_button(XINPUT_GAMEPAD_A, "A");
    check_button(XINPUT_GAMEPAD_B, "B");
    check_button(XINPUT_GAMEPAD_X, "X");
    check_button(XINPUT_GAMEPAD_Y, "Y");
    check_button(XINPUT_GAMEPAD_LEFT_SHOULDER, "LB");
    check_button(XINPUT_GAMEPAD_RIGHT_SHOULDER, "RB");
    check_button(XINPUT_GAMEPAD_BACK, "BACK");
    check_button(XINPUT_GAMEPAD_START, "START");
    check_button(XINPUT_GAMEPAD_DPAD_UP, "UP");
    check_button(XINPUT_GAMEPAD_DPAD_DOWN, "DOWN");
    check_button(XINPUT_GAMEPAD_DPAD_LEFT, "LEFT");
    check_button(XINPUT_GAMEPAD_DPAD_RIGHT, "RIGHT");
    check_button(XINPUT_GAMEPAD_LEFT_THUMB, "L3");
    check_button(XINPUT_GAMEPAD_RIGHT_THUMB, "R3");
}

void GamepadHandler::process_trigger_event(BYTE left, BYTE right, BYTE last_left, BYTE last_right) {
    float lf = left / 255.0f;
    float l_lf = last_left / 255.0f;
    if (lf >= 0.5f && l_lf < 0.5f) {
        if (Config::MAP.find("LT") != Config::MAP.end())
            ActionExecutor::execute_action("LT", Config::MAP["LT"]);
    }

    float rf = right / 255.0f;
    float l_rf = last_right / 255.0f;
    if (rf >= 0.5f && l_rf < 0.5f) {
        if (Config::MAP.find("RT") != Config::MAP.end())
            ActionExecutor::execute_action("RT", Config::MAP["RT"]);
    }
}

void GamepadHandler::process_stick_event(SHORT lx, SHORT ly, SHORT rx, SHORT ry) {
    float flx = normalize_stick(lx);
    float fly = normalize_stick(ly);
    float frx = normalize_stick(rx);
    float fry = normalize_stick(ry);
    
    auto now = std::chrono::steady_clock::now();
    bool log_left = (std::abs(flx) > 0.2f || std::abs(fly) > 0.2f);
    bool log_right = (std::abs(frx) > 0.2f || std::abs(fry) > 0.2f);
    
    if (log_left || log_right) {
        if (Config::LOG_INPUT) {
            std::string lado = log_left ? "ESQ" : "DIR";
            float print_x = log_left ? flx : frx;
            float print_y = log_left ? fly : fry;
            
            std::cout << "[STICK] " << lado << " -> X: " << std::round(print_x * 100.0) / 100.0 
                      << " | Y: " << std::round(print_y * 100.0) / 100.0 << std::endl;
        }
    }
    
    float mx = 0.0f, my = 0.0f;
    float move_x = 0.0f, move_y = 0.0f;
    
    if (Config::MOUSE_STICK == "LEFT") {
        mx = flx; my = fly;
        move_x = frx; move_y = fry;
    } else {
        mx = frx; my = fry;
        move_x = flx; move_y = fly;
    }
    
    ActionExecutor::update_mouse_stick_pos(mx, my);
    
    float deadzone = Config::DEADZONE_STICK.load();
    Config::MOVE_AXIS["w"] = (move_y >= deadzone) ? 1 : 0;
    Config::MOVE_AXIS["s"] = (move_y <= -deadzone) ? 1 : 0;
    Config::MOVE_AXIS["a"] = (move_x <= -deadzone) ? 1 : 0;
    Config::MOVE_AXIS["d"] = (move_x >= deadzone) ? 1 : 0;
    
    if (!Config::MOVE_AXIS["w"]) intercept::key_up("w");
    if (!Config::MOVE_AXIS["s"]) intercept::key_up("s");
    if (!Config::MOVE_AXIS["a"]) intercept::key_up("a");
    if (!Config::MOVE_AXIS["d"]) intercept::key_up("d");
}

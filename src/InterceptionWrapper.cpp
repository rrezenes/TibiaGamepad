#include "InterceptionWrapper.h"
#include <interception.h>
#include <map>
#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>

namespace intercept {
    InterceptionContext context = 0;
    std::atomic<InterceptionDevice> keyboard_device{1};
    std::atomic<InterceptionDevice> mouse_device{11};
    
    std::thread passthrough_thread;
    std::atomic<bool> stop_passthrough{false};
    
    int keyboard_count = 0;
    int mouse_count = 0;
    std::string device_report = "Nenhum dispositivo detectado.";
    
    std::map<std::string, unsigned short> key_map;

    void init_keymap() {
        key_map["a"] = 0x1E; key_map["b"] = 0x30; key_map["c"] = 0x2E; key_map["d"] = 0x20;
        key_map["e"] = 0x12; key_map["f"] = 0x21; key_map["g"] = 0x22; key_map["h"] = 0x23;
        key_map["i"] = 0x17; key_map["j"] = 0x24; key_map["k"] = 0x25; key_map["l"] = 0x26;
        key_map["m"] = 0x32; key_map["n"] = 0x31; key_map["o"] = 0x18; key_map["p"] = 0x19;
        key_map["q"] = 0x10; key_map["r"] = 0x13; key_map["s"] = 0x1F; key_map["t"] = 0x14;
        key_map["u"] = 0x16; key_map["v"] = 0x2F; key_map["w"] = 0x11; key_map["x"] = 0x2D;
        key_map["y"] = 0x15; key_map["z"] = 0x2C;
        
        key_map["1"] = 0x02; key_map["2"] = 0x03; key_map["3"] = 0x04; key_map["4"] = 0x05;
        key_map["5"] = 0x06; key_map["6"] = 0x07; key_map["7"] = 0x08; key_map["8"] = 0x09;
        key_map["9"] = 0x0A; key_map["0"] = 0x0B;
        
        key_map["f1"] = 0x3B; key_map["f2"] = 0x3C; key_map["f3"] = 0x3D; key_map["f4"] = 0x3E;
        key_map["f5"] = 0x3F; key_map["f6"] = 0x40; key_map["f7"] = 0x41; key_map["f8"] = 0x42;
        key_map["f9"] = 0x43; key_map["f10"] = 0x44; key_map["f11"] = 0x57; key_map["f12"] = 0x58;
        
        key_map["ctrl"] = 0x1D;
        key_map["shift"] = 0x2A;
        key_map["alt"] = 0x38;
        key_map["space"] = 0x39;
        key_map["enter"] = 0x1C;
        key_map["tab"] = 0x0F;
        key_map["esc"] = 0x01;
        key_map["backspace"] = 0x0E;
    }

    void passthrough_loop() {
        InterceptionDevice device;
        InterceptionStroke stroke;
        
        while (!stop_passthrough && context) {
            device = interception_wait_with_timeout(context, 100);
            if (device > 0) {
                if (interception_receive(context, device, &stroke, 1) > 0) {
                    // Identify devices based on real hardware activity
                    if (interception_is_keyboard(device)) {
                        keyboard_device = device;
                    } else if (interception_is_mouse(device)) {
                        mouse_device = device;
                    }
                    
                    // Passthrough: send the stroke back to the system
                    interception_send(context, device, &stroke, 1);
                }
            }
        }
    }

    bool init() {
        if (context) return true; // Already initialized

        context = interception_create_context();
        if (!context) return false;
        
        init_keymap();

        keyboard_count = 0;
        mouse_count = 0;
        device_report = "Dispositivos detectados:\n";

        for (int i = 1; i <= INTERCEPTION_MAX_KEYBOARD; ++i) {
            wchar_t hardware_id[512];
            if (interception_get_hardware_id(context, i, hardware_id, sizeof(hardware_id)) > 0) {
                keyboard_count++;
                device_report += "  [Teclado " + std::to_string(i) + "]\n";
            }
        }

        for (int i = 1; i <= INTERCEPTION_MAX_MOUSE; ++i) {
            int dev = INTERCEPTION_MOUSE(i-1);
            wchar_t hardware_id[512];
            if (interception_get_hardware_id(context, dev, hardware_id, sizeof(hardware_id)) > 0) {
                mouse_count++;
                device_report += "  [Mouse " + std::to_string(i) + "]\n";
            }
        }

        if (keyboard_count == 0 && mouse_count == 0) device_report = "Nenhum dispositivo detectado via Interception.";
        
        // Set filters to capture ALL events for passthrough
        if (context) {
            interception_set_filter(context, interception_is_keyboard, INTERCEPTION_FILTER_KEY_ALL);
            interception_set_filter(context, interception_is_mouse, INTERCEPTION_FILTER_MOUSE_ALL);
            
            stop_passthrough = false;
            passthrough_thread = std::thread(passthrough_loop);
        }

        return true;
    }

    int get_keyboard_count() { return keyboard_count; }
    int get_mouse_count() { return mouse_count; }
    std::string get_device_list_report() { return device_report; }

    void cleanup() {
        stop_passthrough = true;
        if (passthrough_thread.joinable()) {
            passthrough_thread.join();
        }

        if (context) {
            // Disable filters before destroying context
            interception_set_filter(context, interception_is_keyboard, INTERCEPTION_FILTER_KEY_NONE);
            interception_set_filter(context, interception_is_mouse, INTERCEPTION_FILTER_MOUSE_NONE);
            
            interception_destroy_context(context);
            context = 0;
        }
    }

    void reset() {
        std::cout << "[LOG] Reiniciando drivers do Interception..." << std::endl;
        cleanup();
        init();
    }

    bool is_initialized() {
        return context != 0;
    }

    void send_key(const std::string& key, unsigned short state) {
        if (!context) return;
        auto it = key_map.find(key);
        if (it != key_map.end()) {
            InterceptionKeyStroke stroke;
            stroke.code = it->second;
            stroke.state = state;
            stroke.information = 0;
            interception_send(context, keyboard_device, (InterceptionStroke*)&stroke, 1);
        } else {
            std::cerr << "[AVISO] Tecla nao mapeada: " << key << std::endl;
        }
    }

    void key_down(const std::string& key) {
        send_key(key, INTERCEPTION_KEY_DOWN);
    }

    void key_up(const std::string& key) {
        send_key(key, INTERCEPTION_KEY_UP);
    }

    void send_mouse(unsigned short state, int dx, int dy) {
        if (!context) return;
        InterceptionMouseStroke stroke;
        stroke.state = state;
        stroke.flags = INTERCEPTION_MOUSE_MOVE_RELATIVE;
        stroke.rolling = 0;
        stroke.x = dx;
        stroke.y = dy;
        stroke.information = 0;
        interception_send(context, mouse_device, (InterceptionStroke*)&stroke, 1);
    }

    void mouse_down(const std::string& button) {
        if (button == "left" || button == "mouse_left") send_mouse(INTERCEPTION_MOUSE_LEFT_BUTTON_DOWN, 0, 0);
        else if (button == "right" || button == "mouse_right") send_mouse(INTERCEPTION_MOUSE_RIGHT_BUTTON_DOWN, 0, 0);
        else if (button == "middle" || button == "mouse_middle") send_mouse(INTERCEPTION_MOUSE_MIDDLE_BUTTON_DOWN, 0, 0);
    }

    void mouse_up(const std::string& button) {
        if (button == "left" || button == "mouse_left") send_mouse(INTERCEPTION_MOUSE_LEFT_BUTTON_UP, 0, 0);
        else if (button == "right" || button == "mouse_right") send_mouse(INTERCEPTION_MOUSE_RIGHT_BUTTON_UP, 0, 0);
        else if (button == "middle" || button == "mouse_middle") send_mouse(INTERCEPTION_MOUSE_MIDDLE_BUTTON_UP, 0, 0);
    }

    void move_relative(int dx, int dy) {
        send_mouse(0, dx, dy);
    }
}

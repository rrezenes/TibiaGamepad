#pragma once
#include <windows.h>
#include <xinput.h>

class GamepadHandler {
public:
    static void init();
    static void start();
    static void stop();
    static bool is_connected();

private:
    static void poll_loop();
    static void process_button_event(WORD buttons, WORD last_buttons);
    static void process_trigger_event(BYTE left_trigger, BYTE right_trigger, BYTE last_left, BYTE last_right);
    static void process_stick_event(SHORT lx, SHORT ly, SHORT rx, SHORT ry);
    
    static float normalize_stick(SHORT value);
};

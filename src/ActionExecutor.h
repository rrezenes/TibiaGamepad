#pragma once
#include <string>
#include <vector>

class ActionExecutor {
public:
    static void init();
    static void cleanup();
    
    // Replicates python's execute_action
    static void execute_action(const std::string& btn_name, const std::vector<std::string>& action_list);
    
    // Starts the heartbeat threads
    static void start_threads();
    
    // Stops the heartbeat threads (optional on exit)
    static void stop_threads();
    
    // Expose mouse update for GamepadHandler
    static void update_mouse_stick_pos(float mx, float my);

private:
    static void movement_loop();
    static void mouse_runner();
};

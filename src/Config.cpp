#include "Config.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Define static members
std::atomic<bool> Config::RUNNING{false};
std::atomic<bool> Config::LOG_INPUT{true};
std::atomic<int> Config::MOUSE_SENSITIVITY{5};
std::atomic<float> Config::DEADZONE_STICK{0.4f};
std::string Config::MOUSE_STICK = "RIGHT";

std::map<std::string, int> Config::MOVE_AXIS = {
    {"w", 0}, {"s", 0}, {"a", 0}, {"d", 0}
};

std::map<std::string, std::vector<std::string>> Config::MAP = {
    {"A", {"", ""}}, {"B", {"", ""}}, {"X", {"", ""}}, {"Y", {"", ""}},
    {"LB", {"", ""}}, {"RB", {"", ""}}, {"LT", {"", ""}}, {"RT", {"", ""}},
    {"BACK", {"", ""}}, {"START", {"", ""}}, 
    {"UP", {"", ""}}, {"DOWN", {"", ""}}, {"LEFT", {"", ""}}, {"RIGHT", {"", ""}},
    {"L3", {"", ""}}, {"R3", {"", ""}}
};

const std::string Config::CONFIG_FILE = "config_tibia.json";

void Config::save() {
    try {
        json j;
        j["mouse_sensitivity"] = MOUSE_SENSITIVITY.load();
        j["log_input"] = LOG_INPUT.load();
        j["deadzone_stick"] = DEADZONE_STICK.load();
        j["mouse_stick"] = MOUSE_STICK;
        j["mapping"] = MAP;

        std::ofstream o(CONFIG_FILE);
        o << std::setw(4) << j << std::endl;
        std::cout << "[LOG] Configurações salvas em " << CONFIG_FILE << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[ERRO] Falha ao salvar: " << e.what() << std::endl;
    }
}

bool Config::load() {
    std::ifstream i(CONFIG_FILE);
    if (i.is_open()) {
        try {
            json j;
            i >> j;
            if (j.contains("mouse_sensitivity")) MOUSE_SENSITIVITY = j["mouse_sensitivity"];
            if (j.contains("log_input")) LOG_INPUT = j["log_input"];
            if (j.contains("deadzone_stick")) DEADZONE_STICK = j["deadzone_stick"];
            if (j.contains("mouse_stick")) MOUSE_STICK = j["mouse_stick"];
            
            if (j.contains("mapping")) {
                for (auto& [key, val] : j["mapping"].items()) {
                    if (MAP.find(key) != MAP.end()) {
                        MAP[key] = val.get<std::vector<std::string>>();
                    }
                }
            }
            std::cout << "[LOG] Perfil carregado do arquivo." << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "[AVISO] Erro ao carregar config: " << e.what() << std::endl;
        }
    }
    return false;
}

#pragma once

#include <string>

struct Settings {
    std::string apiUrl;
    std::string apiKey;
    std::string model;
};

class SettingsStore {
public:
    static Settings load();
    static bool save(const Settings& settings);
};

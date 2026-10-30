#pragma once

struct Config;

class Menu {
public:
    Menu(Config& config, float maxCameraT);
    void draw();

private:
    Config& m_config;
    float m_maxCameraT;
};

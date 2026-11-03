#pragma once

struct Config;

class Menu {
public:
    Menu(Config& config, float cameraLoopLength);
    void draw();

private:
    Config& m_config;
    float m_cameraLoopLength;
};

#pragma once
#include <glm/glm.hpp>
#include "light.h" 
#include "config.h"

glm::vec3 computeSkyColor(const SkyLight& light, const Config& config);
#pragma once

#include <glm/glm.hpp>
#include <memory>


#include "framework/shader.h"
class Model;

class Environment {
public:
	Environment() = default;
	~Environment();

	bool initialize();

	void draw(
		const glm::mat4& view,
		const glm::mat4& projection
	);
	Environment(const Environment&) = delete;
	Environment& operator=(const Environment&) = delete;

private:
	std::unique_ptr<Model> mountain;
	Shader environmentShader;
};
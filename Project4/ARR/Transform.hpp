#pragma once
#include <glm/glm.hpp>

glm::mat4 Rotate(const int i, const float theta);
glm::mat4 Scale(const float x, const float y, const float z);
glm::mat4 Scale(const float value);
glm::mat4 Scale(const glm::vec3 vec);
glm::mat4 Translate(const float x, const float y, const float z);
glm::mat4 Translate(const glm::vec3 vec);
glm::mat4 Perspective(const float rx, const float ry,
	const float front, const float back);

float* Pntr(glm::mat4& m);
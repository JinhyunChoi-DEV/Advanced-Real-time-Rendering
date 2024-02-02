#pragma once
#include <glm/glm.hpp>

struct Color
{
	inline static glm::vec3 white = glm::vec3(1, 1, 1);
	inline static glm::vec3 black = glm::vec3(0, 0, 0);
	inline static glm::vec3 red = glm::vec3(1, 0, 0);
	inline static glm::vec3 green = glm::vec3(0, 1, 0);
	inline static glm::vec3 blue = glm::vec3(0, 0, 1);
	inline static glm::vec3 brown = glm::vec3(77.0f / 255.0f, 17.0f / 255.0f, 10.0f / 255.0f);
	inline static glm::vec3 gold = glm::vec3(152.0f / 255.0f, 93.0f / 255.0f, 15.0f / 255.0f);
};
#pragma once
#include <glm/glm.hpp>
#include <string>
#include "Texture.hpp"

class Shape;
class ShaderProgram;

class SkySphere
{
public:

	void Initialize(std::string fileName);
	void Update(glm::mat4 proj, glm::mat4 view);

private:
	std::string rootPath = "textures/";
	Texture texture;
	Shape* sphere;
	ShaderProgram* shader;
};
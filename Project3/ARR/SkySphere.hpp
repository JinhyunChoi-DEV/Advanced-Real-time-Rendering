#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "Texture.hpp"

class Shape;
class ShaderProgram;

class SkySphere
{
public:

	void Initialize(std::string fileName);
	void Update(glm::mat4 proj, glm::mat4 view);
	Texture texture;
	unsigned int irradMapBuffer;

private:
	void InitIrradianceMap();
	void CreateIrradianceMap();
	std::vector<glm::vec3> shCoefficients;
	std::vector<float> SHCoefficient(const glm::vec3& N);
	std::string rootPath = "textures/";
	Shape* sphere;
	ShaderProgram* shader;
};
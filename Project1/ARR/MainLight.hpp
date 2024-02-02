#pragma once

#include "LightType.hpp"

class ShaderProgram;
class Object;

class MainLight
{
public:
	~MainLight();

	LightType type;
	glm::vec3 direction;
	glm::vec3 position;
	glm::vec3 scale;
	glm::vec3 attenuationConstant;
	float innerAngle;
	float outerAngle;
	float fallOut;

	bool IsDraw;
	glm::vec3 solidObjectColor;

	void Initialize();
	void UpdateLightData(int programId);
	void DrawObject(glm::mat4 proj, glm::mat4 view);

private:
	ShaderProgram* solidShader;
	Object* obj;
};
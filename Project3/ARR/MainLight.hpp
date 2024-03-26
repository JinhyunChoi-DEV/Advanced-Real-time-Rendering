#pragma once

class ShaderProgram;
class Object;

class MainLight
{
public:
	~MainLight();

	glm::vec3 position;
	glm::vec3 color;
	glm::vec3 scale;

	bool IsDraw;
	glm::vec3 solidObjectColor;

	void Initialize();
	void UpdateLightData(int programId);
	void DrawObject(glm::mat4 proj, glm::mat4 view);

private:
	ShaderProgram* solidShader;
	Object* obj;
};
#pragma once
#include <vector>
#include <glm/glm.hpp>


class Shape;
class Object;
class ShaderProgram;

class LocalLight
{
public:
	glm::vec3 position;
	glm::vec3 color;
	float range;
	Shape* shape;
};

struct Light
{
public:
	Light() = default;

	glm::vec4 position;
	glm::vec4 color;
};

struct LocalLightData
{
	unsigned int count;
	std::vector<Light> lights;
};

class LocalLights
{
public:
	//LocalLights(ShaderProgram* shader);
	~LocalLights();

	void Initialize(ShaderProgram* shader);
	void UpdateSSBO();
	void Update(glm::mat4 proj, glm::mat4 view, glm::vec2 screenSize, bool usePBR);
	void Add(glm::vec3 pos, glm::vec3 color, float range);
	void Delete();

	bool IsDrawSphere;
	unsigned int activeCount;
	std::vector<LocalLight*> lights;

private:

	unsigned int countSize = 16;
	unsigned int structSize = 0;
	void UpdateSSBOSize();
	unsigned ssbo;
	Shape* shape;
	ShaderProgram* shader;
	ShaderProgram* solidShader;
	LocalLightData data;
};

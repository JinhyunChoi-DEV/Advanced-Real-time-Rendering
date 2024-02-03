#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "GBO.hpp"
#include "LocalLights.hpp"
#include "MainLight.hpp"
#include "SkySphere.hpp"

class ObjectInstance;
class ShaderProgram;
class Object;
class Shape;

enum DrawMode
{
	Total, WorldPos, Normal, Diffuse, Specular
};

class SampleScene
{
public:
	SampleScene(GLFWwindow* window);
	~SampleScene();
	void Initialize();
	void Update();
	void Draw();
	void DrawGUI();

	bool isQuit = false;

private:
	void CreateShader();
	void CreateObjects();

	void UpdateSpin();
	void UpdateWindowSize();
	void UpdateTransform();
	void UpdateLightPosition(float deltaTime);
	void UpdateGBuffer();

	glm::vec3 GetRandPos(int min, int max);
	glm::vec3 GetRandColor();
	int GetRandRange(int min, int max);


	glm::vec2 screenSize;
	float spin, tilt, speed, ry, front, back;
	float lastTime;
	glm::vec3 tr;
	glm::mat4 proj, worldView, worldInverse;
	Object* bunny1, * bunny2, * bunny3, * bunny4, * bunny5, * table;
	GBO gbo;
	MainLight mainLight;
	LocalLights localLights;
	SkySphere sky;
	unsigned int activeLightCount;

	glm::vec2 prevMousePosition;
	unsigned int uboLight;
	int lightGUIWidth;

	GLFWwindow* window;
	ObjectInstance* models;
	ShaderProgram* gBufferShader, * deferredShader, * localLightShader;
	std::vector<glm::vec3> movePosition;
	std::vector<bool> skipToMove;

	Shape* fsq;
	DrawMode mode;
};

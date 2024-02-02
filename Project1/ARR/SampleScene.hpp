#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "GBO.hpp"
#include "LocalLights.hpp"
#include "MainLight.hpp"

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
	void UpdateGBuffer();


	glm::vec2 screenSize;
	float spin, tilt, speed, ry, front, back;
	glm::vec3 tr;
	glm::mat4 proj, worldView, worldInverse;
	Object* bunny1, * bunny2, * bunny3, * table;
	GBO gbo;
	MainLight mainLight;
	LocalLights localLights;
	unsigned int activeLightCount;

	glm::vec2 prevMousePosition;
	unsigned int uboLight;
	int lightGUIWidth;

	GLFWwindow* window;
	ObjectInstance* models;
	ShaderProgram* gBufferShader, * deferredShader, * localLightShader;

	Shape* fsq;
	DrawMode mode;
};

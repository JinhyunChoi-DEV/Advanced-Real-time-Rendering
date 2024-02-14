#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "GBO.hpp"
#include "FBO.hpp"
#include "LocalLights.hpp"
#include "MainLight.hpp"
#include "SkySphere.hpp"

class ObjectInstance;
class ShaderProgram;
class Object;
class Shape;

enum DrawMode
{
	Total, WorldPos, Normal, Diffuse, Specular, ShadowMap
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

	void UpdateGaussianWeight();
	void UpdateSpin();
	void UpdateWindowSize();
	void UpdateTransform();
	void UpdateLightPosition(float deltaTime);
	void UpdateGBuffer();
	void UpdateShadow();
	void UpdateBlur();

	glm::vec3 GetRandPos(int min, int max);
	glm::vec3 GetRandColor();
	glm::vec3 GetUpVector(glm::vec3 refV);
	int GetRandRange(int min, int max);


	glm::vec2 screenSize;
	float spin, tilt, speed, ry, front, back;
	float lastTime;
	glm::vec3 tr;
	glm::vec3 upVector;
	glm::mat4 proj, worldView, worldInverse, shadowMatrix;
	Object* bunny1, * bunny2, * bunny3, * bunny4, * bunny5, * table;
	Object* quad;
	GBO gbo;
	FBO shadowFBO;
	MainLight mainLight;
	LocalLights localLights;
	SkySphere sky;
	unsigned int activeLightCount;
	int blurWidth;
	unsigned int blurUBO;

	glm::vec2 prevMousePosition;
	unsigned int uboLight;
	int lightGUIWidth;
	std::vector<float> gaussianWeights;

	float alpha;
	float far = 1000;
	bool useBlur = true;
	const int MAX_BLUR_WIDTH = 50;

	GLFWwindow* window;
	ObjectInstance* models;
	ShaderProgram* gBufferShader, * lightStageShader, * localLightShader, * shadowShader, * shadowTestShader, * computeVerticalShader, * computeHorizontalShader;
	std::vector<glm::vec3> movePosition;
	std::vector<bool> skipToMove;

	Shape* fsq;
	DrawMode mode;
};

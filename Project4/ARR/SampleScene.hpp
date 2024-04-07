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
	PBR, Phong, WorldPos, Normal, Diffuse, Specular, ShadowMap
};

struct Block
{
	int N;
	float* hammersley;
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
	void InitHammersleyBlock();

	void UpdateGaussianWeight();
	void UpdateSpin();
	void UpdateWindowSize();
	void UpdateTransform();
	void UpdateLightPosition(float deltaTime);
	void UpdateGBuffer();
	void UpdateShadow();
	void UpdateBlur();
	void UpdateAO();
	void UpdateAOBlur();

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
	FBO aoFBO;
	MainLight mainLight;
	LocalLights localLights;
	SkySphere sky;
	Block block;
	unsigned int activeLightCount;
	int blurWidth;
	unsigned int blurUBO;
	unsigned int blurUBO_AO;

	glm::vec2 prevMousePosition;
	unsigned int uboLight;
	int lightGUIWidth;
	std::vector<float> gaussianWeights;

	float alpha;
	float far = 1000;
	float exposureControl;
	bool useBlur = true;
	const int MAX_BLUR_WIDTH = 50;
	unsigned int id, bindPoint;
	int n = 20;
	float R = 0.5f;
	float s = 1.0f;
	float k = 1.0f;
	float s_AO = 0.01;
	bool drawOnlyAO = false;

	GLFWwindow* window;
	ObjectInstance* models;
	ShaderProgram* gBufferShader, * lightStageShader, * localLightShader, * shadowShader, * ambientOcclusionShader,
		* computeVerticalShader, * computeHorizontalShader, * computeVerticalAOShader, * computeHorizontalAOShader;
	std::vector<glm::vec3> movePosition;
	std::vector<bool> skipToMove;

	Shape* fsq;
	Shape* ambientFSQ;
	DrawMode mode;
};

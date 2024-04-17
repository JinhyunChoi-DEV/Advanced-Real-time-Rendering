#define GLFW_INCLUDE_NONE
#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
#include <glm/gtc/matrix_transform.hpp>
using namespace gl;

#include "Color.hpp"
#include "Shape.hpp"
#include "Transform.hpp"
#include "Object.hpp"

#include "SampleScene.hpp"
#include "ObjectInstance.hpp"

#include <iostream>

#include "Random.hpp"
#include "Input.hpp"
#include "Shader.hpp"

const float PI = 3.14159f;
const float rad = PI / 180.0f;

SampleScene::SampleScene(GLFWwindow* window)
{
	this->window = window;
}

SampleScene::~SampleScene()
{
	window = nullptr;

	delete models;
	delete gBufferShader;
	delete lightStageShader;
	delete localLightShader;
}

void SampleScene::Initialize()
{
	mode = PBR;
	spin = 0.0;
	tilt = 30.0;
	tr = glm::vec3(0.0, 0.0, 50.0);
	speed = 300.0 / 30.0;
	ry = 0.4;
	front = 0.5;
	back = 1000;
	activeLightCount = 0;
	UpdateWindowSize();
	lightGUIWidth = screenSize.x / 5.0f;
	lastTime = glfwGetTime();
	alpha = 0.001;
	exposureControl = 0.5f;
	blurWidth = 7;
	k = 1.0f;
	s = 1.0f;
	R = 0.5f;
	n = 15;
	gaussianWeights = std::vector<float>(2 * MAX_BLUR_WIDTH + sizeof(float), 0.0f);

	models = new ObjectInstance();
	InitHammersleyBlock();
	CreateShader();

	gbo.Create(screenSize.x, screenSize.y);
	shadowFBO.Create(screenSize.x, screenSize.y);
	aoFBO.Create(screenSize.x, screenSize.y);

	localLights.Initialize(localLightShader);
	mainLight.Initialize();
	mainLight.position = glm::vec3(0, 0, 40);
	sky.Initialize("background.hdr");
	CreateObjects();


	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
}

void SampleScene::Update()
{
	float currentTime = glfwGetTime();
	float deltaTime = currentTime - lastTime;
	UpdateWindowSize();
	UpdateSpin();
	UpdateLightPosition(deltaTime);
	UpdateGaussianWeight();

	if (INPUT->KeyTriggered(GLFW_KEY_ESCAPE))
		isQuit = true;

	lastTime = currentTime;
}

void SampleScene::Draw()
{
	UpdateTransform();
	UpdateGBuffer();
	UpdateShadow();
	UpdateBlur();
	UpdateAO();
	UpdateAOBlur();

	glViewport(0, 0, screenSize.x, screenSize.y);
	glClearColor(1, 1, 1, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	int loc, programId;

	// --------------------- Draw SkyBox --------------------- //
	if(npSelected != 1)
		sky.Update(proj, worldView);
	// ------------------------------------------------------- //


	// --------------------- Draw Deferred ------------------- //
	programId = lightStageShader->programId;
	lightStageShader->UseShader();
	mainLight.UpdateLightData(programId);
	loc = glGetUniformLocation(programId, "WorldInverse");
	glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(worldInverse));
	loc = glGetUniformLocation(programId, "mode");
	glUniform1ui(loc, mode);
	loc = glGetUniformLocation(programId, "screenSize");
	glUniform2fv(loc, 1, &screenSize[0]);
	loc = glGetUniformLocation(programId, "shadowMatrix");
	glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(shadowMatrix));
	loc = glGetUniformLocation(programId, "alpha");
	glUniform1f(loc, alpha);
	loc = glGetUniformLocation(programId, "near");
	glUniform1f(loc, front);
	loc = glGetUniformLocation(programId, "far");
	glUniform1f(loc, far);
	loc = glGetUniformLocation(programId, "exposureControl");
	glUniform1f(loc, exposureControl);
	loc = glGetUniformBlockIndex(programId, "HammersleyBlock");
	glUniformBlockBinding(programId, loc, bindPoint);

	loc = glGetUniformLocation(programId, "backgroundTexture");
	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_2D, sky.texture.bufferObj);
	glUniform1i(loc, 10);

	loc = glGetUniformLocation(programId, "irradMap");
	glActiveTexture(GL_TEXTURE11);
	glBindTexture(GL_TEXTURE_2D, sky.irradMapBuffer);
	glUniform1i(loc, 11);

	loc = glGetUniformLocation(programId, "onlyAO");
	glUniform1i(loc, drawOnlyAO);

	loc = glGetUniformLocation(programId, "threshold");
	glUniform1f(loc, threshold);

	loc = glGetUniformLocation(programId, "npSelected");
	glUniform1i(loc, npSelected);

	gbo.BindTexture(0, programId, "gWorldPosition", 0);
	gbo.BindTexture(1, programId, "gNormalVector", 1);
	gbo.BindTexture(2, programId, "gDiffuse", 2);
	gbo.BindTexture(3, programId, "gSpecular", 3);
	gbo.BindTexture(4, programId, "gDepth", 4);
	gbo.BindTexture(5, programId, "gDeviceDepth", 5);
	shadowFBO.BindTexture(7, programId, "shadowMap");
	aoFBO.BindTexture(8, programId, "aoMap");

	fsq->DrawVAO();
	lightStageShader->UnuseShader();
	// ----------------------------------------------------- //

	gbo.CopyDepthBuffer();

	// ---------------- Draw Solid Objects ---------------- //
	//mainLight.DrawObject(proj, worldView);
	// ---------------------------------------------------- //
	 
	// ----------------- Draw Local Lights ---------------- //
	//localLights.UpdateSSBO();
	//localLights.Update(proj, worldView, screenSize, mode == PBR);
	//----------------------------------------------------- //
}

void SampleScene::DrawGUI()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// -------------------- Draw Type Panel -------------------- //
	ImVec2 menubarSize;
	if (ImGui::BeginMainMenuBar()) {
		menubarSize = ImGui::GetWindowSize();
		if (ImGui::BeginMenu("Draw Mode"))
		{
			if (ImGui::MenuItem("Draw PBR Result", "", mode == PBR)) { mode = PBR; }

			if (ImGui::MenuItem("Draw Phong Result", "", mode == Phong)) { mode = Phong; }

			if (ImGui::MenuItem("Draw World Position", "", mode == WorldPos)) { mode = WorldPos; }

			if (ImGui::MenuItem("Draw Vertex Normal", "", mode == Normal)) { mode = Normal; }
			 
			if (ImGui::MenuItem("Draw Diffuse", "", mode == Diffuse)) { mode = Diffuse; }

			if (ImGui::MenuItem("Draw Specular", "", mode == Specular)) { mode = Specular; }

			if (ImGui::MenuItem("Draw ShadowMap", "", mode == ShadowMap)) { mode = ShadowMap; }

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
	// --------------------------------------------------------- //


	// --------------- Light Panel -------------------------- //
	ImVec2 pos(0, menubarSize.y);
	ImGui::SetNextWindowPos(pos);
	ImVec2 size(lightGUIWidth, screenSize.y - menubarSize.y);
	ImGui::SetNextWindowSize(size);
	ImGui::Begin("Light Panel");
	lightGUIWidth = ImGui::GetWindowSize().x;

	if (ImGui::CollapsingHeader("Main Light Info"))
	{
		// ------------------ Position --------------------- //
		ImGui::Text("Position");
		ImGui::DragFloat("X Position", &mainLight.position.x, 0.1f, -500.0f, 500.0f);
		ImGui::DragFloat("Y Position", &mainLight.position.y, 0.1f, -500.0f, 500.0f);
		ImGui::DragFloat("Z Position", &mainLight.position.z, 0.1f, -500.0f, 500.0f);
		ImGui::NewLine();
		// ------------------------------------------------- //


		// ------------------ Light Color --------------------- //
		ImGui::Text("Color");
		ImGui::ColorEdit3("Light Color", &mainLight.color[0]);
		ImGui::NewLine();
		// ------------------------------------------------- //
	}

	if (ImGui::CollapsingHeader("Local Lights Info"))
	{
		ImGui::Checkbox("Draw Light Area", &localLights.IsDrawSphere);

		ImGui::NewLine();
		std::string countLights = "Active Light Count:";
		countLights += std::to_string(localLights.activeCount);
		ImGui::Text(countLights.c_str());

		if (ImGui::Button("Add Light"))
		{
			glm::vec3 pos = GetRandPos(-10, 10);
			glm::vec3 color = GetRandColor();
			float range = GetRandRange(5, 12);

			localLights.Add(pos, color, (float)range);

			pos = GetRandPos(-20, 20);
			movePosition.push_back(pos);
			skipToMove.push_back(false);
		}
		ImGui::SameLine();
		if (ImGui::Button("Delete Light"))
		{
			localLights.Delete();

			if (movePosition.size() > 0)
			{
				movePosition.erase(movePosition.end() - 1);
				skipToMove.erase(skipToMove.end() - 1);
			}
		}
	}
	// ------------------------------------------------------ //

	// ------------------------ Shadow Blur ------------------------ //
	if (ImGui::CollapsingHeader("Shdaow & Blur"))
	{
		ImGui::DragInt("Blur Width", &blurWidth, 0.1f, 1, MAX_BLUR_WIDTH);
		ImGui::Checkbox("Active Blur", &useBlur);
	}
	// ------------------------------------------------------------- //

	if (ImGui::CollapsingHeader("PBR Setting Panel"))
	{
		ImGui::DragFloat("Bunny_1 Shininess", &bunny1->shininess, 0.005f, 0.0f, 1.0f);
		ImGui::DragFloat("Bunny_2 Shininess", &bunny2->shininess, 0.005f, 0.0f, 1.0f);
		ImGui::DragFloat("Bunny_3 Shininess", &bunny3->shininess, 0.005f, 0.0f, 1.0f);
		ImGui::DragFloat("Bunny_4 Shininess", &bunny4->shininess, 0.005f, 0.0f, 1.0f);
		ImGui::DragFloat("Bunny_5 Shininess", &bunny5->shininess, 0.005f, 0.0f, 1.0f);
		ImGui::DragFloat("Table Shininess", &table->shininess, 0.005f, 0.0f, 1.0f);
		ImGui::DragFloat("Exposure Value", &exposureControl, 0.1f, 0.1f, 10000.0f);
	}

	if (ImGui::CollapsingHeader("Ambient Occlusion Panel"))
	{
		ImGui::Checkbox("Draw Only AO", &drawOnlyAO);
		ImGui::DragInt("N count", &n, 0.5f, 10, 30);
		ImGui::DragFloat("R value", &R, 0.005f, 0.1f, 50.0f);
		ImGui::DragFloat("K value", &k, 0.005f, 0.1f, 10.0f);
		ImGui::DragFloat("s Value", &s, 0.005f, 0.1f, 10.0f);
		ImGui::Checkbox("Active Blur", &useBlur);
		ImGui::DragInt("Blur Width", &blurWidth, 0.1f, 1, MAX_BLUR_WIDTH);
		ImGui::DragFloat("s_AO Value", &s_AO, 0.005f, 0.01f, 10.0f);
	}

	if (ImGui::CollapsingHeader("Non-Photorealistic Panel"))
	{
		if (ImGui::RadioButton("InActive Non-Photorealistic", npSelected == 0)) npSelected = 0;
		if (ImGui::RadioButton("Active Only Non-Photorealistic", npSelected == 1)) npSelected = 1;
		if (ImGui::RadioButton("Combine Non-Photorealistic + IBL", npSelected == 2)) npSelected = 2;

		ImGui::DragFloat("Threshold Value", &threshold, 0.005f, 0.0f, 1.0f);
		ImGui::NewLine();


		ImGui::Checkbox("Active Non-Photorealistic Noise", &activeNPNoise);
		ImGui::DragFloat("Noise Intensity", &intensity, 0.00001f, 0.0f, 0.1f, "%.6f");
	}


	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void SampleScene::CreateObjects()
{
	// -------------------- Create Poly And Object -------------------- //
	//Shape* BunnyPoly = new Obj("bunny_a.obj");
	Shape* BunnyPoly = new Obj("bunny.obj");
	//Shape* BunnyPoly = new Obj("sphere.obj");
	Shape* BoxP = new Box();

	fsq = new Quad(1);
	ambientFSQ = new Quad(1);

	bunny1 = new Object(BunnyPoly, Color::white, Color::white, 0.1f);
	bunny2 = new Object(BunnyPoly, Color::red, Color::white, 0);
	bunny3 = new Object(BunnyPoly, Color::blue, Color::white, 0);
	bunny4 = new Object(BunnyPoly, Color::green, Color::white, 0);
	bunny5 = new Object(BunnyPoly, Color::pink, Color::white, 0);
	table = new Object(BoxP, Color::white, Color::white, 0.5f);

	/*
	// center
	bunny1->transform = Translate(0, 0, 2) * Scale(1.5, 1.5, 1.5) * Rotate(0, 90);
	// right
	bunny2->transform = Translate(12, 0, 2) * Scale(1.5, 1.5, 1.5) * Rotate(0, 90) * Rotate(1, 90);
	// left
	bunny3->transform = Translate(-12, 0, 2) * Scale(1.5, 1.5, 1.5) * Rotate(0, 90) * Rotate(1, -90);
	// back
	bunny4->transform = Translate(0, 12, 2) * Scale(1.5, 1.5, 1.5) * Rotate(0, 90);
	// top
	bunny5->transform = Translate(0, -12, 2) * Scale(1.5, 1.5, 1.5) * Rotate(0, 90) * Rotate(1, -180);
	*/

	bunny1->transform = Translate(0, 0, -0.6) * Scale(45) * Rotate(0, 90);
	bunny2->transform = Translate(12, 0, -0.6) * Scale(45) * Rotate(0, 90) * Rotate(1, 90);
	bunny3->transform = Translate(-12, 0, -0.6) * Scale(45) * Rotate(0, 90) * Rotate(1, -90);
	bunny4->transform = Translate(0, 12, -0.6) * Scale(45) * Rotate(0, 90);
	bunny5->transform = Translate(0, -12, -0.6) * Scale(45) * Rotate(0, 90) * Rotate(1, -180);

	/*
	bunny1->transform = Translate(0, 0, 10) * Scale(5);
	bunny2->transform = Translate(20, 0, 10) * Scale(5);
	bunny3->transform = Translate(-20, 0, 10) * Scale(5);
	bunny4->transform = Translate(0, 20, 10) * Scale(5);
	bunny5->transform = Translate(0, -20, 10) * Scale(5);
	*/

	table->transform = Translate(0, 0, 0.5) * Scale(30, 30, 0.25f);

	models->Add(bunny1);
	models->Add(bunny2);
	models->Add(bunny3);
	models->Add(bunny4);
	models->Add(bunny5);
	models->Add(table);
	// ---------------------------------------------------------------- //


	// -------------------- Create Local Lights ---------------------- //
	for (int i = 0; i < activeLightCount; ++i)
	{
		glm::vec3 pos = GetRandPos(-20, 20);
		glm::vec3 color = GetRandColor();
		float range = GetRandRange(5, 15);

		localLights.Add(pos, color, (float)range);

		pos = GetRandPos(-20, 20);
		movePosition.push_back(pos);
		skipToMove.push_back(false);
	}

	// --------------------------------------------------------------- //


	Shape* QuadPolygons = new Quad();
	quad = new Object(QuadPolygons); 
}

void SampleScene::InitHammersleyBlock()
{
	block.N = 30;
	block.hammersley = new float[2 * block.N];
	int kk;
	int pos = 0;
	for (int k = 0; k < block.N; ++k)
	{
		kk = k;
		float u = 0.0f; 
		for (float p = 0.5f; kk; p *= 0.5f, kk >>= 1)
		{
			if (kk & 1)
				u += p;
		}

		float v = (k + 0.5f) / block.N;
		block.hammersley[pos++] = u;
		block.hammersley[pos++] = v;
	}

	glGenBuffers(1, &id);
	bindPoint = 8;
	glBindBufferBase(GL_UNIFORM_BUFFER, bindPoint, id);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(int)*4 + sizeof(float) * 100 * 2, &block, GL_STATIC_DRAW);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(int), sizeof(float) * 100 * 2, block.hammersley);
}

void SampleScene::UpdateGaussianWeight() 
{
	for (int i = 0; i < gaussianWeights.size(); ++i)
	{
		gaussianWeights[i] = 0.0f;
	}

	float s = blurWidth / 2.0f;
	float sum = 0;
	for (int i = 0; i <= blurWidth * 2 + 1; ++i)
	{
		gaussianWeights[i] = exp(-pow(i - blurWidth, 2) / (2.0f * s * s));
		sum += gaussianWeights[i];
	}

	for (int i = 0; i <= blurWidth * 2 + 1; ++i)
	{
		gaussianWeights[i] /= sum;
	}
}


void SampleScene::CreateShader()
{
	// ----------------------- GBuffer ------------------------------ // 
	gBufferShader = new ShaderProgram();
	gBufferShader->AddShader("gBuffer.vert", GL_VERTEX_SHADER);
	gBufferShader->AddShader("gBuffer.frag", GL_FRAGMENT_SHADER);

	glBindAttribLocation(gBufferShader->programId, 0, "inVertex");
	glBindAttribLocation(gBufferShader->programId, 1, "inNormal");
	glBindAttribLocation(gBufferShader->programId, 2, "inTexture");
	glBindAttribLocation(gBufferShader->programId, 3, "inTangent");
	gBufferShader->LinkProgram();
	// -------------------------------------------------------------- // 


	// ----------------------- lightStage ----------------------------- // 
	lightStageShader = new ShaderProgram();
	lightStageShader->AddShader("lightStage.vert", GL_VERTEX_SHADER);
	lightStageShader->AddShader("lightStage.frag", GL_FRAGMENT_SHADER);
	glBindAttribLocation(lightStageShader->programId, 0, "inVertex");

	lightStageShader->LinkProgram();
	// ------------------------------------------------------------- //


	// ---------------------- Local Light -------------------------- //
	localLightShader = new ShaderProgram();
	localLightShader->AddShader("localLights.vert", GL_VERTEX_SHADER);
	localLightShader->AddShader("localLights.frag", GL_FRAGMENT_SHADER);
	glBindAttribLocation(localLightShader->programId, 0, "inVertex");

	localLightShader->LinkProgram();
	// ------------------------------------------------------------- //

	// --------------------------- Shadow ----------------------------- //
	shadowShader = new ShaderProgram();
	shadowShader->AddShader("shadow.vert", GL_VERTEX_SHADER);
	shadowShader->AddShader("shadow.frag", GL_FRAGMENT_SHADER);
	glBindAttribLocation(shadowShader->programId, 0, "inVertex");
	glBindAttribLocation(shadowShader->programId, 1, "inNormal");

	shadowShader->LinkProgram();
	// ---------------------------------------------------------------- //


	// ----------------------- Ambient Occlusion ---------------------- //
	ambientOcclusionShader = new ShaderProgram();
	ambientOcclusionShader->AddShader("ambientOcclusion.vert", GL_VERTEX_SHADER);
	ambientOcclusionShader->AddShader("ambientOcclusion.frag", GL_FRAGMENT_SHADER);
	glBindAttribLocation(ambientOcclusionShader->programId, 0, "inVertex");

	ambientOcclusionShader->LinkProgram();
	// ---------------------------------------------------------------- // 


	// ----------------------- Compute ----------------------- //
	glGenBuffers(1, &blurUBO);
	computeVerticalShader = new ShaderProgram();
	computeVerticalShader->AddShader("computeVertical.comp", GL_COMPUTE_SHADER);
	computeVerticalShader->LinkProgram();
	int programId = computeVerticalShader->programId;
	int loc = glGetUniformBlockIndex(programId, "blurKernel");
	glUniformBlockBinding(programId, loc, 0);
	glBindBuffer(GL_UNIFORM_BUFFER, blurUBO);
	glBufferData(GL_UNIFORM_BUFFER, gaussianWeights.size() * sizeof(float), gaussianWeights.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, blurUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	computeHorizontalShader = new ShaderProgram();
	computeHorizontalShader->AddShader("computeHorizontal.comp", GL_COMPUTE_SHADER);
	computeHorizontalShader->LinkProgram();
	programId = computeHorizontalShader->programId;
	loc = glGetUniformLocation(programId, "blurKernel");
	glUniformBlockBinding(programId, loc, 1);
	glBindBuffer(GL_UNIFORM_BUFFER, blurUBO);
	glBufferData(GL_UNIFORM_BUFFER, gaussianWeights.size() * sizeof(float), gaussianWeights.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, blurUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glGenBuffers(1, &blurUBO_AO);
	computeVerticalAOShader = new ShaderProgram();
	computeVerticalAOShader->AddShader("computeAOVertical.comp", GL_COMPUTE_SHADER);
	computeVerticalAOShader->LinkProgram();
	programId = computeVerticalAOShader->programId;
	loc = glGetUniformLocation(programId, "blurKernel");
	glUniformBlockBinding(programId, loc, 0);
	glBindBuffer(GL_UNIFORM_BUFFER, blurUBO_AO);
	glBufferData(GL_UNIFORM_BUFFER, gaussianWeights.size() * sizeof(float), gaussianWeights.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, blurUBO_AO);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	computeHorizontalAOShader = new ShaderProgram();
	computeHorizontalAOShader->AddShader("computeAOHorizontal.comp", GL_COMPUTE_SHADER);
	computeHorizontalAOShader->LinkProgram();
	programId = computeHorizontalAOShader->programId;
	loc = glGetUniformLocation(programId, "blurKernel");
	glUniformBlockBinding(programId, loc, 1);
	glBindBuffer(GL_UNIFORM_BUFFER, blurUBO_AO);
	glBufferData(GL_UNIFORM_BUFFER, gaussianWeights.size() * sizeof(float), gaussianWeights.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, blurUBO_AO);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	// ------------------------------------------------------------- //
}

void SampleScene::UpdateWindowSize()
{
	int w, h;
	glfwGetWindowSize(window, &w, &h);

	screenSize.x = w;
	screenSize.y = h;
}

void SampleScene::UpdateSpin()
{
	if (ImGui::GetIO().WantCaptureKeyboard || ImGui::GetIO().WantCaptureMouse) return;

	glm::vec2 curMouse = INPUT->mousePosition;

	glm::vec2 delta = curMouse - prevMousePosition;
	int dx = delta.x;
	int dy = delta.y;
	float y = INPUT->scrollValue.y;

	if (INPUT->MouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT) && !INPUT->KeyPressed(GLFW_KEY_LEFT_SHIFT))
	{
		spin += dx / 3.0f;
		tilt += dy / 3.0f;
	}

	if (INPUT->MouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT) && !INPUT->KeyPressed(GLFW_KEY_LEFT_SHIFT))
	{
		tr[0] += dx / 40.0f;
		tr[1] -= dy / 40.0f;
	}

	if (y > 0.0f)
	{
		tr[2] = pow(tr[2], 1.0f / 1.02f);
	}
	else if (y < 0.0f)
	{
		tr[2] = pow(tr[2], 1.02f);
	}

	prevMousePosition = curMouse;
}


void SampleScene::UpdateTransform()
{
	worldView = Translate(tr[0], tr[1], -tr[2]) * Rotate(0, tilt - 90) * Rotate(2, spin);
	proj = Perspective((ry * screenSize.x) / screenSize.y, ry, front, back);
	worldInverse = glm::inverse(worldView);
}

void SampleScene::UpdateLightPosition(float deltaTime)
{
	for (int i = 0; i < localLights.activeCount; ++i)
	{
		auto light = localLights.lights[i];

		float distance = glm::length(light->position - movePosition[i]);

		if (skipToMove[i] && distance > 1.0f)
		{
			skipToMove[i] = false;
			return;
		}

		if (!skipToMove[i] && distance <= 0.9f)
		{
			skipToMove[i] = true;
			movePosition[i] = GetRandPos(-20, 20);
		}

		auto dir = glm::normalize(movePosition[i] - light->position);
		float moveSpeed = 5 * deltaTime;

		light->position += dir * moveSpeed;
	}
}

void SampleScene::UpdateGBuffer()
{
	gbo.Bind();

	glClearColor(0, 0, 0, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	int loc, programId;
	gBufferShader->UseShader();
	programId = gBufferShader->programId;

	loc = glGetUniformLocation(programId, "Projection");
	glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(proj));
	loc = glGetUniformLocation(programId, "WorldView");
	glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(worldView));
	loc = glGetUniformLocation(programId, "near");
	glUniform1f(loc, front);
	loc = glGetUniformLocation(programId, "far");
	glUniform1f(loc, far);
	loc = glGetUniformLocation(programId, "npSelected");
	glUniform1i(loc, npSelected);
	loc = glGetUniformLocation(programId, "activeNoise");
	glUniform1i(loc, activeNPNoise);
	loc = glGetUniformLocation(programId, "intensity");
	glUniform1f(loc, intensity);

	models->Draw(gBufferShader);

	gBufferShader->UnuseShader();
	gbo.Unbind();
}

void SampleScene::UpdateShadow()
{
	shadowFBO.Bind();
	glViewport(0, 0, screenSize.x, screenSize.y);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	shadowShader->UseShader();
	auto dir = glm::normalize(glm::vec3(0, -1, 0));
	upVector = GetUpVector(dir);

	int loc, programId;
	programId = shadowShader->programId;
	auto lightView = glm::lookAt(mainLight.position, dir, glm::vec3(0, 1, 0));
	auto lightProj = glm::perspective(glm::radians(90.f), 1.f, front, far);
	shadowMatrix = lightProj * lightView;

	loc = glGetUniformLocation(programId, "Projection");
	glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(lightProj));

	loc = glGetUniformLocation(programId, "LightView");
	glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(lightView));

	loc = glGetUniformLocation(programId, "near");
	glUniform1f(loc, front);

	loc = glGetUniformLocation(programId, "far");
	glUniform1f(loc, far);

	models->Draw(shadowShader);

	shadowShader->UnuseShader();
	shadowFBO.UnBind();
}

void SampleScene::UpdateBlur()
{
	if (!useBlur)
		return;

	int loc, programId;

	computeVerticalShader->UseShader();
	programId = computeVerticalShader->programId;
	loc = glGetUniformLocation(programId, "blurKernel");
	glUniformBlockBinding(programId, loc, 0);
	glBindBuffer(GL_UNIFORM_BUFFER, blurUBO);
	glBufferData(GL_UNIFORM_BUFFER, gaussianWeights.size() * sizeof(float), gaussianWeights.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, blurUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glDispatchCompute((int)screenSize.x, (int)screenSize.y / 128, 1);
	loc = glGetUniformLocation(programId, "src");
	glBindImageTexture(0, shadowFBO.textureID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	glUniform1i(loc, 0);
	loc = glGetUniformLocation(programId, "dst");
	glBindImageTexture(1, shadowFBO.textureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glUniform1i(loc, 1);
	loc = glGetUniformLocation(programId, "blurWidth");
	glUniform1i(loc, blurWidth);
	computeVerticalShader->UnuseShader();

	computeHorizontalShader->UseShader();
	programId = computeHorizontalShader->programId;
	loc = glGetUniformLocation(programId, "blurKernel");
	glUniformBlockBinding(programId, loc, 1);
	glBindBuffer(GL_UNIFORM_BUFFER, blurUBO);
	glBufferData(GL_UNIFORM_BUFFER, gaussianWeights.size() * sizeof(float), gaussianWeights.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, blurUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glDispatchCompute((int)screenSize.x / 128, (int)screenSize.y, 1);
	loc = glGetUniformLocation(programId, "src");
	glBindImageTexture(0, shadowFBO.textureID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	glUniform1i(loc, 0);
	loc = glGetUniformLocation(programId, "dst");
	glBindImageTexture(1, shadowFBO.textureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glUniform1i(loc, 1);
	loc = glGetUniformLocation(programId, "blurWidth");
	glUniform1i(loc, blurWidth);
	computeHorizontalShader->UnuseShader();
}

void SampleScene::UpdateAO()
{
	aoFBO.Bind();
	glViewport(0, 0, screenSize.x, screenSize.y);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	int programId, loc;
	programId = ambientOcclusionShader->programId;
	ambientOcclusionShader->UseShader();
	loc = glGetUniformLocation(programId, "WorldInverse");
	glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(worldInverse));
	loc = glGetUniformLocation(programId, "R");
	glUniform1f(loc, R);
	loc = glGetUniformLocation(programId, "n");
	glUniform1i(loc, n);
	loc = glGetUniformLocation(programId, "k");
	glUniform1f(loc, k);
	loc = glGetUniformLocation(programId, "s");
	glUniform1f(loc, s);
	loc = glGetUniformLocation(programId, "screenSize");
	glUniform2fv(loc, 1, &screenSize[0]);

	gbo.BindTexture(0, programId, "gWorldPosition", 0);
	gbo.BindTexture(1, programId, "gNormalVector", 1);
	gbo.BindTexture(2, programId, "gDiffuse", 2);
	gbo.BindTexture(3, programId, "gSpecular", 3);
	gbo.BindTexture(4, programId, "gDepth", 4);

	ambientFSQ->DrawVAO();
	ambientOcclusionShader->UnuseShader();
	aoFBO.UnBind();
}

void SampleScene::UpdateAOBlur()
{
	if (!useBlur)
		return;

	int loc, programId;

	computeVerticalAOShader->UseShader();
	programId = computeVerticalAOShader->programId;
	loc = glGetUniformLocation(programId, "blurKernel");
	glUniformBlockBinding(programId, loc, 0);
	glBindBuffer(GL_UNIFORM_BUFFER, blurUBO_AO);
	glBufferData(GL_UNIFORM_BUFFER, gaussianWeights.size() * sizeof(float), gaussianWeights.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, blurUBO_AO);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glDispatchCompute((int)screenSize.x, (int)screenSize.y / 128, 1);

	loc = glGetUniformLocation(programId, "src");
	glBindImageTexture(0, aoFBO.textureID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	glUniform1i(loc, 0);

	loc = glGetUniformLocation(programId, "dst");
	glBindImageTexture(1, aoFBO.textureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glUniform1i(loc, 1);

	loc = glGetUniformLocation(programId, "gNormalVector");
	glBindImageTexture(2, gbo.textureID[1], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	glUniform1i(loc, 2);

	loc = glGetUniformLocation(programId, "gDepth");
	glBindImageTexture(3, gbo.textureID[4], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	glUniform1i(loc, 3); 

	loc = glGetUniformLocation(programId, "blurWidth");
	glUniform1i(loc, blurWidth);

	loc = glGetUniformLocation(programId, "s");
	glUniform1f(loc, s_AO);
	computeVerticalAOShader->UnuseShader();

	computeHorizontalAOShader->UseShader();
	programId = computeHorizontalAOShader->programId;
	loc = glGetUniformLocation(programId, "blurKernel");
	glUniformBlockBinding(programId, loc, 1);
	glBindBuffer(GL_UNIFORM_BUFFER, blurUBO_AO);
	glBufferData(GL_UNIFORM_BUFFER, gaussianWeights.size() * sizeof(float), gaussianWeights.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, blurUBO_AO);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glDispatchCompute((int)screenSize.x/ 128, (int)screenSize.y, 1);

	loc = glGetUniformLocation(programId, "src");
	glBindImageTexture(0, aoFBO.textureID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	glUniform1i(loc, 0);

	loc = glGetUniformLocation(programId, "dst");
	glBindImageTexture(1, aoFBO.textureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glUniform1i(loc, 1);

	loc = glGetUniformLocation(programId, "gNormalVector");
	glBindImageTexture(2, gbo.textureID[1], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	glUniform1i(loc, 2);

	loc = glGetUniformLocation(programId, "gDepth");
	glBindImageTexture(3, gbo.textureID[4], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	glUniform1i(loc, 3);

	loc = glGetUniformLocation(programId, "blurWidth");
	glUniform1i(loc, blurWidth);

	loc = glGetUniformLocation(programId, "s");
	glUniform1f(loc, s_AO);
	computeHorizontalAOShader->UnuseShader(); 
}

glm::vec3 SampleScene::GetRandPos(int min, int max)
{
	int x = GetRandom(min, max);
	int y = GetRandom(min, max);
	int z = GetRandom(min, max);

	return glm::vec3(x, y, z);
}

glm::vec3 SampleScene::GetRandColor()
{
	float r = myrandomf(RNGen);
	float g = myrandomf(RNGen);
	float b = myrandomf(RNGen);

	return glm::vec3(r, g, b);
}

glm::vec3 SampleScene::GetUpVector(glm::vec3 refV)
{
	glm::vec3 arbitraryUp = glm::vec3(0.0f, 1.0f, 0.0f);

	if (glm::abs(glm::dot(refV, arbitraryUp)) > 0.999f)
		arbitraryUp = glm::vec3(0.0f, 0.0f, 1.0f);


	glm::vec3 upVector = glm::cross(refV, arbitraryUp);
	upVector = glm::cross(upVector, refV);

	upVector = glm::normalize(upVector);
	return upVector;
}

int SampleScene::GetRandRange(int min, int max)
{
	return GetRandom(min, max);
}

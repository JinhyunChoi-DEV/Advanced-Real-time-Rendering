#define GLFW_INCLUDE_NONE
#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
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
	delete deferredShader;
	delete localLightShader;
}

void SampleScene::Initialize()
{
	mode = Total;
	spin = 0.0;
	tilt = 30.0;
	tr = glm::vec3(0.0, 0.0, 25.0);
	speed = 300.0 / 30.0;
	ry = 0.4;
	front = 0.5;
	back = 5000.0;
	activeLightCount = 6;
	UpdateWindowSize();
	lightGUIWidth = screenSize.x / 5.0f;

	models = new ObjectInstance();
	CreateShader();

	gbo.CreateGBO(screenSize.x, screenSize.y);
	localLights.Initialize(localLightShader, activeLightCount);
	mainLight.Initialize();
	mainLight.position = glm::vec3(0, 0, 5);
	CreateObjects();


	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
}

void SampleScene::Update()
{
	UpdateWindowSize();
	UpdateSpin();

	if (INPUT->KeyTriggered(GLFW_KEY_ESCAPE))
		isQuit = true;
}

void SampleScene::Draw()
{
	glViewport(0, 0, screenSize.x, screenSize.y);

	UpdateTransform();
	UpdateGBuffer();

	// ------------ Draw Deferred ------------
	glClearColor(0.5, 0.5, 0.5, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	deferredShader->UseShader();
	mainLight.UpdateLightData(deferredShader->programId);
	int loc = glGetUniformLocation(deferredShader->programId, "WorldInverse");
	glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(worldInverse));
	loc = glGetUniformLocation(deferredShader->programId, "mode");
	glUniform1ui(loc, mode);
	loc = glGetUniformLocation(deferredShader->programId, "screenSize");
	glUniform2fv(loc, 1, &screenSize[0]);
	gbo.BindTexture();
	fsq->DrawVAO();
	deferredShader->UnuseShader();
	gbo.UnbindTexture();
	gbo.CopyDepthBuffer();
	// ----------------------------------------


	// ------------- Draw Local Lights ------------
	gbo.BindTexture();
	localLights.UpdateSSBO();
	localLights.Update(proj, worldView, screenSize);
	gbo.UnbindTexture();
	//---------------------------------------------


	// ----------- Draw Solid Objects -------------
	mainLight.DrawObject(proj, worldView);
	// --------------------------------------------
}

void SampleScene::DrawGUI()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::ShowDemoWindow();


	// -------------------- Draw Type Panel -------------------- //
	ImVec2 menubarSize;
	if (ImGui::BeginMainMenuBar()) {
		menubarSize = ImGui::GetWindowSize();
		if (ImGui::BeginMenu("Draw Mode"))
		{
			if (ImGui::MenuItem("Draw Total Result", "", mode == Total)) { mode = Total; }

			if (ImGui::MenuItem("Draw World Position", "", mode == WorldPos)) { mode = WorldPos; }

			if (ImGui::MenuItem("Draw Vertex Normal", "", mode == Normal)) { mode = Normal; }

			if (ImGui::MenuItem("Draw Diffuse", "", mode == Diffuse)) { mode = Diffuse; }

			if (ImGui::MenuItem("Draw Specular", "", mode == Specular)) { mode = Specular; }

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
		// --------------------  Type ------------------------ //
		ImGui::Text("Type");
		if (ImGui::RadioButton("Directional", mainLight.type == Directional)) { mainLight.type = Directional; }
		if (ImGui::RadioButton("Point", mainLight.type == Point)) { mainLight.type = Point; }
		if (ImGui::RadioButton("Spot", mainLight.type == SpotLight)) { { mainLight.type = SpotLight; } }
		ImGui::NewLine();
		// -------------------------------------------------- //

		// ------------------ Position --------------------- //
		ImGui::Text("Position");
		ImGui::DragFloat("X Position", &mainLight.position.x, 0.1f, -500.0f, 500.0f);
		ImGui::DragFloat("Y Position", &mainLight.position.y, 0.1f, -500.0f, 500.0f);
		ImGui::DragFloat("Z Position", &mainLight.position.z, 0.1f, -500.0f, 500.0f);
		ImGui::NewLine();
		// ------------------------------------------------- //


		// ------------------ Direction --------------------- //
		ImGui::Text("Direction");
		ImGui::DragFloat("X Dir", &mainLight.direction.x, 0.01f, -1.f, 1.0f);
		ImGui::DragFloat("Y Dir", &mainLight.direction.y, 0.01f, -1.f, 1.0f);
		ImGui::DragFloat("Z Dir", &mainLight.direction.z, 0.01f, -1.f, 1.0f);
		ImGui::NewLine();
		// ------------------------------------------------- //


		// ----------------- Attenuation --------------------- //
		ImGui::Text("Attenuation");
		ImGui::DragFloat3("Attenuation Constant", &mainLight.attenuationConstant[0], 0.01f, 0.0f, 2.0f);
		ImGui::NewLine();
		// --------------------------------------------------- //


		// ----------------- Spot Angle --------------------- //
		ImGui::Text("Spot Angle");
		if (ImGui::SliderFloat("Inner Angle", &mainLight.innerAngle, 0, 90))
		{
			if (mainLight.innerAngle > mainLight.outerAngle)
				mainLight.innerAngle = mainLight.outerAngle;
		}
		if (ImGui::SliderFloat("Outer Angle", &mainLight.outerAngle, 0, 90))
		{
			if (mainLight.outerAngle < mainLight.innerAngle)
				mainLight.outerAngle = mainLight.innerAngle;
		}
		// -------------------------------------------------- //

		ImGui::SliderFloat("Fallout", &mainLight.fallOut, 1, 30);
		ImGui::NewLine();
	}

	if (ImGui::CollapsingHeader("Local Lights Info"))
	{

	}

	ImGui::End();
	// ------------------------------------------------------ //

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void SampleScene::CreateObjects()
{
	// -------------------- Create Poly And Object -------------------- //
	Shape* BunnyPoly = new Obj("bunny_b.obj");
	Shape* BoxP = new Box();

	fsq = new Quad(1);

	bunny1 = new Object(BunnyPoly, Color::red, Color::white, 120);
	bunny2 = new Object(BunnyPoly, Color::white, Color::white, 1);
	bunny3 = new Object(BunnyPoly, Color::gold, Color::white, 50);
	table = new Object(BoxP, Color::white, Color::black, 1);

	bunny1->transform = Translate(0, 0, 1) * Scale(10, 10, 10) * Rotate(0, 90);// *Scale(10, 10, 10);// *Rotate(1, 45);
	//bunny2->transform = Translate(0, 0, 1) * Rotate(0, 90) * Scale(10, 10, 10); //* Rotate(1, 45);
	//bunny3->transform = Translate(0, -5, 50) * Rotate(0, 90) * Scale(10, 10, 10); //* Rotate(1, 45);
	table->transform = Translate(0, 0, 0.5) * Scale(5, 10, 0.25f);

	models->Add(bunny1);
	//models->Add(bunny2);
	//models->Add(bunny3);
	models->Add(table);
	// ---------------------------------------------------------------- //


	// -------------------- Create Local Lights ---------------------- //

	localLights.Add(glm::vec3(0, 0, 5), glm::vec3(0, 1, 0), 12);
	localLights.Add(glm::vec3(5, 0, 2), glm::vec3(1, 0, 0), 12);
	localLights.Add(glm::vec3(0, 5, 2), glm::vec3(0, 0, 1), 12);
	localLights.Add(glm::vec3(0, 0, -5), glm::vec3(1, 1, 0), 12);
	localLights.Add(glm::vec3(-5, 0, 2), glm::vec3(1, 0, 1), 12);
	localLights.Add(glm::vec3(0, -5, 2), glm::vec3(0, 1, 1), 12);

	/*for (int i = 0; i < activeLightCount; ++i)
	{
		int x = GetRandom(-5, 5);
		int y = GetRandom(-5, 5);
		int z = GetRandom(-5, 5);

		float r = myrandomf(RNGen);
		float g = myrandomf(RNGen);
		float b = myrandomf(RNGen);

		int range = GetRandom(5, 12);

		localLights.Add(glm::vec3(x, y, z), glm::vec3(r, g, b), (float)range);
	}*/

	// --------------------------------------------------------------- //
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


	// ----------------------- Deferred ----------------------------- // 
	deferredShader = new ShaderProgram();
	deferredShader->AddShader("deferred.vert", GL_VERTEX_SHADER);
	deferredShader->AddShader("deferred.frag", GL_FRAGMENT_SHADER);
	glBindAttribLocation(deferredShader->programId, 0, "inVertex");

	deferredShader->LinkProgram();
	// ------------------------------------------------------------- //


	// ---------------------- Local Light -------------------------- //
	localLightShader = new ShaderProgram();
	localLightShader->AddShader("localLights.vert", GL_VERTEX_SHADER);
	localLightShader->AddShader("localLights.frag", GL_FRAGMENT_SHADER);
	glBindAttribLocation(localLightShader->programId, 0, "inVertex");

	localLightShader->LinkProgram();
	// ------------------------------------------------------------- //
}

//void SampleScene::CreateLightUniform()
//{
//	int programId = deferredShader->programId;
//	GLuint uboIndex = glGetUniformBlockIndex(programId, "Lights");
//
//	GLint size;
//	glGetActiveUniformBlockiv(programId, uboIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &size);
//
//	GLint numUniforms;
//	glGetActiveUniformBlockiv(programId, uboIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &numUniforms);
//
//	GLuint* indices = new GLuint[numUniforms];
//
//	GLint maxUniformNameLength;
//	glGetProgramiv(programId, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniformNameLength);
//
//	std::vector<const GLchar*> uniformNames(numUniforms);
//	std::vector<GLchar> uniformNameBuffer(maxUniformNameLength);
//
//	for (int i = 0; i < numUniforms; ++i) {
//		GLsizei length;  // Length of the uniform name
//		glGetActiveUniformName(programId, i, maxUniformNameLength, &length, &uniformNameBuffer[0]);
//
//		std::string uniformName(uniformNameBuffer.data(), length);
//		uniformNames[i] = _strdup(uniformName.c_str());
//	}
//
//	glGetUniformIndices(programId, numUniforms, uniformNames.data(), indices);
//
//	GLint* offset = new GLint[numUniforms];
//	glGetActiveUniformsiv(programId, numUniforms, indices, GL_UNIFORM_OFFSET, offset);
//
//	glGenBuffers(1, &uboLight);
//	glBindBuffer(GL_UNIFORM_BUFFER, uboLight);
//	glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
//	glBindBufferRange(GL_UNIFORM_BUFFER, uboIndex, uboLight, 0, size);
//	glBindBuffer(GL_UNIFORM_BUFFER, 0);
//
//	for (int i = 0; i < numUniforms; ++i)
//		free(const_cast<GLchar*>(uniformNames[i]));
//	delete[] indices;
//}

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
	proj = Perspective((ry * screenSize.x) / screenSize.y, ry, front, 1000);
	worldInverse = glm::inverse(worldView);
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

	models->Draw(gBufferShader);

	gBufferShader->UnuseShader();
	gbo.Unbind();
}


#include <glm/gtc/type_ptr.hpp>

#include "Color.hpp"
#include "Shape.hpp"
#include "Transform.hpp"

#include "MainLight.hpp"
#include "Shader.hpp"
#include "Object.hpp"

MainLight::~MainLight()
{
	delete obj;
	delete solidShader;
}

void MainLight::Initialize()
{
	type = Directional;
	position = glm::vec3(1);
	direction = glm::vec3(0, 0, -1);
	scale = glm::vec3(0.5);
	solidObjectColor = Color::red;
	IsDraw = true;
	innerAngle = 10.0f;
	outerAngle = 60.0f;
	fallOut = 1.0f;
	attenuationConstant = { 0.3f, 0.3f, 0.02f };

	Shape* sphere = new Sphere(32);
	obj = new Object(sphere, solidObjectColor, Color::black, 1);

	solidShader = new ShaderProgram();
	solidShader->AddShader("solidShader.vert", GL_VERTEX_SHADER);
	solidShader->AddShader("solidShader.frag", GL_FRAGMENT_SHADER);

	glBindAttribLocation(solidShader->programId, 0, "inVertex");
	glBindAttribLocation(solidShader->programId, 1, "inNormal");

	solidShader->LinkProgram();
}

void MainLight::UpdateLightData(int programId)
{
	int loc;

	loc = glGetUniformLocation(programId, "mainLight.type");
	glUniform1ui(loc, type);

	loc = glGetUniformLocation(programId, "mainLight.position");
	glUniform3fv(loc, 1, &position[0]);

	loc = glGetUniformLocation(programId, "mainLight.direction");
	glUniform3fv(loc, 1, &direction[0]);

	loc = glGetUniformLocation(programId, "mainLight.attenuationConstants");
	glUniform3fv(loc, 1, &attenuationConstant[0]);

	loc = glGetUniformLocation(programId, "mainLight.innerAngle");
	glUniform1f(loc, innerAngle);

	loc = glGetUniformLocation(programId, "mainLight.outerAngle");
	glUniform1f(loc, outerAngle);

	loc = glGetUniformLocation(programId, "mainLight.fallOut");
	glUniform1f(loc, fallOut);
}

void MainLight::DrawObject(glm::mat4 proj, glm::mat4 view)
{
	if (!IsDraw)
		return;

	// ------------- Draw Solid Object ------------------ //
	solidShader->UseShader();

	int loc, programId;
	programId = solidShader->programId;
	loc = glGetUniformLocation(programId, "WorldView");
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(view));

	loc = glGetUniformLocation(programId, "Projection");
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(proj));

	loc = glGetUniformLocation(programId, "color");
	glUniform3fv(loc, 1, glm::value_ptr(solidObjectColor));

	loc = glGetUniformLocation(programId, "alpha");
	glUniform1f(loc, 1.0f);
	obj->transform = Translate(position) * Scale(scale);
	obj->Draw(solidShader);

	solidShader->UnuseShader();
	// ------------------------------------------------- //
}

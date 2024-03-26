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
	position = glm::vec3(1);
	scale = glm::vec3(2);
	color = glm::vec3(1);
	solidObjectColor = Color::red;
	IsDraw = true;

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

	loc = glGetUniformLocation(programId, "mainLight.position");
	glUniform3fv(loc, 1, &position[0]);

	loc = glGetUniformLocation(programId, "mainLight.color");
	glUniform3fv(loc, 1, &color[0]);
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

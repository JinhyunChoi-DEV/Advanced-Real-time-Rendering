#include "SkySphere.hpp"
#include "Shape.hpp"
#include "Shader.hpp"
#include "Transform.hpp"

void SkySphere::Initialize(std::string fileName)
{
	std::string path = rootPath + fileName;

	sphere = new Sphere(32);
	texture.Load(path);

	shader = new ShaderProgram();
	shader->AddShader("skysphereShader.vert", GL_VERTEX_SHADER);
	shader->AddShader("skysphereShader.frag", GL_FRAGMENT_SHADER);

	glBindAttribLocation(shader->programId, 0, "inVertex");
	glBindAttribLocation(shader->programId, 1, "inNormal");
	glBindAttribLocation(shader->programId, 2, "inTexture");
	shader->LinkProgram();
}

void SkySphere::Update(glm::mat4 proj, glm::mat4 view)
{
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_DEPTH_TEST);

	int loc, programId;

	shader->UseShader();
	programId = shader->programId;

	loc = glGetUniformLocation(programId, "Projection");
	glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(proj));

	auto tempView = glm::mat4(glm::mat3(view));
	loc = glGetUniformLocation(programId, "WorldView");
	glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(tempView));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture.bufferObj);

	sphere->DrawVAO();

	shader->UnuseShader();

	glBindTexture(GL_TEXTURE_2D, 0);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
}

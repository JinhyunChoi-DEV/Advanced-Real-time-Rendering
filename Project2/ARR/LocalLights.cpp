#include "LocalLights.hpp"

#include <glm/gtc/type_ptr.hpp>
#include "Shape.hpp"
#include "Shader.hpp"
#include "Transform.hpp"

//LocalLights::LocalLights(ShaderProgram* shader)
//{
//	Initialize(shader);
//}

LocalLights::~LocalLights()
{
	delete shape;
	delete solidShader;
}

void LocalLights::Initialize(ShaderProgram* shader)
{
	this->shader = shader;
	shape = new Sphere(32);
	this->activeCount = activeCount;

	// ******************************************************** //
	unsigned int index = glGetProgramResourceIndex(shader->programId, GL_SHADER_STORAGE_BLOCK, "LocalLights");

	glGenBuffers(1, &ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 0, &data, GL_DYNAMIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	// ******************************************************** //

	solidShader = new ShaderProgram();
	solidShader->AddShader("solidShader.vert", GL_VERTEX_SHADER);
	solidShader->AddShader("solidShader.frag", GL_FRAGMENT_SHADER);

	glBindAttribLocation(solidShader->programId, 0, "inVertex");
	glBindAttribLocation(solidShader->programId, 1, "inNormal");

	solidShader->LinkProgram();
	IsDrawSphere = false;
}

void LocalLights::UpdateSSBO()
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int), &data.count);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 16, structSize, data.lights.data());
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void LocalLights::Update(glm::mat4 proj, glm::mat4 view, glm::vec2 screenSize)
{
	int loc, programId;
	programId = shader->programId;

	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	shader->UseShader();
	loc = glGetUniformLocation(shader->programId, "WorldView");
	glUniformMatrix4fv(loc, 1, GL_FALSE, &view[0][0]);

	loc = glGetUniformLocation(shader->programId, "Projection");
	glUniformMatrix4fv(loc, 1, GL_FALSE, &proj[0][0]);

	glm::mat4 wInverse = glm::inverse(view);
	loc = glGetUniformLocation(shader->programId, "WorldInverse");
	glUniformMatrix4fv(loc, 1, GL_FALSE, &wInverse[0][0]);

	loc = glGetUniformLocation(programId, "screenSize");
	glUniform2fv(loc, 1, &screenSize[0]);

	loc = glGetUniformLocation(programId, "activeLightCount");
	glUniform1ui(loc, activeCount);

	for (int i = 0; i < activeCount; ++i)
	{
		auto light = lights[i];
		data.lights[i].position = glm::vec4(light->position, light->range);
		auto tr = Translate(light->position) * Scale(light->range);
		loc = glGetUniformLocation(shader->programId, "ModelTransform");
		glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(tr));

		shape->DrawVAO();
	}
	shader->UnuseShader();

	// ------------------- Draw Sphere Object ------------------- //
	if (IsDrawSphere)
	{
		glEnable(GL_DEPTH_TEST);
		solidShader->UseShader();
		programId = solidShader->programId;
		for (int i = 0; i < activeCount; ++i)
		{
			auto light = lights[i];
			loc = glGetUniformLocation(programId, "WorldView");
			glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(view));

			loc = glGetUniformLocation(programId, "Projection");
			glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(proj));

			auto tr = Translate(light->position) * Scale(light->range);
			loc = glGetUniformLocation(programId, "ModelTransform");
			glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(tr));

			loc = glGetUniformLocation(programId, "color");
			glUniform3fv(loc, 1, &light->color[0]);

			loc = glGetUniformLocation(programId, "alpha");
			glUniform1f(loc, 0.5f);
			shape->DrawLineVAO();
		}
		solidShader->UnuseShader();
		glDisable(GL_DEPTH_TEST);
	}
	// -------------------------------------------------------- //

	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
}


void LocalLights::Add(glm::vec3 pos, glm::vec3 color, float range)
{
	LocalLight* newLight = new LocalLight();
	newLight->position = pos;
	newLight->color = color;
	newLight->range = range;
	newLight->shape = shape;

	lights.push_back(newLight);
	activeCount = data.count + 1;

	Light l;
	l.position = glm::vec4(pos, range);
	l.color = glm::vec4(color, 1);

	data.count++;
	data.lights.push_back(l);
	UpdateSSBOSize();
}

void LocalLights::Delete()
{
	if (activeCount <= 0)
		return;

	lights.erase(lights.end() - 1);
	activeCount = data.count - 1;

	data.count--;
	data.lights.erase(data.lights.end() - 1);
	UpdateSSBOSize();
}

void LocalLights::UpdateSSBOSize()
{
	structSize = (sizeof(glm::vec4) * 2) * data.count;

	unsigned long long size = 16 + structSize;
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, size, &data, GL_DYNAMIC_COPY);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

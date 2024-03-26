#include "SkySphere.hpp"

#include <iostream>

#include "Shape.hpp"
#include "Shader.hpp"
#include "Transform.hpp"
#include <omp.h>

const float pi = 3.14159265358979323846f;
void SkySphere::Initialize(std::string fileName)
{
	std::string path = rootPath + fileName;

	sphere = new Sphere(32);
	texture.Load(path);

	InitIrradianceMap();
	CreateIrradianceMap();
	texture.Release();

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

void SkySphere::InitIrradianceMap()
{
	shCoefficients = std::vector<glm::vec3>(9, glm::vec3(0.0f));
	float delta_theta = pi / (float)texture.height;
	float delta_pi = (2.0f * pi) / (float)texture.width;

	const float A0 = pi;
	const float A1 = (2.0f / 3.0f) * pi;
	const float A2 = (1.0f / 4.0f) * pi;

#pragma omp parallel for schedule(dynamic, 1)
	for(int y = 0; y< texture.height; ++y)
	{
		for(int x = 0; x < texture.width; ++x)
		{
			float theta = pi * ((float)y + 0.5f)/ (float)texture.height;
			float phi = 2.0f * pi * ((float)x + 0.5f)/(float)texture.width;
			glm::vec3 N = glm::normalize(glm::vec3(cos(phi) * sin(theta), sin(phi) * sin(theta), cos(theta)));

			int idx = (y * texture.width + x) * texture.channel;
			glm::vec3 pixelValue = glm::vec3(texture.buffer[idx], texture.buffer[idx + 1], texture.buffer[idx + 2]);
			float sin_theta = sin(theta);

			std::vector<float> coefficient = SHCoefficient(N);
			for (int i = 0; i < coefficient.size(); ++i)
				shCoefficients[i] += pixelValue * coefficient[i] * sin_theta * delta_theta * delta_pi;
		}
	}

	shCoefficients[0] *= A0;

	shCoefficients[1] *= A1;
	shCoefficients[2] *= A1;
	shCoefficients[3] *= A1;

	shCoefficients[4] *= A2;
	shCoefficients[5] *= A2;
	shCoefficients[6] *= A2;
	shCoefficients[7] *= A2;
	shCoefficients[8] *= A2;
}

void SkySphere::CreateIrradianceMap()
{
	int width = 400;
	int height = 200;
	std::vector<float> outImage(3.0f * width * height);

#pragma omp parallel for schedule(dynamic, 1)
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			float theta = pi * ((float)y + 0.5f) / (float)height;
			float phi = 2.0f * pi * ((float)x + 0.5f) / (float)width;
			glm::vec3 N = glm::normalize(glm::vec3(cos(phi) * sin(theta), sin(phi) * sin(theta), cos(theta)));
			std::vector<float> coefficient = SHCoefficient(N);

			glm::vec3 irradiance(0.0f);
			for(int n = 0; n < 9; ++n)
				irradiance += shCoefficients[n] * coefficient[n];

			int index = (y * width + x) * 3;
			outImage[index] = irradiance.r;
			outImage[index+1] = irradiance.g;
			outImage[index+2] = irradiance.b;
		}
	}

	glGenTextures(1, &irradMapBuffer);
	glBindTexture(GL_TEXTURE_2D, irradMapBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, outImage.data());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

std::vector<float> SkySphere::SHCoefficient(const glm::vec3& N)
{
	std::vector<float> result(9, 0.0f);

	result[0] = 0.5f * sqrt(1.0f / pi);

	result[1] = 0.5f * sqrt(3.0f / pi)*N.y;
	result[2] = 0.5f * sqrt(3.0f / pi)*N.z;
	result[3] = 0.5f * sqrt(3.0f / pi)*N.x;

	result[4] = 0.5f * sqrt(15.0f / pi)*N.x*N.y;
	result[5] = 0.5f * sqrt(15.0f / pi)*N.y*N.z;
	result[6] = 0.25f * sqrt(5.0f / pi)*(3.0f*N.z*N.z - 1.0f);
	result[7] = 0.5f * sqrt(15.0f / pi)*N.x*N.z;
	result[8] = 0.25f * sqrt(15.0f / pi)*(N.x*N.x - N.y*N.y);

	return result;
}

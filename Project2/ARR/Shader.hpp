#pragma once
#include <string>
#include "main.hpp"

class ShaderProgram
{
public:
	ShaderProgram();

	void AddShader(std::string fileName, gl::GLenum type);
	void LinkProgram();
	void UseShader();
	void UnuseShader();

	int programId;

private:
	const std::string filePath = "shaders/";
};

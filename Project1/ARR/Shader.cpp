#include "Shader.hpp"

#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>

std::string GetCode(std::string filePath)
{
	std::ifstream file;
	std::stringstream fileStream;

	file.open(filePath);
	fileStream << file.rdbuf();
	file.close();

	return fileStream.str();
}

ShaderProgram::ShaderProgram()
{
	programId = glCreateProgram();
}

void ShaderProgram::AddShader(std::string fileName, GLenum type)
{
	std::string fileData = filePath + fileName;
	std::string code = GetCode(fileData);

	const char* src = code.c_str();
	unsigned int shader = glCreateShader(type);
	glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);
	glAttachShader(programId, shader);

	GLboolean compile_result = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_result);
	if (compile_result == GL_TRUE)
	{
		glDeleteShader(shader);
		return;
	}

	std::string error_log = "Shader Compile Error: " + fileName;
	if (type == GL_VERTEX_SHADER)
		error_log += " Vertex Shader\n";
	else
		error_log += " Fragment Shader\n";

	GLint log_length = 0;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);

	GLchar* message = new GLchar[log_length];
	glGetShaderInfoLog(shader, log_length, nullptr, message);

	std::cerr << error_log << message << std::endl;
	assert(false);
	delete[] message;
}

void ShaderProgram::LinkProgram()
{
	glLinkProgram(programId);

	GLboolean status;
	glGetProgramiv(programId, GL_LINK_STATUS, &status);

	if (status == GL_TRUE)
		return;

	std::string error_log = "Shader Link Error: " + programId;

	GLint log_length = 0;
	glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &log_length);

	GLchar* message = new GLchar[log_length];
	glGetProgramInfoLog(programId, log_length, nullptr, message);

	std::cerr << error_log << message << std::endl;
	assert(false);
	delete[] message;
}


void ShaderProgram::UseShader()
{
	glUseProgram(programId);
}

void ShaderProgram::UnuseShader()
{
	glUseProgram(0);
}

//GLuint Compile(std::string filePath, GLenum shaderType)
//{
//	GLuint shader = 0;
//	GLboolean compile_result = GL_FALSE;
//
//	shader = glCreateShader(shaderType);
//
//	std::string tempCode = GetCode(filePath);
//	const char* code = tempCode.c_str();
//	glShaderSource(shader, 1, &code, nullptr);
//	glCompileShader(shader);
//
//	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_result);
//	if (compile_result == GL_FALSE)
//	{
//		std::string error_log = "Failed to compile: ";
//		if (shaderType == GL_FRAGMENT_SHADER)
//			error_log += " fragment shader:\n";
//		else if (shaderType == GL_VERTEX_SHADER)
//			error_log += " vertex shader:\n";
//		GLint log_length = 0;
//		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
//
//		GLchar* log_message = new GLchar[log_length];
//		glGetShaderInfoLog(shader, log_length, nullptr, log_message);
//
//		std::cerr << error_log << log_message << "\n";
//		assert(false);
//		delete[] log_message;
//	}
//	return shader;
//}
//
//GLuint Link(GLuint vertex_handle, GLuint fragment_handle)
//{
//	GLboolean link_result = GL_FALSE;
//	GLuint program = 0;
//	program = glCreateProgram();
//
//	glAttachShader(program, vertex_handle);
//	glAttachShader(program, fragment_handle);
//
//	glLinkProgram(program);
//
//	glGetProgramiv(program, GL_LINK_STATUS, &link_result);
//
//	if (link_result == GL_FALSE)
//	{
//		GLint log_length = 0;
//		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
//
//		GLchar* log_message = new GLchar[log_length];
//		glGetProgramInfoLog(program, log_length, nullptr, log_message);
//
//		std::cerr << "shader program failed to link\n" << log_message << "\n";
//		assert(false);
//		delete[] log_message;
//	}
//	return program;
//}
//
//
//void ShaderProgram::Test(std::string vert, std::string frag)
//{
//	auto vertPath = filePath + vert;
//	auto fragPath = filePath + frag;
//	unsigned vertShader = Compile(vertPath, GL_VERTEX_SHADER);
//	unsigned fragShader = Compile(fragPath, GL_FRAGMENT_SHADER);
//
//	programId = Link(vertShader, fragShader);
//
//	glDeleteProgram(vertShader);
//	glDeleteProgram(fragShader);
//}
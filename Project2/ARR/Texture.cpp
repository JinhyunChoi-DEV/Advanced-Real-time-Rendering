#include <glbinding/gl/gl.h>
using namespace gl;

#include "Texture.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

void Texture::Load(std::string path)
{
	buffer = stbi_load(path.c_str(), &width, &height, &channel, 0);

	glGenTextures(1, &bufferObj);
	glBindTexture(GL_TEXTURE_2D, bufferObj);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	GLenum type;
	if (channel == 3)
		type = GL_RGB;
	else if (channel == 4)
		type = GL_RGBA;

	glTexImage2D(GL_TEXTURE_2D, 0, type, width, height, 0, type, GL_UNSIGNED_BYTE, buffer);
}

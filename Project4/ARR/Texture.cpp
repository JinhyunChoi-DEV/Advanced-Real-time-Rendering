#include <glbinding/gl/gl.h>
using namespace gl;

#include "Texture.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

void Texture::Load(std::string path)
{
	buffer = stbi_loadf(path.c_str(), &width, &height, &channel, 0);

	glGenTextures(1, &bufferObj);
	glBindTexture(GL_TEXTURE_2D, bufferObj);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, buffer);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
}

void Texture::Release()
{
	if (buffer != nullptr)
		stbi_image_free(buffer);
}
 
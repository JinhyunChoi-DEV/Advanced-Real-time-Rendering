#include "GBO.hpp"

#include <iostream>
#include <glbinding/gl/gl.h>
using namespace  gl;

constexpr int numOfTexture = 6 ;
void GBO::Create(const int w, const int h)
{
	width = w;
	height = h;

	textureID = new unsigned int[numOfTexture];
	GLenum* attachments = new GLenum[numOfTexture];

	glGenFramebuffersEXT(1, &fboID);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboID);

	glGenTextures(numOfTexture, textureID);

	for (int i = 0; i < numOfTexture; ++i)
	{
		glBindTexture(GL_TEXTURE_2D, textureID[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, (int)GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (int)GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (int)GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (int)GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (int)GL_LINEAR);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + i, GL_TEXTURE_2D, textureID[i], 0);
		attachments[i] = GL_COLOR_ATTACHMENT0_EXT + i;
	}

	unsigned int depthBuffer;
	glGenRenderbuffersEXT(1, &depthBuffer);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthBuffer);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, width, height); 
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depthBuffer);

	glDrawBuffers(numOfTexture, attachments);

	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
		std::cerr << "GBO Error" << std::endl;

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	delete[] attachments;
}

void GBO::Bind() { glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboID); }
void GBO::Unbind() { glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0); }

void GBO::CopyDepthBuffer()
{
	glBindFramebufferEXT(GL_READ_FRAMEBUFFER, fboID);
	glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER, 0);

	glBlitFramebufferEXT(0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebufferEXT(GL_FRAMEBUFFER, 0);
}

void GBO::BindTexture(const int unit, const int programId, const std::string& name, const int id)
{
	glActiveTexture((gl::GLenum)((int)GL_TEXTURE0 + unit));
	glBindTexture(GL_TEXTURE_2D, textureID[id]);
	int loc = glGetUniformLocation(programId, name.c_str());
	glUniform1i(loc, unit);
}

void GBO::UnbindTexture(const int unit)
{
	glActiveTexture((gl::GLenum)((int)GL_TEXTURE0 + unit));
	glBindTexture(GL_TEXTURE_2D, 0);
}

void GBO::Clear()
{
	/*glDeleteFramebuffers(1, &fboID);
	glDeleteTexturesEXT(numOfTexture, textureID);*/
}

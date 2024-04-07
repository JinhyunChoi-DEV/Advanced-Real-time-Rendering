#pragma once
#include <string>

class FBO
{
public:
	unsigned int fboID;
	unsigned int textureID;
	int width, height;

	void Create(int w, int h);
	void Bind();
	void UnBind();

	void BindTexture(const int unit, const unsigned int programId, const std::string name);
	void UnbindTexture(const int unit);

};

#pragma once
#include <string>

class GBO
{
public:
	unsigned int fboID;
	unsigned int* textureID = nullptr;
	int width, height;

	void Create(const int w, const int h);
	void Bind();
	void Unbind();

	void CopyDepthBuffer();

	void BindTexture(const int unit, const int programId, const std::string& name, const int textureId);
	void UnbindTexture(const int unit);

private:
	void Clear();
};

#pragma once

class GBO
{
public:
	unsigned int fboID;
	unsigned int* textureID = nullptr;
	int width, height;

	void CreateGBO(const int w, const int h);
	void Bind();
	void Unbind();

	void CopyDepthBuffer();

	void BindTexture();
	void UnbindTexture();

private:
	void Clear();
};

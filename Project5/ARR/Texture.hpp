#pragma once
#include <string>

class Texture
{
public:

	void Load(std::string path);
	void Release();

	int width = 0;
	int height = 0;
	int channel = 0;
	unsigned int bufferObj = -1;
	float* buffer = nullptr;
};

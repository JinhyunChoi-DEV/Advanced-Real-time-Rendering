#version 430 core

uniform float near; 
uniform float far;

in vec4 position;
out vec4 shadowData;

void main()
{
	float z = position.w;

	float logDepth = log2(z - near + 1.0f) / log2(far - near+1.0f);
	shadowData = vec4(logDepth, logDepth*logDepth, logDepth*logDepth*logDepth, logDepth*logDepth*logDepth*logDepth);
}
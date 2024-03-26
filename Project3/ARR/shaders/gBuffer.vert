#version 430 core

uniform mat4 WorldView, Projection, NormalTransform, ModelTransform;

in vec4 inVertex;
in vec3 inNormal;
in vec2 inTexture;
in vec3 inTangent;

out vec4 worldPosition;
out vec3 normalVector;
out vec2 uvCoord;

void main()
{
	worldPosition = ModelTransform * inVertex;
	normalVector = inNormal* mat3(NormalTransform);
	uvCoord = inTexture;

	gl_Position = Projection*WorldView*ModelTransform*inVertex;
}
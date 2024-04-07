#version 430 core

uniform mat4 WorldView, Projection, NormalTransform, ModelTransform;

in vec4 inVertex;
in vec3 inNormal;
in vec2 inTexture;
in vec3 inTangent;

out vec4 worldPosition;
out vec3 normalVector;
out vec2 uvCoord;
out vec3 FragPos;

void main()
{
	worldPosition = ModelTransform * inVertex;
	normalVector = inNormal* mat3(NormalTransform);
	uvCoord = inTexture;

	vec4 viewPos = WorldView * ModelTransform * inVertex;
	FragPos = viewPos.xyz;

	gl_Position = Projection*WorldView*ModelTransform*inVertex;
}
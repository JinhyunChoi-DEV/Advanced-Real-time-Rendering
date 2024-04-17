#version 430 core

uniform mat4 WorldView, Projection, NormalTransform, ModelTransform;
uniform int npSelected;
uniform float intensity;
uniform bool activeNoise;

in vec4 inVertex;
in vec3 inNormal;
in vec2 inTexture;
in vec3 inTangent;

out vec4 worldPosition;
out vec3 normalVector;
out vec2 uvCoord;
out vec3 FragPos;
out float DeviceDepth;

float random(float n) {
    return fract(sin(n) * 43758.5453123);
}

void main()
{
	float noise = 0.0;

	if(npSelected == 0 || !activeNoise)
		noise = 0.0;
	else
		noise = intensity;

 	vec3 randomOffset = vec3(random(inVertex.x), random(inVertex.y), random(inVertex.z)) *noise;
    vec4 offsetVertex = inVertex + vec4(randomOffset, 0.0);

	worldPosition = ModelTransform * offsetVertex;
	normalVector = inNormal* mat3(NormalTransform);
	uvCoord = inTexture;

	vec4 viewPos = WorldView * ModelTransform * offsetVertex;
	FragPos = viewPos.xyz;

	gl_Position = Projection*viewPos;
	DeviceDepth = gl_Position.w;
}
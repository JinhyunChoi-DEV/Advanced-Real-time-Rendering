#version 430 core

uniform vec3 diffuse;
uniform vec3 specular;
uniform float shininess;

in vec4 worldPosition;
in vec3 normalVector;
in vec2 uvCoord;
in vec3 FragPos;

layout (location=0) out vec4 gWorldPosition;
layout (location=1) out vec4 gNomalVector;
layout (location=2) out vec4 gDiffuse;
layout (location=3) out vec4 gSpecular;
layout (location=4) out float gDepth;

void main()
{
	vec4 W = worldPosition;
	vec3 N = normalize(normalVector);

	gWorldPosition = W;
	gNomalVector.xyz = N;
	gDiffuse.xyz = diffuse;
	gSpecular = vec4(specular, shininess);
	gDepth = FragPos.z;
}

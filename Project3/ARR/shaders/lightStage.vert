#version 430 core

uniform mat4 WorldInverse;

in vec4 inVertex;

out vec3 eye;

void main()
{	
	gl_Position = inVertex;

	eye = (WorldInverse*vec4(0,0,0,1)).xyz;
}
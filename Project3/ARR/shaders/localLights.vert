#version 430 core

uniform mat4 WorldView, Projection, WorldInverse, ModelTransform;

in vec4 inVertex;

out vec3 eye;

void main()
{
	gl_Position = Projection*WorldView*ModelTransform*inVertex;

	eye = (WorldInverse*vec4(0,0,0,1)).xyz;
}
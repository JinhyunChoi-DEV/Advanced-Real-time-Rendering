#version 430 core

uniform mat4 WorldView, Projection, ModelTransform;

in vec4 inVertex;
in vec3 inNormal;


void main()
{
	gl_Position = Projection* WorldView* ModelTransform * inVertex;
}
#version 430 core

uniform mat4 Projection, LightView, ModelTransform;

in vec4 inVertex;
in vec3 inNormal;

out vec4 position;

void main()
{
	gl_Position = Projection * LightView * ModelTransform * inVertex;

	position = gl_Position;
}
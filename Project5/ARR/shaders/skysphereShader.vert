#version 430 core

uniform mat4 WorldView, Projection, Model;

in vec4 inVertex;
in vec3 inNormal;
in vec2 inTexture;

out vec2 uv;

void main()
{
	vec4 pos = Projection * WorldView * inVertex;
	gl_Position = pos.xyww;

	uv = inTexture;
}
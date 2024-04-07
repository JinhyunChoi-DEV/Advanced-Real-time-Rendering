#version 430 core

uniform sampler2D background;

in vec2 uv;

out vec4 FragColor;
void main()
{
	FragColor = texture(background, uv);
}
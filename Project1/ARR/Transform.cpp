#include "Transform.hpp"

const float pi = 3.14159f;
glm::mat4 Rotate(const int i, const float theta)
{
	glm::mat4 R = glm::mat4(1.0);
	int j = (i + 1) % 3;
	int k = (j + 1) % 3;
	R[j][j] = R[k][k] = cos(theta * pi / 180.0f);
	R[j][k] = sin(theta * pi / 180.0f);
	R[k][j] = -R[j][k];
	return R;
}

glm::mat4 Scale(const float x, const float y, const float z)
{
	glm::mat4 S = glm::mat4(1.0);
	S[0][0] = x;
	S[1][1] = y;
	S[2][2] = z;
	return S;
}

glm::mat4 Scale(const float value)
{
	return Scale(value, value, value);
}

glm::mat4 Scale(const glm::vec3 vec)
{
	return Scale(vec.x, vec.y, vec.z);
}

glm::mat4 Translate(const float x, const float y, const float z)
{
	glm::mat4 T = glm::mat4(1.0f);
	T[3][0] = x;
	T[3][1] = y;
	T[3][2] = z;
	return T;
}

glm::mat4 Translate(const glm::vec3 vec)
{
	return Translate(vec.x, vec.y, vec.z);
}

glm::mat4 Perspective(const float rx, const float ry,
	const float front, const float back)
{
	glm::mat4 P = glm::mat4(1.0);
	P[0][0] = 1.0 / rx;
	P[1][1] = 1.0 / ry;
	P[2][2] = -(back + front) / (back - front);
	P[3][2] = -(2.0f * front * back) / (back - front);
	P[2][3] = -1;
	P[3][3] = 0;
	return P;
}

float* Pntr(glm::mat4& M)
{
	return &(M[0][0]);
}
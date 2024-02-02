#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>

class Shape
{
public:
	Shape() : animated(false) {}
	virtual ~Shape() {};
	virtual void  MakeVAO();
	virtual void  DrawVAO();

	unsigned int vaoID;

	std::vector<glm::vec4> Pnt;
	std::vector<glm::vec3> Nrm;
	std::vector<glm::vec2> Tex;
	std::vector<glm::vec3> Tan;
	std::vector<glm::ivec3> Tri;
	unsigned int count;

	glm::vec3 minP, maxP;
	glm::vec3 Center;
	bool animated;
	float size;
};

class Box : public Shape
{
public:
	Box();

private:
	void face(const glm::mat4x4 tr);
};

class Sphere : public Shape
{
public:
	Sphere(const int n);
};

class Plane : public Shape
{
public:
	Plane(const float range, const int n);
};

class Quad : public Shape
{
public:
	Quad(const int n = 1);
};

class Obj : public Shape
{
public:
	Obj(const std::string name);

private:
	const std::string prefix = "models/";
};
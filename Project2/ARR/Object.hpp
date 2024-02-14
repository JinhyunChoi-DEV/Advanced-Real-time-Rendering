#pragma once
#include "Color.hpp"
#include "Shape.hpp"

class ShaderProgram;
class ObjectInstance;
class Object;

class Object
{
public:
	Object(Shape* shape, const glm::vec3 _d = glm::vec3(), const glm::vec3 _s = glm::vec3(), const float _n = 1);

	Shape* shape;
	glm::vec3 diffuse = Color::white;
	glm::vec3 specular = Color::black;
	glm::mat4x4 transform = glm::mat4x4(1.0f);
	float shininess;
	bool isDraw = true;

	void Draw(ShaderProgram* shader);
	void Add(Object* obj, ObjectInstance* objInstance);

private:
	std::vector<Object*> instances;
};
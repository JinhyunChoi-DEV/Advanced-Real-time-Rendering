#include "Shader.hpp"
#include "Transform.hpp"

#include "Object.hpp"
#include "ObjectInstance.hpp"

Object::Object(Shape* shape, const glm::vec3 _d, const glm::vec3 _s, const float _n)
	: shape(shape), diffuse(_d), specular(_s), shininess(_n)
{
	transform = glm::mat4x4(1.0f);
}

void Object::Draw(ShaderProgram* shader)
{
	int programId = shader->programId;

	int loc = glGetUniformLocation(programId, "ModelTransform");
	glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(transform));

	glm::mat4 inv = glm::inverse(transform);
	loc = glGetUniformLocation(programId, "NormalTransform");
	glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(inv));

	loc = glGetUniformLocation(programId, "diffuse");
	glUniform3fv(loc, 1, &diffuse[0]);

	loc = glGetUniformLocation(programId, "specular");
	glUniform3fv(loc, 1, &specular[0]);

	loc = glGetUniformLocation(programId, "shininess");
	glUniform1f(loc, shininess);


	if (shape)
		if (isDraw)
			shape->DrawVAO();

	if (isDraw)
	{
		for (int i = 0; i < instances.size(); ++i)
		{
			auto child = instances[i];
			child->transform = this->transform * child->transform;
			child->Draw(shader);
		}
	}

}

void Object::Add(Object* obj, ObjectInstance* objInstance)
{
	if (objInstance->Exist(obj))
		objInstance->Delete(obj);

	instances.push_back(obj);
}

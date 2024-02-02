#pragma once
#include <memory>
#include <unordered_map>

class Object;
class ShaderProgram;

class ObjectInstance
{
public:
	ObjectInstance() {}

	Object* Get(Object* obj);
	bool Exist(Object* obj);
	void Add(Object* obj);
	void Delete(Object* obj);
	void Clear();

	void Draw(ShaderProgram* shader);

private:
	std::unordered_map<uintptr_t, Object*> instance;

};
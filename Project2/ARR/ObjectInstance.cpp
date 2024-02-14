#include "ObjectInstance.hpp"
#include "Object.hpp"

Object* ObjectInstance::Get(Object* obj)
{

	if (!Exist(obj))
		return nullptr;

	uintptr_t address = reinterpret_cast<uintptr_t>(obj);
	return instance[address];
}

bool ObjectInstance::Exist(Object* obj)
{
	uintptr_t address = reinterpret_cast<uintptr_t>(obj);
	return instance.contains(address);
}

void ObjectInstance::Add(Object* obj)
{
	if (Exist(obj))
		return;

	uintptr_t address = reinterpret_cast<uintptr_t>(obj);
	instance[address] = obj;
}

void ObjectInstance::Delete(Object* obj)
{
	if (!Exist(obj))
		return;

	uintptr_t address = reinterpret_cast<uintptr_t>(obj);
	instance.erase(address);
}

void ObjectInstance::Clear()
{
	instance.clear();
}

void ObjectInstance::Draw(ShaderProgram* shader)
{
	for (const auto& obj : instance)
		obj.second->Draw(shader);
}

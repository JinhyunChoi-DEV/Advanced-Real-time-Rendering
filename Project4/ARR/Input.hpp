#pragma once

#include <bitset>
#include "main.hpp"

enum class KeyActionState
{
	None,
	Release,
	Press,
	Repeat
};

class Input
{
public:
	Input();
	void Initialize();
	void Update();
	void Terminate();
	void SetKeyboard(int key, int action);
	void SetMouse(double x, double y);
	void SetMouseButton(int key, int action);
	void SetMouseScroll(double x, double y);

	bool KeyPressed(int key);
	bool KeyReleased(int key);
	bool KeyTriggered(int key);

	bool MouseButtonPressed(int key);
	bool MouseButtonReleased(int key);
	bool MouseButtonTriggered(int key);

	glm::vec2 mousePosition{ 0,0 };
	glm::vec2 scrollValue{ 0, 0 };
	//glm::vec2 deltaMousePosition{ 0, 0 };
private:
	void clear();
	KeyActionState convert(int key);

	std::bitset<GLFW_KEY_MENU> keyPressed;
	std::bitset<GLFW_KEY_MENU> keyTriggered;
	std::bitset<GLFW_MOUSE_BUTTON_LAST> mouseButtonPressed;
	std::bitset<GLFW_MOUSE_BUTTON_LAST> mouseButtonTriggered;
};

extern Input* INPUT;
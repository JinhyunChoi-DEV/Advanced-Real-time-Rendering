#include "Input.hpp"
#include "main.hpp"


////////////////////////////////////////////////////////////////////////
// Called for keyboard actions.

void Keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{

	if (action == GLFW_REPEAT) return;

	INPUT->SetKeyboard(key, action);

}

////////////////////////////////////////////////////////////////////////
// Called when a mouse button changes state.
void MouseButton(GLFWwindow* window, int button, int action, int mods)
{
	INPUT->SetMouseButton(button, action);
}

////////////////////////////////////////////////////////////////////////
// Called by GLFW when a mouse moves (while a button is down)
void MouseMotion(GLFWwindow* window, double x, double y)
{
	INPUT->SetMouse(x, y);
}


void Scroll(GLFWwindow* window, double x, double y)
{
	INPUT->SetMouseScroll(x, y);
}

void InitInteraction(GLFWwindow* window)
{
	glfwSetKeyCallback(window, Keyboard);
	glfwSetMouseButtonCallback(window, MouseButton);
	glfwSetCursorPosCallback(window, MouseMotion);
	glfwSetScrollCallback(window, Scroll);
}

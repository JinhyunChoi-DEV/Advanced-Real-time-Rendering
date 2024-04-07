#include "main.hpp"

#include <iostream>
#include "Input.hpp"


int main()
{
	Input* input = new Input();

	if (!glfwInit())  exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_RESIZABLE, 1);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, 0);
	GLFWwindow* window = glfwCreateWindow(1280, 720, "ARR", NULL, NULL);
	if (!window) { glfwTerminate(); exit(-1); }

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	InitInteraction(window);

	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	glbinding::initialize(glfwGetProcAddress);

	SampleScene* scene = new SampleScene(window);

	printf("OpenGL Version: %s\n", glGetString(GL_VERSION));
	printf("GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	printf("Rendered by: %s\n", glGetString(GL_RENDERER));
	fflush(stdout);

	scene->Initialize();

	while (!glfwWindowShouldClose(window) && !scene->isQuit) {
		INPUT->Update();
		glfwPollEvents();

		scene->Update();
		scene->Draw();
		scene->DrawGUI();
		glfwSwapBuffers(window);
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();


	delete input;
	glfwTerminate();
}
#pragma once
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#define GLFW_INCLUDE_NONE
#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
using namespace gl;

#include <GLFW/glfw3.h>

#define GLM_FORCE_CTOR_INIT
#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE
#include <glm/glm.hpp>

#include "interact.hpp"
#include "SampleScene.hpp"
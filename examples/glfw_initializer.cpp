#ifdef __APPLE__
#   include <OpenGL/gl3.h>
#else
#   include <GL/glew.h>
#endif
#include <GLFW/glfw3.h>

#include "glfw_initializer.h"
#include "glfw_window_2d.h"
#include <assert.h>
#include <iostream>

glfw_initializer::glfw_initializer() : is_init(glfwInit())
{
    if(!is_init){
        std::cerr<<"glfw init failed\n";
    }
    assert(is_init);
}
glfw_initializer::~glfw_initializer()
{
    glfwTerminate();
}
glfw_window& glfw_initializer::create2d(window_type t)
{
    windows.push_back(std::make_unique<glfw_window_2d>(t));
    return *windows.back();
}
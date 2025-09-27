#pragma once
#include "glfw_initializer.h"
#include <variant>

struct glfw_window2d_GL_v21;
struct glfw_window2d_GL_v33;

struct glfw_window_2d : glfw_window
{
    glfw_window_2d(window_type t);
    ~glfw_window_2d();
    glfw_window& async_loop(int maxFPS = 30) override;
    glfw_window& event_loop() override;
    union{
        glfw_window2d_GL_v21* v21;
        glfw_window2d_GL_v33* v33;
    } p;
    window_type t;
};

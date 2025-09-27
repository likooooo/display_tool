#pragma once
#include <vector>
#include <memory>

enum class window_type : int
{
    pipline,
    shader,
};
struct glfw_window
{
    virtual glfw_window& async_loop(int maxFPS) = 0;
    virtual glfw_window& event_loop() = 0;
};
struct glfw_initializer
{
    const bool is_init;
    glfw_initializer();
    ~glfw_initializer();
    glfw_window& create2d(window_type t = window_type::pipline);
    std::vector<std::unique_ptr<glfw_window>> windows;
};
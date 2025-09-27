#include "glfw_window_2d.h"
#include "2d/glfw_window2d_GL_v21.hpp"
#include "2d/glfw_window2d_GL_v33.hpp"

glfw_window2d_GL_v21* real_ptr21(void*p)
{
    return reinterpret_cast<glfw_window2d_GL_v21*>(p);
}
glfw_window2d_GL_v33* real_ptr33(void*p)
{
    return reinterpret_cast<glfw_window2d_GL_v33*>(p);
}
glfw_window_2d::glfw_window_2d(window_type type) : t(type)
{
    if(t == window_type::pipline){
        p.v21 = new glfw_window2d_GL_v21;
        printf("use OpenGL2.1\n");
    }
    else{
        p.v33 = new glfw_window2d_GL_v33;
        printf("use OpenGL3.3\n");
    }
}
glfw_window_2d::~glfw_window_2d()
{
    if(t == window_type::pipline){
        delete p.v21;
    }
    else{
        delete p.v33;
    }
}

glfw_window& glfw_window_2d::async_loop(int maxFPS) 
{
    if(t == window_type::pipline){
        p.v21->async_loop(maxFPS);
    }
    else{
        p.v33->async_loop(maxFPS);
    }
    return *this;
}
glfw_window& glfw_window_2d::event_loop()
{
    if(t == window_type::pipline){
        p.v21->event_loop();
    }
    else{
        p.v33->event_loop();
    }
    return *this;
}
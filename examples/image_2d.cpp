#include "glfw_window_2d.h"

int main() 
{
    glfw_initializer().create2d(window_type::pipline).async_loop(30).event_loop();
    return 0;
}
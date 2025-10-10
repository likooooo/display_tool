#include "glfw_window_2d.h"
#include <string>

int main(int argc, char** argv) 
{
    window_type type = argc == 1 ? window_type::pipline : (window_type)(std::stoi(argv[1])); 
    glfw_initializer().create2d(type).async_loop(30).event_loop();
    return 0;
}
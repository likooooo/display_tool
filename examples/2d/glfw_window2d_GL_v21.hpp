#pragma once
#ifdef __APPLE__
#   include <OpenGL/gl3.h>
#else
#   include <GL/glew.h>
#endif
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <cstdio>
#include <thread>
#include <atomic>
#include <chrono>
#include <cmath>

struct Ortho2D 
{ 
    float zoom = 1.0f;  // 1.0 = fit full texture
    float panX = 0.0f; 
    float panY = 0.0f; 
    float scroll_speed = 0.1f;
    float move_speed = 1.0f;
    float lastX{0};
    float lastY{0};
    bool dragging{false}; 
};

// static GLuint make_checker_tex(int N = 256) {
//     std::vector<unsigned char> img(N*N*3);
//     for (int y = 0; y < N; ++y) {
//         for (int x = 0; x < N; ++x) {
//             int c = (((x>>4)&1) ^ ((y>>4)&1)) ? 255 : 60;
//             img[(y*N + x)*3 + 0] = (unsigned char)c;
//             img[(y*N + x)*3 + 1] = (unsigned char)c;
//             img[(y*N + x)*3 + 2] = (unsigned char)c;
//         }
//     }
//     GLuint tex = 0; glGenTextures(1, &tex);
//     glBindTexture(GL_TEXTURE_2D, tex);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, N, N, 0, GL_RGB, GL_UNSIGNED_BYTE, img.data());
//     return tex;
// }

static GLuint make_checker_tex(int N = 256)
{
    std::vector<unsigned char> img(N * N * 3);
    for(int y=0;y<N;++y){
        for(int x=0;x<N;++x){
            int c = (((x>>4)&1) ^ ((y>>4)&1)) ? 255 : 60;
            img[(y*N+x)*3+0] = (unsigned char)c;
            img[(y*N+x)*3+1] = (unsigned char)c;
            img[(y*N+x)*3+2] = (unsigned char)c;
        }
    }
    GLuint t; glGenTextures(1, &t);
    glBindTexture(GL_TEXTURE_2D, t);


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // or GL_NEAREST
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    float borderColor[] = {0.0f, 0.0f, 0.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, N, N, 0, GL_RGB, GL_UNSIGNED_BYTE, img.data());
    glBindTexture(GL_TEXTURE_2D, 0);
    return t;
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    //== TODO : load keybord-binding config file
    if (action == GLFW_PRESS) {
        // 普通键
        if (key == GLFW_KEY_ESCAPE) {
            std::cout << "Escape pressed -> exit\n";
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        // Ctrl + S
        if (key == GLFW_KEY_S && (mods & GLFW_MOD_CONTROL)) {
            std::cout << "Ctrl + S pressed\n";
        }

        // Shift + A
        if (key == GLFW_KEY_A && (mods & GLFW_MOD_SHIFT)) {
            std::cout << "Shift + A pressed\n";
        }

        // Ctrl + Alt + D
        if (key == GLFW_KEY_D && (mods & GLFW_MOD_CONTROL) && (mods & GLFW_MOD_ALT)) {
            std::cout << "Ctrl + Alt + D pressed\n";
        }
    }
}

static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    auto* self = reinterpret_cast<Ortho2D*>(glfwGetWindowUserPointer(window));
    if (!self || !self->dragging) return;
    
    float dx = static_cast<float>(xpos - self->lastX);
    float dy = static_cast<float>(ypos - self->lastY);
    int w,h; glfwGetFramebufferSize(window, &w, &h);
    self->panX -= dx / float(w) / self->zoom * self->move_speed;
    self->panY += dy / float(h) / self->zoom * self->move_speed;
    self->lastX = static_cast<float>(xpos);
    self->lastY = static_cast<float>(ypos);
}

static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    auto* self = reinterpret_cast<Ortho2D*>(glfwGetWindowUserPointer(window));
    if (!self) return;

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            self->dragging = true;
            double mx, my;
            glfwGetCursorPos(window, &mx, &my);
            self->lastX = static_cast<float>(mx);
            self->lastY = static_cast<float>(my);
        } else if (action == GLFW_RELEASE) {
            self->dragging = false;
        }
    }
}

static void scrollCallback(GLFWwindow* w, double, double yoff)
{
    auto* c = reinterpret_cast<Ortho2D*>(glfwGetWindowUserPointer(w));
    if (!c) return;
    c->zoom *= (1.0f + c->scroll_speed * yoff);
    //== max 500%
    c->zoom = std::max(0.2f, c->zoom);
    //== min 10%
    c->zoom = std::min(10.0f, c->zoom); 
}

struct glfw_window2d_GL_v21 final
{
    GLFWwindow* win;
    Ortho2D cam;
    std::vector<GLuint> texture_list;
    std::thread t;
    std::atomic<bool> running{true};
    glfw_window2d_GL_v21()
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        win = glfwCreateWindow(960, 600, "image_2d", nullptr, nullptr);
        glfwSetWindowUserPointer(win, &cam);
        glfwSetKeyCallback(win, keyCallback);
        glfwSetScrollCallback(win, scrollCallback);
        glfwSetCursorPosCallback(win, cursorPosCallback);
        glfwSetMouseButtonCallback(win, mouseButtonCallback);
    }
    ~glfw_window2d_GL_v21()
    {
        if(t.joinable())t.join();
        glDeleteTextures(texture_list.size(), texture_list.data());
        if(win) glfwDestroyWindow(win);
    }
    bool valid() const
    {
        return nullptr != win;
    }
    glfw_window2d_GL_v21& activate(bool flag = true)
    {
        glfwMakeContextCurrent(flag ? win : nullptr);
        return *this;
    }
    glfw_window2d_GL_v21& set_fps_ratio(float ratio/*[0 ~ 1]*/)
    {
        ratio = std::round(1.0f / ratio);
        glfwSwapInterval(int(ratio));
        return *this;
    }

    glfw_window2d_GL_v21& set_scroll_speed(float speed = 0.1f)
    {
        cam.scroll_speed = speed;
        return *this;
    }
    glfw_window2d_GL_v21& set_move_speed(float speed = 0.1f)
    {
        cam.move_speed = speed;
        return *this;
    }
    glfw_window2d_GL_v21& append_texture(const char* path)
    {
        if(nullptr == path) 
            texture_list.push_back(make_checker_tex());        
        //== TODO : load path
        return *this;
    }
    template<class T> glfw_window2d_GL_v21& append_texture(std::vector<T>& vec, int xsize, int ysize)
    {
        return *this;
    }
    glfw_window2d_GL_v21& async_loop(int maxFPS = 30)
    {
        glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
        t = std::thread(&glfw_window2d_GL_v21::loop, this, maxFPS);
        return *this;
    }
    void loop(int maxFPS =  0)
    {
        constexpr float display_ratio = 1.0f;
        static_assert(0 < display_ratio && display_ratio <=1.0f);

        activate().set_fps_ratio(1).set_scroll_speed(0.15).set_move_speed(2.0).append_texture(nullptr);
        
        using clock = std::chrono::high_resolution_clock;
        auto lastTime = clock::now();
        int frames = 0;
        float frameDuration = 1.0 / maxFPS;
        while (running){
            auto frameStart = clock::now();
            int w,h; glfwGetFramebufferSize(win, &w, &h);
            glViewport(0,0,w,h);
            glClearColor(1.0f, 1.0f, 1.0f,1);
            glClear(GL_COLOR_BUFFER_BIT);
            set_ortho(cam, w, h);
            
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, texture_list.back());
            glColor3f(1,1,1);
            glBegin(GL_QUADS);
            glTexCoord2f(0,0); glVertex2f(-display_ratio,-display_ratio);
            glTexCoord2f(1,0); glVertex2f( display_ratio,-display_ratio);
            glTexCoord2f(1,1); glVertex2f( display_ratio, display_ratio);
            glTexCoord2f(0,1); glVertex2f(-display_ratio, display_ratio);
            glEnd();
            glDisable(GL_TEXTURE_2D);
            
            glfwSwapBuffers(win);

            // ---- FPS 统计 ----
            constexpr float print_fps_time_in_second = 5.0; 
            if constexpr(0 < print_fps_time_in_second){
                frames++;
                auto now = clock::now();
                std::chrono::duration<float> elapsed = now - lastTime;
                if (elapsed.count() >= print_fps_time_in_second) {
                    std::cout << "FPS: " << frames / elapsed.count() << std::endl;
                    frames = 0;
                    lastTime = now;
                }
            }
            
            // ---- 帧率限制 ----
            auto frameEnd = clock::now();
            std::chrono::duration<double> frameElapsed = frameEnd - frameStart;
            double sleepTime = frameDuration - frameElapsed.count();
            if (sleepTime > 0) {
                std::this_thread::sleep_for(std::chrono::duration<float>(sleepTime));
            }
        }
        activate(false);
    }
    void event_loop()
    {
        while (!glfwWindowShouldClose(win)) {
            glfwWaitEvents();
        }
        running = false;
    }

private:
    static void set_ortho(const Ortho2D& cam, int w, int h) {
        float aspect = h > 0 ? (float)w / (float)h : 1.0f;
        float s = 1.0f / cam.zoom;
        float vw = aspect * s;
        float vh = 1.0f * s;
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-vw + cam.panX, vw + cam.panX, -vh + cam.panY, vh + cam.panY, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }
};

#pragma once
#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/glew.h>
#endif
#include "glfw_window2d_GL_v21.hpp"


// ---------- shaders ----------
static const char* vertexShaderSrc = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTex;

out vec2 TexCoord;

uniform float uZoom;   // >0
uniform vec2  uPan;    // texture-space offset

void main() {
    // Zoom around texture center (0.5,0.5), then pan.
    vec2 centered = (aTex - vec2(0.5, 0.5)) / uZoom + vec2(0.5, 0.5) + uPan;
    TexCoord = centered;
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";

static const char* fragmentShaderSrc = R"(
#version 330 core
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D tex;
void main() {
    // Sampling outside [0,1] will be handled by texture wrap mode (we set GL_CLAMP)
    FragColor = texture(tex, TexCoord);
}
)";

// ---------- helper: compile/link ----------
static GLuint compileShader(GLenum type, const char* src)
{
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok=0; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if(!ok){
        char buf[1024]; glGetShaderInfoLog(s, 1024, nullptr, buf);
        std::cerr<<"Shader compile error: "<<buf<<"\n";
    }
    return s;
}
static GLuint makeProgram()
{
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexShaderSrc);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);
    GLuint p = glCreateProgram();
    glAttachShader(p, vs);
    glAttachShader(p, fs);
    glLinkProgram(p);
    GLint ok=0; glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if(!ok){
        char buf[1024]; glGetProgramInfoLog(p, 1024, nullptr, buf);
        std::cerr<<"Program link error: "<<buf<<"\n";
    }
    glDeleteShader(vs);
    glDeleteShader(fs);
    return p;
}
// ---------- VAO for fullscreen quad ----------
static GLuint makeQuadVAO()
{
    constexpr float display_ratio = 1.0f;
    static_assert(0 < display_ratio && display_ratio <=1.0f);
    float vertices[] = {
        // pos         // tex
        -display_ratio, -display_ratio,   0.0f, 0.0f,
         display_ratio, -display_ratio,   1.0f, 0.0f,
        -display_ratio,  display_ratio,   0.0f, 1.0f,
         display_ratio,  display_ratio,   1.0f, 1.0f
    };
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // aPos location=0
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // aTex location=1
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    // We won't delete VBO here to keep data; in small sample it's fine
    return VAO;
}

struct glfw_window2d_GL_v33 final
{
    GLFWwindow* win;
    Ortho2D cam;
    std::vector<GLuint> texture_list;
    std::thread t;
    std::atomic<bool> running{true};
    GLuint program;
    GLuint vao;
    GLint locZoom = -1, locPan = -1;
    glfw_window2d_GL_v33()
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
        win = glfwCreateWindow(960, 600, "Checkerboard - zoom/pan (keyboard)", nullptr, nullptr);
        glfwSetWindowUserPointer(win, &cam);
        glfwSetKeyCallback(win, keyCallback);
        glfwSetScrollCallback(win, scrollCallback);
        glfwSetCursorPosCallback(win, cursorPosCallback);
        glfwSetMouseButtonCallback(win, mouseButtonCallback);
    }
    ~glfw_window2d_GL_v33()
    {
        if(t.joinable())t.join();
        glDeleteTextures(texture_list.size(), texture_list.data());
        if(win) glfwDestroyWindow(win);
    }
    bool valid() const
    {
        return nullptr != win;
    }
    glfw_window2d_GL_v33& activate(bool flag = true)
    {
        glfwMakeContextCurrent(flag ? win : nullptr);
        return *this;
    }
    glfw_window2d_GL_v33& set_fps_ratio(float ratio/*[0 ~ 1]*/)
    {
        ratio = std::round(1.0f / ratio);
        glfwSwapInterval(int(ratio));
        return *this;
    }
    glfw_window2d_GL_v33& set_scroll_speed(float speed = 0.1f)
    {
        cam.scroll_speed = speed;
        return *this;
    }
    glfw_window2d_GL_v33& append_texture(const char* path)
    {
        if(nullptr == path) 
            texture_list.push_back(make_checker_tex());        
        //== TODO : load path
        return *this;
    }
    glfw_window2d_GL_v33& async_loop(int maxFPS = 30)
    {
        glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
        t = std::thread(&glfw_window2d_GL_v33::loop, this, maxFPS);
        return *this;
    }
    glfw_window2d_GL_v33& loop(int maxFPS =  0)
    {
        activate().set_fps_ratio(1).set_scroll_speed(0.15).append_texture(nullptr);
#ifndef __APPLE__
        static bool init_glew = true;
        if(init_glew){
            glewExperimental = GL_TRUE;
            if(glewInit() != GLEW_OK){ std::cerr<<"glew init failed\n"; return *this; }
            init_glew = false;
        }
#endif
        // build GL resources
        program = makeProgram();
        locZoom = glGetUniformLocation(program, "uZoom");
        locPan  = glGetUniformLocation(program, "uPan");
        // ensure sampler is 0
        glUseProgram(program);
        GLint locTex = glGetUniformLocation(program, "tex");
        if(locTex >= 0) glUniform1i(locTex, 0);
        glUseProgram(0);
        vao = makeQuadVAO();
        
        using clock = std::chrono::high_resolution_clock;
        auto lastTime = clock::now();
        int frames = 0;
        float frameDuration = 1.0 / maxFPS;
        while (running){
            auto frameStart = clock::now();
            int w,h; glfwGetFramebufferSize(win, &w, &h);
            glBindVertexArray(vao);
            renderFrame(w,h);
            glBindVertexArray(0);
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

        glDeleteVertexArrays(1, &vao);
        glDeleteProgram(program);
        return *this;
    }
    glfw_window2d_GL_v33& event_loop()
    {
        while (!glfwWindowShouldClose(win)) {
            glfwWaitEvents();
        }
        running = false;
        return *this;
    }
private:
    // ---------- update GPU uniforms (call with program bound) ----------
    void uploadCameraUniforms(){
        glUniform1f(locZoom, cam.zoom);
        glUniform2f(locPan, cam.panX, cam.panY);
    }
    // ---------- render ----------
    void renderFrame(int width, int height)
    {
        glViewport(0,0,width,height);
        glClearColor(0.1f,0.1f,0.1f,1);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program);
        // upload camera uniforms
        uploadCameraUniforms();

        glActiveTexture(GL_TEXTURE0);
        //== TODO : texture_list 扩容时可能存在问题。 注意危险
        glBindTexture(GL_TEXTURE_2D, texture_list.back()); 
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);
    }
};
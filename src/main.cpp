#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>

#include <GLFW/glfw3.h>
#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

#ifdef USE_NUKLEAR
#include "nuklear.h"
#include "nuklear_glfw_gl2.h"
#endif

struct Vec2 { float x, y; };
struct Vec3 { float x, y, z; };

struct OrthoCamera2D {
    float zoom = 1.0f;
    Vec2 pan{0.0f, 0.0f};
};

struct OrbitCamera3D {
    float distance = 3.0f;
    float yaw = 0.7f;
    float pitch = 0.5f;
};

static void error_callback(int error, const char* description) {
    std::fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

static void set_ortho(const OrthoCamera2D& cam, int width, int height) {
    float aspect = (height > 0) ? (float)width / (float)height : 1.0f;
    float scale = 1.0f / cam.zoom;
    float viewW = aspect * scale;
    float viewH = 1.0f * scale;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-viewW + cam.pan.x, viewW + cam.pan.x, -viewH + cam.pan.y, viewH + cam.pan.y, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

static void set_perspective(float fovy_deg, float aspect, float znear, float zfar) {
    float f = 1.0f / std::tan(fovy_deg * 0.5f * 3.14159265f / 180.0f);
    float m[16] = {0};
    m[0] = f / aspect;
    m[5] = f;
    m[10] = (zfar + znear) / (znear - zfar);
    m[11] = -1.0f;
    m[14] = (2.0f * zfar * znear) / (znear - zfar);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(m);
    glMatrixMode(GL_MODELVIEW);
}

static void look_at(const Vec3& eye, const Vec3& center, const Vec3& up) {
    auto normalize = [](Vec3 v) {
        float len = std::sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
        return Vec3{v.x/len, v.y/len, v.z/len};
    };
    auto cross = [](Vec3 a, Vec3 b) {
        return Vec3{a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x};
    };
    auto sub = [](Vec3 a, Vec3 b) { return Vec3{a.x-b.x, a.y-b.y, a.z-b.z}; };

    Vec3 f = normalize(sub(center, eye));
    Vec3 s = normalize(cross(f, up));
    Vec3 u = cross(s, f);

    float m[16] = {
        s.x, u.x, -f.x, 0.0f,
        s.y, u.y, -f.y, 0.0f,
        s.z, u.z, -f.z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    glLoadMatrixf(m);
    glTranslatef(-eye.x, -eye.y, -eye.z);
}

int main() {
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) return 1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "display_tool", nullptr, nullptr);
    if (!window) { glfwTerminate(); return 1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

#ifdef USE_NUKLEAR
    struct nk_context* nkctx = nk_glfw3_init(window, NK_GLFW3_INSTALL_CALLBACKS);
    struct nk_font_atlas* atlas;
    nk_glfw3_font_stash_begin(&atlas);
    nk_glfw3_font_stash_end();
#endif

    OrthoCamera2D cam2d;
    OrbitCamera3D cam3d;

    bool view3d = true; // toggle between 2D and 3D
    bool dragging = false;
    bool rotating = false;
    double lastX = 0.0, lastY = 0.0;

    glfwSetScrollCallback(window, [](GLFWwindow* win, double /*xoffset*/, double yoffset){
        int w, h; glfwGetWindowSize(win, &w, &h);
        void* ptr = glfwGetWindowUserPointer(win);
        auto data = reinterpret_cast<std::pair<OrthoCamera2D*, OrbitCamera3D*>*>(ptr);
        if (!data) return;
        if (glfwGetKey(win, GLFW_KEY_TAB) == GLFW_PRESS) {
            data->second->distance *= (yoffset < 0 ? 1.1f : 0.9f);
            if (data->second->distance < 0.2f) data->second->distance = 0.2f;
        } else {
            data->first->zoom *= (yoffset > 0 ? 1.1f : 0.9f);
            if (data->first->zoom < 0.01f) data->first->zoom = 0.01f;
        }
    });

    auto cameras = std::make_pair(&cam2d, &cam3d);
    glfwSetWindowUserPointer(window, &cameras);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        int width, height; glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glClearColor(0.1f, 0.12f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        int leftMouse = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
        int rightMouse = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
        double mx, my; glfwGetCursorPos(window, &mx, &my);

        if (leftMouse == GLFW_PRESS && !dragging) { dragging = true; lastX = mx; lastY = my; }
        if (leftMouse == GLFW_RELEASE && dragging) { dragging = false; }
        if (rightMouse == GLFW_PRESS && !rotating) { rotating = true; lastX = mx; lastY = my; }
        if (rightMouse == GLFW_RELEASE && rotating) { rotating = false; }

        if (!view3d && dragging) {
            float dx = (float)(mx - lastX);
            float dy = (float)(my - lastY);
            cam2d.pan.x -= dx * 0.002f / cam2d.zoom;
            cam2d.pan.y += dy * 0.002f / cam2d.zoom;
            lastX = mx; lastY = my;
        }
        if (view3d && rotating) {
            float dx = (float)(mx - lastX);
            float dy = (float)(my - lastY);
            cam3d.yaw   += dx * 0.005f;
            cam3d.pitch += dy * 0.005f;
            if (cam3d.pitch >  1.5f) cam3d.pitch =  1.5f;
            if (cam3d.pitch < -1.5f) cam3d.pitch = -1.5f;
            lastX = mx; lastY = my;
        }

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) view3d = !view3d;

        if (!view3d) {
            glDisable(GL_DEPTH_TEST);
            set_ortho(cam2d, width, height);
            glBegin(GL_QUADS);
            glColor3f(0.8f, 0.2f, 0.2f);
            glVertex2f(-0.5f, -0.5f);
            glColor3f(0.2f, 0.8f, 0.2f);
            glVertex2f( 0.5f, -0.5f);
            glColor3f(0.2f, 0.2f, 0.8f);
            glVertex2f( 0.5f,  0.5f);
            glColor3f(0.8f, 0.8f, 0.2f);
            glVertex2f(-0.5f,  0.5f);
            glEnd();
        } else {
            glEnable(GL_DEPTH_TEST);
            float aspect = (height > 0) ? (float)width / (float)height : 1.0f;
            set_perspective(60.0f, aspect, 0.01f, 100.0f);
            glLoadIdentity();
            float cx = cam3d.distance * std::cos(cam3d.pitch) * std::cos(cam3d.yaw);
            float cy = cam3d.distance * std::sin(cam3d.pitch);
            float cz = cam3d.distance * std::cos(cam3d.pitch) * std::sin(cam3d.yaw);
            look_at(Vec3{cx, cy, cz}, Vec3{0.0f, 0.0f, 0.0f}, Vec3{0.0f, 1.0f, 0.0f});

            glBegin(GL_LINES);
            glColor3f(1,0,0); glVertex3f(0,0,0); glVertex3f(1,0,0);
            glColor3f(0,1,0); glVertex3f(0,0,0); glVertex3f(0,1,0);
            glColor3f(0,0,1); glVertex3f(0,0,0); glVertex3f(0,0,1);
            glEnd();

            struct Edge { float a[3]; float b[3]; };
            std::vector<Edge> cube = {
                {{-0.5f,-0.5f,-0.5f},{ 0.5f,-0.5f,-0.5f}},
                {{ 0.5f,-0.5f,-0.5f},{ 0.5f, 0.5f,-0.5f}},
                {{ 0.5f, 0.5f,-0.5f},{-0.5f, 0.5f,-0.5f}},
                {{-0.5f, 0.5f,-0.5f},{-0.5f,-0.5f,-0.5f}},
                {{-0.5f,-0.5f, 0.5f},{ 0.5f,-0.5f, 0.5f}},
                {{ 0.5f,-0.5f, 0.5f},{ 0.5f, 0.5f, 0.5f}},
                {{ 0.5f, 0.5f, 0.5f},{-0.5f, 0.5f, 0.5f}},
                {{-0.5f, 0.5f, 0.5f},{-0.5f,-0.5f, 0.5f}},
                {{-0.5f,-0.5f,-0.5f},{-0.5f,-0.5f, 0.5f}},
                {{ 0.5f,-0.5f,-0.5f},{ 0.5f,-0.5f, 0.5f}},
                {{ 0.5f, 0.5f,-0.5f},{ 0.5f, 0.5f, 0.5f}},
                {{-0.5f, 0.5f,-0.5f},{-0.5f, 0.5f, 0.5f}},
            };
            glColor3f(0.9f, 0.9f, 0.9f);
            glBegin(GL_LINES);
            for (auto& e : cube) { glVertex3fv(e.a); glVertex3fv(e.b); }
            glEnd();
        }

#ifdef USE_NUKLEAR
        nk_glfw3_new_frame();
        if (nk_begin(nkctx, "Controls", nk_rect(10, 10, 240, 160), NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE)) {
            nk_layout_row_dynamic(nkctx, 30, 1);
            nk_label(nkctx, "SPACE: toggle 2D/3D", NK_TEXT_LEFT);
            nk_checkbox_label(nkctx, "3D view", &view3d);
            nk_property_float(nkctx, "2D zoom", 0.01f, &cam2d.zoom, 100.0f, 0.01f, 0.005f);
            nk_property_float(nkctx, "3D distance", 0.1f, &cam3d.distance, 50.0f, 0.1f, 0.05f);
        }
        nk_end(nkctx);
        nk_glfw3_render(NK_ANTI_ALIASING_ON, 512*1024, 128*1024);
#endif

        glfwSwapBuffers(window);
    }

#ifdef USE_NUKLEAR
    nk_glfw3_shutdown();
#endif
    glfwTerminate();
    return 0;
}



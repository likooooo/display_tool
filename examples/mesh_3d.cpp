#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <vector>

#include <GLFW/glfw3.h>
#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

struct OrbitCam { float dist=3.0f; float yaw=0.7f; float pitch=0.4f; };

static void perspective(float fovy, float aspect, float zn, float zf){
    float f = 1.0f / std::tan(fovy*0.5f*3.14159265f/180.0f);
    float m[16] = {0}; m[0]=f/aspect; m[5]=f; m[10]=(zf+zn)/(zn-zf); m[11]=-1; m[14]=(2*zf*zn)/(zn-zf); m[15]=0;
    glMatrixMode(GL_PROJECTION); glLoadMatrixf(m); glMatrixMode(GL_MODELVIEW);
}

static void lookAt(float ex,float ey,float ez,float cx,float cy,float cz,float ux,float uy,float uz){
    auto norm=[&](float x,float y,float z){ float l=std::sqrt(x*x+y*y+z*z); return std::vector<float>{x/l,y/l,z/l}; };
    auto sub=[&](float ax,float ay,float az,float bx,float by,float bz){ return std::vector<float>{ax-bx,ay-by,az-bz}; };
    auto cross=[&](const std::vector<float>& a,const std::vector<float>& b){ return std::vector<float>{a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0]}; };
    auto f=norm(sub(cx,cy,cz,ex,ey,ez)[0], sub(cx,cy,cz,ex,ey,ez)[1], sub(cx,cy,cz,ex,ey,ez)[2]);
    auto s=norm(cross(f,{ux,uy,uz})[0], cross(f,{ux,uy,uz})[1], cross(f,{ux,uy,uz})[2]);
    auto u=cross(s,f);
    float m[16]={ s[0],u[0],-f[0],0, s[1],u[1],-f[1],0, s[2],u[2],-f[2],0, 0,0,0,1 };
    glLoadMatrixf(m);
    glTranslatef(-ex,-ey,-ez);
}

int main(){
    if(!glfwInit()) return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,1);
    GLFWwindow* win=glfwCreateWindow(960,600,"mesh_3d",nullptr,nullptr);
    if(!win){ glfwTerminate(); return 1; }
    glfwMakeContextCurrent(win);
    glfwSwapInterval(1);

    OrbitCam cam; bool rotating=false; double lastX=0,lastY=0;
    glfwSetWindowUserPointer(win,&cam);
    glfwSetScrollCallback(win,[](GLFWwindow* w,double, double yoff){ auto* c=(OrbitCam*)glfwGetWindowUserPointer(w); c->dist *= (yoff<0?1.1f:0.9f); if(c->dist<0.2f) c->dist=0.2f; });

    while(!glfwWindowShouldClose(win)){
        glfwPollEvents();
        int w,h; glfwGetFramebufferSize(win,&w,&h);
        glViewport(0,0,w,h);
        glClearColor(0.12f,0.13f,0.16f,1);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        int rmb=glfwGetMouseButton(win,GLFW_MOUSE_BUTTON_LEFT); double mx,my; glfwGetCursorPos(win,&mx,&my);
        if(rmb==GLFW_PRESS && !rotating){ rotating=true; lastX=mx; lastY=my; }
        if(rmb==GLFW_RELEASE && rotating){ rotating=false; }
        if(rotating){ float dx=float(mx-lastX), dy=float(my-lastY); cam.yaw+=dx*0.005f; cam.pitch+=dy*0.005f; if(cam.pitch>1.5f)cam.pitch=1.5f; if(cam.pitch<-1.5f)cam.pitch=-1.5f; lastX=mx; lastY=my; }

        perspective(60.0f, h>0?(float)w/(float)h:1.0f, 0.01f, 100.0f);
        glLoadIdentity();
        float ex=cam.dist*std::cos(cam.pitch)*std::cos(cam.yaw);
        float ey=cam.dist*std::sin(cam.pitch);
        float ez=cam.dist*std::cos(cam.pitch)*std::sin(cam.yaw);
        lookAt(ex,ey,ez, 0,0,0, 0,1,0);

        glBegin(GL_LINES);
        glColor3f(1,0,0); glVertex3f(0,0,0); glVertex3f(1,0,0);
        glColor3f(0,1,0); glVertex3f(0,0,0); glVertex3f(0,1,0);
        glColor3f(0,0,1); glVertex3f(0,0,0); glVertex3f(0,0,1);
        glEnd();

        struct E{ float a[3]; float b[3]; };
        std::vector<E> edges={
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
        glColor3f(0.9f,0.9f,0.9f);
        glBegin(GL_LINES);
        for(const auto& e:edges){ glVertex3fv(e.a); glVertex3fv(e.b);} 
        glEnd();

        glfwSwapBuffers(win);
    }
    glfwTerminate();
    return 0;
}



//
//  main.cpp
//  ZBuffer
//
//  Created by 孟文龙 on 2021/11/11.
//r
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <bitmap.h>

#include <iostream>
#include <fstream>

#include "include/load_obj.h" //必须写在fstream之后！
#include "include/rendering.h"

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const unsigned int ZOOM = 1000;
// ZOOM = 1000 表示obj中坐标变化了1相当于窗口中的坐标变化了1000

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}


void kernel(Bitmap *bitmap, int u, int v, unsigned char *ptr)
{
    int width = bitmap->x;
    int height = bitmap->y;
    int offset = v*width + u;
    for (int y=0; y<width; y++) {
        for (int x=0; x<height; x++) {
            int offset_temp = x + y*height;
            if (offset_temp < offset && offset_temp%width < u){
                ptr[offset_temp*4 + 0] = 255;
                ptr[offset_temp*4 + 1] = 255;
                ptr[offset_temp*4 + 2] = 0;
                ptr[offset_temp*4 + 3] = 255;
            }
        }
    }
}




int main(int argc, const char * argv[]) {
    
    //初始化glfw
    glfwInit();
    // glfwWindowHint一旦加上，glDrawPixels便无法使用
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // wocore，这里还必须使用core
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
#ifdef __APPLE__
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "ZBuffer", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    //将我们窗口的上下文设置为当前线程的主上下文
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    
    
    
    
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    printf("%d, %d\n", width, height);
    //让其在Apple的retina显示屏上也能正常显示
    
    Bitmap bitmap(width, height);
    unsigned char *ptr = bitmap.get_ptr();
    
    int width_t, height_t, nrChannels;
        stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    unsigned char *texture = stbi_load(std::filesystem::path("resources/SkillObj_Dummy_Tex_Diffuse.png").c_str(), &width_t, &height_t, &nrChannels, 0);
    // texture是RGBARGBARGBA
    Bitmap tex(width_t, height_t);
    tex.pixels = texture;
    kernel(&tex, 800, 600, tex.pixels);
    
    std::string objfile = "resources/AvatarSkill_Dummy_Model.obj";
    TriangleMesh mesh;
    loadObj(objfile, mesh);
    
    // 构建分类多边形表
    // classifiedPolygons是一个数组，数组中的元素是指向vector<Polygon>的指针，每个vector<Polygon>对应一条y扫描线，所以classifiedPolygons本身是一个二重指针
    std::cout << "Building classified polygon table..." << std::endl;
    vector<Polygon> classifiedPolygons[height];
    buildClassifiedPolygonTable(classifiedPolygons, mesh.faces, mesh.verts, mesh.bounding_sphere_c, ZOOM, width, height);
    // 看看分类多边形表构建得对不对
    
    
    
    while(!glfwWindowShouldClose(window))
    {
        // 输入
        processInput(window);   //按ESC退出
        
        // 渲染
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        //glDrawPixels(bitmap.x, bitmap.y, GL_RGBA, GL_UNSIGNED_BYTE, bitmap.pixels);
        glDrawPixels(width_t, height_t, GL_RGBA, GL_UNSIGNED_BYTE, texture);
        
        // 检查并调用事件，交换缓冲
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwTerminate();
    std::cout << "Hello, World!\n";
    return 0;
}


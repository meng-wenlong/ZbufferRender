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
#include "include/hierarchyZbuffer.h"

// settings
const unsigned int SCR_WIDTH = 512;     //必须是2^n
const unsigned int SCR_HEIGHT = 512;    //必须是2^n
const unsigned int ZOOM = 512;
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


int main(int argc, const char * argv[]) {
    
    //初始化glfw
    glfwInit();
    
    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "ZBuffer by mwl", NULL, NULL);
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
    
    // Bitmap bitmap(width, height);
    // unsigned char *ptr = bitmap.get_ptr();
    
    //载入纹理图片
    int width_t, height_t, nrChannels;
        stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    unsigned char *texture = stbi_load(std::filesystem::path("resources/SkillObj_Panda_Tex_Diffuse.png").c_str(), &width_t, &height_t, &nrChannels, 0);
    // texture是RGBARGBARGBA
    Bitmap tex(width_t, height_t);
    tex.pixels = texture;
    // kernel(&tex, 800, 600, tex.pixels);
    
    //载入obj文件
    std::string objfile = "resources/SkillObj_Panda_Model.obj";
    TriangleMesh mesh;
    loadObj(objfile, mesh);
    
    
    //定义帧缓冲器并初始化
    Bitmap framebuffer(width, height);
    unsigned char *ptr = framebuffer.get_ptr();
    for (int y=0; y<width; y++) {
        for (int x=0; x<height; x++) {
            int offset = x + y*height;
            ptr[offset*4 + 0] = 255;
            ptr[offset*4 + 1] = 255;
            ptr[offset*4 + 2] = 255;
            ptr[offset*4 + 3] = 255;
        }
    }
    
    //定义层次Zbuffer并初始化
    HierarchyZbuffer hierarchyZbuffer(SCR_WIDTH);
    
    //开始绘制（每个三角形）
    
    
    while(!glfwWindowShouldClose(window))
    {
        // 输入
        processInput(window);   //按ESC退出
        
        // 渲染
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        //glDrawPixels(bitmap.x, bitmap.y, GL_RGBA, GL_UNSIGNED_BYTE, bitmap.pixels);
        //glDrawPixels(width_t, height_t, GL_RGBA, GL_UNSIGNED_BYTE, texture);
        glDrawPixels(width, height, GL_RGBA, GL_UNSIGNED_BYTE, framebuffer.get_ptr());
        
        // 检查并调用事件，交换缓冲
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwTerminate();
    std::cout << "Hello, World!\n";
    return 0;
}


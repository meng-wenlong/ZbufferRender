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
#include <cmath>

#include "include/load_obj.h" //必须写在fstream之后！
#include "include/rendering.h"
#include "include/hierarchyZbuffer.h"

// settings
#define DIM 512
const unsigned int SCR_WIDTH = DIM;     //必须是2^n
const unsigned int SCR_HEIGHT = DIM;    //必须是2^n
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
    HierarchyZbuffer hierarchyZbuffer(width);
    
    //verts转化为屏幕像素坐标
    double xc = (mesh.bounding_box[0].x + mesh.bounding_box[1].x)/2;
    double yc = (mesh.bounding_box[0].y + mesh.bounding_box[1].y)/2;
    double zc = (mesh.bounding_box[0].z + mesh.bounding_box[1].z)/2;
    vector<vec3f> zoomed_verts;
    for (int i=0; i<mesh.verts.size(); i++) {
        vec3f temp;
        temp.x = (mesh.verts[i].x - xc) * ZOOM + width/2;
        temp.y = (mesh.verts[i].y - yc) * ZOOM + width/2;
        temp.z = (mesh.verts[i].z - zc) * ZOOM;
        zoomed_verts.push_back(temp);
    }
    
    //开始绘制（每个三角形）
    for (int i=0; i<mesh.faces.size(); i++) {
        //判断三角形是否被挡住
        bool passZtest = ztest(mesh.faces[i], zoomed_verts, hierarchyZbuffer);
        if (passZtest) {
            renderOneTriangle(mesh.faces[i], zoomed_verts, mesh.uvs, &tex, hierarchyZbuffer, framebuffer);
        }
    }
    
    while(!glfwWindowShouldClose(window))
    {
        // 输入
        processInput(window);   //按ESC退出
        
        // 渲染
        for (int y=0; y<width; y++) {   //fb清零
            for (int x=0; x<height; x++) {
                int offset = x + y*height;
                ptr[offset*4 + 0] = 255;
                ptr[offset*4 + 1] = 255;
                ptr[offset*4 + 2] = 255;
                ptr[offset*4 + 3] = 255;
            }
        }
        HierarchyZbuffer hierarchyZbuffer(width); //重新初始化Zbuffer
        //旋转zoomed_verts
        static double theta = 0;
        for (int i=0; i<zoomed_verts.size(); i++) {
            zoomed_verts[i].x -= width/2;
            double xt = zoomed_verts[i].x * cos(theta) + zoomed_verts[i].z * sin(theta);
            double zt = zoomed_verts[i].z * cos(theta) - zoomed_verts[i].x * sin(theta);
            zoomed_verts[i].x = xt + width/2;
            zoomed_verts[i].z = zt;
        }
        theta += 0.01;
        for (int i=0; i<mesh.faces.size(); i++) { //绘制
            //判断三角形是否被挡住
            bool passZtest = ztest(mesh.faces[i], zoomed_verts, hierarchyZbuffer);
            if (passZtest) {
                renderOneTriangle(mesh.faces[i], zoomed_verts, mesh.uvs, &tex, hierarchyZbuffer, framebuffer);
            }
        }
        
        glDrawPixels(width, height, GL_RGBA, GL_UNSIGNED_BYTE, framebuffer.get_ptr());
        
        // 检查并调用事件，交换缓冲
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwTerminate();
    std::cout << "END!\n";
    return 0;
}


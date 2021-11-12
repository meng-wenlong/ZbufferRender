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


// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
#define DIM     600

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


void kernel( unsigned char* ptr, int width, int height ) {
    for (int y=0; y<width; y++) {
        for (int x=0; x<height; x++) {
            int offset = x + y*height;
            
            ptr[offset*4 + 0] = 255;
            ptr[offset*4 + 1] = 255;
            ptr[offset*4 + 2] = 0;
            ptr[offset*4 + 3] = 255;
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
    
    //int width, height, nrChannels;
        stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    //unsigned char *data = stbi_load(std::filesystem::path("resources/textures/container.jpg").c_str(), &width, &height, &nrChannels, 0);
    // data是RGBRGBRGB
    Bitmap bitmap(width, height);
    unsigned char *ptr = bitmap.get_ptr();
    kernel(ptr, bitmap.x, bitmap.y);
    
    while(!glfwWindowShouldClose(window))
    {
        // 输入
        processInput(window);   //按ESC退出
        
        // 渲染
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawPixels(bitmap.x, bitmap.y, GL_RGBA, GL_UNSIGNED_BYTE, bitmap.pixels);
        
        // 检查并调用事件，交换缓冲
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwTerminate();
    std::cout << "Hello, World!\n";
    return 0;
}


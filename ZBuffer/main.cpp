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
const unsigned int ZOOM = 1024;
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
    unsigned char *texture = stbi_load(std::filesystem::path("resources/SkillObj_Dummy_Tex_Diffuse.png").c_str(), &width_t, &height_t, &nrChannels, 0);
    // texture是RGBARGBARGBA
    Bitmap tex(width_t, height_t);
    tex.pixels = texture;
    // kernel(&tex, 800, 600, tex.pixels);
    
    //载入obj文件
    std::string objfile = "resources/AvatarSkill_Dummy_Model.obj";
    TriangleMesh mesh;
    loadObj(objfile, mesh);
    
    // 构建分类多边形表
    // classifiedPolygons 是一个数组，数组中的元素是指向 vector<Polygon> 的指针，每个 vector<Polygon> 对应一条y扫描线，所以 classifiedPolygons 本身是一个二重指针
    std::cout << "Building classified polygon table..." << std::endl;
    vector<Polygon> classifiedPolygons[height];
    buildClassifiedPolygonTable(classifiedPolygons, mesh.faces, mesh.verts, mesh.bounding_sphere_c, ZOOM, width, height);
    // 看看分类多边形表构建得对不对
    // たぶんいい感じ
    
    // 次に、分類エッジテーブルを構築します 
    // 「分类边表」中的x坐标只需要记录上端点的
    std::cout << "Building classified edge table..." << std::endl;
    vector<CEdge> classifiedEdges[height];
    buildClassifiedEdgeTable(classifiedEdges, mesh.faces, mesh.verts, mesh.bounding_sphere_c, ZOOM, width, height);
    
    //定义活化多边形表和活化边表
    vector<Polygon> activatedPolygons;
    vector<AEdge> activatedEdges;
    
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
    //定义z缓冲器并初始化
    double *zbuffer = (double *)malloc( sizeof(double)* width * height );
    for (int y=0; y<width; y++) {
        for (int x=0; x<height; x++) {
            int offset = x + y*height;
            zbuffer[offset] = -1e12; //-10000应该是最小值了
        }
    }
    
    
    //开始扫描
    for(int i=0; i<height; i++) {
        for(int j=0; j<classifiedPolygons[i].size(); j++) { //遍历相应height的分类多边形表
            activatedPolygons.push_back(classifiedPolygons[i][j]);
            int Pid = classifiedPolygons[i][j].Pid;
            
            vector<CEdge> CEdge_in_P;
            for (int k=0; k<classifiedEdges[i].size(); k++) {
                if (classifiedEdges[i][k].Pid == Pid) {
                    CEdge_in_P.push_back(classifiedEdges[i][k]);
                }
            }
            double dzx = - classifiedPolygons[i][j].a / classifiedPolygons[i][j].c;
            double dzy = - classifiedPolygons[i][j].b / classifiedPolygons[i][j].c;
            struct AEdge AEdge_temp;
            
            if (CEdge_in_P.size() == 2) { //正常的情况
                //根据斜率判断哪条边在左
                double k0 = double( CEdge_in_P[0].xi.back().back() - CEdge_in_P[0].xi.front().front() ) / CEdge_in_P[0].dy;
                double k1 = double( CEdge_in_P[1].xi.back().back() - CEdge_in_P[1].xi.front().front() ) / CEdge_in_P[1].dy;
                if(k0 < k1) {
                    AEdge_temp = {CEdge_in_P[0].x, CEdge_in_P[0].xi, CEdge_in_P[0].dy, (CEdge_in_P[1].x), CEdge_in_P[1].xi, CEdge_in_P[1].dy, CEdge_in_P[0].z, dzx, dzy, Pid};
                } else {
                    AEdge_temp = {CEdge_in_P[1].x, CEdge_in_P[1].xi, CEdge_in_P[1].dy, CEdge_in_P[0].x, CEdge_in_P[0].xi, CEdge_in_P[0].dy, CEdge_in_P[1].z, dzx, dzy, Pid};
                }
                activatedEdges.push_back(AEdge_temp);
            } else if(CEdge_in_P.size() == 3) { // 两种特殊情况
                //三角形正好卡在扫描线上了，不渲染。
                if(CEdge_in_P[0].dy == 0 && CEdge_in_P[1].dy == 0 && CEdge_in_P[2].dy == 0) {
                    activatedPolygons.pop_back();
                } else { //有一条边卡在了扫描线上
                    //去除卡在扫描线上的边
                    for(vector<CEdge>::iterator it=CEdge_in_P.begin(); it!=CEdge_in_P.end();) {
                        if ((*it).dy == 0) {CEdge_in_P.erase(it);}
                        else {it++;}
                    }
                    if (CEdge_in_P.size()<2) {
                        std::cout << "Error in inserting AEdgeTable!" << std::endl;
                        exit(-1);
                    }
                    //根据x值大小判断哪一条边在左，哪一条边在右（x值小的在左）
                    if(CEdge_in_P[0].x < CEdge_in_P[1].x) {
                        AEdge_temp = {CEdge_in_P[0].x, CEdge_in_P[0].xi, CEdge_in_P[0].dy, CEdge_in_P[1].x, CEdge_in_P[1].xi, CEdge_in_P[1].dy, CEdge_in_P[0].z, dzx, dzy, Pid};
                    } else {
                        AEdge_temp = {CEdge_in_P[1].x, CEdge_in_P[1].xi, CEdge_in_P[1].dy, CEdge_in_P[0].x, CEdge_in_P[0].xi, CEdge_in_P[0].dy, CEdge_in_P[1].z, dzx, dzy, Pid};
                    }
                    activatedEdges.push_back(AEdge_temp);
                }
            } else {
                std::cout << "Error in inserting AEdgeTable!" << std::endl;
                exit(-1);
            }
        } //遍历相应height的分类多边形表
        
        //增量式更新
        for (int i_ae=0; i_ae<activatedEdges.size(); i_ae++) {
            int l_index = activatedEdges[i_ae].xil.size() - activatedEdges[i_ae].dyl - 1;
            int r_index = activatedEdges[i_ae].xir.size() - activatedEdges[i_ae].dyr - 1;
            int ul = activatedEdges[i_ae].xil[l_index].front();
            int ur = activatedEdges[i_ae].xir[r_index].back();
            //深度映射
            int deltax = ul - activatedEdges[i_ae].xl;
            int deltay = l_index;
            double zx = activatedEdges[i_ae].zl + activatedEdges[i_ae].dzx * deltax + activatedEdges[i_ae].dzx * deltay;
            for (int i_u=ul; i_u<=ur; i_u++) {
                //比较zx与当前zbuffer中的深度值
                //当前zbuffer的坐标（i_u, i）
                if (zx > zbuffer[i*width + i_u]) {
                    zbuffer[i*width + i_u] = zx;
                    setPixel(&framebuffer, i_u, i, mesh.faces[activatedEdges[i_ae].Pid], mesh.verts, mesh.uvs, mesh.bounding_sphere_c, &tex, ZOOM);
                    //int offset = i*width + i_u;
                    //ptr[offset*4 + 0] = 0;
                    //ptr[offset*4 + 1] = 0;
                    //ptr[offset*4 + 2] = 0;
                    //ptr[offset*4 + 3] = 255;
                }
                //每向➡️移动一个元素，更新深度值
                zx += activatedEdges[i_ae].dzx;
            }
        }
        
        //更新活化边表：dyl = dyl - 1, dyr = dyr - 1
        //如果dyl或dyr小于0，相应的边就要从一个边对中去掉，从活化边表中找到合适的边来代替
        // xl = xl + dxl, xr = xr + dxr
        // zl = zl + (dzl * dxl) + dzy
        for (vector<AEdge>::iterator it=activatedEdges.begin(); it!=activatedEdges.end();) {
            (*it).dyl -= 1;
            (*it).dyr -= 1;
            
            if ( ((*it).dyl < 0 && (*it).dyr < 0) || (*it).dyl<-1 || (*it).dyr<-1 ) {
                activatedEdges.erase(it);
            } else if( (*it).dyl < 0 ) {
                //左边先结束，更新左边
                // 遍历当前height的分类边表，找到对应Pid的边
                int flag = 0;
                for (int j=0; j<classifiedEdges[i].size(); j++) {
                    if (classifiedEdges[i][j].Pid == (*it).Pid) {
                        (*it).xl = classifiedEdges[i][j].x;
                        (*it).xil = classifiedEdges[i][j].xi;
                        (*it).dyl = classifiedEdges[i][j].dy - 1;
                        (*it).zl = classifiedEdges[i][j].z;
                        flag = 1;
                        break;
                    }
                }
                if (flag == 0) {
                    cout << "Debug" << endl;
                }
                it ++;
            } else if ( (*it).dyr < 0 ) {
                //右边先结束更新右边
                int flag = 0;
                for (int j=0; j<classifiedEdges[i].size(); j++) {
                    if (classifiedEdges[i][j].Pid == (*it).Pid) {
                        (*it).xr = classifiedEdges[i][j].x;
                        (*it).xir = classifiedEdges[i][j].xi;
                        (*it).dyr = classifiedEdges[i][j].dy - 1;
                        flag = 1;
                        break;
                    }
                }
                if (flag == 0) {
                    cout << "Debug" << endl;
                }
                it ++;
            } else { it ++; }
        }
        
        //更新活化多边形表：dy = dy - 1（似乎没什么用）
        //当dy<0时，该多边形要从活化多边形表中剔除
        for(vector<Polygon>::iterator it=activatedPolygons.begin(); it!=activatedPolygons.end();) {
            (*it).dy -= 1;
            if ( (*it).dy < 0 ) {
                activatedPolygons.erase(it);
            } else { it ++; }
        }
        
    } //处理第i条扫描线
    
    
    
    
    
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


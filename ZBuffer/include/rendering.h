//
//  rendering.h
//  ZBuffer
//
//  Created by 孟文龙 on 2021/11/12.
//

#ifndef rendering_h
#define rendering_h

#include "tri_vec.h"
#include "load_obj.h"

struct Polygon {
    double a;
    double b;
    double c;
    double d;
    int Pid;
    int dy;
};

//分类边
struct CEdge {
    int x;      //边下端点的x坐标
    double z;   //边下端点的深度值
    //double dx;  //相邻两条扫描线交点的x坐标差
    vector<vector<int>> xi; //bresenham化的边的x坐标
    int dy;     //边所跨越的扫描线数目
    int Pid;    //边所属的多边形的编号
};

//活化边
struct AEdge {
    int     xl;     //左交点的x坐标
    vector<vector<int>> xil;    //(左交点边上)两相邻扫描线交点的x坐标之差
    int     dyl;    //剩余扫描线数
    int     xr;
    vector<vector<int>> xir;
    int     dyr;
    double  zl;     //左交点处多边形所在的平面的深度值
    double  dzx;    //沿扫描线向右走过一个像素时，多边形所在的平面的深度增量
    double  dzy;    //沿y方向向下移过一根扫描线时，多边形所在的平面的深度增量
    int     Pid;
};

void getPixel(Bitmap *bitmap, int u, int v, unsigned char *ptr);
void buildClassifiedPolygonTable(vector<Polygon>* ptr, vector<TriangleFace>& faces, vector<vec3f>& verts, vec3f centor, unsigned int zoom, int width, int height);
void bresenham(vector<int> &xloc, vector<int> &yloc, int x0, int x1, int y0, int y1);
struct CEdge processOneCEdge(vec3f vert0, vec3f vert1, double yc, double xc, unsigned int zoom, int width, int height, unsigned int i);
void buildClassifiedEdgeTable(vector<CEdge>* ptr, vector<TriangleFace>& faces, vector<vec3f>& verts, vec3f centor, unsigned int zoom, int width, int height);


// 构建顶点颜色表
// 不能构建顶点颜色表，因为同一个顶点在不同面中的颜色可能不一样
// 如何获取面的颜色：面中存放了uv的id

// 根据(u, v)坐标从 bitmap 里取值放到 ptr 中
void getPixel(Bitmap *bitmap, int u, int v, unsigned char *ptr) {
    int width = bitmap->x;
    //int height = bitmap->y;
    int offset = v*width + u;
    ptr[0] = (bitmap->pixels)[offset*4 + 0];
    ptr[1] = (bitmap->pixels)[offset*4 + 1];
    ptr[2] = (bitmap->pixels)[offset*4 + 2];
    ptr[3] = (bitmap->pixels)[offset*4 + 3];
}

void setPixel(Bitmap *framebuffer, int x, int y, TriangleFace face, vector<vec3f> &verts, vector<vec2f> &uvs, vec3f center, Bitmap *tex, const unsigned int zoom) {
    double xc = center.x;
    double yc = center.y;
    int width = framebuffer->x;
    int height = framebuffer->y;
    int width_t = tex->x;
    int height_t = tex->y;
    //得到A，B，C三点的坐标
    double xA = floor( (verts[ face.v[0]-1 ].x - xc)*zoom + width/2 );
    double yA = floor( (verts[ face.v[0]-1 ].y - yc)*zoom + height/2 );
    double xB = floor( (verts[ face.v[1]-1 ].x - xc)*zoom + width/2 );
    double yB = floor( (verts[ face.v[1]-1 ].y - yc)*zoom + height/2 );
    double xC = floor( (verts[ face.v[2]-1 ].x - xc)*zoom + width/2 );
    double yC = floor( (verts[ face.v[2]-1 ].y - yc)*zoom + height/2 );
    //得到ABC三点的rgba
    unsigned char ColorA[4];
    unsigned char ColorB[4];
    unsigned char ColorC[4];
    getPixel(tex, int(width_t*uvs[ face.w[0]-1 ].u), int(height_t*uvs[ face.w[0]-1 ].v), ColorA);
    getPixel(tex, int(width_t*uvs[ face.w[1]-1 ].u), int(height_t*uvs[ face.w[1]-1 ].v), ColorB);
    getPixel(tex, int(width_t*uvs[ face.w[2]-1 ].u), int(height_t*uvs[ face.w[2]-1 ].v), ColorC);
    //得到alpha，beta，gama
    double alpha = ((xB-x)*(yC-yB) + (y-yB)*(xC-xB)) / ((xB-xA)*(yC-yB) + (yA-yB)*(xC-xB));
    double beta = ((xC-x)*(yA-yC) + (y-yC)*(xA-xC)) / ((xC-xB)*(yA-yC) + (yB-yC)*(xA-xC));
    double gama = 1 - alpha - beta;
    unsigned char *ptr = framebuffer->get_ptr();
    int offset = y*width + x;
    double uA = uvs[ face.w[0]-1 ].u;
    double vA = uvs[ face.w[0]-1 ].v;
    double uB = uvs[ face.w[1]-1 ].u;
    double vB = uvs[ face.w[1]-1 ].v;
    double uC = uvs[ face.w[2]-1 ].u;
    double vC = uvs[ face.w[2]-1 ].v;
    double u_ = alpha*uA + beta*uB + gama*uC;
    double v_ = alpha*vA + beta*vB + gama*vC;
    int u = u_ * width_t;
    int v = v_ * height_t;
    int offset_uv = v*width_t + u;
//    ptr[offset*4 + 0] = alpha*ColorA[0] + beta*ColorB[0] + gama*ColorC[0];
//    ptr[offset*4 + 1] = alpha*ColorA[1] + beta*ColorB[1] + gama*ColorC[1];
//    ptr[offset*4 + 2] = alpha*ColorA[2] + beta*ColorB[2] + gama*ColorC[2];
//    ptr[offset*4 + 3] = alpha*ColorA[3] + beta*ColorB[3] + gama*ColorC[3];
    ptr[offset*4 + 0] = tex->pixels[offset_uv*4 + 0];
    ptr[offset*4 + 1] = tex->pixels[offset_uv*4 + 1];
    ptr[offset*4 + 2] = tex->pixels[offset_uv*4 + 2];
    ptr[offset*4 + 3] = tex->pixels[offset_uv*4 + 3];
}

// 构建分类多边形表
// 引用底层也是用指针实现的，使用引用的好处是可以让代码看起来简单，并且不需要拷贝数据
void buildClassifiedPolygonTable(vector<Polygon>* ptr, vector<TriangleFace>& faces, vector<vec3f>& verts, vec3f centor, unsigned int zoom, int width, int height) {
    double yc = centor.y;
    for (unsigned int i=0; i<faces.size(); i++) {
        // faces中有三个顶点，取三个顶点的y值
        // 求出面的y跨度和v跨度
        double ymax = fmax( verts[ faces[i].v[0]-1 ].y, verts[ faces[i].v[1]-1 ].y, verts[ faces[i].v[2]-1 ].y );
        double ymin = fmin( verts[ faces[i].v[0]-1 ].y, verts[ faces[i].v[1]-1 ].y, verts[ faces[i].v[2]-1 ].y );
        int vmax = zoom*(ymax - yc) + height/2; //转化为int
        int vmin = zoom*(ymin - yc) + height/2; //转化为int
        
        // 求出面对应的a，b，c，d值
        // a = (y2 - y1)*(z3 - z1) - (y3 - y1)*(z2 - z1)
        // b = (z2 - z1)*(x3 - x1) - (z3 - z1)*(x2 - x1)
        // c = (x2 - x1)*(y3 - y1) - (x3 - x1)*(y2 - y1)
        // d = -a * x1 - b * y1 - c * z1
        double x1 = verts[ faces[i].v[0]-1 ].x * zoom;
        double x2 = verts[ faces[i].v[1]-1 ].x * zoom;
        double x3 = verts[ faces[i].v[2]-1 ].x * zoom;
        double y1 = verts[ faces[i].v[0]-1 ].y * zoom;
        double y2 = verts[ faces[i].v[1]-1 ].y * zoom;
        double y3 = verts[ faces[i].v[2]-1 ].y * zoom;
        double z1 = verts[ faces[i].v[0]-1 ].z * zoom;
        double z2 = verts[ faces[i].v[1]-1 ].z * zoom;
        double z3 = verts[ faces[i].v[2]-1 ].z * zoom;
        
        double a = (y2 - y1)*(z3 - z1) - (y3 - y1)*(z2 - z1);
        double b = (z2 - z1)*(x3 - x1) - (z3 - z1)*(x2 - x1);
        double c = (x2 - x1)*(y3 - y1) - (x3 - x1)*(y2 - y1);
        double d = -a * x1 - b * y1 - c * z1;
        
        // 暂时不需要添加颜色信息
        struct Polygon triangle = {a, b, c, d, static_cast<int>(i), (vmax-vmin)};
        ptr[vmin].push_back( triangle );
    }
}

// bresenham算法将直线光栅化
void bresenham(vector<int> &xloc, vector<int> &yloc, int x0, int x1, int y0, int y1) {
    bool steep = abs(y1 - y0) > abs(x1 - x0);
    if(steep) {
        swap(x0, y0);
        swap(x1, y1);
    }
    if (x0 > x1) {
        swap(x0, x1);
        swap(y0, y1);
    }
    int deltax = x1 - x0;
    int deltay = abs(y1 - y0);
    double error = 0;
    double deltaerr = double(deltay) / deltax;
    int ystep;
    int y = y0;
    if(y0 < y1) {
        ystep = 1;
    } else {
        ystep = -1;
    }
    for (int x=x0; x<=x1; x++) {
        if (steep) {
            xloc.push_back(y);
            yloc.push_back(x);
        } else {
            xloc.push_back(x);
            yloc.push_back(y);
        }
        error += deltaerr;
        if (error >= 0.5) {
            y += ystep;
            error -= 1;
        }
    }
}

//输入两个节点和相关参数，返回需要插入的CEdge
struct CEdge processOneCEdge(vec3f vert0, vec3f vert1, double yc, double xc, unsigned int zoom, int width, int height, unsigned int i) {
    double ymax = fmax(vert0.y, vert1.y);
    double ymin = fmin(vert0.y, vert1.y);
    int vmax = zoom*(ymax - yc) + height/2; //转化为int
    int vmin = zoom*(ymin - yc) + height/2; //转化为int
    int dy = vmax - vmin;
    
    double d_x_ymax = (vert0.y > vert1.y)? vert0.x : vert1.x;
    double d_z_ymax = (vert0.y > vert1.y)? vert0.z : vert1.z;
    double d_x_ymin = (vert0.y < vert1.y)? vert0.x : vert1.x;
    double d_z_ymin = (vert0.y < vert1.y)? vert0.z : vert1.z;
    int x_ymin = (d_x_ymin - xc) * zoom + width/2;
    double z_ymin = d_z_ymin * zoom;
    //int u_ymax = zoom*(x_ymax - xc) + width/2;
    
    // dx = -1/k = -(x2 - x1)/(y2 - y1) = (x1 - x2)/(y2 - y1)
    // double dx = 1600;
    // double delta_y = vert1.y - vert0.y;
    //相邻两条扫描线交点的x坐标差——1/k
    // double dx = (vert1.x - vert0.x) / (delta_y * zoom); // 交给编译器来处理吧
    
//    if (delta_y > GLH_EPSILON_2) {
//        dx = (vert0.x - vert1.x) / delta_y;
//    } else {dx = width;} //对斜率太小的线的处理
    // 在这里出现了问题
    vector<vector<int>> xi(dy+1);
    vector<int> xloc;
    vector<int> yloc;
    int x0 = (vert0.x - xc) * zoom + width/2;
    int x1 = (vert1.x - xc) * zoom + width/2;
    int y0 = (vert0.y - yc) * zoom + height/2;
    int y1 = (vert1.y - yc) * zoom + height/2;
    // bresenham
    bresenham(xloc, yloc, x0, x1, y0, y1);
//    if (yloc.front() > yloc.back()) { //保证y是从小到大排列
//        reverse(xloc.begin(), xloc.end());
//        reverse(yloc.begin(), yloc.end());
//    }
    for (int k=0; k<yloc.size(); k++) {
        xi[yloc[k] - vmin].push_back(xloc[k]);
    }
    
    struct CEdge tri_edge = {x_ymin, z_ymin, xi, dy, static_cast<int>(i)};
    return tri_edge;
}

// 生成分类边表
void buildClassifiedEdgeTable(vector<CEdge>* ptr, vector<TriangleFace>& faces, vector<vec3f>& verts, vec3f centor, unsigned int zoom, int width, int height) {
    double yc = centor.y;
    double xc = centor.x;
    for (unsigned int i=0; i<faces.size(); i++) {
        //每个三角形有三条边【0，1】【0，2】【1，2】
        // --- 处理边【0，1】---
        // double ymax = fmax( verts[ faces[i].v[0] ].y, verts[ faces[i].v[1] ].y );
        // int vmax = zoom*(ymax - yc) + height/2; //转化为int
        double ymin = fmin( verts[ faces[i].v[0]-1 ].y, verts[ faces[i].v[1]-1 ].y );
        int vmin = zoom*(ymin - yc) + height/2; //转化为int
        struct CEdge tri_edge = processOneCEdge(verts[ faces[i].v[0]-1 ], verts[ faces[i].v[1]-1 ], yc, xc, zoom, width, height, i);
        // push
        ptr[vmin].push_back( tri_edge );
        
        // --- 处理边【0，2】---
        ymin = fmin( verts[ faces[i].v[0]-1 ].y, verts[ faces[i].v[2]-1 ].y );
        vmin = zoom*(ymin - yc) + height/2; //转化为int
        tri_edge = processOneCEdge(verts[ faces[i].v[0]-1 ], verts[ faces[i].v[2]-1 ], yc, xc, zoom, width, height, i);
        // push
        ptr[vmin].push_back( tri_edge );
        
        // --- 处理边【1，2】---
        ymin = fmin( verts[ faces[i].v[1]-1 ].y, verts[ faces[i].v[2]-1 ].y );
        vmin = zoom*(ymin - yc) + height/2; //转化为int
        tri_edge = processOneCEdge(verts[ faces[i].v[1]-1 ], verts[ faces[i].v[2]-1 ], yc, xc, zoom, width, height, i);
        // push
        ptr[vmin].push_back( tri_edge );
    }
}

#endif /* rendering_h */

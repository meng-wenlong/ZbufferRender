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
    //int x;      //边上端点的x坐标 (应该是double型的)
    double x;
    double z;   //边上端点的深度值
    double dx;  //相邻两条扫描线交点的x坐标差
    int dy;     //边所跨越的扫描线数目
    int Pid;    //边所属的多边形的编号
};

//活化边
struct AEdge {
    double  xl;     //左交点的x坐标
    double  dxl;    //(左交点边上)两相邻扫描线交点的x坐标之差
    int     dyl;    //剩余扫描线数
    double  xr;
    double  dxr;
    int     dyr;
    double  zl;     //左交点处多边形所在的平面的深度值
    double  dzx;    //沿扫描线向右走过一个像素时，多边形所在的平面的深度增量
    double  dzy;    //沿y方向向下移过一根扫描线时，多边形所在的平面的深度增量
    int     Pid;
};

void getPixel(Bitmap *bitmap, int u, int v, unsigned char *ptr);
void buildClassifiedPolygonTable(vector<Polygon>* ptr, vector<TriangleFace>& faces, vector<vec3f>& verts, vec3f centor, unsigned int zoom, int width, int height);
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

// 构建分类多边形表
// 引用底层也是用指针实现的，使用引用的好处是可以让代码看起来简单，并且不需要拷贝数据
void buildClassifiedPolygonTable(vector<Polygon>* ptr, vector<TriangleFace>& faces, vector<vec3f>& verts, vec3f centor, unsigned int zoom, int width, int height) {
    double yc = centor.y;
    for (unsigned int i=0; i<faces.size(); i++) {
        // faces中有三个顶点，取三个顶点的y值
        // 求出面的y跨度和v跨度
        double ymax = fmax( verts[ faces[i].v[0] ].y, verts[ faces[i].v[1] ].y, verts[ faces[i].v[2] ].y );
        double ymin = fmin( verts[ faces[i].v[0] ].y, verts[ faces[i].v[1] ].y, verts[ faces[i].v[2] ].y );
        int vmax = zoom*(ymax - yc) + height/2; //转化为int
        int vmin = zoom*(ymin - yc) + height/2; //转化为int
        
        // 求出面对应的a，b，c，d值
        // a = (y2 - y1)*(z3 - z1) - (y3 - y1)*(z2 - z1)
        // b = (z2 - z1)*(x3 - x1) - (z3 - z1)*(x2 - x1)
        // c = (x2 - x1)*(y3 - y1) - (x3 - x1)*(y2 - y1)
        // d = -a * x1 - b * y1 - c * z1
        double x1 = verts[ faces[i].v[0] ].x;
        double x2 = verts[ faces[i].v[1] ].x;
        double x3 = verts[ faces[i].v[2] ].x;
        double y1 = verts[ faces[i].v[0] ].y;
        double y2 = verts[ faces[i].v[1] ].y;
        double y3 = verts[ faces[i].v[2] ].y;
        double z1 = verts[ faces[i].v[0] ].z;
        double z2 = verts[ faces[i].v[1] ].z;
        double z3 = verts[ faces[i].v[2] ].z;
        
        double a = (y2 - y1)*(z3 - z1) - (y3 - y1)*(z2 - z1);
        double b = (z2 - z1)*(x3 - x1) - (z3 - z1)*(x2 - x1);
        double c = (x2 - x1)*(y3 - y1) - (x3 - x1)*(y2 - y1);
        double d = -a * x1 - b * y1 - c * z1;
        
        // 暂时不需要添加颜色信息
        struct Polygon triangle = {a, b, c, d, static_cast<int>(i), (vmax-vmin)};
        ptr[vmin].push_back( triangle );
    }
}

//输入两个节点和相关参数，返回需要插入的CEdge
struct CEdge processOneCEdge(vec3f vert0, vec3f vert1, double yc, double xc, unsigned int zoom, int width, int height, unsigned int i) {
    double ymax = fmax(vert0.y, vert1.y);
    double ymin = fmin(vert0.y, vert1.y);
    int vmax = zoom*(ymax - yc) + height/2; //转化为int
    int vmin = zoom*(ymin - yc) + height/2; //转化为int
    int dy = vmax - vmin;
    
    double x_ymax = (vert0.y > vert1.y)? vert0.x : vert1.x;
    double z_ymax = (vert0.y > vert1.y)? vert0.z : vert1.z;
    double x_ymin = (vert0.y < vert1.y)? vert0.x : vert1.x;
    double z_ymin = (vert0.y < vert1.y)? vert0.z : vert1.z;
    //int u_ymax = zoom*(x_ymax - xc) + width/2;
    
    // dx = -1/k = -(x2 - x1)/(y2 - y1) = (x1 - x2)/(y2 - y1)
    // double dx = 1600;
    double delta_y = vert1.y - vert0.y;
    //相邻两条扫描线交点的x坐标差——1/k
    double dx = (vert1.x - vert0.x) / (delta_y * zoom); // 交给编译器来处理吧
//    if (delta_y > GLH_EPSILON_2) {
//        dx = (vert0.x - vert1.x) / delta_y;
//    } else {dx = width;} //对斜率太小的线的处理
    
    struct CEdge tri_edge = {x_ymin, z_ymin, dx, dy, static_cast<int>(i)};
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
        double ymin = fmin( verts[ faces[i].v[0] ].y, verts[ faces[i].v[1] ].y );
        int vmin = zoom*(ymin - yc) + height/2; //转化为int
        struct CEdge tri_edge = processOneCEdge(verts[ faces[i].v[0] ], verts[ faces[i].v[1] ], yc, xc, zoom, width, height, i);
        // push
        ptr[vmin].push_back( tri_edge );
        
        // --- 处理边【0，2】---
        ymin = fmin( verts[ faces[i].v[0] ].y, verts[ faces[i].v[2] ].y );
        vmin = zoom*(ymin - yc) + height/2; //转化为int
        tri_edge = processOneCEdge(verts[ faces[i].v[0] ], verts[ faces[i].v[2] ], yc, xc, zoom, width, height, i);
        // push
        ptr[vmin].push_back( tri_edge );
        
        // --- 处理边【1，2】---
        ymin = fmin( verts[ faces[i].v[1] ].y, verts[ faces[i].v[2] ].y );
        vmin = zoom*(ymin - yc) + height/2; //转化为int
        tri_edge = processOneCEdge(verts[ faces[i].v[1] ], verts[ faces[i].v[2] ], yc, xc, zoom, width, height, i);
        // push
        ptr[vmin].push_back( tri_edge );
    }
}

#endif /* rendering_h */

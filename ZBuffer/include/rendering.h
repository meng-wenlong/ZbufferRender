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


void getPixel(Bitmap *bitmap, int u, int v, unsigned char *ptr);
void buildClassifiedPolygonTable(vector<Polygon>* ptr, vector<TriangleFace>& faces, vector<vec3f>& verts, vec3f centor, unsigned int zoom, int width, int height);


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
    for (unsigned int i=0; i<faces.size(); i++) {
        // faces中有三个顶点，取三个顶点的y值
        // 求出面的y跨度和v跨度
        double ymax = fmax( verts[ faces[i].v[0] ].y, verts[ faces[i].v[1] ].y, verts[ faces[i].v[2] ].y );
        double ymin = fmin( verts[ faces[i].v[0] ].y, verts[ faces[i].v[1] ].y, verts[ faces[i].v[2] ].y );
        double yc = centor.y;
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
        struct Polygon triangle = {a, b, c, d, static_cast<int>(i), (vmax-vmin+1)};
        ptr[vmax].push_back( triangle );
    }
}

#endif /* rendering_h */

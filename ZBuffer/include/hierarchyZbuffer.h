//
//  hierarchyZbuffer.h
//  ZBuffer
//
//  Created by 孟文龙 on 2021/11/24.
//  层次ZBuffer

#ifndef hierarchyZbuffer_h
#define hierarchyZbuffer_h

#include <vector>
#include <queue>
#include "bitmap.h"
#include "tri_vec.h"
#include "rendering.h"

using namespace std;

double min2(double a, double b) {
    return (a<b)?a:b;
}

double min3(double a, double b, double c) {
    return min2( min2(a, b), c );
}

double min4(double a, double b, double c, double d) {
    return min2( min2(a, b), min2(c, d) );
}

double max2(double a, double b) {
    return (a>b)?a:b;
}

double max3(double a, double b, double c) {
    return max2( max2(a, b), c );
}

double max4(double a, double b, double c, double d) {
    return max2( max2(a, b), max2(c, d) );
}


class QuardtreeNode {
public:
    vec2f boundary[2];
    int depth;
    double z;
    bool divided;
    QuardtreeNode *father;
    
    //子节点
    QuardtreeNode *nw;
    QuardtreeNode *ne;
    QuardtreeNode *sw;
    QuardtreeNode *se;
    
    QuardtreeNode(QuardtreeNode *father, vec2f *bound) {
        //拷贝边界
        memcpy(boundary, bound, 2*sizeof(vec2f));
        if (father == NULL) {
            depth = 0;
        } else {
            depth = father->depth + 1;
        }
        z = -1e12;
        divided = false;
        this->father = father;
        
        nw = NULL;
        ne = NULL;
        sw = NULL;
        se = NULL;
    }
    ~QuardtreeNode() {
        delete nw;
        delete ne;
        delete sw;
        delete se;
    }
    
    void subdivide() {
        double umin = boundary[0].u;
        double vmin = boundary[0].v;
        double umax = boundary[1].u;
        double vmax = boundary[1].v;
        double uhalf = umin + (umax - umin + 1)/2;
        double vhalf = vmin + (vmax - vmin + 1)/2;
        
        vec2f nw_b[2];
        nw_b[0].u = umin;
        nw_b[1].u = uhalf - 1; //从 min 到 half 要减去1
        nw_b[0].v = vhalf;
        nw_b[1].v = vmax;
        nw = new QuardtreeNode(this, nw_b);
        vec2f ne_b[2];
        ne_b[0].u = uhalf;
        ne_b[1].u = umax;
        ne_b[0].v = vhalf;
        ne_b[1].v = vmax;
        ne = new QuardtreeNode(this, ne_b);
        vec2f sw_b[2];
        sw_b[0].u = umin;
        sw_b[1].u = uhalf - 1;
        sw_b[0].v = vmin;
        sw_b[1].v = vhalf - 1;
        sw = new QuardtreeNode(this, sw_b);
        vec2f se_b[2];
        se_b[0].u = uhalf;
        se_b[1].u = umax;
        se_b[0].v = vmin;
        se_b[1].v = vhalf - 1;
        se = new QuardtreeNode(this, se_b);
        
        divided = true;
    }
    
    bool inThisNode(TriangleFace *face, vec3f *verts) {
        double xA = floor( verts[face->v[0] - 1].x );
        double yA = floor( verts[face->v[0] - 1].y );
        double xB = floor( verts[face->v[1] - 1].x );
        double yB = floor( verts[face->v[1] - 1].y );
        double xC = floor( verts[face->v[2] - 1].x );
        double yC = floor( verts[face->v[2] - 1].y );
        double xmin = boundary[0].u;
        double xmax = boundary[1].u;
        double ymin = boundary[0].v;
        double ymax = boundary[1].v;
        
        bool AinNode = xA>=xmin && xA<=xmax && yA>=ymin && yA<=ymax;
        bool BinNode = xB>=xmin && xB<=xmax && yB>=ymin && yB<=ymax;
        bool CinNode = xC>=xmin && xC<=xmax && yC>=ymin && yC<=ymax;
        
        if (AinNode && BinNode && CinNode) {
            return true;
        } else {
            return false;
        }
    }
    
    //更新z的时候检查是否要更新父节点的z
    void updateZ(double z) {
        this->z = z;
        if (father != NULL) {
            double zmin = min4(father->nw->z, father->ne->z, father->sw->z, father->se->z);
            if (zmin != father->z) { father->updateZ(zmin); }
        }
    }
    
};

//层次ZBuffer
class HierarchyZbuffer {
public:
    double *zbuffer;             //记录最底层信息
    QuardtreeNode *Quadtree;     //记录上层信息
    vector<QuardtreeNode *> zbuffer_toQuadtree;  //zbuffer到四叉树叶子节点上的映射
    
    //假设width == height == DIM
    //DIM必须是2^n
    HierarchyZbuffer(int DIM) {
        zbuffer_toQuadtree.insert(zbuffer_toQuadtree.begin(), DIM*DIM, NULL);
        
        //初始化zbuffer
        zbuffer = (double *)malloc( sizeof(double)* DIM * DIM );
        for (int y=0; y<DIM; y++) {
            for (int x=0; x<DIM; x++) {
                int offset = x + y*DIM;
                zbuffer[offset] = -1e12;
            }
        }
        
        //建立四叉树并初始化
        //广度优先遍历
        vec2f boundary[2];
        boundary[0].u = 0;
        boundary[0].v = 0;
        boundary[1].u = DIM - 1;
        boundary[1].v = DIM - 1;
        Quadtree = new QuardtreeNode(NULL, boundary); //根节点
        
        queue<QuardtreeNode *> nodeQueue;
        nodeQueue.push(Quadtree);
        QuardtreeNode *node;
        int max_depth = log2(DIM);
        while (!nodeQueue.empty()) {
            node = nodeQueue.front();
            nodeQueue.pop();
            if (node->depth < max_depth) {
                node->subdivide();
            } else {
                int x = node->boundary[0].u;
                int y = node->boundary[0].v;
                int offset = y*DIM + x;
                zbuffer_toQuadtree[offset] = node;
            }
            if (node->divided) {
                nodeQueue.push(node->nw);
                nodeQueue.push(node->ne);
                nodeQueue.push(node->sw);
                nodeQueue.push(node->se);
            }
        }
    }
    ~HierarchyZbuffer() {
        free(zbuffer);
        delete Quadtree;
    }
};

bool ztest(TriangleFace face, vector<vec3f> &verts, HierarchyZbuffer &hierarchyZbuffer) {
    QuardtreeNode *node = hierarchyZbuffer.Quadtree;
    bool inNode = true;
    while (inNode) {
        if (node->nw->inThisNode(&face, verts.data())) {
            node = node->nw;
        } else if(node->ne->inThisNode(&face, verts.data())) {
            node = node->ne;
        } else if(node->sw->inThisNode(&face, verts.data())) {
            node = node->sw;
        } else if(node->se->inThisNode(&face, verts.data())) {
            node = node->se;
        } else {
            inNode = false;
        }
    }
    //node指向的节点是完全包含该三角形的
    
    double zA = verts[face.v[0] - 1].z;
    double zB = verts[face.v[1] - 1].z;
    double zC = verts[face.v[2] - 1].z;
    double zmax = max3(zA, zB, zC);
    if (zmax < node->z) {
        return false;
    }
    
    return true;
}

vec3f findBottomVec(vec3f A, vec3f B, vec3f C) {
    if (A.y < B.y && A.y < C.y) {
        return A;
    }
    if (B.y < A.y && B.y < C.y) {
        return B;
    }
    if (C.y < A.y && C.y < B.y) {
        return C;
    }
    vector<vec3f> temp;
    temp.push_back(A);
    temp.push_back(B);
    temp.push_back(C);
    double ymax = max3(A.y, B.y, C.y);
    for (vector<vec3f>::iterator it = temp.begin(); it != temp.end();) {
        if ((*it).y == ymax) {
            it = temp.erase(it);
        } else {
            it ++;
        }
    }
    //这下子，temp中只有两个vec
    if (temp.size() != 2) {
        printf("Error in finding bottom vec\n");
        exit(-1);
    }
    if (temp[0].x < temp[1].x) {
        return temp[0];
    } else {
        return temp[1];
    }
}

// 绘制一个三角形
void renderOneTriangle(TriangleFace face, vector<vec3f> &verts, vector<vec2f> uvs, Bitmap *tex, HierarchyZbuffer &hierarchyZbuffer, Bitmap &framebuffer) {
    int width = framebuffer.x;
    // int height = framebuffer.y;
    int width_t = tex->x;
    int height_t = tex->y;
    //得到y的最大值和最小值
    double xA = floor( verts[face.v[0]-1].x );
    double yA = floor( verts[face.v[0]-1].y );
    double xB = floor( verts[face.v[1]-1].x );
    double yB = floor( verts[face.v[1]-1].y );
    double xC = floor( verts[face.v[2]-1].x );
    double yC = floor( verts[face.v[2]-1].y );
    double zA = verts[face.v[0]-1].z;
    double zB = verts[face.v[1]-1].z;
    double zC = verts[face.v[2]-1].z;
    
    int ymin = min3(yA, yB, yC);
    int ymax = max3(yA, yB, yC);
    int dy = ymax - ymin;
    
    //找到左下的那个顶点
    vec3f bottomVec = findBottomVec(verts[face.v[0]-1], verts[face.v[1]-1], verts[face.v[2]-1]);
    int x_bottom = floor(bottomVec.x);
    double z_bottom = bottomVec.z;
    
    double a = (yB - yA)*(zC - zA) - (yC - yA)*(zB - zA);
    double b = (zB - zA)*(xC - xA) - (zC - zA)*(xB - xA);
    double c = (xB - xA)*(yC - yA) - (xC - xA)*(yB - yA);
    double dzx = -a / c;
    double dzy = -b / c;
    //得到ABC三点的rgba
//    unsigned char ColorA[4];
//    unsigned char ColorB[4];
//    unsigned char ColorC[4];
//    getPixel(tex, int(width_t*uvs[ face.w[0]-1 ].u), int(height_t*uvs[ face.w[0]-1 ].v), ColorA);
//    getPixel(tex, int(width_t*uvs[ face.w[1]-1 ].u), int(height_t*uvs[ face.w[1]-1 ].v), ColorB);
//    getPixel(tex, int(width_t*uvs[ face.w[2]-1 ].u), int(height_t*uvs[ face.w[2]-1 ].v), ColorC);
    //两个y长度的数组，一个存储x的最小值，一个存储x的最大值
    vector<int> xmin(dy+1, 65535); //初始化
    vector<int> xmax(dy+1, 0); //初始化
    vector<int> xloc;
    vector<int> yloc;
    bresenham(xloc, yloc, xA, xB, yA, yB);
    for (int k=0; k<yloc.size(); k++) {
        if (xloc[k] < xmin[yloc[k] - ymin]) {
            xmin[yloc[k] - ymin] = xloc[k];
        }
        if (xloc[k] > xmax[yloc[k] - ymin]) {
            xmax[yloc[k] - ymin] = xloc[k];
        }
    }
    xloc.clear();
    yloc.clear();
    bresenham(xloc, yloc, xA, xC, yA, yC);
    for (int k=0; k<yloc.size(); k++) {
        if (xloc[k] < xmin[yloc[k] - ymin]) {
            xmin[yloc[k] - ymin] = xloc[k];
        }
        if (xloc[k] > xmax[yloc[k] - ymin]) {
            xmax[yloc[k] - ymin] = xloc[k];
        }
    }
    xloc.clear();
    yloc.clear();
    bresenham(xloc, yloc, xB, xC, yB, yC);
    for (int k=0; k<yloc.size(); k++) {
        if (xloc[k] < xmin[yloc[k] - ymin]) {
            xmin[yloc[k] - ymin] = xloc[k];
        }
        if (xloc[k] > xmax[yloc[k] - ymin]) {
            xmax[yloc[k] - ymin] = xloc[k];
        }
    }
    //扫描线填充
    for (int y=ymin; y<=ymax; y++) {
        //计算深度
        int deltax = xmin[y-ymin] - x_bottom;
        int deltay = y - ymin;
        double zx = z_bottom + dzx*deltax + dzy*deltay;
        for (int x=xmin[y-ymin]; x<=xmax[y-ymin]; x++) {
            if (zx > hierarchyZbuffer.zbuffer[y*width + x]) {
                //更新深度
                hierarchyZbuffer.zbuffer[y*width + x] = zx;
                hierarchyZbuffer.zbuffer_toQuadtree[y*width + x]->updateZ(zx);
                
                double alpha = ((xB-x)*(yC-yB) + (y-yB)*(xC-xB)) / ((xB-xA)*(yC-yB) + (yA-yB)*(xC-xB));
                double beta = ((xC-x)*(yA-yC) + (y-yC)*(xA-xC)) / ((xC-xB)*(yA-yC) + (yB-yC)*(xA-xC));
                double gama = 1 - alpha - beta;
                unsigned char *ptr = framebuffer.get_ptr();
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
                ptr[offset*4 + 0] = tex->pixels[offset_uv*4 + 0];
                ptr[offset*4 + 1] = tex->pixels[offset_uv*4 + 1];
                ptr[offset*4 + 2] = tex->pixels[offset_uv*4 + 2];
                ptr[offset*4 + 3] = tex->pixels[offset_uv*4 + 3];
                
            }
            zx += dzx;
        }
    }
}



#endif /* hierarchyZbuffer_h */

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

double min4(double a, double b, double c, double d) {
    return min2( min2(a, b), min2(c, d) );
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
        this->father = father;
        divided = false;
        if (father == NULL) {
            depth = 0;
        } else {
            depth = father->depth + 1;
        }
        
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
        double xA = verts[face->v[0] - 1].x;
        double yA = verts[face->v[0] - 1].y;
        double xB = verts[face->v[1] - 1].x;
        double yB = verts[face->v[1] - 1].y;
        double xC = verts[face->v[2] - 1].x;
        double yC = verts[face->v[2] - 1].y;
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
            if (zmin < father->z) { father->updateZ(zmin); }
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


#endif /* hierarchyZbuffer_h */

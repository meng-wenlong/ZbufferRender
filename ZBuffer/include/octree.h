//
//  octree.h
//  ZBuffer
//
//  Created by 孟文龙 on 2021/11/25.
//

#ifndef octree_h
#define octree_h

#include "hierarchyZbuffer.h"

bool contains(vec3f *boundary, TriangleFace *face, vec3f *verts) {
    vec3f A;
    A.x = verts[face->v[0] - 1].x;
    A.y = verts[face->v[0] - 1].y;
    A.z = verts[face->v[0] - 1].z;
    vec3f B;
    B.x = verts[face->v[1] - 1].x;
    B.y = verts[face->v[1] - 1].y;
    B.z = verts[face->v[1] - 1].z;
    vec3f C;
    C.x = verts[face->v[2] - 1].x;
    C.y = verts[face->v[2] - 1].y;
    C.z = verts[face->v[2] - 1].z;
    
    bool inside = false;
    if (A.x >= boundary[0].x && A.x <= boundary[1].x &&
        A.y >= boundary[0].y && A.y <= boundary[1].y &&
        A.z >= boundary[0].z && A.z <= boundary[1].z) {
        inside = true;
    } else if (B.x >= boundary[0].x && B.x <= boundary[1].x &&
               B.y >= boundary[0].y && B.y <= boundary[1].y &&
               B.z >= boundary[0].z && B.z <= boundary[1].z) {
        inside = true;
    } else if (C.x >= boundary[0].x && C.x <= boundary[1].x &&
               C.y >= boundary[0].y && C.y <= boundary[1].y &&
               C.z >= boundary[0].z && C.z <= boundary[1].z) {
        inside = true;
    }
    
    return inside;
}


class OctreeNode {
public:
    vec3f boundary[2];
    int capability;
    vector<TriangleFace *> faces;
    bool divided;
    
    //子节点
    OctreeNode *nwf;
    OctreeNode *nef;
    OctreeNode *swf;
    OctreeNode *sef;
    OctreeNode *nwb;
    OctreeNode *neb;
    OctreeNode *swb;
    OctreeNode *seb;
    
    OctreeNode(vec3f *bound, int n) {
        memcpy(boundary, bound, 2*sizeof(vec3f));
        capability = n;
        divided = false;
        
        nwf = NULL;
        nef = NULL;
        swf = NULL;
        sef = NULL;
        nwb = NULL;
        neb = NULL;
        swb = NULL;
        seb = NULL;
    }
    ~OctreeNode() {
        delete nwf;
        delete nef;
        delete swf;
        delete sef;
        delete nwb;
        delete neb;
        delete swb;
        delete seb;
    }
    
    void subdivide() {
        double xmin = boundary[0].x;
        double ymin = boundary[0].y;
        double zmin = boundary[0].z;
        double xmax = boundary[1].x;
        double ymax = boundary[1].y;
        double zmax = boundary[1].z;
        double xhalf = xmin + (xmax - xmin + 1)/2;
        double yhalf = ymin + (ymax - ymin + 1)/2;
        double zhalf = zmin + (zmax - zmin)/2;
        
        //从 min 到 half 要减去1
        vec3f nwf_b[2];
        nwf_b[0].x = xmin;
        nwf_b[1].x = xhalf - 1;
        nwf_b[0].y = yhalf;
        nwf_b[1].y = ymax;
        nwf_b[0].z = zhalf;
        nwf_b[1].z = zmax;
        nwf = new OctreeNode(nwf_b, capability);
        
        vec3f nef_b[2];
        nef_b[0].x = xhalf;
        nef_b[1].x = xmax;
        nef_b[0].y = yhalf;
        nef_b[1].y = ymax;
        nef_b[0].z = zhalf;
        nef_b[1].z = zmax;
        nef = new OctreeNode(nef_b, capability);
        
        vec3f swf_b[2];
        swf_b[0].x = xmin;
        swf_b[1].x = xhalf - 1;
        swf_b[0].y = ymin;
        swf_b[1].y = yhalf - 1;
        swf_b[0].z = zhalf;
        swf_b[1].z = zmax;
        swf = new OctreeNode(swf_b, capability);
        
        vec3f sef_b[2];
        sef_b[0].x = xhalf;
        sef_b[1].x = xmax;
        sef_b[0].y = ymin;
        sef_b[1].y = yhalf - 1;
        sef_b[0].z = zhalf;
        sef_b[1].z = zmax;
        sef = new OctreeNode(sef_b, capability);
        
        vec3f nwb_b[2];
        nwb_b[0].x = xmin;
        nwb_b[1].x = xhalf - 1;
        nwb_b[0].y = yhalf;
        nwb_b[1].y = ymax;
        nwb_b[0].z = zmin;
        nwb_b[1].z = zhalf;
        nwb = new OctreeNode(nwb_b, capability);
        
        vec3f neb_b[2];
        neb_b[0].x = xhalf;
        neb_b[1].x = xmax;
        neb_b[0].y = yhalf;
        neb_b[1].y = ymax;
        neb_b[0].z = zmin;
        neb_b[1].z = zhalf;
        neb = new OctreeNode(neb_b, capability);
        
        vec3f swb_b[2];
        swb_b[0].x = xmin;
        swb_b[1].x = xhalf - 1;
        swb_b[0].y = ymin;
        swb_b[1].y = yhalf - 1;
        swb_b[0].z = zmin;
        swb_b[1].z = zhalf;
        swb = new OctreeNode(swb_b, capability);
        
        vec3f seb_b[2];
        seb_b[0].x = xhalf;
        seb_b[1].x = xmax;
        seb_b[0].y = ymin;
        seb_b[1].y = yhalf - 1;
        seb_b[0].z = zmin;
        seb_b[1].z = zhalf;
        seb = new OctreeNode(seb_b, capability);
        
        divided = true;
    }
    
    bool insrt(TriangleFace *face, vec3f *verts) {
        if (!contains(boundary, face, verts)) { //不在空间内
            return false;
        }
        
        if (faces.size() < capability) {    //在空间内，且空间没满
            faces.push_back(face);
            return true;
        } else {
            if (!divided) {     //在空间内，空间满了。如果没有分割，先分割空间
                this->subdivide();
                for (int i=0; i<faces.size(); i++) {
                    this->insrt(faces[i], verts);
                }
            }
            //在空间内，空间满了。插入子空间
            bool insert = false;
            if (nwf->insrt(face, verts)) {insert = true;}
            if (nef->insrt(face, verts)) {insert = true;}
            if (swf->insrt(face, verts)) {insert = true;}
            if (sef->insrt(face, verts)) {insert = true;}
            if (nwb->insrt(face, verts)) {insert = true;}
            if (neb->insrt(face, verts)) {insert = true;}
            if (swb->insrt(face, verts)) {insert = true;}
            if (seb->insrt(face, verts)) {insert = true;}
            return insert;
        }
    }
};

bool cubeZtest(OctreeNode *cube, vector<vec3f> &verts, HierarchyZbuffer &hierarchyZbuffer) {
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
    //node指向的节点是完全包含该立方体的
    if (cube->boundary[1].z < node->z) {
        return false;
    }
    
    return true;
}

#endif /* octree_h */

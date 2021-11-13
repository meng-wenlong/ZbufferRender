//
//  load_obj.h
//  ZBuffer
//
//  Created by 孟文龙 on 2021/11/12.
//

#ifndef load_obj_h
#define load_obj_h

#include "tri_vec.h"
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

#define dot(u, v) ((u).x * (v).x + (u).y * (v).y + (u).z * (v).z)
#define norm2(v) dot(v, v)     // norm2 = squared length of vector
#define norm(v) sqrt(norm2(v)) // norm = length of vector
#define d(u, v) norm(u - v)    // distance = norm of difference

// Obj loader
struct TriangleFace
{
    int v[3]; // vertex indices
    int w[3]; // uv indices
};

// vector是一个能够存放任意类型的动态数组
// 容器特性：
// 1.顺序序列
// 2.动态数组
// 3.能够感知内存分配器的
struct TriangleMesh
{
    vector<vec3f> verts;        // 顶点容器
    vector<vec2f> uvs;          // uv容器
    vector<TriangleFace> faces; // 面容器
    //模型的包围盒
    vec3f bounding_box[2];
    //模型的包围球参数
    vec3f bounding_sphere_c;    // 中心坐标
    float bounding_sphere_r;    // 半径
};

//TriangleMesh mesh;

void loadObj(const std::string filename, TriangleMesh &mesh);

void loadObj(const std::string filename, TriangleMesh &mesh)
{
    std::ifstream in(filename.c_str());

    if (!in.good())
    {
        cout << "ERROR: loading obj:(" << filename << ") file is not good"
             << "\n";
        exit(0);
    }

    char buffer[256], str[255];
    double f1, f2, f3;

    while (!in.getline(buffer, 255).eof())
    {
        buffer[255] = '\0';
        sscanf(buffer, "%s", str, 255); //sscanf_s为Windows下特有

        // reading a vertex
        if (buffer[0] == 'v' && (buffer[1] == ' ' || buffer[1] == 32))
        {
            if (sscanf(buffer, "v %lf %lf %lf", &f1, &f2, &f3) == 3)
            {
                mesh.verts.push_back(vec3f(f1, f2, f3));
            }
            else
            {
                cout << "ERROR: vertex not in wanted format in OBJLoader"
                     << "\n";
                exit(-1);
            }
        }
        // reading a uv
        else if (buffer[0] == 'v' && buffer[1] == 't' && (buffer[2] == ' ' || buffer[2] == 32))
        {
            if (sscanf(buffer, "vt %lf %lf", &f1, &f2) == 2)
            {
                mesh.uvs.push_back(vec2f(f1, f2));
            }
            else
            {
                cout << "ERROR: uv not in wanted format in OBJLoader"
                     << "\n";
                exit(-1);
            }
        }
        // reading FaceMtls
        else if (buffer[0] == 'f' && (buffer[1] == ' ' || buffer[1] == 32))
        {
            TriangleFace f;
            int nt = sscanf(buffer, "f %d/%d/%*d %d/%d/%*d %d/%d/%*d", &f.v[0], &f.w[0], &f.v[1], &f.w[1], &f.v[2], &f.w[2]);
            if (nt != 6)
            {
                cout << "ERROR: I don't know the format of that FaceMtl"
                     << "\n";
                exit(-1);
            }

            mesh.faces.push_back(f);
        }
    }
    float xmin, ymin, zmin, xmax, ymax, zmax;
    int Pxmin, Pxmax, Pymin, Pymax, Pzmin, Pzmax;

    //calculate the bounding sphere
    xmin = xmax = mesh.verts[0].x;
    ymin = ymax = mesh.verts[0].y;
    zmin = zmax = mesh.verts[0].z;
    Pxmin = Pxmax = Pymin = Pymax = Pzmin = Pzmax = 0;

    //calculate the bounding box
    mesh.bounding_box[0] = vec3f(mesh.verts[0].x, mesh.verts[0].y, mesh.verts[0].z);
    mesh.bounding_box[1] = vec3f(mesh.verts[0].x, mesh.verts[0].y, mesh.verts[0].z);

    for (unsigned int i = 1; i < mesh.verts.size(); i++)
    {
        //update min value
        mesh.bounding_box[0].x = min(mesh.verts[i].x, mesh.bounding_box[0].x);
        mesh.bounding_box[0].y = min(mesh.verts[i].y, mesh.bounding_box[0].y);
        mesh.bounding_box[0].z = min(mesh.verts[i].z, mesh.bounding_box[0].z);

        //update max value
        mesh.bounding_box[1].x = max(mesh.verts[i].x, mesh.bounding_box[1].x);
        mesh.bounding_box[1].y = max(mesh.verts[i].y, mesh.bounding_box[1].y);
        mesh.bounding_box[1].z = max(mesh.verts[i].z, mesh.bounding_box[1].z);

        //update the  x min and max
        if (mesh.verts[i].x < xmin)
        {
            xmin = mesh.verts[i].x;
            Pxmin = i;
        }
        else if (mesh.verts[i].x > xmax)
        {
            xmax = mesh.verts[i].x;
            Pxmax = i;
        }
        //update the y min and max
        if (mesh.verts[i].y < ymin)
        {
            ymin = mesh.verts[i].y;
            Pymin = i;
        }
        else if (mesh.verts[i].y > ymax)
        {
            ymax = mesh.verts[i].y;
            Pymax = i;
        }
        //update the z min and max
        if (mesh.verts[i].z < zmin)
        {
            zmin = mesh.verts[i].z;
            Pzmin = i;
        }
        else if (mesh.verts[i].z > zmax)
        {
            zmax = mesh.verts[i].z;
            Pzmax = i;
        }
    }

    //calculate the bounding sphere
    vec3f dVx = mesh.verts[Pxmax] - mesh.verts[Pxmin];
    vec3f dVy = mesh.verts[Pymax] - mesh.verts[Pymin];
    vec3f dVz = mesh.verts[Pzmax] - mesh.verts[Pzmin];
    float dx2 = norm2(dVx);
    float dy2 = norm2(dVy);
    float dz2 = norm2(dVz);

    vec3f center;
    float radius2;
    float radius;

    if (dx2 >= dy2 && dx2 >= dz2)
    {                                                // x direction is largest extent
        center = mesh.verts[Pxmin] + (dVx / 2.0);    // Center = midpoint of extremes
        radius2 = norm2(mesh.verts[Pxmax] - center); // radius squared
    }
    else if (dy2 >= dx2 && dy2 >= dz2)
    {                                                // y direction is largest extent
        center = mesh.verts[Pymin] + (dVy / 2.0);    // Center = midpoint of extremes
        radius2 = norm2(mesh.verts[Pymax] - center); // radius squared
    }
    else
    {
        center = mesh.verts[Pzmin] + (dVz / 2.0);    // Center = midpoint of extremes
        radius2 = norm2(mesh.verts[Pzmax] - center); // radius squared
    }

    radius = sqrt(radius2);

    // now check that all points V[i] are in the ball
    // and if not, expand the ball just enough to include them
    vec3f dV;
    float dist2, dist;
    for (unsigned int i = 0; i < mesh.verts.size(); i++)
    {
        dV = mesh.verts[i] - center;
        dist2 = norm2(dV);
        if (dist2 <= radius2) // V[i] is inside the ball already
            continue;

        // V[i] not in ball, so expand ball to include it
        dist = sqrt(dist2);
        radius = (radius + dist) / 2.0; // enlarge radius just enough
        radius2 = radius * radius;
        center = center + ((dist - radius) / dist) * dV; // shift Center toward V[i]
    }

    mesh.bounding_sphere_c = center;
    mesh.bounding_sphere_r = radius;
    
    // claculate uv bounding
    double umin = mesh.uvs[0].u;
    double umax = umin;
    double vmin = mesh.uvs[0].v;
    double vmax = vmin;
    for (unsigned int i=0; i<mesh.uvs.size(); i++)
    {
        double u = mesh.uvs[i].u;
        double v = mesh.uvs[i].v;
        if (u > umax) {umax = u;}
        else if (u < umin) {umin = u;}
        if (v > vmax) {vmax = v;}
        else if (v < vmin) {vmin = v;}
    }

    cout << "----------obj file loaded-------------" << endl;
    cout << "number of faces:" << mesh.faces.size() << " number of vertices:" << mesh.verts.size() << endl;
    cout << "obj bounding box: min:("
         << mesh.bounding_box[0].x << "," << mesh.bounding_box[0].y << "," << mesh.bounding_box[0].z << ") max:("
         << mesh.bounding_box[1].x << "," << mesh.bounding_box[1].y << "," << mesh.bounding_box[1].z << ")" << endl
         << "obj bounding sphere center:" << mesh.bounding_sphere_c.x << "," << mesh.bounding_sphere_c.y << "," << mesh.bounding_sphere_c.z << endl
         << "obj bounding sphere radius:" << mesh.bounding_sphere_r << endl;
    cout << "uv bounding: min:("
         << umin << "," << vmin << ") max:("
         << umax << "," << vmax << ")" << endl;
}

#endif /* load_obj_h */

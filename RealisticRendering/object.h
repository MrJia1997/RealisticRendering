#pragma once

#include <vector>
#include <map>
#include "Vec.h"


// Implementing Half-edge structure

typedef trimesh::point point;
typedef trimesh::vec3 vec3f;
typedef trimesh::vec4 vec4f;

class vertex;
class half_edge;
class face;

class vertex {
public:
    int         id;
    point       position;
    vec3f       normal;
    vec3f       texCoord;
    vec4f       color;
    half_edge   *pEdge;
    int         degree;

public:
    vertex(const vec3f& p = vec3f(0.f, 0.f, 0.f),
        const vec3f& n = vec3f(0.f, 0.f, 0.f),
        const vec4f& c = vec4f(1.f, 1.f, 1.f, 1.f)) :
        id(-1),
        position(p),
        normal(n),
        pEdge(nullptr),
        degree(0),
        color(c) {}
    ~vertex() {}

public:
    int     get_id() { return id;}
    vec3f&  get_position() { return position; }
    vec3f&  get_normal() { return normal; }
    vec3f&  get_texCoord() { return texCoord; }
    vec4f&  get_color() { return color; }
    int     get_degree() { return degree; }

    void set_id(const int i) { id = i; }
    void set_position(const vec3f& p) { position = p; }
    void set_normal(const vec3f& n) { normal = n; }
    void set_color(const vec4f& c) { color = c; }

};

class half_edge {
public:
    int         id;
    
    vertex      *pVertex;  // vertex at the end of the half edge
    vec3f       texCoord;  // texCoord of the vertex above
    vec3f       normal;
    face        *pFace;
    
    half_edge   *pOppo;
    half_edge   *pNext;
    half_edge   *pPrev;

public:
    half_edge() :
        id(-1),
        pVertex(nullptr),
        pFace(nullptr),
        pOppo(nullptr),
        pNext(nullptr),
        pPrev(nullptr) {}
    ~half_edge() {}

public:
    int get_id() { return id;}

    void set_id(int i) { id = i; }
};

class face {
public:
    int         id;
    vec3f       normal;
    vec4f       color;

    half_edge   *pEdge; // one of the half-edges on the face boundary
    int         valence;

public:
    face() :
        id(-1),
        pEdge(nullptr),
        valence(0) {}
    ~face() {}

public:
    int     get_id() { return id; }
    vec3f&  get_normal() { return normal; }
    vec4f&  get_color() { return color; }
    int     get_valence() { return valence; }

    void set_id(int i) { id = i; }
    void set_normal(const vec3f& n) { normal = n; }
    void set_color(const vec4f& c) { color = c; }
};

class object {
private:
    std::vector<vertex*>    vertices;
    std::vector<half_edge*> halfEdges;
    std::vector<face*>      faces;

    std::map<std::pair<vertex*, vertex*>, half_edge*> halfEdgesMap;

    int numParts;

    float xmin, ymin, zmin, xmax, ymax, zmax;

public:
    object();
    ~object();

    std::vector<vertex*>    get_vertices() { return vertices; }
    std::vector<half_edge*> get_half_edges() { return halfEdges; }
    std::vector<face*>      get_faces() { return faces; }

    vertex*     insert_vertex(const vec3f& v);
    half_edge*  insert_half_edge(vertex* start, vertex* end);
    face*       insert_face(std::vector<vertex*>& vs);

    void update_boundingbox();
    void normalize(float size);

    int read_obj_file(std::string fileName);

    std::vector<vertex> raw_data();

private:
    void clear_vertices();
    void clear_half_edges();
    void clear_faces();
    void clear_all();
};
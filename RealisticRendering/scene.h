#pragma once
#include "object.h"
#include <vector>
#include <map>

#include <QMatrix4x4>

#include <CGAL/Simple_cartesian.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_triangle_primitive.h>
#include <CGAL/centroid.h>

#include <CGAL/intersections.h>
#include <CGAL/Bbox_3.h>

typedef CGAL::Simple_cartesian<double> K;
typedef K::Point_3 Point;
typedef K::Segment_3 Segment;
typedef K::Vector_3 Vector;
typedef K::Triangle_3 Triangle;
typedef K::Ray_3 Ray;

typedef std::vector<Triangle>::iterator Iterator;
typedef CGAL::AABB_triangle_primitive<K, Iterator> Primitive;

typedef CGAL::AABB_traits<K, Primitive> Traits;
typedef Traits::Point_and_primitive_id Point_and_Primitive_id;
typedef CGAL::AABB_tree<Traits> Tree;
typedef Tree::Primitive_id Primitive_id;
typedef Tree::Object_and_primitive_id Object_and_Primitive_id;

struct TreeandTri {
    std::vector<K::Triangle_3> triangles;
    Tree tree;
};


class scene {
public:
    std::vector<object*> objects;
    std::map<object*, QMatrix4x4> transMatrices;
    std::vector<TreeandTri*> aabbTrees;

public:
    scene() {}
    ~scene();
    
    void clear_all();
    int read_scene_file(std::string fileName);
    void build_aabb_trees();
};
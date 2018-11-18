#pragma once
#include "object.h"
#include <vector>
#include <map>
#include <Eigen/Dense>

typedef Eigen::Matrix4f Matrix4f;

class scene {
public:
    std::vector<object*> objects;
    std::map<object*, Matrix4f> transMatrices;
    
public:
    scene() {}
    ~scene();
    
    void clear_all();
    int read_scene_file(std::string fileName);
};
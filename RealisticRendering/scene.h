#pragma once
#include "object.h"
#include <vector>
#include <map>
#include <QMatrix4x4>

class scene {
public:
    std::vector<object*> objects;
    std::map<object*, QMatrix4x4> transMatrices;
    
public:
    scene() {}
    ~scene();
    
    void clear_all();
    int read_scene_file(std::string fileName);
};
#include "scene.h"
#include <QString>
#include <fstream>
#include <sstream>

#include <CGAL/Polyhedron_3.h>
#include <CGAL/AABB_face_graph_triangle_primitive.h>

scene::~scene() {
    clear_all();
}

void scene::clear_all() {
    for (auto o : objects) {
        if (o != nullptr) {
            delete o;
        }
    }
    for (auto t : aabbTrees) {
        if (t != nullptr) {
            delete t;
        }
    }
    objects.clear();
    aabbTrees.clear();
    transMatrices.clear();
}

std::string get_path(std::string fileName) {
    QString qFileName = QString::fromStdString(fileName);
    int first = qFileName.lastIndexOf("/");
    QString qRes = qFileName.left(first + 1);
    return qRes.toStdString();
}

bool is_absolute_path(std::string fileName) {
    if (fileName.length() == 0)
        return false;

    // Windows style
    if (fileName.length() > 3 && fileName[1] == ':' && fileName[2] == '/')
        return true;
    // *nix style
    if (fileName[0] == '/')
        return true;

    return false;
}

int scene::read_scene_file(std::string fileName) {
    // octothrope(#) for comment
    // O for obj file name (in double quotes)
    // S for size restriction
    // T for transformation matrix
    clear_all();

    std::ifstream fileIn(fileName, std::ios::in);
    if (!fileIn)
        return -1;

    std::string scenePath = get_path(fileName);

    std::string buf;
    object* tempObject = nullptr;
    try {
        while (std::getline(fileIn, buf)) {
            // split on spaces
            std::istringstream iss(buf);
            std::vector<std::string> res(std::istream_iterator<std::string>{iss},
                std::istream_iterator<std::string>());

            if (res.size() == 0)
                continue;

            if (res[0].size() > 0 && res[0][0] == '#')
                continue;

            if (res[0] == "O") {
                tempObject = nullptr;
                if (res.size() >= 2) {
                    std::string objName = buf.substr(2);
                    size_t len = objName.length();
                    if (len <= 2 || objName[0] != '"' || objName[len - 1] != '"')
                        continue;

                    // delete double quotes
                    objName = objName.substr(1, len - 2);
                    // add path
                    if (!is_absolute_path(objName))
                        objName = scenePath + objName;
                    
                    object *o = new object();
                    int readRes = o->read_obj_file(objName);
                    if (readRes == -1) {
                        delete o;
                        continue;
                    }

                    objects.push_back(o);
                    tempObject = o;
                }
                else {
                    throw std::length_error("obj file name error");
                }
            }
            else if (res[0] == "S") {
                if (res.size() == 2) {
                    float size = std::stof(res[1]);
                    tempObject->normalize(size);
                }
                else {
                    throw std::length_error("size error");
                }
            }
            else if (res[0] == "T") {
                QMatrix4x4 trans;
                trans.setToIdentity();

                for (int i = 0; i < 4; i++) {
                    std::getline(fileIn, buf);
                    std::istringstream issTrans(buf);
                    std::vector<std::string> resTrans(std::istream_iterator<std::string>{issTrans},
                        std::istream_iterator<std::string>());

                    for (int j = 0; j < 4; j++) {
                        trans(i, j) = std::stof(resTrans[j]);
                    }
                }

                if (tempObject == nullptr)
                    continue;
                transMatrices.insert(std::make_pair(tempObject, trans));
            }
        }
    }
    catch (...) {
        clear_all();
        fileIn.close();
        return -1;
    }

    qDebug() << "Read obj file over.";

    for (auto o : objects) {
        QMatrix4x4 trans = transMatrices[o];
        std::vector<vertex*> vertices = o->get_vertices();
        for (auto v : vertices) {
            QVector4D coor(v->position.x, v->position.y, v->position.z, 1.0);
            QVector4D afterTrans = trans * coor;
            v->position = vec3f(afterTrans.x(), afterTrans.y(), afterTrans.z());
        }
    }

    qDebug() << "Calculate transform over.";

    build_aabb_trees();

    return 0;
}

void scene::build_aabb_trees() {
    for (auto o : objects) {
        TreeandTri *t = new TreeandTri;

        t->triangles.resize(o->get_faces().size());
        std::vector<face*> fs = o->get_faces();
        for (auto f : fs) {
            Point p1, p2, p3;
            vertex* v1 = f->pEdge->pVertex,
                *v2 = f->pEdge->pNext->pVertex,
                *v3 = f->pEdge->pNext->pNext->pVertex;
            p1 = Point(v1->position.x, v1->position.y, v1->position.z);
            p2 = Point(v2->position.x, v2->position.y, v2->position.z);
            p3 = Point(v3->position.x, v3->position.y, v3->position.z);
            Triangle tri(p1, p2, p3);
            t->triangles.push_back(tri);
        }

        
        t->tree.rebuild(t->triangles.begin(), t->triangles.end());
        t->tree.accelerate_distance_queries();

        aabbTrees.push_back(t);

        qDebug() << "Build object aabb tree over.";
    }

}

#include "scene.h"
#include <QString>
#include <fstream>
#include <sstream>

scene::~scene() {
    clear_all();
}

void scene::clear_all() {
    for (auto o : objects) {
        if (o != nullptr) {
            delete o;
        }
    }
    objects.clear();
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

    return 0;
}

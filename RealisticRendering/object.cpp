#include "object.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <QString>
#include <QDebug>

object::object() {
    vertices = std::vector<vertex*>();
    halfEdges = std::vector<half_edge*>();
    faces = std::vector<face*>();

    xmax = ymax = zmax = 1.f;
    xmin = ymin = zmin = -1.f;

    numParts = 0;
}

object::~object() {
    clear_all();
}

void object::clear_vertices() {
    for (auto v : vertices) {
        if (v != nullptr) {
            delete v;
        }
        else {
            qCritical() << "Delete vertex nullptr.";
            exit(-1);
        }
    }
        
    vertices.clear();
}

void object::clear_half_edges() {
    for (auto he : halfEdges) {
        if (he != nullptr) {
            delete he;
        }
        else {
            qCritical() << "Delete half-edge nullptr.";
            exit(-1);
        }
    }

    halfEdges.clear();
}

void object::clear_faces() {
    for (auto f : faces) {
        if (f != nullptr) {
            delete f;
        }
        else {
            qCritical() << "Delete face nullptr.";
            exit(-1);
        }
    }

    faces.clear();
}

void object::clear_all(){
    clear_vertices();
    clear_half_edges();
    clear_faces();
    halfEdgesMap.clear();

    xmax = ymax = zmax = 1.f;
    xmin = ymin = zmin = -1.f;

    numParts = 0;
}

vertex* object::insert_vertex(const vec3f & v) {
    vertex* pV = new vertex(v);
    
    pV->set_id(static_cast<int>(vertices.size()));
    vertices.push_back(pV);
    return pV;
}

half_edge* object::insert_half_edge(vertex * start, vertex * end) {
    if (start == nullptr || end == nullptr)
        return nullptr;

    auto findRes = halfEdgesMap.find(std::make_pair(start, end));
    if (findRes != halfEdgesMap.end())
        return findRes->second;

    half_edge* pHE = new half_edge();
    pHE->pVertex = end;
    pHE->pVertex->degree++;
    start->pEdge = pHE;

    halfEdgesMap.insert({ std::make_pair(start, end), pHE });
    
    pHE->set_id(static_cast<int>(halfEdges.size()));
    halfEdges.push_back(pHE);
    return pHE;
}

face* object::insert_face(std::vector<vertex*>& vs) {
    int valence = static_cast<int>(vs.size());
    if (valence < 3)
        return nullptr;

    face* pF = new face();
    pF->valence = valence;

    half_edge *pHE1 = nullptr, *pHE2 = nullptr;
    std::vector<half_edge*> tempHalfEdges;

    for (int i = 0; i < valence; i++) {
        pHE1 = insert_half_edge(vs[i], vs[(i + 1) % valence]);
        pHE2 = insert_half_edge(vs[(i + 1) % valence], vs[i]);

        if (pF->pEdge == nullptr)
            pF->pEdge = pHE1;


        pHE1->pFace = pF;
        pHE1->pOppo = pHE2;
        pHE2->pOppo = pHE1;
        tempHalfEdges.push_back(pHE1);
    }

    for (int i = 0; i < valence; i++) {
        tempHalfEdges[i]->pNext = tempHalfEdges[(i + 1) % valence];
        tempHalfEdges[(i + 1) % valence]->pPrev = tempHalfEdges[i];
    }

    pF->set_id(static_cast<int>(faces.size()));
    faces.push_back(pF);
    return pF;
}

void object::update_boundingbox() {
#define COOR_MIN static_cast<float>(1e10)
#define COOR_MAX static_cast<float>(-1e10)
     
    xmax = ymax = zmax = COOR_MIN;
    xmin = ymin = zmin = COOR_MAX;

    for (auto v : vertices) {
        xmin = std::min(xmin, v->position.x);
        ymin = std::min(ymin, v->position.y);
        zmin = std::min(zmin, v->position.z);
        xmax = std::max(xmax, v->position.x);
        ymax = std::max(ymax, v->position.y);
        zmax = std::max(zmax, v->position.z);
    }
     
}

void object::normalize(float size) {
    float rangeX = xmax - xmin,
        rangeY = ymax - ymin,
        rangeZ = zmax - zmin;

    float rangeMax = std::max(rangeX, std::max(rangeY, rangeZ));
    float scale = size / rangeMax;

    vec3f center;
    center.x = (xmax + xmin) / 2.f;
    center.y = (ymax + ymin) / 2.f;
    center.z = (zmax + zmin) / 2.f;

    for (auto v : vertices) {
        v->position = (v->position - center) * scale;
    }
}

int object::read_obj_file(std::string fileName) {
    // TODO: support g
    // TODO: support usemtl
    
    clear_all();

    std::ifstream fileIn(fileName, std::ios::in);
    if (!fileIn)
        return -1;
    
    std::string buf;
    try {
        std::vector<vec3f> tempTexCoord, tempNormal;
        
        while (std::getline(fileIn, buf)) {
            // split on spaces
            std::istringstream iss(buf);
            std::vector<std::string> res(std::istream_iterator<std::string>{iss}, 
                std::istream_iterator<std::string>());

            if (res.size() == 0)
                continue;
            
            if (res[0].size() > 0 && res[0][0] == '#')
                continue;

            if (res[0] == "v") {
                if (res.size() >= 4) {
                    vec3f v;
                    v.x = std::stof(res[1]);
                    v.y = std::stof(res[2]);
                    v.z = std::stof(res[3]);
                    // TODO: support W, now default 1.0

                    insert_vertex(v);
                }
                else {
                    throw std::length_error("vertex length error");
                }
            }
            else if (res[0] == "vt") {
                if (res.size() >= 3) {
                    vec3f texCoord;
                    texCoord.x = std::stof(res[1]);
                    texCoord.y = std::stof(res[2]);
                    // TODO: support W, now default 0.0;
                    texCoord.z = 0.0;

                    tempTexCoord.push_back(texCoord);
                }
                else {
                    throw std::length_error("vertex texture length error");
                }
            }
            else if (res[0] == "vn") {
                if (res.size() == 4) {
                    vec3f n;
                    n.x = std::stof(res[1]);
                    n.y = std::stof(res[2]);
                    n.z = std::stof(res[3]);

                    tempNormal.push_back(n);
                }
                else {
                    throw std::length_error("vertex normal length error");
                }
            }
            else if (res[0] == "f") {
                if (res.size() >= 4) {
                    std::vector<vertex*> vs;
                    std::vector<int> vts, vns;
                    for (int i = 1; i < res.size(); i++) {
                        QString temp = QString::fromStdString(res[i]);
                        QStringList split = temp.split("/");
                        
                        int v = -1, vt = -1, vn = -1;
                        v = split.at(0).toInt() - 1;
                        if (split.size() >= 2 && split.at(1).length())
                            vt = split.at(1).toInt() - 1;
                        if (split.size() >= 3)
                            vn = split.at(2).toInt() - 1;

                        vertex* vert = vertices[v];
                        vs.push_back(vert);
                        vts.push_back(vt);
                        vns.push_back(vn);
                    }
                    if (static_cast<int>(vs.size()) >= 3) {
                        face *f = insert_face(vs);
                        int valence = f->get_valence();
                        half_edge *p = f->pEdge;
                        for (int i = 0; i < valence; i++) {
                            if (vts[i] != -1) {
                                p->texCoord = tempTexCoord[vts[i]];
                                p->pVertex->texCoord = tempTexCoord[vts[i]];
                            }
                            if (vns[i] != -1) {
                                p->normal = tempNormal[vns[i]];
                            }
                            p = p->pNext;
                        }
                    }
                    else {
                        throw std::length_error("face length error");
                    }
                }
                else {
                    throw std::length_error("face length error");
                }
            }
        }

    }
    catch (...) {
        clear_all();
        fileIn.close();
        return -1;
    }

    update_boundingbox();
    // TODO: update normal

    return 0;
}


#include "Object.h"
#include "Mesh.h"
#include <iostream>

Object::Object() {
    m_meshList = list<IMesh *>(0);
}

Object::Object(string fileName, string textureName) {
    m_meshList = list<IMesh *>(0);
    Mesh *mesh = new Mesh(fileName, textureName);
    m_meshList.push_back(mesh);
}

Object::~Object() {
    list<IMesh *>::iterator mesh;
    for (mesh = m_meshList.begin(); mesh != m_meshList.end(); ++mesh) {
        delete *mesh;
    }
}

list<IMesh *>* Object::getMeshes() {
    return &m_meshList;
}

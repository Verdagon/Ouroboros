#include "Object.h"
#include "Mesh.h"
#include <iostream>

Object::Object() {
    m_meshList = list<IMesh *>(0);
}

Object::Object(string fileName, string textureName) {
    m_meshList = list<IMesh *>(0);
    m_mesh = new Mesh(fileName, textureName);
    m_meshList.push_back(m_mesh);
}

Object::~Object() {
    list<IMesh *>::iterator mesh;
    for (mesh = m_meshList.begin(); mesh != m_meshList.end(); ++mesh) {
        delete *mesh;
    }
}

void Object::setLoc(vec3 loc) {
    m_mesh->meshMtx = mat4::Translate(loc.x, loc.y, loc.z);
    m_mesh->meshMtx = mat4::Rotate(90, vec3(0, 1, 0)) * m_mesh->meshMtx;
}

list<IMesh *>* Object::getMeshes() {
    return &m_meshList;
}

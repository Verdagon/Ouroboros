#include "TextObject.h"
#include "Mesh.h"
#include <iostream>



TextObject::TextObject() {
    m_meshList = list<IMesh *>(0);
}

TextObject::TextObject(ivec2 dimensions, ivec2 loc) {
    m_dimensions = dimensions;
    m_loc = loc;
    m_meshList = list<IMesh *>(0);
    //push in one plane 
    m_plane = new Plane("charmap_white.png", vec3(100, 100, 0), vec3(loc.x, loc.y, 0));
    m_plane->display = false;
}

TextObject::~TextObject() {
    list<IMesh *>::iterator mesh;
    for (mesh = m_meshList.begin(); mesh != m_meshList.end(); ++mesh) {
        delete *mesh;
    }
}

list<IMesh *>* TextObject::getMeshes() {
    // Push in the master plane to get 
    if (m_meshList.size() == 0)
        m_meshList.push_back(m_plane);
    return &m_meshList;
}

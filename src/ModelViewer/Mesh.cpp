#include <iostream>
#include "Mesh.h"

Mesh::Mesh(string meshName, string textureName) {
    m_meshName = meshName;
    m_textureName = textureName;
    meshRef = MeshRef();
    meshMtx = mat4::Identity();
    textureMtx =  mat4::Identity();
    color = vec4(0.8, 0.8, 0.8, 1.0);
    normalType = LOAD_NORMAL_VERTEX;
    display = true;
    size = 4;
}

Mesh::~Mesh() {
    
}

string Mesh::getMeshName() {
    return m_meshName;
}

string Mesh::getTextureName() {
    return m_textureName;
}

MeshData* Mesh::getMeshData() {
    return NULL;
}

ImageData* Mesh::getImageData() {
    return NULL;
}
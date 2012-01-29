#include <iostream>
#include "Plane.h"

Plane::Plane(string textureName, vec3 dimensions, vec3 loc) : Mesh("Plane", textureName) {
    m_dimensions = dimensions;
    m_location = loc;
    meshMtx = mat4::Translate(loc.x, loc.y, loc.z);
    meshRef = MeshRef();
    meshMtx = mat4::Identity();
    textureMtx =  mat4::Identity();
    color = vec4(0.8, 0.8, 0.8, 1.0);
    normalType = LOAD_NORMAL_FACE;
    display = true;
}

Plane::Plane(Plane *previousPlane) : Mesh("Plane", previousPlane->getTextureName()){
    m_dimensions = previousPlane->m_dimensions;
    m_location = previousPlane->m_location;
    meshMtx = mat4::Translate(m_location.x, m_location.y, m_location.z);
    meshRef = previousPlane->meshRef;
    textureRef = previousPlane->textureRef;
    textureMtx = previousPlane->textureMtx;
    color = previousPlane->color;
    normalType = LOAD_NORMAL_FACE;
    display = true;
}

Plane::~Plane() {
    
}

MeshData* Plane::getMeshData() {
    float verts[] = {
        -0.5, -0.5, 0.0,   0.0, 0.0, 1.0,    0.0, 0.0, 0.0,   0.0, 1.0,
        -0.5,  0.5, 0.0,   0.0, 0.0, 1.0,    0.0, 0.0, 0.0,   0.0, 0.0,
         0.5,  0.5, 0.0,   0.0, 0.0, 1.0,    0.0, 0.0, 0.0,   1.0, 0.0,
         0.5, -0.5, 0.0,   0.0, 0.0, 1.0,    0.0, 0.0, 0.0,   1.0, 1.0
    };
    
    MeshData *meshData = new MeshData();
    meshData->vertexCount = 4;
    meshData->vertices = new float[44];
    for (int i = 0; i < 44; i++) {
        if ((i % 11) == 0)
            meshData->vertices[i] = m_dimensions.x * verts[i];
        else if ((i % 11) == 1)
            meshData->vertices[i] = m_dimensions.y * verts[i];
        else
            meshData->vertices[i] = verts[i];
    }
    
    meshData->indexCount = 6;
    meshData->indices = new unsigned short[6];
    meshData->indices[0] = 0;
    meshData->indices[1] = 1;
    meshData->indices[2] = 2;
    meshData->indices[3] = 2;
    meshData->indices[4] = 3;
    meshData->indices[5] = 0;
    
    return meshData;
}
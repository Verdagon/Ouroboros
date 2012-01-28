#include "Plane.hpp"

using namespace std;

Plane::Plane(int width, int height) {
    m_size = ivec2(width/2, height/2);
}

Plane::~Plane() {

}

int Plane::GetVertexCount() const {
    return 4;
}

int Plane::GetTriangleIndexCount() const {
    return 6;
}

void Plane::GenerateVertices(vector<float> &vertices) const {
    
    //vertices.resize(32);
    vertices.resize(20);
    /*
    for (int i = 0; i < 32; i++) {
        if (i % 8 == 0) {
            vertices[i] = Vertices[i] * m_size.x;
        } else if (i % 8 == 1) {
            vertices[i] = Vertices[i] * m_size.y;
        } else {
            vertices[i] = Vertices[i];
        }
    }*/
    for (int i = 0; i < 20; i++) {
        if (i % 5 == 0) {
            vertices[i] = Vertices[i] * m_size.x;
        } else if (i % 5 == 1) {
            vertices[i] = Vertices[i] * m_size.y;
        } else {
            vertices[i] = Vertices[i];
        }
    }
}

void Plane::GenerateTriangleIndices(vector<unsigned short> &indices) const {
    
    indices.resize(6);
    for (int i = 0; i < 6; i++) {
        indices[i] = Indices[i];
    }
}
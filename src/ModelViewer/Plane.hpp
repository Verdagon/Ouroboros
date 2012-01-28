#ifndef PLANE_H
#define PLANE_H

#include "Interfaces.hpp"
#include <iostream>
#include <fstream>

const float Vertices[] = {
    -1, 1, 0, 0, 0,
    1, 1, 0, 1, 0,
    1, -1, 0, 1, 1,
    -1, -1, 0, 0, 1
};

const short Indices [] = {
    0, 1, 2, 0, 2, 3
};

class Plane : public IMesh {
public:
    Plane(int width, int height);
    ~Plane();
    int GetVertexCount() const;
    int GetTriangleIndexCount() const;
    void GenerateVertices(vector<float>& vertices) const;
    void GenerateTriangleIndices(vector<unsigned short>& indices) const;
private:
    ivec2 m_size;
};

#endif
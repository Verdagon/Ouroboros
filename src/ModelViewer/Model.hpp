#ifndef MODEL_H
#define MODEL_H

#include "Interfaces.hpp"

class Model : public IMesh {
public:
    Model(string name, string path);
    ~Model();
    int GetVertexCount() const;
    int GetTriangleIndexCount() const;
    void GenerateVertices(vector<float>& vertices) const;
    void GenerateTriangleIndices(vector<unsigned short>& indices) const;
private:
    vector<float> loadedVerts;
    vector<unsigned short> loadedIndices;
    int vertexCount;
    int indexCount;
};

#endif

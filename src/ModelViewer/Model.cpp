#include <iostream>
#include <fstream>
#include "Model.hpp"

using namespace std;

Model::Model(string name, string path)
{
    int i, j;
    float vert[8];
    short index;
    
    cout << "Loading mesh: " << name << "\n";
    
    char* fullPath = (char*) malloc(name.size() + path.size() + 2);
    strcpy(fullPath, path.c_str());
    strcat(fullPath, "/");
    strcat(fullPath, name.c_str());
    
    ifstream file(fullPath, ios::in|ios::binary);
    
    file.read((char*)&vertexCount, sizeof(int));
    cout << "with " << vertexCount << " vertices\n";
    //loadedVerts.resize(vertexCount * 8);
    loadedVerts.resize(vertexCount * 5);
    
    for (i = 0; i < vertexCount; ++i) {
        file.read((char*)&vert, sizeof(float) * 5);
        for (j = 0; j < 5; ++j) {
            loadedVerts[i * 5 + j] = vert[j];
        }
        /*
        file.read((char*)&vert, sizeof(float) * 8);
        for (j = 0; j < 8; ++j) {
            loadedVerts[i * 8 + j] = vert[j];
        }*/
    }
    
    file.read((char*)&indexCount, sizeof(int));
    
    loadedIndices.resize(indexCount);
    for (i = 0; i < indexCount; ++i) {
        file.read((char*)&index, sizeof(short));
        loadedIndices[i] = index;
    }
    
    file.close();
    free(fullPath);
}

Model::~Model() {
    
}

int Model::GetVertexCount() const
{
    return vertexCount;
}

int Model::GetTriangleIndexCount() const
{
    return indexCount;
}

void Model::GenerateVertices(vector<float> &vertices) const
{
    int i;
    
    //vertices.resize(vertexCount * 8);
    vertices.resize(vertexCount * 5);
    for (i = 0; i < loadedVerts.size() ; ++i) {
        vertices[i] = loadedVerts[i];
    }
}

void Model::GenerateTriangleIndices(vector<unsigned short> &indices) const
{
    int i;
    
    indices.resize(indexCount);
    for (i = 0; i < loadedIndices.size(); ++i) {
        indices[i] = loadedIndices[i];
    }
}
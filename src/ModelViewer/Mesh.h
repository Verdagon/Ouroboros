#ifndef Ouroboros_Mesh_h
#define Ouroboros_Mesh_h

#include "Interfaces.h"

class Mesh: public IMesh {
public:
    Mesh(string meshName, string textureName);
    ~Mesh();
    string getMeshName();
    string getTextureName();
    virtual MeshData* getMeshData();
    virtual ImageData* getImageData();
private:
    string m_meshName;
    string m_textureName;
};

#endif

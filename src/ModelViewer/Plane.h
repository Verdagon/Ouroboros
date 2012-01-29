#ifndef Ouroboros_Plane_h
#define Ouroboros_Plane_h

#include "Mesh.h"

class Plane: public Mesh {
public:
    Plane(string textureName, vec3 dimensions, vec3 loc);
    Plane(Plane *previousPlane);
    ~Plane();
    MeshData* getMeshData();
protected:
    vec3 m_dimensions;
    vec3 m_location;
};

#endif

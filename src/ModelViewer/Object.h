#ifndef Ouroboros_Object3d_h
#define Ouroboros_Object3d_h

#include "Interfaces.h"

class Object : public IObject {
public:
    Object();
    Object(string fileName, string textureName);
    ~Object();
    void setLoc(vec3 loc);
    list<IMesh *>* getMeshes();
protected:
    list<IMesh *> m_meshList;
    vec3 m_loc;
    IMesh *m_mesh;
};

#endif

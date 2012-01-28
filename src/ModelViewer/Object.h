#ifndef Ouroboros_Object3d_h
#define Ouroboros_Object3d_h

#include "Interfaces.h"

class Object : public IObject {
public:
    Object();
    Object(string fileName, string textureName);
    ~Object();
    list<IMesh *>* getMeshes();
protected:
    list<IMesh *> m_meshList;
};

#endif

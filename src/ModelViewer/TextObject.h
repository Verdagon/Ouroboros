#ifndef Ouroboros_TextObject_h
#define Ouroboros_TextObject_h

#include "Interfaces.h"
#include "Plane.h"

class TextObject : public IObject {
public:
    TextObject();
    TextObject(ivec2 dimensions, ivec2 loc);
    ~TextObject();
    void setText(string text);
    list<IMesh *>* getMeshes();
protected:
    list<IMesh *> m_meshList;
    Plane *m_plane;
    ivec2 m_dimensions;
    ivec2 m_loc;
};


#endif

#ifndef Ouroboros_TextObject_h
#define Ouroboros_TextObject_h

#include "Interfaces.h"
#include "Plane.h"

#define SIZE 40
#define SCALE 0.125
#define WORD_TOTAL_SIZE 32

class TextObject : public IObject {
public:
    TextObject();
    TextObject(ivec2 dimensions, ivec2 loc);
    ~TextObject();
    void setText(string text);
    list<IMesh *>* getMeshes();
protected:
    list<IMesh *> m_meshList;
    IMesh *m_plane;
    ivec2 m_dimensions;
    ivec2 m_loc;
};


#endif

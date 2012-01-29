
#ifndef Ouroboros_Camera_h
#define Ouroboros_Camera_h

#include "Interfaces.h"

class Camera: public ICamera {
public:
    Camera(ivec2 start);
    ~Camera();
    void setLoc(ivec2 loc);
    void setHight(float hight);
private:
    float m_height;
    vec3 m_dir;
};

#endif

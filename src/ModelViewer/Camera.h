
#ifndef Ouroboros_Camera_h
#define Ouroboros_Camera_h

#include "Interfaces.h"

class Camera: public ICamera {
public:
    Camera(vec3 start);
    ~Camera();
    void rotLocal(float pitch, float yaw);
    void moveInOut(float dist);
private:
    float m_pitch, m_yaw, m_dist;
};

#endif

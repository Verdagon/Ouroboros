#include "Camera.h"
#include <iostream>

#define DISTANCE 20

Camera::Camera(ivec2 start) {
    m_height = 30;
    up = vec3(0, 0, 1);
    m_dir = vec3(1, 1, 0);
    m_dir.Normalize();
    
    ref = vec3(start.x, start.y, 0);
    eye = ref - m_dir * DISTANCE;
    eye.z = m_height;
}

Camera::~Camera() {
    
}

void Camera::setLoc(ivec2 loc) {
    ref = vec3(loc.x, loc.y, 0);
    //eye = vec3(0, 0, 20);
    m_dir = ref - eye;
    m_dir.z = 0;
    m_dir.Normalize();
    eye = ref - m_dir * DISTANCE;
    eye.z = m_height;
    up = vec3(0, 0, 1);
    //std::cout << "(" << eye.x << "," << eye.y << "," << eye.z << ")\n";
}

void Camera::setHight(float hight) {
    m_height = hight;
}
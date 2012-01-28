#ifndef STATICVIEW_H
#define STATICVIEW_H

#include "Interfaces.hpp"
#include "Plane.hpp"
#include <iostream>

class StaticView: public IView {
public:
    StaticView(string textureName, ivec2 screen);
    ~StaticView();
    void GetVisuals(vector<Visual>* visuals);
    void GetDrawList(DrawList* list);
    void AddSubview(IView* subview);
    void ClearSubviews();
    mat4 GetProjection();
    mat4 GetTranslation();
protected:
    string* m_textureName;
    ivec2 m_screen;
    int m_meshIndex;
    int m_textureIndex;
    int m_group;
    bool m_listUpdated;
};

#endif
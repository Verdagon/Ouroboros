#ifndef MOVINGVIEW_H
#define MOVINGVIEW_H

#include "Interfaces.hpp"
#include "TextView.hpp"
#include "Plane.hpp"
#include <iostream>

class MovingView: public IView {
public:
    MovingView(ivec2 screen);
    ~MovingView();
    void GetVisuals(vector<Visual>* visuals);
    void GetDrawList(DrawList* list);
    void AddSubview(IView* subview);
    void ClearSubviews();
    mat4 GetProjection();
    mat4 GetTranslation();
private:
    ivec2 m_screen;
    TextView* m_text;
    int m_meshIndex;
    int m_textureIndex;
    int m_group;
    bool m_listUpdated;
};



#endif

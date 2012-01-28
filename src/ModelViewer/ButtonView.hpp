#ifndef BUTTONVIEW_H
#define BUTTONVIEW_H

#include "Interfaces.hpp"
#include "TextView.hpp"
#include "Plane.hpp"
#include <iostream>

class ButtonView: public IView {
public:
    ButtonView(string text, ivec2 location, ivec2 size);
    ~ButtonView();
    void GetVisuals(vector<Visual>* visuals);
    void GetDrawList(DrawList* list);
    void AddSubview(IView* subview);
    void ClearSubviews();
    mat4 GetProjection();
    mat4 GetTranslation();
private:
    ivec2 m_location;
    ivec2 m_size;
    IView* m_textView;
    int m_group;
    int m_planeIndex;
    int m_buttonIndex;
};

#endif
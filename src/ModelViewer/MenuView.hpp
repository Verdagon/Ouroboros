#ifndef MENUVIEW_H
#define MENUVIEW_H

#include "Interfaces.hpp"
#include <iostream>

class MenuView: public IView {
public:
    MenuView(ivec2 screen);
    ~MenuView();
    void GetVisuals(vector<Visual>* visuals);
    void GetDrawList(DrawList* list);
    void AddSubview(IView* subview);
    void ClearSubviews();
    mat4 GetProjection();
    mat4 GetTranslation();
private:
    ivec2 m_screen;
    vector<IView*> m_subViews;
    bool m_listUpdated;
};

#endif
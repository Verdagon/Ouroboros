#ifndef TEXTVIEW_H
#define TEXTVIEW_H

#include "Interfaces.hpp"
#include "Plane.hpp"
#include <iostream>

class TextView: public IView {
public:
    TextView(string text, ivec2 location, int size);
    ~TextView();
    void GetVisuals(vector<Visual>* visuals);
    void GetDrawList(DrawList* list);
    void AddSubview(IView* subview);
    void ClearSubviews();
    mat4 GetProjection();
    mat4 GetTranslation();
private:
    void AssembleChars();
    ivec2 m_location;
    int m_size;
    string* m_text;
    bool m_assembled;
    vector<Visual> m_letters;
    int m_group;
    int m_planeIndex;
    int m_charMapIndex;
};

#endif
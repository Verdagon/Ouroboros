#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include "Interfaces.hpp"
#include "TextView.hpp"
#include "Model.hpp"
#include <iostream>

class GameView: public IView {
public:
    GameView(IMap* map, ivec2 screen);
    ~GameView();
    void GetVisuals(vector<Visual>* visuals);
    void GetDrawList(DrawList* list);
    void AddSubview(IView* subview);
    void ClearSubviews();
    mat4 GetProjection();
    mat4 GetTranslation();
    
    //used by game state
    //void SetBallLoc(vec3 loc);
    //void SetViewLoc(vec3 loc);
private:
    ivec2 m_screen;
    mat4 m_viewTranslation;
    vec3 m_ballLoc;
    vec3 m_viewLoc;
    IMap* m_map;
    int m_group;
    bool m_listUpdated;
};



#endif
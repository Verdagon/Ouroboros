#ifndef MAINVIEW_H
#define MAINVIEW_H

#include "Interfaces.hpp"
#include "Plane.hpp"
#include <iostream>

class MainView: public IMainView {
public:
    MainView(DeviceType device, int width, int height);
    ~MainView();
    void GetVisuals(vector<Visual>* visuals);
    void GetLists(vector<DrawList*>* list);
    void SetStateViews(IState* state);
    mat4 GetProjection(int group);
    mat4 GetTranslation(int group);
private:
    ivec2 m_viewPort;
    DrawList* m_loadingList;
    DrawList* m_backGroundList;
    DrawList* m_foreGroundList;
    Visual m_loadingVisual;
    vector<Visual> m_visible;
    IView* m_backGroundView;
    IView* m_foreGroundView;
    bool m_loading;
};

#endif
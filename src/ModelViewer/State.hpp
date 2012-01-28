#ifndef STATE_H
#define STATE_H

#include "Interfaces.hpp"
#include "MenuView.hpp"

class State: public IState {
public:
    State(IState* oldState, ivec2 screen);
    ~State();
    StateType GetType();
    IView* GetBackGroundView();
    IView* GetForeGroundView();
    virtual void UpdateAnimations(float td);
protected:
    StateType m_type;
    ivec2 m_screen;
    IView* m_backGroundView;
    IView* m_foreGroundView;
};


#endif

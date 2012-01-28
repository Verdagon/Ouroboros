#include "State.hpp"

State::State(IState* oldState, ivec2 screen) {
    m_screen = screen;
    
    if (oldState == NULL) {
        m_foreGroundView = new MenuView(screen);
    } else {
        m_foreGroundView = oldState->GetForeGroundView();
    }
    m_backGroundView = NULL;
}

State::~State() {
    delete m_backGroundView;
}

StateType State::GetType() {
    return m_type;
}

IView* State::GetBackGroundView() {
    return m_backGroundView;
}

IView* State::GetForeGroundView() {
    return m_foreGroundView;
}

void State::UpdateAnimations(float td) {
    //Nothing for now
}
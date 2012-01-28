#include "StaticState.hpp"

StaticState::StaticState(IState* oldState, ivec2 screen): State(oldState, screen) {
    m_backGroundView = new StaticView("Background_Iphone.png", screen);
    m_type = StateTypeStatic;
}
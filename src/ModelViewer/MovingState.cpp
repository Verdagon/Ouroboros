#include "MovingState.hpp"

MovingState::MovingState(IState* oldState, ivec2 screen):State(oldState, screen) {
    m_backGroundView = new MovingView(screen);
    m_type = StateTypeMoving;
}
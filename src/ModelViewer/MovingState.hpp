#ifndef MOVINGSTATE_H
#define MOVINGSTATE_H

#include "State.hpp"
#include "MovingView.hpp"

class MovingState: public State {
public:
    MovingState(IState* oldState, ivec2 screen);
    ~MovingState();
private:
    
};


#endif
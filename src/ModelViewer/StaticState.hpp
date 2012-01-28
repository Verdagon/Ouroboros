#ifndef STATICSTATE_H
#define STATICSTATE_H

#include "State.hpp"
#include "StaticView.hpp"

class StaticState: public State {
public:
    StaticState(IState* oldState, ivec2 screen);
    ~StaticState();
private:
    
};


#endif
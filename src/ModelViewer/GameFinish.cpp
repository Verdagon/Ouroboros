#include "GameFinish.hpp"

GameFinish::GameFinish(IApplicationEngine* appEngine):Controller(appEngine) {
    
    IState* curState = m_appEngine->GetState();
    if (curState == NULL || curState->GetType() != StateTypeGame) {
        //m_gameState = new GameState(curState, m_screen);
        //m_appEngine->SetState(m_gameState);
    } else {
        m_gameState = (GameState*) curState;
    }
    
    //Clear the foreground view and populate it with buttons
    ClearButtons();
    AddButton(new Button<GameFinish>("Replay", ivec2(-75,-200), ivec2(100,20), this, &GameFinish::RestartGame));
    AddButton(new Button<GameFinish>("Next", ivec2(75,-200), ivec2(100,20), this, &GameFinish::LoadHolePicker));
}

GameFinish::~GameFinish() {
    
}

void GameFinish::RestartGame() {
    MoveToController(new GameMain(m_appEngine));
}

void GameFinish::LoadHolePicker() {
    MoveToController(new HolePicker(m_appEngine));
}
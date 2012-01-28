#include "GamePause.hpp"

GamePause::GamePause(IApplicationEngine* appEngine):Controller(appEngine) {
    
    IState* curState = m_appEngine->GetState();
    if (curState == NULL || curState->GetType() != StateTypeGame) {
        //m_gameState = new GameState(curState, m_screen);
        //m_appEngine->SetState(m_gameState);
    } else {
        m_gameState = (GameState*) curState;
    }
    
    //Clear the foreground view and populate it with buttons
    ClearButtons();
    AddButton(new Button<GamePause>("Continue", ivec2(0,100), ivec2(140,20), this, &GamePause::ContinueGame));
    AddButton(new Button<GamePause>("Replay", ivec2(75,-100), ivec2(100,20), this, &GamePause::RestartGame));
    AddButton(new Button<GamePause>("Menu", ivec2(-75,-100), ivec2(100,20), this, &GamePause::LoadHolePicker));
}

GamePause::~GamePause() {
    
}

void GamePause::ContinueGame() {
    // Go back to the current game
    MoveToController(new GameMain(m_appEngine));
}

void GamePause::RestartGame() {
    // Restart the game
    m_gameState->Restart();
    MoveToController(new GameMain(m_appEngine));
}

void GamePause::LoadHolePicker() {
    // Go back to the hole picker
    MoveToController(new HolePicker(m_appEngine));
}
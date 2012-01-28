#include "ScoreBoard.hpp"

ScoreBoard::ScoreBoard(IApplicationEngine* appEngine):Controller(appEngine) {
    
    IState* curState = m_appEngine->GetState();
    if (curState == NULL || curState->GetType() != StateTypeStatic) {
        m_curState = new StaticState(curState, m_screen);
        m_appEngine->SetState(m_curState);
    } else {
        m_curState = (StaticState*) curState;
    }
    
    //Clear the foreground view and populate it with buttons
    ClearButtons();
    AddButton(new Button<ScoreBoard>("Back", ivec2(-75,-200), ivec2(100,20), this, &ScoreBoard::LoadMenu));
    AddButton(new Button<ScoreBoard>("Clear", ivec2(75,-200), ivec2(100,20), this, &ScoreBoard::LoadMenuClear));
}

ScoreBoard::~ScoreBoard() {
    
}

void ScoreBoard::LoadMenu() {
    MoveToController(new MainMenu(m_appEngine));
}

void ScoreBoard::LoadMenuClear() {
    MoveToController(new MainMenu(m_appEngine));
}

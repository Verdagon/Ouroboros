#include "MainMenu.hpp"

MainMenu::MainMenu(IApplicationEngine* appEngine):Controller(appEngine) {
    
    IState* curState = m_appEngine->GetState();
    if (curState == NULL || curState->GetType() != StateTypeStatic) {
        m_curState = new StaticState(curState, m_screen);
        m_appEngine->SetState(m_curState);
    } else {
        m_curState = (StaticState*) curState;
    }
    
    //Clear the foreground view and populate it with buttons
    ClearButtons();
    
    // Set to load different values for iphone and ipad
    AddButton(new Button<MainMenu>("Score", ivec2(-75,-100), ivec2(100,20), this, &MainMenu::LoadScores));
    AddButton(new Button<MainMenu>("Start", ivec2(75,-100), ivec2(100,20), this, &MainMenu::LoadHolePicker));
}

MainMenu::~MainMenu() {
    
}

void MainMenu::LoadHolePicker() {
    MoveToController(new HolePicker(m_appEngine));
}

void MainMenu::LoadScores() {
    MoveToController(new ScoreBoard(m_appEngine));
}
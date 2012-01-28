#include "HolePicker.hpp"

HolePicker::HolePicker(IApplicationEngine* appEngine):Controller(appEngine) {
    
    IState* curState = m_appEngine->GetState();
    if (curState == NULL || curState->GetType() != StateTypeMoving) {
        m_movingState = new MovingState(curState, m_screen);
        m_appEngine->SetState(m_movingState);
    } else {
        m_movingState = (MovingState*) curState;
    }
    
    //Clear the foreground view and populate it with buttons
    ClearButtons();
    AddButton(new Button<HolePicker>("Menu", ivec2(-75,-200), ivec2(100,20), this, &HolePicker::LoadMenu));
    AddButton(new Button<HolePicker>("Play", ivec2(75,-200), ivec2(100,20), this, &HolePicker::LoadGame));
    
    //Use dedicated image for these buttons
    AddButton(new Button<HolePicker>("<", ivec2(-110,200), ivec2(20,20), this, &HolePicker::LastHole));
    AddButton(new Button<HolePicker>(">", ivec2(110,200), ivec2(20,20), this, &HolePicker::NextHole));
}

HolePicker::~HolePicker() {
    
}

void HolePicker::LoadGame() {
    // Start a game
    IMap hole;
    hole.ResourcePath = m_appEngine->GetResourcePath();
    hole.Map = new string("Hole03.map");
    GameState* newGame = new GameState(m_movingState, hole, m_screen);
    m_appEngine->SetState(newGame);
    
    MoveToController(new GameMain(m_appEngine));
}

void HolePicker::LoadMenu() {
    // Go back to the main menu
    MoveToController(new MainMenu(m_appEngine));
}

void HolePicker::NextHole() {
    std::cout << "Switch to next hole\n";
}

void HolePicker::LastHole() {
    std::cout << "Switch to last hole\n";
}
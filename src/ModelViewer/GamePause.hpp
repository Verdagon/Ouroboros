#ifndef GAMEPAUSE_H
#define GAMEPAUSE_H

#include "Controller.hpp"
#include "GameMain.hpp"
#include "HolePicker.hpp"
#include <iostream>
#include <fstream>

class GamePause: public Controller {
public:
    GamePause(IApplicationEngine* appEngine);
    ~GamePause();
    void ContinueGame();
    void RestartGame();
    void LoadHolePicker();
private:
    GameState* m_gameState;
};

#endif
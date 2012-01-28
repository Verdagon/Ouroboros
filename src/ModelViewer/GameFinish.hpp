#ifndef GAMEFINISH_H
#define GAMEFINISH_H

#include "Controller.hpp"
#include "GameMain.hpp"
#include "HolePicker.hpp"
#include <iostream>
#include <fstream>

class GameFinish: public Controller {
public:
    GameFinish(IApplicationEngine* appEngine);
    ~GameFinish();
    void RestartGame();
    void LoadHolePicker();
private:
    GameState* m_gameState;
};

#endif
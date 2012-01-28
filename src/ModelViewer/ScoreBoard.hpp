#ifndef SCOREBOARD_H
#define SCOREBOARD_H

#include "Controller.hpp"
#include "MainMenu.hpp"
#include <iostream>
#include <fstream>

class ScoreBoard: public Controller {
public:
    ScoreBoard(IApplicationEngine* appEngine);
    ~ScoreBoard();
    void LoadMenu();
    void LoadMenuClear();
private:
    StaticState* m_curState;
};

#endif
#ifndef MAINMENU_H
#define MAINMENU_H

#include "Controller.hpp"
#include "ScoreBoard.hpp"
#include "HolePicker.hpp"
#include <iostream>
#include <fstream>

class MainMenu: public Controller {
public:
    MainMenu(IApplicationEngine* appEngine);
    ~MainMenu();
    void LoadHolePicker();
    void LoadScores();
private:
    StaticState* m_curState;
};

#endif
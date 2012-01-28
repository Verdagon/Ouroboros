#ifndef HOLEPICKER_H
#define HOLEPICKER_H

#include "Controller.hpp"
#include "MainMenu.hpp"
#include "GameMain.hpp"
#include <iostream>
#include <fstream>

class HolePicker: public Controller {
public:
    HolePicker(IApplicationEngine* appEngine);
    ~HolePicker();
    void LoadMenu();
    void LoadGame();
    void NextHole();
    void LastHole();
private:
    MovingState* m_movingState;
};

#endif
#ifndef GAMEMAIN_H
#define GAMEMAIN_H

#include "Controller.hpp"
#include "GameFinish.hpp"
#include "GamePause.hpp"
#include <iostream>
#include <fstream>

class GameMain: public Controller {
public:
    GameMain(IApplicationEngine* appEngine);
    ~GameMain();
    void PauseGame();
    void GameDone();
    void OnFingerUp(vec2 location);
    void OnFingerDown(vec2 location);
    void OnFingerMove(vector<vec2> touches);
    void Tic(float td);
    bool TouchBall(vec2 location);
private:
    GameState* m_gameState;
    bool m_putting;
    float m_hype;
    vec2 m_touchStart;
};

#endif

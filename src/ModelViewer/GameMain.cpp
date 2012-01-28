#include "GameMain.hpp"

#define BALLTOUCHRADIUS 40

GameMain::GameMain(IApplicationEngine* appEngine):Controller(appEngine) {
    
    IState* curState = m_appEngine->GetState();
    if (curState == NULL || curState->GetType() != StateTypeGame) {
        //m_gameState = new GameState(curState, m_screen);
        //m_appEngine->SetState(m_gameState);
    } else {
        m_gameState = (GameState*) curState;
    }
    
    //Clear the foreground view and populate it with buttons
    ClearButtons();
    
    //Another custom button for this as well
    AddButton(new Button<GameMain>(".", ivec2(110,200), ivec2(20, 20), this, &GameMain::PauseGame));
    //In place of finishing a game
    //AddButton(new Button<GameMain>("Finish", ivec2(0,-100), ivec2(100,20), this, &GameMain::GameDone));
    
    m_putting = false;
    m_hype = 0;
}

GameMain::~GameMain() {
    
}

void GameMain::PauseGame() {
    MoveToController(new GamePause(m_appEngine));
}

void GameMain::GameDone() {
    MoveToController(new GameFinish(m_appEngine));
}

void GameMain::OnFingerUp(vec2 location) {
    if (m_putting) {
        vec3 end = m_gameState->GetPickLoc(location, 0);
        vec3 start = m_gameState->GetPickLoc(m_touchStart, 0);
        //vec3 delta = end - start;
        vec3 delta = start - end;
        //Hit the ball
        m_gameState->HitBall(vec2(delta.x, delta.y));
        /*
        vec2 delta = location - m_touchStart;
        vec3 vector = m_gameState->GetPickLoc(delta, 0);
        m_gameState->HitBall(vec2(vector.x, vector.y));
         */
    }
    m_putting = false;
    m_hype = 0;
    
    Controller::OnFingerUp(location);
}

void GameMain::OnFingerDown(vec2 location) {
    
    m_putting = TouchBall(location);
    m_touchStart = location;
    
    Controller::OnFingerDown(location);
}

void GameMain::OnFingerMove(vector<vec2> touches) {
    if (!m_putting) {
        if (touches.size() < 3) {
            
            //Move with one finger
            vec3 tmp1 = m_gameState->GetPickLoc(touches[0], 0);
            vec3 tmp2 = m_gameState->GetPickLoc(touches[1], 0);
            vec3 delta = tmp1 - tmp2;
            vec3 viewLoc = m_gameState->GetViewLoc();
            viewLoc = delta + viewLoc;
            m_gameState->SetViewLoc(viewLoc);
            m_hype = 0;
        } else {
            
            //pinch zoom
            float length = touches[1].x - touches[3].x;
            float height = touches[1].y - touches[3].y;
            float nextHype = sqrt(length * length + height * height);
            if (m_hype != 0) {
                float scale = m_hype / nextHype;
                vec3 viewLoc = m_gameState->GetViewLoc();
                viewLoc.z = viewLoc.z * scale;
                m_gameState->SetViewLoc(viewLoc);
            }
            m_hype = nextHype;
        }
    } else {
        m_hype = 0;
    }
    Controller::OnFingerMove(touches);
}

void GameMain::Tic(float td) {
    m_gameState->UpdateAnimations(td);
}

bool GameMain::TouchBall(vec2 location) {
    location.y -= 10;
    vec2 ballpoint = m_gameState->GetBallScreenLoc();
    vec2 delta = ballpoint - location;
    if ((delta.x * delta.x + delta.y * delta.y) < (BALLTOUCHRADIUS * BALLTOUCHRADIUS)) {
        return true;
    }
    return false;
}
#include "Controller.hpp"

Controller::Controller(IApplicationEngine* appEngine) {
    
    m_appEngine = appEngine;
    m_screen = *m_appEngine->GetScreenSize();
    m_buttons.resize(0);
}

Controller::~Controller() {
    ClearButtons();
}

void Controller::OnFingerUp(vec2 location) {
    location = location - (m_screen / 2);
    vector<IButton*>::iterator button = m_buttons.begin();
    while (button != m_buttons.end()) {
        if ((*button)->CheckTouch(location)) {
            (*button)->Action();
            return;
        }
        button++;
    }
}

void Controller::OnFingerDown(vec2 location) {
    
}

void Controller::OnFingerMove(vector<vec2> touches) {
    
}

void Controller::AddButton(IButton *button) {
    m_appEngine->GetState()->GetForeGroundView()->AddSubview(button->GetView());
    m_buttons.push_back(button);
}

void Controller::Tic(float td) {
    //Update animations
}

void Controller::ClearButtons() {
    m_appEngine->GetState()->GetForeGroundView()->ClearSubviews();
    
    vector<IButton*>::iterator button = m_buttons.begin();
    while (button != m_buttons.end()) {
        delete (*button);
    }
    m_buttons.resize(0);
}

void Controller::MoveToController(IController *newController) {
    m_appEngine->SetController(newController);
}
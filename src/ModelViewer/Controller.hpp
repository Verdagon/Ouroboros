#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "Interfaces.hpp"

#include "TextView.hpp"
#include "ButtonView.hpp"

#include "StaticState.hpp"
#include "MovingState.hpp"
#include "GameState.hpp"

#include <iostream>
#include <fstream>

class IButton {
public:
    virtual IView* GetView() = 0;
    virtual bool CheckTouch(vec2 touch) = 0;
    virtual void Action() = 0;
};

template <class C> class Button: public IButton {
public:
    Button(string text, ivec2 location, ivec2 size, C* controller, void(C::*act)()) {
        m_controller = controller;
        m_location = location;
        m_size = size;
        m_act = act;
        m_view = new ButtonView(text, location, size);
    };
    ~Button() {
        delete m_view;
    }
    IView* GetView() {
        return m_view;
    }
    bool CheckTouch(vec2 touch) {
        bool inx = touch.x >= m_location.x - m_size.x && touch.x <= m_location.x + m_size.x;
        //Invert the y value of the touchpoint
        bool iny = -touch.y >= m_location.y - m_size.y && -touch.y <= m_location.y + m_size.y;
        return inx && iny;
    }
    void Action() {
        // Call the action
        (*m_controller.*m_act)();
    };
private:
    void (C::*m_act)();
    C* m_controller;
    ivec2 m_location;
    ivec2 m_size;
    IView* m_view;
};

class Controller: public IController {
public:
    Controller(IApplicationEngine* appEngine);
    ~Controller();
    virtual void OnFingerUp(vec2 location);
    virtual void OnFingerDown(vec2 location);
    virtual void OnFingerMove(vector<vec2> touches);
    virtual void Tic(float td);
    IView* GetBackGroundView();
    IView* GetForeGroundView();
    void AddButton(IButton* button);
    void ClearButtons();
    void MoveToController(IController* newController);
protected:
    IApplicationEngine* m_appEngine;
    vector<IButton*> m_buttons;
    ivec2 m_screen;
};

#endif
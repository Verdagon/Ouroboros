#ifndef APPLICATIONENGINE_H
#define APPLICATIONENGINE_H

#include "Interfaces.hpp"
#include "Matrix.hpp"
//#include "MainView.hpp"
//#include "MainMenu.hpp"
#include <iostream>
#include <fstream>

using namespace std;

class ApplicationEngine : public IApplicationEngine {
public:
    ApplicationEngine(DeviceType deviceType, IRenderingEngine* renderingEngine, IResourceManager* resourceManager);
    ~ApplicationEngine();
    void Initialize(int width, int height);
    
    //Used by controllers
    string* GetResourcePath();
    //void SetController(IController *controller);
    //void SetState(IState* state);
    //IState* GetState();
    
    void OnFingerUp(vec2 location);
    void OnFingerDown(vec2 location);
    void OnFingerMove(vector<vec2> touches);
    void Render() const;
    void UpdateAnimations(float td);
    void AppWillResignActive();
    void AppWillBecomeActive();
    void AppWillEnterBackground();
    void AppWillEnterForeground();
    void AppWillTerminate();
    ivec2* GetScreenSize();
private:
    ivec2 m_mainScreenSize;
    DeviceType m_deviceType;
    //MainView* m_mainView;
    //IController* m_curController;
    //IController* m_lastController;
    //IState* m_curState;
    IRenderingEngine* m_renderingEngine;
    IResourceManager* m_resourceManager;
};

#endif

IApplicationEngine* CreateApplicationEngine(DeviceType deviceType, IRenderingEngine* renderingEngine, IResourceManager* resourceManager) {
    return new ApplicationEngine(deviceType, renderingEngine, resourceManager);
}

ApplicationEngine::ApplicationEngine(DeviceType deviceType, IRenderingEngine* renderingEngine, IResourceManager* resourceManager) {
    m_deviceType = deviceType;
    if (deviceType == DeviceTypeIphone) {
        cout << "App loaded on iphone\n";
    } else if(deviceType == DeviceTypeIpod) {
        cout << "App loaded on ipod\n";
    } else if (deviceType == DeviceTypeIpad) {
        cout << "App loaded on ipad\n";
    } else {
        cout << "App loaded on unknown device\n";
    }
    m_renderingEngine = renderingEngine;
    m_resourceManager = resourceManager;
    //m_curController = NULL;
    //m_lastController = NULL;
    //m_curState = NULL;
}

ApplicationEngine::~ApplicationEngine() {
    delete m_renderingEngine;
    //delete m_mainView;
}

void ApplicationEngine::Initialize(int width, int height) {
    m_mainScreenSize = ivec2(width, height);
    
    //Initalize the main view
    //m_mainView = new MainView(m_deviceType, width, height);
    
    //Initalize the rendering engine
    //m_renderingEngine->Initialize(m_mainView, width, height);
    m_renderingEngine->Initialize(width, height);
    
    //Initalize Screen and Game Controllers here
    //SetController(new MainMenu(this));
}

string* ApplicationEngine::GetResourcePath() {
    return m_resourceManager->GetResourcePath();
}

/*
void  ApplicationEngine::SetController(IController *controller) {
    delete m_lastController;
    m_lastController = m_curController;
    m_curController = controller;
    m_renderingEngine->ReLoadLists();
}

void ApplicationEngine::SetState(IState *state) {
    m_mainView->SetStateViews(state);
    delete m_curState;
    m_curState = state;
}

IState* ApplicationEngine::GetState() {
    return m_curState;
}*/

void ApplicationEngine::Render() const {
    list<IObject3d *> objectList(0);
    m_renderingEngine->render(objectList);
}

void ApplicationEngine::UpdateAnimations(float td) {
    //m_curController->Tic(td);
}

void ApplicationEngine::OnFingerUp(vec2 location) {
        //m_curController->OnFingerUp(location);
}

void ApplicationEngine::OnFingerDown(vec2 location) {
        //m_curController->OnFingerDown(location);
}

void ApplicationEngine::OnFingerMove(vector<vec2> touches) {
        //m_curController->OnFingerMove(touches);
}

void ApplicationEngine::AppWillResignActive() {
    cout << "App Will Resign Active\n";
}

void ApplicationEngine::AppWillBecomeActive() {
    cout << "App Will Become Active\n";
}

void ApplicationEngine::AppWillEnterBackground() {
    cout << "App Will Enter Background\n";
}

void ApplicationEngine::AppWillEnterForeground() {
    cout << "App Will Enter Foreground\n";
}

void ApplicationEngine::AppWillTerminate() {
    cout << "App Will Terminate\n";
}

ivec2* ApplicationEngine::GetScreenSize() {
    return &m_mainScreenSize;
}
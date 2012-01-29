#ifndef APPLICATIONENGINE_H
#define APPLICATIONENGINE_H

#include "Interfaces.h"
#include "Matrix.hpp"
#include "Camera.h"
#include "Object.h"
#include "TextObject.h"
#include "Game.h"
#include <iostream>
#include <fstream>

using namespace std;

class ApplicationEngine : public IApplicationEngine {
public:
    ApplicationEngine(DeviceType deviceType, IRenderingEngine* renderingEngine, IResourceManager* resourceManager);
    ~ApplicationEngine();
    void Initialize(int width, int height);
    string* GetResourcePath();
    void OnFingerUp(vec2 location);
    void OnFingerDown(vec2 location);
    void OnFingerMove(vector<vec2> touches);
    void Render();
    void UpdateAnimations(float td);
    void AppWillResignActive();
    void AppWillBecomeActive();
    void AppWillEnterBackground();
    void AppWillEnterForeground();
    void AppWillTerminate();
    ivec2* GetScreenSize();
private:
    ivec2 m_mainScreenSize;
    vec2 m_lastLoc;
    DeviceType m_deviceType;
    Camera *m_camera;
    TextObject *m_testText;
    list<IObject *> m_objects3d;
    list<IObject *> m_objects2d;
    IRenderingEngine* m_renderingEngine;
    IResourceManager* m_resourceManager;
    Map *m_map;
    std::list<Creature *> m_creatures;
    Creature *m_player;
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
    m_objects3d = list<IObject *>(0);
    m_objects2d = list<IObject *>(0);
    
    GenerateOptions options = {
        10, 10,
        1337,
        2,
        2,
        6,
        6,
        1,
        10,
        Tile(false, '#'),
        Tile(true, '.')
    };
    MapTiles *mapTiles = generateMap(options);
    
//    displayMapTiles(*mapTiles);
    
    m_map = new Map(8, mapTiles);
//    
    m_renderingEngine->addObject(m_map);
    m_objects3d.push_back(m_map);
//
    {
        int playerRadius = 3;
        Position playerCenter = m_map->findCenterOfRandomWalkableAreaOfRadius(playerRadius);
        playerCenter.x = 4;
        playerCenter.y = 4;
        m_player = new Creature('@', playerRadius, playerCenter);
        m_player->setLoc(vec3(playerCenter.x, playerCenter.y, 10));
        
        m_map->placeCreature(m_player);
        
        m_renderingEngine->addObject(m_player);
        m_objects3d.push_back(m_player);
    }
    
//    Position destination = map.findCenterOfRandomWalkableAreaOfRadius(playerRadius);
//    
//    std::cout << "player is at " << playerCenter << " going to travel to " << destination << std::endl;
//    
//    std::list<Position> path;
//    if (!map.findPath(player, destination, &path))
//        assert(false);
//    
//    for (std::list<Position>::iterator i = path.begin(), iEnd = path.end(); i != iEnd; i++)
//        std::cout << "step: " << *i << std::endl;
}

ApplicationEngine::~ApplicationEngine() {
    delete m_renderingEngine;
    //delete m_mainView;
}

void ApplicationEngine::Initialize(int width, int height) {
    m_mainScreenSize = ivec2(width, height);
    
    m_camera = new Camera();
    m_camera->fwd = vec3(-25, 0, -100);
    m_camera->eye = vec3(25, 0, 100);
    m_camera->ref = vec3(0, 0, 0);
    m_camera->up = vec3(0, 0, 1);
    
    m_renderingEngine->setCamera(m_camera);
    m_renderingEngine->Initialize(width, height);
}

string* ApplicationEngine::GetResourcePath() {
    return m_resourceManager->GetResourcePath();
}

void ApplicationEngine::Render() {
    m_renderingEngine->render(m_objects3d, m_objects2d);
}

void ApplicationEngine::UpdateAnimations(float td) {
    //m_testText->setText("ABC");
    //m_curController->Tic(td);
}

void ApplicationEngine::OnFingerUp(vec2 location) {
    // todo: autodetect w and h
    int width = 320;
    int height = 480;
    
    int deltaX = 0;
    if (location.x < width / 3)
        deltaX = -1;
    if (location.x > width * 2/3)
        deltaX = 1;
    
    int deltaY = 0;
    if (location.y < height / 3)
        deltaY = -1;
    if (location.y > height * 2/3)
        deltaY = 1;
    
    m_player->center.x += deltaX;
    m_player->center.y += deltaY;
    
    m_player->setLoc(vec3(m_player->center.x, m_player->center.y, 10));
}

void ApplicationEngine::OnFingerDown(vec2 location) {
    m_lastLoc = location;
}

void ApplicationEngine::OnFingerMove(vector<vec2> touches) {
    vec2 move = touches[0] - m_lastLoc;
    move = move * 0.5;
    std::cout << "(" << move.x << "," << move.y << ")\n";
    m_camera->rotLocal(move.y, -move.x);
    m_lastLoc = touches[0];
    
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
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
    void setPlayerAndCameraPos(Position pos);
    
    ivec2 m_mainScreenSize;
    vec2 m_lastLoc;
    ivec2 m_direction;
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
    
    m_renderingEngine->setCamera(m_camera);
    m_renderingEngine->Initialize(width, height);
    
    m_direction = ivec2(0, 0);
    
    
    GenerateOptions options = {
        100, 100,
        1347,
        3,
        4,
        6,
        8,
        3,
        20,
        Tile(false, '#'),
        Tile(true, '.')
    };
    MapTiles *mapTiles = generateMap(options);
    
    //    displayMapTiles(*mapTiles);
    
    m_map = new Map(8, mapTiles);
    
    m_renderingEngine->addObject(m_map->tiles);
    m_objects3d.push_back(m_map->tiles);
    
    {
        int playerRadius = 3;
        Position playerCenter = m_map->findCenterOfRandomWalkableAreaOfRadius(playerRadius);
        m_player = new Creature('@', playerRadius, playerCenter);
        
        m_map->placeCreature(m_player);
        m_renderingEngine->addObject(m_player);
        m_objects3d.push_back(m_player);
        
        setPlayerAndCameraPos(playerCenter);
    }
    
//    Object *myMesh1 = new Object("atsym.obj", "atsym.png");
//    myMesh1->setLoc(vec3(0, 0, 0));
//    m_renderingEngine->addObject(myMesh1);
//    m_objects3d.push_back(myMesh1);
//    
//    Object *myMesh2 = new Object("atsym.obj", "atsym.png");
//    myMesh2->setLoc(vec3(10, 0, 0));
//    m_renderingEngine->addObject(myMesh2);
//    m_objects3d.push_back(myMesh2);
}

void ApplicationEngine::setPlayerAndCameraPos(Position pos) {
    m_map->removeCreature(m_player);
    
    if (m_map->areaIsWalkable(pos, m_player->radius))
        m_player->center = pos;
    else
        pos = m_player->center;
    
    m_map->placeCreature(m_player);
    
    std::cout << "Putting player at " << pos << std::endl;
    m_player->setCenter(pos);
    
    m_map->tiles->setLightPosition(m_map->tileCoordAtPosition(pos));
    
    m_camera->ref = vec3(m_player->center.x - (m_player->radius), -(m_player->center.y - (m_player->radius)), 0);
    m_camera->fwd = vec3(0, 15, -30);
    m_camera->eye = m_camera->ref - m_camera->fwd;
    m_camera->up = vec3(0, 0, 1);
}

string* ApplicationEngine::GetResourcePath() {
    return m_resourceManager->GetResourcePath();
}

void ApplicationEngine::Render() {
    m_renderingEngine->render(m_objects3d, m_objects2d);
}

void ApplicationEngine::UpdateAnimations(float td) {
    setPlayerAndCameraPos(Position(m_player->center.x + m_direction.x, m_player->center.y + m_direction.y));
    //m_testText->setText("ABC");
    //m_curController->Tic(td);
}

void ApplicationEngine::OnFingerUp(vec2 location) {
    m_direction = ivec2(0, 0);
}

void ApplicationEngine::OnFingerDown(vec2 location) {
    // todo: autodetect w and h
    int width = m_mainScreenSize.x;
    int height = m_mainScreenSize.y;
    
    int deltaX = 0;
    if (location.x < width / 3)
        m_direction.x = -1;
    if (location.x > width * 2/3)
        m_direction.x = 1;
    
    int deltaY = 0;
    if (location.y < height / 3)
        m_direction.y = -1;
    if (location.y > height * 2/3)
        m_direction.y = 1;
}

void ApplicationEngine::OnFingerMove(vector<vec2> touches) {
    // todo: autodetect w and h
    int width = m_mainScreenSize.x;
    int height = m_mainScreenSize.y;
    
    int deltaX = 0;
    if (touches[0].x < width / 3)
         m_direction.x = -1;
    if (touches[0].x > width * 2/3)
         m_direction.x = 1;
    
    int deltaY = 0;
    if (touches[0].y < height / 3)
         m_direction.y = -1;
    if (touches[0].y > height * 2/3)
         m_direction.x = -1;
    
    //std::cout << "moving by " << deltaX << "," << deltaY << std::endl;
    
    //setPlayerAndCameraPos(Position(m_player->center.x + deltaX, m_player->center.y + deltaY));
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
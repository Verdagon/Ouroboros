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
    void setPlayerAndCameraPos(Position pos, bool force);
    void unitAttack(Creature *attackingCreature);
    void unitAct(Creature *actingCreature);
    bool unitCanMoveTo(Creature *unit, const Position &pos);
    void addCreature(Creature *creature);
    void killCreature(Creature *victim);
    
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
    float currentTime;
    float m_hype;
};

#endif

IApplicationEngine* CreateApplicationEngine(DeviceType deviceType, IRenderingEngine* renderingEngine, IResourceManager* resourceManager) {
    return new ApplicationEngine(deviceType, renderingEngine, resourceManager);
}

ApplicationEngine::ApplicationEngine(DeviceType deviceType, IRenderingEngine* renderingEngine, IResourceManager* resourceManager) {
    currentTime = 0;
    
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
    m_hype = 0;
    
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
    
    m_camera = new Camera(ivec2(0, 0));
    
    m_renderingEngine->setCamera(m_camera);
    m_renderingEngine->Initialize(width, height);
    
    m_direction = ivec2(0, 0);
    
    
    GenerateOptions options = {
        40, 50,
        1347,
        3,
        4,
        4,
        6,
        1,
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
        addCreature(m_player);
        
        setPlayerAndCameraPos(playerCenter, true);
    }
    
    for (int i = 0; i < 50; i++) {
        int goblinRadius = 2;
        Position goblinCenter = m_map->findCenterOfRandomWalkableAreaOfRadius(goblinRadius);
        if (!m_map->areaIsWalkable(goblinCenter, goblinRadius))
            continue;
        
        Creature *goblin = new Creature(i % 2 ? 'g' : 'r', goblinRadius, goblinCenter);
        goblin->nextActionTime = 3 + i * .1;
        goblin->setCenter(goblinCenter);
        addCreature(goblin);
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

void ApplicationEngine::addCreature(Creature *creature) {
    
    m_creatures.push_back(creature);
    
    m_map->placeCreature(creature);
    m_renderingEngine->addObject(creature);
    m_objects3d.push_back(creature);
}

void ApplicationEngine::unitAttack(Creature *attackingCreature) {
    Position hitBoxOrigin = attackingCreature->center + topLeftOffsetForRadius(attackingCreature->radius + 2);
    Position hitBoxSize = areaForRadius(attackingCreature->radius + 2);
    
    for (int x = hitBoxOrigin.x; x < hitBoxOrigin.x + hitBoxSize.x; x++) {
        for (int y = hitBoxOrigin.y; y < hitBoxOrigin.y + hitBoxSize.y; y++) {
            if (m_map->grid[Position(x, y)].inhabitingCreature == NULL)
                continue;
            
            if (m_map->grid[Position(x, y)].inhabitingCreature == attackingCreature)
                continue;
            
            Creature *victim = m_map->grid[Position(x, y)].inhabitingCreature;
            victim->markedForDeath = true;
        }
    }
    
    attackingCreature->startSpinning(currentTime);
    
//    inline Position areaForRadius(int radius) {
//        int length = radius * 2 - 1;
//        return Position(length, length);
//    }
//    
//    inline Position topLeftOffsetForRadius(int radius) {
//        return Position(-(radius - 1), -(radius - 1));
//    }
}

void ApplicationEngine::killCreature(Creature *victim) {
    victim->setVisible(false);
    m_map->removeCreature(victim);
    std::list<Creature *>::iterator victimIter = std::find(m_creatures.begin(), m_creatures.end(), victim);
    assert(victimIter != m_creatures.end());
    m_creatures.erase(victimIter);
}

void ApplicationEngine::setPlayerAndCameraPos(Position pos, bool force) {
    if (force || m_player->center != pos) {
        if (unitCanMoveTo(m_player, pos)) {
            m_map->removeCreature(m_player);
            m_player->setCenter(pos);
            m_map->placeCreature(m_player);
            
            m_map->tiles->setLightPosition(m_map->tileCoordAtPosition(pos));
            
            for (std::list<Creature *>::iterator i = m_creatures.begin(), iEnd = m_creatures.end(); i != iEnd; i++) {
                Creature *creature = *i;
                if (creature == m_player)
                    continue;
                
                float dist = (float)posDistance((Position&)creature->center, (Position&)m_player->center);
                creature->setVisible(dist < 10 * m_map->tileLengthInMapUnits);
            }
            
            m_camera->setLoc(ivec2(m_player->center.x - (m_player->radius), -(m_player->center.y - (m_player->radius))));
            //m_camera->ref = vec3(m_player->center.x - (m_player->radius), -(m_player->center.y - (m_player->radius)), 0);
            //m_camera->fwd = vec3(0, 15, -30);
            //m_camera->eye = m_camera->ref - m_camera->fwd;
            //m_camera->up = vec3(0, 0, 1);
        }
    }
}

string* ApplicationEngine::GetResourcePath() {
    return m_resourceManager->GetResourcePath();
}

void ApplicationEngine::Render() {
    m_renderingEngine->render(m_objects3d, m_objects2d);
}

bool ApplicationEngine::unitCanMoveTo(Creature *creature, const Position &pos) {
    if (creature->radius > m_map->grid[pos].maximumRadiusOfInhabitingCreature)
        return false;
    
    Position creatureOrigin = pos + topLeftOffsetForRadius(creature->radius);
    Position creatureSize = areaForRadius(creature->radius);
    
    for (int x = creatureOrigin.x; x < creatureOrigin.x + creatureSize.x; x++)
        for (int y = creatureOrigin.y; y < creatureOrigin.y + creatureSize.y; y++)
            if (m_map->grid[Position(x, y)].inhabitingCreature && m_map->grid[Position(x, y)].inhabitingCreature != creature)
                return false;
    
    return true;
}

void ApplicationEngine::unitAct(Creature *actingCreature) {
    if (posDistance(actingCreature->center, m_player->center) < 10) {
        if (!m_map->findPath(actingCreature, m_player->center, &actingCreature->path)) {
            std::cout << "creature " << actingCreature << " couldnt find path" << std::endl;
            actingCreature->markedForDeath = true;
            actingCreature->nextActionTime += .2;
            return;
        }
    }
    else if (actingCreature->path.empty()) {
        if (!m_map->findPath(actingCreature, m_player->center, &actingCreature->path)) {
            std::cout << "creature " << actingCreature << " couldnt find path" << std::endl;
            actingCreature->markedForDeath = true;
            actingCreature->nextActionTime += .2;
            return;
        }
    }
    
    if (actingCreature->path.front() == actingCreature->center)
        actingCreature->path.pop_front();
    
    if (!actingCreature->path.empty()) {
        Position nextStep = actingCreature->path.front();
        assert(posDistance(actingCreature->center, nextStep) < 3);
        
        if (unitCanMoveTo(actingCreature, nextStep)) {
            actingCreature->path.pop_front();
            m_map->removeCreature(actingCreature);
            actingCreature->setCenter(nextStep);
            m_map->placeCreature(actingCreature);
        }
    }
    
    actingCreature->nextActionTime += .2;
}

void ApplicationEngine::UpdateAnimations(float td) {
    currentTime += td;
    
    
    setPlayerAndCameraPos(Position(m_player->center.x + m_direction.x, m_player->center.y + m_direction.y), false);
    //m_testText->setText("ABC");
    //m_curController->Tic(td);
    
    for (std::list<Creature *>::iterator i = m_creatures.begin(), iEnd = m_creatures.end(); i != iEnd; i++) {
        Creature *creature = *i;
        creature->animate(currentTime);
        if (creature != m_player)
            if (creature->nextActionTime < currentTime)
                unitAct(creature);
    }
    
    for (std::list<Creature *>::iterator i = m_creatures.begin(), iEnd = m_creatures.end(); i != iEnd; ) {
        if ((*i)->markedForDeath) {
            std::list<Creature *>::iterator next = i;
            next++;
            killCreature(*i);
            i = next;
        }
        else
            i++;
    }
}

void ApplicationEngine::OnFingerUp(vec2 location) {
    m_direction = ivec2(0, 0);
    
    int width = m_mainScreenSize.x;
    int height = m_mainScreenSize.y;
    
    if (location.x > width / 3 && location.x < width * 2/3 && location.y > height / 3 && location.y < height * 2/3) {
        std::cout << "Attacking!" << std::endl;
        unitAttack(m_player);
    }
    
    m_hype = 0;
}

void ApplicationEngine::OnFingerDown(vec2 location) {
    
    vec3 playerPoint = vec3(m_player->center.x - m_player->radius, -(m_player->center.y - m_player->radius) , 0);
    vec3 touchPoint = m_renderingEngine->getPickLoc(location, playerPoint);
    //std::cout << "(" << playerPoint.x << ", " << playerPoint.y << ") - (" << touchPoint.x << ", " << touchPoint.y << ")\n";
    
    m_direction.x = 0;
    if (touchPoint.x < (playerPoint.x - 0.15)) {
        m_direction.x = -1;
    }
    if (touchPoint.x > (playerPoint.x + 0.15)) {
        m_direction.x = 1;
    }
    
    m_direction.y = 0;
    if (touchPoint.y < (playerPoint.y - 0.15)) {
        m_direction.y = 1;
    }
    if (touchPoint.y > (playerPoint.y + 0.15)) {
        m_direction.y = -1;
    }
}

void ApplicationEngine::OnFingerMove(vector<vec2> touches) {
    
    if (touches.size() < 3) {
        vec3 playerPoint = vec3(m_player->center.x - m_player->radius, -(m_player->center.y - m_player->radius) , 0);
        vec3 touchPoint = m_renderingEngine->getPickLoc(touches[0], playerPoint);
        //std::cout << "(" << playerPoint.x << ", " << playerPoint.y << ") - (" << touchPoint.x << ", " << touchPoint.y << ")\n";
        
        m_direction.x = 0;
        if (touchPoint.x < (playerPoint.x - 0.15)) {
            m_direction.x = -1;
        }
        if (touchPoint.x > (playerPoint.x + 0.15)) {
            m_direction.x = 1;
        }
        
        m_direction.y = 0;
        if (touchPoint.y < (playerPoint.y - 0.15)) {
            m_direction.y = 1;
        }
        if (touchPoint.y > (playerPoint.y + 0.15)) {
            m_direction.y = -1;
        }
        
        m_hype = 0;
    }
    else {
        float length =  touches[1].x - touches[3].x;
        float height = touches[1].y - touches[3].y;
        float nextHype = sqrt(length * length + height * height);
        if (m_hype != 0) {
            float scale = m_hype / nextHype;
            m_camera->setHight(scale);
        }
        m_hype = nextHype;
    }
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
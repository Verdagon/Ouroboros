#include "GameState.hpp"
#include <fstream>

#define BUFFER_SIZE 32

using namespace std;

GameState::GameState(IState* oldState, IMap map, ivec2 screen): State(oldState, screen) {
    m_type = StateTypeGame;
    m_curMap = map;
    LoadMap();
    
    m_backGroundView = m_gameView = new GameView(&m_curMap, screen);
    
    m_lastTranslation = m_lastProjection = m_inverted = mat4::Identity();
}

void GameState::LoadMap() {
    int total, mode;
    float x, y, z, w;
    char buffer[BUFFER_SIZE];
    
    m_curMap.Meshes = new vector<string>(0);
    m_curMap.Textures = new vector<string>(0);
    m_curMap.Visuals = new vector<Visual>(0);
    
    char* fullPath = (char*) malloc(m_curMap.ResourcePath->size() + m_curMap.Map->size() + 2);
    strcpy(fullPath, m_curMap.ResourcePath->c_str());
    strcat(fullPath, "/");
    strcat(fullPath, m_curMap.Map->c_str());
    
    cout << fullPath << "\n";
    
    ifstream file(fullPath, ios::in|ios::binary);
    
    //Read the meshes
    file.read((char*)&total, sizeof(int));
    cout << total << "\n\n";
    
    for (int i = 0; i < total; i++) {
        file.read(buffer, BUFFER_SIZE);
        m_curMap.Meshes->push_back(string(buffer));
    }
    //Add the ball mesh last
    m_curMap.Meshes->push_back("Ball.obj");
    m_curMap.BallVisual.Mesh = total;
    
    //Read the textures
    file.read((char*)&total, sizeof(int));
    for (int i = 0; i < total; i++) {
        file.read(buffer, BUFFER_SIZE);
        m_curMap.Textures->push_back(string(buffer));
    }
    //Add the ball mesh last
    m_curMap.Textures->push_back("Ball.png");
    m_curMap.BallVisual.Texture = total;
    
    //Read the Visuals
    file.read((char*)&total, sizeof(int));
    
    for (int i = 0; i < total; i++) {
        Visual newVisual;
        file.read((char*)&newVisual.Mesh, sizeof(int));
        file.read((char*)&newVisual.Texture, sizeof(int));
        file.read((char*)&mode, sizeof(int));
        if (mode == 1) {
            newVisual.Mode = BlendModeDepth;
        } else if (mode == 2) {
            newVisual.Mode = BlendModeBlend;
        } else {
            newVisual.Mode = BlendModeNone;
        }
        file.read((char*)&x, sizeof(float));
        file.read((char*)&y, sizeof(float));
        file.read((char*)&z, sizeof(float));
        newVisual.Location = vec3(x, y, z);
        file.read((char*)&x, sizeof(float));
        file.read((char*)&y, sizeof(float));
        file.read((char*)&z, sizeof(float));
        file.read((char*)&w, sizeof(float));
        newVisual.Orientation = Quaternion(x, y, z, w);
        file.read((char*)&newVisual.Scale, sizeof(float));
        
        //Create the translation matrix
        mat4 scale = mat4::Scale(newVisual.Scale);
        mat4 pos = mat4::Translate(newVisual.Location.x, newVisual.Location.y, newVisual.Location.z);
        mat4 rot = newVisual.Orientation.ToMatrix();
        newVisual.Translation = scale * rot * pos;
        
        m_curMap.Visuals->push_back(newVisual);
    }
    
    //Load the start and hole locations
    file.read((char*)&m_startLoc.x, sizeof(float));
    file.read((char*)&m_startLoc.y, sizeof(float));
    file.read((char*)&m_startLoc.z, sizeof(float));
    file.read((char*)&m_holeLoc.x, sizeof(float));
    file.read((char*)&m_holeLoc.y, sizeof(float));
    file.read((char*)&m_holeLoc.z, sizeof(float));
    
    //Set the starting view location
    m_curMap.ViewLoc = vec3(m_startLoc.x, m_startLoc.y, 20);
    
    //Set the ball visual
    m_curMap.BallVisual.Location = m_startLoc;
    m_curMap.BallVisual.Orientation = Quaternion(0,0,0,1);
    m_curMap.BallVisual.Mode = BlendModeDepth;
    m_curMap.BallVisual.Scale = 1;
    m_curMap.BallVisual.Translation = mat4::Translate(m_startLoc.x, m_startLoc.y, m_startLoc.z);
    
    //Read the areas
    file.read((char*)&total, sizeof(int));
    
    vector<Face*>* faces = new vector<Face*>(0);
    for (int i = 0; i < total; i++) {
        vec4 planeEqu;
        vec2 slopeVec;
        vector<Edge>* sides = new vector<Edge>(0);
        
        //Read the plane equation
        file.read((char*)&planeEqu.x, sizeof(float));
        file.read((char*)&planeEqu.y, sizeof(float));
        file.read((char*)&planeEqu.z, sizeof(float));
        file.read((char*)&planeEqu.w, sizeof(float));
        
        //Read the slope vector
        file.read((char*)&slopeVec.x, sizeof(float));
        file.read((char*)&slopeVec.y, sizeof(float));
        
        //Read in the edges
        for (int j = 0; j < 3; j++) {
            vec2 point;
            vec2 vector;
            float length;
            
            //Read the line segment of the edge
            file.read((char*)&point.x, sizeof(float));
            file.read((char*)&point.y, sizeof(float));
            file.read((char*)&vector.x, sizeof(float));
            file.read((char*)&vector.y, sizeof(float));
            file.read((char*)&length, sizeof(float));
            LineSeg lineSeg = LineSeg(point, vector, length);
            
            //Read the rest of the edge info
            float hitV, pLine, pMin, pMax;
            file.read((char*)&hitV, sizeof(float));
            file.read((char*)&pLine, sizeof(float));
            file.read((char*)&pMin, sizeof(float));
            file.read((char*)&pMax, sizeof(float));
            sides->push_back(Edge(lineSeg, hitV, pLine, pMin, pMax));
        }
        
        faces->push_back(new Face(planeEqu, slopeVec, sides));
    }
    
    m_ball = new BallState(m_startLoc, faces);
    
    file.close();
    free(fullPath);
}

vec3 GameState::GetViewLoc() {
    return m_curMap.ViewLoc;
}

void GameState::SetViewLoc(vec3 loc) {
    m_curMap.ViewLoc = loc;
    //m_gameView->SetViewLoc(loc);
}

void GameState::UpdateAnimations(float td) {
    //Update the ball location
    if (m_ball->Moving()) {
        m_ball->Roll(td);
        m_curMap.BallVisual.Location = m_ball->GetBallLoc();
        // implement animiated tracking of the ball
        vec3 ballLoc = m_ball->GetBallLoc();
        m_curMap.ViewLoc.x = ballLoc.x;
        m_curMap.ViewLoc.y = ballLoc.y;
    }
}

void GameState::HitBall(vec2 direction) {
    m_ball->Hit(direction);
}

void GameState::Restart() {
    m_ball->Reset();
    m_curMap.BallVisual.Location = m_ball->GetBallLoc();
    m_curMap.ViewLoc = vec3(m_startLoc.x, m_startLoc.y, 20);
}

vec3 GameState::GetScreenLoc(vec3 point) {
    vec4 tmp1, tmp2, tmp3, tmp4, tmp5;
    
    mat4 translation = m_backGroundView->GetTranslation();
    mat4 projection = m_backGroundView->GetProjection();
    
    tmp1.x = translation.x.x * point.x + translation.y.x * point.y + translation.z.x * point.z + translation.w.x;
    tmp1.y = translation.x.y * point.x + translation.y.y * point.y + translation.z.y * point.z + translation.w.y;
    tmp1.z = translation.x.z * point.x + translation.y.z * point.y + translation.z.z * point.z + translation.w.z;
    tmp1.w = translation.x.w * point.x + translation.y.w * point.y + translation.z.w * point.z + translation.w.w;
    
    tmp2.x = projection.x.x * tmp1.x + projection.y.x * tmp1.y + projection.z.x * tmp1.z + projection.w.x * tmp1.w;
    tmp2.y = projection.x.y * tmp1.x + projection.y.y * tmp1.y + projection.z.y * tmp1.z + projection.w.y * tmp1.w;
    tmp2.z = projection.x.z * tmp1.x + projection.y.z * tmp1.y + projection.z.z * tmp1.z + projection.w.z * tmp1.w;
    tmp2.w = -tmp1.z;
    
    if (tmp2.w == 0.0) {
        return vec3(0,0,0);
    }
    
    tmp2.w = 1/tmp2.w;
    
    tmp2.x *= tmp2.w;
    tmp2.y *= tmp2.w;
    tmp2.z *= tmp2.w;
    
    vec3 screenCords;
    screenCords.x = (tmp2.x * 0.5 + 0.5) * m_screen.x;
    screenCords.y = m_screen.y - (tmp2.y * 0.5 + 0.5) * m_screen.y;
    screenCords.z = tmp2.z * 0.5 +0.5;
    
    return screenCords;
}

vec3 GameState::GetPickLoc(vec2 coords, float planeZ) {
    vec4 tmp1, tmp2;
    
    //Get the model view projection matricies.
    mat4 translation = m_backGroundView->GetTranslation();
    mat4 projection = m_backGroundView->GetProjection();
    
    //Find the z value to the plane.
    tmp1.x = translation.z.x * planeZ + translation.w.x;
    tmp1.y = translation.z.y * planeZ + translation.w.y;
    tmp1.z = translation.z.z * planeZ + translation.w.z;
    tmp1.w = translation.z.w * planeZ + translation.w.w;
    
    float z = projection.x.z * tmp1.x + projection.y.z * tmp1.y + projection.z.z * tmp1.z + projection.w.z * tmp1.w;
    float w = -tmp1.z;
    
    if (w == 0.0) {
        return vec3(0,0,0);
    }
    w = 1/w;
    z *= w;
    
    //Find the world space coordnates on the plane.
    tmp1.x = (coords.x / m_screen.x) * 2 - 1;
    tmp1.y = ((m_screen.y - coords.y) / m_screen.y) * 2 - 1;
    tmp1.z = z;
    tmp1.w = 1;
    
    //Save some processing power if the inverse matrix is the same.
    if (!(translation == m_lastTranslation && projection == m_lastProjection)) {
        m_lastTranslation = translation;
        m_lastProjection = projection;
        m_inverted = translation * projection;
        m_inverted = m_inverted.Inverse();
    }
    
    tmp2.x = m_inverted.x.x * tmp1.x + m_inverted.y.x * tmp1.y + m_inverted.z.x * tmp1.z + m_inverted.w.x * tmp1.w;
    tmp2.y = m_inverted.x.y * tmp1.x + m_inverted.y.y * tmp1.y + m_inverted.z.y * tmp1.z + m_inverted.w.y * tmp1.w;
    tmp2.z = m_inverted.x.z * tmp1.x + m_inverted.y.z * tmp1.y + m_inverted.z.z * tmp1.z + m_inverted.w.z * tmp1.w;
    tmp2.w = m_inverted.x.w * tmp1.x + m_inverted.y.w * tmp1.y + m_inverted.z.w * tmp1.z + m_inverted.w.w * tmp1.w;
    
    tmp2.w = 1 / tmp2.w;
    
    vec3 point;
    point.x = tmp2.x * tmp2.w;
    point.y = tmp2.y * tmp2.w;
    point.z = planeZ;
    return point;
}

vec2 GameState::GetBallScreenLoc() {
    vec3 loc = GetScreenLoc(m_curMap.BallVisual.Location);
    return vec2(loc.x, loc.y);
}
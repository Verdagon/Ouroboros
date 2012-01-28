#include "GameView.hpp"

GameView::GameView(IMap* map, ivec2 screen) {
    m_map = map;
    m_screen = screen;
    m_listUpdated = false;
    m_viewTranslation = mat4::Identity();

}

GameView::~GameView() {
    
}

void GameView::GetVisuals(vector<Visual> *visuals) {
    visuals->resize(0);
    
    //First put in the ball
    //Update for ball rotation
    m_map->BallVisual.Group = m_group;
    vec3 ballLoc = m_map->BallVisual.Location;
    m_map->BallVisual.Translation = mat4::Translate(ballLoc.x, ballLoc.y, ballLoc.z);
    visuals->push_back(m_map->BallVisual);
    
    vector<Visual>::iterator visual = m_map->Visuals->begin();
    while (visual != m_map->Visuals->end()) {
        visual->Group = m_group;
        visual->TextureMatrix = mat4::Identity();
        visuals->push_back(*visual);
        visual++;
    }
}

void GameView::GetDrawList(DrawList *list) {
    
    //Quit if the list is already put together
    if (!m_listUpdated) {
        
        //Reset the id
        list->id = -1;
        
        //Get the group number
        m_group = list->group;
        
        //Put in the meshes
        list->Meshes.resize(0);
        vector<string>::iterator meshName = m_map->Meshes->begin();
        while (meshName != m_map->Meshes->end()) {
            list->Meshes.push_back(new Model(*meshName, *m_map->ResourcePath));
            meshName++;
        }
        
        //Put in the textures
        list->Textures.resize(0);
        vector<string>::iterator texture = m_map->Textures->begin();
        while (texture != m_map->Textures->end()) {
            list->Textures.push_back(*texture);
            texture++;
        }
    }
    
    m_listUpdated = true;
}

void GameView::AddSubview(IView *subview) {
    //Empty for now
}

void GameView::ClearSubviews() {
    //Empty for now
}

mat4 GameView::GetProjection() {
    float h = 4.0f * m_screen.x / m_screen.y;
    return mat4::Frustum(-h / 2, h / 2, -2, 2, 5, 200);
}

mat4 GameView::GetTranslation() {
    //return mat4::Translate(0, 0, -20);
    return mat4::Translate(-m_map->ViewLoc.x, -m_map->ViewLoc.y, -m_map->ViewLoc.z);
    //return m_viewTranslation;
}

/*
void GameView::SetBallLoc(vec3 loc) {
    m_map->BallVisual.Location = loc;
    mat4 trans = mat4::Translate(loc.x, loc.y, loc.z);
    m_map->BallVisual.Translation = trans;
}

void GameView::SetViewLoc(vec3 loc) {
    m_map->ViewLoc = -loc;
    m_viewTranslation = mat4::Translate(-loc.x, -loc.y, -loc.z);
}
*/
#include "StaticView.hpp"

StaticView::StaticView(string textureName, ivec2 screen) {
    m_textureName = new string(textureName);
    m_screen = screen;
    m_listUpdated = false;
}

StaticView::~StaticView() {
    delete m_textureName;
}

void StaticView::GetVisuals(vector<Visual> *visuals) {
    Visual visual = Visual();
    visual.Translation = mat4::Identity();
    visual.Mesh = m_meshIndex;
    visual.Texture = m_textureIndex;
    visual.Mode = BlendModeBlend;
    visual.Group = m_group;
    visual.TextureMatrix = mat4::Identity();
    visuals->push_back(visual);
}

void StaticView::GetDrawList(DrawList *list) {
    
    //Quit if the list is already put together
    if (!m_listUpdated) {
        //Set the projections
        //list->Projection = mat4::Parallel(-m_screen.x/2, m_screen.x/2, -m_screen.y/2, m_screen.y/2, 5, 20);
        //list->Translation = mat4::Identity();
        
        //Reset the id
        list->id = -1;
        
        //Reset flags
        list->Plane = false;
        list->CharMap = false;
        
        //Get the group number
        m_group = list->group;
        
        //Put in the mesh and texture
        list->Meshes.resize(0);
        m_meshIndex = list->Meshes.size();
        list->Meshes.push_back(new Plane(m_screen.x, m_screen.y));
        
        list->Textures.resize(0);
        m_textureIndex = list->Textures.size();
        list->Textures.push_back(*m_textureName);
    }
    
    //Update any subviews
    
    m_listUpdated = true;
}

void StaticView::AddSubview(IView *subview) {
    //Empty for now
}

void StaticView::ClearSubviews() {
    //Empty for now
}

mat4 StaticView::GetProjection() {
    return mat4::Parallel(-m_screen.x/2, m_screen.x/2, -m_screen.y/2, m_screen.y/2, 5, 20);
}

mat4 StaticView::GetTranslation() {
    return mat4::Identity();
}


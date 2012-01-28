#include "MovingView.hpp"

MovingView::MovingView(ivec2 screen) {
    m_screen = screen;
    m_listUpdated = false;
    
    //Display text for Moving Background
    m_text = new TextView("Moving Background", ivec2(0,0), 20);
}

MovingView::~MovingView() {
    
}

void MovingView::GetVisuals(vector<Visual> *visuals) {
    Visual visual = Visual();
    visual.Translation = mat4::Identity();
    visual.Mesh = m_meshIndex;
    visual.Texture = m_textureIndex;
    visual.Mode = BlendModeBlend;
    visual.Group = m_group;
    visual.TextureMatrix = mat4::Identity();
    visuals->push_back(visual);
    
    //Add in the text
    m_text->GetVisuals(visuals);
    
}

void MovingView::GetDrawList(DrawList *list) {
    
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
        std::cout << "Background index: " << m_textureIndex << "\n";
        //list->Textures.push_back("Background_Iphone.png");
        list->Textures.push_back("Background_Iphone.png");
        
        //Update any subviews
        m_text->GetDrawList(list);
    }
    
    m_listUpdated = true;
}

void MovingView::AddSubview(IView *subview) {
    //Empty for now
}

void MovingView::ClearSubviews() {
    //Empty for now
}

mat4 MovingView::GetProjection() {
    return mat4::Parallel(-m_screen.x/2, m_screen.x/2, -m_screen.y/2, m_screen.y/2, 5, 20);
}

mat4 MovingView::GetTranslation() {
    return mat4::Identity();
}

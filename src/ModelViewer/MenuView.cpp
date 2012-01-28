#include "MenuView.hpp"

MenuView::MenuView(ivec2 screen) {
    m_screen = screen;
    m_listUpdated = false;
}

MenuView::~MenuView() {

}

void MenuView::GetVisuals(vector<Visual> *visuals) {
    
    //Get the visuals from each subview
    vector<IView*>::iterator subView = m_subViews.begin();
    while (subView != m_subViews.end()) {
        (*subView)->GetVisuals(visuals);
        
        subView++;
    }
}

void MenuView::GetDrawList(DrawList *list) {
    
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
        list->Button = false;
        
        //Clear the meshes
        vector<IMesh*>::iterator mesh = list->Meshes.begin();
        while (mesh != list->Meshes.end()) {
            delete *mesh;
            mesh++;
        }
        list->Meshes.resize(0);
        
        //Clear the textures
        list->Textures.resize(0);
    }
    
    //Add to the list from the subviews
    vector<IView*>::iterator subView = m_subViews.begin();
    while (subView != m_subViews.end()) {
        (*subView)->GetDrawList(list);
        
        subView++;
    }
    
    m_listUpdated = true;
}

void MenuView::AddSubview(IView *subview) {
    m_subViews.push_back(subview);
}

void MenuView::ClearSubviews() {
    m_subViews.resize(0);
}

mat4 MenuView::GetProjection() {
    return mat4::Parallel(-m_screen.x/2, m_screen.x/2, -m_screen.y/2, m_screen.y/2, 5, 20);
}

mat4 MenuView::GetTranslation() {
    return mat4::Identity();
}
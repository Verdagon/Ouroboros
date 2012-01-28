#include "ButtonView.hpp"

ButtonView::ButtonView(string text, ivec2 location, ivec2 size) {
    m_textView = new TextView(text, location, size.y);
    m_location = location;
    m_size = size * 0.4;
}

ButtonView::~ButtonView() {
    delete m_textView;
}

void ButtonView::GetVisuals(vector<Visual> *visuals) {
    
    //Middel of button
    Visual visual = Visual();
    visual.Mode = BlendModeBlend;
    visual.Group = m_group;
    visual.Mesh = m_planeIndex;
    visual.Texture = m_buttonIndex;
    mat4 size = mat4::Scale(m_size.x, m_size.y, 1);
    visual.Translation = size * mat4::Translate(m_location.x, m_location.y, 0);
    visual.TextureMatrix = mat4::Identity();
    visuals->push_back(visual);
    
    //Set the corner size
    size = mat4::Scale(8);
    
    //Top Left of button
    visual.Texture = m_buttonIndex + 1;
    visual.Translation = size * mat4::Translate(m_location.x - m_size.x - 8, m_location.y + m_size.y + 8, 0);
    visuals->push_back(visual);
    
    //Top Right of button
    visual.Texture = m_buttonIndex + 2;
    visual.Translation = size * mat4::Translate(m_location.x + m_size.x + 8, m_location.y + m_size.y + 8, 0);
    visuals->push_back(visual);
    
    //Bottom Right of button
    visual.Texture = m_buttonIndex + 3;
    visual.Translation = size * mat4::Translate(m_location.x + m_size.x + 8, m_location.y - m_size.y - 8, 0);
    visuals->push_back(visual);
    
    //Bottom Right of button
    visual.Texture = m_buttonIndex + 4;
    visual.Translation = size * mat4::Translate(m_location.x - m_size.x - 8, m_location.y - m_size.y - 8, 0);
    visuals->push_back(visual);
    
    //Set the Top and Bottom size
    size = mat4::Scale(m_size.x, 8, 1);
    
    //Top of Button
    visual.Texture = m_buttonIndex + 5;
    visual.Translation = size * mat4::Translate(m_location.x, m_location.y + m_size.y + 8, 0);
    visuals->push_back(visual);
    
    //Bottom of Button
    visual.Texture = m_buttonIndex + 6;
    visual.Translation = size * mat4::Translate(m_location.x, m_location.y - m_size.y - 8, 0);
    visuals->push_back(visual);
    
    //Set the Left and Right size
    size = mat4::Scale(8, m_size.y, 1);
    
    //Left of Button
    visual.Texture = m_buttonIndex + 7;
    visual.Translation = size * mat4::Translate(m_location.x - m_size.x - 8, m_location.y, 0);
    visuals->push_back(visual);
    
    //Right of Button
    visual.Texture = m_buttonIndex + 8;
    visual.Translation = size * mat4::Translate(m_location.x + m_size.x + 8, m_location.y, 0);
    visuals->push_back(visual);
    
    m_textView->GetVisuals(visuals);
}

void ButtonView::GetDrawList(DrawList *list) {
    
    //Check for a unit plane
    if (!list->Plane) {
        list->PlaneIndex = list->Meshes.size();
        list->Meshes.push_back(new Plane(2, 2));
        list->Plane = true;
    }
    
    //Check for the button textures
    if (!list->Button) {
        list->ButtonIndex = list->Textures.size();
        list->Textures.push_back("ButtonM.png");
        list->Textures.push_back("ButtonTL.png");
        list->Textures.push_back("ButtonTR.png");
        list->Textures.push_back("ButtonBR.png");
        list->Textures.push_back("ButtonBL.png");
        list->Textures.push_back("ButtonT.png");
        list->Textures.push_back("ButtonB.png");
        list->Textures.push_back("ButtonL.png");
        list->Textures.push_back("ButtonR.png");
        list->Button = true;
    }
    
    m_textView->GetDrawList(list);
    
    //Get the plane and button indexies
    m_planeIndex = list->PlaneIndex;
    m_buttonIndex = list->ButtonIndex;
    m_group = list->group;
}

void ButtonView::AddSubview(IView *subview) {
    //Not implemented
}

void ButtonView::ClearSubviews() {
    //Empty for now
}

mat4 ButtonView::GetProjection() {
    return mat4::Identity();
}

mat4 ButtonView::GetTranslation() {
    return mat4::Identity();
}
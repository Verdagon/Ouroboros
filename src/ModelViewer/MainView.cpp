#include "MainView.hpp"

MainView::MainView(DeviceType device, int width, int height) {
    m_viewPort = ivec2(width, height);
    
    //Create a draw list for the loading screen (should last for the duration of the program)
    m_loadingList = new DrawList();
    m_loadingList->id = -1;
    m_loadingList->group = 0;
    //m_loadingList->Projection = mat4::Parallel(-width/2, width/2, -height/2, height/2, 5, 20);
    //m_loadingList->Translation = mat4::Identity();
    m_loadingList->Meshes.push_back(new Plane(width, height));
    m_loadingList->Textures.push_back("Loading_Iphone.png");
    
    m_backGroundList = new DrawList();
    m_backGroundList->id = -1;
    m_backGroundList->group = 1;
    
    m_foreGroundList = new DrawList();
    m_foreGroundList->id = -1;
    m_foreGroundList->group = 2;
    
    //Create a visual for the loading screen
    m_loadingVisual = Visual();
    m_loadingVisual.Translation = mat4::Identity();
    m_loadingVisual.Mesh = 0;
    m_loadingVisual.Texture = 0;
    m_loadingVisual.Mode = BlendModeBlend;
    m_loadingVisual.Group = 0;
    m_loadingVisual.TextureMatrix = mat4::Identity();
    
    m_loading = true;
}

MainView::~MainView() {
    delete m_loadingList->Meshes[0];
}

void MainView::GetVisuals(vector<Visual> *visuals) {
    visuals->resize(0);
    
    if (m_loading) {
        //Show only the loading screen when loading
        visuals->push_back(m_loadingVisual);
    } else {
        //Get the visuals from background and foreground views
        m_backGroundView->GetVisuals(visuals);
        m_foreGroundView->GetVisuals(visuals);
    }
}

void MainView::GetLists(vector<DrawList*> *list) {
    
    // Put in the loading list
    list->resize(0);
    list->push_back(m_loadingList);
    
    // Create new lists for the background and foreground
    m_backGroundView->GetDrawList(m_backGroundList);
    list->push_back(m_backGroundList);
    
    m_foreGroundView->GetDrawList(m_foreGroundList);
    list->push_back(m_foreGroundList);
    
    // Take off the loading screen
    m_loading = false;
}

void MainView::SetStateViews(IState* state) {
    m_loading = true;
    m_backGroundView = state->GetBackGroundView();
    m_foreGroundView = state->GetForeGroundView();
}

mat4 MainView::GetProjection(int group) {
    if (group == 1) {
        return m_backGroundView->GetProjection();
    } else if (group == 2) {
        return m_foreGroundView->GetProjection();
    } else {
        return mat4::Parallel(-m_viewPort.x/2, m_viewPort.x/2, -m_viewPort.y/2, m_viewPort.y/2, 5, 20);
    }
}

mat4 MainView::GetTranslation(int group) {
    if (group == 1) {
        return m_backGroundView->GetTranslation();
    } else if (group == 2) {
        return m_foreGroundView->GetTranslation();
    } else {
        return mat4::Identity();
    }
}



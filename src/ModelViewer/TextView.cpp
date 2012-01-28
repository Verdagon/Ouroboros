#include "TextView.hpp"

#define SIZE 40
#define SCALE 0.125
#define WORD_TOTAL_SIZE 32

TextView::TextView(string text, ivec2 location, int size) {
    m_text = new string(text);
    m_location = location;
    m_size = size;
    m_assembled = false;
}

TextView::~TextView() {
    delete m_text;
}

void TextView::GetVisuals(vector<Visual> *visuals) {
    
    // Put together the letters if not done yet
    if (!m_assembled) {
        AssembleChars();
        m_assembled = true;
    }
    
    // Add each letter as a visual
    vector<Visual>::iterator visual = m_letters.begin();
    while (visual != m_letters.end()) {
        visuals->push_back(*visual);
        visual++;
    }
}

void TextView::GetDrawList(DrawList *list) {
    
    //Check for a unit plane
    if (!list->Plane) {
        list->PlaneIndex = list->Meshes.size();
        list->Meshes.push_back(new Plane(2, 2));
        list->Plane = true;
    }
    
    //Check for a char map
    if (!list->CharMap) {
        list->CharMapIndex = list->Textures.size();
        std::cout << "Char map index: " << list->CharMapIndex << "\n";
        list->Textures.push_back("charmap_white.png");
        list->CharMap = true;
    }
    
    //Get the plane and char map indexies
    m_planeIndex = list->PlaneIndex;
    m_charMapIndex = list->CharMapIndex;
    m_group = list->group;
}

void TextView::AddSubview(IView *subview) {
    //Not implemented
}

void TextView::ClearSubviews() {
    //Empty for now
}

void TextView::AssembleChars() {
    //Limit the text size to 32 chars
    int mapValues[WORD_TOTAL_SIZE];
    int spaces[WORD_TOTAL_SIZE + 1];
    int total;
    
    const char *c;
    c = m_text->c_str();
    
    // Get the values for the char map.
    for (total = 0; total < WORD_TOTAL_SIZE && c[total] != '\0'; total++) {
        int value = (int) c[total];
        if (value >= 48 && value <= 57) {
            //Numbers
            mapValues[total] = value - 46;
        } else if (value >= 65 && value <= 90) {
            //Upper case letters
            mapValues[total] = value - 53;
        } else if (value >= 97 && value <= 122) {
            //Lower case letters
            mapValues[total] = value - 59;
        } else if (value == 32) {
            mapValues[total] = 0;
        } else {
            mapValues[total] = 1;
        }
    }
    
    spaces[0] = 0;
    int spacingSmallest = m_size / 2;
    int spacingSmall = m_size / 1.5;
    int spacingLowerCase = m_size / 1.2;
    int spacingUpperCase = m_size;
    int spacingLarge = m_size * 1.1;
    
    for (int i = 1; i <= total; i++) {
        int letter = mapValues[i-1];
        if (letter == 1 || letter == 46 || letter == 47 || letter == 49) {
            spaces[i] = spaces[i-1] + spacingSmallest;
        } else if (letter == 3 || letter == 20 || letter == 21 || letter == 43 || letter == 55 || letter == 56 || letter == 57) {
            spaces[i] = spaces[i-1] + spacingSmall;
        } else if (letter == 24 || letter == 34 || letter == 50 || letter == 60) {
            spaces[i] = spaces[i-1] + spacingLarge;
        } else if (letter >= 12 && letter <= 37) {
            spaces[i] = spaces[i-1] + spacingUpperCase;
        } else {
            spaces[i] = spaces[i-1] + spacingLowerCase;
        }
        
        
        spaces[i-1] = spaces[i] - ((spaces[i] - spaces[i-1]) >> 1);
    }
    
    int shift = (spaces[0] + spaces[total-1]) >> 1;
    
    // Assemble the Visuals
    m_letters.resize(0);
    Visual visual = Visual();
    visual.Mode = BlendModeBlend;
    visual.Group = m_group;
    visual.Mesh = m_planeIndex;
    visual.Texture = m_charMapIndex;
    mat4 size = mat4::Scale(m_size);
    mat4 scale = mat4::Scale(SCALE);
    
    for (int i = 0; i < total; i++) {
        spaces[i] = spaces[i] - shift;
        visual.Translation = size * mat4::Translate(m_location.x + spaces[i], m_location.y, 0);
        visual.TextureMatrix = scale * mat4::Translate(mapValues[i] % 8 * SCALE, mapValues[i] / 8 * SCALE, 0);
        m_letters.push_back(visual);
    }
}

mat4 TextView::GetProjection() {
    return mat4::Identity();
}

mat4 TextView::GetTranslation() {
    return mat4::Identity();
}

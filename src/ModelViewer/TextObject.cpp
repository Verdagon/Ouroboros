#include "TextObject.h"
#include "Mesh.h"
#include <iostream>



TextObject::TextObject() {
    m_meshList = list<IMesh *>(0);
}

TextObject::TextObject(ivec2 dimensions, ivec2 loc) {
    m_dimensions = dimensions;
    m_loc = loc;
    m_meshList = list<IMesh *>(0);
    //push in one plane 
    m_plane = new Plane("charmap_white.png", vec3(dimensions.y, dimensions.y, 0), vec3(loc.x, loc.y, 10));
    m_plane->display = false;
}

TextObject::~TextObject() {
    list<IMesh *>::iterator mesh;
    for (mesh = m_meshList.begin(); mesh != m_meshList.end(); ++mesh) {
            delete *mesh;
    }
}

void TextObject::setText(string text) {
    list<IMesh *>::iterator mesh;
    for (mesh = m_meshList.begin(); mesh != m_meshList.end(); ++mesh) {
        if (*mesh != m_plane)
            delete *mesh;
    }
    m_meshList.clear();
    
    //Limit the text size to 32 chars
    int mapValues[WORD_TOTAL_SIZE];
    int spaces[WORD_TOTAL_SIZE + 1];
    int total;
    
    const char *c;
    c = text.c_str();
    
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
    int spacingSmallest = m_dimensions.y / 2;
    int spacingSmall = m_dimensions.y / 1.5;
    int spacingLowerCase = m_dimensions.y / 1.2;
    int spacingUpperCase = m_dimensions.y;
    int spacingLarge = m_dimensions.y * 1.1;
    
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
    
    for (int i = 0; i < total; i++) {
        spaces[i] = spaces[i] - shift;
        Plane *object = new Plane((Plane *)m_plane);
        object->meshMtx = mat4::Translate((float)m_loc.x + spaces[i], (float)m_loc.y, 0);
        mat4 scale = mat4::Scale(SCALE);
        object->textureMtx = scale * mat4::Translate(mapValues[i] % 8 * SCALE, mapValues[i] / 8 * SCALE, 0);
        m_meshList.push_back(object);
    }
    
    /*
    // Assemble the Objects
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
    }*/
    
}

list<IMesh *>* TextObject::getMeshes() {
    // Push in the master plane to get 
    if (m_meshList.size() == 0)
        m_meshList.push_back(m_plane);
    return &m_meshList;
}

#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#include <iostream>
#include "Interfaces.hpp"
#include "Matrix.hpp"

namespace ES1 {

struct Mesh {
    GLuint VertexBuffer;
    GLuint IndexBuffer;
    int IndexCount;
};

struct Texture {
    GLuint Texture;
};

struct DrawGroup {
    vector<Mesh> meshes;
    vector<Texture> textures;
    //mat4* Projection;
    //mat4* Translation;
    int group;
    int id;
};
    
class RenderingEngine : public IRenderingEngine {
public:
    RenderingEngine(IResourceManager* resourceManager);
    void Initialize(IMainView* view, int width, int height);
    void ReLoadLists();
    void Render();
private:
    void AddGroup(DrawGroup* newGroup, DrawList* list);
    void RemoveGroup(DrawGroup* group);
    void SetBlending(BlendMode mode);
    void SetPngTexture(string name) const;
    vector<DrawGroup> m_drawgroups;
    ivec2 m_mainScreenSize;
    BlendMode m_blendMode;
    IMainView* m_view;
    GLuint m_colorRenderbuffer;
    GLuint m_depthRenderbuffer;
    IResourceManager* m_resourceManager;
    int m_listCounter;
    int m_curGroup;
};

IRenderingEngine* CreateRenderingEngine(IResourceManager* resourceManager)
{
    return new RenderingEngine(resourceManager);
}

RenderingEngine::RenderingEngine(IResourceManager* resourceManager) {
    m_resourceManager = resourceManager;
    glGenRenderbuffersOES(1, &m_colorRenderbuffer);
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, m_colorRenderbuffer);
    m_blendMode = BlendModeNone;
    m_listCounter = 0;
    //m_drawgroups = new vector<DrawGroup*>(0);
    m_curGroup = -1;
}

void RenderingEngine::Initialize(IMainView* view, int width, int height)
{
    m_view = view;
    m_mainScreenSize = ivec2(width, height);
    
    // Use if render buffer is different size than screen
    //int width, height;
    //glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &width);
    //glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &height);
    
    // Create a depth buffer the same size as the color buffer.
    glGenRenderbuffersOES(1, &m_depthRenderbuffer);
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, m_depthRenderbuffer);
    glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT16_OES, width, height);
    
    // Create the framebuffer object.
    GLuint frameBuffer;
    glGenFramebuffersOES(1, &frameBuffer);
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, frameBuffer);
    glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, m_colorRenderbuffer);
    glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, m_depthRenderbuffer);
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, m_colorRenderbuffer);
    
    // Set up various GL state.
    glEnableClientState(GL_VERTEX_ARRAY);
    //glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);
    glColor4f(1, 1, 1, 1);
}

void RenderingEngine::ReLoadLists() {
    vector<DrawList*> lists = vector<DrawList*>(0);
    m_view->GetLists(&lists);
    
    //Iterate through the list for anyting new using ids
    vector<DrawList*>::iterator newDrawlist = lists.begin();
    for (int i = 0; newDrawlist != lists.end(); i++, newDrawlist++) {
        if (m_drawgroups.size() <= i) {
            m_drawgroups.resize(i + 1);
            AddGroup(&m_drawgroups[i], *newDrawlist);
            //DrawGroup newGroup = AddGroup(*newDrawlist);
            //m_drawgroups[i] = newGroup;
        } else if (m_drawgroups[i].id != (*newDrawlist)->id ) {
            RemoveGroup(&m_drawgroups[i]);
            AddGroup(&m_drawgroups[i], *newDrawlist);
            //m_drawgroups[i] = AddGroup(*newDrawlist);
        }
        //Do nothing if already there.
    }
}

void RenderingEngine::AddGroup(DrawGroup* newGroup, DrawList *list) {
    
    // Iterate through the meshes
    vector<IMesh*>::iterator mesh = list->Meshes.begin();
    while (mesh != list->Meshes.end()) {
        
        // Create the VBO for the vertices.
        vector<float> vertices;
        (*mesh)->GenerateVertices(vertices);
        GLuint vertexBuffer;
        glGenBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), &vertices[0], GL_STATIC_DRAW);
        
        // Create the VBO for the indices
        int indexCount = (*mesh)->GetTriangleIndexCount();
        GLuint indexBuffer;
        vector<GLushort> indices(indexCount);
        (*mesh)->GenerateTriangleIndices(indices);
        glGenBuffers(1, &indexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(GLushort), &indices[0], GL_STATIC_DRAW);
        
        // Add the mesh to the group
        Mesh newMesh = Mesh();
        newMesh.VertexBuffer = vertexBuffer;
        newMesh.IndexBuffer = indexBuffer;
        newMesh.IndexCount = indexCount;
        newGroup->meshes.push_back(newMesh);
        
        mesh++;
    }
    
    // Iterate through the textures
    vector<string>::iterator texture = list->Textures.begin();
    while (texture != list->Textures.end()) {
        
        // Open each texture
        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        SetPngTexture(*texture);
        Texture newTexture = Texture();
        newTexture.Texture = textureID;
        
        newGroup->textures.push_back(newTexture);
        
        texture++;
    }
    
    //Generate a simple id for the group and list
    int id = ++m_listCounter;
    list->id = id;
    newGroup->id = id;
    newGroup->group = list->group;
    
    //Get the matrixies
    //newGroup->Projection = m_view->GetProjection(list->group);
    //newGroup->Translation = m_view->GetTranslation(list->group);
}

void RenderingEngine::RemoveGroup(DrawGroup* group) {
    
    // Remove the meshes
    vector<Mesh>::iterator mesh = group->meshes.begin();
    while (mesh != group->meshes.end()) {
        glDeleteBuffers(1, &mesh->VertexBuffer);
        glDeleteBuffers(1, &mesh->IndexBuffer);
        
        mesh++;
    }
    // Resize the vector accordingly
    group->meshes.resize(0);
    
    // Remove the textures
    vector<Texture>::iterator texture = group->textures.begin();
    while (texture != group->textures.end()) {
        glDeleteTextures(1, &texture->Texture);
        
        texture++;
    }
    group->textures.resize(0);
}

void RenderingEngine::SetBlending(BlendMode mode) {
    if (m_blendMode != mode) {
        switch (m_blendMode) {
            case BlendModeNone:
                //Do nothing.
                break;
            case BlendModeDepth:
                glDisable(GL_DEPTH_TEST);
                break;
            case BlendModeDepthLights:
                //Implement this later.
                break;
            case BlendModeBlend:
                glDisable(GL_BLEND);
                break;
        }
        switch (mode) {
            case BlendModeNone:
                //Do nothing.
                break;
            case BlendModeDepth:
                glEnable(GL_DEPTH_TEST);
                break;
            case BlendModeDepthLights:
                //Implement this later.
                break;
            case BlendModeBlend:
                glEnable(GL_BLEND);
                break;
        }
    }
    m_blendMode = mode;
}

void RenderingEngine::Render() {
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Use if render buffer is different size than screen
    //int width, height;
    //glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &width);
    //glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &height);
    
    //glViewport(0, 0, width, height);
    glViewport(0, 0, m_mainScreenSize.x, m_mainScreenSize.y);
    
    vector<Visual> visuals = vector<Visual>(0);
    m_view->GetVisuals(&visuals);
    vector<Visual>::iterator visual = visuals.begin();
    while (visual != visuals.end()) {
        if (m_curGroup != visual->Group) {
            m_curGroup = visual->Group;
            
            //Set the Group projection
            glMatrixMode(GL_PROJECTION);
            //mat4* projection = m_drawgroups[m_curGroup].Projection;
            mat4 projection = m_view->GetProjection(m_curGroup);
            glLoadMatrixf(projection.Pointer());
            
            //Set the Group translation
            glMatrixMode(GL_MODELVIEW);
            //mat4* translation = m_drawgroups[m_curGroup].Translation;
            mat4 translation = m_view->GetTranslation(m_curGroup);
            glLoadMatrixf(translation.Pointer());
        }
        
        //Set the visual translation (needs to be setup)
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glMultMatrixf(visual->Translation.Pointer());
        
        glMatrixMode(GL_TEXTURE);
        glPushMatrix();
        glLoadMatrixf(visual->TextureMatrix.Pointer());
        
        //Set the blending mode of the visual
        SetBlending(visual->Mode);
        
        //Draw the Mesh
        //int stride = sizeof(vec3) + sizeof(vec3) + sizeof(vec2);
        int stride = sizeof(vec3) + sizeof(vec2);
        //const GLvoid* texCoordOffset = (const GLvoid*) (2 * sizeof(vec3));
        const GLvoid* texCoordOffset = (const GLvoid*) (sizeof(vec3));
        const Mesh& mesh = m_drawgroups[m_curGroup].meshes[visual->Mesh];
        const Texture& texture = m_drawgroups[m_curGroup].textures[visual->Texture];
        
        glBindTexture(GL_TEXTURE_2D, texture.Texture);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.VertexBuffer);
        glVertexPointer(3, GL_FLOAT, stride, 0);
        //const GLvoid* normalOffset = (const GLvoid*) sizeof(vec3);
        //glNormalPointer(GL_FLOAT, stride, normalOffset);
        glTexCoordPointer(2, GL_FLOAT, stride, texCoordOffset);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.IndexBuffer);
        glDrawElements(GL_TRIANGLES, mesh.IndexCount, GL_UNSIGNED_SHORT, 0);
        
        //Restore the group matrix
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        
        //Restore the texture matrix if modified
        glMatrixMode(GL_TEXTURE);
        glPopMatrix();
        
        //Increment the iterator
        visual++;
    }
}

void RenderingEngine::SetPngTexture(string name) const {
    TextureDescription description = m_resourceManager->LoadPngImage(name);
    
    GLenum format;
    switch (description.Format) {
        case TextureFormatGray:
            format = GL_LUMINANCE;
            break;
        case TextureFormatGrayAlpha:
            format = GL_LUMINANCE_ALPHA;
            break;
        case TextureFormatRgb:
            format = GL_RGB;
            break;
        case TextureFormatRgba:
            format = GL_RGBA;
    }
    
    GLenum type;
    switch (description.BitsPerComponent) {
        case 8:
            type = GL_UNSIGNED_BYTE;
            break;
        case 4:
            if (format == GL_RGBA) {
                type = GL_UNSIGNED_SHORT_4_4_4_4;
                break;
            }
        default:
            //assert(!"Unsupported format.");
            exit(1);
    }
    
    void* data = m_resourceManager->GetImageData();
    ivec2 size = description.size;
    glTexImage2D(GL_TEXTURE_2D, 0, format, size.x, size.y, 0, format, type, data);
    m_resourceManager->UnloadImage();
}

}

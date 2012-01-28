#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#include "Interfaces.hpp"
#include "Matrix.hpp"
#include <iostream>

namespace ES2
{

struct UniformHandles {
    GLuint Modelview;
    GLuint Projection;
    GLuint NormalMatrix;
    GLuint TextureMatrix;
    GLuint LightPosition;
    GLint AmbientMaterial;
    GLint SpecularMaterial;
    GLint Shininess;
};

struct AttributeHandels {
    GLint Position;
    GLint Normal;
    GLint DiffuseMaterial;
    GLint TextureCoord;
};

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
    //mat4 Projection;
    //mat4 Translation;
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
    GLuint BuildShader(string* source, GLenum shaderType) const;
    GLuint BuildProgram(string* vShader, string* fShader) const;
    void SetPngTexture(string name) const;
    IMainView* m_view;
    BlendMode m_blendMode;
    vector<DrawGroup> m_drawgroups;
    ivec2 m_mainScreenSize;
    GLuint m_colorRenderbuffer;
    GLuint m_depthRenderbuffer;
    IResourceManager* m_resourceManager;
    UniformHandles m_uniforms;
    AttributeHandels m_attributes;
    mat4 m_curModelView;
    int m_listCounter;
    int m_curGroup;
};

IRenderingEngine* CreateRenderingEngine(IResourceManager* resourceManager)
{
    return new RenderingEngine(resourceManager);
}

RenderingEngine::RenderingEngine(IResourceManager* resourceManager)
{
    m_resourceManager = resourceManager;
    glGenRenderbuffers(1, &m_colorRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_colorRenderbuffer);
    //m_drawgroups = new vector<DrawGroup*>(0);
    m_curGroup = -1;
    m_listCounter = 0;
    m_blendMode = BlendModeNone;
    m_curModelView = mat4::Identity();
}

void RenderingEngine::Initialize(IMainView* view, int width, int height)
{
    m_view = view;
    m_mainScreenSize = ivec2(width, height);
    
    // Extract width and height.
    //int width, height;
    //glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
    //glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
    
    // Create a depth buffer that has the same size as the color buffer.
    glGenRenderbuffers(1, &m_depthRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
    
    // Create the framebuffer object.
    GLuint framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_colorRenderbuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_colorRenderbuffer);
    
    // Create the GLSL program.
    string* vertexShader = m_resourceManager->GetVertexShader("Simple");
    string* fragmentShader = m_resourceManager->GetFragmentShader ("Simple");
    GLuint program = BuildProgram(vertexShader , fragmentShader);
    m_resourceManager->UnloadShaders();
    glUseProgram(program);
    
    // Extract the handles to attributes and uniforms.
    m_attributes.Position = glGetAttribLocation(program, "Position");
    m_attributes.Normal = glGetAttribLocation(program, "Normal");
    m_attributes.DiffuseMaterial = glGetAttribLocation(program, "DiffuseMaterial");
    m_attributes.TextureCoord = glGetAttribLocation(program, "TextureCoord");
    m_uniforms.Projection = glGetUniformLocation(program, "Projection");
    m_uniforms.Modelview = glGetUniformLocation(program, "Modelview");
    m_uniforms.NormalMatrix = glGetUniformLocation(program, "NormalMatrix");
    m_uniforms.TextureMatrix = glGetUniformLocation(program, "TextureMatrix");
    m_uniforms.LightPosition = glGetUniformLocation(program, "LightPosition");
    m_uniforms.AmbientMaterial = glGetUniformLocation(program, "AmbientMaterial");
    m_uniforms.SpecularMaterial = glGetUniformLocation(program, "SpecularMaterial");
    m_uniforms.Shininess = glGetUniformLocation(program, "Shininess");
    
    // Initialize various state.
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnableVertexAttribArray(m_attributes.Position);
    glEnableVertexAttribArray(m_attributes.Normal);
    glEnableVertexAttribArray(m_attributes.TextureCoord);
    m_blendMode = BlendModeNone;
}
    
void RenderingEngine::ReLoadLists() {
    //Get the draw list from main view
    //vector<DrawList*>* newDrawlists = m_view->GetLists();
    vector<DrawList*> lists = vector<DrawList*>(0);
    m_view->GetLists(&lists);
    
    //Iterate through the list for anyting new using ids
    vector<DrawList*>::iterator newDrawlist = lists.begin();
    for (int i = 0; newDrawlist != lists.end(); i++, newDrawlist++) {
        if (m_drawgroups.size() <= i) {
            m_drawgroups.resize(i + 1);
            //DrawGroup* newGroup = AddGroup(*newDrawlist);
            //m_drawgroups[i] = newGroup;
            AddGroup(&m_drawgroups[i], *newDrawlist);
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
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        SetPngTexture(*texture);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        // Add the texture to the group
        Texture newTexture = Texture();
        newTexture.Texture = textureID;
        newGroup->textures.push_back(newTexture);
        
        texture++;
    }
    
    //Generate a simple id for the group and list
    int id = ++m_listCounter;
    list->id = id;
    newGroup->id = id;
    
    //Get the matrixies
    //newGroup->Projection = list->Projection;
    //newGroup->Translation = list->Translation;
}

// Update to remove everything
void RenderingEngine::RemoveGroup(DrawGroup* group) {
    // Remove the meshes
    vector<Mesh>::iterator mesh = group->meshes.begin();
    while (mesh != group->meshes.end()) {
        glDeleteBuffers(1, &mesh->VertexBuffer);
        glDeleteBuffers(1, &mesh->IndexBuffer);
        
        mesh++;
    }
    group->meshes.resize(0);
    
    // Remove the textures
    vector<Texture>::iterator texture = group->textures.begin();
    while (texture != group->textures.end()) {
        glDeleteTextures(1, &texture->Texture);
        
        texture++;
    }
    group->textures.resize(0);
}

void RenderingEngine::SetBlending(BlendMode mode)
{
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

    
void RenderingEngine::Render()
{
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Replace this
    int width, height;
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
    
    // Set the main viewport.
    glViewport(0, 0, width, height);
    
    vector<Visual> visuals = vector<Visual>(0);
    m_view->GetVisuals(&visuals);
    vector<Visual>::iterator visual = visuals.begin();
    while (visual != visuals.end()) {
        if (m_curGroup != visual->Group) {
            m_curGroup = visual->Group;
            
            //Set the Group projection
            //mat4 projection = m_drawgroups[m_curGroup].Projection;
            mat4 projection = m_view->GetProjection(m_curGroup);
            glUniformMatrix4fv(m_uniforms.Projection, 1, 0, projection.Pointer());
            
            //Set the Group translation
            //m_curModelView = m_drawgroups[m_curGroup].Translation;
            m_curModelView = m_view->GetTranslation(m_curGroup);
        }
        
        //Apply the visual translation
        mat4 modelview = visual->Translation * m_curModelView;
        glUniformMatrix4fv(m_uniforms.Modelview, 1, 0, modelview.Pointer());
        
        //Apply the texture matrix
        mat4 texturematrix = visual->TextureMatrix;
        glUniformMatrix4fv(m_uniforms.TextureMatrix, 1, 0, texturematrix.Pointer());
        //mat4 texturematrixfull = visual->TextureMatrix;
        //mat2 texturematrix = texturematrixfull.ToMat2();
        //glUniformMatrix2fv(m_uniforms.TextureMatrix, 1, 0, texturematrix.Pointer());
        
        // Set the normal matrix. (use for lighting)
        //mat3 normalMatrix = modelview.ToMat3();
        //glUniformMatrix3fv(m_uniforms.NormalMatrix, 1, 0, normalMatrix.Pointer());
        
        // Set the diffuse color.
        glVertexAttrib4f(m_attributes.DiffuseMaterial, 1, 1, 1, 1);
        
        //Set the blending mode
        SetBlending(visual->Mode);
        
        // Draw the mesh.
        //int stride = sizeof(vec3) + sizeof(vec3) + sizeof(vec2);
        int stride = sizeof(vec3) + sizeof(vec2);
        //const GLvoid* normalOffset = (const GLvoid*) sizeof(vec3);
        //const GLvoid* texCoordOffset = (const GLvoid*) (2 * sizeof(vec3));
        const GLvoid* texCoordOffset = (const GLvoid*) (sizeof(vec3));
        GLint position = m_attributes.Position;
        //GLint normal = m_attributes.Normal;
        GLint texCoord = m_attributes.TextureCoord;
        
        const Mesh& mesh = m_drawgroups[m_curGroup].meshes[visual->Mesh];
        const Texture& texture = m_drawgroups[m_curGroup].textures[visual->Texture];
        
        glBindTexture(GL_TEXTURE_2D, texture.Texture);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.VertexBuffer);
        glVertexAttribPointer(position, 3, GL_FLOAT, GL_FALSE, stride, 0);
        //glVertexAttribPointer(normal, 3, GL_FLOAT, GL_FALSE, stride, normalOffset);
        glVertexAttribPointer(texCoord, 2, GL_FLOAT, GL_FALSE, stride, texCoordOffset);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.IndexBuffer);
        glDrawElements(GL_TRIANGLES, mesh.IndexCount, GL_UNSIGNED_SHORT, 0);
        
        visual++;
    }
}

GLuint RenderingEngine::BuildShader(string* source, GLenum shaderType) const
{
    GLuint shaderHandle = glCreateShader(shaderType);
    const char* cstring = source->c_str();
    glShaderSource(shaderHandle, 1, &cstring, 0);
    glCompileShader(shaderHandle);
    
    GLint compileSuccess;
    glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compileSuccess);
    
    if (compileSuccess == GL_FALSE) {
        GLchar messages[256];
        glGetShaderInfoLog(shaderHandle, sizeof(messages), 0, &messages[0]);
        std::cout << messages;
        exit(1);
    }
    
    return shaderHandle;
}

GLuint RenderingEngine::BuildProgram(string* vertexShaderSource, string* fragmentShaderSource) const
{
    GLuint vertexShader = BuildShader(vertexShaderSource, GL_VERTEX_SHADER);
    GLuint fragmentShader = BuildShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
    
    GLuint programHandle = glCreateProgram();
    glAttachShader(programHandle, vertexShader);
    glAttachShader(programHandle, fragmentShader);
    glLinkProgram(programHandle);
    
    GLint linkSuccess;
    glGetProgramiv(programHandle, GL_LINK_STATUS, &linkSuccess);
    if (linkSuccess == GL_FALSE) {
        GLchar messages[256];
        glGetProgramInfoLog(programHandle, sizeof(messages), 0, &messages[0]);
        std::cout << messages;
        exit(1);
    }
    
    return programHandle;
}

void RenderingEngine::SetPngTexture(string name) const
{
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
            std::cout << "Unsupported format\n";
            exit(1);
    }
    
    void* data = m_resourceManager->GetImageData();
    ivec2 size = description.size;
    glTexImage2D(GL_TEXTURE_2D, 0, format, size.x, size.y, 0, format, type, data);
    m_resourceManager->UnloadImage();
}

}
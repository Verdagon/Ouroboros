#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#include "Interfaces.h"
#include "Matrix.hpp"
#include <iostream>

#define VERTEX_STRIDE 11
#define NORMAL_OFFSET 3
#define TEXTURE_OFFSET 9

struct UniformHandles {
    GLuint modelview;
    GLuint projection;
    GLuint normalMatrix;
    GLuint textureMatrix;
    GLuint lightPosition;
    GLint ambientMaterial;
    GLint specularMaterial;
    GLint shininess;
    GLint sampler;
};

struct AttributeHandles {
    GLint position;
    GLint normal;
    GLint diffuseMaterial;
    GLint textureCoord;
};

class RenderingEngine : public IRenderingEngine {
public:
    RenderingEngine(IResourceManager* resourceManager);
    ~RenderingEngine();
    void Initialize(int width, int height);
    void setCamera(ICamera *camera);
    void addObject(IObject *obj);
    void removeObject(IObject *obj);
    void render(list<IObject *> &objects3d, list<IObject *> &objects2d);
private:
    GLuint buildShader(string *source, GLenum shaderType) const;
    GLuint buildProgram(string *vShader, string *fShader) const;
    void loadMesh(IMesh *newMesh);
    //void unLoadMesh(IMesh *rmvMesh);
    list<MeshRef>::iterator findMeshRef(MeshRef mesh);
    list<TextureRef>::iterator findTextureRef(TextureRef texture);
    void setPngTexture(string name) const;
    IResourceManager *m_resourceManager;
    UniformHandles m_uniforms;
    AttributeHandles m_attributes;
    ICamera *m_camera;
    list<MeshRef> m_meshList;
    list<TextureRef> m_textureList;
    GLuint m_colorRenderbuffer;
    GLuint m_depthRenderbuffer;
    ivec2 m_mainScreenSize;
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
}

RenderingEngine::~RenderingEngine() {
    
}

void RenderingEngine::Initialize(int width, int height)
{
    m_mainScreenSize = ivec2(width, height);
    
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
    GLuint program = buildProgram(vertexShader , fragmentShader);
    m_resourceManager->UnloadShaders();
    glUseProgram(program);
    
    // Extract the handles to attributes and uniforms.
    m_attributes.position = glGetAttribLocation(program, "Position");
    m_attributes.normal = glGetAttribLocation(program, "Normal");
    m_attributes.diffuseMaterial = glGetAttribLocation(program, "DiffuseMaterial");
    m_attributes.textureCoord = glGetAttribLocation(program, "TextureCoord");
    m_uniforms.projection = glGetUniformLocation(program, "Projection");
    m_uniforms.modelview = glGetUniformLocation(program, "Modelview");
    m_uniforms.normalMatrix = glGetUniformLocation(program, "NormalMatrix");
    m_uniforms.textureMatrix = glGetUniformLocation(program, "TextureMatrix");
    m_uniforms.lightPosition = glGetUniformLocation(program, "LightPosition");
    m_uniforms.ambientMaterial = glGetUniformLocation(program, "AmbientMaterial");
    m_uniforms.specularMaterial = glGetUniformLocation(program, "SpecularMaterial");
    m_uniforms.shininess = glGetUniformLocation(program, "Shininess"); 
    m_uniforms.sampler = glGetUniformLocation(program, "Sampler");
    
    // Initialize various states.
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnableVertexAttribArray(m_attributes.position);
    glEnableVertexAttribArray(m_attributes.normal);
    glEnableVertexAttribArray(m_attributes.textureCoord);
    
    // Set up some default material parameters.
    glUniform3f(m_uniforms.ambientMaterial, 0.4f, 0.4f, 0.4f);
    glUniform3f(m_uniforms.specularMaterial, 0.5, 0.5, 0.5);
    glUniform1f(m_uniforms.shininess, 50);
    
    // set a texture
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(m_uniforms.sampler, 0);
}

void RenderingEngine::setCamera(ICamera *camera) {
    m_camera = camera;
}

/*
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
*/
    
void RenderingEngine::render(list<IObject *> &objects3d, list<IObject *> &objects2d)
{
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glViewport(0, 0, m_mainScreenSize.x, m_mainScreenSize.y);
    
    // Set the projection matrix
    float h = 4.0f * m_mainScreenSize.x / m_mainScreenSize.y;
    
    mat4 projectionMatrix = mat4::Frustum(-h / 2, h / 2, -2, 2, 4, 200);
    projectionMatrix = mat4::LookAt(projectionMatrix, m_camera->eye, m_camera->ref, m_camera->up);
    glUniformMatrix4fv(m_uniforms.projection, 1, 0, projectionMatrix.Pointer());

    // Set the light position.
    vec4 lightPosition(1, 1, 1, 0);
    glUniform3fv(m_uniforms.lightPosition, 1, lightPosition.Pointer());
    
    list<IObject *>::iterator obj;
    list<IMesh *> *objMeshes;
    list<IMesh *>::iterator mesh;
    
    glEnable(GL_DEPTH_TEST);
    
    for (obj = objects3d.begin(); obj != objects3d.end(); ++obj) {
        objMeshes = (*obj)->getMeshes();
        for (mesh = objMeshes->begin(); mesh != objMeshes->end(); ++mesh) {
            if ((*mesh)->display) {
                // Set the model view matrix
                mat4 modelview = (*mesh)->meshMtx;
                glUniformMatrix4fv(m_uniforms.modelview, 1, 0, modelview.Pointer());
                
                // Set the normal matrix
                mat3 normalMatrix = modelview.ToMat3();
                glUniformMatrix3fv(m_uniforms.normalMatrix, 1, 0, normalMatrix.Pointer());
                
                // Set the texture matrix
                mat4 texturematrix = (*mesh)->textureMtx;
                glUniformMatrix4fv(m_uniforms.textureMatrix, 1, 0, texturematrix.Pointer());
                
                // Set the diffuse color.
                vec3 color = vec3(0.8, 0.8, 0.8);
                glVertexAttrib4f(m_attributes.diffuseMaterial, color.x, color.y, color.z, 1);
                
                // Draw the surface.
                MeshRef meshRef = (*mesh)->meshRef;
                TextureRef textureRef = (*mesh)->textureRef;
                int stride = 11 * sizeof(GLfloat);
                const GLvoid* normalOffset = (const GLvoid*) (3 * sizeof(GLfloat));
                const GLvoid* texCoordOffset = (const GLvoid*) (3 * sizeof(vec3));
                GLint position = m_attributes.position;
                GLint normal = m_attributes.normal;
                GLint texCoord = m_attributes.textureCoord;
                
                glBindTexture(GL_TEXTURE_2D, textureRef.textureBuffer);
                glBindBuffer(GL_ARRAY_BUFFER, meshRef.vertexBuffer);
                glVertexAttribPointer(position, 3, GL_FLOAT, GL_FALSE, stride, 0);
                glVertexAttribPointer(normal, 3, GL_FLOAT, GL_FALSE, stride, normalOffset);
                glVertexAttribPointer(texCoord, 2, GL_FLOAT, GL_FALSE, stride, texCoordOffset);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshRef.indexBuffer);
                glDrawElements(GL_TRIANGLES, meshRef.indexCount, GL_UNSIGNED_SHORT, 0);
            }
        }
    }
    
    glDisable(GL_DEPTH_TEST);
    
    projectionMatrix = mat4::Parallel(-m_mainScreenSize.x/2, m_mainScreenSize.x/2, -m_mainScreenSize.y/2, m_mainScreenSize.y/2, 0, 10);
    glUniformMatrix4fv(m_uniforms.projection, 1, 0, projectionMatrix.Pointer());
    
    glEnable(GL_BLEND);
    
    for (obj = objects2d.begin(); obj != objects2d.end(); ++obj) {
        objMeshes = (*obj)->getMeshes();
        for (mesh = objMeshes->begin(); mesh != objMeshes->end(); ++mesh) {
            if ((*mesh)->display) {
                // Set the model view matrix
                mat4 modelview = (*mesh)->meshMtx;
                glUniformMatrix4fv(m_uniforms.modelview, 1, 0, modelview.Pointer());
                
                // Set the normal matrix
                mat3 normalMatrix = modelview.ToMat3();
                glUniformMatrix3fv(m_uniforms.normalMatrix, 1, 0, normalMatrix.Pointer());
                
                // Set the texture matrix
                mat4 texturematrix = (*mesh)->textureMtx;
                glUniformMatrix4fv(m_uniforms.textureMatrix, 1, 0, texturematrix.Pointer());
                
                // Set the diffuse color.
                vec3 color = vec3(0.8, 0.8, 0.8);
                glVertexAttrib4f(m_attributes.diffuseMaterial, color.x, color.y, color.z, 1);
                
                // Draw the surface.
                MeshRef meshRef = (*mesh)->meshRef;
                TextureRef textureRef = (*mesh)->textureRef;
                int stride = 11 * sizeof(GLfloat);
                const GLvoid* normalOffset = (const GLvoid*) (3 * sizeof(GLfloat));
                const GLvoid* texCoordOffset = (const GLvoid*) (3 * sizeof(vec3));
                GLint position = m_attributes.position;
                GLint normal = m_attributes.normal;
                GLint texCoord = m_attributes.textureCoord;
                
                glBindTexture(GL_TEXTURE_2D, textureRef.textureBuffer);
                glBindBuffer(GL_ARRAY_BUFFER, meshRef.vertexBuffer);
                glVertexAttribPointer(position, 3, GL_FLOAT, GL_FALSE, stride, 0);
                glVertexAttribPointer(normal, 3, GL_FLOAT, GL_FALSE, stride, normalOffset);
                glVertexAttribPointer(texCoord, 2, GL_FLOAT, GL_FALSE, stride, texCoordOffset);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshRef.indexBuffer);
                glDrawElements(GL_TRIANGLES, meshRef.indexCount, GL_UNSIGNED_SHORT, 0);
            }
        }
    }
    
    glDisable(GL_BLEND);
    
    /*
    // Replace this
    int width, height;
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
    
    // Set the main viewport.
    glViewport(0, 0, width, height);*/
    
    /*
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
    }*/
}

GLuint RenderingEngine::buildShader(string* source, GLenum shaderType) const
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

GLuint RenderingEngine::buildProgram(string* vertexShaderSource, string* fragmentShaderSource) const
{
    GLuint vertexShader = buildShader(vertexShaderSource, GL_VERTEX_SHADER);
    GLuint fragmentShader = buildShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
    
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
    

void RenderingEngine::addObject(IObject *obj) {
    // get meshes from the new object
    list<IMesh *> *objMeshes = obj->getMeshes();
    list<IMesh *>::iterator mesh;
    for (mesh = objMeshes->begin(); mesh != objMeshes->end(); ++mesh) {
        loadMesh(*mesh);
    }
}

void RenderingEngine::removeObject(IObject *obj) {
    
}

void RenderingEngine::loadMesh(IMesh *newMesh) {
    list<MeshRef>::iterator meshRef = findMeshRef(newMesh->meshRef);
    //list<MeshRef>::iterator meshRef = m_meshList.begin();
    //find(meshRef, m_meshList.end(), newMesh->meshRef);
    if (meshRef != m_meshList.end()) {
        newMesh->meshRef = *meshRef;
        meshRef->count += 1;
    }
    else {
        // Check if the mesh is from file or procedural.
        MeshData *meshData;
        meshData = newMesh->getMeshData();
        if (meshData == NULL) {
            meshData = m_resourceManager->readMeshData(newMesh->getMeshName(), newMesh->normalType, newMesh->size);
            if (meshData == NULL) {
                std::cout << "Could not read file: " << newMesh->getMeshName() << "\n";
                return;
            }
        }
        
        // Add the mesh VBO
        GLuint vertexBuffer;
        glGenBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, meshData->vertexCount * VERTEX_STRIDE * sizeof(GLfloat),
                     meshData->vertices, GL_STATIC_DRAW);
        
        GLuint indexBuffer;
        glGenBuffers(1, &indexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshData->indexCount * sizeof(GLushort),
                     meshData->indices, GL_STATIC_DRAW);
        
        // Setup a new mesh reference for the render engine
        MeshRef newMeshRef(newMesh->getMeshName(), vertexBuffer, indexBuffer, meshData->indexCount);
        newMesh->meshRef = newMeshRef;
        m_meshList.push_back(newMeshRef);
        
        // Clean up the mesh data
        delete[] meshData->vertices;
        delete[] meshData->indices;
        delete meshData;
    }
    
    list<TextureRef>::iterator textureRef = findTextureRef(newMesh->textureRef);
    //list<TextureRef>::iterator textureRef = m_textureList.begin();
    //std::find(textureRef, m_textureList.end(), newMesh->textureRef);
    if (textureRef != m_textureList.end()) {
        newMesh->textureRef = *textureRef;
        textureRef->count += 1;
    }
    
    else {
        // Open each texture
        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        setPngTexture(newMesh->getTextureName());
        glGenerateMipmap(GL_TEXTURE_2D);
        
        // Setup a new texture reference
        TextureRef newTextrueRef(newMesh->getTextureName(), textureID);
        newMesh->textureRef = newTextrueRef;
        m_textureList.push_back(newTextrueRef);
    }
}

/*
void RenderingEngine::unLoadMesh(IMesh* rmvMesh) {
    list<MeshRef>::iterator meshRef = findMeshRef(rmvMesh->meshRef);
    //list<MeshRef>::iterator meshRef = m_meshList.begin();
    //std::find(meshRef, m_meshList.end(), rmvMesh->meshRef);
    if (meshRef != m_meshList.end()) {
        meshRef->count -= 1;
        if (meshRef->count < 1) {
            // remove the VBOs
            glDeleteBuffers(1, &meshRef->vertexBuffer);
            glDeleteBuffers(1, &meshRef->indexBuffer);
            m_meshList.erase(meshRef);
        }
    }
    rmvMesh->meshRef = MeshRef();
    
    list<TextureRef>::iterator textureRef = findTextureRef(rmvMesh->textureRef);
    //list<TextureRef>::iterator textureRef = m_textureList.begin();
    //std::find(textureRef, m_textureList.end(), rmvMesh->textureRef);
    if (textureRef != m_textureList.end()) {
        textureRef->count -= 1;
        if (textureRef->count < 1) {
            // remove the texture
            glDeleteTextures(1, &textureRef->textureBuffer);
            m_textureList.erase(textureRef);
        }
    }
    rmvMesh->textureRef = TextureRef();
}*/

list<MeshRef>::iterator RenderingEngine::findMeshRef(MeshRef mesh) {
    list<MeshRef>::iterator curMesh = m_meshList.begin();
    while (curMesh != m_meshList.end() && !(*curMesh == mesh))
        ++curMesh;
    return curMesh;
}

list<TextureRef>::iterator RenderingEngine::findTextureRef(TextureRef texture) {
    list<TextureRef>::iterator curTexture = m_textureList.begin();
    while (curTexture != m_textureList.end() && !(*curTexture == texture))
        ++curTexture;
    return curTexture;
}

void RenderingEngine::setPngTexture(string name) const {
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
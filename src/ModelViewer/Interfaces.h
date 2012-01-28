#pragma once
#include "Vector.hpp"
#include "Quaternion.hpp"
#include <vector>
#include <list>
#include <string>

using std::vector;
using std::list;
using std::string;

enum DeviceType {
    DeviceTypeIphone,
    DeviceTypeIpod,
    DeviceTypeIpad,
    DeviecTypeUnknown,
};

enum LOAD_NORMAL_TYPE {
    LOAD_NORMAL_VERTEX,
    LOAD_NORMAL_FACE,
};

// Data for creating VBOs
struct MeshData {
    int vertexCount;
    float* vertices;
    int indexCount;
    unsigned short* indices;
    vec3 mean, min, max;
};

// RenderingEngine uses this for keeping track of VBOs
class MeshRef {
public:
    MeshRef() {
        name = "";
        vertexBuffer = 0;
        indexBuffer = 0;
        indexCount = 0;
        count = 0;
    }
    MeshRef(string n, unsigned int vb, unsigned int ib, int idxCount) {
        name = n;
        vertexBuffer = vb;
        indexBuffer = ib;
        indexCount = idxCount;
        count = 1;
    }
    MeshRef(const MeshRef &other) {
        name = other.name;
        vertexBuffer = other.vertexBuffer;
        indexBuffer = other.indexBuffer;
        indexCount = other.indexCount;
        count = other.count;
    }
    int operator==(const MeshRef &rhs) {
        if (name == rhs.name) return 1;
        return 0;
    }
    string name;
    unsigned int vertexBuffer;
    unsigned int indexBuffer;
    int indexCount;
    int count;
};

class TextureRef {
public:
    TextureRef() {
        name = "";
        count = 0;
    }
    TextureRef(string n, unsigned int tb) {
        name = n;
        textureBuffer = tb;
        count = 1;
    }
    TextureRef(const TextureRef &other) {
        name = other.name;
        textureBuffer = other.textureBuffer;
        count = other.count;
    }
    int operator==(const TextureRef &rhs) {
        if (name == rhs.name) return 1;
        return 0;
    }
    string name;
    unsigned int textureBuffer;
    int count;
};

struct ImageData {
    void* pixels;
    ivec2 size;
};

struct IMesh {
    MeshRef meshRef;       // References used by render engine
    TextureRef textureRef;
    mat4 meshMtx;          // Matrix used by render engine
    mat4 textureMtx;
    vec3 mean, min, max;   // Bounding box.
    vec4 color;
    float size;            // the size to load the mesh as.
    LOAD_NORMAL_TYPE normalType;
    
    // File name and Mesh type
    // Name used as a key by Rendering Engine for finding duplicate meshes.
    virtual string getMeshName() = 0;
    virtual string getTextureName() = 0;
    
    // Used for procedural meshes. Meshes on file should be 
    virtual MeshData* getMeshData() = 0;
    virtual ImageData* getImageData() = 0;
    virtual ~IMesh() {}
};

struct IObject {
    virtual list<IMesh *>* getMeshes() = 0;
};

struct ICamera {
    vec3 fwd; // direction vector of the camera.
    vec3 eye; // eye location of the camera.
    vec3 ref; // reference point the camera looks at.
    vec3 up;  // up vector for the camera (controls roll.
};

struct IApplicationEngine {
    //Initalize the game (update to signal using ipad or iphone)
    virtual void Initialize(int width, int height) = 0;
    virtual string* GetResourcePath() = 0;
    
    //Render the current frame
    virtual void Render() = 0;
    
    //Handle user input
    virtual void OnFingerUp(vec2 location) = 0;
    virtual void OnFingerDown(vec2 location) = 0;
    virtual void OnFingerMove(vector<vec2> touches) = 0;
    
    //Update animations
    virtual void UpdateAnimations(float dt) = 0;
    
    //Signal the game for application actions
    virtual void AppWillResignActive() = 0;
    virtual void AppWillBecomeActive() = 0;
    virtual void AppWillEnterBackground() = 0;
    virtual void AppWillEnterForeground() = 0;
    virtual void AppWillTerminate() = 0;
    
    //Information for views
    virtual ivec2* GetScreenSize() = 0;
    virtual ~IApplicationEngine() {}
};

struct IRenderingEngine {
    virtual void Initialize(int width, int height) = 0;
    virtual void setCamera(ICamera *camera) = 0;
    virtual void addObject(IObject *obj) = 0;      // Before any object can be renderd it needs to be added by this method.
    virtual void removeObject(IObject *obj) = 0;   // Removes the object from the rendering engine.
    virtual void render(list<IObject *> &objects) = 0;
    virtual ~IRenderingEngine() {}
};

enum TextureFormat {
    TextureFormatGray,
    TextureFormatGrayAlpha,
    TextureFormatRgb,
    TextureFormatRgba,
};

struct TextureDescription {
    TextureFormat Format;
    int BitsPerComponent;
    ivec2 size;
};

struct IResourceManager {
    virtual string* GetResourcePath() = 0;
    virtual TextureDescription LoadPngImage(string filename) = 0;
    virtual MeshData* readMeshData(string fileName, LOAD_NORMAL_TYPE normalType, float scale) = 0;
    virtual void* GetImageData() = 0;
    virtual void UnloadImage() = 0;
    virtual string* GetVertexShader(string vShaderFile) = 0;
    virtual string* GetFragmentShader(string fShaderFile) = 0;
    virtual void UnloadShaders() = 0;
    virtual ~IResourceManager() {}
};

IResourceManager* CreateResourceManager();

IApplicationEngine* CreateApplicationEngine(DeviceType deviceType, IRenderingEngine* renderingEngine, IResourceManager* resourceManager);

IRenderingEngine* CreateRenderingEngine(IResourceManager* resourceManager);


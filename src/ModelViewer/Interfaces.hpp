#pragma once
#include "Vector.hpp"
#include "Quaternion.hpp"
#include <vector>
#include <string.h>

using std::vector;
using std::string;

enum BlendMode {
    BlendModeNone,
    BlendModeDepth,
    BlendModeDepthLights,
    BlendModeBlend,
};

enum DeviceType {
    DeviceTypeIphone,
    DeviceTypeIpod,
    DeviceTypeIpad,
    DeviecTypeUnknown,
};

enum StateType {
    StateTypeStatic,
    StateTypeMoving,
    StateTypeGame,
};

struct IMesh {
    virtual int GetVertexCount() const = 0;
    virtual int GetTriangleIndexCount() const = 0;
    virtual void GenerateVertices(vector<float>& vertices) const = 0;
    virtual void GenerateTriangleIndices(vector<unsigned short>& indices) const = 0;
    virtual ~IMesh() {}
};

struct DrawList {
    //mat4 Projection;
    //mat4 Translation;
    vector<IMesh*> Meshes;
    vector<string> Textures;
    
    // Find something better
    bool Plane;
    int PlaneIndex;
    bool CharMap;
    int CharMapIndex;
    bool Button;
    int ButtonIndex;
    
    int group;
    int id;
};

struct Visual {
    BlendMode Mode;
    mat4 Translation;
    mat4 TextureMatrix;
    int Mesh;
    int Texture;
    int Group;
    
    //Not used by render engine
    vec3 Location;
    Quaternion Orientation;
    float Scale;
};

struct IMap {
    string* ResourcePath;
    string* Map;
    vector<string>* Meshes;
    vector<string>* Textures;
    vector<Visual>* Visuals;
    Visual BallVisual;
    vec3 ViewLoc;
};

struct IView {
    virtual void GetVisuals(vector<Visual>* visuals) = 0;
    virtual void GetDrawList(DrawList* drawList) = 0;
    virtual void AddSubview(IView* subview) = 0;
    virtual void ClearSubviews() = 0;
    virtual mat4 GetProjection() = 0;
    virtual mat4 GetTranslation() = 0;
};

struct IController {
    virtual void OnFingerUp(vec2 location) = 0;
    virtual void OnFingerDown(vec2 location) = 0;
    virtual void OnFingerMove(vector<vec2> touches) = 0;
    virtual void Tic(float td) = 0;
};

struct IState {
    virtual StateType GetType() = 0;
    virtual IView* GetBackGroundView() = 0;
    virtual IView* GetForeGroundView() = 0;
    virtual void UpdateAnimations(float td) = 0;
};

struct IMainView {
    virtual void GetVisuals(vector<Visual>* visuals) = 0;
    virtual void GetLists(vector<DrawList*>* list) = 0;
    virtual void SetStateViews(IState* state) = 0;
    virtual mat4 GetProjection(int group) = 0;
    virtual mat4 GetTranslation(int group) = 0;
};

struct IApplicationEngine {
    //Initalize the game (update to signal using ipad or iphone)
    virtual void Initialize(int width, int height) = 0;
    
    //Use to switch controllers
    virtual string* GetResourcePath() = 0;
    virtual void SetController(IController *controller) = 0;
    virtual void SetState(IState* state) = 0;
    virtual IState* GetState() = 0;
    
    //Render the current frame
    virtual void Render() const = 0;
    
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
    
    //Destructor
    virtual ~IApplicationEngine() {}
};

struct IRenderingEngine {
    virtual void Initialize(IMainView* view, int width, int height) = 0;
    virtual void ReLoadLists() = 0;
    virtual void Render() = 0;
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
    virtual void* GetImageData() = 0;
    virtual void UnloadImage() = 0;
    virtual string* GetVertexShader(string vShaderFile) = 0;
    virtual string* GetFragmentShader(string fShaderFile) = 0;
    virtual void UnloadShaders() = 0;
    virtual ~IResourceManager() {}
};

IResourceManager* CreateResourceManager();

IApplicationEngine* CreateApplicationEngine(DeviceType deviceType, IRenderingEngine* renderingEngine, IResourceManager* resourceManager);

namespace ES1 { IRenderingEngine* CreateRenderingEngine(IResourceManager* resourceManager); }
namespace ES2 { IRenderingEngine* CreateRenderingEngine(IResourceManager* resourceManager); }
#ifndef MD5MODEL_H_NEW
#define MD5MODEL_H_NEW

#include "MD5Animation.h"
#include "../glIncludes.h"
#include "../Matrix.h"

class MD5Model;

class MD5ModelLoader
{
    struct Joint
    {
        std::string name;
        int         parentID;
        glm::vec3   pos;
        glm::quat   orient;
    };
    typedef std::vector<Joint> JointList;

    struct Vertex
    {
        glm::vec3  pos;
        glm::vec3  normal;
        glm::vec3  tangent;
        glm::vec3  binormal;
        glm::vec2  texCoord0;
        glm::vec4  boneWeights;
        glm::ivec4 boneIndices;

        // DEPRICATED
        int        startWeight;
        int        weightCount;
    };
    typedef std::vector<Vertex> VertexList;

    struct SimpleVertex
    {
        float pos[3];
        float normal[3];
        float tangent[3];
        float binormal[3];
        float texCoord0[2];

        int   padding[2];
    };
    typedef std::vector<SimpleVertex> VertexBuffer;

    struct SimpleAnimatedVertex
    {
        float pos[3];
        float normal[3];
        float tangent[3];
        float binormal[3];
        float texCoord0[2];
        float boneWeights[4];
        int   boneIndices[4];

        int   padding[2];
    };
    typedef std::vector<SimpleAnimatedVertex> AnimatedVertexBuffer;

    struct Triangle
    {
        int indices[3];
    };
    typedef std::vector<Triangle> TriangleList;

    struct Weight
    {
        int       jointID;
        float     bias;
        glm::vec3 pos;
    };
    typedef std::vector<Weight> WeightList;

    typedef std::vector<GLuint> IndexBuffer;

public:
    struct Mesh
    {
        Mesh() : vertexArrayObject(0), vertexBufferObject(0),  indexBufferObject(0) { }

        // Vertex buffer Object IDs for the vertex streams
        GLuint vertexArrayObject;
        GLuint vertexBufferObject;
        GLuint indexBufferObject;

        // A texture ID for the material
        GLuint diffuseTexID;
        GLuint normalTexID;

        int numIndexs;

        // This vertex list stores the vertices's in the bind pose.
        VertexList   verts;
        TriangleList tris;
        WeightList   weights;

        // These buffers are used for rendering the animated mesh
        VertexBuffer         vertexBuffer;
        AnimatedVertexBuffer animatedVertexBuffer;
        IndexBuffer          indexBuffer;
    };
    typedef std::vector<Mesh> MeshList;

    typedef std::vector<glm::mat4x4> MatrixList;
    typedef std::vector<MD5AnimationLoader *> AnimationList;

private:
    int MD5Version;
    int numJoints;
    int numMeshes;

    JointList joints;
    MeshList  meshes;

    bool          hasAnimation;
    AnimationList animations;

    MatrixList inverseBindPose;

    void loadModel(const std::string& filename);

    // Build the bind-pose and the inverse bind-pose matrix array for the model.
    void buildBindPose( const JointList& joints );

    // Prepare the mesh for rendering
    // Compute vertex positions and normals
    void prepareMesh(Mesh& mesh);
    void prepareNormals(Mesh& mesh);

    // Create the VBOs that are used to render the mesh with shaders
    void prepareVertexBuffer(Mesh& mesh);
    void createVertexBuffers(Mesh& mesh);

    void destroyMesh(Mesh& mesh);

public:
    MD5ModelLoader(const std::string &filename, float _scale, vec3 _forward,
                   bool hasAnimation = false, const std::string &animationFilename = "");
    ~MD5ModelLoader();

    MD5Model *getModel() const;

    const glm::mat4& getInverseBindPose(int pose) const;
    const MD5ModelLoader::Mesh& getMesh(int mesh) const;

    float scale;
    vec3  forward;
};

class MD5Model
{
private:
    typedef std::vector<glm::mat4x4> MatrixList;
    typedef std::vector<MD5Animation *> AnimationList;

    const MD5ModelLoader *baseModel;

    int numJoints;
    int numMeshes;

    float explode;

    bool          hasAnimation;
    AnimationList animations;

    int currentAnimation;

    MatrixList animatedBones;

    void renderMesh(const MD5ModelLoader::Mesh& mesh);

public:
    explicit MD5Model(const MD5ModelLoader *_baseModel, int _numJoints, int _numMeshes);
    virtual ~MD5Model();

    void update(float fDeltaTime);
    void render(GLuint shader);

    bool isAnimated();

    void setCurrentAnimation(int anim);
    void setAnimationDirection(bool reverse, bool restart = false);

    void addAnimation(MD5Animation *animation);
};
#endif /* MD5MODEL_H_ */

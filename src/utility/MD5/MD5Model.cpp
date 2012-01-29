#include "MD5Model.h"
#include "Helpers.h"
#include "../glIncludes.h"
#include "../glHelpers.h"
#include "../../include/glm/gtc/type_ptr.hpp"
#include "../Shader.h"
#include "../../Resources/Manager.h"
#include <stddef.h>
#include "../../Logging.h"

#define BUFFER_OFFSET(i) ((char*)NULL + (i))

////////////////////////////////////////////////////////////////////////////////
// Model Loader Class
////////////////////////////////////////////////////////////////////////////////

MD5ModelLoader::MD5ModelLoader(const std::string &filename, float _scale, vec3 _forward,
                               bool _hasAnimation, const std::string &animationFilename) :
MD5Version(-1),
numJoints(-1),
numMeshes(-1),
joints(),
meshes(),
hasAnimation(_hasAnimation),
animations(),
inverseBindPose(),
scale(_scale),
forward(_forward)
{
    loadModel(filename);

    if (hasAnimation)
    {
        for (int i = 0; i < 1; ++i)
        {
            animations.push_back(new MD5AnimationLoader(animationFilename));
        }
    }

    joints.clear();
}

void MD5ModelLoader::loadModel(const std::string &filename)
{
    std::string param;
    std::string junk;   // Read junk from the file

    std::ifstream file(filename.c_str());

    int fileLength = getFileLength(file);
    assert(fileLength > 0);

    joints.clear();
    meshes.clear();
    
    file >> param;

    while (!file.eof())
    {
        if (param == "MD5Version")
        {
            file >> MD5Version;
            assert(MD5Version == 10);
        }
        else if (param == "commandline")
        {
            ignoreLine(file, fileLength);
        }
        else if (param == "numJoints")
        {
            file >> numJoints;
            joints.reserve(numJoints);
        }
        else if (param == "numMeshes")
        {
            file >> numMeshes;
            meshes.reserve(numMeshes);
        }
        else if (param == "joints")
        {
            Joint joint;
            file >> junk; // Read the '{' character
            for (int i = 0; i < numJoints; ++i)
            {
                file >> joint.name >> joint.parentID >> junk
                     >> joint.pos.x >> joint.pos.y >> joint.pos.z >> junk >> junk
                     >> joint.orient.x >> joint.orient.y >> joint.orient.z >> junk;
                
                removeQuotes(joint.name);
                computeQuatW(joint.orient);

                joints.push_back(joint);
                ignoreLine(file, fileLength);
            }
            file >> junk; // Read the '}' character

            buildBindPose(joints);
        }
        else if (param == "mesh")
        {
            Mesh mesh;
            int numVerts, numTris, numWeights;

            file >> junk; // Read the '{' character
            file >> param;
            while (param != "}")  // Read until we get to the '}' character
            {
                if (param == "shader")
                {
                    file.ignore(fileLength, '\n'); // Ignore everything else on the line
                }
                else if (param == "numverts")
                {
                    file >> numVerts;               // Read in the vertices
                    ignoreLine(file, fileLength);

                    for (int i = 0; i < numVerts; ++i)
                    {
                        Vertex vert;

                        file >> junk >> junk >> junk                    // vert vertIndex (
                            >> vert.texCoord0.x >> vert.texCoord0.y >> junk  //  s t )
                            >> vert.startWeight >> vert.weightCount;

                        ignoreLine(file, fileLength);

                        vert.texCoord0.y = 1.0f - vert.texCoord0.y; //TODO fix this in the file not here.
                        mesh.verts.push_back(vert);
                    }  
                }
                else if (param == "numtris")
                {
                    file >> numTris;
                    ignoreLine(file, fileLength);

                    for (int i = 0; i < numTris; ++i)
                    {
                        Triangle tri;
                        //TODO fix clockwise/counterclockwise
                        //file >> junk >> junk >> tri.indices[0] >> tri.indices[1] >> tri.indices[2];
                        file >> junk >> junk >> tri.indices[0] >> tri.indices[2] >> tri.indices[1];

                        ignoreLine(file, fileLength);

                        mesh.tris.push_back(tri);
                        mesh.indexBuffer.push_back((GLuint) tri.indices[0]);
                        mesh.indexBuffer.push_back((GLuint) tri.indices[1]);
                        mesh.indexBuffer.push_back((GLuint) tri.indices[2]);
                    }
                }
                else if (param == "numweights")
                {
                    file >> numWeights;
                    ignoreLine(file, fileLength);

                    for (int i = 0; i < numWeights; ++i)
                    {
                        Weight weight;
                        file >> junk >> junk >> weight.jointID >> weight.bias >> junk
                            >> weight.pos.x >> weight.pos.y >> weight.pos.z >> junk;

                        ignoreLine(file, fileLength);
                        mesh.weights.push_back(weight);
                    }
                }
                else
                {
                    ignoreLine(file, fileLength);
                }
            
                file >> param;
            }

            prepareMesh(mesh);
            prepareNormals(mesh);
            prepareVertexBuffer(mesh);
            createVertexBuffers(mesh);

            mesh.verts.clear();
            mesh.tris.clear();
            mesh.weights.clear();
            mesh.vertexBuffer.clear();
            mesh.animatedVertexBuffer.clear();
            mesh.indexBuffer.clear();

            meshes.push_back(mesh);
        }

        file >> param;
    }

    assert((int) joints.size() == numJoints);
    assert((int) meshes.size() == numMeshes);
}

void MD5ModelLoader::buildBindPose(const JointList& joints)
{
    inverseBindPose.clear();

    JointList::const_iterator iter = joints.begin();
    while (iter != joints.end())
    {
        const Joint& joint = (*iter);
        glm::mat4x4 boneTranslation = glm::translate(joint.pos);
        glm::mat4x4 boneRotation = glm::toMat4(joint.orient);

        glm::mat4x4 boneMatrix = boneTranslation * boneRotation;
        glm::mat4x4 inverseBoneMatrix = glm::inverse(boneMatrix);

        inverseBindPose.push_back(inverseBoneMatrix);

        ++iter;
    }
}

// Compute the position of the vertices in object local space
// in the skeleton's bind pose
void MD5ModelLoader::prepareMesh(Mesh& mesh)
{
    // Compute vertex positions
    for (unsigned int i = 0; i < mesh.verts.size(); ++i)
    {
        glm::vec3 finalPos(0);
        Vertex& vert = mesh.verts[i];

        vert.pos = glm::vec3(0);
        vert.normal = glm::vec3(0);
        vert.tangent = glm::vec3(0);
        vert.binormal = glm::vec3(0);
        vert.boneWeights = glm::vec4(0);
        vert.boneIndices = glm::ivec4(0);

        // Sum the position of the weights
        for (int j = 0; j < vert.weightCount; ++j)
        {
            assert(j < 4);

            Weight& weight = mesh.weights[vert.startWeight + j];
            Joint& joint = joints[weight.jointID];
            
            // Convert the weight position from Joint local space to object space
            glm::vec3 rotPos = joint.orient * weight.pos;

            vert.pos += (joint.pos + rotPos) * weight.bias;
            vert.boneIndices[j] = weight.jointID;
            vert.boneWeights[j] = weight.bias;
        }
    }
}

// Compute the vertex normals in the Mesh's bind pose
void MD5ModelLoader::prepareNormals(Mesh& mesh)
{
    // Loop through all triangles and calculate the normal of each triangle
    for (unsigned int i = 0; i < mesh.tris.size(); ++i)
    {
        Vertex& vertex1 = mesh.verts[mesh.tris[i].indices[0]];
        Vertex& vertex2 = mesh.verts[mesh.tris[i].indices[1]];
        Vertex& vertex3 = mesh.verts[mesh.tris[i].indices[2]];

        glm::vec3 v1 = vertex1.pos;
        glm::vec3 v2 = vertex2.pos;
        glm::vec3 v3 = vertex3.pos;

        glm::vec2 c1 = vertex1.texCoord0;
        glm::vec2 c2 = vertex2.texCoord0;
        glm::vec2 c3 = vertex3.texCoord0;

        glm::vec3 v2v1 = glm::normalize(v2 - v1);
        glm::vec3 v3v1 = glm::normalize(v3 - v1);

        glm::vec2 c2c1 = glm::normalize(c2 - c1);
        glm::vec2 c3c1 = glm::normalize(c3 - c1);

        float determinant = (c2c1.x * c3c1.y) - (c2c1.y * c3c1.x);

        glm::vec3 tangent  = glm::vec3(1.0, 0.0, 0.0);
        glm::vec3 binormal = glm::vec3(0.0, 1.0, 0.0);
        glm::vec3 normal   = glm::vec3(0.0, 0.0, 1.0);

        if (fneq(determinant, 0.0))
        {
            determinant = 1.0 / determinant;

            tangent  = determinant * ((c3c1.y * v3v1) - (c2c1.y * v2v1));
            binormal = determinant * ((c3c1.x * v3v1) - (c2c1.x * v2v1));

            normal = glm::cross(tangent, binormal);
            //TODO maybe need to deal with handedness
            //if (fless(determinant,  0)) normal = normal * -1.0;
        }

        normal = glm::normalize(normal);
        tangent = glm::normalize(tangent);
        binormal = glm::normalize(binormal);

        vertex1.normal += normal;
        vertex2.normal += normal;
        vertex3.normal += normal;

        vertex1.tangent += tangent;
        vertex2.tangent += tangent;
        vertex3.tangent += tangent;

        vertex1.binormal += binormal;
        vertex2.binormal += binormal;
        vertex3.binormal += binormal;
    }

    // Now normalize all the normals
    for (unsigned int i = 0; i < mesh.verts.size(); ++i)
    {
        Vertex& vert = mesh.verts[i];

        vert.normal = glm::normalize(vert.normal);
        vert.tangent = glm::normalize(vert.tangent);
        vert.binormal = glm::normalize(vert.binormal);
    }
}

void MD5ModelLoader::prepareVertexBuffer(Mesh& mesh)
{
    mesh.vertexBuffer.clear();
    mesh.animatedVertexBuffer.clear();

    if (hasAnimation)
    {
        for (unsigned int i = 0; i < mesh.verts.size(); ++i)
        {
            Vertex& vert = mesh.verts[i];
            SimpleAnimatedVertex vertex;

            vertex.pos[0] = vert.pos[0];
            vertex.pos[1] = vert.pos[1];
            vertex.pos[2] = vert.pos[2];

            vertex.normal[0] = vert.normal[0];
            vertex.normal[1] = vert.normal[1];
            vertex.normal[2] = vert.normal[2];

            vertex.tangent[0] = vert.tangent[0];
            vertex.tangent[1] = vert.tangent[1];
            vertex.tangent[2] = vert.tangent[2];

            vertex.binormal[0] = vert.binormal[0];
            vertex.binormal[1] = vert.binormal[1];
            vertex.binormal[2] = vert.binormal[2];

            vertex.texCoord0[0] = vert.texCoord0[0];
            vertex.texCoord0[1] = vert.texCoord0[1];

            vertex.boneWeights[0] = vert.boneWeights[0];
            vertex.boneWeights[1] = vert.boneWeights[1];
            vertex.boneWeights[2] = vert.boneWeights[2];
            vertex.boneWeights[3] = vert.boneWeights[3];

            vertex.boneIndices[0] = vert.boneIndices[0];
            vertex.boneIndices[1] = vert.boneIndices[1];
            vertex.boneIndices[2] = vert.boneIndices[2];
            vertex.boneIndices[3] = vert.boneIndices[3];

            mesh.animatedVertexBuffer.push_back(vertex);
        }
    }
    else
    {
        for (unsigned int i = 0; i < mesh.verts.size(); ++i)
        {
            Vertex& vert = mesh.verts[i];
            SimpleVertex vertex;

            vertex.pos[0] = vert.pos[0];
            vertex.pos[1] = vert.pos[1];
            vertex.pos[2] = vert.pos[2];

            vertex.normal[0] = vert.normal[0];
            vertex.normal[1] = vert.normal[1];
            vertex.normal[2] = vert.normal[2];

            vertex.tangent[0] = vert.tangent[0];
            vertex.tangent[1] = vert.tangent[1];
            vertex.tangent[2] = vert.tangent[2];

            vertex.binormal[0] = vert.binormal[0];
            vertex.binormal[1] = vert.binormal[1];
            vertex.binormal[2] = vert.binormal[2];

            vertex.texCoord0[0] = vert.texCoord0[0];
            vertex.texCoord0[1] = vert.texCoord0[1];

            mesh.vertexBuffer.push_back(vertex);
        }
    }
}

void MD5ModelLoader::createVertexBuffers(Mesh& mesh)
{
    // Create and bind VAO
    createVertexArray(mesh.vertexArrayObject);
    glBindVertexArray(mesh.vertexArrayObject);

    // Create and bind VBO for vertex data
    createVertexBuffer(mesh.vertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBufferObject);

    if (hasAnimation)
    {
        // copy data into the buffer object
        glBufferData(GL_ARRAY_BUFFER, mesh.animatedVertexBuffer.size() * sizeof(SimpleAnimatedVertex), &(mesh.animatedVertexBuffer[0]), GL_STATIC_DRAW);

        // set up vertex attributes
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SimpleAnimatedVertex), (void*)offsetof(SimpleAnimatedVertex, pos));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SimpleAnimatedVertex), (void*)offsetof(SimpleAnimatedVertex, normal));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(SimpleAnimatedVertex), (void*)offsetof(SimpleAnimatedVertex, tangent));

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(SimpleAnimatedVertex), (void*)offsetof(SimpleAnimatedVertex, binormal));

        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(SimpleAnimatedVertex), (void*)offsetof(SimpleAnimatedVertex, texCoord0));

        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(SimpleAnimatedVertex), (void*)offsetof(SimpleAnimatedVertex, boneWeights));

        glEnableVertexAttribArray(6);
        //glVertexAttribPointer(6, 4, GL_INT,   GL_FALSE, sizeof(SimpleAnimatedVertex), (void*)offsetof(SimpleAnimatedVertex, m_BoneIndices));
        glVertexAttribPointer(6, 4, GL_FLOAT,   GL_FALSE, sizeof(SimpleAnimatedVertex), (void*)offsetof(SimpleAnimatedVertex, boneIndices));
    }
    else
    {
        // copy data into the buffer object
        glBufferData(GL_ARRAY_BUFFER, mesh.vertexBuffer.size() * sizeof(SimpleVertex), &(mesh.vertexBuffer[0]), GL_STATIC_DRAW);

        // set up vertex attributes
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (void*)offsetof(SimpleVertex, pos));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (void*)offsetof(SimpleVertex, normal));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (void*)offsetof(SimpleVertex, tangent));

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (void*)offsetof(SimpleVertex, binormal));

        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (void*)offsetof(SimpleVertex, texCoord0));
    }

    // Create and bind VBO for index data
    createVertexBuffer(mesh.indexBufferObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBufferObject);

    // copy data into the buffer object
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * mesh.indexBuffer.size(), &(mesh.indexBuffer[0]), GL_STATIC_DRAW);
    mesh.numIndexs = mesh.indexBuffer.size();

    glBindVertexArray(0);
}

MD5ModelLoader::~MD5ModelLoader()
{
    for (unsigned int i = 0; i < meshes.size(); ++i)
    {
        destroyMesh(meshes[i]);
    }

    for (unsigned int i = 0; i < animations.size(); ++i)
    {
        delete animations[i];
    }

    meshes.clear();
    animations.clear();
    inverseBindPose.clear();
}

void MD5ModelLoader::destroyMesh(Mesh& mesh)
{
    // Delete all the VBO's
    deleteVertexArray(mesh.vertexArrayObject);
    deleteVertexBuffer(mesh.vertexBufferObject);
    deleteVertexBuffer(mesh.indexBufferObject);
}

MD5Model *MD5ModelLoader::getModel() const
{
    MD5Model *model = new MD5Model(this, numJoints, numMeshes);

    if (hasAnimation)
    {
        for (unsigned int i = 0; i < animations.size(); ++i)
        {
            model->addAnimation(animations[i]->getAnimation());
        }
    }

    return model;
}

const glm::mat4& MD5ModelLoader::getInverseBindPose(int pose) const
{
    return inverseBindPose[pose];
}

const MD5ModelLoader::Mesh& MD5ModelLoader::getMesh(int mesh) const
{
    return meshes[mesh];
}

////////////////////////////////////////////////////////////////////////////////
// Model Render Class
////////////////////////////////////////////////////////////////////////////////

MD5Model::MD5Model(const MD5ModelLoader *_baseModel, int _numJoints, int _numMeshes) :
baseModel(_baseModel),
numJoints(_numJoints),
numMeshes(_numMeshes),
explode(0.0f),
hasAnimation(false),
animations(),
currentAnimation(0),
animatedBones()
{
    animatedBones.assign(numJoints, glm::mat4x4(1.0f));
}

MD5Model::~MD5Model()
{
    for (unsigned int i = 0; i < animations.size(); ++i)
    {
        delete animations[i];
    }

    animations.clear();
    animatedBones.clear();
}

////////////////////////////////////////////////////////////////////////////////
// Updating
////////////////////////////////////////////////////////////////////////////////

void MD5Model::update(float fDeltaTime)
{
    if (hasAnimation)
    {
        //animations[currentAnimation]->update(fDeltaTime * 0.00001);
        animations[currentAnimation]->update(fDeltaTime);
        
        const MatrixList& animatedSkeleton = animations[currentAnimation]->getSkeletonMatrixList();
        // Multiply the animated skeleton joints by the inverse of the bind pose.
        for (int i = 0; i < numJoints; ++i)
        {
            //animatedBones[i] = animatedSkeleton[i] * inverseBindPose[i];
            animatedBones[i] = animatedSkeleton[i] * (baseModel->getInverseBindPose(i));
        }        
    }
}

////////////////////////////////////////////////////////////////////////////////
// Rendering
////////////////////////////////////////////////////////////////////////////////

void MD5Model::render(GLuint shader)
{
    /*
    explode += 0.005;
    if (explode > 1.0)
    {
        explode = 0.0;
    }
    */

    if (hasAnimation)
    {
        glUniformMatrix4fv(glGetUniformLocation(shader, "boneMatrix"), animatedBones.size(), GL_FALSE, glm::value_ptr(animatedBones[0]));
    }
    //glUniform1f(glGetUniformLocation(shader, "explode_factor"), explode);

    // Render the meshes
    //for (unsigned int i = 0; i < m_Meshes.size(); ++i)
    for (int i = 0; i < numMeshes; ++i)
    {
        renderMesh(baseModel->getMesh(i));
    }
}

void MD5Model::renderMesh(const MD5ModelLoader::Mesh& mesh)
{
    glBindVertexArray(mesh.vertexArrayObject);

    // Draw mesh from index buffer
    glDrawElements(GL_TRIANGLES, mesh.numIndexs, GL_UNSIGNED_INT, BUFFER_OFFSET(0));

    glBindVertexArray(0);

    //glUseProgram(0);
}

////////////////////////////////////////////////////////////////////////////////
// Getters and Setters
////////////////////////////////////////////////////////////////////////////////

bool MD5Model::isAnimated()
{
    return hasAnimation;
}

void MD5Model::setCurrentAnimation(int anim)
{
    currentAnimation = anim;
}

void MD5Model::setAnimationDirection(bool reverse, bool restart)
{

}

void MD5Model::addAnimation(MD5Animation *animation)
{
    animations.push_back(animation);
    hasAnimation = true;
}

#include "MD5Animation.h"
#include "Helpers.h"
#include "../glIncludes.h"
#include "../../include/glm/gtx/compatibility.hpp"
#include<fstream>

////////////////////////////////////////////////////////////////////////////////
// Animation Loader Class
////////////////////////////////////////////////////////////////////////////////

MD5AnimationLoader::MD5AnimationLoader(const std::string& filename) :
MD5Version(0),
numFrames(0),
numJoints(0),
framRate(0),
numAnimatedComponents(0),
jointInfos(),
bounds(),
baseFrames(),
frames(),
skeletons()
{
    loadAnimation(filename);

    jointInfos.clear();
    bounds.clear();
    baseFrames.clear();
    frames.clear();
}

bool MD5AnimationLoader::loadAnimation(const std::string& filename)
{
    std::string param;
    std::string junk;   // Read junk from the file

    std::ifstream file(filename.c_str());
    int fileLength = getFileLength(file);
    assert(fileLength > 0);

    file >> param;

    while(!file.eof())
    {
        if (param == "MD5Version")
        {
            file >> MD5Version;
            assert(MD5Version == 10);
        }
        else if (param == "commandline")
        {
            file.ignore(fileLength, '\n'); // Ignore everything else on the line
        }
        else if (param == "numFrames")
        {
            file >> numFrames;
            file.ignore(fileLength, '\n');
        }
        else if (param == "numJoints")
        {
            file >> numJoints;
            file.ignore(fileLength, '\n');
        }
        else if (param == "frameRate")
        {
            file >> framRate;
            file.ignore(fileLength, '\n');
        }
        else if (param == "numAnimatedComponents")
        {
            file >> numAnimatedComponents;
            file.ignore(fileLength, '\n');
        }
        else if (param == "hierarchy")
        {
            file >> junk; // read in the '{' character
            for (int i = 0; i < numJoints; ++i)
            {
                JointInfo joint;
                file >> joint.name >> joint.parentID >> joint.flags >> joint.startIndex;
                removeQuotes(joint.name);

                jointInfos.push_back(joint);

                file.ignore(fileLength, '\n');
            }
            file >> junk; // read in the '}' character
        }
        else if (param == "bounds")
        {
            file >> junk; // read in the '{' character
            file.ignore(fileLength, '\n');
            for (int i = 0; i < numFrames; ++i)
            {
                Bound bound;
                file >> junk; // read in the '(' character
                file >> bound.min.x >> bound.min.y >> bound.min.z;
                file >> junk >> junk; // read in the ')' and '(' characters.
                file >> bound.max.x >> bound.max.y >> bound.max.z;
                
                bounds.push_back(bound);

                file.ignore(fileLength, '\n');
            }

            file >> junk; // read in the '}' character
            file.ignore(fileLength, '\n');
        }
        else if (param == "baseframe")
        {
            file >> junk; // read in the '{' character
            file.ignore(fileLength, '\n');

            for (int i = 0; i < numJoints; ++i)
            {
                BaseFrame baseFrame;
                file >> junk;
                file >> baseFrame.pos.x >> baseFrame.pos.y >> baseFrame.pos.z;
                file >> junk >> junk;
                file >> baseFrame.orient.x >> baseFrame.orient.y >> baseFrame.orient.z;
                file.ignore(fileLength, '\n');

                baseFrames.push_back(baseFrame);
            }
            file >> junk; // read in the '}' character
            file.ignore(fileLength, '\n');
        }
        else if (param == "frame")
        {
            FrameData frame;
            file >> frame.frameID >> junk; // Read in the '{' character
            file.ignore(fileLength, '\n');

            for (int i = 0; i < numAnimatedComponents; ++i)
            {
                float frameData;
                file >> frameData;
                frame.frameData.push_back(frameData);
            }

            frames.push_back(frame);

            // Build a skeleton for this frame
            buildFrameSkeleton(frame);
            //buildFrameSkeleton(skeletons, jointInfos, baseFrames, frame);

            file >> junk; // Read in the '}' character
            file.ignore(fileLength, '\n' );        
        }

        file >> param;
    }

    assert((int) jointInfos.size() == numJoints);
    assert((int) baseFrames.size() == numJoints);
    assert((int) bounds.size() == numFrames);
    assert((int) frames.size() == numFrames);
    assert((int) skeletons.size() == numFrames);

    return true;
}

void MD5AnimationLoader::buildFrameSkeleton(const FrameData& frameData)
//void MD5AnimationLoader::buildFrameSkeleton(FrameSkeletonList& skeletons, const JointInfoList& jointInfos, const BaseFrameList& baseFrames, const FrameData& frameData)
{
    FrameSkeleton skeleton;

    for ( unsigned int i = 0; i < jointInfos.size(); ++i )
    {
        unsigned int j = 0;

        const JointInfo& jointInfo = jointInfos[i];
        // Start with the base frame position and orientation.
        SkeletonJoint animatedJoint = baseFrames[i];

        animatedJoint.parent = jointInfo.parentID;

        if ( jointInfo.flags & 1 ) // Pos.x
        {
            animatedJoint.pos.x = frameData.frameData[ jointInfo.startIndex + j++ ];
        }
        if ( jointInfo.flags & 2 ) // Pos.y
        {
            animatedJoint.pos.y = frameData.frameData[ jointInfo.startIndex + j++ ];
        }
        if ( jointInfo.flags & 4 ) // Pos.x
        {
            animatedJoint.pos.z  = frameData.frameData[ jointInfo.startIndex + j++ ];
        }
        if ( jointInfo.flags & 8 ) // Orient.x
        {
            animatedJoint.orient.x = frameData.frameData[ jointInfo.startIndex + j++ ];
        }
        if ( jointInfo.flags & 16 ) // Orient.y
        {
            animatedJoint.orient.y = frameData.frameData[ jointInfo.startIndex + j++ ];
        }
        if ( jointInfo.flags & 32 ) // Orient.z
        {
            animatedJoint.orient.z = frameData.frameData[ jointInfo.startIndex + j++ ];
        }

        computeQuatW( animatedJoint.orient );

        if ( animatedJoint.parent >= 0 ) // Has a parent joint
        {
            SkeletonJoint& parentJoint = skeleton.joints[animatedJoint.parent];
            glm::vec3 rotPos = parentJoint.orient * animatedJoint.pos;

            animatedJoint.pos = parentJoint.pos + rotPos;
            animatedJoint.orient = parentJoint.orient * animatedJoint.orient;

            animatedJoint.orient = glm::normalize( animatedJoint.orient );
        }
        
        skeleton.joints.push_back(animatedJoint);

        // Build the bone matrix for GPU skinning.
        glm::mat4x4 boneTranslate = glm::translate( animatedJoint.pos );
        glm::mat4x4 boneRotate = glm::toMat4( animatedJoint.orient );
        glm::mat4x4 boneMatrix = boneTranslate * boneRotate;
        
        skeleton.boneMatrices.push_back( boneMatrix );
    }

    skeletons.push_back(skeleton);

}

MD5AnimationLoader::~MD5AnimationLoader()
{
    skeletons.clear();
}

MD5Animation *MD5AnimationLoader::getAnimation() const
{
    MD5Animation *animation = new MD5Animation(this, numFrames, numJoints, framRate);

    return animation;
}

const MD5AnimationLoader::FrameSkeleton& MD5AnimationLoader::getSkeleton(int skeleton) const
{
    return skeletons[skeleton];
}

////////////////////////////////////////////////////////////////////////////////
// Animation Updater Class
////////////////////////////////////////////////////////////////////////////////

MD5Animation::MD5Animation(const MD5AnimationLoader *_baseAnimation, int _numJoints, int _numMeshes, int _frameRate) :
baseAnimation(_baseAnimation),
numFrames(_numJoints),
numJoints(_numMeshes),
framRate(_frameRate),
frameDuration(-1),
animDuration(-1),
animTime(0.0f),
animatedSkeleton(),
animateInReverse(false)
{
    frameDuration = 1.0f / (float)framRate;
    animDuration = (frameDuration * (float)numFrames);

    animatedSkeleton.joints.assign(numJoints, MD5AnimationLoader::SkeletonJoint());
    animatedSkeleton.boneMatrices.assign(numJoints, glm::mat4x4(1.0));
}

MD5Animation::~MD5Animation()
{

}

////////////////////////////////////////////////////////////////////////////////
// Updating
////////////////////////////////////////////////////////////////////////////////

void MD5Animation::update(float fDeltaTime)
{
    if (numFrames < 1) return;

    animTime += animateInReverse ? -fDeltaTime : fDeltaTime;

    while (animTime > animDuration) animTime -= animDuration;
    while (animTime < 0.0f) animTime += animDuration;

    // Figure out which frame we're on
    float fFramNum = animTime * (float)framRate;
    int iFrame0 = (int)(animateInReverse ? ceilf(fFramNum) : floorf(fFramNum));
    int iFrame1 = (int)(animateInReverse ? floorf(fFramNum) : ceilf(fFramNum));
    iFrame0 = iFrame0 % numFrames;
    iFrame1 = iFrame1 % numFrames;

    float fInterpolate = fmodf(animTime, frameDuration) / frameDuration;
    if (animateInReverse) fInterpolate = 1.0f - fInterpolate;

    interpolateSkeletons(animatedSkeleton, baseAnimation->getSkeleton(iFrame0), baseAnimation->getSkeleton(iFrame1), fInterpolate);
}

void MD5Animation::interpolateSkeletons(MD5AnimationLoader::FrameSkeleton& finalSkeleton, const MD5AnimationLoader::FrameSkeleton& skeleton0,
                                        const MD5AnimationLoader::FrameSkeleton& skeleton1, float fInterpolate)
{
    for (int i = 0; i < numJoints; ++i)
    {
        MD5AnimationLoader::SkeletonJoint& finalJoint = finalSkeleton.joints[i];
        glm::mat4x4& finalMatrix = finalSkeleton.boneMatrices[i];

        const MD5AnimationLoader::SkeletonJoint& joint0 = skeleton0.joints[i];
        const MD5AnimationLoader::SkeletonJoint& joint1 = skeleton1.joints[i];

        finalJoint.parent = joint0.parent;

        finalJoint.pos = glm::lerp(joint0.pos, joint1.pos, fInterpolate);
        //finalJoint.m_Pos = glm::mix(joint0.m_Pos, joint1.m_Pos, fInterpolate);
        finalJoint.orient = glm::mix(joint0.orient, joint1.orient, fInterpolate); //TODO fix glm's mix

        // Build the bone matrix for GPU skinning.
        finalMatrix = glm::translate(finalJoint.pos) * glm::toMat4( finalJoint.orient);
    }
}

////////////////////////////////////////////////////////////////////////////////
// Getters and Setters
////////////////////////////////////////////////////////////////////////////////

const MD5AnimationLoader::FrameSkeleton& MD5Animation::getSkeleton() const
{
    return animatedSkeleton;
}

const MD5AnimationLoader::SkeletonMatrixList& MD5Animation::getSkeletonMatrixList() const
{
    return animatedSkeleton.boneMatrices;
}

void MD5Animation::setAnimationDirection(bool reverse, bool restart)
{
    animateInReverse = reverse;

    if (restart) animTime = 0.0f;
}

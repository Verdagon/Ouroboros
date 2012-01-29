#ifndef MD5ANIMATION_H_
#define MD5ANIMATION_H_

#include "../Vector.h"
#include "../../include/glm/gtc/quaternion.hpp"
#include <vector>

class MD5Animation;

class MD5AnimationLoader
{
    // The JointInfo stores the information necessary to build the
    // skeletons for each frame
    struct JointInfo
    {
        std::string name;
        int         parentID;
        int         flags;
        int         startIndex;
    };
    typedef std::vector<JointInfo> JointInfoList;

    struct Bound
    {
        glm::vec3 min;
        glm::vec3 max;
    };
    typedef std::vector<Bound> BoundList;

    struct BaseFrame
    {
        glm::vec3 pos;
        glm::quat orient;
    };
    typedef std::vector<BaseFrame> BaseFrameList;

    struct FrameData
    {
        int frameID;
        std::vector<float> frameData;
    };
    typedef std::vector<FrameData> FrameDataList;

public:
    // A Skeleton joint is a joint of the skeleton per frame
    struct SkeletonJoint
    {
        int       parent;
        glm::vec3 pos;
        glm::quat orient;

        SkeletonJoint() : parent(-1), pos(0) { }

        SkeletonJoint(const BaseFrame& copy) :
        pos(copy.pos),
        orient(copy.orient) { }
    };
    typedef std::vector<SkeletonJoint> SkeletonJointList;
    typedef std::vector<glm::mat4x4> SkeletonMatrixList;

    // A frame skeleton stores the joints of the skeleton for a single frame.
    struct FrameSkeleton
    {
        SkeletonMatrixList boneMatrices;
        SkeletonJointList  joints;
    };
    typedef std::vector<FrameSkeleton> FrameSkeletonList;

private:
    int MD5Version;
    int numFrames;
    int numJoints;
    int framRate;
    int numAnimatedComponents;

    JointInfoList     jointInfos;
    BoundList         bounds;
    BaseFrameList     baseFrames;
    FrameDataList     frames;
    FrameSkeletonList skeletons;    // All the skeletons for all the frames

    bool loadAnimation(const std::string& filename);
    void buildFrameSkeleton(const FrameData& frameData);
    //void buildFrameSkeleton(FrameSkeletonList& skeletons, const JointInfoList& jointInfo,
    //                               const BaseFrameList& baseFrames, const FrameData& frameData);

public:
    MD5AnimationLoader(const std::string& filename);
    ~MD5AnimationLoader();

    MD5Animation *getAnimation() const;

    const FrameSkeleton& getSkeleton(int skeleton) const;
};

class MD5Animation
{
private:
    const MD5AnimationLoader *baseAnimation;

    int numFrames;
    int numJoints;
    int framRate;

    float frameDuration;
    float animDuration;
    float animTime;

    MD5AnimationLoader::FrameSkeleton animatedSkeleton;

    bool animateInReverse;

    // Build the frame skeleton for a particular frame
    void interpolateSkeletons(MD5AnimationLoader::FrameSkeleton& finalSkeleton, const MD5AnimationLoader::FrameSkeleton& skeleton0,
                              const MD5AnimationLoader::FrameSkeleton& skeleton1, float fInterpolate);

public:
    MD5Animation(const MD5AnimationLoader *_baseAnimation, int _numJoints, int _numMeshes, int _frameRate);
    virtual ~MD5Animation();

    void update(float fDeltaTime);

    const MD5AnimationLoader::FrameSkeleton& getSkeleton() const;
    const MD5AnimationLoader::SkeletonMatrixList& getSkeletonMatrixList() const;

    void setAnimationDirection(bool reverse, bool restart = false);
};
#endif /* MD5ANIMATION_H_ */

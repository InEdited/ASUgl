#ifndef HI
#define HI

// your declarations (and certain types of definitions) here


#include <map>
#include "jointtransform.h"


class Keyframe{
private:
	float timestamp_;
	std::map<std::string, JointTransform> pose_;

public:
	Keyframe() = default;
	Keyframe(float timestamp,std::map<std::string, JointTransform> pose);
	float gettimestamp();
	std::map<std::string, JointTransform> getJointKeyFrames();
};

#endif
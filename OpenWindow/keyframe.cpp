#include "keyframe.h"

Keyframe::Keyframe(float timestamp, std::map<std::string, JointTransform> pose)
{
	timestamp_ = timestamp;
	pose_ = pose;
}

float Keyframe::gettimestamp()
{
	return timestamp_;
}

std::map<std::string, JointTransform> Keyframe::getJointKeyFrames()
{
	return pose_;
}

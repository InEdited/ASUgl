#include "animation.h"

Animation::Animation(float seconds, std::vector<Keyframe> keyframes)
{
	length = seconds;
	keyframes_ = keyframes;
}

float Animation::getlength()
{
	return length;
}

std::vector<Keyframe> Animation::getkeyframe()
{
	return keyframes_;
}

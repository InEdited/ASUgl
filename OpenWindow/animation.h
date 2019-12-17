#pragma once

#include "keyframe.h"
#include <vector>

class Animation {
private:
	float length;
	std::vector<Keyframe> keyframes_;

public:
	Animation() = default;
	Animation(float seconds, std::vector<Keyframe> keyframes);
	float getlength();
	std::vector<Keyframe>getkeyframe();
};
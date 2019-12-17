#pragma once
#include "animation.h"
#include "geometry.h"
#include "keyframe.h"
#include "jointtransform.h"
#include "joint.h"
#include <map> 
#include <set>
class ColladaModel;

class Animator {
private:
	ColladaModel* entity_;
	Animation current_animation_;
	float animation_time_;

	void increaseAnimationTime();
	std::map<std::string,Matrix> calculateCurrentAnimationPose();
	void applyPoseToJoints(std::map<std::string, Matrix> currentPose, Joint joint, Matrix parentTransform);
	std::vector<Keyframe> getPreviousAndNextFrames();
	float calculateProgression(Keyframe previousFrame, Keyframe nextFrame);
	std::map<std::string, Matrix> interpolatePoses(Keyframe previousFrame, Keyframe nextFrame, float progression);

public:
	Animator(ColladaModel entity);
	void doanimation(Animation animation);
	void update();	
};

template< class Key,
	class T,
	class Comparator,
	class MapAllocator,
	class SetAllocator>
	void make_key_set(const std::map<Key, T, Comparator, MapAllocator>& map,
		std::set<Key, Comparator, SetAllocator>& set)
{
	set.clear();
	typedef typename std::map<Key, T, Comparator, MapAllocator> map_type;
	typename map_type::const_iterator itr = map.begin();
	while (map.end() != itr)
	{
		set.insert((itr++)->first);
	}
}


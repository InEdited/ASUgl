#include "animator.h"
#include "colladamodel.h"

void Animator::increaseAnimationTime()
{
	animation_time_ += (1/60.f);
	if (animation_time_ > current_animation_.getlength()) 
	{
		animation_time_ = (int)(animation_time_) % (int)(current_animation_.getlength());
	}
}



void Animator::applyPoseToJoints(std::map<std::string, Matrix> currentPose, Joint joint, Matrix parentTransform)
{
	Matrix currentLocalTransform = currentPose.at(joint.name);
	Matrix currentTransform = parentTransform * currentLocalTransform;
	for (Joint childJoint : joint.children) {
		applyPoseToJoints(currentPose, childJoint, currentTransform);
	}
	currentTransform = currentTransform * joint.getInverseBindTransform();
	joint.setTransform(currentTransform);
}

std::vector<Keyframe> Animator::getPreviousAndNextFrames()
{
	std::vector<Keyframe> allFrames = current_animation_.getkeyframe();
	Keyframe previousFrame = allFrames[0];
	Keyframe nextFrame = allFrames[0];
	for (int i = 1; i < allFrames.capacity(); i++) {
		nextFrame = allFrames[i];
		if (nextFrame.gettimestamp() > animation_time_) {
			break;
		}
		previousFrame = allFrames[i];
	}
	return std::vector<Keyframe>{ previousFrame, nextFrame };
}

float Animator::calculateProgression(Keyframe previousFrame, Keyframe nextFrame)
{
	float totalTime = nextFrame.gettimestamp() - previousFrame.gettimestamp();
	float currentTime = animation_time_ - previousFrame.gettimestamp();
	return currentTime / totalTime;
}

std::map<std::string, Matrix> Animator::interpolatePoses(Keyframe previousFrame, Keyframe nextFrame, float progression)
{
	std::map<std::string, Matrix> currentPose ;
	std::set<std::string> key_set;
	make_key_set(previousFrame.getJointKeyFrames(), key_set);
	for (std::string jointName : key_set)
	{
		JointTransform previousTransform = previousFrame.getJointKeyFrames().at(jointName);
		JointTransform nextTransform = nextFrame.getJointKeyFrames().at(jointName);
		JointTransform currentTransform = JointTransform::interpolate(previousTransform, nextTransform, progression);
		currentPose.insert_or_assign(jointName, currentTransform.getlocationtransform());
		make_key_set(previousFrame.getJointKeyFrames(), key_set);
	}
	return currentPose;
}

Animator::Animator(ColladaModel entity)
{
	entity_ = &entity;
}

void Animator::doanimation(Animation animation)
{
	animation_time_ = 0;
	current_animation_ = animation;
}

void Animator::update()
{
	if (&current_animation_ == NULL) {
		return;
	}
	increaseAnimationTime();
	std::map<std::string, Matrix> currentPose = calculateCurrentAnimationPose();
	applyPoseToJoints(currentPose, entity_->getrootjoint(), Matrix());
}

std::map<std::string, Matrix> Animator::calculateCurrentAnimationPose()
{
	std::vector<Keyframe> frames = getPreviousAndNextFrames();
	float progression = calculateProgression(frames[0], frames[1]);
	return interpolatePoses(frames[0], frames[1], progression);
}

template<typename TK, typename TV>
std::vector<TK> extract_keys(std::map<TK, TV> const& input_map) {
	std::vector<TK> retval;
	for (auto const& element : input_map) {
		retval.push_back(element.first);
	}
	return retval;
}

template<typename TK, typename TV>
std::vector<TV> extract_values(std::map<TK, TV> const& input_map) {
	std::vector<TV> retval;
	for (auto const& element : input_map) {
		retval.push_back(element.second);
	}
	return retval;
}
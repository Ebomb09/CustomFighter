#ifndef SKELETON_H
#define SKELETON_H

#include "clothing.h"
#include "math.h"

struct Bone {
	Vector2& start;
	Vector2& end;
};

struct Skeleton {

	Vector2	head, 
			torso[2],
			shoulder[2],
			elbow[2],
			wrist[2],
			hip[2],
			knee[2],
			heel[2],
			toes[2],
			fingers[2];

	Bone 	neck, 
			spine,
			collar[2], 	
			upperarm[2],
			forearm[2], 
			pelvis[2],	
			thigh[2], 
			calf[2],
			foot[2],
			hand[2];

	// Shortcut pointers
	Vector2 *const joints = &head;
	Bone *const bones = &neck;

	static const int jointCount = 19;
	static const int boneCount = 18;

	Skeleton();
	Skeleton(const Skeleton& copy);
	Skeleton(Skeleton&& move);

	Skeleton& operator=(const Skeleton& copy);
	Skeleton& operator=(Skeleton&& move);	

	void setDefault();
	
	Bone& getBone(int index);
	Vector2& getJoint(int index);

	void moveJoint(int index, Vector2 mov);
	void moveJoint(Vector2& joint, Vector2 mov);

	void draw(std::vector<Clothing*> clothes, float headAngle = 0.f);

private:

	void drawBone(std::vector<Clothing*> list, int part, Bone& bone, float width = 2, bool flip = false);
	void drawTorso(std::vector<Clothing*> list);
	void drawNeck(std::vector<Clothing*> list);	
	void drawHead(std::vector<Clothing*> list, float headAngle = 0.f);
	void drawUpperArm(std::vector<Clothing*> list, int side);
	void drawForeArm(std::vector<Clothing*> list, int side);
	void drawHand(std::vector<Clothing*> list, int side);
	void drawThigh(std::vector<Clothing*> list, int side);
	void drawCalf(std::vector<Clothing*> list, int side);
	void drawFoot(std::vector<Clothing*> list, int side);

	bool torsoTopFlipped();
	bool torsoBottomFlipped();
};

#endif
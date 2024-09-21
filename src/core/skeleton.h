#ifndef SKELETON_H
#define SKELETON_H

#include "clothing.h"
#include "math.h"

namespace SkeletonDrawOrder {

	enum {
		LegLeft,
		LegRight,
		ArmLeft,
		ArmRight,
		Body,
		Head,		
		Total
	};

	const std::string String[] {
		"Leg Left",
		"Leg Right",
		"Arm Left",
		"Arm Right",
		"Body",
		"Head",	
		"Total"
	};
};

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

	int order[SkeletonDrawOrder::Total] {
		SkeletonDrawOrder::LegLeft,
		SkeletonDrawOrder::ArmLeft,
		SkeletonDrawOrder::Body,
		SkeletonDrawOrder::Head,			
		SkeletonDrawOrder::LegRight,
		SkeletonDrawOrder::ArmRight			
	};

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

	void rotateJoint(int index, float rotate);
	void rotateJoint(Vector2& joint, Vector2& origin, float rotate);

	void draw(std::vector<Clothing> clothes, float headAngle = 0.f);
	void draw(sf::RenderTarget* renderer, std::vector<Clothing> clothes, float headAngle = 0.f, bool outline=true);

private:

	void drawBone(sf::RenderTarget* renderer, std::vector<Clothing> list, int part, Bone& bone, float width = 2, bool flip = false);
	void drawTorso(sf::RenderTarget* renderer,std::vector<Clothing> list);
	void drawNeck(sf::RenderTarget* renderer, std::vector<Clothing> list);	
	void drawHead(sf::RenderTarget* renderer, std::vector<Clothing> list, float headAngle = 0.f);
	void drawUpperArm(sf::RenderTarget* renderer, std::vector<Clothing> list, int side);
	void drawForeArm(sf::RenderTarget* renderer, std::vector<Clothing> list, int side);
	void drawHand(sf::RenderTarget* renderer, std::vector<Clothing> list, int side);
	void drawThigh(sf::RenderTarget* renderer, std::vector<Clothing> list, int side);
	void drawCalf(sf::RenderTarget* renderer, std::vector<Clothing> list, int side);
	void drawFoot(sf::RenderTarget* renderer, std::vector<Clothing> list, int side);

	bool torsoTopFlipped();
	bool torsoBottomFlipped();
};

#endif
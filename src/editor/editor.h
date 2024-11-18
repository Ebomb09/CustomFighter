#ifndef SKELETON_ANIMATOR_EDITOR_H
#define SKELETON_ANIMATOR_EDITOR_H

#include <string>
#include <vector>

#include "core/animation.h"
#include "core/player.h"

struct Editor {
	std::string fileName = "";
	Animation anim;
	int keyFrame = -1;
	int selected = -1;
	int dragZone = -1;
	Player player;
	int timer = 0;

	enum Mode {
		Frames,
	    Joints,
	    HitBoxes,
	    HurtBoxes
	};

	struct {
		bool drawGrid = true;
		bool drawSkeleton = true;
		bool drawModel = true;
		bool drawEffects = true;
		bool drawHitBox = true;
		bool drawHurtBox = true;

		int mode = Mode::Joints;

		bool playback = false;
		bool playbackTest = false;
		int playbackSpeed = 1;
	} settings;

	void update();

	void selectJoint();
	void selectRectangle();
	void selectDefault();

    Frame* 						getKeyFrame		(int key = -1);

	Skeleton* 					getPose			(int key = -1);
    Skeleton* 					getGrabee		(int key = -1);

	std::vector<Vector2*> 		getJoints		(int key = -1);
	Vector2* 					getJoint		(int index = -1, int key = -1);	
	Skeleton* 					getJointOwner	(int index = -1, int key = -1);

	std::vector<Rectangle*> 	getBoxes		(int key = -1);
	Rectangle*					getBox			(int index = -1, int key = -1);

    std::vector<HitBox>* 		getHitBoxes		(int key = -1);
	HitBox* 					getHitBox		(int index = -1, int key = -1);

	std::vector<HurtBox>* 		getHurtBoxes	(int key = -1);
	HurtBox* 					getHurtBox		(int index = -1, int key = -1);

	void setKeyFrame(int index);
	void setSelected(int index);
	void setDragZone(int mode);

	void drawGrid();
	void drawSkeleton();
    void drawModel();
	void drawEffects();
    void drawHitBox();
    void drawHurtBox();

    void resetPlayer();
};

#endif
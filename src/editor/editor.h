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
	    Joints,
	    HitBoxes,
	    HurtBoxes
	};

	struct {
		bool drawGrid = true;
		bool drawSkeleton = true;
		bool drawModel = true;
		bool drawHitBox = true;
		bool drawHurtBox = true;

		int mode = Mode::Joints;

		bool playback = false;
		int playbackSpeed = 1;
	} settings;

	void update();

	void selectJoint();
	void selectRectangle();
	void selectDefault();

	void setKeyFrame(int index);
	void setSelected(int index);
	void setDragZone(int mode);

	void drawGrid();
	void drawSkeleton();
    void drawModel();
    void drawHitBox();
    void drawHurtBox();

    void resetPlayer();

    Skeleton getSkeleton();
    std::vector<Rectangle*> getBoxes();

    Frame& getKeyFrame();
    std::vector<HitBox>& getHitBoxes();
    std::vector<HurtBox>& getHurtBoxes();    
};

#endif
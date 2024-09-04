#ifndef GAME_CHARACTER_H
#define GAME_CHARACTER_H

#include <vector>
#include <string>

#include "skeleton.h"
#include "clothing.h"
#include "animation.h"
#include "move.h"

#include "button.h"

using std::vector;
using std::string;

const Rectangle StageBounds = {
	-384,
	224,
	768,
	256
};

const Rectangle CameraBounds = {
	0, 
	0,
	256,
	192
};

struct Player {

	int gameIndex 	= 0;
	int seatIndex 	= -1;
	int team 		= -1;
	
	Button::Flag in;

	struct Config {
		std::vector<string>	clothes;
		string 				moves			[Move::Total];
		string 				motions			[Move::Total];

		void loadFromText(std::string str);
		string saveToText();
		void loadFromFile(std::string fileName);
		void saveToFile(std::string fileName);
	}config;

	struct State {
		int				tagCount		= 0;
		int				health			= 100;
		int				accDamage		= 0;
		int				stun			= 0;
		int				hitStop			= 0;
		int				hitKeyFrame		= 0;
		int				side			= 1;
		Vector2			position		= {0, 0};
		Vector2			velocity		= {0, 0};
		int				moveIndex		= Move::Stand;
		int				moveFrame		= 0;
		float 			look			= 0;
		Button::Flag	button			[Button::History];

	}state;

	Button::Flag readInput();
	void advanceFrame(std::vector<Player> others);
	void draw();

	void dealDamage(int dmg);

	int getTarget(std::vector<Player> others);

	Vector2 getSOCD(int index = 0);
	string getMotion(int index = 0);

	bool inMove(int move);
	void setMove(int move, bool loop = false);
	bool doneMove();

	bool taggedIn(std::vector<Player> others);

	HitBox getCollision(vector<Player> others);

	int getKeyFrame();
	Frame getFrame();
	
	Skeleton getSkeleton();
	vector<HitBox> getHitBoxes();
	vector<HurtBox> getHurtBoxes();	

	vector<Clothing*> getClothes();	

	Rectangle getRealBoundingBox();
	Rectangle getScreenBoundingBox();	
};

#endif
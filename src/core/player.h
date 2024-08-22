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

	int gameIndex = 0;
	int seatIndex = -1;

	struct Config {
		vector<string>	clothes;
		string 			moves			[Move::Total];
		string 			motions			[Move::Total];

		void loadFromText(string str);
		string saveToText();
		void loadFromFile(std::string fileName);
		void saveToFile(std::string fileName);
	}config;

	struct State {
		int 			target			= -1;
		int				health			= 100;
		int				accDamage		= 0;
		int				stun			= 0;
		int				hitStop			= 0;
		int				hitKeyFrame		= -1;
		int				side			= 1;
		Vector2			position		= {0, 0};
		Vector2			velocity		= {0, 0};
		int				moveIndex		= Move::Stand;
		int				moveFrame		= 0;
		float 			look			= 0;
		Button::Flag	button			[Button::History];

	}state;

	Button::Flag readInput();
	void advanceFrame(Button::Flag in, vector<Player> others);
	void draw();

	void dealDamage(int dmg);

	Vector2 getSOCD(int index = 0);
	string getMotion(int index = 0);

	bool inMove(int move);
	void setMove(int move, bool loop = false);
	bool doneMove();

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
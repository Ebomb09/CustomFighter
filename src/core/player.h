#ifndef GAME_CHARACTER_H
#define GAME_CHARACTER_H

#include <vector>
#include <string>

#include "skeleton.h"
#include "clothing.h"
#include "animation.h"
#include "move.h"

#include "button.h"

#define MAX_PLAYERS 4

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
		std::vector<std::string>	clothes;
		std::string 				moves			[Move::Total];
		std::string 				motions			[Move::Total];

		void loadFromText(std::string str);
		std::string saveToText();
		void loadFromFile(std::string fileName);
		void saveToFile(std::string fileName);
	}config;

	struct State {
		int				tagCount		= 0;
		int				health			= 100;
		int				accDamage		= 0;
		int				stun			= 0;
		int				hitStop			= 0;
		int				hitKeyFrame		[MAX_PLAYERS] {-1, -1, -1, -1};
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
	std::string getInputBuffer();
	int searchBestMove(std::string buffer);

	bool inMove(int move);
	void setMove(int move, bool loop = false);
	bool doneMove();

	bool taggedIn(std::vector<Player> others);

	HitBox getCollision(std::vector<Player> others);

	int getKeyFrame();
	Frame getFrame();
	
	Skeleton getSkeleton();
	std::vector<HitBox> getHitBoxes();
	std::vector<HurtBox> getHurtBoxes();	

	std::vector<Clothing*> getClothes();	

	Rectangle getRealBoundingBox();
	Rectangle getScreenBoundingBox();	
};

#endif
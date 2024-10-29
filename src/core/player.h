#ifndef GAME_CHARACTER_H
#define GAME_CHARACTER_H

#include <vector>
#include <string>

#include "math.h"
#include "skeleton.h"
#include "clothing.h"
#include "animation.h"
#include "move.h"
#include "video.h"
#include "stage.h"

#include "button.h"

#define MAX_PLAYERS 4

struct Player {

	// Game state specification
	int gameIndex 	= 0;
	int team 		= -1;

	// Player specification
	int aiLevel		= 0;
	int seatIndex 	= -1;

	Button::Flag in;

	struct Config {
		float armSize = 8.f;
		float legSize = 8.5f;
		float shoulderSize = 1.0f;
		float hipSize = 1.0f;
		float height = 1.0f;

		struct Cloth {
			std::string name	= "";
			int r 				= 128;
			int g				= 128;
			int b				= 128;
		};

		std::vector<Cloth>			clothes;
		std::string 				moves			[Move::Total];
		std::string 				motions			[Move::Total];

		void loadFromText(std::string str);
		std::string saveToText();
		void loadFromFile(std::string fileName);
		void saveToFile(std::string fileName);

		int calculatePoints();
	}config;

	struct Effect {
		const static int Max = 10;

		int id = -1;
		int counter = 0;
		int lifeTime = 0;
		sf::Color blend = sf::Color::White;
		Vector2 position = {0, 0};
		Vector2 velocity = {0, 0};
		Vector2 size = {0, 0};
		float angle = 0;
	};

	struct State {
		int				counter			= 0;
		int				tagCount		= 0;
		int				health			= 100;
		int				accDamage		= 0;
		int				stun			= 0;
		int				hitStop			= 0;
		int				hitKeyFrame		[MAX_PLAYERS] {-1, -1, -1, -1};
		int				side			= 1;
		Vector2			position		= {0, 0};
		Vector2			velocity		= {0, 0};
		Vector2			pushBack		= {0, 0};
		int 			aiMove			= -1;
		int				moveIndex		= Move::Stand;
		int				moveFrame		= 0;
		int 			fromMoveIndex	= -1;
		int				fromMoveFrame	= -1;
		float 			look			= 0;
		int				grabIndex		= -1;
		int				grabeeIndex		= -1;
		Button::Flag	button			[Button::History];
		Effect			effects			[Effect::Max];
	}state;

	Button::Flag readInput();
	Button::Flag readInput(std::vector<Player>& others);

	void setState(const Player::State& _state);
	void advanceFrame(std::vector<Player>& others);
	void advanceEffects(std::vector<Player>& others);

	void draw(Renderer* renderer = NULL);
	void drawShadow(Renderer* renderer = NULL);
	void drawEffects(Renderer* renderer = NULL);

	Effect& createEffect();

	void dealDamage(int dmg);

	const Vector2& getSOCD(int index = 0);
	std::vector<std::string> getInputBuffer();
	std::vector<std::string> getMotionBuffer(const std::string& motion);
	bool matchLeftConform(const std::string& a, const std::string& b);
	int searchBestMove(const std::vector<std::string>& inputBuffer);

	bool inMove(int move);
	bool doneMove();
	void setMove(int move, bool loop = false);

	const int& getTarget(std::vector<Player>& others);
	const bool& getTaggedIn(std::vector<Player>& others);

	bool inCorner();

	struct Collision {
		bool collided 		= false;
		bool block 			= false;
		int index 			= -1;
		Vector2 location;
		HitBox hitBox;
	};
	const Collision& getCollision(std::vector<Player>& others);

	Vector2 getCameraCenter(std::vector<Player>& others);

	const int& getKeyFrame();
	const Frame& getFrame();
	const Skeleton& getSkeleton();
	const std::vector<HitBox>& getHitBoxes();
	const std::vector<HurtBox>& getHurtBoxes();
	const std::vector<Clothing>& getClothes();
	const std::vector<Animation*>& getAnimations();

	Rectangle getRealBoundingBox();

	struct Cache {
		bool enabled 					= false;

		Frame			frame;
		int 			frameCounter	= -1;

		Vector2			socd;
		int 			socdCounter		= -1;

		int 			target;
		int 			targetCounter	= -1;
		
		bool			tagged;
		int 			taggedCounter	= -1;

		Collision		collision;
		int				collisionCounter = -1;

		std::vector<Animation*>	anims;
		std::vector<Clothing>	clothes;
	}cache;
};

#endif
#include "game_state.h"
#include "game_tools.h"
#include "random.h"

#include "core/menu.h"
#include "core/audio.h"
#include "core/video.h"
#include "core/save.h"

#include <json.hpp>

#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <string>
#include <vector>

using std::vector, std::string;
using namespace std::chrono;

void Game::init(int _playerCount, int _roundMax, int _timerMax, int _gameMode) {
	gameMode = _gameMode;
	playerCount = _playerCount;
	roundMax = _roundMax;
	timerMax = _timerMax;

	for(int i = 0; i < playerCount; i ++) {
		players[i].gameIndex = i;
		players[i].cache.enabled = true;
	}

	// Setup teams
    if(playerCount == 2) {
        players[0].team = 0;
        players[1].team = 1;

    }else if(playerCount == 4) {
        players[0].team = 0;
        players[1].team = 0;
        players[2].team = 1;
        players[3].team = 1;        
    }

	// Get a stage
	stage = g::save.getStage(g::save.getRandomStage());

	if(gameMode == Mode::Rounds)
		__Rounds__loadChoices();

    resetGame();
}

bool Game::done() {
	return state.flow == Flow::Done;
}

Game::SaveState Game::saveState() {
	return {
		state,
		{
			players[0].state,
			players[1].state,
			players[2].state,
			players[3].state
		}		
	};
}

void Game::loadState(SaveState save) {
	state = save.game;

	for(int i = 0; i < playerCount; i ++)
		players[i].state = save.players[i];
}

void Game::resetGame() {

	// Reset game state
	state = Game::State();

	// Reset the vote for rematch
    for(int i = 0; i < playerCount; i ++)
    	state.rematch[i] = 0;

	// Set the configs to the default punch / kicks
	if(gameMode == Mode::Rounds) 
		__Rounds__resetPlayerConfigs();
	
	nextRound();
}

void Game::resetRound() {
	state.flow = Flow::Countdown;
	state.timer = 2 * 60;

	state.round ++;
	state.slomo = 0;

	// Reset players to default states
	for(int i = 0; i < playerCount; i ++) 
		players[i].state = Player::State();

    // 2 Player Game
    if(playerCount == 2) {
        players[0].state.position = -75;
        players[0].state.side = 1;
        players[1].state.position = 75; 
        players[1].state.side = -1;

    // 4 Player Tag Game
    } else if(playerCount == 4) {
        players[0].state.position = -75;
        players[0].state.side = 1;
        players[1].state.position = -25;
        players[1].state.side = 1;
        players[2].state.position = 75;
        players[2].state.side = -1;
        players[3].state.position = 25;
        players[3].state.side = -1;
    }	

	// Game mode specific configuration
	switch(gameMode) {

		case Mode::Rounds:

			for(int i = 0; i < playerCount; i ++) {
				players[i].state.health = state.round * 10;
			}
			break;
	}

	// Set camera size
	g::video.camera.w = CameraBounds.w;
	g::video.camera.h = CameraBounds.w * g::video.getSize().y / g::video.getSize().x;

	// Center camera
    g::video.camera.x = -g::video.camera.w / 2.f;
    g::video.camera.y = g::video.camera.h;

    // Play music
    vector<string> songs = {
    	"Metalmania",
    	"Rocket Power"
    };

    vector<string> climax = {
    	"Exhilarate",
    	"Ready Aim Fire"
    };

    // One point away
    if(state.lWins >= roundMax - 1 || state.rWins >= roundMax - 1) {

    	if(!state.songClimax)
    		g::audio.playMusic(g::save.getMusic(climax[rand() % climax.size()]));

    	state.songClimax = true;

    }else {

    	if(!state.songNormal) 
    		g::audio.playMusic(g::save.getMusic(songs[rand() % songs.size()]));

    	state.songNormal = true;
    }

    // Play announcer round
    if(state.lWins == roundMax - 1 && state.rWins == roundMax - 1)
    	g::audio.playSound(g::save.getSound("round_final"));    	
    else
        g::audio.playSound(g::save.getSound("round_" + std::to_string(state.round)));    	
}

void Game::nextRound() {

	if(state.rWins == roundMax || state.lWins == roundMax) {
		state.flow = Flow::RematchScreen;

	// Get back into the next fight
	}else if(gameMode == Mode::Versus){
		resetRound();

	// Prepare to select new moves
	}else if(gameMode == Mode::Rounds){

		if(state.round > 0) {
			state.flow = Flow::RoundsScreen;
			state.roundsChooser = 0;
			__Rounds__prepareChoices();

		}else {
			resetRound();
		}
	}
}

void Game::__Rounds__prepareChoices() {
	Player& ply = players[state.roundsChooser];

	int seed = (
		std::abs(state.timer) + 
		ply.state.health +
		ply.state.counter +
		state.roundsChooser
		) % Random::Total;

	// Create the four choices
	for(int i = 0; i < 4; i ++) {
		vector<int> qualified = __Rounds__getQualifiedChoices(i);

		state.roundsChoice[i] = qualified[Random::Integer[seed] % qualified.size()];
		seed ++;
	}
}

vector<int> Game::__Rounds__getQualifiedChoices(int choice) {
	vector<__Rounds__Choice> list = __Rounds__choices;
	vector<int> out;

	for(int i = 0; i < list.size(); i ++) {
		bool equiped = false;

		for(int j = 0; j < Move::Total; j ++) {

			if(players[state.roundsChooser].config.moves[j] == list[i].move) {
				equiped = true;
				break;
			}

			if(players[state.roundsChooser].config.motions[j] == list[i].motion) {
				equiped = true;
				break;
			}
		}

		if(!equiped)
			out.push_back(i);
	}
	return out;
}

void Game::__Rounds__loadChoices() {
	std::fstream file("data/gamemode/rounds_choices.json", std::fstream::in);
	nlohmann::json json;

	if(file.good()) {

		try {
			json = nlohmann::json::parse(file);
			file.close();

			for(auto& entry : json) {
				__Rounds__Choice ch;

				if(entry["isDefault"].is_boolean())
					ch.isDefault = entry["isDefault"];

				if(entry["move"].is_string())
					ch.move = entry["move"];

				if(entry["motion"].is_string())
					ch.motion = entry["motion"];

				__Rounds__choices.push_back(ch);
			}

		}catch(nlohmann::json::exception e) {
			file.close();
			return;
		}
	}
	file.close();
	return;
}

void Game::__Rounds__resetPlayerConfigs() {

	for(int i = 0; i < playerCount; i ++) {

		// Set to no choice
		for(int j = 0; j < __Rounds__confSize; j ++)
			state.roundsConf[i][j] = -1;

		// Set the default config to specific animations
		for(int j = 0; j < __Rounds__choices.size(); j ++) {

			// Find first free slot to fill
			if(__Rounds__choices[j].isDefault)  {

				for(int u = 0; u < __Rounds__confSize; u ++) {

					if(state.roundsConf[i][u] == -1) {
						state.roundsConf[i][u] = j;
						break;
					}
				}
			}
		}
	}
}

void Game::__Rounds__fixPlayerConfigs() {

	for(int i = 0; i < playerCount; i ++) {

		for(int j = 0; j < __Rounds__confSize; j ++) {

			if(state.roundsConf[i][j] != -1) {
				players[i].config.moves[Move::Custom00 + j] = __Rounds__choices[state.roundsConf[i][j]].move;
				players[i].config.motions[Move::Custom00 + j] = __Rounds__choices[state.roundsConf[i][j]].motion;

				// Clear the cache to signal change has occured
				players[i].cache.anims.clear();

			}else {
				players[i].config.moves[Move::Custom00 + j] = "";
				players[i].config.motions[Move::Custom00 + j] = "";
			}
		}
	}
}

void Game::readInput() {
	vector<Player> others;

	for(int i = 0; i < playerCount; i ++)
		others.push_back(players[i]);

	for(int i = 0; i < playerCount; i ++)
		players[i].in = players[i].readInput(others);
}

void Game::advanceFrame() {

	// Update the configs continuously to account for rollback frames during move selection
	if(gameMode == Mode::Rounds) 
		__Rounds__fixPlayerConfigs();

	if(state.timer > 0)
		state.timer --;

	switch(state.flow) {

		case Flow::Countdown: {
			vector<Player> others;

			for(int i = 0; i < playerCount; i ++) {
				players[i].in = Button::Flag();
				others.push_back(players[i]);
			}

			for(int i = 0; i < playerCount; i ++) 
				players[i].advanceFrame(others);

			if(state.timer <= 0) {
				state.flow = Flow::PlayRound;
				state.timer = timerMax * 60;

				g::audio.playSound(g::save.getSound("announcer_fight"));
			}
			break;
		}

		case Flow::KO:
		case Flow::DoubleKO:
		case Flow::TimeUp: {
			vector<Player> others;

			for(int i = 0; i < playerCount; i ++) {
				players[i].in = Button::Flag();
				others.push_back(players[i]);
			}

			for(int i = 0; i < playerCount; i ++) 
				players[i].advanceFrame(others);

			if(state.timer <= 0) {
				nextRound();
			}
			break;
		}

		case Flow::PlayRound: {

			if(state.slomo > 0)
				state.slomo --;

			if(state.slomo % 2 == 1)
				break;

			vector<Player> others;

			for(int i = 0; i < playerCount; i ++)
				others.push_back(players[i]);

			for(int i = 0; i < playerCount; i ++)
				players[i].advanceFrame(others);

			if(state.timer <= 0) {
				state.flow = Flow::TimeUp;
				state.timer = 4 * 60;

				// Check which team has the most health
				int lTeamHP = 0;
				int rTeamHP = 0;

				for(int i = 0; i < playerCount; i ++) {

					if(players[i].team == 0)
						lTeamHP += players[i].state.health;
					else
						rTeamHP += players[i].state.health;
				}

				if(rTeamHP < lTeamHP) {
					state.lWins ++;

				}else if(rTeamHP > lTeamHP) {
					state.rWins ++;

				}else {
					state.rWins ++;
					state.lWins ++;
				}
				g::audio.playSound(g::save.getSound("announcer_timeup"));
				break;
			}

			// Check for any knockouts
			int lTeamAlive = 0;
			int rTeamAlive = 0;

			for(int i = 0; i < playerCount; i ++) {

				// Count alive players
				if(players[i].state.health > 0) {

					if(players[i].team == 0) {
						lTeamAlive ++;

					}else {
						rTeamAlive ++;
					}
				}

				// Slomo on player KO
				if(players[i].state.health <= 0 && others[i].state.health > 0) {

					// Hyperbolize the player KO
					if(std::abs(players[i].state.velocity.x) < 2)
						players[i].state.velocity.x = -players[i].state.side * 2;

					players[i].state.velocity.y += 3;
					state.slomo = 120;
				}
			}
	
			// Check knockout win conditions
			if(lTeamAlive == 0 && rTeamAlive == 0) {
				state.flow = Flow::DoubleKO;
				state.timer = 4 * 60;
				state.rWins ++;
				state.lWins ++;
				g::audio.playSound(g::save.getSound("announcer_ko"));

			}else if(lTeamAlive == 0) {
				state.flow = Flow::KO;
				state.timer = 4 * 60;
				state.rWins ++;
				g::audio.playSound(g::save.getSound("announcer_ko"));

			}else if(rTeamAlive == 0) {
				state.flow = Flow::KO;
				state.timer = 4 * 60;
				state.lWins ++;
				g::audio.playSound(g::save.getSound("announcer_ko"));
			}

			break;
		}

		case Flow::RematchScreen: {

			// Steal the players buttons
			for(int i = 0; i < playerCount; i ++) {

				if(players[i].in.D) 
					state.rematch[i] = -1;

				else if(players[i].in.B)
					state.rematch[i] = 1;
			}

			// If all agreed resetGame, if one didn't we are done
			int sum = 0;

			for(int i = 0; i < playerCount; i ++) {
				sum += state.rematch[i];

				if(state.rematch[i] < 0)
					state.flow = Flow::Done;
			}

			if(sum == playerCount)
				resetGame();
				
			break;
		}

		case Flow::RoundsScreen: {
			Player& ply = players[state.roundsChooser];

			int choice = -1;

			if(ply.in.A) choice = 0;
			if(ply.in.B) choice = 1;
			if(ply.in.C) choice = 2;
			if(ply.in.D) choice = 3;

			if(choice != -1) {

				// Save the choosen moves
				for(int i = 0; i < __Rounds__confSize; i ++) {

					if(state.roundsConf[state.roundsChooser][i] == -1) {
						state.roundsConf[state.roundsChooser][i] = state.roundsChoice[choice];
						break;
					}
				}

				state.roundsChooser ++;

				// All players have choosen their new power
				if(state.roundsChooser >= playerCount) {
					resetRound();

				// Continue picking new moves
				}else{
					__Rounds__prepareChoices();
				}
			}
			break;
		}
	}

	setCamera(players, playerCount);
}

void Game::draw() {
    stage.draw();

    drawHealthBars(players, playerCount);

    // Draw round wins
    drawRoundTokens(state.lWins, state.rWins, roundMax);

    // Draw rematch
    if(state.flow == Flow::RematchScreen) {

    	for(int i = 0; i < playerCount; i ++) {

    		string str = "Rematch [ ]";

    		if(state.rematch[i] > 0) {
    			str = "Rematch [X]";
    		}

	   	    sf::Text txt;
		    txt.setString(str);
		    txt.setFont(*g::save.getFont("Anton-Regular"));
		    txt.setCharacterSize(64);
		    txt.setFillColor(sf::Color::White);
		    txt.setOutlineThickness(1);
		    txt.setOutlineColor(sf::Color::Black);
		    txt.setPosition({g::video.getSize().x / 2.f - txt.getLocalBounds().width / 2.f, 128.f + i * 72.f});
		    g::video.draw(txt); 
    	}
    }

    // Draw clock
    if(state.flow == Flow::PlayRound) {
	    sf::Text txt;
	    txt.setString(std::to_string(state.timer / 60));
	    txt.setFont(*g::save.getFont("Anton-Regular"));
	    txt.setCharacterSize(64);
	    txt.setFillColor(sf::Color::White);
		txt.setOutlineThickness(1);	 	    
	    txt.setOutlineColor(sf::Color::Black);
	    txt.setPosition({g::video.getSize().x / 2.f - txt.getLocalBounds().width / 2.f, 0});
	    g::video.draw(txt);    	
    }

    // Draw Players
    for(int i = 0; i < playerCount; i ++) 
    	players[i].drawShadow();  

    for(int i = 0; i < playerCount; i ++) 
    	players[i].draw();

    for(int i = 0; i < playerCount; i ++) 
    	players[i].drawEffects();  

    // Draw round header
    if(state.flow == Flow::Countdown) {
	    sf::Text txt;
	    txt.setString("Round " + std::to_string(state.round));
	    txt.setFont(*g::save.getFont("Anton-Regular"));
	    txt.setCharacterSize(64);
	    txt.setFillColor(sf::Color::Red);
		txt.setOutlineThickness(1);
		txt.setOutlineColor(sf::Color::Black);
	    txt.setPosition({g::video.getSize().x / 2.f - txt.getLocalBounds().width / 2.f, g::video.getSize().y / 2.f});
	    g::video.draw(txt); 
    }
	
	if(state.flow == Flow::PlayRound) {

		if(state.timer > (timerMax - 1) * 60) {
			sf::Text txt;
			txt.setString("Fight!");
			txt.setFont(*g::save.getFont("Anton-Regular"));
			txt.setCharacterSize(64);
			txt.setFillColor(sf::Color::Red);
			txt.setOutlineThickness(1);
			txt.setOutlineColor(sf::Color::Black);
			txt.setPosition({g::video.getSize().x / 2.f - txt.getLocalBounds().width / 2.f, g::video.getSize().y / 2.f});
			g::video.draw(txt);  
		}
    }
	
	if(state.flow == Flow::RoundsScreen) {

		// Render the choices
		for(int i = 0; i < 4; i ++) {

			Rectangle area = {
				(float)g::video.getSize().x / 2 * (float)(i % 2), 
				(float)g::video.getSize().y / 2 * (float)(i / 2), 
				(float)g::video.getSize().x / 2, 
				(float)g::video.getSize().y / 2
			};	

			Rectangle iconDiv = {
				area.x,
				area.y,
				64,
				64
			};

			Rectangle plyDiv = {
				area.x,
				area.y,
				area.w,
				area.h / 2.f
			};

			Rectangle moveDiv = {
				area.x,
				plyDiv.y + plyDiv.h,
				area.w,
				area.h / 4.f
			};

			Rectangle inputDiv = {
				area.x,
				moveDiv.y + moveDiv.h,
				area.w,
				area.h / 4.f
			};

			sf::RectangleShape sh = area;
			sh.setFillColor(sf::Color::Black);
			g::video.draw(sh);

			// Create the dummy animation
			Player dummy;
			dummy.config = players[state.roundsChooser].config;
			auto time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count(); 
			dummy.config.moves[0] = __Rounds__choices[state.roundsChoice[i]].move;
			dummy.state.moveIndex = 0;                                         
			dummy.state.moveFrame = (time / 17) % dummy.getAnimations()[0]->getFrameCount();

			Menu::renderPlayer(dummy, dummy.getRealBoundingBox(), plyDiv);
			Menu::renderText(__Rounds__choices[state.roundsChoice[i]].move, "Anton-Regular", sf::Color::White, moveDiv, -1);
			Menu::renderText(__Rounds__choices[state.roundsChoice[i]].motion, "fight", sf::Color::White, inputDiv, 0);

			sh = iconDiv;
			sh.setFillColor(sf::Color::White);
			g::video.draw(sh);

			string buttonStr = "";
			if(i == 0)	buttonStr = "A";
			if(i == 1)	buttonStr = "B";
			if(i == 2)	buttonStr = "C";
			if(i == 3)	buttonStr = "D";

			Menu::renderText(buttonStr, "fight", sf::Color::Black, iconDiv, 0);
		}
	}

	if(state.flow == Flow::KO || state.flow == Flow::DoubleKO || state.flow == Flow::TimeUp) {
	    sf::Text txt;

		if(state.flow == Flow::KO)
			txt.setString("KO");

		else if(state.flow == Flow::DoubleKO)
			txt.setString("Double KO");

		else if(state.flow == Flow::TimeUp)
			txt.setString("Time Up");

	    txt.setFont(*g::save.getFont("Anton-Regular"));
	    txt.setCharacterSize(64);
	    txt.setFillColor(sf::Color::Red);
		txt.setOutlineThickness(1);
	    txt.setOutlineColor(sf::Color::Black);
	    txt.setPosition({g::video.getSize().x / 2.f - txt.getLocalBounds().width / 2.f, g::video.getSize().y / 2.f});
	    g::video.draw(txt);
    }
}
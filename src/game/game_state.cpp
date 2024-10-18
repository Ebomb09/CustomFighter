#include "game_state.h"
#include "game_tools.h"

#include "core/audio.h"
#include "core/video.h"
#include "core/save.h"

#include <thread>
#include <string>
#include <vector>

using std::vector, std::string;

void Game::init(int _playerCount, int _roundMax, int _timerMax) {
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

    resetGame();
}

bool Game::done() {
	return state.done;
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

    for(int i = 0; i < playerCount; i ++)
    	state.rematch[i] = 0;

	resetRound();
}

void Game::resetRound() {
	state.round ++;
	state.timer = (timerMax + 3) * 60;
	state.judge = false;
	state.slomo = 0;
	state.playJudgement = false;
	state.playFight = false;

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
    if(state.lWins == roundMax - 1 || state.rWins == roundMax - 1) {

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
		state.gameOver = true;

	}else {
		resetRound();
	}
}

static void readPlayerInput(Player& ply, vector<Player>& others) {
	ply.in = ply.readInput(others);
}

void Game::readInput() {

	// Only start reading input once the match begins
	if(state.timer > timerMax * 60) 
		return;

	vector<Player> others;

	for(int i = 0; i < playerCount; i ++)
		others.push_back(players[i]);
		
	/*
	// Start worker threads to read in the inputs
	vector<std::thread> tasks(playerCount);

	for(int i = 0; i < playerCount; i ++) 
		tasks[i] = std::thread(readPlayerInput, std::ref(players[i]), std::ref(others));
	
	for(int i = 0; i < playerCount; i ++)
		tasks[i].join();
	*/

	// Read inputs
	for(int i = 0; i < playerCount; i ++)
		players[i].in = players[i].readInput(others);
}

static void advancePlayerFrame(Player& ply, vector<Player>& others) {
	ply.advanceFrame(others);
}

void Game::advanceFrame() {

	if(state.done)
		return;

	state.timer --;

	// Check if want to rematch
	if(state.gameOver) {

		// Steal the players buttons
		for(int i = 0; i < playerCount; i ++) {

			if(players[i].in.D) {
				state.rematch[i] = -1;

			}else if(players[i].in.B) {
				state.rematch[i] = 1;				
			}
			players[i].in = Button::Flag();
		}

		// If all agreed resetGame, if one didn't we are done
		int sum = 0;

		for(int i = 0; i < playerCount; i ++) {
			sum += state.rematch[i];

			if(state.rematch[i] < 0)
				state.done = true;
		}

		if(sum == playerCount)
			resetGame();

		return;
	}

	// Slomo every other frame
	if(state.slomo > 0)
		state.slomo --;

	if(state.slomo % 2 == 0) {

		// Update players
		vector<Player> others;

		for(int i = 0; i < playerCount; i ++) {
			others.push_back(players[i]);

			// Still in round start
			if(state.timer > timerMax * 60) 
				players[i].in = Button::Flag();
		}		

		/*
		// Start worker threads to determine the next player state
		vector<std::thread> tasks(playerCount);

		for(int i = 0; i < playerCount; i ++) 
			tasks[i] = std::thread(advancePlayerFrame, std::ref(players[i]), std::ref(others));

		for(int i = 0; i < playerCount; i ++) 
			tasks[i].join();
		*/

		// Advance frame
		for(int i = 0; i < playerCount; i ++)
			players[i].advanceFrame(others);

		// Check for any KOs
		for(int i = 0; i < playerCount; i ++) {

			// Slomo on player KO
			if(players[i].state.health <= 0 && others[i].state.health > 0) {
				state.slomo = 120;

				// Hyperbolize the player KO
				if(std::abs(players[i].state.velocity.x) < 2)
					players[i].state.velocity.x = -players[i].state.side * 2;

				players[i].state.velocity.y += 3;
			}
		}
	}

	setCamera(players, playerCount);

	// Consider win conditions
	if(state.timer > 0) {

		// Check alive status of teams
		if(state.judge == Judgement::None) {
			int lTeamAlive = 0;
			int rTeamAlive = 0;

			for(int i = 0; i < playerCount; i ++) {

				if(players[i].state.health > 0) {

					if(players[i].team == 0) {
						lTeamAlive ++;

					}else {
						rTeamAlive ++;
					}
				}
			}

			if(lTeamAlive == 0 && rTeamAlive == 0) {
				state.timer = 0;
				state.rWins ++;
				state.lWins ++;
				state.judge = Judgement::DoubleKO;

			}else if(lTeamAlive == 0) {
				state.timer = 0;
				state.rWins ++;
				state.judge = Judgement::KO;

			}else if(rTeamAlive == 0) {
				state.timer = 0;
				state.lWins ++;
				state.judge = Judgement::KO;
			}
		}

	// Round over
	}else {

		// Check which team has the most health
		if(state.judge == Judgement::None) {
			int lTeamHP = 0;
			int rTeamHP = 0;

			for(int i = 0; i < playerCount; i ++) {

				if(players[i].team == 0)
					lTeamHP += players[i].state.health;
				else
					rTeamHP += players[i].state.health;
			}

			if(rTeamHP < lTeamHP)
				state.lWins ++;

			else if(rTeamHP > lTeamHP)
				state.rWins ++;

			state.judge = Judgement::TimeUp;
		}

		// Wait a couple seconds after judgement to go next
		if(state.timer < -4 * 60)
			nextRound();
	}
}

void Game::draw() {
    stage.draw();

    drawHealthBars(players, playerCount);

    // Draw round wins
    drawRoundTokens(state.lWins, state.rWins, roundMax);

    // Draw rematch
    if(state.gameOver) {

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
    if(state.timer < timerMax * 60 && state.timer >= 0) {
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
    if(state.timer > (timerMax + 1) * 60) {
	    sf::Text txt;
	    txt.setString("Round " + std::to_string(state.round));
	    txt.setFont(*g::save.getFont("Anton-Regular"));
	    txt.setCharacterSize(64);
	    txt.setFillColor(sf::Color::Red);
		txt.setOutlineThickness(1);	    
		txt.setOutlineColor(sf::Color::Black);	    
	    txt.setPosition({g::video.getSize().x / 2.f - txt.getLocalBounds().width / 2.f, g::video.getSize().y / 2.f});
	    g::video.draw(txt); 

    }else if(state.timer > timerMax * 60) {
	    sf::Text txt;
	    txt.setString("Fight!");
	    txt.setFont(*g::save.getFont("Anton-Regular"));
	    txt.setCharacterSize(64);
	    txt.setFillColor(sf::Color::Red);
		txt.setOutlineThickness(1);
	    txt.setOutlineColor(sf::Color::Black);
	    txt.setPosition({g::video.getSize().x / 2.f - txt.getLocalBounds().width / 2.f, g::video.getSize().y / 2.f});
	    g::video.draw(txt);  

	    if(!state.playFight) {
	    	g::audio.playSound(g::save.getSound("announcer_fight"));
	    	state.playFight = true;
	    }

    }else if(state.judge) {

	    sf::Text txt;

	    if(state.judge == Judgement::KO) {
	    	txt.setString("KO");

		    if(!state.playJudgement) {
		    	g::audio.playSound(g::save.getSound("announcer_ko"));
		    	state.playJudgement = true;
		    }	 


	    }else if(state.judge == Judgement::TimeUp) {
	    	txt.setString("Time Up");

		    if(!state.playJudgement) {
		    	g::audio.playSound(g::save.getSound("announcer_timeup"));
		    	state.playJudgement = true;
		    }	 


	    }else if(state.judge == Judgement::DoubleKO) {
	    	txt.setString("Double KO");

		    if(!state.playJudgement) {
		    	g::audio.playSound(g::save.getSound("announcer_ko"));
		    	state.playJudgement = true;
		    }	    	
	    }

	    txt.setFont(*g::save.getFont("Anton-Regular"));
	    txt.setCharacterSize(64);
	    txt.setFillColor(sf::Color::Red);
		txt.setOutlineThickness(1);
	    txt.setOutlineColor(sf::Color::Black);
	    txt.setPosition({g::video.getSize().x / 2.f - txt.getLocalBounds().width / 2.f, g::video.getSize().y / 2.f});
	    g::video.draw(txt);  
    }
}
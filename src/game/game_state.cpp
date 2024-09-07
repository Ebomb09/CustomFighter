#include "game_state.h"
#include "game_tools.h"

#include "core/render_instance.h"
#include "core/save.h"

#include <string>
#include <vector>

using std::vector, std::string;

void Game::init(int _playerCount, int _roundMax, int _timerMax) {
	playerCount = _playerCount;
	roundMax = _roundMax;
	timerMax = _timerMax;

	for(int i = 0; i < playerCount; i ++)
		players[i].gameIndex = i;

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
}

void Game::nextRound() {

	if(state.rWins == roundMax || state.lWins == roundMax) {
		state.gameOver = true;

	}else {
		resetRound();
	}
}

void Game::advanceFrame() {

	if(state.done)
		return;

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

	state.timer --;

	// Update players
	vector<Player> others;

	for(int i = 0; i < playerCount; i ++) {
		others.push_back(players[i]);

		// Still in round start
		if(state.timer > timerMax * 60) 
			players[i].in = Button::Flag();
	}

	for(int i = 0; i < playerCount; i ++) 
		players[i].advanceFrame(others);

	setCamera(players, playerCount);	

	// Consider win conditions
	if(state.timer > 0) {

		// Check alive status of teams
		if(!state.judge) {
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

			}else if(lTeamAlive == 0) {
				state.timer = 0;
				state.rWins ++;
				state.judge = true;

			}else if(rTeamAlive == 0) {
				state.timer = 0;
				state.lWins ++;
				state.judge = true;
			}
		}

	// Round over
	}else {

		// Check which team has the most health
		if(!state.judge) {
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

			state.judge = true;
		}

		// Wait a couple seconds after judgement to go next
		if(state.timer < -3 * 60)
			nextRound();
	}
}

void Game::draw() {
    drawStage(0);
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
    	players[i].draw();  

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
    }    
}
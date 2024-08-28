#include "game_state.h"
#include "game_tools.h"

#include "core/render_instance.h"
#include "core/save.h"

#include <vector>

using std::vector;

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

	resetRound();
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

void Game::resetRound() {
	state.round ++;
	state.timer = timerMax * 60;
	state.judge = false;

	// Reset players to default states
	for(int i = 0; i < playerCount; i ++) 
		players[i].state = Player::State();

    // 2 Player Game
    if(playerCount == 2) {
        players[0].state.position = -75;
        players[1].state.position = 75; 

    // 4 Player Tag Game
    } else if(playerCount == 4) {
        players[0].state.position = -75;
        players[1].state.position = -100;
        players[2].state.position = 75;
        players[3].state.position = 100;
    }	
}

void Game::nextRound() {

	if(state.rWins == roundMax || state.lWins == roundMax) {
		state.gameOver = true;

	}else {
		resetRound();
	}
}

void Game::advanceFrame() {

	if(state.gameOver)
		return;

	state.timer --;

	if(state.timer > 0) {

		// Update players
		vector<Player> others;

		for(int i = 0; i < playerCount; i ++)
			others.push_back(players[i]);

		for(int i = 0; i < playerCount; i ++)
			players[i].advanceFrame(others);

		// Check alive status of teams
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

		setCamera(players, playerCount);

	// Round over
	}else {

		// Determine winner by whichever team has the most health remaining
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
		if(state.timer < -5 * 60)
			nextRound();
	}
}

void Game::draw() {
    drawStage(0);
    drawHealthBars(players, playerCount);  

    // Draw clock
    if(state.timer >= 0) {
	    sf::Text txt;
	    txt.setString(std::to_string(state.timer / 60));
	    txt.setFont(*g::save.getFont("Anton-Regular"));
	    txt.setCharacterSize(64);
	    txt.setFillColor(sf::Color::White);
	    txt.setPosition({g::video.getSize().x / 2 - txt.getLocalBounds().width / 2, 0});
	    g::video.draw(txt);    	
    }

    for(int i = 0; i < playerCount; i ++)
    	players[i].draw();  
}
#ifndef GAME_FIGHT_TOOLS_H
#define GAME_FIGHT_TOOLS_H

#include "core/player.h"

#include <vector>

void setCamera(std::vector<Player> players);
void setCamera(Player* players, int count);

void drawHealthBars(std::vector<Player> players);
void drawHealthBars(Player* players, int count);
void drawRoundTokens(int lWin, int rWin, int winMax);

void drawStage(int index);

#endif
#include "move.h"

int Move::toCategory(int move) {

	if(move >= Move::Custom00 && move < Move::Total)
		return MoveCategory::Custom;

	return move;
}
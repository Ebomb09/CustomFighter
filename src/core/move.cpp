#include "move.h"

std::vector<int> Move::getValidCategories(int move) {
	std::vector<int> categories;

	if(move < 0) {
		// Return nothing, not a valid move
		return categories;

	}else if(move >= Move::Custom00 && move < Move::Total) {
		categories.push_back(MoveCategory::Normal);
		categories.push_back(MoveCategory::CommandNormal);	
		categories.push_back(MoveCategory::Special);
		categories.push_back(MoveCategory::Super);
		categories.push_back(MoveCategory::Grab);	

		categories.push_back(MoveCategory::AirNormal);
		categories.push_back(MoveCategory::AirCommandNormal);	
		categories.push_back(MoveCategory::AirSpecial);
		categories.push_back(MoveCategory::AirSuper);
		categories.push_back(MoveCategory::AirGrab);	

	}else {
		categories.push_back(move);
	}
	return categories;
}
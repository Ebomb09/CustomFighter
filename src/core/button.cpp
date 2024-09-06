#include "button.h"
#include "input_interpreter.h"

#include <fstream>
#include <json.hpp>

void Button::Config::loadFromFile(std::filesystem::path path) {
	std::fstream file(path, std::fstream::in);

	if(!file.good()) {
		file.close();
		return;
	}

	nlohmann::json json = nlohmann::json::parse(file);
	file.close();

	if(json["index"].is_number_integer()) 	index	= json["index"];
	if(json["Up"].is_number_integer()) 		Up 		= json["Up"];
	if(json["Down"].is_number_integer())	Down 	= json["Down"];
	if(json["Left"].is_number_integer())	Left 	= json["Left"];
	if(json["Right"].is_number_integer())	Right 	= json["Right"];
	if(json["A"].is_number_integer())		A 		= json["A"];
	if(json["B"].is_number_integer())		B 		= json["B"];
	if(json["C"].is_number_integer())		C 		= json["C"];
	if(json["D"].is_number_integer())		D 		= json["D"];
	if(json["Taunt"].is_number_integer())	Taunt 	= json["Taunt"];
	return;
}

void Button::Config::saveToFile(std::filesystem::path path) {
	std::fstream file(path, std::fstream::out | std::fstream::trunc);

	nlohmann::json json;
	json["index"]	= index;
	json["Up"] 		= Up;
	json["Down"] 	= Down; 
	json["Left"] 	= Left; 
	json["Right"] 	= Right;
	json["A"] 		= A; 	
	json["B"] 		= B; 	
	json["C"] 		= C; 	
	json["D"] 		= D; 	
	json["Taunt"] 	= Taunt;

	file << json;
	file.close();
}
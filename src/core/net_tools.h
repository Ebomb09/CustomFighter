#ifndef GAME_NET_TOOLS_H
#define GAME_NET_TOOLS_H

#include <string>
#include <vector>
#include <string>
#include <json.hpp>

nlohmann::json runQuery(std::string server, nlohmann::json postFields);
bool verify_UDP_hole_punch(int localPort, std::vector<std::string> remotes);

#endif
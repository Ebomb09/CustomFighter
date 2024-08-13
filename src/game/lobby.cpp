#include "lobby.h"

#include "core/save.h"
#include "core/render_instance.h"
#include "core/input_interpreter.h"
#include "core/net_tools.h"

nlohmann::json listRoom(){
    nlohmann::json postFields;
    postFields["reason"] = "list-room";

    return runQuery(g::save.serverList[0], postFields);
}

nlohmann::json createRoom(string password=""){
    nlohmann::json postFields;
    postFields["reason"] = "create-room";
    postFields["password"] = password;
    
    return runQuery(g::save.serverList[0], postFields);
}

nlohmann::json joinRoom(int code, string password, int slot, string remote, Player::Config config){
    nlohmann::json postFields;
    postFields["reason"] = "join-room";
    postFields["code"] = code;
    postFields["password"] = password;
    postFields["slot"] = slot;
    postFields["remote"] = remote;
    postFields["config"] = config.saveToText();

    return runQuery(g::save.serverList[0], postFields);
}

nlohmann::json leaveRoom(int code, string remote) {
    nlohmann::json postFields;
    postFields["reason"] = "leave-room";
    postFields["code"] = code;
    postFields["remote"] = remote;

    return runQuery(g::save.serverList[0], postFields);
}

nlohmann::json statusRoom(int code, string remote){
    nlohmann::json postFields;
    postFields["reason"] = "status-room";
    postFields["code"] = code;
    postFields["remote"] = remote;

    return runQuery(g::save.serverList[0], postFields);
}

bool Lobby::run(Room& room) {
    nlohmann::json roomList;

    while (g::video.window.isOpen()) {
        g::input.prepEvents();

        sf::Event event;

        while(g::video.window.pollEvent(event)){
            ImGui::SFML::ProcessEvent(event);

            if(!ImGui::GetIO().WantCaptureKeyboard && !ImGui::GetIO().WantCaptureMouse)
                g::input.processEvent(event);
        }

        if(g::input.windowClose)
            g::video.window.close();

        ImGui::SFML::Update(g::video.window, g::video.clock.restart());

        g::video.window.clear();

        ImGui::Begin("Browser");

        // Room Browser
        if(room.code == -1) {

            // List all rooms
            if(ImGui::BeginTable("Lobbies", 4)){

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);            
                ImGui::TableHeader("Code");
                ImGui::TableSetColumnIndex(1);               
                ImGui::TableHeader("Players");
                ImGui::TableSetColumnIndex(2);               
                ImGui::TableHeader("Password");

                for(int i = 0; i < roomList.size(); i ++) {
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    int code = roomList[i].value("code", 0);
                    ImGui::Text("%d", code);

                    ImGui::TableSetColumnIndex(1);
                    int players = roomList[i].value("player_count", 0);          
                    ImGui::Text("%d/4", players);
                    
                    // Password
                    ImGui::TableSetColumnIndex(2);
                    string password = roomList[i].value("password", " ");
                    ImGui::InputText(("##Password" + std::to_string(i)).c_str(), &password);
                    roomList[i]["password"] = password;

                    // Attempt room
                    ImGui::TableSetColumnIndex(3);  
                    if(ImGui::Button(("Join##" + std::to_string(i)).c_str()) ) {

                        if(joinRoom(roomList[i].value("code", 0), "", -1, g::save.getNetworkAddress(), g::save.playerConfig[0])["response"] == "OK")
                            room.code = roomList[i]["code"];
                    }
                }
                ImGui::EndTable();
            }

            // Refresh all rooms
            if(ImGui::Button("Refresh")) {
                nlohmann::json msg = listRoom();
      
                if(msg["response"] == "OK") {
                    roomList = msg["rooms"];
                }
            }

            // Create room
            if(ImGui::Button("Create")) {
                nlohmann::json msg = createRoom("");

                if(msg["response"] == "OK" && msg["code"].is_number_integer()) {

                    if(joinRoom(msg["code"], "", -1, g::save.getNetworkAddress(), g::save.playerConfig[0])["response"] == "OK")
                        room.code = msg["code"];
                }
            }

        // Room viewer
        }else {
            ImGui::Text("Room Code: %d", room.code);

            if(ImGui::Button("Leave")) {
                leaveRoom(room.code, g::save.getNetworkAddress());
                room.code = -1;
            }

            // Refresh all players within the room
            room.refresh --;
            if(room.refresh < 0) {
                room.refresh = 150;

                nlohmann::json msg = statusRoom(room.code, g::save.getNetworkAddress());

                if(msg.value("response", "") == "OK") {
                    room.remotes.clear();
                    room.configs.clear();

                    for(int i = 0; i < msg["player"].size(); i ++) {
                        string remote = msg["player"][i].value("remote", "");
                        Player::Config cfg;
                        cfg.loadFromText(msg["player"][i].value("config", ""));

                        if(remote != "") {
                            room.remotes.push_back(remote);
                            room.configs.push_back(cfg);
                        }
                    }
                }
            }

            // Show all players in the room
            if(ImGui::BeginTable("Players", 2)) {

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);            
                ImGui::TableHeader("Remote");
                ImGui::TableSetColumnIndex(1);               
                ImGui::TableHeader("Config");

                for(int i = 0; i < room.remotes.size(); i ++) {
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%s", room.remotes[i].c_str());

                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%s", room.configs[i].saveToText().c_str());                    
                }
                ImGui::EndTable();
            }

            // Found the wanted players
            if(room.remotes.size() == 2) 
                return true;
        }

        ImGui::End();

        ImGui::SFML::Render(g::video.window);
        g::video.window.display();
    }
	return false;
}
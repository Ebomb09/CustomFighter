#include "net_tools.h"

#include <SFML/System.hpp>
#include <sockpp/udp_socket.h>
#include <curl/curl.h>

using std::vector;
using std::string;
using nlohmann::json;

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    string* write_data = (string*)userdata; 
    *write_data += string(ptr, nmemb);
    return nmemb;
}

json runQuery(string server, json postFields) {
    json json;

    CURL* curl = curl_easy_init();
    if(curl) {
        string write_data = "";
        curl_easy_setopt(curl, CURLOPT_URL, server.c_str());

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYSTATUS, false);

        string data = "data=" + postFields.dump();
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.size());

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &write_data);

        CURLcode res = curl_easy_perform(curl);
        if(res == CURLE_OK) {

            try{
                json = nlohmann::json::parse(write_data);

            }catch(nlohmann::json::exception e) {
                json = nlohmann::json();
                json["response"] = "BAD";
            }
        }
        curl_easy_cleanup(curl);
    }
    return json;
}

bool verify_UDP_hole_punch(int localPort, vector<string> remotes) {
    sockpp::initialize();

    // Verify for each player
    vector<bool> holePunch;
    holePunch.resize(remotes.size(), false);

    // Initialize the test socket
    sockpp::udp_socket sock;
    sock.bind(sockpp::inet_address(localPort));
    sock.set_non_blocking(true);

    if(!sock)
        std::cerr << "bind: " << sock.last_error_str() << "\n";

    for(int i = 0; i < remotes.size(); i ++) {

        if(remotes[i] == "localhost" || remotes[i] == "") {
            holePunch[i] = true;
            continue;
        }

        // Get Address components
        char ip[46];
        int octet[4], port;
        std::sscanf(remotes[i].c_str(), "%d.%d.%d.%d:%d", &octet[0], &octet[1], &octet[2], &octet[3], &port);
        std::sprintf(ip, "%d.%d.%d.%d", octet[0], octet[1], octet[2], octet[3]);

        sockpp::inet_address peer = sockpp::inet_address(ip, port);

        for(int j = 0; j < 10; j ++) {
            int size;

            // Try sending echo
            size = sock.send_to("ECHO", peer);

            if(size == -1)
                std::cerr << "send_to: " << sock.last_error_str() << "\n";

            // Wait for real-time response
            sf::sleep(sf::seconds(0.5f));


            if(size != -1) {

                // Try receiving message
                char msg[1024] = "";
                sockpp::inet_address from;
                size = sock.recv_from(msg, sizeof(msg), &from);

                if(size == -1) {
                    std::cerr << "recv: " << sock.last_error_str() << "\n";

                }else if(from.to_string() != remotes[i]) {
                    std::cerr << "remote: Invalid message sender\n";

                }else {
                    std::cout << "msg: " << msg << "\n";
                    std::cout << "from: " << from.to_string() << "\n";
                    holePunch[i] = true;
                    break;
                }
            }
        }
    }
    sock.close();

    // Return if each verify worked
    for(int i = 0; i < holePunch.size(); i ++) {

        if(!holePunch[i])
            return false;
    }
    return true;
}
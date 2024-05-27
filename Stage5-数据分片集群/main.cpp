#include"Server.h"
#include<iostream>
#include"HashRing.h"

int main(const int argc, char *argv[]) {
    if (argc < 2) {
        std::cout<<"Default port"<<std::endl;
        Server server;
        server.run();
    } else {
        const std::string input = argv[1];
        try {
            const size_t port = std::stoul(input);
            if (IsPortValid(port)) {
                Server server(port);
                server.run();
            } else {
                std::cout << "Invalid port: " << port << std::endl;
            }
        } catch (const std::invalid_argument &error) {
            std::cout << error.what() << std::endl << "Invalid: " << input << std::endl;
        }catch (const std::out_of_range &error) {
            std::cout << error.what() << std::endl << "Out of range: " << input << std::endl;
        }
    }

    return 0;
}

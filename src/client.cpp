#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        std::cerr << "Failed to create socket" << std::endl;
        return 2;
    }

    sockaddr_in server_address{};
    socklen_t server_size = sizeof(server_address);

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(54000);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(client_socket, reinterpret_cast<sockaddr *>(&server_address), server_size) == -1) {
        std::cerr << "Failed to connect to server" << std::endl;
        return 2;
    }

    char buffer[4096];
    while (true) {
        std::string input;
        std::getline(std::cin, input);
        send(client_socket, input.c_str(), input.size(), 0);

        memset(buffer, 0, sizeof(buffer));
        long bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);

        if (bytes_received <= 0) {
            std::cerr << "Receive failed" << std::endl;
            break;
        }
        std::cout << "Server: " << buffer << std::endl;
    }

    close(client_socket);
    return 0;
}
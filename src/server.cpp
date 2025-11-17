#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int main() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }

    sockaddr_in server_address{};
    socklen_t server_size = sizeof(server_address);

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(54000);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, reinterpret_cast<sockaddr *>(&server_address), server_size) == -1) {
        std::cerr << "Bind failed" << std::endl;
        close(server_socket);
        return 1;
    }

    if (listen(server_socket, 5) == -1) {
        std::cerr << "Listen failed" << std::endl;
        close(server_socket);
        return 1;
    }
    std::cout << "Server listening on port 54000" << std::endl;

    sockaddr_in client_address{};
    socklen_t client_size = sizeof(client_address);
    int client_socket = accept(server_socket, reinterpret_cast<sockaddr *>(&client_address), &client_size);

    if (client_socket == -1) {
        std::cerr << "Accept failed" << std::endl;
        close(server_socket);
        return 1;
    }
    std::cout << "Client connected" << std::endl;

    char buffer[4096];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        long bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            std::cerr << "Receive failed" << std::endl;
            break;
        }
        std::cout << "Client: " << buffer << std::endl;
    }

    close(server_socket);
    close(client_socket);
    return 0;
}
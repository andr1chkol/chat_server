#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

std::vector<int> clients;
std::mutex clients_mutex;

void handleClient(int client_socket);

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

    while (true) {
        int client_socket = accept(server_socket, reinterpret_cast<sockaddr *>(&client_address), &client_size);

        if (client_socket == -1) {
            std::cerr << "Accept failed" << std::endl;
            close(server_socket);
            return 1;
        }
        std::cout << "Client connected" << std::endl;

        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients.push_back(client_socket);
        }

        std::thread clientThread(handleClient, client_socket);
        clientThread.detach();
    }

    close(server_socket);
    return 0;
}

void handleClient(int client_socket) {
    char buffer[4096];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        long bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            clients.erase(
                std::remove(clients.begin(), clients.end(), client_socket),
                clients.end()
            );
            close(client_socket);
            std::cerr << "Receive failed" << std::endl;
            return;
        }
        std::cout << "Client: " << buffer << std::endl;
        std::lock_guard<std::mutex> lock(clients_mutex);
        for (int client : clients) {
            if (client == client_socket) {
                continue;
            }
            send(client, buffer, bytes_received, 0);
        }
    }
}
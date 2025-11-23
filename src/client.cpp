#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <atomic>
#include <string>

std::atomic<bool> running(true);
void listenToServer(int client_socket);
void inputToServer(int client_socket);

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

    std::string name;
    std::cout << "Enter your name: ";
    getline(std::cin, name);
    send(client_socket, name.c_str(), name.size()+1, 0);

    std::thread Listen(listenToServer, client_socket);
    std::thread Input(inputToServer, client_socket);

    Input.join();
    running = false;
    shutdown(client_socket, SHUT_RDWR);
    Listen.join();

    close(client_socket);
    return 0;
}

void listenToServer(int client_socket) {
    char buffer[4096];
    while (running) {
        memset(buffer, 0, sizeof(buffer));
        long bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);

        if (bytes_received <= 0) {
            std::cerr << "Server Disconnected" << std::endl;
            running = false;
            return;
        }
        std::cout << buffer << std::endl;
    }
}

void inputToServer(int client_socket) {
    while (running) {
        std::string input;
        getline(std::cin, input);
        if (input == "/exit") {
            running = false;
            send(client_socket, input.c_str(), input.size()+1, 0);
            return;
        }

        send(client_socket, input.c_str(), input.size()+1, 0);
    }
}
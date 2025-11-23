#include <iostream>
#include <vector>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <ctime>

std::vector<int> clients;
std::mutex clients_mutex;
std::unordered_map<int, std::string> usernames;

std::string getTime();
void clientSession(int client_socket);

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

        std::thread clientThread(clientSession, client_socket);
        clientThread.detach();
    }
    close(server_socket);
    return 0;
}

void clientSession(int client_socket) {
    char buffer[4096];
    char name[64];

    long name_received = recv(client_socket, name, sizeof(name), 0);
    if (name_received == -1) {
        std::cerr << "Receive name failed" << std::endl;
        close(client_socket);
        return;
    }
    name[name_received] = '\0';

    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        usernames[client_socket] = name;
    }

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        long bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);

        if (bytes_received <= 0) {
            {
                std::lock_guard<std::mutex> lock(clients_mutex);
                clients.erase(
                    std::remove(clients.begin(), clients.end(), client_socket),
                    clients.end()

                );
                usernames.erase(client_socket);
            }
            close(client_socket);
            std::cerr << "Receive failed" << std::endl;
            return;
        }

        std::string input = buffer;
        // exit command
        if (input == "/exit") {
            std::string username = usernames[client_socket];
            std::string message = "[" + username + "] left the chat";
            {
                std::lock_guard<std::mutex> lock(clients_mutex);
                for (int client : clients) {
                    if (client != client_socket) {
                        send(client, message.c_str(), message.size(), 0);
                    }
                }
            }

            {
                std::lock_guard<std::mutex> lock(clients_mutex);
                clients.erase(
                    std::remove(clients.begin(), clients.end(), client_socket),
                    clients.end()

                );
                usernames.erase(client_socket);
            }
            close(client_socket);
            std::cout << "Client exit" << std::endl;
            return;
        }

        if (input.rfind("/name ", 0) == 0) {
            std::string newUsername = input.substr(6);
            std::string oldUsername = usernames[client_socket];
            std::string message = "[" + oldUsername + "] has changed username to [" + newUsername + "]" ;
            {
                std::lock_guard<std::mutex> lock(clients_mutex);
                for (int client : clients) {
                    if (client != client_socket) {
                        send(client, message.c_str(), message.size(), 0);
                    }
                }
                usernames[client_socket] = newUsername;
            }
            continue;
        }

        if (input == "/users") {
            std::string users = "Users: \n";
            {
                std::lock_guard<std::mutex> lock(clients_mutex);
                for (int client : clients) {
                    users += "- " + usernames[client] + "\n";
                }
            }
            send (client_socket, users.c_str(), users.size() + 1, 0);
            continue;
        }

        { // sending messages
            std::lock_guard<std::mutex> lock(clients_mutex);
            for (int client : clients) {
                if (client == client_socket) {
                    continue;
                }
                std::string username = usernames[client_socket];
                std::string message = "[" + username + " | " + getTime() +"]: " + buffer;
                send(client, message.c_str(), message.size(), 0);
            }
        }
    }
}

std::string getTime() {
    time_t now = time(nullptr);
    tm *ltm = localtime(&now);

    char buf[16];
    sprintf(buf, "%02d:%02d", ltm->tm_hour, ltm->tm_min);

    return {buf};
}
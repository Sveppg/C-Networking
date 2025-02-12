#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

#define BUF_SIZE 1024

std::vector<int> clients;
std::mutex clients_mutex;

/*
*   Send message to all clients active -> Broadcasting
*/

void send_to_all_clients(const std::string& message) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (int sock : clients) {
        send(sock, message.c_str(), message.size(), 0);
    }
}

/*
*   Handle Client connection and messages -> mutex lock & unlock
*/

void handle_client(int client_socket) {
    char buffer[BUF_SIZE];

    while (true) {
        ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            std::cout << "Client getrennt.\n";
            break;
        }

        buffer[bytes_received] = '\0'; // Null-Terminator setzen
        std::string message = buffer;

        // **Debug: Nachricht ausgeben**
        std::cout << "Client: " << message << std::endl;

        // Broadcast an alle anderen Clients
        std::lock_guard<std::mutex> lock(clients_mutex);
        for (int sock : clients) {
            if (sock != client_socket) {
                send(sock, message.c_str(), message.size(), 0);
            }
        }
    }

    close(client_socket);
    std::lock_guard<std::mutex> lock(clients_mutex);
    clients.erase(std::remove(clients.begin(), clients.end(), client_socket), clients.end());
}

/*
*   Main for Error-Checking, Port re-using and threading
*/

int main() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        std::cerr << "Error creating socket." << std::endl;
        return EXIT_FAILURE;
    }

    // Option für direktes Wiederverwenden des Ports setzen
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8081);  // TCP Port 8081

    if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "Error binding socket.\n";
        return -1;
    }

    if (listen(server_socket, 10) == -1) {
        std::cerr << "Error listening on socket.\n";
        return -1;
    }

    std::cout << "Server is listening on port " << ntohs(server_addr.sin_port) << "...\n";

  //Starte einen neuen Thread für Server-Eingabe
    std::thread([]() {
        std::string server_message;
        while (true) {
            std::getline(std::cin, server_message);
            server_message = server_message + "\n";
            send_to_all_clients(server_message);
        }
    }).detach();

    while (true) {
        sockaddr_in client_addr = {};
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_len);
        if (client_socket == -1) {
            std::cerr << "Error accepting connection.\n";
            continue;
        }

        std::cout << "Client connected.\n";
        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients.push_back(client_socket);
        }

        std::thread(handle_client, client_socket).detach();
    }

    close(server_socket);
    return 0;
}

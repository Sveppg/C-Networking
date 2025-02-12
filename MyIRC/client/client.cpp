#include <iostream>
#include <thread>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>

#define BUF_SIZE 1024

void receive_messages(int socket) {
    char buffer[BUF_SIZE];
    while (true) {
        ssize_t bytes_received = recv(socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) break;

        buffer[bytes_received] = '\0';
        std::cout << "Server: " << buffer << "\n";
    }
}

int main() {
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        std::cerr << "Error creating socket.\n";
        return -1;
    }

    sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8081);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (connect(client_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        if(connect(client_socket, (sockaddr*)&server_addr, sizeof(server_addr)== -1)){
            std::cerr << "Error connecting to server.\n";
            return -1;
        }
    }

    std::cout << "Connected to the server.\n";

    // Starte den Empfänger-Thread
    std::thread(receive_messages, client_socket).detach();

    // Automatische Nachricht senden
    std::string auto_message = "Hallo Server, ich bin verbunden!\n";
    send(client_socket, auto_message.c_str(), auto_message.size(), 0);
    std::cout << "Gesendet: " << auto_message;

    // Normale Chat-Schleife
    std::string message;
    while (true) {
        std::getline(std::cin, message);
        message += "\n";
        send(client_socket, message.c_str(), message.size(), 0);
    }

    close(client_socket);
    return 0;
}

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <winsock2.h>
#include <vector>
#include <thread>
#include <algorithm> // For std::remove

#pragma comment(lib, "ws2_32.lib")

std::vector<SOCKET> clients;

void BroadcastMessage(const std::string& message, SOCKET sender) {
    for (SOCKET client : clients) {
        if (client != sender) { // Don't send the message back to the sender
            send(client, message.c_str(), message.length(), 0);
        }
    }
}

void ClientHandler(SOCKET clientSocket) {
    char buffer[1024];
    while (true) {
        ZeroMemory(buffer, sizeof(buffer)); // Clear buffer before receiving

        // Receive message from client
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0'; // Null-terminate the received message
            std::string message(buffer);
            std::cout << "Server received: " << message << std::endl;

            // Broadcast the message to all other clients
            BroadcastMessage(message, clientSocket);
        } else if (bytesReceived == 0) {
            std::cout << "Client disconnected." << std::endl;
            break; // Client disconnected gracefully
        } else {
            std::cerr << "Error in receiving data: " << WSAGetLastError() << std::endl;
            break; // Error occurred
        }
    }

    // Clean up client socket
    closesocket(clientSocket);
    clients.erase(std::remove(clients.begin(), clients.end(), clientSocket), clients.end());
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return -1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create server socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return -1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(54000);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Binding failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return -1;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listening failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return -1;
    }

    std::cout << "Server is listening on port 54000" << std::endl;

    while (true) {
        sockaddr_in clientAddr;
        int clientSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);

        if (clientSocket != INVALID_SOCKET) {
            clients.push_back(clientSocket); // Add client to the list
            std::cout << "New client connected!" << std::endl;

            std::thread(ClientHandler, clientSocket).detach(); // Start a new thread for this client
        } else {
            std::cerr << "Invalid client socket: " << WSAGetLastError() << std::endl;
        }
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}

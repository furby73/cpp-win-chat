#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <winsock2.h>
#include <windows.h>
#include <commctrl.h>
#include <thread>
#include <string>
#include <mutex>

#pragma comment(lib, "ws2_32.lib")

// Function prototypes
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void ReceiveMessages(SOCKET sock);
void UpdateChatWindow(const std::string& message);

// Global variables
HWND hChatArea;
HWND hInputField;
SOCKET clientSocket;
std::mutex chatMutex;

int main() {
    // Initialize Winsock
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // Create a socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        MessageBox(NULL, "Failed to create socket.", "Error", MB_OK | MB_ICONERROR);
        return -1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(54000);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to server
    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        MessageBox(NULL, "Failed to connect to server.", "Error", MB_OK | MB_ICONERROR);
        closesocket(clientSocket);
        return -1;
    }

    // Create main window
    const char CLASS_NAME[] = "ChatClient";
    WNDCLASS wc = { };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "Chat Client",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 300,
        NULL,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    // Start a thread to handle incoming messages
    std::thread receiveThread(ReceiveMessages, clientSocket);
    receiveThread.detach();

    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}

// Window Procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HINSTANCE hInst = GetModuleHandle(NULL);
    switch (uMsg) {
    case WM_CREATE: {
        // Create chat area
        hChatArea = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY,
            10, 10, 360, 200, hwnd, NULL, hInst, NULL);

        // Create input field
        hInputField = CreateWindowEx(0, "EDIT", "",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            10, 220, 280, 25, hwnd, NULL, hInst, NULL);

        // Create send button
        CreateWindow("BUTTON", "Send",
            WS_CHILD | WS_VISIBLE,
            300, 220, 70, 25, hwnd, (HMENU)1, hInst, NULL);
    } break;

    case WM_COMMAND:
        if (LOWORD(wParam) == 1) { // Send button clicked
            char buffer[1024];
            GetWindowText(hInputField, buffer, sizeof(buffer));

            if (strlen(buffer) > 0) {
                send(clientSocket, buffer, strlen(buffer), 0);
                UpdateChatWindow("You: " + std::string(buffer));
                SetWindowText(hInputField, ""); // Clear input field
            }
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

// Function to receive messages from the server
void ReceiveMessages(SOCKET sock) {
    char buffer[1024];
    while (true) {
        ZeroMemory(buffer, sizeof(buffer));
        int bytesReceived = recv(sock, buffer, sizeof(buffer), 0);

        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0'; // Null-terminate the received message
            UpdateChatWindow("Server: " + std::string(buffer));
        } else if (bytesReceived == 0) {
            UpdateChatWindow("Server closed the connection.");
            break;
        } else {
            UpdateChatWindow("Error receiving data.");
            break;
        }
    }
}

// Function to update the chat area
void UpdateChatWindow(const std::string& message) {
    chatMutex.lock();
    SendMessage(hChatArea, EM_SETSEL, -1, -1); // Move cursor to end
    SendMessage(hChatArea, EM_REPLACESEL, TRUE, (LPARAM)message.c_str());
    SendMessage(hChatArea, EM_REPLACESEL, TRUE, (LPARAM)"\r\n");
    chatMutex.unlock();
}

# CppWin Chat Application

A chat application built using C++ and WinSock for Windows. This project includes both the client and server components, allowing multiple clients to communicate through a centralized server.

## Table of Contents

- [Features](#features)
- [Prerequisites](#prerequisites)

## Features

- **Client-Server Architecture**: Multiple clients can connect to a single server.
- **Real-time Communication**: Send and receive messages in real-time.
- **GUI Interface**: The client features a simple graphical user interface using Windows API.
- **Threaded Server**: Server handles each client connection in a separate thread for efficient communication.

## Prerequisites

- **Operating System**: Windows
- **Compiler**: Visual Studio or any C++ compiler that supports WinSock.
- **Libraries**:
  - `winsock2.lib`
  - `ws2_32.lib`
  - `comctl32.lib` (for GUI components)

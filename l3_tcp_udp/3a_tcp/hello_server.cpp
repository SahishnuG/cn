// hello_server_win.cpp
// Usage: hello_server_win <port>
// Compile (MSVC): cl /EHsc hello_server_win.cpp ws2_32.lib
// Compile (MinGW): g++ hello_server.cpp -lws2_32 -o hello_server.exe

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <port>\n";
        return 1;
    }
    const char* port = argv[1];

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        std::cerr << "WSAStartup failed\n"; return 1;
    }

    addrinfo hints = {}, *res = nullptr;
    hints.ai_family = AF_INET;            // IPv4
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(nullptr, port, &hints, &res) != 0) {
        std::cerr << "getaddrinfo failed\n"; WSACleanup(); return 1;
    }

    SOCKET listenSock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (listenSock == INVALID_SOCKET) {
        std::cerr << "socket failed\n"; freeaddrinfo(res); WSACleanup(); return 1;
    }

    // allow reuse
    BOOL opt = TRUE;
    setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    if (bind(listenSock, res->ai_addr, (int)res->ai_addrlen) == SOCKET_ERROR) {
        std::cerr << "bind failed\n"; freeaddrinfo(res); closesocket(listenSock); WSACleanup(); return 1;
    }
    freeaddrinfo(res);

    if (listen(listenSock, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "listen failed\n"; closesocket(listenSock); WSACleanup(); return 1;
    }

    std::cout << "Hello server listening on port " << port << "...\n";

    while (true) {
        sockaddr_in clientAddr;
        int addrLen = sizeof(clientAddr);
        SOCKET conn = accept(listenSock, (sockaddr*)&clientAddr, &addrLen);
        if (conn == INVALID_SOCKET) {
            std::cerr << "accept failed\n"; continue;
        }

        const char* srv_msg = "Server: Hello client!\n";
        send(conn, srv_msg, (int)strlen(srv_msg), 0);

        char buf[512];
        int n = recv(conn, buf, (int)sizeof(buf) - 1, 0);
        if (n > 0) {
            buf[n] = '\0';
            std::cout << "Received from client: " << buf;
        } else {
            std::cout << "No greeting received or client closed connection\n";
        }

        closesocket(conn);
        std::cout << "Connection closed. Waiting for next client...\n";
    }

    closesocket(listenSock);
    WSACleanup();
    return 0;
}

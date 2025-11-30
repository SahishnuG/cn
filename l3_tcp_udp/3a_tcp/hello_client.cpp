// hello_client_win.cpp
// Usage: hello_client_win <server_ip_or_hostname> <port>
// Compile (MSVC): cl /EHsc hello_client_win.cpp ws2_32.lib
// Compile (MinGW): g++ hello_client.cpp -lws2_32 -o hello_client.exe

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <server> <port>\n";
        return 1;
    }
    const char* server = argv[1];
    const char* port = argv[2];

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) { std::cerr << "WSAStartup failed\n"; return 1; }

    addrinfo hints = {}, *res = nullptr;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(server, port, &hints, &res) != 0) { std::cerr << "getaddrinfo failed\n"; WSACleanup(); return 1; }

    SOCKET sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock == INVALID_SOCKET) { std::cerr << "socket failed\n"; freeaddrinfo(res); WSACleanup(); return 1; }

    if (connect(sock, res->ai_addr, (int)res->ai_addrlen) == SOCKET_ERROR) {
        std::cerr << "connect failed\n"; freeaddrinfo(res); closesocket(sock); WSACleanup(); return 1;
    }
    freeaddrinfo(res);

    char buf[512];
    int n = recv(sock, buf, (int)sizeof(buf) - 1, 0);
    if (n > 0) {
        buf[n] = '\0';
        std::cout << "Received from server: " << buf;
    }

    const char* msg = "Client: Hello server!\n";
    send(sock, msg, (int)strlen(msg), 0);

    closesocket(sock);
    WSACleanup();
    return 0;
}

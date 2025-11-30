// udp_hello_server.cpp
// Usage: hello_server <port>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#pragma comment(lib, "ws2_32.lib")

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <port>\n";
        return 1;
    }

    const char* port = argv[1];

    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    sockaddr_in serverAddr {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(port));
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(sock, (sockaddr*)&serverAddr, sizeof(serverAddr));
    std::cout << "UDP Hello Server listening on port " << port << "...\n";

    // Receive from anyone
    sockaddr_in clientAddr {};
    int len = sizeof(clientAddr);
    char buffer[512];

    int n = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                     (sockaddr*)&clientAddr, &len);
    buffer[n] = '\0';

    std::cout << "Client says: " << buffer << "\n";

    // Send reply
    const char* hello = "Hello from Server!";
    sendto(sock, hello, (int)strlen(hello), 0,
           (sockaddr*)&clientAddr, len);

    closesocket(sock);
    WSACleanup();
    return 0;
}

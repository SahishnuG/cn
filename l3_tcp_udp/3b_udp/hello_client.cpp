// udp_hello_client.cpp
// Usage: hello_client <server_ip> <port>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#pragma comment(lib, "ws2_32.lib")

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <server_ip> <port>\n";
        return 1;
    }

    const char* serverIP = argv[1];
    const char* port = argv[2];

    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    sockaddr_in serverAddr {};
    serverAddr.sin_family = AF_INET;
    inet_pton(AF_INET, serverIP, &serverAddr.sin_addr);
    serverAddr.sin_port = htons(atoi(port));

    const char* msg = "Hello from Client!";
    sendto(sock, msg, (int)strlen(msg), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));

    // Receive reply
    char buffer[512];
    int len = sizeof(serverAddr);
    int n = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                     (sockaddr*)&serverAddr, &len);
    buffer[n] = '\0';

    std::cout << "Server responds: " << buffer << "\n";

    closesocket(sock);
    WSACleanup();
    return 0;
}

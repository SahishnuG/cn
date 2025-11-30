// udp_chat.cpp
// Usage: udp_chat <my_port> <peer_ip> <peer_port>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>
#include <string>
#pragma comment(lib, "ws2_32.lib")

void receive_thread(SOCKET sock) {
    sockaddr_in sender {};
    int slen = sizeof(sender);
    char buf[512];

    while (true) {
        int n = recvfrom(sock, buf, sizeof(buf)-1, 0, (sockaddr*)&sender, &slen);
        if (n > 0) {
            buf[n] = '\0';
            std::cout << "\nPeer: " << buf << "\n> ";
        }
    }
}

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cout << "Usage: udp_chat <my_port> <peer_ip> <peer_port>\n";
        return 1;
    }

    const char* my_port = argv[1];
    const char* peer_ip = argv[2];
    const char* peer_port = argv[3];

    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    // Bind to local port
    sockaddr_in myAddr {};
    myAddr.sin_family = AF_INET;
    myAddr.sin_addr.s_addr = INADDR_ANY;
    myAddr.sin_port = htons(atoi(my_port));
    bind(sock, (sockaddr*)&myAddr, sizeof(myAddr));

    // Peer address
    sockaddr_in peerAddr {};
    peerAddr.sin_family = AF_INET;
    inet_pton(AF_INET, peer_ip, &peerAddr.sin_addr);
    peerAddr.sin_port = htons(atoi(peer_port));

    std::cout << "UDP Peer-to-Peer Chat Started\n";
    std::cout << "Type messages and press Enter\n";

    // Background thread receives messages
    std::thread recvThread(receive_thread, sock);
    recvThread.detach();

    // Main thread sends messages
    while (true) {
        std::string msg;
        std::cout << "> ";
        std::getline(std::cin, msg);

        sendto(sock, msg.c_str(), (int)msg.size(), 0,
               (sockaddr*)&peerAddr, sizeof(peerAddr));
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}

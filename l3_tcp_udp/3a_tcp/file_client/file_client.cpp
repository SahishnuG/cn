// file_client_win.cpp
// Usage:
//   file_client_win <server> <port> GET <filename>
//   file_client_win <server> <port> PUT <filename>
// Compile (MSVC): cl /EHsc file_client_win.cpp ws2_32.lib
// Compile (MinGW): g++ file_client.cpp -lws2_32 -o file_client.exe

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>

#pragma comment(lib, "Ws2_32.lib")

int recv_line(SOCKET s, std::string &out) {
    out.clear();
    char c;
    while (true) {
        int r = recv(s, &c, 1, 0);
        if (r <= 0) return r;
        if (c == '\n') break;
        out.push_back(c);
    }
    return (int)out.size();
}

int recv_nbytes(SOCKET s, char* buf, int n) {
    int got = 0;
    while (got < n) {
        int r = recv(s, buf + got, n - got, 0);
        if (r <= 0) return r;
        got += r;
    }
    return got;
}

int send_all(SOCKET s, const char* buf, int len) {
    int sent = 0;
    while (sent < len) {
        int r = send(s, buf + sent, len - sent, 0);
        if (r == SOCKET_ERROR) return -1;
        sent += r;
    }
    return sent;
}

int main(int argc, char** argv) {
    if (argc != 5) { std::cerr << "Usage: " << argv[0] << " <server> <port> <GET|PUT> <filename>\n"; return 1; }
    const char* server = argv[1];
    const char* port = argv[2];
    std::string cmd = argv[3];
    std::string fname = argv[4];

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) { std::cerr << "WSAStartup failed\n"; return 1; }

    addrinfo hints = {}, *res = nullptr;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(server, port, &hints, &res) != 0) { std::cerr << "getaddrinfo failed\n"; WSACleanup(); return 1; }

    SOCKET sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock == INVALID_SOCKET) { std::cerr << "socket failed\n"; freeaddrinfo(res); WSACleanup(); return 1; }

    if (connect(sock, res->ai_addr, (int)res->ai_addrlen) == SOCKET_ERROR) { std::cerr << "connect failed\n"; freeaddrinfo(res); closesocket(sock); WSACleanup(); return 1; }
    freeaddrinfo(res);

    if (cmd == "GET") {
        std::string line = "GET " + fname + "\n";
        send_all(sock, line.c_str(), (int)line.size());

        std::string reply;
        if (recv_line(sock, reply) <= 0) { std::cerr << "No reply\n"; closesocket(sock); WSACleanup(); return 1; }
        if (reply == "ERR") {
            std::cerr << "Server reports error (file not found?)\n";
            closesocket(sock); WSACleanup(); return 1;
        } else if (reply == "OK") {
            if (recv_line(sock, reply) <= 0) { std::cerr << "No size\n"; closesocket(sock); WSACleanup(); return 1; }
            uint64_t sz = std::stoull(reply);
            std::ofstream ofs(fname, std::ios::binary);
            if (!ofs) { std::cerr << "Cannot open local file for writing\n"; closesocket(sock); WSACleanup(); return 1; }
            const int BUF = 64*1024;
            std::vector<char> buffer(BUF);
            uint64_t remaining = sz;
            while (remaining > 0) {
                int toread = (int)std::min<uint64_t>(BUF, remaining);
                int got = recv_nbytes(sock, buffer.data(), toread);
                if (got <= 0) { std::cerr << "recv error\n"; break; }
                ofs.write(buffer.data(), got);
                remaining -= (uint64_t)got;
            }
            ofs.close();
            std::cout << "Received " << sz << " bytes into " << fname << "\n";
        } else {
            std::cerr << "Unexpected reply: " << reply << "\n";
        }
    } else if (cmd == "PUT") {
        std::ifstream ifs(fname, std::ios::binary | std::ios::ate);
        if (!ifs) { std::cerr << "Cannot open file to send\n"; closesocket(sock); WSACleanup(); return 1; }
        uint64_t sz = (uint64_t)ifs.tellg();
        ifs.seekg(0);

        std::string line = "PUT " + fname + "\n";
        send_all(sock, line.c_str(), (int)line.size());

        std::string reply;
        if (recv_line(sock, reply) <= 0) { std::cerr << "No reply\n"; closesocket(sock); WSACleanup(); return 1; }
        if (reply != "READY") { std::cerr << "Server not ready: " << reply << "\n"; closesocket(sock); WSACleanup(); return 1; }

        std::string sizestr = std::to_string(sz) + "\n";
        send_all(sock, sizestr.c_str(), (int)sizestr.size());

        const int BUF = 64*1024;
        std::vector<char> buffer(BUF);
        while (ifs) {
            ifs.read(buffer.data(), BUF);
            std::streamsize r = ifs.gcount();
            if (r > 0) {
                if (send_all(sock, buffer.data(), (int)r) < 0) { std::cerr << "send error\n"; break; }
            }
        }
        if (recv_line(sock, reply) > 0) {
            std::cout << "Server reply: " << reply << "\n";
        } else {
            std::cerr << "No final reply from server\n";
        }
        std::cout << "Uploaded " << sz << " bytes from " << fname << "\n";
    } else {
        std::cerr << "Unknown command: " << cmd << " (use GET or PUT)\n";
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
// GET to download a file from server
// PUT to upload a file to server
// run client and server in different directories to actually see file transfer
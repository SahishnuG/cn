// file_server_win.cpp
// Usage: file_server_win <port>
// Compile (MSVC): cl /EHsc file_server_win.cpp ws2_32.lib
// Compile (MinGW): g++ -std=c++17 file_server_win.cpp -lws2_32 -o file_server_win.exe

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>
#include <sstream>

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
    if (argc != 2) { std::cerr << "Usage: " << argv[0] << " <port>\n"; return 1; }
    const char* port = argv[1];

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) { std::cerr << "WSAStartup failed\n"; return 1; }

    addrinfo hints = {}, *res = nullptr;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if (getaddrinfo(nullptr, port, &hints, &res) != 0) { std::cerr << "getaddrinfo failed\n"; WSACleanup(); return 1; }

    SOCKET listenSock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (listenSock == INVALID_SOCKET) { std::cerr << "socket failed\n"; freeaddrinfo(res); WSACleanup(); return 1; }

    BOOL opt = TRUE;
    setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    if (bind(listenSock, res->ai_addr, (int)res->ai_addrlen) == SOCKET_ERROR) {
        std::cerr << "bind failed\n"; freeaddrinfo(res); closesocket(listenSock); WSACleanup(); return 1;
    }
    freeaddrinfo(res);

    if (listen(listenSock, SOMAXCONN) == SOCKET_ERROR) { std::cerr << "listen failed\n"; closesocket(listenSock); WSACleanup(); return 1; }

    std::cout << "File server listening on port " << port << "...\n";

    while (true) {
        sockaddr_in clientAddr;
        int addrLen = sizeof(clientAddr);
        SOCKET conn = accept(listenSock, (sockaddr*)&clientAddr, &addrLen);
        if (conn == INVALID_SOCKET) { std::cerr << "accept failed\n"; continue; }
        std::cout << "Client connected\n";

        std::string line;
        if (recv_line(conn, line) <= 0) { closesocket(conn); continue; }
        std::string cmd, fname;
        {
            std::istringstream ss(line);
            ss >> cmd >> fname;
        }

        if (cmd == "GET") {
            std::ifstream ifs(fname, std::ios::binary | std::ios::ate);
            if (!ifs) {
                std::string err = "ERR\n";
                send_all(conn, err.c_str(), (int)err.size());
                std::cerr << "GET: file not found: " << fname << "\n";
            } else {
                std::streamsize sz = ifs.tellg();
                ifs.seekg(0);
                std::string ok = "OK\n" + std::to_string((uint64_t)sz) + "\n";
                send_all(conn, ok.c_str(), (int)ok.size());
                const int BUF = 64*1024;
                std::vector<char> buffer(BUF);
                while (ifs) {
                    ifs.read(buffer.data(), BUF);
                    std::streamsize r = ifs.gcount();
                    if (r > 0) {
                        if (send_all(conn, buffer.data(), (int)r) < 0) { std::cerr << "send error\n"; break; }
                    }
                }
                std::cout << "Sent file " << fname << " (" << sz << " bytes)\n";
            }
        } else if (cmd == "PUT") {
            std::string ready = "READY\n";
            send_all(conn, ready.c_str(), (int)ready.size());
            if (recv_line(conn, line) <= 0) { closesocket(conn); continue; }
            uint64_t sz = std::stoull(line);
            std::ofstream ofs(fname, std::ios::binary);
            if (!ofs) { std::cerr << "Cannot open file to write: " << fname << "\n"; closesocket(conn); continue; }
            const int BUF = 64*1024;
            std::vector<char> buffer(BUF);
            uint64_t remaining = sz;
            while (remaining > 0) {
                int toread = (int)std::min<uint64_t>(BUF, remaining);
                int got = recv_nbytes(conn, buffer.data(), toread);
                if (got <= 0) { std::cerr << "recv error\n"; break; }
                ofs.write(buffer.data(), got);
                remaining -= (uint64_t)got;
            }
            ofs.close();
            std::cout << "Received file " << fname << " (" << sz << " bytes)\n";
            std::string done = "OK\n";
            send_all(conn, done.c_str(), (int)done.size());
        } else {
            std::string err = "ERR\n";
            send_all(conn, err.c_str(), (int)err.size());
            std::cerr << "Unknown command: " << cmd << "\n";
        }

        closesocket(conn);
        std::cout << "Client disconnected\n";
    }

    closesocket(listenSock);
    WSACleanup();
    return 0;
}

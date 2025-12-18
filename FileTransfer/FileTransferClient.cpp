#pragma comment(lib, "Ws2_32.lib")

#include<stdio.h>
#include<iostream>
#include <WinSock2.h>
#include <winsock.h>
#include <WS2tcpip.h>

constexpr int BUFFER_SIZE = 4096;

void SendAll(SOCKET sock, const char* buffer, const int len)
{
    int totalSent = 0;
    while (totalSent < len)
    {
        int n = send(sock, buffer + totalSent, len - totalSent, 0);
        if (n <= 0)
        {
            break;
        }
        totalSent += n;
    }
}

void Error(const char* msg)
{
    printf(msg);
    std::cerr << WSAGetLastError();
    exit(1);
}

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        Error("Hostname or Port no. not provided. Terminating program");
    }

    WSADATA wsa;
    int r = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (r != 0)
    {
        std::cerr << "WSAStartup failed: " << r << "\n";
        return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        Error("Socket creation failed");
    }

    int n;
    char buffer[BUFFER_SIZE]{};

    sockaddr_in clientSocket;
    clientSocket.sin_family = AF_INET;
    clientSocket.sin_port = htons(atoi(argv[2]));

    inet_pton(AF_INET, argv[1], &clientSocket.sin_addr);

    if (connect(sock, (sockaddr*)&clientSocket, sizeof(clientSocket)) < 0)
    {
        Error("Connection failed");
    }

    FILE* file = fopen("image.jpg", "rb");
    fseek(file, 0, SEEK_END);
    uint64_t fileSize = _ftelli64(file);
    fseek(file, 0, SEEK_SET);

    SendAll(sock, (char*)&fileSize, sizeof(fileSize));

    while (!feof(file))
    {
        int n = fread(buffer, 1, BUFFER_SIZE, file);
        if (n > 0)
        {
            SendAll(sock, buffer, n);
        }
    }

    closesocket(sock);

    WSACleanup();

    return 0;
}
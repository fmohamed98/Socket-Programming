#pragma comment(lib, "Ws2_32.lib")

#include<stdio.h>
#include<iostream>
#include <WinSock2.h>
#include <winsock.h>
#include <WS2tcpip.h>

constexpr int BUFFER_SIZE = 4096;

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

    // For chat
    while (true)
    {
        printf("You : ");
        memset(buffer, 0, BUFFER_SIZE);
        fgets(buffer, BUFFER_SIZE, stdin);


        n = send(sock, buffer, (int)strlen(buffer), 0);
        if (n == -1)
        {
            Error("Error on writing");
        }

        memset(buffer, 0, BUFFER_SIZE);
        if (n = recv(sock, buffer, BUFFER_SIZE, 0) == -1)
        {
            Error("Error on reading");
        }

        printf("Server : %s", buffer);
        if (!strncmp("Bye", buffer, 3))
        {
            break;
        }
    }

    closesocket(sock);

    WSACleanup();

    return 0;
}
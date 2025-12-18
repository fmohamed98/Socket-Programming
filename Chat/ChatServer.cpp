// Socket_Programming.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
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
    std::cerr<<WSAGetLastError();
    exit(1);
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        Error("Port no. not provided. Terminating program");
    }

    WSADATA wsa;
    int r = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (r != 0)
    {
        std::cerr << "WSAStartup failed: " << r << "\n";
        return 1;
    }

    SOCKET sockfd, newsockfd;
    int portno, n;
    char buffer[BUFFER_SIZE]{};

    sockaddr_in serverAddress, clientAddress{};
    int clientLen;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET)
    {
        Error("Error opening socket");
    }

    portno = atoi(argv[1]);

    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(portno);

    if (bind(sockfd, (sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
    {
        Error("Binding failed");
    }

    listen(sockfd, 5);
    clientLen = sizeof(clientAddress);

    newsockfd = accept(sockfd, (sockaddr*)&clientAddress, &clientLen);
    if(newsockfd == INVALID_SOCKET)
    {
        Error("Error on accept");
    }

    //For chat
    while (true)
    {
        memset(buffer, 0, BUFFER_SIZE);
        n = recv(newsockfd, buffer, BUFFER_SIZE, 0);
        if (n == SOCKET_ERROR)
        {
            Error("Error on reading");
        }

        printf("Client : %s", buffer);
        printf("You : ");
        memset(buffer, 0, BUFFER_SIZE);
        fgets(buffer, BUFFER_SIZE, stdin);

        n = send(newsockfd, buffer, (int)strlen(buffer), 0);
        if (n == SOCKET_ERROR)
        {
            Error("Error on writing");
        }

        if (!strncmp("Bye", buffer, 3))
        {
            break;
        }
    }

    closesocket(newsockfd);
    closesocket(sockfd);

    WSACleanup();
    return 0;
}

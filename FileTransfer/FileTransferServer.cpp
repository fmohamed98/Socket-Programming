// Socket_Programming.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#pragma comment(lib, "Ws2_32.lib")

#include<stdio.h>
#include<stdint.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

constexpr int BUFFER_SIZE = 4096;

uint32_t crc32(const char* data, size_t len)
{
    uint32_t crc = 0xFFFFFFFF;

    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++)
            crc = (crc >> 1) ^ (0xEDB88320 & -(int)(crc & 1));
    }

    return ~crc;
}

int RecvAll(SOCKET sock, char* buffer, const int len)
{
    int totalReceived = 0;
    while (totalReceived < len)
    {
        int n = recv(sock, buffer + totalReceived, len - totalReceived, 0);
        if (n <= 0)
        {
            break;
        }
        totalReceived += n;
    }

    return totalReceived;
}

void Error(const char* msg)
{
    printf(msg);
    int errorCode = WSAGetLastError();
    printf(", Error code : %d", errorCode);
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
        printf("WSAStartup failed: %d\n",r);
        return 1;
    }

    SOCKET sockfd, newsockfd;
    char buffer[BUFFER_SIZE]{};

    sockaddr_in serverAddress{}, clientAddress{};
    int clientLen;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET)
    {
        Error("Error opening socket");
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(atoi(argv[1]));

    if (bind(sockfd, (sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
    {
        Error("Binding failed");
    }

    listen(sockfd, 5);
    clientLen = sizeof(clientAddress);

    newsockfd = accept(sockfd, (sockaddr*)&clientAddress, &clientLen);
    if (newsockfd == INVALID_SOCKET)
    {
        Error("Error on accept");
    }

    int fileNameSize;
    char fileName[256];
    RecvAll(newsockfd, (char*)&fileNameSize, sizeof(fileNameSize));
    RecvAll(newsockfd, fileName, fileNameSize);

    fileName[fileNameSize] = '\0';

    char filePath[256];
    printf("Enter output path : ");
    fgets(filePath, 256, stdin);
    filePath[strcspn(filePath, "\n")] = '\0';

    FILE* file = fopen(strcat(filePath,fileName), "wb");
       
    uint64_t fileSize;
    RecvAll(newsockfd, (char*)&fileSize, sizeof(fileSize));

    uint64_t received = 0;
    uint32_t checkSum = 0;
    while (received < fileSize)
    {
        uint64_t toRead = (fileSize - received) > BUFFER_SIZE ? BUFFER_SIZE : (fileSize - received);
        uint64_t n = recv(newsockfd, buffer, toRead, 0);
        fwrite(buffer, 1, n, file);

        checkSum = crc32(buffer, n) ^ checkSum;
        received += n;
    }

    uint32_t receivedCheckSum;
    RecvAll(newsockfd, (char*)&receivedCheckSum, sizeof(receivedCheckSum));

    if (received != fileSize)
    {
        printf("Transfer incomplete\n");
    }
    else
    {
        printf("Transfer complete.\n");
    }

    if (checkSum != receivedCheckSum)
    {
        printf("CheckSum mismatch");
    }
    else
    {
        printf("File is not corrupt");
    }

    fclose(file);
    closesocket(newsockfd);
    closesocket(sockfd);

    WSACleanup();
    return 0;
}

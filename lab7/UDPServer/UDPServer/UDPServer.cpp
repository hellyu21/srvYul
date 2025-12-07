#include <iostream>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main()
{
    WSADATA data;
    WORD version = MAKEWORD(2, 2);

    int wsOk = WSAStartup(version, &data);
    if (wsOk != 0)
    {
        cout << "Can't start Winsock! Error: " << wsOk << endl;
        return 1;
    }

    SOCKET in = socket(AF_INET, SOCK_DGRAM, 0);
    if (in == INVALID_SOCKET)
    {
        cout << "Can't create socket! Error: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverHint;
    serverHint.sin_addr.S_un.S_addr = ADDR_ANY;
    serverHint.sin_family = AF_INET;
    serverHint.sin_port = htons(54000);

    if (bind(in, (sockaddr*)&serverHint, sizeof(serverHint)) == SOCKET_ERROR)
    {
        cout << "Can't bind socket! Error: " << WSAGetLastError() << endl;
        closesocket(in);
        WSACleanup();
        return 1;
    }

    sockaddr_in client;
    int clientLength = sizeof(client);
    char buf[1024];

    cout << "Server started on port 54000..." << endl;

    while (true)
    {
        ZeroMemory(buf, 1024);
        clientLength = sizeof(client);

        int bytesIn = recvfrom(in, buf, 1024, 0, (sockaddr*)&client, &clientLength);
        if (bytesIn == SOCKET_ERROR)
        {
            cout << "Error receiving from client. Error: " << WSAGetLastError() << endl;
            continue;
        }

        char clientIp[INET_ADDRSTRLEN];
        ZeroMemory(clientIp, INET_ADDRSTRLEN);

        if (InetNtopA(AF_INET, &client.sin_addr, clientIp, INET_ADDRSTRLEN) == NULL)
        {
            cout << "Error converting IP address. Error: " << WSAGetLastError() << endl;
            strcpy_s(clientIp, "Unknown");
        }

        cout << "Message received from " << clientIp << ":" << ntohs(client.sin_port)
            << " - " << buf << endl;
    }

    closesocket(in);
    WSACleanup();

    return 0;
}
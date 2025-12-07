#include <iostream> 
#include <WS2tcpip.h>
#include <string>

#pragma comment (lib, "ws2_32.lib")

using namespace std;

int main()
{
    WSADATA data;
    int wsOk = WSAStartup(MAKEWORD(2, 2), &data);
    if (wsOk != 0)
    {
        cout << "Can't start Winsock! Error: " << wsOk << endl;
        return 1;
    }


    sockaddr_in server;
    ZeroMemory(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(54000);
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);


    SOCKET out = socket(AF_INET, SOCK_DGRAM, 0);
    if (out == INVALID_SOCKET)
    {
        cout << "Can't create socket! Error: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    string msg = "";
    cout << "UDP Client started. Type messages to send (type 'exit' to quit):" << endl;

    while (true)
    {
        cout << "> ";
        getline(cin, msg);

        if (msg == "exit")
        {
            cout << "Exiting..." << endl;
            break;
        }

        int sendok = sendto(out,
            msg.c_str(),
            msg.size() + 1,
            0,
            (sockaddr*)&server,
            sizeof(server));

        if (sendok == SOCKET_ERROR)
        {
            cout << "Send failed! Error: " << WSAGetLastError() << endl;
        }
        else
        {
            cout << "Sent " << sendok << " bytes to server" << endl;
        }
    }

    closesocket(out);
    WSACleanup();

    return 0;
}
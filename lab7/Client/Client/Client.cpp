#include <iostream>
#include <string>
#include <WS2tcpip.h>
#include <chrono>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main() {
    string ipAddress = "127.0.0.1";
    int port = 54000;

    WSAData data;
    WORD ver = MAKEWORD(2, 2);
    int wsResult = WSAStartup(ver, &data);
    if (wsResult != 0) {
        cerr << "Can't start Winsock, Err #" << wsResult << endl;
        return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        cerr << "Can't create socket, Err #" << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

    int connResult = connect(sock, (sockaddr*)&hint, sizeof(hint));
    if (connResult == SOCKET_ERROR) {
        cerr << "Can't connect to server, Err #" << WSAGetLastError() << endl;
        cerr << "Make sure the server is running on " << ipAddress << ":" << port << endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    cout << "Connected to server " << ipAddress << ":" << port << endl;
    cout << "Type your messages (enter '#exit' to disconnect, '#shutdown' to stop server):" << endl;

    char buf[4096];
    string userInput;
    auto startTime = chrono::steady_clock::now();
    const int MAX_SESSION_TIME_MINUTES = 5;

    do {
        auto currentTime = chrono::steady_clock::now();
        auto elapsedMinutes = chrono::duration_cast<chrono::minutes>(currentTime - startTime).count();

        if (elapsedMinutes >= MAX_SESSION_TIME_MINUTES) {
            cout << "Session time limit reached. Disconnecting..." << endl;
            userInput = "#exit";
        }
        else {
            cout << "> ";
            getline(cin, userInput);
        }

        if (userInput.size() > 0) {
            int sendResult = send(sock, userInput.c_str(), userInput.size() + 1, 0);
            if (sendResult == SOCKET_ERROR) {
                cerr << "Send failed, Err #" << WSAGetLastError() << endl;
                break;
            }

            ZeroMemory(buf, 4096);
            int bytesReceived = recv(sock, buf, 4096, 0);
            if (bytesReceived > 0) {
                cout << "SERVER> " << string(buf, 0, bytesReceived) << endl;
            }
            else if (bytesReceived == 0) {
                cout << "Server closed the connection." << endl;
                break;
            }
            else {
                cerr << "Error in recv(), Err #" << WSAGetLastError() << endl;
                break;
            }
        }

    } while (userInput != "#exit" && userInput != "#shutdown");

    closesocket(sock);
    WSACleanup();
    cout << "Disconnected from server." << endl;

    return 0;
}
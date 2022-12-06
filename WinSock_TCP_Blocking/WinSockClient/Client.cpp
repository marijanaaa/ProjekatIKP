#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 27016
#define SERVER_IP_ADDRESS "127.0.0.1"

bool InitializeWindowsSockets();

int __cdecl main() 
{
    SOCKET connectSocket = INVALID_SOCKET;
    int iResult;
    char *messageToSend = "this is a test";
    

    if(InitializeWindowsSockets() == false)
    {

		return 1;
    }

    connectSocket = socket(AF_INET,
                           SOCK_STREAM,
                           IPPROTO_TCP);

    if (connectSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

   


    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);
    serverAddress.sin_port = htons(DEFAULT_PORT);

    //neblokirajuci 
    unsigned long int nonBlockingMode = 1;
    iResult = ioctlsocket(connectSocket, FIONBIO, &nonBlockingMode);
    if (iResult == SOCKET_ERROR)
    {
        printf("ioctlsocket failed with error: %ld\n", WSAGetLastError());
        return 1;
    }

   
    iResult = connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress));
    if (iResult == SOCKET_ERROR)
    {
        printf("Unable to connect to server.\n");
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }
    //


    
    

    while (true) {
        iResult = send(connectSocket, messageToSend, (int)strlen(messageToSend) + 1, 0);

        if (iResult == SOCKET_ERROR)
        {
            printf("send failed with error: %d\n", WSAGetLastError());
            closesocket(connectSocket);
            WSACleanup();
            return 1;
        }

        printf("Bytes Sent: %ld\n", iResult);
    }

    iResult = shutdown(connectSocket, SD_BOTH);

    if (iResult == SOCKET_ERROR)
    {
        printf("Shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    closesocket(connectSocket);
    WSACleanup();
    return 0;

}

bool InitializeWindowsSockets()
{
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
    {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        return false;
    }
	return true;
}

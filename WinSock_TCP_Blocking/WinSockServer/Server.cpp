#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27016"

bool InitializeWindowsSockets();

int  main(void) 
{

    SOCKET listenSocket = INVALID_SOCKET;

    SOCKET acceptedSocket = INVALID_SOCKET;

    int iResult;

    char recvbuf[DEFAULT_BUFLEN];
    
    if(InitializeWindowsSockets() == false)
    {

		return 1;
    }
    

    addrinfo *resultingAddress = NULL;
    addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // IPv4 address
    hints.ai_socktype = SOCK_STREAM; // Provide reliable data streaming
    hints.ai_protocol = IPPROTO_TCP; // Use TCP protocol
    hints.ai_flags = AI_PASSIVE;     // 


    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &resultingAddress);
    if ( iResult != 0 )
    {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }


    listenSocket = socket(AF_INET,      // IPv4 address famly
                          SOCK_STREAM,  // stream socket
                          IPPROTO_TCP); // TCP

    if (listenSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(resultingAddress);
        WSACleanup();
        return 1;
    }


    iResult = bind( listenSocket, resultingAddress->ai_addr, (int)resultingAddress->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(resultingAddress);
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(resultingAddress);


    iResult = listen(listenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

	printf("Server initialized, waiting for clients.\n");

   //Proveriti da li listenSocket treba da se prebaci u neblokirajuci rezim
    unsigned long int nonBlMode1 = 1;
    iResult = ioctlsocket(listenSocket, FIONBIO, &nonBlMode1);
    if (iResult == SOCKET_ERROR)
    {
        printf("ioctlsocket failed with error: %ld\n", WSAGetLastError());
        return 1;
    }

    do
    {

        acceptedSocket = accept(listenSocket, NULL, NULL);

        if (acceptedSocket == INVALID_SOCKET)
        {
            printf("accept failed with error: %d\n", WSAGetLastError());
            closesocket(listenSocket);
            WSACleanup();
            return 1;
        }
        //neblokirajuci rezim
        unsigned long int nonBlMode = 1;
        iResult = ioctlsocket(acceptedSocket, FIONBIO, &nonBlMode);
        if (iResult == SOCKET_ERROR)
        {
            printf("ioctlsocket failed with error: %ld\n", WSAGetLastError());
            return 1;
        }

        //dodajemo select
        FD_SET set;
        timeval timeVal;

        FD_ZERO(&set);
        // Add socket we will wait to read from
        FD_SET(acceptedSocket, &set);

        // Set timeouts to zero since we want select to return
        // instantaneously
        timeVal.tv_sec = 0;
        timeVal.tv_usec = 0;

        iResult = select(0 /* ignored */, &set, NULL, NULL, &timeVal);

        // lets check if there was an error during select
        if (iResult == SOCKET_ERROR)
        {
            fprintf(stderr, "select failed with error: %ld\n", WSAGetLastError());
            continue;
        }


        do
        {

          
            iResult = recv(acceptedSocket, recvbuf, DEFAULT_BUFLEN, 0);
            if (iResult > 0)
            {
                printf("Message received from client: %s.\n", recvbuf);
            }
            else if (iResult == 0)
            {

                printf("Connection with client closed.\n");
                closesocket(acceptedSocket);
            }
            else
            {

                printf("recv failed with error: %d\n", WSAGetLastError());
                closesocket(acceptedSocket);
            }
        } while (iResult > 0);



    } while (1);


    iResult = shutdown(acceptedSocket, SD_SEND);
    if (iResult == SOCKET_ERROR)
    {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(acceptedSocket);
        WSACleanup();
        return 1;
    }

    closesocket(listenSocket);
    closesocket(acceptedSocket);
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

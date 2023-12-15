#undef UNICODE

//НЕ UPD А UDP!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1 в названии





#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <thread>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"


#define NUMBER_OF_THREADS 10

int iResult;

SOCKET ListenSocket = INVALID_SOCKET;
SOCKET ClientSocket = INVALID_SOCKET;

char recvbuf[DEFAULT_BUFLEN];
int recvbuflen = DEFAULT_BUFLEN;

int iSendResult;

sockaddr_in SenderAddr;
int SenderAddrSize = sizeof(SenderAddr);

sockaddr_in RecvAddr;

void threadStart() {
    printf("Thread has been started\n");
    while (true) {

        // No longer need server socket
        //closesocket(ListenSocket);

        // Receive until the peer shuts down the connection
        do {

            iResult = recvfrom(ListenSocket, recvbuf, recvbuflen, 0 , (SOCKADDR*)&SenderAddr,&SenderAddrSize);
            if (iResult > 0) {
                printf("Bytes received: %d\n", iResult);

                RecvAddr = SenderAddr;

                // TODO Куда слать?
                // Вытащить из senderAddr?
                // 
                // Echo the buffer back to the sender
                iSendResult = sendto(ListenSocket, recvbuf, iResult, 0, (SOCKADDR*)&RecvAddr, sizeof(RecvAddr));
                if (iSendResult == SOCKET_ERROR) {
                    printf("send failed with error: %d\n", WSAGetLastError());
                    closesocket(ClientSocket);
                    WSACleanup();
                    return;
                }
                printf("Bytes sent: %d\n", iSendResult);
            }
            else if (iResult == 0)
                printf("Connection closing...\n");
            else {
                printf("recv failed with error: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
                return;
            }

        } while (iResult > 0);

        // shutdown the connection since we're done
        iResult = shutdown(ClientSocket, SD_SEND);
        if (iResult == SOCKET_ERROR) {
            printf("shutdown failed with error: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return;
        }

        // cleanup
        closesocket(ClientSocket);
    }
    printf("Thread is closing...\n");
}

int __cdecl main(void)
{
    WSADATA wsaData;



    struct addrinfo* result = NULL;
    struct addrinfo hints;


    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for the server to listen for client connections.
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    std::thread threads[NUMBER_OF_THREADS];
    for (int i = 0; i < NUMBER_OF_THREADS; i++)threads[i] = std::thread(threadStart);

    //th1.join();
    //std::thread threads[NUMBER_OF_THREADS](threadStart);
    for (int i = 0; i < NUMBER_OF_THREADS; i++) threads[i].join();
    WSACleanup();
    closesocket(ListenSocket);
    printf("main thread is ended\n");
    return 0;
}

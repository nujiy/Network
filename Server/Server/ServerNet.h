#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS


#include <winSock2.h>
#include <winsock.h>
#include <windows.h>
#include <stdlib.h>
#include <WS2tcpip.h>
#include <iostream>
#include <time.h>
#include <assert.h>
#include <cstring>
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib")

#define IP_SIZE 129

#define ServerPort 5884
//maximum size of a pkg
#define MAX_MSG_SIZE 4096


class ServerNet
{
public:
	ServerNet();	//init server
	~ServerNet();
	ServerNet(const ServerNet &) = delete;
	ServerNet &operator=(const ServerNet &) = delete;
	void WaitForClient();	//wait for connection from client after listen successfully

private:
	WORD winsockVersion;
	WSADATA wsaData;
	
	HANDLE hThread;			//threads

	SOCKET socketServer;	//server socket
	SOCKET socketClient;	//client socket

	SOCKADDR_IN addrServer;	//data from server
	SOCKADDR_IN addrClient;	//Msg from client

	int rstValue; //To signal whether initial successfully
	int addrLength;	//Data length

	wchar_t bufferForIP[IP_SIZE];
};

//the struct of client
typedef struct Client* P_Client;

struct Client
{
	int Num;			//the No. which is defined in Server
	SOCKET sock_Client;	//socket
	char IP[20];		//IP address
	u_short Port;		//the port of client
	P_Client Next;		//next client
public:
	//Add new client to the client list
	static void Client_add(P_Client Head, SOCKET newSock, char* newIP, u_short newPort);
    //delete the client 
	static int Client_del(P_Client Head, SOCKET oldSock);
	//get the info from client list and create a file in buff
	static int getList(P_Client Head, char * Message);
	//get the number of client from socket
	static int getNum(P_Client Head, SOCKET sock);
	//get socket frome the client
	static SOCKET getSock(P_Client Head, int n);
};

//create new thread
DWORD WINAPI CreateClientThread(LPVOID lpParameter);
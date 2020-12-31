#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <windows.h>
#include <mutex>
#include <chrono>
#include <thread>
#include <iostream>
#include <string.h>
#include <string>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

#define IP_SIZE 129

#define ServerPort 5884

#define MAX_MSG_SIZE 4096



using namespace std;

typedef char Msg[MAX_MSG_SIZE];

struct queueNode
{
	Msg message;
	struct queueNode* next;

};

typedef struct queueNode* queueList;


class Queue {
	queueList head;
	queueList tail;
public:
	Queue();
	void Enqueue(char* msg);
	string Dequeue();
	~Queue();
};

class ClientNet
{
private:
	SOCKET m_sock;
	timed_mutex iotex;
	timed_mutex qutex; 
	Queue infoQueue;

public:
	// connect to server
	int ClientConnect(int port, const char* address);
	// send message
	int ClientSend(const char* msg, int len);
	// close connection
	void ClientClose();
	// receive message
	int  ClientReceive(char* recvBuffer);
	// listen the request from server
	void ClientListenStart(bool *isConnect);
	// test function to lock the resource
	bool real_test_lock();
	// process the message queue 
	void ProcessQueue(char* listenBuffer, bool *isConnect);
};


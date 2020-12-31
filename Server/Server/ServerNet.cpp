#include "ServerNet.h"

using namespace std;

extern P_Client list_client;

ServerNet::ServerNet()
{
	cout << "start initializing Server!" << endl;
	
	winsockVersion = MAKEWORD(2, 2); //the version of windows socket is 2.2
	addrLength = sizeof(SOCKADDR_IN); 
	addrServer.sin_family = AF_INET;
	addrServer.sin_addr.S_un.S_addr = htonl(INADDR_ANY);      //htonl将主机字节序转换为网络字节序
	addrServer.sin_addr.S_un.S_addr = inet_addr("127.0.1.1"); //bind host ip address
	addrServer.sin_port = htons(ServerPort);                  //bind listening port

	memset(bufferForIP, 0, IP_SIZE);

	//start WSA
	rstValue = WSAStartup(winsockVersion, &wsaData);
	if (rstValue != 0)
	{
		cout << "WSA Startup() failed!" << endl;
		getchar();
		exit(1);
	}
	cout << "WSA started up successfully!" << endl;

	//socket()
	socketServer = socket(AF_INET, SOCK_STREAM, 0);
	if (socketServer == INVALID_SOCKET)
	{
		WSACleanup();
		cout << "socket() failed" << endl;
		getchar();
		exit(1);
	}
	cout << "Server socket created successfully!" << endl;

	//Bind()
	rstValue = bind(socketServer, (SOCKADDR *)&addrServer, sizeof(SOCKADDR));
	if (rstValue == SOCKET_ERROR)
	{
		cout << "bind failed! code:" << WSAGetLastError() << endl;
		closesocket(socketServer);
		WSACleanup();
		getchar();
		exit(1);
	}
	cout << "Server socket bound successfully!" << endl;

	//finish bind, start listening
	rstValue = listen(socketServer, 5);
	if (rstValue == SOCKET_ERROR)
	{
		cout << "listen() faild! code:" << WSAGetLastError() << endl;
		closesocket(socketServer); //close socket
		getchar();
		exit(1);
	}
	cout << "Server socket started to listen...\n";

	printf("Welcome,the Host %s is running!Now Wating for someone comes in!\n", inet_ntoa(addrServer.sin_addr));
}

ServerNet::~ServerNet()
{
	closesocket(socketServer);
	closesocket(socketClient);
	WSACleanup();
	cout << "The Socket is closed" << endl;
}

void ServerNet:: WaitForClient()
{
	while (true)
	{
		//if connect to client, then continue
		//else block
		socketClient = accept(socketServer, (SOCKADDR *)&addrClient, &addrLength);
		//connection failed
		if (socketClient == INVALID_SOCKET)
		{
			printf("Accept Failed\n");
			continue;
		}
		printf("Accept Success\n");
		InetNtopW(addrClient.sin_family, &addrClient, bufferForIP, IP_SIZE);
		//print the ip and port of client
		cout << "A new client connected! The IP address: " << inet_ntoa(addrClient.sin_addr) << ", port number: " << ::ntohs(addrClient.sin_port) << endl;
		//add the client to the list
		Client::Client_add(list_client, socketClient, inet_ntoa(addrClient.sin_addr), ntohs(addrClient.sin_port));
		//create subthread
		hThread = CreateThread(NULL, 0, CreateClientThread, (LPVOID)socketClient, 0, NULL);
		if (hThread == NULL)
		{
			cerr << "Failed to create a new thread!Error code: " << ::WSAGetLastError() << endl;
			WSACleanup();
			getchar();
			exit(1);
		}
		CloseHandle(hThread);
	}
	return;
}

void Client::Client_add(P_Client Head, SOCKET newSock, char* newIP, u_short newPort)
{
	P_Client newnode = (P_Client)malloc(sizeof(struct Client));
	strcpy(newnode->IP, newIP);
	newnode->Port = newPort;
	newnode->Next = NULL;
	newnode->sock_Client = newSock;
	newnode->Num = 1;

	P_Client ptr = Head;
	while (ptr->Next != NULL)
	{
		newnode->Num++;
		ptr = ptr->Next;
	}
	ptr->Next = newnode;
	cout << "The Client has already added to the list" << endl;
	return;
}

int Client:: Client_del(P_Client Head, SOCKET oldSock)
{
	P_Client ptr = Head;
	P_Client ptr2 = NULL;
	while (ptr->Next != NULL)
	{
		if (ptr->Next->sock_Client == oldSock)
		{
			ptr2 = ptr->Next;
			ptr->Next = ptr2->Next;
			cout << "The Client has been Deleted from the list" << endl;
			break;
		}
		ptr = ptr->Next;
	}
	free(ptr2);
	ptr2 = Head;
	int Number = 0;
	while (ptr2 != NULL)
	{
		ptr2->Num = Number;
		ptr2 = ptr2->Next;
		Number++;
	}

	if (ptr->Next == NULL)
		cout << "Deletion failed" << endl;
	return 1;
}

int Client::getList(P_Client Head, char * Message)
{
	P_Client ptr = Head->Next;
	char Current[10];
	while (ptr != NULL)
	{
		memset(Current, 0, 10);
		sprintf(Current, "%d", ptr->Num);
		strcat(Message, Current);
		strcat(Message, " ");
		strcat(Message, ptr->IP);
		strcat(Message, " ");
		memset(Current, 0, 10);
		sprintf(Current, "%d", (int)ptr->Port);
		strcat(Message, Current);
		strcat(Message, "\n");
		ptr = ptr->Next;

	}
	return 0;
}

int Client::getNum(P_Client Head, SOCKET sock)
{
	P_Client ptr = Head->Next;

	while (ptr != NULL)
	{
		if (ptr->sock_Client == sock)
			return ptr->Num;
		ptr = ptr->Next;
	}

	return -1;
}

SOCKET Client::getSock(P_Client Head, int n)
{
	P_Client ptr = Head->Next;
	SOCKET Find = INVALID_SOCKET;
	while (ptr != NULL)
	{
		if (n == ptr->Num)
		{
			Find = ptr->sock_Client;
			break;
		}
		ptr = ptr->Next;
	}
	return Find;
}

DWORD WINAPI CreateClientThread(LPVOID lpParameter)
{
	//get the socket from client
	SOCKET socketClient = (SOCKET)lpParameter;
	//create buffer
	char bufMessage[MAX_MSG_SIZE];
	//To estimate whether already recieved 
	int retValue = 0;
	int sendResult = 0;
	//the buffer sending to other client
	char AllMessage[MAX_MSG_SIZE];
	memset(AllMessage, 0, MAX_MSG_SIZE);
	SOCKET Mark = INVALID_SOCKET;

	//connect to client and send "Hello"
	memset(bufMessage, 0, MAX_MSG_SIZE);
	strcpy(bufMessage, "Hello!\n");
	sendResult = send(socketClient, bufMessage, MAX_MSG_SIZE, 0);
	do
	{

		memset(bufMessage, 0, MAX_MSG_SIZE);
		retValue = recv(socketClient, bufMessage, MAX_MSG_SIZE, 0);
		if (retValue > 0)
		{
			if (strcmp(bufMessage, "exit") == 0)
			{
				cout << "Message received: " << bufMessage << endl;
				cout << "Client requests to close the connection" << endl;
				break;
			}
			else if (strstr(bufMessage, "time") != NULL)
			{
				//print the request msg from client 
				cout << "Message received: " << bufMessage << endl;
				//reset Buffer
				memset(bufMessage, 0, MAX_MSG_SIZE);
				cout << "Client requests to get the time" << endl;
				//get current time and copy to buffer
				time_t t = time(0);
				strftime(bufMessage, sizeof(bufMessage), "%Y-%m-%d %H-%M-%S\n", localtime(&t));

			}
			else if (strstr(bufMessage, "GetComputerName") != NULL)
			{
				//print the request Msg and fill buffer
				cout << "Message received: " << bufMessage << endl;
				memset(bufMessage, 0, MAX_MSG_SIZE);
				//request host name and copy tot buffer
				char host[256];
				if (gethostname(host, sizeof(host)) == SOCKET_ERROR)
				{
					cout << "CaNNOT get host name..." << endl;
					break;
				}
				else
				{
					cout << "The name of the computer: " << host << endl;
					const char* str = "The name of the computer: ";
					strcpy(bufMessage, str);
					strcat(bufMessage, host);
					cout << bufMessage << endl;
				}
			}
			else if (strstr(bufMessage, "GetClientList") != NULL)
			{
				cout << "Message received: " << bufMessage << endl;
				memset(bufMessage, 0, MAX_MSG_SIZE);
				Client::getList(list_client, bufMessage);
				cout << bufMessage << endl;
			}
			else if (strstr(bufMessage, "Send") != NULL)
			{
				//receive send, print the message received
				cout << "Message received: " << bufMessage << endl;
				//get client number and get socket
				int num_client = bufMessage[5] - '0';
				if (!(1 <= num_client && num_client <= 9))
				{
					memset(bufMessage, 0, MAX_MSG_SIZE);
					strcpy(bufMessage, "The Number is Error");
					cout << bufMessage << endl;
				}
				else
				{
					Mark = Client::getSock(list_client, num_client);
					if (Mark == INVALID_SOCKET)
					{
						memset(bufMessage, 0, MAX_MSG_SIZE);
						strcpy(bufMessage, "The Number is Error,Please check the client list again");
						cout << bufMessage << endl;
					}
					else
					{
						char * Message = &(bufMessage[7]);
						memset(AllMessage, 0, MAX_MSG_SIZE);
						strcpy(AllMessage, "The Message From Client ");
						char temp[5];
						//get the number of client
						num_client = Client::getNum(list_client, socketClient);
						temp[0] = num_client + '0';
						temp[1] = ' ';
						temp[2] = ':';
						temp[3] = ' ';
						temp[4] = '\0';
						strcat(AllMessage, temp);
						//copy message to ALLMessage, send Buffer
						strncat(AllMessage, Message, MAX_MSG_SIZE - 28);
						cout << "The Message send: " << AllMessage << endl;
						//send message
						sendResult = send(Mark, AllMessage, MAX_MSG_SIZE, 0);
						if (sendResult == SOCKET_ERROR)
						{
							memset(bufMessage, 0, MAX_MSG_SIZE);
							strcpy(bufMessage, "Message send Failed,Check the Clinet list and try again");
							cout << bufMessage << endl;
						}
						else
						{
							//successfullly, send a message of success to client
							memset(bufMessage, 0, MAX_MSG_SIZE);
							strcpy(bufMessage, "Message send success");
							cout << bufMessage << endl;
						}

					}

				}
			}
			else
			{
				memset(bufMessage, 0, MAX_MSG_SIZE);
				strcpy(bufMessage, "Can't realize the construct");
			}

			sendResult = send(socketClient, bufMessage, MAX_MSG_SIZE, 0);
			if (sendResult == SOCKET_ERROR)
			{
				cerr << "Failed to send message to client!Error code: " << ::GetLastError() << endl;
				::closesocket(socketClient);
				getchar();
				return 1;
			}
			else
			{
				cout << "Send Success" << endl;
			}

		}
		else if (retValue == 0)
		{
			cout << "connection closed..." << endl;
			break;
		}
		else
		{
			cerr << "Failed to receive message from client!Error code: " << ::GetLastError() << endl;
			closesocket(socketClient);
			Client::Client_del(list_client, socketClient);
			system("pause");
			return 1;
		}
	} while (retValue > 0);

	Client::Client_del(list_client, socketClient);
	retValue = shutdown(socketClient, SD_SEND);
	if (retValue == SOCKET_ERROR)
	{
		cerr << "Failed to shutdown the client socket!Error code: " << ::GetLastError() << "\n";
		::closesocket(socketClient);
		return 1;
	}
	return 0;
}
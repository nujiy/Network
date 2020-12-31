#include"ClientNet.h"

#define SLEEP_2 Sleep(1000)
using namespace std;

ClientNet client;

char msgBuffer[MAX_MSG_SIZE];
char recvBuffer[MAX_MSG_SIZE];
char listenBuffer[MAX_MSG_SIZE];
bool Is_connect = false;

thread *pthread_id2 = NULL;
thread *pthread_id = NULL;


//listen the message from message queue
void Polling() {
	client.ClientListenStart(&Is_connect);
}


//listen the return from server and add to queue
void Polling_() {
	client.ProcessQueue(listenBuffer, &Is_connect);
}


int main()
{
	int rlt = 0;
	string msg;

	pthread_id = new thread(Polling);
	pthread_id2 = new thread(Polling_);

	if (rlt == 0)
	//connect successfully
	{
		//send message
		printf("connect successfully. input `q` to quit, `help` for instructions\n");
		cout << endl;
		cout << "\t1. connect" << endl;
		cout << "\t2. disconnect" << endl;
		cout << "\t3. get server name" << endl;
		cout << "\t4. get client list" << endl;
		cout << "\t5. get current time" << endl;
		cout << "\t6. send message to client" << endl;
		cout << endl;
		//start message queue 
		while (true)
		{
			while (!client.real_test_lock());
			printf("local/> ");
			getline(cin, msg);
			if (msg == "q") {
				//disconnect before quit
				if (Is_connect = true) 
				{
					cout << "please wait...";
					Is_connect = false;
					SLEEP_2;
					client.ClientClose();
					cout << "done" << endl;
				}
				break;
			}
			else if (msg == "help") {
				cout << endl;
				cout << "\t1. connect" << endl;
				cout << "\t2. disconnect" << endl;
				cout << "\t3. get server name" << endl;
				cout << "\t4. get client list" << endl;
				cout << "\t5. get current time" << endl;
				cout << "\t6. send message to client" << endl;
				cout << "\t`q`to quit, `help`for instructions" << endl;
				cout << endl;
			}
			else
			{
				if (msg == "1") {
					if (Is_connect == false) 
					{
						int port;
						string ip;
						cout << "IP: ";
						getline(cin, ip);
						rlt = client.ClientConnect(ServerPort, ip.c_str());
						if (rlt != 0) {
							cout << "Error: Connection failed. ( " << rlt << " )\n" << endl;
							continue;
						}
						cout << "Connection success!\n" << endl;
						Is_connect = true;
					}
					else 
					{
						cout << "Error: Can't connect to more than ONE server.\n" << endl;
					}
				}
				else if (msg == "2") {
					string opt;
					cout << "Sure to disconnect? [Y/N]: ";
					getline(cin, opt);
					if (opt == "Y") {
						// disconnect
						if (Is_connect == true) 
						{
							cout << "Please waiting...";
							Is_connect = false;
							//close
							SLEEP_2;
							//release the point
							cout << "Disconnect success\n" << endl;
							strcpy_s(msgBuffer, "exit");
							client.ClientClose();
						}
						else {
							cout << "Error: No active connections. Please create one first.\n" << endl;
						}

					}
					else if (opt == "N") {
						cout << "Warning: Operation abort.\n" << endl;
					}
					else {
						cout << "Error: No such option.\n" << endl;
					}

				}
				else if (msg == "3") {
					strcpy_s(msgBuffer, "GetComputerName");
					rlt = client.ClientSend(msgBuffer, MAX_MSG_SIZE);
					//clean the buffer
					msgBuffer[0] = '\0';
				}
				else if (msg == "4") {
					strcpy_s(msgBuffer, "GetClientList");
					rlt = client.ClientSend(msgBuffer, MAX_MSG_SIZE);
					msgBuffer[0] = '\0';
				}
				else if (msg == "5") {
					strcpy_s(msgBuffer, "time");
					rlt = client.ClientSend(msgBuffer, MAX_MSG_SIZE);
					msgBuffer[0] = '\0';

				}
				else if (msg == "6") 
				{
					string num;
					string msg;
					cout << "Please select the client No. : ";
					getline(cin, num);
					cout << "Please enter the info: ";
					getline(cin, msg);
					string pkg = "Send " + num + " " + msg;
					strcpy_s(msgBuffer, pkg.c_str());
					rlt = client.ClientSend(msgBuffer, MAX_MSG_SIZE);
					msgBuffer[0] = '\0';
				}
				else {
					cout << "Error: No such option\n" << endl;
				}
			}
		}
		//clos socket
	}
	system("pause");
	return 0;
}
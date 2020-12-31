#include "ClientNet.h"


#define SLEEP_1 Sleep(500)

using namespace std;

int ClientNet::ClientConnect(int port, const char* address)
{
	//connectflag  
	//0-success 1-WSAStartfailed 2-socketfailed 3-connectfailed
	int rlt = 0;   
	
	//set the time out of send and receive
	struct timeval timeout = { 3, 0 };
	
	struct linger so_linger;
	so_linger.l_onoff = TRUE;
	so_linger.l_linger = 30;

	int suc = setsockopt(m_sock, SOL_SOCKET, SO_DONTLINGER, (const char*)&so_linger, sizeof(so_linger));
	if (suc)
		perror("setsockopt(2)");
	//record the error 
	int iErrMsg;
	//start up WinSock
	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested = MAKEWORD(2, 2);
	iErrMsg = WSAStartup(wVersionRequested, &wsaData);
	if (iErrMsg != NO_ERROR)
		//WSAStartup error
	{
		printf("failed with WSAStartup error: %d\n", iErrMsg);
		rlt = 1;
		return rlt;
	}

	//create Socket
	m_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (m_sock == INVALID_SOCKET)
		//socket error
	{
		printf("failed with socket error: %d\n", WSAGetLastError());
		rlt = 2;
		return rlt;
	}


	//the data from server
	SOCKADDR_IN servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	servaddr.sin_addr.s_addr = inet_addr(address);

	//connect to target server
	iErrMsg = connect(m_sock, (SOCKADDR*)&servaddr, sizeof(servaddr));
	if (iErrMsg != NO_ERROR)
	{
		printf("failed with connect error: %d\n", iErrMsg);
		rlt = 3;
		return rlt;
	}

	// success
	// start listen
	return rlt;

}


int ClientNet::ClientSend(const char* msg, int len)
{
	int rlt = 0;

	int iErrMsg = 0;

	//send message to sock
	iErrMsg = send(m_sock, msg, len, 0);
	if (iErrMsg < 0)
		//fail
	{
		printf("send msg failed with error: %d\n", iErrMsg);
		rlt = 1;
		return rlt;
	}
	else if (iErrMsg == 0)
	{
		rlt = 3;
		printf("connection timeout.\n");
		return rlt;
	}
	iotex.lock();
	return rlt;
}




/*close Socket*/
void ClientNet::ClientClose()
{
	closesocket(m_sock);
}


void ClientNet::ClientListenStart(bool *isConnect) {
	while (true) {
		//end the thread
		string res;
		if (qutex.try_lock() == false) 
		{
			SLEEP_1;
			continue;
		}
		if ((res = infoQueue.Dequeue()) == "") 
		{
			qutex.unlock();
			SLEEP_1;
			continue;
		}
		cout << "\nSERVER/> ";
		cout << res.c_str() << endl << endl;
		qutex.unlock();
		if (real_test_lock() == false) {
			iotex.unlock();
		}
		else
			cout << "\nlocalhost/> ";
		SLEEP_1;
	}
}



void ClientNet::ProcessQueue(char* listenBuffer, bool *isConnect) {
	while (1) {
		// end the control of thread
		int iErrMsg = 0;
		iErrMsg = recv(m_sock, listenBuffer, MAX_MSG_SIZE, 0);
		//time out but connect correctly
		if (iErrMsg < 0 && (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)) continue;
		else if (iErrMsg < 0) {
			continue;
		}
		// network error
		else if (iErrMsg == 0) { cout << "error: Network Suddenly Disconnected." << endl; continue; };
		qutex.lock();
		infoQueue.Enqueue(listenBuffer);
		qutex.unlock();
	}
}


bool ClientNet::real_test_lock() 
{
	if (iotex.try_lock() == false) 
		return false;
	else {
		iotex.unlock();
		return true;
	}

}

Queue::Queue() 
{
	head = new struct queueNode;
	head->next = NULL;
	tail = head;
}

void Queue::Enqueue(char* msg) 
{
	if (tail == NULL) return;
	tail->next = new struct queueNode;
	tail = tail->next;
	strcpy_s(tail->message, msg);
	tail->next = NULL;
}
string Queue::Dequeue() 
{
	if (head == tail) return "";
	queueList temp = head->next;
	string ret = temp->message;
	head->next = temp->next;
	if (tail == temp) tail = head;
	delete temp;
	temp = NULL;
	return ret;
}
Queue::~Queue() 
{
	delete head;
	head = NULL;
}



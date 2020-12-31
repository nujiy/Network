#include "ServerNet.h"
#pragma warning(disable:4996)

using namespace std;


//list_client:the list of client
//NOTE: the head is empty
P_Client list_client;	

int main()
{
	//init
	ServerNet svr;
	
	list_client = (P_Client)malloc(sizeof(struct Client));
	list_client->Next = NULL;
	list_client->Num = 0;
	//wait for clients' connection

	svr.WaitForClient();
	//server over
	getchar();
	return 0;
}

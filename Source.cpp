#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <vector>
#include <processthreadsapi.h>
#include <Windows.h>
#include <time.h>
#include <stdlib.h>

#pragma comment (lib, "ws2_32.lib")

using namespace std;

struct MailingInfo {
	SOCKET sock;
	int type;
};

struct Info {
	sockaddr_in client;
	int clientSize;
	char host[NI_MAXHOST];		// Client's remote name
	char service[NI_MAXSERV];	// Service (i.e. port) the client is connect on
	HANDLE th;
	DWORD th_id;
};

DWORD WINAPI SendData(LPVOID lpParameter) {
	MailingInfo* p = (MailingInfo*)lpParameter;

	string isContinue;

	switch (p->type) {
	case 1: {
		int hour = 0;

		while (true) {
			srand(time(NULL));
			string msg = "Prediction on hour: " + to_string(hour) + " is " + to_string(rand() % 40) + " C^";
			send(p->sock, msg.c_str(), msg.size() + 1, 0);
			hour++;
			Sleep(5000);
		}
	}

	case 2: {
		int minutes = 0;

		while (true) {
			srand(time(NULL));
			string msg = "Prediction on minute: " + to_string(minutes) + " is " + to_string(rand() % 30) + "." + to_string(rand() % 30) + " eth.";
			send(p->sock, msg.c_str(), msg.size() + 1, 0);
			minutes++;
			Sleep(5000);
		}
	}

	case 3: {
		int day = 0;

		while (true) {
			srand(time(NULL));
			string msg = "Prediction on day: " + to_string(day) + " is " + to_string(rand() % 30) + "." + to_string(rand() % 30) + " usd.";
			send(p->sock, msg.c_str(), msg.size() + 1, 0);
			day++;
			Sleep(5000);
		}
	}

	}
	return 0;
}


int main()
{
	//Init winsock
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2); //request winsock 2.2 version

	int wsOk = WSAStartup(ver, &wsData);
	if (wsOk != 0)
	{
		cerr << "Can't Initialize winsock! Quitting" << endl;
		exit(3); //3 - exit code to error init winsock
	}

	// Create a socket
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);

	//AF_INET is ised to communicate by ipv4
	//TCP - SOCK_STREAM
	//UDP - SOCK_DGRAM

	if (listening == INVALID_SOCKET)
	{
		cerr << "Can't create a socket! Quitting" << endl;
		exit(4); //4 - exit code to error creating socket
	}

	// Bind the ip address and port to a socket
	sockaddr_in hint;				//socket for IP protocols
	hint.sin_family = AF_INET;
	hint.sin_port = htons(54000);		//port number to be accessed by process
	hint.sin_addr.S_un.S_addr = INADDR_ANY;  //address to link socket with(INADDR_ANY - all adresses from local host - 0.0.0.0)

	bind(listening, (sockaddr*)&hint, sizeof(hint)); //binds socket to port and adress

	//places a socket in state of listening for incoming connection 
	listen(listening, SOMAXCONN);

	fd_set master;	//set of incoming sockets clients
	FD_ZERO(&master); //clears set
	FD_SET(listening, &master);		//sets first FD set element as server socket

	vector<Info> clientsInfo;	//info about each client socket

	while (true) {
		fd_set copy = master;

		int socketCount = select(0, &copy, nullptr, nullptr, nullptr);

		for (int i = 0; i < socketCount; i++) {
			SOCKET sock = copy.fd_array[i];

			if (sock == listening) {
				clientsInfo.push_back({});
				clientsInfo[i].clientSize = sizeof(clientsInfo[i].client);

				ZeroMemory(clientsInfo[i].host, NI_MAXHOST);
				ZeroMemory(clientsInfo[i].service, NI_MAXSERV);

				//accepts incoming connection
				SOCKET client = accept(listening, (sockaddr*)&clientsInfo[i].client, &clientsInfo[i].clientSize);
				FD_SET(client, &master);


				if (getnameinfo((sockaddr*)&clientsInfo[i].client, sizeof(clientsInfo[i].client), clientsInfo[i].host, NI_MAXHOST, clientsInfo[i].service, NI_MAXSERV, 0) == 0)
				{
					cout << clientsInfo[i].host << " connected on port " << clientsInfo[i].service << endl;
				}
				else
				{
					inet_ntop(AF_INET, &clientsInfo[i].client.sin_addr, clientsInfo[i].host, NI_MAXHOST);
					cout << clientsInfo[i].host << " connected on port " << ntohs(clientsInfo[i].client.sin_port) << endl;
				}
			}
			else {
				char buf[4096];
				ZeroMemory(buf, 4096);

				int bytesIn = recv(sock, buf, 4096, 0);

				if (bytesIn == SOCKET_ERROR) {
					closesocket(sock);
					FD_CLR(sock, &master);
				}
				else {
					if (!strcmp(buf, "1") || !strcmp(buf, "2") || !strcmp(buf, "3")) {

						for (int i = 0; i < master.fd_count; i++) {
							SOCKET outSock = master.fd_array[i];

							if (outSock != listening && outSock == sock) {

								string SubsciptionAccepted = "Sucessfully subscribed to mailing service...";

								int oper = atoi(buf);
								cout << to_string(oper) + " option of mailing was chosed by client " + clientsInfo[i - 1].host + " on port: " + clientsInfo[i - 1].service << endl;

								send(outSock, SubsciptionAccepted.c_str(), SubsciptionAccepted.size() + 1, 0);

								MailingInfo* mailing = new MailingInfo;
								mailing->sock = outSock;
								mailing->type = oper;

								clientsInfo[i - 1].th = CreateThread(NULL, 0, &SendData, (void*)mailing, 0, &clientsInfo[i - 1].th_id);

								if (clientsInfo[i - 1].th == NULL) {
									cerr << "Error creating thread..." << endl;
									exit(5);
								}

								break;
							}
						}
					}
					else if (!strcmp(buf, "N")) {
						for (int i = 0; i < master.fd_count; i++) {
							SOCKET outSock = master.fd_array[i];

							if (outSock != listening && outSock == sock) {

								TerminateThread(clientsInfo[i - 1].th, 4);

								string SubsciptionCanceled = "Your subscription was cancelled.";
								send(outSock, SubsciptionCanceled.c_str(), SubsciptionCanceled.size() + 1, 0);

								break;
							}
						}
					}
				}
			}
		}
	}

	// Cleanup winsock
	WSACleanup();

	system("pause");

	return 0;
}

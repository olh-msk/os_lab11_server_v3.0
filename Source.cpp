#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <vector>
#include <processthreadsapi.h>

#pragma comment (lib, "ws2_32.lib")

using namespace std;

DWORD WINAPI SendData(LPVOID lpParameter) {



	return 0;
}

struct MailingInfo {
	SOCKET sock;
	int type;
};

struct Info {
	sockaddr_in client;
	int clientSize;
	char host[NI_MAXHOST];		// Client's remote name
	char service[NI_MAXSERV];	// Service (i.e. port) the client is connect on
};

void main()
{
	// Initialze winsock
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	vector<HANDLE> myhandle;

	int wsOk = WSAStartup(ver, &wsData);
	if (wsOk != 0)
	{
		cerr << "Can't Initialize winsock! Quitting" << endl;
		return;
	}

	// Create a socket
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET)
	{
		cerr << "Can't create a socket! Quitting" << endl;
		return;
	}

	// Bind the ip address and port to a socket
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(54000);
	hint.sin_addr.S_un.S_addr = INADDR_ANY; // Could also use inet_pton .... 

	bind(listening, (sockaddr*)&hint, sizeof(hint));

	// Tell Winsock the socket is for listening 
	listen(listening, SOMAXCONN);

	/*// While loop: accept and echo message back to client
	char buf[4096];
	string SubsciptionAccepted = "Sucessfully subscripted to mailing service: client ";
	SubsciptionAccepted += host;
	SubsciptionAccepted += "..\n";

	ZeroMemory(buf, 4096);

	int bytesReceived = recv(clientSocket, buf, 4096, 0);
	if (bytesReceived == SOCKET_ERROR)
	{
		cerr << "Error in recv(). Quitting" << endl;
		exit(3);
	}
	int oper = atoi(buf);
	cout << to_string(oper) + " option of mailing was chosed by client " + host;

	send(clientSocket, SubsciptionAccepted.c_str(), SubsciptionAccepted.size() + 1, 0);


	switch (oper) {
	case 1:
		do {
			send(clientSocket, buf, bytesReceived + 1, 0);
		} while (true);
	case 2:
		do {
			send(clientSocket, buf, bytesReceived + 1, 0);
		} while (true);
	case 3:
		do {
			send(clientSocket, buf, bytesReceived + 1, 0);
		} while (true);
	}

	// Close the socket
	closesocket(clientSocket);*/

	fd_set master;	//set of incoming sockets clients
	FD_ZERO(&master); //clears set
	FD_SET(listening, &master);

	vector<Info> clientsInfo;


	while (true) {
		fd_set copy = master;

		int socketCount = select(0, &copy, nullptr, nullptr, nullptr);

		for (int i = 0; i < socketCount; i++) {
			SOCKET sock = copy.fd_array[i];

			if (sock == listening) {
				clientsInfo.push_back({});
				clientsInfo[i].clientSize = sizeof(clientsInfo[i].client);

				ZeroMemory(clientsInfo[i].host, NI_MAXHOST); // same as memset(host, 0, NI_MAXHOST);
				ZeroMemory(clientsInfo[i].service, NI_MAXSERV);

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
								cout << to_string(oper) + " option of mailing was chosed by client \n"; // +clientsInfo[i].host;

								send(outSock, SubsciptionAccepted.c_str(), SubsciptionAccepted.size() + 1, 0);

								MailingInfo* mailing = new MailingInfo;
								mailing->sock = outSock;
								mailing->type = oper;
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
}

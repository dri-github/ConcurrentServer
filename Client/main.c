#include <WinSock2.h>
#include <Windows.h>
#include <stdio.h>

#pragma comment(lib, "WS2_32.lib")
#pragma warning(disable: 4996)

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
		return -1;

	while (TRUE) {
		SOCKET s;
		if ((s = socket(AF_INET, SOCK_STREAM, NULL)) == INVALID_SOCKET)
			return -1;

		CHAR ipText[16];
		memset(ipText, 0, sizeof(ipText));

		printf("Target IP: ");
		if (!scanf("%s", ipText))
			return 1;
		ipText[sizeof(ipText) - 1] = '\0';

		SOCKADDR_IN addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(2000);

		struct hostent* tHost;
		if ((tHost = gethostbyname(ipText)) != NULL) {
			int i = 0;
			while (tHost->h_addr_list[i] != NULL) {
				addr.sin_addr.s_addr = *(long*)tHost->h_addr_list[i++];
				if (connect(s, &addr, sizeof(addr)) != SOCKET_ERROR)
					break;
			}

			if (i >= 0 && tHost->h_addr_list[--i] == NULL) {
				printf("Connection error\n");
				continue;
			}
		}
		else {
			addr.sin_addr.s_addr = inet_addr(ipText);
			if (connect(s, &addr, sizeof(addr)) != SOCKET_ERROR) {
				printf("Connection error\n");
				continue;
			}
			else {
				if (!strcmp(ipText, "*")) {
					SOCKET sUDP;
					if ((sUDP = socket(AF_INET, SOCK_DGRAM, NULL)) == INVALID_SOCKET) {
						printf("UDP socket error\n");
						continue;
					}

					BOOL trueflag = TRUE;
					if (setsockopt(sUDP, SOL_SOCKET, SO_BROADCAST,
						(char*)&trueflag, sizeof(trueflag)) < 0)
						continue;

					CHAR ipBroadcast[20];
					printf("\nFor connect by server call\n");
					printf("Broadcast address: ");
					if (!scanf("%s", ipBroadcast))
						return 1;

					SOCKADDR_IN addrIndex;
					addrIndex.sin_family = AF_INET;
					addrIndex.sin_port = htons(2000);
					addrIndex.sin_addr.s_addr = inet_addr(ipBroadcast);
					
					CHAR req[16] = "index";
					sendto(sUDP, req, strlen(req) + 1, NULL, (struct sockaddr_in*)&addrIndex, sizeof(addrIndex));

					SOCKADDR_IN clnt;
					memset(&clnt, 0, sizeof(clnt));
					SIZE_T lc = sizeof(clnt);
					if (recvfrom(sUDP, req, sizeof(req), NULL, (struct sockaddr*)&clnt, &lc) == SOCKET_ERROR) {
						printf("UDP recvfrom error\n");
						continue;
					}

					addr.sin_addr.s_addr = clnt.sin_addr.s_addr;
					if (connect(s, &addr, sizeof(addr)) == SOCKET_ERROR) {
						printf("Connection by UDP error\n");
						continue;
					}
				}
			}
		}

		CHAR serviceName[16];
		memset(serviceName, 0, sizeof(serviceName));
		printf("Service: ");
		if (!scanf("%s", serviceName))
			return 1;
		printf("========================================\n");

		send(s, serviceName, strlen(serviceName) + 1, NULL);

		CHAR serviceNameRecv[64];
		memset(serviceNameRecv, 0, sizeof(serviceNameRecv));
		if (recv(s, serviceNameRecv, sizeof(serviceNameRecv), NULL) == SOCKET_ERROR) {
			printf("Recv server error\n");
			continue;
		}

		if (strcmp(serviceName, serviceNameRecv) != 0) {
			printf("%s\n", serviceNameRecv);
			continue;
		}

		recv(s, serviceNameRecv, sizeof(serviceNameRecv), NULL);
		CHAR message[100] = "Start";

		getchar();
		do {
			if (!strcmp(message, "\0"))
				break;

			printf("Message: %s\n", message);
			//memset(message, 0, sizeof(message));
			printf("\n");
			printf("Send: ");
			fgets(message, sizeof(message), stdin);

			for (char* it = message - 1; (*it) != '\0'; it++) {
				if ((*it) == '\n') {
					(*it) = '\0';
					break;
				}
			}

			if (send(s, message, strlen(message) + 1, NULL) == SOCKET_ERROR)
				break;

			memset(message, 0, sizeof(message));
		} while (recv(s, message, sizeof(message), NULL) != SOCKET_ERROR);

		printf("Close connection\n");
		if (closesocket(s) == SOCKET_ERROR)
			return -1;
	}

	if (WSACleanup() == SOCKET_ERROR)
		return -1;

	return 0;
}
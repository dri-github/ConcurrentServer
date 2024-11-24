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

		CHAR serviceName[16];
		memset(serviceName, 0, sizeof(serviceName));
		printf("Service: ");
		if (!scanf("%s", serviceName))
			return 1;
		printf("========================================\n");

		SOCKADDR_IN addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(2000);
		addr.sin_addr.s_addr = inet_addr(ipText);

		if (connect(s, &addr, sizeof(addr)) == SOCKET_ERROR)
			return -1;

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

		do {
			if (!strcmp(message, "\0"))
				break;

			printf("Message: %s\n", message);
			memset(message, 0, sizeof(message));
			printf("\n");
			printf("Send: ");
			//gets(message);
			scanf("%s", message);
			if (!strcmp(message, "\\0"))
				strcpy(message, "");
			message[63] = '\0';

			if (send(s, message, strlen(message) + 1, NULL) == SOCKET_ERROR)
				break;
		} while (recv(s, message, sizeof(message), NULL) != SOCKET_ERROR);

		printf("Close connection\n");
		if (closesocket(s) == SOCKET_ERROR)
			return -1;
	}

	if (WSACleanup() == SOCKET_ERROR)
		return -1;

	return 0;
}
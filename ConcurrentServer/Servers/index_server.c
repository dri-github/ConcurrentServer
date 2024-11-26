#include "index_server.h"

DWORD WINAPI IndexServer(LPVOID lpParam)
{
	SOCKET s;
	if ((s = socket(AF_INET, SOCK_DGRAM, NULL)) == INVALID_SOCKET) {
		printf("[IndexServer] Status: Error create socket\n");
		return -1;
	}

	SOCKADDR_IN serv;
	serv.sin_family = AF_INET;
	serv.sin_port = htons(2000);
	serv.sin_addr.s_addr = INADDR_ANY;

	if (bind(s, (LPSOCKADDR)&serv, sizeof(serv)) == SOCKET_ERROR) {
		printf("[IndexServer] Status: Error bind\n");
		return -1;
	}

	while (TRUE) {
		CHAR ibuf[16];
		SOCKADDR_IN from;
		INT lc = sizeof(from);
		printf("[IndexServer] Status: Wait connection\n");
		if (recvfrom(s, ibuf, sizeof(ibuf), NULL, (struct sockaddr_in*)&from, &lc) == SOCKET_ERROR)
			continue;

		printf("[IndexServer] Status: Compleate connection\n");
		if (sendto(s, ibuf, sizeof(ibuf), NULL, (struct sockaddr_in*)&from, lc) == SOCKET_ERROR)
			continue;
	}

	closesocket(s);

    return 0;
}

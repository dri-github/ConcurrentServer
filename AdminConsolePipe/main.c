#include <WinSock2.h>
#include <Windows.h>
#include <stdio.h>

#pragma comment(lib, "WS2_32.lib")
#pragma warning(disable: 4996)

int main(int argc, char* argv[]) {
	//std::string target_name;
	//std::cout << "Write target name: ";
	//std::cin >> target_name;
	//target_name.insert(0, "\\\\");
	//target_name += "\\pipe\\Tybe";

	while (TRUE) {
		CHAR hostname[100] = "\\\\";
		LPSTR lpHost = hostname + sizeof(CHAR) * 2;
		printf("Hostname: ");
		scanf("%s", lpHost);
		
		strcat(hostname, "\\pipe\\ConcurrentServer");

		HANDLE hPipe;
		if ((hPipe = CreateFileA(hostname, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL)) == INVALID_HANDLE_VALUE) {
			printf("Connection error. Server not exist\n");
			continue;
		}

		printf("Connected to the server\n");
		while (TRUE) {
			CHAR buffer[100];
			printf("Command: ");
			scanf("%s", buffer);

			DWORD write_size = 0;
			if (!WriteFile(hPipe, buffer, sizeof(buffer), &write_size, NULL)) {
				break;
			}

			if (!strcmp(buffer, "shutdown") || !strcmp(buffer, "exit")) {
				break;
			}

			CHAR result[64];
			memset(result, 0, sizeof(result));
			DWORD result_size = 0;
			ReadFile(hPipe, result, write_size, &result_size, NULL);

			printf(result);
		}

		printf("Connection lost\n");
		DisconnectNamedPipe(hPipe);
		CloseHandle(hPipe);
	}
	return 0;
}
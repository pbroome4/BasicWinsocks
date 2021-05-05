#include <stdio.h>

#include <winsock2.h>
#include <WS2tcpip.h>
#include <windows.h> //after winsock2.h so winsock.h isnt included
#include <Winbase.h>

#pragma comment(lib, "Ws2_32.lib");

#define SERVER_IP "172.31.43.33"
#define DEFAULT_PORT "9999"

/*
* prints out string of getLastError()
*/
void print_getLastError() {
	DWORD flags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER;
	LPCVOID source = NULL; // ignored due to choice of flags
	DWORD message_id = GetLastError();
	DWORD language_id = 0;
	WCHAR* buffer;
	DWORD size = 256;

	int test = FormatMessage(flags, source, message_id, language_id, &buffer, size, NULL); // formats the message into buffer
	if (test < 0) {
		printf("func failed\n");
		exit(1);
	}


	printf("%S\n", buffer);
}


int main() {
	int test;

	/* Winsock init */
	printf("setting up winsock\n");
	WSADATA wsa_data;
	test = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (test != 0) {
		printf("WSAStartup() failed\n");
		print_getLastError();
		WSACleanup();
		exit(1);
	}

	/* create a addrinfo to hold hints and result socket addr info*/
	printf("resolving server address and port\n");
	struct addrinfo hints,
		* result,
		* ptr;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;


	/* Get addr info for server*/
	test = getaddrinfo(SERVER_IP, DEFAULT_PORT, &hints, &result);
	if (test != 0) {
		printf("getaddrinfo failed\n");
		print_getLastError();
		WSACleanup();
		exit(1);
	}


	/* create Client socket*/
	printf("creating client socket\n");
	ptr = result; // the first addrinfo returned by getaddrinfo
	SOCKET client_socket = INVALID_SOCKET;
	client_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
	if (client_socket == INVALID_SOCKET) {
		printf("socket failed\n");
		print_getLastError();
		freeaddrinfo(&hints);
		WSACleanup();
		exit(1);
	}


	/* Connect to server */
	printf("connecting to server\n");
	test = connect(client_socket, ptr->ai_addr, ptr->ai_addrlen);
	if (test == SOCKET_ERROR) {		//should instead try the next addrinfo in result from getaddrinfo()
		printf("connect() failed\n");
		print_getLastError();
		closesocket(client_socket);
		client_socket = INVALID_SOCKET;
	}
	// here, maybe consider trying another addrinfo in result ???
	freeaddrinfo(result);
	if (client_socket == INVALID_SOCKET) {
		printf("unable to connect to server\n");
		WSACleanup();
		exit(1);
	}



	/* Continously send data to client */
	printf("sending data to server\n");
#define DEFAULT_BUFFLEN 512
	char send_buffer[DEFAULT_BUFFLEN];
	int send_result;
	char* read_result;
	do {
		ZeroMemory(send_buffer, sizeof(send_buffer));
		read_result = fgets(&send_buffer, DEFAULT_BUFFLEN, stdin);
		if (read_result == NULL) {
			break;
		}
		//sprintf_s(send_buffer, DEFAULT_BUFFLEN, "this is some sample data\n");
		send_result = send(client_socket, send_buffer, strlen(send_buffer), 0);
		if (send_result == -1) {
			printf("send() failed\n");
			print_getLastError();
			closesocket(client_socket);
			WSACleanup();
			exit(1);
		}
	} while (1);

	/* Shutdown client */
	printf("shutting down client");
	test = shutdown(client_socket, SD_SEND);
	if (test == SOCKET_ERROR) {
		printf("shutdown failed\n");
		print_getLastError();
		closesocket(client_socket);
		WSACleanup();
		exit(1);
	}


	/* Clean up*/
	closesocket(client_socket);
	WSACleanup();
}
#include <stdio.h>

#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")
#define DEFAULT_PORT "9999"




int main() {

	/* start up winsock2 */
	printf("starting up winsock2\n");
	WSADATA wsa_data;
	int test = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (test != 0) {
		printf("WSAStartup() failed\n");
		exit(1);
	}


	/* resolve address and port to use by server */
	printf("resolving socket details. port: %s\n", DEFAULT_PORT);
	struct addrinfo* result = NULL;
	struct addrinfo* ptr = NULL;
	struct addrinfo hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;


	test = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (test != 0) {
		printf("getaddrinfo() failed with error %d\n", WSAGetLastError());
		WSACleanup();
		exit(1);
	}
	ptr = result;


	/* create a socket to listen for client connections*/
	printf("setting up server\n");
	SOCKET listen_socket = INVALID_SOCKET;
	listen_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
	if (listen_socket == INVALID_SOCKET) {
		printf("socket() failed with error %d\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		exit(1);
	}

	/* bind the socket to an address/port */
	printf("binding server socket\n");
	test = bind(listen_socket, ptr->ai_addr, ptr->ai_addrlen);
	if (test == SOCKET_ERROR) {
		printf("bind() failed with error %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(listen_socket);
		WSACleanup();
		exit(1);
	}
	freeaddrinfo(result); //no longer needed after bind



	/* listen for connections*/
	printf("attempting to listen for connections\n");
	test = listen(listen_socket, 10);
	if (test == SOCKET_ERROR) {
		printf("listen() failed with error %d\n", WSAGetLastError());
		closesocket(listen_socket);
		WSACleanup();
		exit(1);
	}


	/* Accept connections*/
	printf("listening for connections\n");
	SOCKET client_socket;
	client_socket = accept(listen_socket, NULL, NULL);
	if (client_socket == INVALID_SOCKET) {
		printf("accept() failed with error %d\n", WSAGetLastError());
		closesocket(listen_socket);
		WSACleanup();
		exit(1);
	}


	/* Recieve data */
	printf("client connected\n");
#define DEFAULT_BUFFLEN 512
	char recv_buffer[DEFAULT_BUFFLEN];
	int recv_result, send_result;
	do {
		ZeroMemory(recv_buffer, sizeof(recv_buffer));
		recv_result = recv(client_socket, recv_buffer, DEFAULT_BUFFLEN, 0);
		if (recv_result == -1) {
			printf("recv() failed %d\n", WSAGetLastError());
			WSACleanup();
			exit(1);
		}
		else if (recv_result == 0) {
			printf("closing connection\n");
			closesocket(client_socket);
		}
		else if (recv_result > 0) {
			printf("RECIEVED: %s", recv_buffer);
		}
	} while (recv_result > 0);

}
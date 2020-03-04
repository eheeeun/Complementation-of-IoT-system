#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <process.h>

#pragma comment (lib , "ws2_32.lib")

#define BUFSIZE 512
#define PORT 4000

DWORD WINAPI ProcessClient(LPVOID arg) {
	SOCKET client_sock = (SOCKET)arg;
	char buf[BUFSIZE + 1];
	SOCKADDR_IN clientaddr;
	int addrlen;
	int retval;

	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (SOCKADDR*)&clientaddr, &addrlen);

	// Ŭ���̾�Ʈ�� ������ ���
	while (1) {
		retval = recv(client_sock, buf, BUFSIZE, 0);
		if (retval == SOCKET_ERROR){ break; }
		else if (retval == 0) break;

		// ���� ������ ���
		buf[retval] = '\0';
		printf("[TCP /%s:%d] %s\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port), buf);

		// ������ ������
		retval = send(client_sock, buf, retval, 0);

		if (retval == SOCKET_ERROR){
			printf("send() error\n");
			break;
		}
	}
	// closesocket()
	closesocket(client_sock);
	printf("TCP ����, Ŭ���̾�Ʈ ���� : IP �ּ� = %s, ��Ʈ��ȣ = %d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

	return 0;
}

int main(){
	// �����ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return -1;

	// ���� �� �����ų ����
	int return_val;

	// socket() 
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) printf("����() ������\n");

	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(PORT);

	/*
	XP �������� INADDR_ANY�� ���� �� ��Ʈ���� �Ѱ��� ��� INADDR_ANY�� ���õǾ�����,
	2003 ���ķδ� INADDR_NONE�� �������� �Ѱ��ش�, INADDR_NONE�� IP�뿪�� A.B.C.D �� �ϳ��� 255�� �ʰ� �� ��� ����
	INADDR_ANY�� ��� �ּҷ� �����ϴ� ������ �޾Ƶ��δ�.
	*/

	serveraddr.sin_addr.s_addr = htons(INADDR_ANY);
	return_val = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr)); // connect�� �ƴ϶� bind

	if (return_val == SOCKET_ERROR) printf("bind() error\n");

	// listen()
	return_val = listen(listen_sock, SOMAXCONN);
	if (return_val == SOCKET_ERROR) printf("listen() error\n");

	// ������ ��ſ� ����� ����
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	char buf[BUFSIZE + 1];
	int addrlen;

	HANDLE hThread;              // ������ �ڵ�
	DWORD ThreadID;              // ������ ���̵�
	
	// Ŭ���̾�Ʈ�� ������ ��� 
	while (1){ 
		addrlen = sizeof(clientaddr);
		// accept()
		client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			printf("accept() ����\n");
			continue;
		}
		printf("TCP ����, Ŭ���̾�Ʈ ���� : IP �ּ� = %s, ��Ʈ��ȣ = %d\n", 
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		hThread = CreateThread(NULL, 0, ProcessClient, (LPVOID)client_sock, 0, &ThreadID);
		if (hThread == NULL) { printf("failed to create thread...\n"); }
		else{ CloseHandle(hThread);}
	}
	closesocket(listen_sock);

	// ���� ����
	WSACleanup();
	return 0;

}
//��ó: https://wonjayk.tistory.com/156 [����ļ� ��԰� ���� ��α�]

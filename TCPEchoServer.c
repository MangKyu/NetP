#include <stdio.h> /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h> /* for sockaddr_in and inet_addr() */
#include <stdlib.h> /* for atoi() and exit() */
#include <string.h> /* for memset() */
#include <unistd.h> /* for close() */
#include "ProtocolTCP.h"
#define MAXPENDING 5 /* Maximum outstanding connection requests */

void DieWithError(char *errorMessage); /* Error handling function */
void HandleTCPClient(int clntSocket); /* TCP client handling function */
void connMessage(int clntSocket);
#define echoServPort 55555 /*Server port */


int main(int argc, char *argv[]){
	int servSock; /* Socket descriptor for server */
	int clntSock; /* Socket descriptor for client */
	struct sockaddr_in echoServAddr; /* Local address */
	struct sockaddr_in echoClntAddr; /* Client address */
	unsigned int clntLen; /* Length of client address data structure */
	
	/* Create socket for incoming connections */
	if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		DieWithError("socket() failed");
	
	/* Construct local address structure */
	memset(&echoServAddr, 0, sizeof(echoServAddr)); /* Zero out structure */
	echoServAddr.sin_family = AF_INET; /* Internet address family */
	echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
	echoServAddr.sin_port = htons(echoServPort); /* Local port */
	
	/* Bind to the local address */
	if (bind(servSock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
		DieWithError("bind() failed");
	
	/* Mark the socket so it will listen for incoming connections */
	if (listen(servSock, MAXPENDING) < 0)
		DieWithError("listen() failed");

	for (;;) /* Run forever */{
		/* Set the size of the in-out parameter */
		clntLen = sizeof(echoClntAddr);
		/* Wait for a client to connect */
		if ((clntSock = accept(servSock, (struct sockaddr *) &echoClntAddr,&clntLen)) < 0)
			DieWithError("accept() failed");
		/* clntSock is connected to a client! */
		printf("Client Ip: %s\n", inet_ntoa(echoClntAddr.sin_addr));
		printf("Port: %d\n",echoClntAddr.sin_port);
		connMessage(clntSock);
		HandleTCPClient(clntSock);
	}
	close(clntSock);
	close(servSock);
	/* NOT REACHED */
}

void connMessage(int clntSocket){
	char buff[RCVBUFSIZE]; /* Buffer for echo string */
	int recvMsgSize = 0; /* Size of received message */

	/* Receive message from client */
	if ((recvMsgSize = recv(clntSocket, buff, RCVBUFSIZE, 0)) < 0)
		DieWithError("recv() failed");
	
	/* Send received string and receive again until end of transmission */
	buff[recvMsgSize] = '\0';
	printf("Received[conn]: %s\n",buff);
	strcpy(buff, "hi");
	recvMsgSize = strlen(buff);	
	if (send(clntSocket, buff, strlen(buff), 0)<0 )
   	DieWithError("send() failed");

	printf("Send[conn]: %s\n",buff);

}

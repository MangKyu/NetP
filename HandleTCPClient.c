#include <stdio.h> /* for printf() and fprintf() */
#include <sys/socket.h> /* for recv() and send() */
#include <unistd.h> /* for close() */
#include "ProtocolTCP.h"
#include <string.h>
#include <arpa/inet.h>
void DieWithError(char *errorMessage); /* Error handling function */
void writeHistory(char *echoMsg);
void HandleTCPClient(int clntSock);
void echoMessage(int clntSock);
char* recvMsg(int clntSock);
void recvFile(int clntSock);

void HandleTCPClient(int clntSock) {
	
	while (1){
		char *cmdStr = recvMsg(clntSock);
		printf("%s", cmdStr);
		if (!strcmp(cmdStr,EchoReq)){
			echoMessage(clntSock);
		}
		else if (!strcmp(cmdStr, FileUpReq)){
			recvFile(clntSock);
		}
		else if (!strcmp(cmdStr, FileDnReq)){
			printf("FileDnReq\n");
		}
		else if (!strcmp(cmdStr, LsReq)){
			printf("LsReq\n");
		}
		else if (!strcmp(cmdStr, RlsReq)){
			printf("RlsReq\n");
		}
		else {
			printf("Wait for another Client");
			break;
		}
	}
	close(clntSock); /* Close client socket */
}

void recvFile(int clntSock){
	char* fName;//[FileName];
	size_t fSize = 0, recvSize = 0, bufSize = 0;
	char buf[RCVBUFSIZE];
	FILE *fp;
	char path[FileName] = "./";

	/*get file name*/
	fName = recvMsg(clntSock);
	printf("Received: %s \n", fName);
	strcat(path, fName);

	/*get file size*/
	recv(clntSock, &fSize, sizeof(fSize), 0);
	printf("Received: %d bytes\n", fSize);


	/* check whether file is exist */
	if (access(path, F_OK) == 0){
		strcpy(fName, "temp.txt");
	}

	fp = fopen(fName, "wb");
	if (fp == NULL)
		DieWithError("File Open Error");
	
	/* receive the file*/
	bufSize = RCVBUFSIZE;
	while (fSize != 0){
		if (fSize < RCVBUFSIZE)
			bufSize = fSize;

		recvSize = recv(clntSock, buf, bufSize, 0);
		fSize -= recvSize;
		fwrite(buf, sizeof(char), recvSize, fp);
		recvSize = 0;
	}

	fclose(fp);
}

void echoMessage(int clntSock){

	char echoBuffer[RCVBUFSIZE]; /* Buffer for echo string */
	int recvMsgSize; /* Size of received message */

	if ((recvMsgSize = recv(clntSock, echoBuffer, RCVBUFSIZE, 0)) < 0)
		DieWithError("recv() failed");

	echoBuffer[recvMsgSize] = '\0';
	printf("Received: %s\n", echoBuffer);
	writeHistory(echoBuffer);
	if (send(clntSock, echoBuffer, recvMsgSize, 0) != recvMsgSize)
		DieWithError("send() failed");
	printf("Send: %s\n", echoBuffer);
}

/* write echo_history log*/
void writeHistory(char *echoMsg){
	
	FILE *fp = fopen("echo_history.log","a");
	if(fp == NULL)
		DieWithError("fopen() failed");

	fprintf(fp,"%s%s",echoMsg,"\r\n");

	fclose(fp);
}
/*receive message and return msg*/
char* recvMsg(int sock){
	char buff[RCVBUFSIZE];
	int recvMsgSize; /* Size of received message */

	if ((recvMsgSize = recv(sock, buff, RCVBUFSIZE, 0)) < 0)
		DieWithError("recv() failed");
	buff[recvMsgSize] = '\0';

	char *rtnBuff = buff;
	return rtnBuff;
}
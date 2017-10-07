#include <stdio.h> /* for printf() and fprintf() */
#include <sys/socket.h> /* for recv() and send() */
#include <unistd.h> /* for close() */
#include "ProtocolTCP.h"
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
void DieWithError(char *errorMessage); /* Error handling function */
void writeHistory(char *echoMsg);
void HandleTCPClient(int clntSock);
void echoMessage(int clntSock);
char* recvMsg(int clntSock);
void recvFile(int clntSock);
void sendFile(int clntSock);
int getFileSize(FILE *fp);
char* sendMsg(int sock, char *str, char *errorMsg);

void HandleTCPClient(int clntSock) {
	
	while (1){
		char* cmdStr = recvMsg(clntSock);
		if (!strcmp(cmdStr,EchoReq)){
			echoMessage(clntSock);
		}
		else if (!strcmp(cmdStr, FileUpReq)){
			recvFile(clntSock);
		}
		else if (!strcmp(cmdStr, FileDnReq)){
			sendFile(clntSock);
			getchar();
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

/* get file size*/
int getFileSize(FILE *fp){
	int fsize = 0;
	fseek(fp, 0, SEEK_END);
	fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	return fsize;
}

void sendFile(int clntSock){
	FILE *fp;
	int fSize = 0, totalSize = 0, sendSize = 0;
	char fName[FileName];
	char buf[RCVBUFSIZE];

	/*enter file name*/
	printf("Filename to put to server -> ");
	scanf("%s", fName);

	/* open file*/
	fp = fopen(fName, "rb");
	if (fp == NULL)
		DieWithError("File Open Error");


	/*send file name*/
	sendMsg(clntSock, fName, "FileName send Error");

	/*send file size as char* */
	fSize = getFileSize(fp);
	char fsize[RCVBUFSIZE];
	sprintf(fsize, "%d", fSize);
	sendMsg(clntSock, fsize, "Send Size Error");

	printf("Sending => ###############\n");

	/*send file content*/
	while (totalSize != fSize){
		sendSize = fread(buf, 1, RCVBUFSIZE, fp);
		totalSize += sendSize;
		send(clntSock, buf, sendSize, 0);
	}
	printf("%s(%d bytes) uploading success \n", fName, totalSize);

	fclose(fp);
}


/*send message to Client*/
char* sendMsg(int sock, char *str, char *errorMsg){
	printf("Send: ");
	if (str == NULL){
		str = (char *)malloc(sizeof(str));
		scanf("%s", str);
	}
	else{
		printf("%s \n", str);
	}

	if (send(sock, str, strlen(str), 0) <0)
		DieWithError(errorMsg);

	return str;
}

void recvFile(int clntSock){
	char* fName;//[FileName];
	int fSize = 0, recvSize = 0, bufSize = 0;
	char buf[RCVBUFSIZE];
	FILE *fp;
	char path[FileName] = "./";
	char* fsize;
	int n = 0;

	/*get file name and file size*/
	fName = recvMsg(clntSock);
	strcat(path, fName);

	fsize = recvMsg(clntSock);
	fSize = atoi(fsize);


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

	if ((recvMsgSize = recv(clntSock, echoBuffer, RCVBUFSIZE-1, 0)) < 0)
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
	char *rtnBuf;
	int recvMsgSize; /* Size of received message */

	if ((recvMsgSize = recv(sock, buff, RCVBUFSIZE-1, 0)) < 0)
		DieWithError("recv() failed");
	buff[recvMsgSize] = '\0';

	printf("Received: %s \n", buff);
	rtnBuf = buff;
	return rtnBuf;
}
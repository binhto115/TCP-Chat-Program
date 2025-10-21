// function prototypes for sendPDU() and recvPDU() 


#ifndef __FUNCTION_PROTOTYPE_H__
#define __FUNCTION_PROTOTYPE_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

// sendPDU
int sendPDU(int clientSocket, uint8_t *dataBuffer, int LengthOfData);

// recvPDU
int recvPDU(int socketNumber, uint8_t *dataBuffer, int bufferSize);

// Add new socket function
void addnewSocket(int newServer);

// process current client
int processClient(int ServerPollCall);

// serverControl
void serverControl(int mainServerSocket);//, int portNumber);

// void readFromStdin
int readFromStdin(uint8_t * buffer);

// processFromServer
int processFromServer(int socketNum, char *handle_name);

// client Control
void clientControl(int socketNum, char *handle_name);

int getLastAddedSocket();

int getSenderInfo(uint8_t *dataBuffer, char *senderName);

void commandBroadcast(uint8_t *dataBuffer, int messageLen);

void commandUnicast(uint8_t *dataBuffer, int messageLen);

void commandMulticast(uint8_t *dataBuffer, int messageLen);

void commandList(int clientSocket);

void handleUnicastCommand(uint8_t *inputBuffer, int inputLen, int socketNum, char *handle_name);

void handleBroadcastCommand(uint8_t *inputBuffer, int inputLen, int socketNum, char *handle_name);

void handleMulticastCommand(uint8_t *inputBuffer, int inputLen, int socketNum, char *handle_name);

void processCommand(uint8_t *inputBuffer, int inputLen, int socketNum, char *handle_name);

int getSenderNameForPrint(uint8_t *dataBuffer, int offset, char *senderName);

void printBroadcast(uint8_t *dataBuffer, int offset);

void printUnicast(uint8_t *dataBuffer, int offset);

void printMulticast(uint8_t *dataBuffer, int offset);

void printError7(uint8_t *dataBuffer, int offset);

void printHandleCount(uint8_t *dataBuffer, int offset);

void printList(uint8_t *dataBuffer, int offset); 








#endif

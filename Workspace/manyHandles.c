/******************************************************************************
* myClient.c
*
* Writen by Prof. Smith, updated Jan 2023
* Use at your own risk.  
*
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdint.h>
#include "pollLib.h"
#include "networks.h"
#include "safeUtil.h"
#include "functionPrototype.h"
#define MAXBUF 1024
#define DEBUG_FLAG 1
#define MAX_CLIENTS 300

int sockets[MAX_CLIENTS];

void registerHandle(int socketNum, char *handleName);
void checkArgs(int argc, char * argv[]);

int main(int argc, char * argv[]) {
	int socketNum = 0;         //socket descriptor
	
	checkArgs(argc, argv);
	
	setupPollSet();
	
	for (int i = 0; i < MAX_CLIENTS; i++) {
		char handleName[50];
		sprintf(handleName, "test%d", i);

		/* set up the TCP Client socket  */
		socketNum = tcpClientSetup(argv[1], argv[2], DEBUG_FLAG);
		sockets[i] = socketNum;
		addToPollSet(socketNum);
		
		registerHandle(socketNum, handleName);
	}
	printf("Finished registering %d handles! Blocking forever...\n", MAX_CLIENTS);
	while (1) { 
		pollCall(-1);
	}
		clientControl(socketNum, argv[1]);	
		
	return 0;
}

void registerHandle(int socketNum, char *handleName) {
        uint8_t buffer[MAXBUF];
        int offset = 0;

        buffer[offset++] = 1;  // FLAG_1
        buffer[offset++] = strlen(handleName); // handle length
        memcpy(&buffer[offset], handleName, strlen(handleName));
        offset += strlen(handleName);

        sendPDU(socketNum, buffer, offset);
        uint8_t response[MAXBUF];
        int respLen = recvPDU(socketNum, response, MAXBUF);

        if (respLen > 0 && response[0] == 2) { // FLAG_2
                printf("Registered handle: %s on socket %d\n", handleName, socketNum);
        } else if (response[0] == 3) { // FLAG_3
                printf("Handle already exists: %s\n", handleName);
        } else {
                printf("Unexpected server response.\n");
        }
}

void checkArgs(int argc, char * argv[]) {
	//check command line arguments
	if (argc != 3) {
		printf("usage: %s handle-name host-name port-number \n", argv[0]);
		exit(-1);
	}
	
}	

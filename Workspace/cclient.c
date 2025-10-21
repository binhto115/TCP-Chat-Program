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

#include "networks.h"
#include "safeUtil.h"
#include "functionPrototype.h"
#define MAXBUF 1024
#define DEBUG_FLAG 1

void checkArgs(int argc, char * argv[]);

int main(int argc, char * argv[]) {
	// ----- Socket Descriptor -----
	int socketNum = 0;     
	
	// ----- Check Number of Command-line Arguments -----
	checkArgs(argc, argv);

	// ----- set up the TCP Client socket -----
	socketNum = tcpClientSetup(argv[2], argv[3], DEBUG_FLAG);
	
	// ----- Calling ClientControl -----	
	clientControl(socketNum, argv[1]);	
	
	return 0;
}

void checkArgs(int argc, char * argv[]) {
	// ----- Check command line arguments -----
	if (argc != 4) {
		printf("usage: %s handle-name host-name port-number \n", argv[0]);
		exit(-1);
	}
	
}	

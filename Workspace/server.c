/******************************************************************************
* myServer.c
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
#include "handleTable.h"
#define MAXBUF 1024
#define DEBUG_FLAG 1

void recvFromClient(int clientSocket);
int checkArgs(int argc, char *argv[]);

int main(int argc, char *argv[]) {
	int mainServerSocket = 0;   //socket descriptor for the server socket
	int portNumber = 0;
	
	// ----- Check Arguments -----
	portNumber = checkArgs(argc, argv);
	
	// ----- Create the Server Socket -----
	mainServerSocket = tcpServerSetup(portNumber);
	
	// ----- Initialize Handle Table -----
	handle_table();
	
	// ----- Server Control -----
	serverControl(mainServerSocket);
	free_table();
	return 0;
}

int checkArgs(int argc, char *argv[]) {
	// ----- Checks args and Returns Port Number -----
	int portNumber = 0;
	if (argc > 2) {
		fprintf(stderr, "Usage %s [optional port number]\n", argv[0]);
		exit(-1);
	}
		
	// ----- Input Port Number -----
	if (argc == 2) {
		portNumber = atoi(argv[1]);
	}
	
	return portNumber;
}

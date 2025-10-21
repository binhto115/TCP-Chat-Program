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
#include "pollLib.h"

#define MAXBUF 1400
#define MAX_MSG_LEN 199
#include "handleTable.h"
static int isHandleListDone = 0;

// ----- Flag values for packet formatting -----
#define FLAG_1 1
#define FLAG_2 2
#define FLAG_3 3
#define FLAG_4 4
#define FLAG_5 5 
#define FLAG_6 6 
#define FLAG_7 7
#define FLAG_8 8
#define FLAG_9 9
#define FLAG_10 10
#define FLAG_11 11
#define FLAG_12 12
#define FLAG_13 13

void addNewSocket(int newServer);
void processStdin(int socketNum, char* handle_name);

// From Lab 2
int sendPDU(int clientSocket, uint8_t *dataBuffer, int LengthOfData) {
	//  copies of PDU length in no and ho
	int PDU_length_in_hosts = LengthOfData + 2;	
	uint16_t PDU_length_in_networks = htons(PDU_length_in_hosts);
	uint8_t newBuffer[PDU_length_in_hosts]; // size of the new buffer is PUD length in networks

	// memcpy() data into the new buffer
	memcpy(newBuffer, &PDU_length_in_networks, sizeof(PDU_length_in_networks));
	memcpy(newBuffer + 2, dataBuffer, LengthOfData);//sizeof(dataBuffer));
 	
	ssize_t sendMSG = send(clientSocket, newBuffer, PDU_length_in_hosts, 0);	
	if (sendMSG < 0) {
		perror("Failed to send message.\n");
		exit(-1);
	} else if (sendMSG == 0) {
		printf("Connection closed, no bytes were sent.\n");
	}
	//close(clientSocket);
	return sendMSG; // before was -2, w/o makes sense cuz of the header.
}

int recvPDU(int socketNumber, uint8_t *databuffer, int bufferSize) {
	// Store received bytes for PDU Length
	uint8_t PDU_len_buffer[2];	
	
	// First Recv 
	ssize_t msg_received = recv(socketNumber, PDU_len_buffer, 2, MSG_WAITALL);
	if (msg_received < 0) {
		perror("MSG Receive Failed.\n");
		exit(-1);
	} else if (msg_received == 0) {
		printf("Connection closed.\n");
		return 0;
	} //else {
		//printf("MSG Receive Succeeded.\n");
	//}
	
	// convert PDU length to host
	uint16_t PDU_length_in_networks;
	memcpy(&PDU_length_in_networks, PDU_len_buffer, 2); // copy 2-bytes of PDU len from buffer to nhots() it
	uint16_t PDU_length_in_hosts = ntohs(PDU_length_in_networks);
	
	
	// Chec to see if payloadbuffer is large enough to receive the PDU
	if (bufferSize < (PDU_length_in_hosts-2)) {
		printf("BufferSize is not large enough to hold.\n");
		exit(-1);
	}

	// second recv
	ssize_t msg_received2 = recv(socketNumber, databuffer, PDU_length_in_hosts-2, MSG_WAITALL);
	if (msg_received2 < 0) {
		perror("MSG Receive2 Failed.\n");
		exit(-1);
	} else if (msg_received2 == 0) {
		printf("Connetion on Receive2 closed.\n");
	} //else {
		//printf("MSG Receive2 Succeeded.\n");
	//}
	return msg_received2; // return the payload length only
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ----- Server Control -----
void serverControl(int mainServerSocket) {//, int portNumber) {
	// Set up pollset
	setupPollSet();
	
	// Initializa Handle Table
	handle_table();
	check_size(); // Check the current size of the table

	// add main server socket to poll set
	addToPollSet(mainServerSocket);

	while (1) {
		int ServerPollCall = pollCall(-1); // blocks forever (until a socket ready)

		// Accept new client connection
		if (ServerPollCall == mainServerSocket) {
			addNewSocket(mainServerSocket); // accept new connection
		
			int newClient = getLastAddedSocket();
			
			// receive the first msg of the socket handle
			uint8_t client_handle_name[MAXBUF];
			int handle_len = recvPDU(newClient, client_handle_name, MAXBUF);
			
			if (handle_len > 0) {
				uint8_t checkFlag = client_handle_name[0];
				if (checkFlag == FLAG_1) {
					// Check if the handle exists or not
					uint8_t handleLen = client_handle_name[1]; // get the handleName
					char handleName[MAXBUF];

					// copy to handleName
					memcpy(handleName, &client_handle_name[2], handleLen);
					handleName[handleLen] = '\0';

					printf("handle: %s\n", handleName);
									
					if (look_up_socket_by_handle(handleName) == -1) {	
						add_handle(handleName, newClient); // add the client to the table
						check_size();
						printf("Socket: %d added!\n", look_up_socket_by_handle(handleName));
						printf("Handle: %s added!\n", look_up_handle_by_socket(newClient));
					
						// Send a flag 2 (good handle) message back
						uint8_t flag2 = FLAG_2;
						sendPDU(newClient, &flag2, sizeof(flag2)); 
					} else {
						uint8_t flag3 = FLAG_3; // error flag - handle exists!
						sendPDU(newClient, &flag3, sizeof(flag3));
					}	
				}	
			}		
		} else {
			// Handle current clients
			// if ^c, then print the two messages and remove/close the socket
			int connectedClient = processClient(ServerPollCall); // handle current client
			if (connectedClient <= 0) {
				printf("From Server Side - socket %d closed...\n", ServerPollCall);
				close(ServerPollCall);
				printf("From Server Side - socket %d removed....\n", ServerPollCall);
				removeFromPollSet(ServerPollCall);
				remove_handle(ServerPollCall);	
			}	
		}	
	}
}



static int lastAddedSocket = -1;

int getLastAddedSocket() {
    return lastAddedSocket;
}

void addNewSocket(int newServer) {
	int newClient = tcpAccept(newServer, 1);
	if (newClient >= 0) {
		lastAddedSocket = newClient;
		addToPollSet(newClient);
		printf("New client %d added.\n", newClient);
	}
}

// At the end of function
// --------------------------------- processClient Helper Functions ---------------------------------
// --------------------------------- processClient Helper Functions ---------------------------------
// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

int getSenderInfo(uint8_t *dataBuffer, char *senderName) {
	int offset = 1; // Skip flag
	uint8_t senderLen = dataBuffer[offset++];
	memcpy(senderName, &dataBuffer[offset], senderLen);
	senderName[senderLen] = '\0';
	offset += senderLen;
	return offset;
}

// ----- Helper for %B -----
void commandBroadcast(uint8_t *dataBuffer, int messageLen) {
	// ----- calling getSenderInfo -----
	char senderName[100];
	getSenderInfo(dataBuffer, senderName);
		
	// ----- Look up Sender -----
	int senderSocket = look_up_socket_by_handle(senderName);
	
	// ----- Look up other clients -----
	int handleCountHost = numOfActiveHandles();
	for (int i = 0; i < handleCountHost; i++) {
		const char *handle = getActiveHandle(i);
		int handleSocket = look_up_socket_by_handle(handle);
		if ((handle != NULL) && (strcmp(handle, senderName) != 0) && (handleSocket != senderSocket)) {
			sendPDU(handleSocket, dataBuffer, messageLen);
		}	
	}	
}

// ----- Helper for %M -----
void commandUnicast(uint8_t *dataBuffer, int messageLen) {
	// ----- calling getSenderInfo -----
	char senderName[100];
 	int offset = getSenderInfo(dataBuffer, senderName);
			
	int senderSocket = look_up_socket_by_handle(senderName); // look up sender socket
	offset++;  // Skip number of destinations
	
	// Get destination socket
	uint8_t destLen = dataBuffer[offset++];

	char destName[MAXBUF];
	memcpy(destName, &dataBuffer[offset], destLen);
	destName[destLen] = '\0';
	int destSocket = look_up_socket_by_handle(destName);
	

	if (destSocket != -1) {
		// Send message to the specified client
		sendPDU(destSocket, dataBuffer, messageLen);
	} else {
		// -----  ERROR Flag 7 to Sender
		uint8_t errorBuffer[MAXBUF];
		int errorOffset = 0;
		errorBuffer[errorOffset++] = FLAG_7;
				
		// ----- Error Sender Info -----
		errorBuffer[errorOffset++] = destLen;
		memcpy(&errorBuffer[errorOffset], destName, destLen);
		errorOffset += destLen;
	
		// ----- Send it -----
		sendPDU(senderSocket, errorBuffer, errorOffset);		
	}
}

// ----- Helper for %C -----
void commandMulticast(uint8_t *dataBuffer, int messageLen) {
	// ----- calling getSenderInfo -----
	char senderName[100];
 	int offset = getSenderInfo(dataBuffer, senderName);

	int senderSocket = look_up_socket_by_handle(senderName); // look up sender socket
			
	int numHandles = dataBuffer[offset++];
	uint8_t destLen[numHandles];
	char destName[numHandles][100]; // max 100 per name
			
	for (int i=0; i < numHandles; i++) {
		destLen[i] = dataBuffer[offset++];
		memcpy(destName[i], &dataBuffer[offset], destLen[i]);
		offset += destLen[i];
		destName[i][destLen[i]] = '\0';
			
		int destSocket = look_up_socket_by_handle(destName[i]);
		if (destSocket != -1) {
			// Send message to the specified client
			sendPDU(destSocket, dataBuffer, messageLen);
		} else {
			// Send flag_7 error packet back to sender
			uint8_t errorBuffer[MAXBUF];
			int errorOffset = 0;
			errorBuffer[errorOffset++] = FLAG_7;
					
			// ----- ERROR Destination Sender Info -----
			uint8_t nonExistHandleLen = strlen(destName[i]);
			errorBuffer[errorOffset++] = nonExistHandleLen;
			memcpy(&errorBuffer[errorOffset], destName[i], nonExistHandleLen);
			errorOffset += nonExistHandleLen;
					
			// ----- Send It -----				
			sendPDU(senderSocket, errorBuffer, errorOffset);
		}	
	} 		

}

// ----- Helper for %L -----
void commandList(int clientSocket) {
	// ----- Create Buffer to format Message -----
	uint8_t responseBuffer[5];
	responseBuffer[0] = FLAG_11;
		
	// ----- Byte-order Conversion -----
	int handleCountHost = numOfActiveHandles();
	int handleCountNetwork = htonl(handleCountHost);
	memcpy(&responseBuffer[1], &handleCountNetwork, sizeof(int));

	// ----- Send Flag 11 ------		
	sendPDU(clientSocket, responseBuffer, sizeof(int)+1); // int = 32 bits + 8 bits = 5 bytes
	
	// ----- Send Flag 12 ------
	for (int i = 0; i < handleCountHost; i++) {
		const char *flag12Handle = getActiveHandle(i);
		if (flag12Handle != NULL) {
			// ----- Buffer for Message Format -----
			uint8_t flag12Buffer[MAXBUF];
						
			// ----- Flag Offset -----
			int flag12Offset = 0;
			flag12Buffer[flag12Offset++] = FLAG_12;
					
			// ----- Handle Name -----
			uint8_t flag12Len = strlen(flag12Handle);
			flag12Buffer[flag12Offset++] = flag12Len;
					
			// ----- Handle -----
			memcpy(&flag12Buffer[flag12Offset], flag12Handle, flag12Len);
			flag12Offset += flag12Len;
					
			// ----- Send it -----
			sendPDU(clientSocket, flag12Buffer, flag12Offset);
		}
	}

	// ----- Send Flag 13 -----
	uint8_t flag13 = FLAG_13;
	sendPDU(clientSocket, &flag13, sizeof(flag13));
}

int processClient(int ServerPollCall) {
	uint8_t dataBuffer[MAXBUF];
	int messageLen = 0;
	
	// get msg from a client 
	if ((messageLen = recvPDU(ServerPollCall, dataBuffer, MAXBUF)) < 0) {
		perror("From Server Side - recvPDU call failed.\n");
		exit(-1);	
	}
		
	if (messageLen > 0) {
		// Message sent from client to server.
		printf("From Server Side - message received on socket %d, length: %d  Data: %s\n", 
			ServerPollCall, messageLen, dataBuffer);
		
		//  Forwarding the message to the client
		int offset = 0;
		int flag = dataBuffer[offset];
		if (flag == FLAG_4) { 
			commandBroadcast(dataBuffer, messageLen);		
		} else if (flag == FLAG_5) {
			commandUnicast(dataBuffer, messageLen);
		} else if (flag == FLAG_6) {
			commandMulticast(dataBuffer, messageLen);
		} else if (flag == FLAG_10) {
			commandList(ServerPollCall);
		}
		return messageLen;
	} else {
		// Client closes connection		
		printf("From Server Side - Socket %d: Connection closed by the other side\n", ServerPollCall);
		return 0;
	}
}

int readFromStdin(uint8_t *buffer) {
	char aChar = 0;
	int inputLen = 0;

	// Important you don't input more characters than you have space
	buffer[0] = '\0';
	//printf("Enter data: ");
	while (inputLen < (MAXBUF -1) && aChar != '\n') {
		aChar = getchar();
		if (aChar != '\n') {
			buffer[inputLen] = aChar;
			inputLen++;
		}
	}
	// null terminate the string
	buffer[inputLen] = '\0';
	inputLen++;

	return inputLen;
}

// --------------------------------- processCommand Helper Functions ---------------------------------
// --------------------------------- processCommand Helper Functions ---------------------------------
// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

void handleUnicastCommand(uint8_t *inputBuffer, int inputLen, int socketNum, char *handle_name) {
	uint8_t buildBuffer[MAXBUF];
    	int byte_offset = 0;

   	if (inputLen > MAXBUF) inputLen = MAXBUF - 1;
    	inputBuffer[inputLen] = '\0';

    	strtok((char *)inputBuffer, " ");
    	char *handle = strtok(NULL, " ");
    	char *msg = strtok(NULL, "");

	if (!handle || strlen(handle) > 100 || !msg || strlen(msg) == 0) {
        	printf("Invalid unicast command format.\n");
        	return;
   	}	

    	int messageTotalLength = strlen(msg);
	int messageLengthStart = 0;
    	int messageSegmentLength = 0;
	   
	while (messageLengthStart < messageTotalLength) {
        	byte_offset = 0;

		// ------ Breaking Message into 200-bytes -----
		if ((messageTotalLength - messageLengthStart) >= MAX_MSG_LEN) {
				messageSegmentLength = MAX_MSG_LEN;
		} else {
			messageSegmentLength = messageTotalLength - messageLengthStart;
		}

	        char messageSegment[200] = {0};
 	        strncpy(messageSegment, &msg[messageLengthStart], messageSegmentLength);
        	buildBuffer[byte_offset++] = FLAG_5;
	        buildBuffer[byte_offset++] = strlen(handle_name);
                memcpy(&buildBuffer[byte_offset], handle_name, strlen(handle_name));
        	byte_offset += strlen(handle_name);

        	buildBuffer[byte_offset++] = 1; // 1 destination
        	buildBuffer[byte_offset++] = strlen(handle);
       		memcpy(&buildBuffer[byte_offset], handle, strlen(handle));
       	 	byte_offset += strlen(handle);

        	strcpy((char *)&buildBuffer[byte_offset], messageSegment);
        	byte_offset += strlen(messageSegment) + 1;
        	buildBuffer[byte_offset] = '\0';

        	sendPDU(socketNum, buildBuffer, byte_offset);

        	messageLengthStart += messageSegmentLength;
    	}
}

void handleBroadcastCommand(uint8_t *inputBuffer, int inputLen, int socketNum, char *handle_name) {
	uint8_t buildBuffer[MAXBUF];
    	int byte_offset = 0;

   	if (inputLen > MAXBUF) inputLen = MAXBUF - 1;
    	inputBuffer[inputLen] = '\0';

	// ----- Strtokenize User Command ----- 
	strtok((char *)inputBuffer, " "); // Extract command
	char *msg = strtok(NULL, ""); // Extract text

	// ---- Check Command Format -----	
	if (!msg || (strlen(msg) == 0)) {
		printf("Invalid broadcast command\n");
		return;
	}
	// ----- Split Message ------
	int messageTotalLength = strlen(msg);
	int messageLengthStart = 0;
	int messageSegmentLength = 0;
	
	while (messageLengthStart < messageTotalLength) {
		byte_offset = 0; // Reset for each message 			
				
		// ------ Breaking Message into 200-bytes -----
		if ((messageTotalLength - messageLengthStart) >= MAX_MSG_LEN) {
			messageSegmentLength = MAX_MSG_LEN;
		} else {
			messageSegmentLength = messageTotalLength - messageLengthStart;
		}
				
		// ----- Putting Message in Segments -----
		char messageSegment[200]; // Buffer to hold message of size 200 bytes
		memset(messageSegment, 0, sizeof(messageSegment));
		strncpy(messageSegment, &msg[messageLengthStart], messageSegmentLength);
		
		messageSegment[messageSegmentLength] = '\0';
				
		// ----- Message Formatting -----
		buildBuffer[byte_offset++] = FLAG_4;
		buildBuffer[byte_offset++] = strlen(handle_name);
			
		// ----- Handle Name of The Sender -----
		memcpy(&buildBuffer[byte_offset], handle_name, strlen(handle_name));
		byte_offset += strlen(handle_name);
			
		// ----- Message -----
		strcpy((char *)&buildBuffer[byte_offset], messageSegment);
		byte_offset += strlen(messageSegment) + 1;
		buildBuffer[byte_offset] = '\0';

		// ----- Send it -----
		sendPDU(socketNum, buildBuffer, byte_offset);
			
		// ----- Increment the Next Segment -----
		messageLengthStart += messageSegmentLength; 
	}	
}

void handleMulticastCommand(uint8_t *inputBuffer, int inputLen, int socketNum, char *handle_name) {
	int byte_offset = 0;
	uint8_t buildBuffer[MAXBUF];
	if (inputLen > MAXBUF) {
		inputLen = MAXBUF - 1;
	}

	//  strtokenize the user command
	char *msgCommand = strtok((char *)inputBuffer, " "); // extract command
	if (!msgCommand || strlen(msgCommand) == 0) {
		printf("Invalid format\n");
		return;
	}	
	char *numHandleStr = strtok(NULL, " "); // Extract number of handles as string
	if (!numHandleStr) {
		printf("Incorrect command format. No handle counts\n");
		return;
	} else if ((numHandleStr == NULL) || (strlen(numHandleStr) == 0)) {
		printf("Invalid command format\n");
		return;
	}
		
	// Convert to integer	
	int numHandles = atoi(numHandleStr); // extract number of handles
	if (numHandles > 9 || numHandles < 2) {
		printf("Incorrect format.\n");
		return;
	}
		
	char *tokenHandles[numHandles]; // loop and store then handles
	for (int i = 0; i < numHandles; i++) {
		tokenHandles[i] = strtok(NULL, " ");
		if (!tokenHandles[i] || strlen(tokenHandles[i]) > 100 ) {
			printf("Invalid multicast command.\n");
			return;
		} else if (tokenHandles[i] == NULL) {
			printf("Invalid multicast command\n");
			return;	
		} else if (!tokenHandles[i]) {
			printf("Invalid multicast command\n");
			return;
		}
	}
		
	char *msg = strtok(NULL, ""); // Extract message
	if (!msg || strlen(msg) == 0) {
		printf("Invalid multicast format\n");
		return;
	}
	// Split message to 199-bytes length
	int messageTotalLength = strlen(msg);
	int messageLengthStart = 0;
	int messageSegmentLength = 0;
		
		
	while (messageLengthStart < messageTotalLength) {
		byte_offset = 0; // Reset for each message 			

		// ------ Breaking Message into 200-bytes -----
		if ((messageTotalLength - messageLengthStart) >= MAX_MSG_LEN) {
			messageSegmentLength = MAX_MSG_LEN;
		} else {
			messageSegmentLength = messageTotalLength - messageLengthStart;
		}
		// ----- Putting Message in Segments -----
		char messageSegment[200]; // buffer to hold message of size 200 bytes
		memset(messageSegment, 0, sizeof(messageSegment));
		strncpy(messageSegment, &msg[messageLengthStart], messageSegmentLength);
		messageSegment[messageSegmentLength] = '\0';

		// ------- BUILD MESSSAGE PACKET --------		
		// Message formatting
		buildBuffer[byte_offset++] = FLAG_6;
		
		buildBuffer[byte_offset++] = strlen(handle_name);			
		memcpy(&buildBuffer[byte_offset], handle_name, strlen(handle_name));
		byte_offset += strlen(handle_name);

		buildBuffer[byte_offset++] = numHandles;
		
		// Formatting each handle
		for (int i = 0; i < numHandles; i++) {
			buildBuffer[byte_offset++] = strlen(tokenHandles[i]);
			memcpy(&buildBuffer[byte_offset], tokenHandles[i], strlen(tokenHandles[i]));
			byte_offset += strlen(tokenHandles[i]);
		}
		
		// Message
		strcpy((char *)&buildBuffer[byte_offset], messageSegment);
		byte_offset += strlen(messageSegment) + 1;// for null-terminator	
		buildBuffer[byte_offset] = '\0'; // Add a null-terminator
	
		// Send it
		sendPDU(socketNum, buildBuffer, byte_offset);			
		messageLengthStart += messageSegmentLength; // Increment the next message segment
	}
}

// ----- Process Command -----
void processCommand(uint8_t *inputBuffer, int inputLen, int socketNum, char *handle_name) {
	if (inputLen > MAXBUF) {
		inputLen = MAXBUF - 1;
	}
	inputBuffer[inputLen] = '\0';

	if ((inputBuffer[0] == '%') && (inputBuffer[1] == 'M' || inputBuffer[1] == 'm')) {
		handleUnicastCommand(inputBuffer, inputLen, socketNum, handle_name);		
	} else if ((inputBuffer[0] == '%') && (inputBuffer[1] == 'B' || inputBuffer[1] == 'b')) { 
		handleBroadcastCommand(inputBuffer, inputLen, socketNum, handle_name);
	} else if ((inputBuffer[0] == '%') && (inputBuffer[1] == 'C' || inputBuffer[1] == 'c')) { 
		handleMulticastCommand(inputBuffer, inputLen, socketNum, handle_name);
	} else if ((inputBuffer[0] == '%') && (inputBuffer[1] == 'L' || inputBuffer[1] == 'l')) {	
		if (strlen((char*)inputBuffer) > 2) {
			printf("Invalid list command\n");
			return;
		}
		// ----- Message Formatting -----
		uint8_t flag10 = FLAG_10;
		sendPDU(socketNum, &flag10, sizeof(flag10));
	}
}

// --------------------------------- processFromServer Helper Functions ---------------------------------
// --------------------------------- processFromServer Helper Functions ---------------------------------
// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

int getSenderNameForPrint(uint8_t *dataBuffer, int offset, char *senderName) {
	uint8_t senderLen = dataBuffer[offset++]; // Skip Sender Length
	memcpy(senderName, &dataBuffer[offset], senderLen);
	senderName[senderLen] = '\0';
	offset += senderLen; // Skip sender
	return offset;	
}

void printBroadcast(uint8_t *dataBuffer, int offset) {
	char senderName[MAXBUF];
	offset = getSenderNameForPrint(dataBuffer, offset, senderName);	

	// ----- Get Message -----
	char *senderMessage = (char *)&dataBuffer[offset];
		
	// ----- Text Displaying -----
	printf("\n");
	printf("%s: %s\n", senderName, senderMessage);
}

void printUnicast(uint8_t *dataBuffer, int offset) {
	char senderName[MAXBUF];
	offset = getSenderNameForPrint(dataBuffer, offset, senderName);	
		
	uint8_t destNum = dataBuffer[offset++]; // always 1 because it's a %M
	if (destNum > 1) {
		printf("too many clients.\n");
		exit(-1);
	}
				
	// Extract Receiver's info and null-terminate the receiver's name
	uint8_t destLen = dataBuffer[offset++];
	char destName[MAXBUF];
	memcpy(destName, &dataBuffer[offset], destLen); 
	destName[destLen] = '\0';
	offset += destLen;
		
	char *senderMessage = (char *)&dataBuffer[offset]; // get message and typcase to save it	
	
	// ----- Text Displaying -----
	printf("\n");
	printf("%s: %s\n", senderName, senderMessage);
}

void printMulticast(uint8_t *dataBuffer, int offset) {
	char senderName[MAXBUF];
	offset = getSenderNameForPrint(dataBuffer, offset, senderName);	
		
	int destNum = dataBuffer[offset++]; // 2-9
	uint8_t destLen[destNum];
	char destName[destNum][100];
	for (int i = 0; i < destNum; i++) {
		destLen[i] = dataBuffer[offset++];
		memcpy(destName[i], &dataBuffer[offset], destLen[i]);
		offset += destLen[i];
		destName[i][destLen[i]] = '\0';
	}

	char *senderMessage = (char *)&dataBuffer[offset]; // get msg and typecast it
		
	// ----- Text Displaying -----
	printf("\n");
	printf("%s: %s\n", senderName, senderMessage);
}

void printError7(uint8_t *dataBuffer, int offset) {
	// ----- Parsing -----
	uint8_t destLen = dataBuffer[offset++];
	char destName[100];
	memcpy(destName, &dataBuffer[offset], destLen);
	destName[destLen] = '\0';
	offset += destLen;

	// ----- Error Message -----
	char errorMessage[MAXBUF];
	snprintf(errorMessage, MAXBUF, "\nClient with handle %s does not exist", destName);	
			
	// ----- Display the Error Message in Response to flags 5 and 6 -----
	printf("%s\n", errorMessage);
}

void printHandleCount(uint8_t *dataBuffer, int offset) {

	// ----- Display the number of active handles (4-bytes) -----
	uint32_t numActiveHandles;
	memcpy(&numActiveHandles, &dataBuffer[offset], sizeof(uint32_t));
	int numActiveHandleHost = ntohl(numActiveHandles);
	printf("\nNumber of clients: %d\n", numActiveHandleHost);
	isHandleListDone = 1;
}

void printList(uint8_t *dataBuffer, int offset) {

	uint8_t handleLen = dataBuffer[offset++];
	char handleName[MAXBUF];
	memcpy(handleName, &dataBuffer[offset++], handleLen);
	offset += handleLen;
	handleName[handleLen] = '\0';
	printf("  %s\n", handleName);
}

// ------------ processFromServer -------------
int processFromServer(int socketNum, char *handle_name) {
	uint8_t dataBuffer[MAXBUF]; // Stores the msg sent by client via user's input
	int msgLen = 0;	
		
	// Get the length of the message sent by the client
	if ((msgLen = recvPDU(socketNum, dataBuffer, MAXBUF)) <= 0) {
		printf("Server has terminated.\n");
		exit(-1);
	}

	dataBuffer[msgLen] = '\0'; // Null-terminate the string
	
	// check if the message sent by the user is "Server terminated"
	if (strcmp((char *)dataBuffer, "Server terminated\0") == 0) {
		printf("Server has terminated via user input.\n");
		return 0;	
	}

	// ----- Message Parsing to Display -----
	int offset = 0;
	uint8_t flag = dataBuffer[offset++];
	if (flag == FLAG_4) {
		printBroadcast(dataBuffer, offset);
	} if (flag == FLAG_5) {
		printUnicast(dataBuffer, offset);
	} else if (flag == FLAG_6) {
		printMulticast(dataBuffer, offset);
	} else if (flag == FLAG_7) {	
		printError7(dataBuffer, offset);
	} else if (flag == 11) {
		printHandleCount(dataBuffer, offset);
	} else if (flag == 12) {
		printList(dataBuffer, offset);
	} else if (flag == 13) {
		isHandleListDone = 0;
	}
	return msgLen;	
}

// --------------------------------- Client Control ---------------------------------
// --------------------------------- Client Control ---------------------------------
// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
void clientControl(int socketNum, char *handle_name) {	
	// ----- Check Initial Handle's Size ----
	if (strlen(handle_name) > 100) {
		printf("Invalid command: Handle name exceeds 100 characters.\n");	
		exit(-1);
	} else {
		// ----- Set Up Pollset -----
		setupPollSet();
	
		// ----- Add the socket num and STDIN_FILENO -----
		addToPollSet(socketNum);
		addToPollSet(STDIN_FILENO);
	
		printf("$: ");
		fflush(stdout); // force the msg to print immediately
		printf("handle_name: %s\n", handle_name);
	
		// ---- Initial Message Formatting -----
		// Deal with flags 1, 2, 3
		// Send to server to check if this client exists!
		uint8_t handleBuffer[MAXBUF];
		int offset = 0;
		handleBuffer[offset++] = FLAG_1; // check if handle is unique
		handleBuffer[offset++] = strlen(handle_name); // Add handle length
		memcpy(&handleBuffer[offset], handle_name, strlen(handle_name)); // add handle_name
		offset += strlen(handle_name);
	
		// Send the client's handle name to server as the first msg
		sendPDU(socketNum, handleBuffer, offset);
						
		// ----- Error Response to Flag 3 -----
		uint8_t handleResponse[MAXBUF];
		int handleRespondLen = recvPDU(socketNum, handleResponse, MAXBUF);
		if (handleRespondLen < 0) {
			printf("No specified client.\n");
			exit(-1);
		}
		if (handleResponse[0] == FLAG_3) {
			printf("Handle %s already existed.\n", handle_name);
			exit(-1);
		} else if (handleResponse[0] == FLAG_2) {		
			printf("Handle %s is good to use.\n", handle_name);
		}
		printf("$: ");
		fflush(stdout);
	
		// ----- Client Blocking -----	
		while (1) {
			int ClientPollCall = pollCall(-1);
			if (ClientPollCall == socketNum) {
				int returnServerProcess = processFromServer(socketNum, handle_name);	
				if (returnServerProcess == 0) {
					printf("Server has terminated.\n");
					exit(-1);
				}
				
				if (!isHandleListDone) {
					printf("$: ");
					fflush(stdout);
		
				}
			} else if (ClientPollCall == STDIN_FILENO) {
				processStdin(socketNum, handle_name);
				printf("$: ");
				fflush(stdout);
			}
		}
	}
}

void processStdin(int socketNum, char* handle_name) {
	uint8_t buffer[MAXBUF];
	int sendLen = 0;
	
	sendLen = readFromStdin(buffer);	
	//sendPDU(socketNum, buffer, sendLen);
	processCommand(buffer, sendLen, socketNum, handle_name);
}

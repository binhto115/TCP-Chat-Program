/* Look-up Handle Table  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "handleTable.h"
#include <stdbool.h>

// ----- Variable Initialization -----
static int *active_handle = NULL; // is the handle currently being used? 
static int capacity = 0; // Capacity of the table
static int handle_size = 0; // Size of handle name
static int *socket = NULL; // socket #
static char ** handle_name = NULL; // what's the handle name

void handle_table() {
	capacity = INITIAL_CAPACITY; // Initialize the capacity of the table
	handle_size = 0;

	handle_name = malloc(capacity * sizeof(char *)); 
	socket = malloc(capacity * sizeof(int));
	active_handle = malloc(capacity * sizeof(int));

	// Allocating space for str handle name (null-terminated) up to HANLDE_TABLE_NAME_SIZE -1 + '\0'	
	for (int i = 0; i < capacity; i++) {
		handle_name[i] = malloc(HANDLE_TABLE_NAME_SIZE * sizeof(char));
		memset(handle_name[i], 0, HANDLE_TABLE_NAME_SIZE);	
		active_handle[i] = 0;
	}
}

void check_size() {
	printf("Table capacity: %d\n", capacity);
	printf("table size: %d\n", handle_size);	
}

int numOfActiveHandles() {
	return handle_size;
}

void resize_table() {
	int new_capacity = capacity * RESIZE_FACTOR;
	handle_name = realloc(handle_name, new_capacity * sizeof(char *));
	socket = realloc(socket, new_capacity * sizeof(int));
	active_handle = realloc(active_handle, new_capacity * sizeof(int));

	for (int i = capacity; i < new_capacity; i++) {
		handle_name[i] = malloc(HANDLE_TABLE_NAME_SIZE * sizeof(char));
		memset(handle_name[i], 0, HANDLE_TABLE_NAME_SIZE * sizeof(char));
		active_handle[i] = 0;
	}	
	capacity = new_capacity;
}	

int look_up_socket_by_handle(const char *name) {
	// Loop through the table to see if the current handle is active and
	// matches the handle we're trying to find 
	for (int i = 0; i < capacity; i++) {
		if (active_handle[i] == 1 && strcmp(handle_name[i], name) == 0) {
			return socket[i];
		}
	}
	return -1;
	
}

const char *look_up_handle_by_socket(int socket_num) {
	// Loop through the table to see if the socket matches the socket we're looking for
	// and the handle name that is associated with the socket # has to be active
	for (int i = 0; i < capacity; i++) {
		if ((socket[i] == socket_num) && active_handle[i] == 1) {
			return handle_name[i];
		}
	}
	return NULL;
}

void add_handle(const char *name, int socket_num) {
	// ----- Check the Capacity Limit -----
	if (handle_size == capacity) {
		resize_table();
	}
	
	// ----- Add Handles -----
	for (int i = 0; i < capacity; i++) {
		if (active_handle[i] == 0) {
			strncpy(handle_name[i], name, HANDLE_TABLE_NAME_SIZE - 1);
			handle_name[i][HANDLE_TABLE_NAME_SIZE -1] = '\0';
			socket[i] = socket_num;
			active_handle[i] = 1;
			handle_size++;
			return;
		}
	}
}

void remove_handle(int socket_int) {
	// ----- Remove Handles -----
	for (int i = 0; i < capacity; i++) {
		if ((active_handle[i] == 1) && socket[i] == socket_int) {
			active_handle[i] = 0;
			handle_size--;
			return;
		}
	}
	printf("Unable to remove handle.\n");
}

void free_table() {
	// ----- Free -----
	for (int i = 0; i < capacity; i++) {
		free(handle_name[i]);
	}
	
	free(socket);
	free(handle_name);
	free(active_handle);
}

const char *getActiveHandle(int index) {
	// ----- Is it active? -----
	if (index < handle_size && index >= 0 && active_handle[index]) {
		return handle_name[index];
	}
	return NULL; // Zero active handles	
}

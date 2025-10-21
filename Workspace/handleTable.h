#ifndef HANDLE_TABLE_H
#define HANDLE_TABLE_H

#include  <stdbool.h>

// Define the length of handle names and initial capacity of the array
#define HANDLE_TABLE_NAME_SIZE 100
#define INITIAL_CAPACITY 2 
#define RESIZE_FACTOR 2

// Function prototypes
void handle_table(); // Initialize the 2d array lookup table
void resize_table(); // Resize the table by 2x
int look_up_socket_by_handle(const char *name); // Look up the socket number by the handle name
const char *look_up_handle_by_socket(int socket_num); // Look up the handle name by the socket number 
void add_handle(const char *name, int socket_num); // Add a handle to the lookup table
void remove_handle(int socket_num); // Remove a handle from the table
void free_table(); // free the table after usage
void check_size(); // check the size of the table
int numOfActiveHandles(); // return the number of active handles 
const char *getActiveHandle(int index); // grab handles

#endif

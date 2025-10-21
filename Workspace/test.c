#include <stdio.h>
#include "handleTable.h"

int main() {

	// Initialize array
	handle_table();
	check_size();
	add_handle("Binh1", 1);
	add_handle("Binh2", 2);
	cdd heck_size();

	printf("First handle: %s\n", look_up_handle_by_socket(1));	
	printf("Second handle: %s\n", look_up_handle_by_socket(2));
	
	add_handle("Binh3", 3);
	check_size();
	printf("Third handle: %s\n", look_up_handle_by_socket(3));
	
	add_handle("binh4", 4);
	check_size();
	add_handle("binh5", 5);	
	printf("Fifth handle: %s\n", look_up_handle_by_socket(5));
	check_size();	
	

	printf("Socket #%d\n", look_up_socket_by_handle("Binh1"));
	printf("Socket #%d\n", look_up_socket_by_handle("binh5"));
	
	remove_handle(1);
	remove_handle(2);
	remove_handle(3);
	remove_handle(4);
	remove_handle(5);
	add_handle("binh1", 1);
	check_size();
	printf("Socket #%d\n", look_up_socket_by_handle("binh1"));
	
	free_table();
 	return 0;
}

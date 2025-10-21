# TCP-Chat-Program

This project implements a simple TCP-based chat system consisting of two programs written in C/:
  server – acts as a message router and forwarding agent for connected clients.
  cclient – the chat client program that connects to the server and exchanges messages with other clients through it.
All communication between clients passes through the server, forming a logical star topology where the server sits at the center managing and routing all traffic.

server:
  Listens on a specified TCP port for incoming client connections.
  Maintains a handle table mapping each client’s unique handle (username) to its socket descriptor.
  Forwards messages between clients based on protocol flags:
  Unicast (%M) – message sent to a specific client.
  Broadcast (%B) – message sent to all connected clients.
  Multicast (%C) – message sent to a subset of clients.
  Handles client disconnections and cleans up handle table entries.
  Supports multiple simultaneous clients using fork() or select()/poll() for multiplexing.

cclient:
  Connects to the server using TCP sockets with the following command format:
  ./cclient <handle-name> <server-hostname> <server-port>
  Registers its handle name with the server.
  Sends and receives messages through the server, which routes them to other clients.
  Displays incoming messages and allows the user to type and send messages interactively.

  Usage:
  ./server <port>
  ./cclient <handle> <server-host> <port>

#include "server.h"

#define PORT 5872
#define MAX_CLIENTS 5
#define BUFFER_SIZE 1024 


void *clientHandler(void *socket) {

    // Receive packets from the client
    char recvdata[PACKETSZ];
    memset(recvdata, 0, PACKETSZ);
    int ret = recv(conn_fd, recvdata, PACKETSZ, 0); // receive data from client
    if(ret == -1)
        perror("recv error");    
    packet_t *ackpacket = deserializeData(recvdata);
    // Determine the packet operatation and flags
    
    // Receive the image data using the size

    // Process the image data based on the set of flags

    // Acknowledge the request and return the processed image data
}

int main(int argc, char* argv[]) {
    int listen_fd, conn_fd;
    // Creating socket file descriptor
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1) {
        perror("socket error");
    }

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // Listen to any of the network interface (INADDR_ANY)
    servaddr.sin_port = htons(PORT); // Port number
        // above segment from lab 12
    // Bind the socket to the port
    int ret = bind(listen_fd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    if (ret == -1) {
        perror("bind to socket error");
    }
    // Listen on the socket
    ret = listen(listen_fd, MAX_CLIENTS);
    if (ret == -1) {
        perror("listen to socket error");
    }
    // Accept connections and create the client handling threads
    struct sockaddr_in clientaddr;
    socklen_t clientaddr_len = sizeof(clientaddr);
    conn_fd = accept(listen_fd, (struct sockaddr *) &clientaddr, &clientaddr_len);
    if (conn_fd == -1) {
        perror("accept from client error");
    }

    // CLIENT HANDLING THREADS PART [using clientHandler()?]

    // Release any resources
    close(conn_fd);
    close(listen_fd);
    return 0;
}

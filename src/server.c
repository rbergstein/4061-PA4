#include "server.h"

#define PORT 5872
#define MAX_CLIENTS 5
#define BUFFER_SIZE 1024 


void *clientHandler(void *socket) {
    // Receive packets from the client
    char recvdata[BUFFER_SIZE];
    memset(recvdata, 0, BUFFER_SIZE);
    int ret = recv(&socket, recvdata, BUFFER_SIZE, 0); // receive data from client
    if (ret == -1)
        perror("recv error");    
    packet_t *ackpacket = deserializeData(recvdata);
    // Determine the packet operatation and flags
    char temp_operation = ackpacket->operation;
    char temp_flags = ackpacket->flags;

    // Receive the image data using the size
    int width;
    int height;                            
    int bpp; 
    uint8_t *image_result = stbi_load(ackpacket->file_name, &width, &height, &bpp,  CHANNEL_NUM); 

    // Process the image data based on the set of flags
    uint8_t **result_matrix = (uint8_t **)malloc(sizeof(uint8_t*) * width);
    uint8_t **img_matrix = (uint8_t **)malloc(sizeof(uint8_t*) * width);
    for (int i = 0; i < width; i++){
        result_matrix[i] = (uint8_t *)malloc(sizeof(uint8_t) * height);
        img_matrix[i] = (uint8_t *)malloc(sizeof(uint8_t) * height);
    }

    linear_to_image(image_result, img_matrix, width, height);

    if (ackpacket->flags == IMG_FLAG_ROTATE_180) {
        flip_left_to_right(img_matrix, result_matrix, width, height);
    }  
    else {
        flip_upside_down(img_matrix, result_matrix ,width, height);
    }  

    uint8_t* img_array = (uint8_t *)malloc(sizeof(uint8_t) * width * height);
    flatten_mat(result_matrix, img_array, width, height);

    // Acknowledge the request and return the processed image data
    packet_t packet;
    packet = (packet_t) {
            .operation = htons(temp_operation),
            .flags = htons(temp_flags), 
            .size = htons(ackpacket->size) };
            
    char *serializedData = serializePacket(&packet);
    ret = send(socket, serializedData, sizeof(packet), 0); 
    if (ret == -1)
        perror("send error");

    for (int i = 0; i < width; i++) {
        free(result_matrix[i]);
        free(img_matrix[i]);
    }
    free(result_matrix);
    free(img_matrix);
    free(img_array);
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

    while (1) {
        conn_fd = accept(listen_fd, (struct sockaddr *) &clientaddr, &clientaddr_len);
        if (conn_fd == -1) {
            perror("accept from client error");
        }

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, (void *)clientHandler, &conn_fd) != 0) {
            perror("cannot create thread");
        };
    }

    // Release any resources
    close(conn_fd);
    close(listen_fd);
    return 0;
}

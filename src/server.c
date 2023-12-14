#include "server.h"

#define PORT 6872
#define MAX_CLIENTS 5
#define BUFFER_SIZE 1024 


void *clientHandler(void *socket) {
    // Receive packets from the client
    int sock_fd = *((int *)socket);
    printf("--in clientHandler--\n");
    while(1) {
        char recvdata[PACKETSZ];
        memset(recvdata, 0, PACKETSZ);
        int ret = recv(sock_fd, recvdata, PACKETSZ, 0); // receive data from client
        if (ret == -1)
            perror("recv error");    
        packet_t *ackpacket = deserializeData(recvdata);

        // Determine the packet operatation and flags
        char temp_operation = ackpacket->operation;
        char temp_flags = ackpacket->flags;
        int size = ntohl(ackpacket->size);

        // if (temp_operation == IMG_OP_ACK) {
        //     continue;
        // } 
        if (temp_operation == IMG_OP_EXIT) {
            break;
        }

        // Receive the image data using the size
        char img_data[size];
        ret = recv(sock_fd, img_data, size, 0); 
        if (ret == -1)
            perror("recv error");

        char fname[100] = "tempXXXXXX.png";
        int temp_file = mkstemp(fname);  //create and open temp file 
        printf("%d", temp_file);
        if (temp_file == -1) {
            perror("temp file error");
            //return -1;
        }
        
        // FILE *temp_file = fopen(fd, "w");
        // if (temp_file == NULL) {
        //     perror("can't open file");
        //     //return -1;
        // }

        //write file data to temp file
        //fwrite(temp_file, 1, size, fsize);
        if (write(temp_file, img_data, size) == -1) {
            perror("write to temp file error");
        }
        close(temp_file);

        int width;
        int height;                            
        int bpp; 
        uint8_t *image_result = stbi_load(temp_file, &width, &height, &bpp,  CHANNEL_NUM); 

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
        
        char fname2[100] = "tempXXXXXX.png";
        int temp_file_rotated = mkstemp(fname2);  //create and open 2nd temp file 
        printf("%d", temp_file_rotated);
        if (temp_file_rotated == -1) {
            perror("temp file error");
            //return -1;
        }
        char f_buf[BUFFER_SIZE];
        sprintf(f_buf, "%d", temp_file_rotated);
        stbi_write_png(f_buf, width, height, CHANNEL_NUM, img_array, width*CHANNEL_NUM);

        //read file back into buffer and send
        //FILE *tf = open(temp_file_rotated, "rb");
        lseek(temp_file_rotated, 0, SEEK_END); // set file pointer to end of file
        int file_size = ftell(temp_file_rotated); // get size of file
        lseek(temp_file_rotated, 0, SEEK_SET); // set file pointer back to start
        
         // Acknowledge the request and return the processed image data
        packet_t packet;
        packet = (packet_t) {
                .operation = IMG_OP_ACK,
                .flags = temp_flags, 
                .size = htonl(file_size) };
                
        char *serializedData = serializePacket(&packet);
        ret = send(sock_fd, serializedData, sizeof(packet), 0); 
        if (ret == -1)
            perror("send error");

        char pack_buf[size];
        memset(pack_buf, 0, size);
        size_t bytes_read;
        int temp_size = packet.size;

        while (temp_size > 0) {

            bytes_read = read(pack_buf, temp_file_rotated, sizeof(pack_buf));
            if (bytes_read == -1) {
                perror("server byte read error");
                close(temp_file_rotated);
                return -1;
            }
            if (send(sock_fd, pack_buf, bytes_read, 0) == -1) {
                perror("server data send error");
                close(temp_file_rotated);
                return -1;
            }
        }

        close(temp_file_rotated);

        //free
        for (int i = 0; i < width; i++) {
            free(result_matrix[i]);
            free(img_matrix[i]);
        }

        free(result_matrix);
        free(img_matrix);
        free(img_array);
        free(serializedData);
    }
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
    fprintf(stdout, "Server exiting...\n");
    return 0;
}

#include "client.h"

#define PORT 5872
#define BUFFER_SIZE 1024 

int send_file(int socket, const char *filename) {
    // Open the file
    FILE *f = fopen(filename, "rb");
    if (f == NULL) {
        perror("can't open file");
        return -1;
    }

    fseek(f, 0, SEEK_END); // set file pointer to end of file
    int file_size = ftell(f); // get size of file
    fseek(f, 0, SEEK_SET); // set file pointer back to start

    // Set up the request packet for the server and send it
    packet_t packet;
    // packet.operation = ;         fill this here??
    // packet.flags = ;             not sure.
    packet.size = htol(file_size + 1);

    int ret = send(socket, &packet, packet.size, 0);
    if (ret == -1) {
        perror("packet send error");
        fclose(f);
        return -1;
    }
    // Send the file data
    char pack_buf[BUFFER_SIZE];
    size_t bytes_read;

    while ((bytes_read = fread(pack_buf, 1, sizeof(pack_buf), f)) > 0) {
        if (send(socket, pack_buf, bytes_read, 0) == -1) {
            perror("file data send error");
            fclose(f);
            return -1;
        }
    }

    fclose(f);
}

int receive_file(int socket, const char *filename) {
    // Open the file
    FILE *f = fopen(filename, "w");
    if (f == NULL) {
        perror("can't open file");
        return -1;
    }
    // Receive response packet
    packet_t packet;
    int ret = recv(socket, &packet, sizeof(packet), 0);
    if (ret == -1) {
        perror("recieve packet error");
    }
    // Receive the file data
    char pack_buf[BUFFER_SIZE];
    size_t bytes_received;
    size_t total_bytes_received = 0;

    while (packet.size > 0) {
        bytes_received = recv(socket, pack_buf, sizeof(pack_buf), 0);
        if (bytes_received == -1) {
            perror("file data receive error");
            fclose(f);
            return -1;
        }

        // Write the data to the file
        fwrite(pack_buf, 1, bytes_received, f);

        packet.size -= bytes_received;
    }
    
    fclose(f);

    return 0;
}

int main(int argc, char* argv[]) {
    if(argc != 4){
        fprintf(stderr, "Usage: ./client File_Path_to_images File_Path_to_output_dir Rotation_angle. \n");
        return 1;
    }
    char* path_to_images = argv[1];
    char* output_dir = argv[2];
    int rotation_angle = atoi(argv[3]);
    
    // Set up socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); // create socket to establish connection
    if(sockfd == -1)
        perror("Failed to set up socket");

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // server IP, since the server is on same machine, use localhost IP
    servaddr.sin_port = htons(PORT); // Port the server is listening on

    // Connect the socket
    int ret = connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)); // establish connection to server
    if(ret == -1)
        perror("Failed to connect socket");

    // Read the directory for all the images to rotate
    DIR *dir = opendir(path_to_images);
    struct dirent *entry;

    if (dir == NULL) {
        perror("Failed to open directory");
        exit(EXIT_FAILURE);
    }

    request_t req_queue[MAX_QUEUE_LEN];
    int index_counter = 0;

    while ((entry = readdir(dir)) != NULL) { 
        if ((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0)) {
            continue;
        }

        const char* file_ext = strrchr(entry->d_name, '.');
        if (file_ext && strcmp(file_ext, ".png") == 0) {
            if (index_counter < BUFFER_SIZE) {
                req_queue[index_counter].file_name = strdup(entry->d_name); //memory allocation for file_name
                req_queue[index_counter].rotation_angle = rotation_angle;
                index_counter++;
            }
        }
    }
    // Send the image data to the server

    while (index_counter > 0) {
        //pop from queue
        index_counter--;
        char *f_name = req_queue[index_counter].file_name;
        // send a packet with the IMG_FLAG_ROTATE_XXX message header desired rotation Angle, Image size, and data.
        packet_t packet;
        if (rotation_angle == 180) {
            packet = (packet_t) {
                .operation = htons(IMG_OP_ROTATE),
                .flags = htons(IMG_FLAG_ROTATE_180), 
                .size = htonl(strlen(f_name) + 1) };
        }
        else {
            packet = (packet_t) {
                .operation = htons(IMG_OP_ROTATE),
                .flags = htons(IMG_FLAG_ROTATE_270), 
                .size = htonl(strlen(f_name) + 1) };
            
        }    
        char *serializedData = serializePacket(&packet);
        int ret = send(sockfd, serializedData, sizeof(serializedData), 0);
        if (ret == -1) {
            perror("packet send error");
        }
        //receive the response packet containing the processed image from the server
        // char recvdata[sizeof(packet)];
        // memset(recvdata, 0, sizeof(packet));
        // ret = recv(sockfd, recvdata, sizeof(packet), 0);
        // if (ret == -1) {
        //     perror("recieve packet error");
        // }
        //save the image to a specified directory (e.g., 'output')
    }
            

    // Check that the request was acknowledged

    // Receive the processed image and save it in the output dir

    // Terminate the connection once all images have been processed

    // Release any resources
    return 0;
}

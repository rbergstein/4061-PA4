#include "client.h"

#define PORT 5872
#define BUFFER_SIZE 1024 

int send_file(int socket, const char *filename) {
    // Open the file
    FILE *f = fopen(filename, "r");
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
    packet.size = file_size;

    int ret = send(socket, &packet, sizeof(packet), 0);
    if (ret == -1) {
        perror("packet send error");
    }
    return 0;
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
    
    // Receive response packet

    // Receive the file data

    // Write the data to the file
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

    request_t reqlist[MAX_QUEUE_LEN];
    int index_counter = 0;

    while ((entry = readdir(dir)) != NULL) { 
        if ((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0)) {
            continue;
        }

        const char* file_ext = strrchr(entry->d_name, '.');
        if (file_ext && strcmp(file_ext, ".png") == 0) {
            if (index_counter < BUFFER_SIZE) {
                reqlist[index_counter].file_name = strdup(entry->d_name); //memory allocation for file_name
                reqlist[index_counter].rotation_angle = rotation_angle;
                index_counter++;
            }
        }
    }
    // Send the image data to the server

    // Check that the request was acknowledged

    // Receive the processed image and save it in the output dir

    // Terminate the connection once all images have been processed

    // Release any resources
    return 0;
}

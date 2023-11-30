#include "client.h"

#define PORT 8080
#define BUFFER_SIZE 1024 

int send_file(int socket, const char *filename) {
    // Open the file

    // Set up the request packet for the server and send it

    // Send the file data
    return 0;
}

int receive_file(int socket, const char *filename) {
    // Open the file

    // Receive response packet

    // Receive the file data

    // Write the data to the file
    return 0;
}

int main(int argc, char* argv[]) {
    if(argc != 3){
        fprintf(stderr, "Usage: ./client File_Path_to_images File_Path_to_output_dir Rotation_angle. \n");
        return 1;
    }
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
    DIR *dir = opendir(argv[1]);
    struct dirent *entry;

    if (dir == NULL) {
        perror("Failed to open directory");
        exit(EXIT_FAILURE);
    }

    request_t reqlist[BUFFER_SIZE];
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

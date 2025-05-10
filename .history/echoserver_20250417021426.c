#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

#define BUFFER_SIZE 1024
#define BACKLOG 5

int verbose = 0; 

void handleConnection(int *sock_fd_ptr){
    int sock_fd = *sock_fd_ptr;
    // free the pointer to avoid memory leak
    free(sock_fd_ptr);

    printf("handling connection on %d\n", sock_fd);
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    while(1){
        // receive data from client
        bytes_received = recv(sock_fd, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                printf("client disconnected\n");
            } else {
                perror("recv failed");
            }
            break;
        }

        // print received message
        if (verbose) {
            printf("received: %.*s\n", (int)bytes_received, buffer);
        }

        // send data back to client
        ssize_t bytes_sent = send(sock_fd, buffer, bytes_received, 0);
        if (bytes_sent < 0) {
            perror("send failed");
            break;
        }
    }

    printf("done with connection %d\n", sock_fd);
    close(sock_fd);
    return;
}

int main(int argc, char* argv[]){
    int opt;
    int port = 8080;

    // parse command line arguments
    while ((opt = getopt(argc, argv, "p:v")) != -1) {
        switch (opt) {
            case 'p':
                port = atoi(optarg);
                break;
            case 'v':
                verbose = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-p port] [-v]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // create socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket failed");
        close;
    }

    // bind socket address and port
    struct sockaddr_in socket_address;
    memset(&socket_address, '\0', sizeof(socket_address));
    socket_address.sin_family = AF_INET;
    socket_address.sin_addr.s_addr = INADDR_ANY;
    socket_address.sin_port = htons(port);

    // print error message if bind fails
    if(bind(server_fd, (struct sockaddr*)&socket_address, sizeof(socket_address)) < 0){
        perror("bind failed");
        close(server_fd);
        return 1;
    }

    // listen for incoming connections
    if(listen(server_fd, BACKLOG) < 0){
        perror("listen failed");
        close(server_fd);
        return 1;
    }

    printf("server listening on port %d\n", port);

    // accept incoming connections, create a new thread for each connection
    struct sockaddr_in client_address;
    socklen_t client_address_length = sizeof(client_address);

    while(1){
        pthread_t thread;
        int* client_fd_buffer = malloc(sizeof(int));

        *client_fd_buffer = accept(server_fd, (struct sockaddr*)&client_address, &client_address_length);

        if (*client_fd_buffer < 0) {
            perror("accept failed");
            free(client_fd_buffer);
            continue;
        }

        printf("accepted connection on %d\n", *client_fd_buffer);

        pthread_create(&thread, NULL, (void* (*)(void*))handleConnection, (void*)client_fd_buffer);
    }

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define SERVER_PORT "5432"
#define MAX_LINE 256
#define MAX_PENDING 5

/*
 * Create, bind and passive open a socket on a local interface for the provided service.
 * Returns a passively opened socket or -1 on error.
 */
int bind_and_listen( const char *service );

/*
 * Return the maximum socket descriptor in the provided fd_set.
 */
int find_max_fd(const fd_set *fs);

int main(void){
    // all_sockets stores all active sockets including the listening socket.
    fd_set all_sockets;
    FD_ZERO(&all_sockets);
    // call_set is a temporary set used for select().
    fd_set call_set;
    FD_ZERO(&call_set);

    // Create the listening socket.
    int listen_socket = bind_and_listen(SERVER_PORT);
    if(listen_socket < 0){
        fprintf(stderr, "Failed to bind and listen\n");
        exit(EXIT_FAILURE);
    }
    FD_SET(listen_socket, &all_sockets);

    // max_socket holds the highest-numbered file descriptor.
    int max_socket = listen_socket;
    char buffer[MAX_LINE];

    printf("Server listening on port %s\n", SERVER_PORT);

    while(1) {
        // Copy the master set because select modifies the set.
        call_set = all_sockets;
        int num_s = select(max_socket + 1, &call_set, NULL, NULL, NULL);
        if( num_s < 0 ){
            perror("ERROR in select() call");
            return -1;
        }
        // Iterate over file descriptors starting at 3 (skip std descriptors).
        for( int s = 3; s <= max_socket; ++s ){
            if( !FD_ISSET(s, &call_set) )
                continue;

            // New connection ready.
            if( s == listen_socket ){
                struct sockaddr_storage client_addr;
                socklen_t addr_len = sizeof(client_addr);
                int new_sock = accept(listen_socket, (struct sockaddr *)&client_addr, &addr_len);
                if(new_sock < 0){
                    perror("accept error");
                    continue;
                }
                printf("Socket %d connected\n", new_sock);
                FD_SET(new_sock, &all_sockets);
                if(new_sock > max_socket)
                    max_socket = new_sock;
            }
            // Existing connected socket is ready.
            else{
                int bytes_read = read(s, buffer, sizeof(buffer) - 1);
                if(bytes_read <= 0){
                    // bytes_read == 0 indicates the connection was closed by the client.
                    if(bytes_read == 0)
                        printf("Socket %d closed\n", s);
                    else
                        perror("read error");
                    close(s);
                    FD_CLR(s, &all_sockets);
                } else {
                    buffer[bytes_read] = '\0';  // Null-terminate the received data.
                    printf("Socket %d sent: %s\n", s, buffer);
                }
            }
        }
    }
    return 0;
}

int find_max_fd(const fd_set *fs) {
    int ret = 0;
    for(int i = FD_SETSIZE - 1; i >= 0 && ret == 0; --i){
        if( FD_ISSET(i, fs) ){
            ret = i;
        }
    }
    return ret;
}

int bind_and_listen( const char *service ) {
    struct addrinfo hints;
    struct addrinfo *rp, *result;
    int s;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;       // Allow IPv4 or IPv6.
    hints.ai_socktype = SOCK_STREAM;   // Stream socket.
    hints.ai_flags = AI_PASSIVE;         // For wildcard IP address.
    hints.ai_protocol = 0;              // Any protocol.

    if ((s = getaddrinfo(NULL, service, &hints, &result)) != 0 ) {
        fprintf(stderr, "stream-talk-server: getaddrinfo: %s\n", gai_strerror(s));
        return -1;
    }

    for (rp = result; rp != NULL; rp = rp->ai_next ) {
        int listenfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (listenfd == -1)
            continue;

        int opt = 1;
        if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            close(listenfd);
            continue;
        }
        
        if (bind(listenfd, rp->ai_addr, rp->ai_addrlen) == 0) {
            if (listen(listenfd, MAX_PENDING) == -1) {
                perror("listen failed");
                close(listenfd);
                freeaddrinfo(result);
                return -1;
            }
            freeaddrinfo(result);
            return listenfd;
        }
        close(listenfd);
    }
    perror("stream-talk-server: bind");
    freeaddrinfo(result);
    return -1;
}

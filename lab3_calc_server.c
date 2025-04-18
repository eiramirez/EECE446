/* This code is an updated version of the sample code from "Computer Networks: A Systems
 * Approach," 5th Edition by Larry L. Peterson and Bruce S. Davis. Some code comes from
 * man pages, mostly getaddrinfo(3). */
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SERVER_PORT "5432"
#define BUF_SIZE 256 // This can be smaller. What size?
#define MAX_PENDING 5

/*
 * Create, bind and passive open a socket on a local interface for the provided service.
 * Argument matches the second argument to getaddrinfo(3).
 *
 * Returns a passively opened socket or -1 on error. Caller is responsible for calling
 * accept and closing the socket.
 */
int bind_and_listen( const char *service );

int main( void ) {
	char buf[BUF_SIZE];
	int s, new_s;
	uint32_t x, y;
	uint32_t sum;


	/* Bind socket to local interface and passive open */
	if ( ( s = bind_and_listen( SERVER_PORT ) ) < 0 ) {
		exit( 1 );
	}
	// Prepare a sockaddr_storage to hold the client's address.
	struct sockaddr_storage client_addr;
    socklen_t addr_len = sizeof(client_addr);
	// Accept a connection from the client
	if ((new_s = accept(s, (struct sockaddr *)&client_addr, &addr_len)) == -1) {
        perror("accept");
        close(s);
        exit(EXIT_FAILURE);
    }


	// Now process the client
	while( 1 ) {

		// Receive two uint32_t values into a buffer (buf)
		int len = recv(new_s, buf, sizeof(buf), 0);
		if (len < 0) {
			perror("recv failed");
			exit(EXIT_FAILURE);
		}
		// Copy the values out of the buffer into variables (x and y)
		memcpy(&x, buf, sizeof(len));
		memcpy(&y, buf + sizeof(len), sizeof(len));

		// Add the numbers (into sum)
		sum = x + y;

		// Copy the sum back to the buffer (buf)
		memcpy(buf, &sum, sizeof(sum));

		// Send the buffer back to the client. Only send the bytes for the sum!
		if (send(new_s, buf, sizeof(buf), 0) < 0) {
			perror("Send failed");
			exit(EXIT_FAILURE);
		}
	}

	close( s );
	// Close any other sockets you use.

	return 0;
}

int bind_and_listen( const char *service ) {
	struct addrinfo hints;
	struct addrinfo *rp, *result;
	int s;

	/* Build address data structure */
	memset( &hints, 0, sizeof( struct addrinfo ) );
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = 0;

	/* Get local address info */
	if ( ( s = getaddrinfo( NULL, service, &hints, &result ) ) != 0 ) {
		fprintf( stderr, "stream-talk-server: getaddrinfo: %s\n", gai_strerror( s ) );
		return -1;
	}

	/* Iterate through the address list and try to perform passive open */
	for ( rp = result; rp != NULL; rp = rp->ai_next ) {
		if ( ( s = socket( rp->ai_family, rp->ai_socktype, rp->ai_protocol ) ) == -1 ) {
			continue;
		}

		if ( !bind( s, rp->ai_addr, rp->ai_addrlen ) ) {
			break;
		}

		close( s );
	}
	if ( rp == NULL ) {
		perror( "stream-talk-server: bind" );
		return -1;
	}
	if ( listen( s, MAX_PENDING ) == -1 ) {
		perror( "stream-talk-server: listen" );
		close( s );
		return -1;
	}
	freeaddrinfo( result );

	return s;
}

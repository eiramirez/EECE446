#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>

int main(void) {
    fd_set readfds;
    struct timeval tv;
    int retval;
    char buffer[1024];

    while (1) {
        // Reinitialize the file descriptor set and add STDIN.
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        
        // Set the timeout to 2 seconds (reinitialize each loop since select may change it).
        tv.tv_sec = 2;
        tv.tv_usec = 0;
        
        // Wait for input on STDIN.
        retval = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv);
        
        // Print out the result of select() and the current timeout values.
        printf("select returned: %d\n", retval);
        printf("Time remaining: %ld seconds, %ld microseconds\n", tv.tv_sec, tv.tv_usec);
        
        // Check if STDIN is marked as ready.
        if (FD_ISSET(STDIN_FILENO, &readfds))
            printf("Standard input is in the descriptor set.\n");
        else
            printf("Standard input is NOT in the descriptor set.\n");
        
        if (retval < 0) {
            perror("select error");
            exit(EXIT_FAILURE);
        } else if (retval == 0) {
            // No input: timeout occurred.
            printf("No input received for 2 seconds. Exiting...\n");
            break;
        } else {
            // Input available: read and echo it.
            if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
                printf("Echo: %s", buffer);
            }
        }
    }
    return 0;
}

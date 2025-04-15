#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>

int main(void) {
    // Task 1: Working with file descriptor sets
    fd_set fdset;
    FD_ZERO(&fdset);               // Clear the set.
    FD_SET(STDIN_FILENO, &fdset);    // Add standard input.

    // Check if standard output (STDOUT_FILENO) is in fdset.
    // (It will not be, unless you add it.)
    if (FD_ISSET(STDOUT_FILENO, &fdset))
        printf("YES\n");
    else
        printf("NO\n");
    
    // Task 2: Initializing a timeout of 1.35 seconds
    struct timeval tv;
    tv.tv_sec = 1;       // 1 second
    tv.tv_usec = 350000; // 350,000 microseconds = 0.35 seconds

    printf("Timeout set to: %ld seconds and %ld microseconds\n", tv.tv_sec, tv.tv_usec);
    
    return 0;
}

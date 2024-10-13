#include <stdlib.h>     // exit, EXIT_FAILURE
#include <stdio.h>      // perror, sprintf, fopen, FILE, fgets, fwrite, fclose
#include <string.h>     // strlen, strncmp, strtok, strcmp, memset, memcpy, strcpy, strcat
#include <sys/socket.h> // socket, AF_INET, SOCK_DGRAM, socklen_t
                        // recvfrom, sendto, ssize_t, struct sockaddr, bind, 
                        // MSG_WAITALL, MSG_CONFIRM
#include <netinet/in.h> // struct sockaddr_in, INADDR_ANY
#include <stdbool.h>    // true
#include <arpa/inet.h>  // htons
#include <pthread.h>    // pthread_mutex*

#include "sysmond.h"

extern int                      data_available;
extern char                     data[ 2 ][ 1024 ];

extern struct sysmond_arguments sysmond_args;
extern pthread_mutex_t          mutex;

void*       udp_thread( void* );
extern void dbg( char* );


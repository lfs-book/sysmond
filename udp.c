#include "udp.h"

#define BUFFER_SIZE_IN  1024
#define BUFFER_SIZE_OUT 1024

// Runs on a separate thread because main loop sleeps
// Listen for a udp packet on port sysmond_args.udpPort
// Ignore the content
// Respond to sender with system data
void* udp_thread( void* )
{
   int                udp_socket;
   char               data_in [ BUFFER_SIZE_IN  ]; 
   char               data_out[ BUFFER_SIZE_OUT ];  
   socklen_t          data_len; 
   struct sockaddr_in serv_addr;
   struct sockaddr_in client_addr;
   socklen_t          socket_len = sizeof( struct sockaddr_in );

   // Server data
   memset( (char*)& serv_addr, 0, sizeof(serv_addr) );
   serv_addr.sin_family      = AF_INET;
   serv_addr.sin_addr.s_addr = INADDR_ANY;
   serv_addr.sin_port        = htons( sysmond_args.udpPort );

   // Create a UDP socket
   if ( ( udp_socket = socket( AF_INET, SOCK_DGRAM, 0) ) < 0 ) 
   {
      perror( "cannot create socket\n" );
      exit( EXIT_FAILURE );
   }

   // Bind to the scoket
   if ( bind( udp_socket, (struct sockaddr*)& serv_addr, sizeof(serv_addr) ) < 0 ) 
   {
      perror( "bind failed" );
      exit( EXIT_FAILURE );
   }

   while( true )
   {
      // Wait for input to socket
      memset( (char*) data_in, 0, BUFFER_SIZE_IN );

      // Client data
      memset( (char*)& client_addr, 0, sizeof(client_addr) );

      int n =
      recvfrom( udp_socket, 
                data_in, 
                BUFFER_SIZE_IN, 
                MSG_WAITALL, 
                (struct sockaddr*)& client_addr, 
                &socket_len 
              );

      if ( n < 0 )
      {
         char t[40];
         sprintf( t, ": Error from recvfrom().  'n' returned %d\n", n ); 
         dbg( t );
      }
      
      // Debug
      //char t[ BUFFER_SIZE_IN ];
      //memset( (char*) t, 0, BUFFER_SIZE_IN );
      //sprintf( t, " - data out size %d\n", strlen(data[ data_available ]) );
      //dbg( t );

      //char* a = inet_ntoa( client_addr.sin_addr );
      //int   p = ntohs( client_addr.sin_port );
      //sprintf( t, "- got request from: %s:%d\n", a, p );
      //dbg( t );

      // This is where we get the data from the system
      // The data[ 2 ] string array uses data_available to 
      // decide which data is current.

      memset( (char*) data_out, 0, BUFFER_SIZE_OUT );
      pthread_mutex_lock( &mutex );
      memcpy( data_out, data[ data_available ], strlen( data[ data_available ] ) );
      pthread_mutex_unlock( &mutex );

      int end = strlen( data_out );;
      data_out[ end ] = '\0';

      // Send data
      ssize_t sent =
      sendto( udp_socket, 
              data_out, 
              strlen( data_out ) + 1, // Add trailing null
              MSG_CONFIRM, 
              (struct sockaddr*)& client_addr,  
              socket_len
            ); 

      //sprintf( t, "- data sent %d\n", sent);
      //dbg( t );
   }

   // Never exits
}


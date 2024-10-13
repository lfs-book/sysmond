#include <stdio.h>  // FILE, fopen, fwrite, fclose, fprintf
#include <unistd.h> // unlink
#include <string.h> // strlen, memset, strerror
#include <stdlib.h> // exit
#include <signal.h> // struct sigaction, sigaction, sigemptyset, SA_*, SIG*
#include <errno.h>  // errno 
                    
#include "sysmond.h"

// Handle SIGTERM and SIGUSR1 signals
void signalHandler( int sig )
{
   FILE* file;

   switch( sig ) 
   {
     case SIGTERM:
        unlink( sysmond_args.pidFile );  // remove pidFile
        exit( 0 );
        break;

     case SIGUSR1:
        file = fopen( sysmond_args.dbgFile, "a" );
        if ( file != NULL )
        {
           fwrite( data[ data_available ], 
                   strlen( data[ data_available ] ),
                   1,
                   file );
           fclose( file );
        }
        break;
   }
}  

void install_sighandler()
{
   struct sigaction new;
   int              ret;

   // Initialize struct sigaction new
   memset( &new, 0, sizeof( struct sigaction ) );
   new.sa_handler = signalHandler;
   sigemptyset( &new.sa_mask );
   new.sa_flags = SA_RESTART;

   // Set up SIGTERM
   ret = sigaction( SIGTERM, &new, NULL );
   if ( ret == -1) 
   {
      fprintf(stderr, "Could not set sighandler for SIGTERM: %s\n",
              strerror( errno ) 
             );
      exit( EXIT_FAILURE );
   }

   // Set up SIGUSR1
   ret = sigaction( SIGUSR1, &new, NULL );
   if ( ret == -1 ) 
   {
      fprintf( stderr, "Could not set sighandler for SIGUSR1: %s\n",
               strerror( errno ) 
             );
      exit( EXIT_FAILURE ); 
   }
}

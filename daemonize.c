#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>
#include <pthread.h>
#include <netinet/in.h>
#include <libgen.h>  // basename

#include "sysmond.h"

void daemonize()
{
   int         pid;
   struct stat fileStat;
   FILE*       file;

   // Change to root directory
   if ( chdir( "/" ) < 0 ) 
   {
       perror( "chdir()" );
       exit( EXIT_FAILURE );
   }

   // See if pid file exists  
   if ( ! (stat( sysmond_args.pidFile, &fileStat ) ) &&
        ( (! S_ISREG( fileStat.st_mode )           ) || 
          (fileStat.st_size > 11     ) 
        )          
      )
   {
      fprintf( stderr,
               "Error: PID file `%s' already exists and looks suspicious.\n",
               sysmond_args.pidFile );
      exit( EXIT_FAILURE );
   }

   // Set up signal handler  
   install_sighandler();

   // Open pidFile
   if ( ! ( file = fopen( sysmond_args.pidFile, "w" ) ) ) 
   {
      fprintf( stderr, "fopen(\"%s\"): %s\n", sysmond_args.pidFile,
               strerror( errno )
             );
      exit( EXIT_FAILURE );
   }

   // fork and write pidFile
   if ( (pid = fork() ) == -1 ) 
   {
      perror( "fork()" );
      exit( EXIT_FAILURE );
   } 
   else if ( pid != 0 ) 
   {
      // Original process
      fprintf( file, "%d\n", pid );
      fclose( file );

      exit( EXIT_SUCCESS );
   }

   // Now continue running the forked process
   fclose( file );
   close( STDIN_FILENO  );
   close( STDOUT_FILENO );
   close( STDERR_FILENO );

   nice( -1 );         // Bump priority
   set_affinity( 1 );  // Just run on CPU 1
}

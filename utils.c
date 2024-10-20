#define _GNU_SOURCE

#include <stdio.h>   // perror, snprintf, fopen, FILE, fgets, fwrite, fclose
#include <time.h>    // time_t, time(), ctime, gmtime, struct tm
#include <string.h>  // strlen
#include <pthread.h> // pthread_create, pthread_setaffinity_np, CPU_ZERO, CPU_SET

#include "sysmond.h"
#include "utils.h"

// Globals
char*           time_string;

// Start functions

// Populate time_string with formatted time without newline
void print_time()
{
   const time_t now         = time( NULL );
                time_string = ctime( &now );
   time_string[ 24 ]        = 0;             // Remove newline
}

// Write txt to debug file
void dbg( char* txt )
{
   FILE* file;

   file = fopen( sysmond_args.dbgFile, "a" );
   if ( file != NULL )
   {
      print_time();
      strcat( time_string, " " );
      fwrite( time_string, 
              strlen( time_string ),
              1,
              file );

      fwrite( txt, 
              strlen( txt ),
              1,
              file );
      
      fclose( file );
   }
}

// Attach a process/threaad to a specific CPU
void set_affinity( int cpu )
{
   cpu_set_t  mask;

   CPU_ZERO( &mask );
   CPU_SET( cpu, &mask );
   pthread_setaffinity_np( pthread_self(), sizeof(mask), &mask );
   // Ignore result
}


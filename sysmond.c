#define _GNU_SOURCE

#include <stdio.h>   // perror, snprintf, fopen, FILE, fgets, fwrite, fclose
#include <unistd.h>  // getuid, optind
#include <string.h>  // strtok, strcmp, strlen
#include <stdlib.h>  // strtol, exit, EXIT_FAILURE
#include <dirent.h>  // DIR, struct dirent, opendir, readdir
#include <libgen.h>  // basename
#include <stdbool.h> // true
#include <pthread.h> // pthread_create, pthread_setaffinity_np, CPU_ZERO, CPU_SET

#include "sysmond.h"

// Globals
int             data_available = 0;
char            data[ 2 ][ 1024 ];
pthread_mutex_t mutex;

// Start functions

// Determine if 'name' is already running
int is_running( char* name)
{
    DIR*           dir;
    struct dirent* ent;
    char*          endptr;
    char           buf[512];
    int            running = 0;
    char*          base    = basename( name ); // Remove path

    if ( ! (dir = opendir("/proc")) ) 
    {   
        perror( "can't open /proc" );
        return -1;
    }

    while( (ent = readdir(dir)) != NULL )
    {   
        // If endptr is not a null character, the directory is not
        // entirely numeric, so ignore it 
        long lpid = strtol( ent->d_name, &endptr, 10 );
        if (*endptr != '\0') continue;
        
        // Try to open the cmdline file
        snprintf( buf, sizeof(buf), "/proc/%ld/cmdline", lpid );
        FILE* fp = fopen( buf, "r" );
        
        if (fp) 
        {   
            if ( fgets(buf, sizeof(buf), fp) != NULL )
            {   
                char* first = basename( strtok( buf, " " ) );

                // Check the first token in the file, the program name.
                // We should always match this instance, but if another 
                // is running, we will get more than 1.
                
                if ( ! strcmp( first, base) ) running++;
            }

            fclose( fp );
        }
    }

    closedir( dir );
    return running;
}

int main( int argc, char* argv[])
{
  // This is really to get the config file. 
  if ( parseArgs( argc, argv) ) exit( EXIT_FAILURE );

  if ( getuid() != 0 )
  {
     printf( "You must be root to run this program.\n" );
     exit( EXIT_FAILURE );
  }

  // If already running, exit
  int count = is_running( argv[ 0 ] );

  // The minimum count is 1, the current process.
  if ( count > 1 )
  {
     printf( "sysmond is already running. Exiting.\n" );
     exit( EXIT_FAILURE );  
  }

  // Now read cfgFile and then redo parseArgs() 
  // That's easier to do so the command line takes precedence.
  readConfig( sysmond_args.cfgFile );

  // Run a second time to override the configuration file.
  // If the first run did not fail, then this one won't either.
  optind = 1;
  parseArgs( argc, argv);

  // Make the program a daemon via fork
  // This will only return for the forked process
  daemonize();

  // Probably not needed, but initialize the data strings.
  strcpy( data[ 0 ], "Thread 0\n" );
  strcpy( data[ 1 ], "Thread 1\n" );

  // Set up udp port and listen as separate thread
  pthread_t thread_handle;
  pthread_mutex_init( &mutex, NULL );
  pthread_create( &thread_handle, NULL, udp_thread, NULL);

  // Loop for fetching data from the system
  // Never exits.  Program terminates via SIGTERM signal.
  
  while( true )
  {
      // Get data.  Check current output line and put in the other.
      // Toggle between stings 0 and 1 for each loop iteration.
      int inactive_string = ( data_available == 0 ) ? 1 : 0;

      // Put data in data[ inactive_string ]
      char* outstring = data[ inactive_string ];
      get_data( outstring );

      // Toggle pointing to most recent data
      pthread_mutex_lock( &mutex );
      data_available = inactive_string;
      pthread_mutex_unlock( &mutex );

      sleep( sysmond_args.scanTime );
  }

  return 0;
}


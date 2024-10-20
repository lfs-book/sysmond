#include <stdio.h>   // needed printf, fopen, fgets, fclose, fprintf
#include <unistd.h>  // needed getopt(), optarg
#include <string.h>  // needed strlen, strncmp, strtok, strcmp
#include <getopt.h>  // needed struct option
#include <stdlib.h>  // needed atoi
                 
#include "sysmond.h"
#include "args.h"
#include "version.h"

static const char*         shortOptions = "c:p:d:u:s:h:";

static const struct option longOptions[] = 
{
   { "config-file",     required_argument, NULL, 'c' },
   { "pid-file",        required_argument, NULL, 'p' },
   { "debug-file",      required_argument, NULL, 'd' },
   { "udp-port",        required_argument, NULL, 'u' },
   { "scan-time",       required_argument, NULL, 's' },
   { "help",            no_argument,       NULL, 'h' },
   { NULL,              0,                 NULL,  0  }
}; 

// Defaults
struct sysmond_arguments sysmond_args =
{
   .pidFile  = "/run/sysmond.pid",
   .cfgFile  = "/etc/sysmond.conf",
   .dbgFile  = "/tmp/sysmond.dbg",
   .scanTime = 1,
   .udpPort  = 12686
}; 

// These are used for parsing the configuration file.
// Could the paths be more than 63 charaters long?
char pidFile[64];
char dbgFile[64];

// Start functions

void help( char* name )
{  
   printf( "Syntax: %s {options} \nVersion %s\n%s", name, VERSION,
   "  -c, --config-file <filename>    -- config file location (default /etc/sysmond.conf)\n"
   "  -p, --pid-file    <filename>    -- pid file location    (default /run/sysmond.pid)\n"
   "  -d, --debug-file  <filname>     -- debug file location  (default /tmp/sysmond.dbg)\n"
   "  -u, --udp-port    <port number> -- udp port for communication (default 12686)\n"
   "  -s, --scan-time   <seconds>     -- time between scans   (default 1 second)\n"
   "  -h, --help                      -- display help and exit\n" );
   _exit( EXIT_FAILURE );
}

void readConfig( const char* cfgFile )
{
   char  buf[ 512 ];
   FILE* fp = fopen( cfgFile, "r" );

   if ( fp == NULL )
   {
      printf( "Warning configuration file %s not found,\n", cfgFile);
      return;
   }

   while( fgets(buf, sizeof(buf), fp) != NULL )
   {
     // Use 2 to allow for newline char on a blank line.
     if ( strlen( buf ) < 2 ) continue; 

     char* cfgItem = strtok( buf, " \t=" );

     if ( ! strncmp( buf, "#", 1 ) ) continue;  // comment 

     char* value = strtok( NULL, " \t=" );      // get next token
     
     value[ strcspn( value, "\n" ) ] = 0;       // Remove newline

     if ( strcmp( cfgItem, "pid-file" ) == 0 ) 
     {
        strcpy( pidFile, value );
        sysmond_args.pidFile = pidFile;
     }
     else if ( strcmp( cfgItem, "debug-file" ) == 0)
     {
       strcpy( dbgFile, value );
       sysmond_args.dbgFile = dbgFile;
     }
     
     else if ( strcmp( cfgItem, "udp-port" ) == 0 )
       sysmond_args.udpPort = atoi( value );

     else if ( strcmp( cfgItem, "scan-time" ) == 0 )
       sysmond_args.scanTime = atoi( value );
   }

   fclose( fp );
   return;
}

int parseArgs( int argc, char** argv )
{  
   int c; 

   while( (c = getopt_long( argc, argv, shortOptions, longOptions, NULL) ) != EOF ) 
   {
      switch( c ) 
      {
        case 'c':
           sysmond_args.cfgFile = optarg;
           break;
        case 'p':
           sysmond_args.pidFile = optarg;
           break;
        case 'd':
           sysmond_args.dbgFile = optarg;
           break;
        case 'u':
           sysmond_args.udpPort = atoi( optarg );
           break;
        case 's':
           sysmond_args.scanTime = atoi( optarg );
           break;
        case 'h':
           help( argv[0] );
           break;
        default:
           fprintf( stderr, "Unknown option.\n" );
           help( argv[0] );
      }

      if ( sysmond_args.udpPort < 1025 || sysmond_args.udpPort > 49150 )
      {
         fprintf( stderr, "The UDP port must be in the range 1025-49150.\n"
                          "The default is 12686.\n" );
         help( argv[0] );
      }

      if ( sysmond_args.scanTime < 1 || sysmond_args.scanTime > 60 )
      {
         fprintf( stderr, "The scan time must be in the range 1-60.\n"
                          "The default is 1 second.\n");
         help( argv[0] );
      }
   }

   return 0;
}


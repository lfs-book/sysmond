#include <stdio.h>   // {,s,f}printf, fopen, fgets, fclose, fseek, SEEK_SET
#include <string.h>  // strlen, strncmp, strtok, strcmp, memset
#include <stdbool.h> // true
#include <unistd.h>  // gethostname
#include <time.h>    // time, time_t, ctime_r
#include <stdlib.h>  // strtol

#include "sysmond.h"

void get_data( char* out_string )
{
   static bool  first = true;
   static FILE* loadavgFile;
   static FILE* statFile;
   static FILE* meminfoFile;
   static FILE* uptimeFile;

   static char  hostname[ 64 ];

   char         buffer  [ 48 ];
   char         buffer2 [ 64 ];
   time_t       now;
   struct tm*   info;
   int          dst;
   static long  previousWork;
   static long  previousTotal;
   long         user;
   long         nice;
   long         system;
   long         idle;
   long         work;
   long         total;
   long         MemTotal = 0;  // Initialize to prevent a warning
   long         MemAvailable;
   double       percent;

   if ( first )
   {
      // Get the host name for the output string
      // This only needs to be done once
      memset     ( hostname, 0, 64 );
      gethostname( hostname, 64 );

      // Open files and populate initial values
      // Load average
      loadavgFile = fopen( "/proc/loadavg", "r" );
      setvbuf( loadavgFile, NULL, _IONBF, 0 );

      // System uptime
      uptimeFile = fopen( "/proc/uptime", "r" );
      setvbuf( uptimeFile, NULL, _IONBF, 0 );

      // CPU usage
      statFile = fopen( "/proc/stat", "r" );
      setvbuf( statFile, NULL, _IONBF, 0 );

      // We need to initialize previousWork and previousTotal
      fgets( buffer, sizeof( buffer), statFile);

      strtok( buffer, " " ); // Discard "cpu"
      
      user   = strtol( strtok( NULL, " " ), NULL, 10 );
      nice   = strtol( strtok( NULL, " " ), NULL, 10 );
      system = strtol( strtok( NULL, " " ), NULL, 10 );
      idle   = strtol( strtok( NULL, " " ), NULL, 10 );

      previousWork  = user + nice + system;
      previousTotal = previousWork + idle;

      // Memory
      meminfoFile = fopen( "/proc/meminfo", "r" );
      setvbuf( meminfoFile, NULL, _IONBF, 0 );
      
      // Don't do this again
      first = false;
   }

   // Copy the hostname and a semi-colon
   sprintf( out_string, "%s;", hostname ); 

   // Get the current time -- seconds since Epoch and format
   now    = time( NULL );
   ctime_r( &now, buffer );
   buffer[ 24 ] = 0;                        // Remove \n
   tzset();                                 // Get the time zone
   info = gmtime( &now );
   dst  = ( info->tm_isdst > 0 ) ? 1 : 0;
   sprintf( buffer2, "time:%s %s;", buffer, tzname[ dst ] );  
   strcat( out_string, buffer2 );

   // Get the uptime
   fseek( uptimeFile, 0, SEEK_SET );
   fgets( buffer, sizeof( buffer), uptimeFile);
   char* seconds = strtok( buffer, " " );
   sprintf( buffer2, "uptime:%s;", seconds );
   strcat( out_string, buffer2 );

   // Get the load average - 1, 5, and 15 minute averages
   fseek( loadavgFile, 0, SEEK_SET );
   fgets( buffer, sizeof( buffer), loadavgFile);

   for ( int i = 0, j = 0; i < strlen(buffer); i++ )
   {
      char buffer3[ 80 ];

      if ( buffer[ i ] == ' ' ) j++;
      if ( j < 3 ) continue;

      memcpy( buffer2, buffer, i );
      buffer2[ i ] = 0;
      sprintf( buffer3, "load:%s;", buffer2 ); // 
      strcat( out_string, buffer3 );
      break;
   }

   // Get CPU percentage
   // Only need first line: "cpu" user nice system idle
   // The last 4 are a count of "jiffies"
   fseek( statFile, 0, SEEK_SET );
   fgets( buffer, sizeof( buffer), statFile);

   strtok( buffer, " " ); // Discard "cpu"

   user   = strtol( strtok( NULL, " " ), NULL, 10 );
   nice   = strtol( strtok( NULL, " " ), NULL, 10 );
   system = strtol( strtok( NULL, " " ), NULL, 10 );
   idle   = strtol( strtok( NULL, " " ), NULL, 10 );

   work   = user + nice + system;
   total  = work + idle;
   
   if ( total - previousTotal <= 0 )
      percent = 0.0;
   else
      percent = (double) (work - previousWork) / ( total - previousTotal );

   sprintf( buffer, "cpu%%:%4.2f;", percent );  // Note the semicolon here
   strcat ( out_string, buffer );

   previousWork  = work;
   previousTotal = total;

   // Get the memory usage (MemTotal - MemAvailable) / MemTotal
   fseek( meminfoFile, 0, SEEK_SET );

   while( true )
   {
      char* cp;

      fgets( buffer, sizeof( buffer), meminfoFile);

      cp = strtok( buffer, " " );
      strcpy( buffer2, cp );

      if ( strcmp( buffer2, "MemTotal:" ) == 0 )
      {
         cp = strtok( NULL, " " );
         strcpy( buffer2, cp );

         MemTotal = strtol( buffer2, NULL, 10 );
         continue;
      }

      if ( strcmp( buffer2, "MemAvailable:" ) == 0 )
      {
         cp = strtok( NULL, " " );
         strcpy( buffer2, cp );

         MemAvailable = strtol( buffer2, NULL, 10 );
         break;
      }
   }

   percent = ( (MemTotal - MemAvailable) * 100.0 ) / MemTotal;
   sprintf( buffer, "mem%%:%4.2f\n", percent );  // Note the new line here
   strcat ( out_string, buffer );

   get_temps( out_string );
   return;
}



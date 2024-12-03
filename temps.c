#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <stdio.h>
#include <dirent.h>  // DIR, struct dirent, opendir, readdir

#include "sysmond.h"

/* See /usr/share/doc/linux<version>/hwmon/sysfs-interface.rst

   Output for each interface:
     buffer = interface:<name>,label:degC,...\n

     strcat( out_string, buffer );

*/

// Start functions

void get_temps( char* out_string )
{
   DIR*           dir_fp;
   DIR*           subdir_fp;
   struct dirent* ent;     // /sys/class/hwmon entry
   struct dirent* sub_ent; // subdirectory entry
   char           path     [ 288 ];   // Size takes into account
   char           name     [ 288 ];   // max size of filename
   char           interface[ 128 ];
   char           buffer   [ 256 ];

   char tempString [ 32 ][ 32 ];
   char tempEntries[ 32 ][  4 ];  // Numeric strings 1 or 2 chars
   int  n;  // Number of temperature entries

   // For each entry in hwmon, get info
   dir_fp = opendir( "/sys/class/hwmon" );

   while( (ent = readdir( dir_fp )) != NULL )
   {
      // Skip . and ..
      if ( strncmp( ent->d_name, ".", 1 ) == 0 ) continue;

      sprintf( path, "/sys/class/hwmon/%s", ent->d_name );      // Path to subdir
      sprintf( name, "/sys/class/hwmon/%s/name", ent->d_name ); // Path to subdir/name

      // Get interface name
      FILE* fp = fopen( name, "r" );
      fgets( interface, sizeof(interface), fp );
      fclose( fp );

      // Add to buffer string and remove newline
      //printf( "interface %s", interface ); // includes a newline

      sprintf( buffer, "interface;%s", interface ); 
      buffer[ strcspn(buffer, "\n") ] = 0;

      // Open the subdirectory 
      subdir_fp = opendir( path );
      n = 0;

      // Get 'temp<num>_' entries for each file in the directory
      while( (sub_ent = readdir( subdir_fp ) ) != NULL ) 
      {
         // We only want entries like temp??_
         if ( strncmp( sub_ent->d_name, "temp", 4 ) != 0 ) continue;

         char filename[32]; // name of directory entry
         strcpy( filename, sub_ent->d_name );
         char* p = strtok( sub_ent->d_name, "_" );

         char num[8];
         strcpy( num, &p[4] );
         // num is now number as in "temp" . num . "_somthing"

         // If new num, save it in tempEntries
         if ( n > 0 ) 
         {
            for ( int i = 0; i < n; i++ )
            {
               if ( strcmp( num, tempEntries[ i ] ) == 0 ) goto continue2;
            }
         }
          
         strcpy( tempEntries[ n ], num );
         n++;

continue2:
      } // while readdir( subdir_fp )

      closedir( subdir_fp );  // close subdirectory
      
      chdir( path );  // Make it easier to get file names

      // Cycle through for each temperature entry
      for ( int i = 0; i < n; i++ )
      {
         // Read label if it exists
         char label_name[ 192 ];
         char label     [  32 ];
         
         sprintf( label_name, "temp%s_label", tempEntries[ i ] ); 

         FILE* label_fp = fopen( label_name, "r" );
         if ( label_fp != NULL )
         {
            fgets( label, sizeof(label), label_fp);
            fclose( label_fp );
         }
         else 
            strcpy( label, label_name );

         // Remove newline
         label[ strcspn(label, "\n") ] = 0;

         // Now get the input temperature
         char temperature     [  32 ];
         char temperature_name[ 192 ];

         sprintf( temperature_name, "temp%s_input", tempEntries[ i ] );

         FILE* input_fp = fopen( temperature_name, "r" ); // open input file
         fgets( temperature, sizeof(temperature), input_fp );
         fclose( input_fp );

         int degC = (int) strtol( temperature, NULL, 10 ) / 1000;

         // If label only has one digit, insert a 0.
         if ( strlen( tempEntries[ i ] ) == 1 &&
              strncmp( label, "temp", 4) == 0 ) 
            sprintf( label, "temp0%c", tempEntries[ i ][ 0 ] ); 

         sprintf( tempString[ i ], ",%s:%d", label, degC );

         // Add to buffer string
         strcat( buffer, tempString[ i ] );


//   Is data[256] large enough???

      } // for each temp entry

      // Add a newline to the buffer
      strcat( buffer, "\n" );
      strcat( out_string, buffer ); 

   } // while reading /sys/class/hwmon
   closedir( dir_fp ); 
}


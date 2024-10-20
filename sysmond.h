//Shared variables

struct sysmond_arguments 
{
   char* cfgFile;
   char* pidFile;
   char* dbgFile;
   int   scanTime;
   int   udpPort;
}; 

extern struct sysmond_arguments sysmond_args;

extern int  data_available;
extern char data[ 2 ][ 1024 ];

extern void  readConfig        ( const char* ); // args.c
extern int   parseArgs         ( int, char** ); // args.c
extern void* udp_thread        ( void* );       // udp.c
extern void  daemonize         ( void );        // daemonize.c
extern void  install_sighandler( void );        // signal.c
extern void  signalHandler     ( int );         // signal.c
extern void  get_data          ( char* );       // data.c
extern void  get_temps         ( char* );       // temps.c
extern void  set_affinity      ( int );         // sysmond.c

// Local functions
void         dbg               ( char* );
int          is_running        ( char* );
void         print_time        ( void );


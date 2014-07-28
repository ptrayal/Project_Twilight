/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 **************************************************************************/

/***************************************************************************
*	ROM 2.4 is copyright 1993-1998 Russ Taylor			                   *
*	ROM has been brought to you by the ROM consortium		               *
*	    Russ Taylor (rtaylor@hypercube.org)				                   *
*	    Gabrielle Taylor (gtaylor@hypercube.org)			               *
*	    Brian Moore (zump@rom.org)					                       *
*	By using this code, you have agreed to follow the terms of the	       *
*	ROM license, in the file Rom24/doc/rom.license			               *
***************************************************************************/

/*
 * This file contains all of the OS-dependent stuff:
 *   startup, signals, BSD sockets for tcp/ip, i/o, timing.
 *
 * The data flow for input is:
 *    Game_loop ---> Read_from_descriptor ---> Read
 *    Game_loop ---> Read_from_buffer
 *
 * The data flow for output is:
 *    Game_loop ---> Process_Output ---> Write_to_descriptor -> Write
 *
 * The OS-dependent functions are Read_from_descriptor and Write_to_descriptor.
 * -- Furey  26 Jan 1993
 */

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

#include "twilight.h"
#include "db.h"
//#include "olc.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
#include "interp.h"


/*
 * Signal handling.
 * Apollo has a problem with __attribute(atomic) in signal.h,
 *   I dance around it.
 */
#if defined(apollo)
#define __attribute(x)
#endif

#if defined(unix)
#include <signal.h>
#endif

#if defined(apollo)
#undef __attribute
#endif



/*
 * Socket and TCP/IP stuff.
 */
#if	defined(macintosh) || defined(MSDOS)
const	char	echo_off_str	[] = { '\0' };
const	char	echo_on_str	[] = { '\0' };
const	char 	go_ahead_str	[] = { '\0' };
#endif

#if	defined(unix)
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "telnet.h"
const	char	echo_off_str	[] = { IAC, WILL, TELOPT_ECHO, '\0' };
const	char	echo_on_str		[] = { IAC, WONT, TELOPT_ECHO, '\0' };
const	char 	go_ahead_str	[] = { IAC, GA, '\0' };
#endif



/*
 * OS-dependent declarations.
 */
#if	defined(_AIX)
#include <sys/select.h>
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
void	bzero		args( ( char *b, int length ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	setsockopt	args( ( int s, int level, int optname, void *optval,
			    int optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
#endif

#if	defined(apollo)
#include <unistd.h>
void	bzero		args( ( char *b, int length ) );
#endif

#if	defined(__hpux)
int	accept		args( ( int s, void *addr, int *addrlen ) );
int	bind		args( ( int s, const void *addr, int addrlen ) );
void	bzero		args( ( char *b, int length ) );
int	getpeername	args( ( int s, void *addr, int *addrlen ) );
int	getsockname	args( ( int s, void *name, int *addrlen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	setsockopt	args( ( int s, int level, int optname,
 				const void *optval, int optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
#endif

#if	defined(interactive)
#include <net/errno.h>
#include <sys/fnctl.h>
#endif

#if	defined(linux)
/*
    Linux shouldn't need these. If you have a problem compiling, try
    uncommenting these functions.
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	listen		args( ( int s, int backlog ) );
*/

int	close		args( ( int fd ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
//int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	socket		args( ( int domain, int type, int protocol ) );
//int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

#if	defined(macintosh)
#include <console.h>
#include <fcntl.h>
#include <unix.h>
struct	timeval
{
	time_t	tv_sec;
	time_t	tv_usec;
};
#if	!defined(isascii)
#define	isascii(c)		( (c) < 0200 )
#endif
static	long			theKeys	[4];

int	gettimeofday		args( ( struct timeval *tp, void *tzp ) );
#endif

#if	defined(MIPS_OS)
extern	int		errno;
#endif

#if	defined(MSDOS)
int	gettimeofday	args( ( struct timeval *tp, void *tzp ) );
int	kbhit		args( ( void ) );
#endif

#if	defined(NeXT)
int	close		args( ( int fd ) );
int	fcntl		args( ( int fd, int cmd, int arg ) );
#if	!defined(htons)
u_short	htons		args( ( u_short hostshort ) );
#endif
#if	!defined(ntohl)
u_long	ntohl		args( ( u_long hostlong ) );
#endif
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

#if	defined(sequent)
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
int	close		args( ( int fd ) );
int	fcntl		args( ( int fd, int cmd, int arg ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
#if	!defined(htons)
u_short	htons		args( ( u_short hostshort ) );
#endif
int	listen		args( ( int s, int backlog ) );
#if	!defined(ntohl)
u_long	ntohl		args( ( u_long hostlong ) );
#endif
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	setsockopt	args( ( int s, int level, int optname, caddr_t optval,
			    int optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

/* This includes Solaris Sys V as well */
#if defined(sun)
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
void	bzero		args( ( char *b, int length ) );
int	close		args( ( int fd ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	listen		args( ( int s, int backlog ) );
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );

#if !defined(__SVR4)
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );

#if defined(SYSV)
int setsockopt		args( ( int s, int level, int optname,
			    const char *optval, int optlen ) );
#else
int	setsockopt	args( ( int s, int level, int optname, void *optval,
			    int optlen ) );
#endif
#endif
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

#if defined(ultrix)
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
void	bzero		args( ( char *b, int length ) );
int	close		args( ( int fd ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	setsockopt	args( ( int s, int level, int optname, void *optval,
			    int optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif



/*
 * Global variables.
 */
DESCRIPTOR_DATA *   descriptor_list;	/* All open descriptors		*/
DESCRIPTOR_DATA *   d_next;		/* Next descriptor in loop	*/
FILE *		    fpReserve;		/* Reserved file handle		*/
bool		    god;		/* All new chars are gods!	*/
bool		    merc_down;		/* Shutdown			*/
bool		    wizlock;		/* Game is wizlocked		*/
bool		    newlock;		/* Game is newlocked		*/
char		    str_boot_time[MAX_INPUT_LENGTH];
time_t		    current_time;	/* time of this pulse */	
bool		    MOBtrigger = TRUE;	/* act() switch			*/


/*
 * OS-dependent local functions.
 */
#if defined(macintosh) || defined(MSDOS)
void	game_loop_mac_msdos	args( ( void ) );
bool	read_from_descriptor	args( ( DESCRIPTOR_DATA *d ) );
bool	write_to_descriptor	args( ( int desc, char *txt, int length ) );
#endif

#if defined(unix)
void	game_loop_unix		args( ( int control ) );
int	init_socket		args( ( int port ) );
void	init_descriptor		args( ( int control ) );
bool	read_from_descriptor	args( ( DESCRIPTOR_DATA *d ) );
bool	write_to_descriptor	args( ( int desc, char *txt, int length ) );
#endif




/*
 * Other local functions (OS-independent).
 */
bool	check_parse_name	args( ( char *name ) );
bool	check_reconnect		args( ( DESCRIPTOR_DATA *d, char *name,
				    bool fConn ) );
bool	check_playing		args( ( DESCRIPTOR_DATA *d, char *name ) );
int	main			args( ( int argc, char **argv ) );
void	nanny			args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	process_output		args( ( DESCRIPTOR_DATA *d, bool fPrompt ) );
void	read_from_buffer	args( ( DESCRIPTOR_DATA *d ) );
void	stop_idling		args( ( CHAR_DATA *ch ) );
void    bust_a_prompt           args( ( CHAR_DATA *ch ) );
void	desc_gen		args( ( CHAR_DATA *ch ) );
void	clear_character		args( ( CHAR_DATA *ch ) );
void	confirm_new_password args( (DESCRIPTOR_DATA *d, char *argument) );
void	get_new_password	args( (DESCRIPTOR_DATA *d, char *argument) );

/* For Copyover */
int port, control;

bool parse_gen_physical		args( (CHAR_DATA *ch,char *argument) );
bool parse_gen_social		args( (CHAR_DATA *ch,char *argument) );
bool parse_gen_mental		args( (CHAR_DATA *ch,char *argument) );

bool parse_gen_talents		args( (CHAR_DATA *ch,char *argument) );
bool parse_gen_skills		args( (CHAR_DATA *ch,char *argument) );
bool parse_gen_knowledges	args( (CHAR_DATA *ch,char *argument) );

bool parse_gen_virtues	args( (CHAR_DATA *ch,char *argument) );
void purge_socials		args ( ( ) );

void cleanup_mud(int control)
{
	MOB_INDEX_DATA *mob_index;
	ROOM_INDEX_DATA *room_index;
	OBJ_INDEX_DATA *obj_index;
	SHOP_DATA *pShop, *pShop_next;

	AREA_DATA *pArea, *pArea_next;
	CHAR_DATA *ch, *ch_next;
	OBJ_DATA *obj, *obj_next;
	PACK_DATA *pack, *pack_next;
	ORG_DATA *org, *org_next;
	STOCKS *stock, *stock_next;
	NEWSPAPER *paper, *paper_next;
	DESCRIPTOR_DATA *d, *d_next;
	NOTE_DATA *pbg, *pbg_next;
	NOTE_DATA *pnote, *pnote_next;
	HELP_DATA *help, *help_next;
	PLOT_DATA *pPlot, *pPlot_next;
	EVENT_DATA *pEvent, *pEvent_next;

	int iHash = 0;
	int mob_count = 0, obj_count = 0, room_count = 0;

	log_string(LOG_ERR, "Project Twilight is shutting down and cleaning up its memory.");

	close(control);
	control = -1;
	log_string(LOG_ERR, "cleaning character list");
	for(ch = char_list; ch != NULL; ch = ch_next)
	{
		ch_next = ch->next;

		extract_char(ch, true);
	}

	log_string(LOG_ERR, "cleaning objects");
	for(obj = object_list; obj != NULL; obj = obj_next)
	{
		obj_next = obj->next;

		extract_obj(obj);
	}

	log_string(LOG_ERR, "cleaning helpfiles");
	for(help = help_list; help; help = help_next) {
		help_next = help->next;

		free_help(help);
	}

	log_string(LOG_ERR, "cleaning tips");
	for(help = tip_list; help; help = help_next) {
		help_next = help->next;

		free_help(help);
	}

	log_string(LOG_ERR, "cleaning packs");
	for(pack = pack_list; pack != NULL; pack = pack_next)
	{
		pack_next = pack->next;

		free_pack(pack);
	}

	log_string(LOG_ERR, "cleaning organizations");
	for(org = org_list; org != NULL; org = org_next)
	{
		org_next = org->next;

		free_org(org);
	}

	log_string(LOG_ERR, "cleaning stocks");
	for(stock = stock_list; stock != NULL; stock = stock_next)
	{
		stock_next = stock->next;

		free_stock(stock);
	}

	log_string(LOG_ERR, "cleaning news papers");
	for(paper = paper_list; paper != NULL; paper = paper_next)
	{
		paper_next = paper->next;

		free_newspaper(paper);
	}

	log_string(LOG_ERR, "cleaning active descriptors");
	for(d = descriptor_list; d != NULL; d = d_next)
	{
		d_next = d->next;

		free_descriptor(d);
	}

	log_string(LOG_ERR, "cleaning background list");
	for(pbg = bg_list; pbg != NULL; pbg = pbg_next)
	{
		pbg_next = pbg->next;

		free_note(pbg);
	}

	log_string(LOG_ERR, "Cleaning knowledge list");
	for(pbg = know_list; pbg != NULL; pbg = pbg_next)
	{
		pbg_next = pbg->next;

		free_note(pbg);
	}

	log_string(LOG_ERR, "Cleaning news list");
	for(pnote = news_list; pnote != NULL; pnote = pnote_next)
	{
		pnote_next = pnote->next;

		free_note(pnote);
	}

	log_string(LOG_ERR, "Cleaning out index hashes.");
	for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	{
		MOB_INDEX_DATA *next_mob_index;
		OBJ_INDEX_DATA *next_obj_index;
		ROOM_INDEX_DATA *next_room_index;
		//log_string("cleaning out mob indexes");
		for( mob_index = mob_index_hash[iHash]; mob_index; mob_index = next_mob_index )
		{
			next_mob_index = mob_index->next;

			if( mob_index == mob_index_hash[iHash] )
				mob_index_hash[iHash] = mob_index->next;
			else
			{
				MOB_INDEX_DATA *tmid;

				for( tmid = mob_index_hash[iHash]; tmid; tmid = tmid->next )
				{
					if( tmid->next == mob_index )
						break;
				}
				if( !tmid )
					log_string(LOG_ERR, Format("cleanup_mud: mid not in hash list %d", mob_index->vnum ));
				else
					tmid->next = mob_index->next;
			}

			free_mob_index(mob_index);
			mob_count++;
		}
		//log_string("cleaning out object indexes");
		for( obj_index = obj_index_hash[iHash]; obj_index; obj_index = next_obj_index )
		{
			next_obj_index = obj_index->next;
			if( obj_index == obj_index_hash[iHash] )
				obj_index_hash[iHash] = obj_index->next;
			else
			{
				OBJ_INDEX_DATA *toid;

				for( toid = obj_index_hash[iHash]; toid; toid = toid->next )
				{
					if( toid->next == obj_index )
						break;
				}
				if( !toid )
					log_string(LOG_ERR, Format("cleanup_mud: oid not in hash list %d", obj_index->vnum ));
				else
					toid->next = obj_index->next;
			}
			free_obj_index(obj_index);
			obj_count++;
		}
		//log_string("cleaning out room indexes.");
		for( room_index = room_index_hash[iHash]; room_index; room_index = next_room_index )
		{
			next_room_index = room_index->next;

			if( room_index == room_index_hash[iHash] )
				room_index_hash[iHash] = room_index->next;
			else
			{
				ROOM_INDEX_DATA *trid;

				for( trid = room_index_hash[iHash]; trid; trid = trid->next )
				{
					if( trid->next == room_index )
						break;
				}

				if( !trid )
					log_string(LOG_ERR, Format("cleanup_mud: rid not in hash list %d", room_index->vnum ));
				else
					trid->next = room_index->next;
			}

			free_room_index(room_index);
			room_count++;
		}
	}

	log_string(LOG_ERR, "Cleaning out area list");
	for ( pArea = area_first; pArea != NULL; pArea = pArea_next )
	{
		pArea_next = pArea->next;
		//delete pArea;
		free_area(pArea);
	}

	log_string(LOG_ERR, "cleaning out remaining shop data");
	for(pShop = shop_list; pShop != NULL; pShop = pShop_next )
	{
		pShop_next = pShop->next;

		free_shop(pShop);
	}

	log_string(LOG_ERR, "cleaning out social table.");
	purge_socials();

	log_string(LOG_ERR, "cleaning out plots");
	for(pPlot = plot_list; pPlot; pPlot = pPlot_next) {
		pPlot_next = pPlot->next;

		free_plot(pPlot);
	}

	log_string(LOG_ERR, "Cleaning out events / scripts / actors ");
	for(pEvent = event_list; pEvent; pEvent = pEvent_next) {
		pEvent_next = pEvent->next;

		free_event(pEvent);
	}

    purgeExtractedWorldData();
	log_string(LOG_GAME, "Project Twilight has completed its cleanup procedure and may now shutdown.");
	return;
}

int main( int argc, char **argv )
{
	struct timeval now_time;
	bool fCopyOver = FALSE;

	/*
	 * Init time.
	 */
	gettimeofday( &now_time, NULL );
	current_time 	= (time_t) now_time.tv_sec;
	strncpy( str_boot_time, ctime( &current_time ), sizeof(str_boot_time));

	/*
	 * Reserve one channel for our use.
	 */
	if ( ( fpReserve = fopen( NULL_FILE, "r" ) ) == NULL )
	{
		perror( NULL_FILE );
		exit( 1 );
	}

	/*
	 * Get the port number and initial log file number.
	 */
	port = MUD_PORT;
	if ( argc > 1 )
	{
		if ( !is_number( argv[1] ) )
		{
			fprintf( stderr, "Usage: %s [port #] [first log #]\n", argv[0] );
			exit( 1 );
		}
		else if ( ( port = atoi( argv[1] ) ) <= 1024 )
		{
			fprintf( stderr, "Port number must be above 1024.\n" );
			exit( 1 );
		}

		/* Are we recovering from a copyover? */
		if (argv[3] && argv[3][0])
		{
			fCopyOver = TRUE;
			control = atoi(argv[3]);
		}
		else
			fCopyOver = FALSE;
	}

	if( argv[2] == NULL || !is_number( argv[2] ) )
		snprintf(logfile, sizeof(logfile), "%d", 1000);
	else
		snprintf(logfile, sizeof(logfile), "%s", argv[2] );

	/*
	 * Run the game.
	 */
//	if (!fCopyOver)
		control = init_socket( port );

	boot_db( );
	/* init_web(port+1); */
	log_string( LOG_CONNECT, Format("Project Twilight loaded on port %d.", port) );
	log_to_file(logfile, GLOBAL_XML_IN, "Project Twilight restarted.");
/*
	if(fCopyOver)
		copyover_recover();
*/
	game_loop_unix( control );
	/* shutdown_web(); */

	cleanup_mud(control);


	/*
	 * That's all, folks.
	 */
	log_string( LOG_GAME, "Normal termination of game." );
	if(fpReserve)
	{
		closeReserve();
	}
	exit( 0 );
	return 0;
}



#if defined(unix)
int init_socket( int port )
{
	static struct sockaddr_in sa_zero;
	struct sockaddr_in sa;
	int x = 1;
	int fd;

	if ( ( fd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
	{
		perror( "Init_socket: socket" );
		exit( 1 );
	}

	if ( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR,
			(char *) &x, sizeof(x) ) < 0 )
	{
		perror( "Init_socket: SO_REUSEADDR" );
		close(fd);
		exit( 1 );
	}

#if defined(SO_DONTLINGER) && !defined(SYSV)
	{
		struct	linger	ld;

		ld.l_onoff  = 1;
		ld.l_linger = 1000;

		if ( setsockopt( fd, SOL_SOCKET, SO_DONTLINGER,
				(char *) &ld, sizeof(ld) ) < 0 )
		{
			perror( "Init_socket: SO_DONTLINGER" );
			close(fd);
			exit( 1 );
		}
	}
#endif

	sa		    = sa_zero;
	sa.sin_family   = AF_INET;
	sa.sin_port	    = htons( port );

	if ( bind( fd, (struct sockaddr *) &sa, sizeof(sa) ) < 0 )
	{
		perror("Init socket: bind" );
		close(fd);
		exit(1);
	}


	if ( listen( fd, 3 ) < 0 )
	{
		perror("Init socket: listen");
		close(fd);
		exit(1);
	}

	return fd;
}
#endif



#if defined(macintosh) || defined(MSDOS)
void game_loop_mac_msdos( void )
{
	struct timeval last_time;
	struct timeval now_time;
	static DESCRIPTOR_DATA dcon;

	gettimeofday( &last_time, NULL );
	current_time = (time_t) last_time.tv_sec;

	/*
	 * New_descriptor analogue.
	 */
	dcon.descriptor	= 0;
	dcon.connected	= CON_GET_NAME;
	PURGE_DATA( dcon.host );
	dcon.host		= str_dup( "localhost" );
	dcon.outsize	= 2000;
	ALLOC_DATA(dcon.outbuf, DESCRIPTOR_DATA, dcon.outsize);
	dcon.next		= descriptor_list;
	dcon.showstr_head	= NULL;
	dcon.showstr_point	= NULL;
	dcon.pEdit		= NULL; /* OLC */
	dcon.pString	= NULL;
	dcon.editor		= 0;	/* End OLC */
	descriptor_list	= &dcon;

	/*
	 * Send the greeting.
	 */
	{
		extern char * help_greeting;
		if ( help_greeting[0] == '.' )
			write_to_buffer( &dcon, help_greeting+1, 0 );
		else
			write_to_buffer( &dcon, help_greeting  , 0 );
	}

	/* Main loop */
	while ( !merc_down )
	{
		DESCRIPTOR_DATA *d;

		/*
		 * Process input.
		 */
		for ( d = descriptor_list; d != NULL; d = d_next )
		{
			d_next	= d->next;
			d->fcommand	= FALSE;

#if defined(MSDOS)
			if ( kbhit( ) )
#endif
			{
				if ( d->character != NULL )
					d->character->timer = 0;
				if ( !read_from_descriptor( d ) )
				{
					if ( d->character != NULL && d->connected == CON_PLAYING)
						save_char_obj( d->character );
					d->outtop	= 0;
					close_socket( d );
					continue;
				}
			}

			if (d->character != NULL && d->character->daze > 0)
				--d->character->daze;

			if ( d->character != NULL && d->character->wait > 0 )
			{
				--d->character->wait;
				continue;
			}

			read_from_buffer( d );
			if ( !IS_NULLSTR(d->incomm) )
			{
				d->fcommand	= TRUE;
				if ( d->pProtocol != NULL )
					d->pProtocol->WriteOOB = 0;
				stop_idling( d->character );

				/*OLC*/
				if ( d->showstr_point )
					show_string( d, d->incomm );
				else
					if ( d->pString )
						string_add( s->character, d->incomm );
					else
						switch ( d->connected )
						{
						case CON_PLAYING:
							if( !run_olc_editor( d ))
								substitute_alias( d, d->incomm );
							break;
						default:
							nanny( d, d->incomm );
							break;
						}

				d->incomm[0]	= '\0';
			}
		}



		/*
		 * Autonomous game motion.
		 */
		update_handler( );



		/*
		 * Output.
		 */
		for ( d = descriptor_list; d != NULL; d = d_next )
		{
			d_next = d->next;

			if ( ( d->fcommand || d->outtop > 0 ) )
			{
				if ( !process_output( d, TRUE ) )
				{
					if ( d->character != NULL && d->connected == CON_PLAYING)
						save_char_obj( d->character );
					d->outtop	= 0;
					close_socket( d );
				}
			}
		}



		/*
		 * Synchronize to a clock.
		 * Busy wait (blargh).
		 */
		now_time = last_time;
		for ( ; ; )
		{
			int delta;

#if defined(MSDOS)
			if ( kbhit( ) )
#endif
			{
				if ( dcon.character != NULL )
					dcon.character->timer = 0;
				if ( !read_from_descriptor( &dcon ) )
				{
					if ( dcon.character != NULL && d->connected == CON_PLAYING)
						save_char_obj( d->character );
					dcon.outtop	= 0;
					close_socket( &dcon );
				}
#if defined(MSDOS)
				break;
#endif
			}

			gettimeofday( &now_time, NULL );
			delta = ( now_time.tv_sec  - last_time.tv_sec  ) * 1000 * 1000
					+ ( now_time.tv_usec - last_time.tv_usec );
			if ( delta >= 1000000 / PULSE_PER_SECOND )
				break;
		}
		last_time    = now_time;
		current_time = (time_t) last_time.tv_sec;
	}

	return;
}
#endif



#if defined(unix)
void game_loop_unix( int control )
{
	static struct timeval null_time;
	struct timeval last_time;

	signal( SIGPIPE, SIG_IGN );
	gettimeofday( &last_time, NULL );
	current_time = (time_t) last_time.tv_sec;


	/* Main loop */
	while ( !merc_down )
	{
		fd_set in_set;
		fd_set out_set;
		fd_set exc_set;
		DESCRIPTOR_DATA *d;
		int maxdesc;

		/*
		 * Poll all active descriptors.
		 */
		FD_ZERO( &in_set  );
		FD_ZERO( &out_set );
		FD_ZERO( &exc_set );
		FD_SET( control, &in_set );
		maxdesc	= control;
		for ( d = descriptor_list; d; d = d->next )
		{
			maxdesc = UMAX( maxdesc, d->descriptor );
			FD_SET( d->descriptor, &in_set  );
			FD_SET( d->descriptor, &out_set );
			FD_SET( d->descriptor, &exc_set );

		}

		if ( select( maxdesc+1, &in_set, &out_set, &exc_set, &null_time ) < 0 )
		{
			perror( "Game_loop: select: poll" );
			exit( 1 );
		}

		/*
		 * New connection?
		 */
		if ( FD_ISSET( control, &in_set ) )
			init_descriptor( control );

		/*
		 * Kick out the freaky folks.
		 */
		for ( d = descriptor_list; d != NULL; d = d_next )
		{
			d_next = d->next;
			if ( FD_ISSET( d->descriptor, &exc_set ) )
			{
				FD_CLR( d->descriptor, &in_set  );
				FD_CLR( d->descriptor, &out_set );
				if ( d->character && d->connected == CON_PLAYING)
					save_char_obj( d->character );
				d->outtop	= 0;
				close_socket( d );
			}
		}

		/*
		 * Process input.
		 */
		for ( d = descriptor_list; d != NULL; d = d_next )
		{
			d_next	= d->next;
			d->fcommand	= FALSE;

			if ( FD_ISSET( d->descriptor, &in_set ) )
			{
				if ( d->character != NULL )
					d->character->timer = 0;
				if ( !read_from_descriptor( d ) )
				{
					FD_CLR( d->descriptor, &out_set );
					if ( d->character != NULL && d->connected == CON_PLAYING)
						save_char_obj( d->character );
					d->outtop	= 0;
					close_socket( d );
					continue;
				}
			}

			if (d->character != NULL && d->character->daze > 0)
				--d->character->daze;

			if ( d->character != NULL && d->character->wait > 0 )
			{
				//log_string("wait");
				--d->character->wait;
				continue;
			}

			read_from_buffer( d );
			if ( !IS_NULLSTR(d->incomm) )
			{
				d->fcommand	= TRUE;
				if ( d->pProtocol != NULL )
					d->pProtocol->WriteOOB = 0;
				stop_idling( d->character );

				/* OLC */
				if(d->showstr_point)
					show_string( d, d->incomm );
				else
					if( d->pString )
						string_add( d->character, d->incomm );
					else
						switch( d->connected )
						{
						case CON_PLAYING:
							if( !run_olc_editor( d ) )
								substitute_alias( d, d->incomm );
							break;
						case CON_MUDLINK:
							break;
						case CON_AIDESC:
							break;
						default:
							nanny( d, d->incomm );
							break;
						}

				d->incomm[0]	= '\0';
			}
		}



		/*
		 * Autonomous game motion.
		 */
		update_handler( );

		/*
		 * Web handler.
		 */
		/* handle_web(); */



		/*
		 * Output.
		 */
		for ( d = descriptor_list; d != NULL; d = d_next )
		{
			d_next = d->next;

			if ( ( d->fcommand || d->outtop > 0 )
					&&   FD_ISSET(d->descriptor, &out_set) )
			{
				if ( !process_output( d, TRUE ) )
				{
					if ( d->character != NULL && d->connected == CON_PLAYING)
						save_char_obj( d->character );
					d->outtop	= 0;
					close_socket( d );
				}
			}
		}

	    purgeExtractedWorldData();

		/*
		 * Synchronize to a clock.
		 * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
		 * Careful here of signed versus unsigned arithmetic.
		 */
		{
			struct timeval now_time;
			long secDelta;
			long usecDelta;

			gettimeofday( &now_time, NULL );
			usecDelta	= ((int) last_time.tv_usec) - ((int) now_time.tv_usec)
					+ 1000000 / PULSE_PER_SECOND;
			secDelta	= ((int) last_time.tv_sec ) - ((int) now_time.tv_sec );
			while ( usecDelta < 0 )
			{
				usecDelta += 1000000;
				secDelta  -= 1;
			}

			while ( usecDelta >= 1000000 )
			{
				usecDelta -= 1000000;
				secDelta  += 1;
			}

			if ( secDelta > 0 || ( secDelta == 0 && usecDelta > 0 ) )
			{
				struct timeval stall_time;

				stall_time.tv_usec = usecDelta;
				stall_time.tv_sec  = secDelta;
				if ( select( 0, NULL, NULL, NULL, &stall_time ) < 0 )
				{
					perror( "Game_loop: select: stall" );
					exit( 1 );
				}
			}
		}

		gettimeofday( &last_time, NULL );
		current_time = (time_t) last_time.tv_sec;
	}

	return;
}
#endif



#if defined(unix)
void init_descriptor( int control )
{
	DESCRIPTOR_DATA *dnew;
	struct sockaddr_in sock;
	struct hostent *from;
	int desc = 0;
	int size = 0;

	size = sizeof(sock);
	getsockname( control, (struct sockaddr *) &sock, (socklen_t *)&size );
	if((desc = accept(control,(struct sockaddr *)&sock,(socklen_t *)&size)) < 0)
	{
		perror( "New_descriptor: accept" );
		return;
	}

#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

	if ( fcntl( desc, F_SETFL, FNDELAY ) == -1 )
	{
		perror( "New_descriptor: fcntl: FNDELAY" );
		return;
	}

	/*
	 * Cons a new descriptor.
	 */
	dnew = new_descriptor();

	assert(dnew); // if calloc fails, we should never see this, but, just incase.

	dnew->descriptor = desc;

	dnew->pProtocol     = ProtocolCreate();
	assert(dnew->pProtocol); // if ProtocolCreate fails, then we have an issue, we will assert out!

	size = sizeof(sock);
	if (getpeername( desc, (struct sockaddr *) &sock, (socklen_t *)&size ) < 0)
	{
		perror( "New_descriptor: getpeername" );
		PURGE_DATA( dnew->host );
		dnew->host = str_dup( "(unknown)" );
	}
	else
	{
		/*
		 * Would be nice to use inet_ntoa here but it takes a struct arg,
		 * which ain't very compatible between gcc and system libraries.
		 */
		int addr;

		addr = ntohl( sock.sin_addr.s_addr );
		from = gethostbyaddr( (char *) &sock.sin_addr,
				sizeof(sock.sin_addr), AF_INET );
		PURGE_DATA( dnew->host );

		// got rid of the buffer,  and used format instead!
		dnew->host = str_dup( from ? from->h_name : Format("%d.%d.%d.%d",
                                ( addr >> 24 ) & 0xFF, ( addr >> 16 ) & 0xFF,
                                ( addr >>  8 ) & 0xFF, ( addr       ) & 0xFF)
		);

                log_string( LOG_CONNECT, Format("Sock.sinaddr:  %s", dnew->host) );
	}

	/*
	 * Swiftest: I added the following to ban sites.  I don't
	 * endorse banning of sites, but Copper has few descriptors now
	 * and some people from certain sites keep abusing access by
	 * using automated 'autodialers' and leaving connections hanging.
	 *
	 * Furey: added suffix check by request of Nickel of HiddenWorlds.
	 */
	if ( check_ban(dnew->host,BAN_ALL))
	{
		write_to_descriptor( desc,
				"Your site has been banned from this mud.\n\r", 0 );
		close( desc );
		free_descriptor(dnew);
		return;
	}
	/*
	 * Init descriptor data.
	 */
	LINK_SINGLE(dnew, next, descriptor_list);

	ProtocolNegotiate(dnew);


	/*
	 * Send the greeting.
	 */
	{
		extern char * help_greeting;
		if ( help_greeting[0] == '.' )
			write_to_buffer( dnew, help_greeting+1, 0 );
		else
			write_to_buffer( dnew, help_greeting  , 0 );
	}

	return;
}
#endif



void close_socket( DESCRIPTOR_DATA *dclose )
{
	CHAR_DATA *ch;

	if ( dclose->outtop > 0 && dclose->outsize <32000 )
		process_output( dclose, FALSE );

	if ( dclose->snoop_by != NULL )
	{
		write_to_buffer( dclose->snoop_by,
				"Your victim has left the game.\n\r", 0 );
	}

	{
		DESCRIPTOR_DATA *d;

		for ( d = descriptor_list; d != NULL; d = d->next )
		{
			if ( d->snoop_by == dclose )
				d->snoop_by = NULL;
		}
	}

	if ( ( ch = dclose->character ) != NULL )
	{
		log_string( LOG_CONNECT, Format("Closing link to %s.", ch->name) );
		/* cut down on wiznet spam when rebooting */
		if ( dclose->connected == CON_PLAYING && !merc_down)
		{
			act( "$n has lost $s link.", ch, NULL, NULL, TO_ROOM, 1 );
			wiznet("\tY[WIZNET]\tn Net death has claimed $N.",ch,NULL,WIZ_LINKS,0,0);
			ch->desc = NULL;
		}
		else
		{
			free_char(dclose->original ? dclose->original :	dclose->character );
		}
	}

	if ( d_next == dclose )
		d_next = d_next->next;

	if ( dclose == descriptor_list )
	{
		descriptor_list = descriptor_list->next;
	}
	else
	{
		DESCRIPTOR_DATA *d;

		for ( d = descriptor_list; d && d->next != dclose; d = d->next )
			;
		if ( d != NULL )
			d->next = dclose->next;
		else
			log_string(LOG_BUG, "Close_socket: dclose not found.");
	}

	ProtocolDestroy( dclose->pProtocol );

	shutdown(dclose->descriptor, SHUT_RDWR);
	close( dclose->descriptor );
	free_descriptor(dclose);
#if defined(MSDOS) || defined(macintosh)
	exit(1);
#endif
	return;
}



bool read_from_descriptor( DESCRIPTOR_DATA *d )
{
	int iStart = 0;

	static char read_buf[MAX_PROTOCOL_BUFFER]={'\0'}; /* <--- Add this line */
	read_buf[0] = '\0';                        /* <--- Add this line */

	/* Hold horses if pending command already. */
	if ( !IS_NULLSTR(d->incomm) )
		return TRUE;

	/* Check for overflow. */
	iStart = 0;
	if ( strlen(d->inbuf) >= sizeof(d->inbuf) - 10 )
	{
		log_string( LOG_ERR, Format("%s input overflow!", d->host) );
		write_to_descriptor( d->descriptor,
				"\n\r*** PUT A LID ON IT!!! ***\n\r", 0 );
		return FALSE;
	}

	/* Snarf input. */
#if defined(macintosh)
	for ( ; ; )
	{
		int c;
		c = getc( stdin );
		if ( c == '\0' || c == EOF )
			break;
		putc( c, stdout );
		if ( c == '\r' )
			putc( '\n', stdout );
		read_buf[iStart++] = c;
		if ( iStart > sizeof(d->inbuf) - 10 )
			break;
	}
#endif

#if defined(MSDOS) || defined(unix)
	for ( ; ; )
	{
		int nRead;

		nRead = read( d->descriptor, read_buf + iStart, sizeof(read_buf) - 10 - iStart );
		if ( nRead > 0 )
		{
			iStart += nRead;
			if ( read_buf[iStart-1] == '\n' || read_buf[iStart-1] == '\r' )
				break;
		}

		else if ( nRead == 0 )
		{
			log_string( LOG_ERR, "EOF encountered on read." );
			return FALSE;
		}
		else if ( errno == EWOULDBLOCK )
			break;
		else
		{
			perror( "Read_from_descriptor" );
			return FALSE;
		}
	}
#endif

	read_buf[iStart] = '\0';
	if(d->pProtocol)
		ProtocolInput( d, read_buf, iStart, d->inbuf );
	else
		strncpy(d->inbuf, read_buf, MSL*4);

	return TRUE;
}



/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer( DESCRIPTOR_DATA *d )
{
    int i = 0, j = 0, k = 0;

    /*
     * Hold horses if pending command already.
     */
    if ( !IS_NULLSTR(d->incomm) )
	return;

    /*
     * Look for at least one new line.
     */
    for ( i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
	if ( d->inbuf[i] == '\0' )
	    return;
    }

    /*
     * Canonical input processing.
     */
    for ( i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
	if ( k >= MAX_INPUT_LENGTH - 2 )
	{
	    write_to_descriptor( d->descriptor, "Line too long.\n\r", 0 );

	    /* skip the rest of the line */
	    for ( ; !IS_NULLSTR(d->inbuf) ; i++ )
	    {
		if ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
		    break;
	    }
	    d->inbuf[i]   = '\n';
	    d->inbuf[i+1] = '\0';
	    break;
	}

	if ( d->inbuf[i] == '\b' && k > 0 )
	    --k;
	else if ( isascii(d->inbuf[i]) && isprint(d->inbuf[i]) )
	    d->incomm[k++] = d->inbuf[i];
    }

    /*
     * Finish off the line.
     */
    if ( k == 0 )
	d->incomm[k++] = ' ';
    d->incomm[k] = '\0';

    /*
     * Deal with bozos with #repeat 1000 ...
     */

    if ( k > 1 || d->incomm[0] == '!' )
    {
    	if ( d->incomm[0] != '!' && str_cmp( d->incomm, d->inlast ) )
	{
	    d->repeat = 0;
	}
	else
	{
	    if (++d->repeat >= 25 && d->character
	    &&  d->connected == CON_PLAYING)
	    {
		log_string( LOG_ERR, Format("%s@%s input spamming!", d->character?d->character->name:"Unknown", d->host) );
		wiznet("\tY[WIZNET]\tn Spam spam spam $N spam spam spam spam spam!",
		       d->character,NULL,WIZ_SPAM,0,get_trust(d->character));
		if (d->incomm[0] == '!')
		    wiznet(d->inlast,d->character,NULL,WIZ_SPAM,0,
			get_trust(d->character));
		else
		    wiznet(d->incomm,d->character,NULL,WIZ_SPAM,0,
			get_trust(d->character));

		d->repeat = 0;
/*
		write_to_descriptor( d->descriptor,
		    "\n\r*** PUT A LID ON IT!!! ***\n\r", 0 );
		strncpy( d->incomm, "quit", sizeof(d->incomm);
*/
	    }
	}
    }


    /*
     * Do '!' substitution.
     */
    if ( d->incomm[0] == '!' )
	strncpy( d->incomm, d->inlast, sizeof(d->incomm));
    else
	strncpy( d->inlast, d->incomm, sizeof(d->inlast));

    /*
     * Shift the input buffer.
     */
    while ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
	i++;
    for ( j = 0; ( d->inbuf[j] = d->inbuf[i+j] ) != '\0'; j++ )
	;
    return;
}



/*
 * Low level output function.
 */
bool process_output( DESCRIPTOR_DATA *d, bool fPrompt )
{
	extern bool merc_down;

	/*
	 * Bust a prompt.
	 */
		if (!merc_down)
		{
			if ( d->pProtocol->WriteOOB ) /* <-- Add this, and the ";" and "else" */
			        ; /* The last sent data was OOB, so do NOT draw the prompt */
			else if ( d->showstr_point)
				write_to_buffer( d, "[Hit Return to continue]\n\r", 0 );
			else if ( fPrompt && d->pString && d->connected == CON_PLAYING )
				write_to_buffer( d, "> ", 2 );
			else if ( fPrompt && d->connected == CON_PLAYING )
			{
				CHAR_DATA *ch;
				CHAR_DATA *victim;

				ch = d->character;

				/* battle prompt */
				if ((victim = ch->fighting) != NULL && can_see(ch,victim))
				{
					char buf[MSL]={'\0'};
					const char *vhdisp;
					const char *chdisp;
					int vhealth, chealth;
					char *pbuff;
					char buffer[MSL*2]={'\0'};

					if(ch == NULL || victim == NULL)
						return FALSE;

					vhealth = victim->health + victim->agghealth -7;
					chealth = ch->health + ch->agghealth - 7;

					if(vhealth < 0 || chealth < 0)
						return TRUE;

					if(vhealth > 7) vhealth = 7;
					if(chealth > 7) chealth = 7;

					vhdisp = str_dup(health_string(victim));

					chdisp = str_dup(health_string(ch));

					snprintf(buf, sizeof(buf), "<%s is %s, You are %s> \n\r",
							IS_NPC(victim) ? victim->short_descr : victim->name, vhdisp, chdisp );
					buf[0]	= UPPER( buf[0] );
					pbuff	= buffer;
					write_to_buffer( d, buffer, 0);
				}

				ch = d->original ? d->original : d->character;
				if (!IS_SET(ch->comm, COMM_COMPACT) )
					write_to_buffer( d, "\n\r", 2 );


				if ( IS_SET(ch->comm, COMM_PROMPT) )
					bust_a_prompt( d->character );

				if (IS_SET(ch->comm,COMM_TELNET_GA))
					write_to_buffer(d,go_ahead_str,0);
			}
		}

	/*
	 * Short-circuit if nothing to write.
	 */
	if ( d->outtop == 0 )
		return TRUE;

	/*
	 * Snoop-o-rama.
	 */
	if ( d->snoop_by != NULL )
	{
		if (d->character != NULL)
			write_to_buffer( d->snoop_by, d->character->name,0);
		write_to_buffer( d->snoop_by, "> ", 2 );
		write_to_buffer( d->snoop_by, d->outbuf, d->outtop );
	}

	/*
	 * OS-dependent output.
	 */
	if ( !write_to_descriptor( d->descriptor, d->outbuf, d->outtop ) )
	{
		d->outtop = 0;
		return FALSE;
	}
	else
	{
		d->outtop = 0;
		return TRUE;
	}
}

extern char *  const   month_name      [];

/*
 * Bust a prompt (player settable prompt)
 * coded by Morgenes for Aldara Mud
 * Extended for use on PT by Dsarky.
 */
void bust_a_prompt( CHAR_DATA *ch )
{
	char buf2[MSL]={'\0'};
	const char *str;
	const char *i;
	char *point;
	char *pbuff;
	char buffer[ MSL*2 ]={'\0'};
	char doors[MAX_INPUT_LENGTH]={'\0'};
	EXIT_DATA *pexit;
	bool found;
	const char *dir_name[] = {"N","E","S","W","U","D"};
	int door = 0;
	char *suf;
	int day = time_info.day + 1;
	STOCKS *stock;

	if ( day > 4 && day <  20 ) suf = "th";
	else if ( day % 10 ==  1       ) suf = "st";
	else if ( day % 10 ==  2       ) suf = "nd";
	else if ( day % 10 ==  3       ) suf = "rd";
	else                             suf = "th";


	point = buffer;
	str = ch->prompt;

	if (IS_SET(ch->comm,COMM_AFK))
	{
		send_to_char("<AFK> ",ch);
		return;
	}

	if (str == NULL || str[0] == '\0')
	{
		door = ch->health + ch->agghealth - 7;
		if(door > 7) door = 7;
		send_to_char(Format("<%s> %s", health_string(ch),ch->prefix),ch);
		return;
	}

	while( *str != '\0' )
	{
		if( *str != '%' )
		{
			*point++ = *str++;
			continue;
		}
		++str;
		switch( *str )
		{
		default :
			i = " "; break;
		case 'e':
			found = FALSE;
			doors[0] = '\0';

			if (ch->position>P_SLEEP) {
				if (IS_AFFECTED(ch,AFF_BLIND) &&
						!IS_SET(ch->plr_flags,PLR_HOLYLIGHT)) {
					found=TRUE;
					strncpy(doors,"blinded", sizeof(doors));
				} else {
					for (door = 0; door < 6; door++)
					{
						if ((pexit = ch->in_room->exit[door]) != NULL
								&&  pexit ->u1.to_room != NULL
								&&  (can_see_room(ch,pexit->u1.to_room)
										||   (IS_AFFECTED(ch,AFF_INFRARED)
												&&    !IS_AFFECTED(ch,AFF_BLIND)))
												&&  !IS_SET(pexit->exit_info,EX_CLOSED))
						{
							found = TRUE;
							strncat(doors,dir_name[door], sizeof(doors));
						}
					}
				}
			} else {
				strncpy(doors,"sleeping", sizeof(doors));
				found=TRUE;
			}
	    if (!found)
	 	strncat(doors,"none", sizeof(doors));
	    snprintf(buf2, sizeof(buf2), "%s",doors);
	    i = buf2; break;
 	 case 'n' :
	    snprintf(buf2, sizeof(buf2), "%s","\n\r");
	    i = buf2; break;
         case 'h' :
            snprintf( buf2, sizeof(buf2), "%s", health_string(ch) );
            i = buf2; break;
         case 'x' :
            snprintf( buf2, sizeof(buf2), "%d", ch->exp );
            i = buf2; break;
         case 'g' :
            snprintf( buf2, sizeof(buf2), "%d", ch->xpgift );
            i = buf2; break;
         case 'd' :
            snprintf( buf2, sizeof(buf2), "%d", ch->dollars);
            i = buf2; break;
	 case 'c' :
	    snprintf( buf2, sizeof(buf2), "%d", ch->cents);
	    i = buf2; break;
         case '%' :
            snprintf( buf2, sizeof(buf2), "%%" );
            i = buf2; break;
	 case 'o' :
	    snprintf( buf2, sizeof(buf2), "%s", olc_ed_name(ch) );
	    i = buf2; break;
	 case 'O' :
	    snprintf( buf2, sizeof(buf2), "%s", olc_ed_vnum(ch) );
	    i = buf2; break;
	 case 'a' :
	    snprintf( buf2, sizeof(buf2), "%d", ch->RBPG );
	    i = buf2; break;
	 case 'A' :
	    snprintf( buf2, sizeof(buf2), "%d", ch->max_RBPG );
	    i = buf2; break;
	 case 'b' :
	    snprintf( buf2, sizeof(buf2), "%d", ch->GHB );
	    i = buf2; break;
	 case 'B' :
	    snprintf( buf2, sizeof(buf2), "%d", ch->max_GHB );
	    i = buf2; break;
	 case 'j' :
	    snprintf( buf2, sizeof(buf2), "%d", ch->pospts );
	    i = buf2; break;
	 case 'T' :
		snprintf( buf2, sizeof(buf2),
		    "%d%s, %d%s %s",
		    (time_info.hour % 12 == 0) ? 12 : time_info.hour %12,
		    time_info.hour >= 12 ? "pm" : "am", day, suf,
		    month_name[time_info.month]);
		i = buf2; break;
	 case 't' :
		snprintf( buf2, sizeof(buf2),
		    "%d%s",
		    (time_info.hour % 12 == 0) ? 12 : time_info.hour %12,
		    time_info.hour >= 12 ? "pm" : "am");
		i = buf2; break;
	case 's' :
		door = ch->stock_ticker;
		stock = stock_list;
		if(stock != NULL) {
		for(door = 0; door < ch->stock_ticker; door++)
		{
		    stock = stock->next;
		    if(stock == NULL) stock = stock_list;
		}
		snprintf( buf2, sizeof(buf2), "%s $%d.%d",
		    stock->name, stock->cost/100, stock->cost%100);
		ch->stock_ticker++;
		i = buf2;
		} else i = " ";
		break;
      }
      ++str;
      while( (*point = *i) != '\0' )
         ++point, ++i;
   }
   *point	= '\0';
   pbuff	= buffer;
   write_to_buffer( ch->desc, buffer, 0 );

   if (!IS_NULLSTR(ch->prefix))
	   write_to_buffer(ch->desc,ch->prefix,0);
   return;
}



/*
 * Append onto an output buffer.
 */
void write_to_buffer( DESCRIPTOR_DATA *d, const char *txt, int length )
{

	// don't write NULL data!
	if(txt == NULL || txt == '\0')
		return;

	length = 0;

    txt = ProtocolOutput( d, txt, &length );  /* <--- Add this line */
    if ( d->pProtocol->WriteOOB > 0 )         /* <--- Add this line */
        --d->pProtocol->WriteOOB;             /* <--- Add this line */
//	const char *new_txt = txt;
	/*
	 * Find length in case caller didn't.
	 */
	if ( length <= 0 )
		length = strlen(txt);

	/*
	 * Initial \n\r if needed.
	 */
	if ( d->outtop == 0 && !d->fcommand && !d->pProtocol->WriteOOB )
	{
		d->outbuf[0]	= '\n';
		d->outbuf[1]	= '\r';
		d->outtop	= 2;
	}

	/*
	 * Expand the buffer as needed.
	 */
	while ( d->outtop + length >= d->outsize )
	{
		char *outbuf;

		if (d->outsize >= 32000)
		{
			log_string(LOG_BUG, "Buffer overflow. Closing.");
			close_socket(d);
			return;
		}
		ALLOC_DATA(outbuf, char, 2 * d->outsize);
		strncpy( outbuf, d->outbuf, d->outtop );
		PURGE_DATA(d->outbuf);
		d->outbuf   = outbuf;
		d->outsize *= 2;
	}

	/*
	 * Copy.
	 */
	strncpy( d->outbuf + d->outtop, txt, length );
	d->outtop += length;
	return;
}



/*
 * Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 *   try lowering the max block size.
 */
bool write_to_descriptor( int desc, char *txt, int length )
{
    int iStart = 0;
    int nWrite = 0;
    int nBlock = 0;

#if defined(macintosh) || defined(MSDOS)
    if ( desc == 0 )
	desc = 1;
#endif

    if ( length <= 0 )
	length = strlen(txt);

    for ( iStart = 0; iStart < length; iStart += nWrite )
    {
	nBlock = UMIN( length - iStart, 4096 );
	if ( ( nWrite = write( desc, txt + iStart, nBlock ) ) < 0 )
	    { perror( "Write_to_descriptor" ); return FALSE; }
    } 

    return TRUE;
}



/*
 * Deal with sockets that haven't logged in yet.
 */
void nanny( DESCRIPTOR_DATA *d, char *argument )
{
	DESCRIPTOR_DATA *d_old, *d_next;
	DESCRIPTOR_DATA backup_d;
	CHAR_DATA *backup = NULL;
	CHAR_DATA *ch;
	char buf[MSL]={'\0'};
	char arg[MAX_INPUT_LENGTH]={'\0'};
	int iClass = 0,race = 0,i = 0;
	bool fOld = FALSE;

	int fBak = 0;
	int fRestore = 0;

	while ( isspace(*argument) )
		argument++;

	ch = d->character;

	switch ( d->connected )
	{

	default:
		log_string(LOG_BUG, Format("Nanny: bad d->connected %d.", d->connected ));
		close_socket( d );
		return;

	case CON_GET_ACCT_NAME:
		if ( IS_NULLSTR(argument) )
		{
			close_socket( d );
			return;
		}

		argument[0] = UPPER(argument[0]);
		if ( !check_parse_name( argument ) )
		{
			write_to_buffer( d, "\tRIllegal name, try another.\tn\n\rName: ", 0 );
			return;
		}

		/*  @@@@@
		if ( (d->account = get_acct(argument)) == NULL )
		{
			write_to_buffer( d, "Do you want to create an account"
		}
		*/

		write_to_buffer( d, "Password: ", 0 );
		d->connected = CON_GET_ACCT_PASS;
		return;

	case CON_GET_ACCT_PASS:
		write_to_buffer( d, "Menu:\n\r", 0 );
		write_to_buffer( d, "OPTIONS...", 0 ); /* i@@@@@ replace with options from table.*/
		d->connected = CON_ACCT_MENUCHOICE;
		return;

	case CON_ACCT_MENUCHOICE:
		write_to_buffer( d, "Nice one bright spark. Accounts are coming in a bit.\n\rName: ", 0 );
		d->connected = CON_GET_NAME;
		return;

	case CON_GET_NAME:
		if ( IS_NULLSTR(argument) )
		{
			close_socket( d );
			return;
		}

		argument[0] = UPPER(argument[0]);
		if ( !check_parse_name( argument ) )
		{
			write_to_buffer( d, "\tRIllegal name, try another.\tn\n\rName: ", 0 );
			return;
		}

		if(!str_cmp(argument, "accountsystem"))
		{
			/* Run account test system */
			d->connected = CON_GET_ACCT_NAME;
			write_to_buffer( d, "Account Login: ", 0 );
			return;
		}

		if(ch == NULL)
		{
			fOld = load_char_obj( d, argument, TRUE, FALSE, FALSE );
			ch   = d->character;
		}

		if (IS_SET(ch->plr_flags, PLR_DENY))
		{
			log_string( LOG_CONNECT, Format("Denying access to %s@%s.", argument, d->host) );
			write_to_buffer( d, "You are denied access.\n\r", 0 );
			close_socket( d );
			return;
		}

		if (check_ban(d->host,BAN_PERMIT)
				&& !IS_SET(ch->plr_flags,PLR_PERMIT))
		{
			log_string( LOG_CONNECT, Format("Denying access to %s@%s. Host %s banned.", argument, d->host, d->host) );
			write_to_buffer(d,"Your site has been banned from this mud.\n\r",0);
			close_socket(d);
			return;
		}

		if ( check_reconnect( d, argument, FALSE ) )
		{
			fOld = TRUE;
		}
		else
		{
			if ( wizlock && !IS_ADMIN(ch))
			{
				write_to_buffer( d, "The game is wizlocked.\n\r", 0 );
				close_socket( d );
				return;
			}
		}

		if ( fOld )
		{
			/* Old player */
			write_to_buffer( d, "Password: ", 0 );
			ProtocolNoEcho( d, true );
			d->connected = CON_GET_OLD_PASSWORD;
			return;
		}
		else
		{
			/* New player */
			if (newlock)
			{
				write_to_buffer( d, "The game is newlocked.\n\r", 0 );
				close_socket( d );
				return;
			}

			if (check_ban(d->host,BAN_NEWBIES) && !IS_SET(ch->act, ACT_REINCARNATE))
			{
				write_to_buffer(d, "New players are not allowed from your site.\n\r",0);
				close_socket(d);
				return;
			}

			write_to_buffer( d, Format("Did I get that right, %s (Y/N)? ", argument), 0 );
			d->connected = CON_CONFIRM_NEW_NAME;
			return;
		}
		break;

	case CON_REINCARNATE:
		if ( IS_NULLSTR(argument) )
		{
			write_to_buffer( d, "What name do you wish to reincarnate as? ", 0 );
			return;
		}

		one_argument(argument, arg);
		arg[0] = UPPER(arg[0]);
		if ( !check_parse_name( arg ) )
		{
			write_to_buffer( d, "Illegal name, try another.\n\rName: ", 0 );
			return;
		}

		PURGE_DATA(ch->oldname);
		ch->oldname = str_dup(ch->name);
		PURGE_DATA(ch->name);
		ch->name = str_dup(arg);
		write_to_buffer( d, Format("Did I get that right, %s (Y/N)? ", arg), 0 );
		d->connected = CON_CONFIRM_REINCARNATE_NAME;
		return;
		break;

	case CON_CONFIRM_REINCARNATE_NAME:
		switch ( *argument )
		{
		case 'y': case 'Y':
			write_to_buffer( d, Format("I'll need you to give me a password for this incarnation: %s", echo_off_str), 0 );
			if(!str_cmp(ch->name, ch->oldname));
			else
			{
				unlink( (char *)Format("%s%s", PLAYER_DIR, capitalize(ch->oldname)));
				save_char_obj(ch);
			}
			clear_character(ch);
			SET_BIT(ch->act, ACT_REINCARNATE);
			save_char_obj(ch);
			log_string( LOG_CONNECT, Format("%s has reincarnated as %s", capitalize(ch->oldname), capitalize(ch->name)));
			wiznet((char*)Format("\tY[WIZNET]\tn %s has reincarnated as %s", capitalize(ch->oldname)),NULL,NULL,WIZ_NEWBIE,0,0);
			PURGE_DATA(ch->oldname);
			ch->trust = 0;
			ch->bg_timer = 6;
			ch->bg_count = 0;
			d->connected = CON_GET_NEW_PASSWORD;
			break;

		case 'n': case 'N':
			write_to_buffer( d, "What name will you reincarnate to? ", 0);
			d->connected = CON_REINCARNATE;
			break;

		default:
			write_to_buffer( d, "Please type Yes or No? ", 0 );
			break;
		}
		break;

		case CON_GET_OLD_PASSWORD:
#if defined(unix)
			write_to_buffer( d, "\n\r", 2 );
#endif

			if ( str_cmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ))
			{
				fBak = load_char_obj(&backup_d, ch->name, FALSE, FALSE, FALSE);
				backup = backup_d.character;
				if ( str_cmp( crypt( argument, backup->pcdata->pwd ),
						backup->pcdata->pwd ))
				{
					write_to_buffer( d, "Wrong password.\n\r", 0 );
					log_string( LOG_CONNECT, Format("\tY[WIZNET]\tn Denying access to %s@%s (bad password).", ch->name, d->host) );
					wiznet(log_buf,NULL,NULL,WIZ_LOGINS,0,get_trust(ch));

					if (d->character->pet) {
						CHAR_DATA *pet=d->character->pet;

						char_to_room(pet,get_room_index( ROOM_VNUM_LIMBO));
						stop_follower(pet);
						extract_char(pet,TRUE);
					}

					close_socket( d );
					return;
				}
				else
				{
					CHAR_DATA *tmp = ch;

					if (backup_d.character->pet) {
						CHAR_DATA *pet=backup_d.character->pet;

						char_to_room(pet,get_room_index( ROOM_VNUM_LIMBO));
						stop_follower(pet);
						extract_char(pet,TRUE);
					}
					ch = backup;
					d->character = backup;
					backup_d.character = NULL;
					/* Remove loaded backup from memory */
					free_char(tmp);
					fRestore = TRUE;
				}

				if(fBak && backup != NULL)
				{
					free_char(backup);
				}
			}

			if ( IS_SET(ch->act, ACT_REINCARNATE) )
			{
				write_to_buffer( d, "What name do you wish to reincarnate as? ", 0 );
				d->connected = CON_REINCARNATE;
				return;
			}

			ProtocolNoEcho( d, false );

			if (check_playing(d,ch->name))
				return;

			if ( check_reconnect( d, ch->name, TRUE ) )
				return;

			log_string( LOG_CONNECT, Format("[WIZNET] %s@%s has connected.", ch->name, d->host) );
			wiznet(log_buf,NULL,NULL,WIZ_SITES,0,get_trust(ch));

			if ( IS_ADMIN(ch) )
			{
				do_function(ch, &do_help, "imotd" );
				d->connected = CON_READ_IMOTD;
			}
			else
			{
				do_function(ch, &do_help, "motd" );
				d->connected = CON_READ_MOTD;
			}

			/* PFile backup */
			if(!fBak) {
				snprintf(arg, sizeof(arg), "cp %s%s %s%s",
						PLAYER_DIR, d->character->name,
						PLAYER_BACKUP_DIR, d->character->name);
				system(arg);
			}
			else if(fBak && fRestore) {
				snprintf(arg, sizeof(arg), "cp %s%s %s%s", PLAYER_BACKUP_DIR, d->character->name, PLAYER_DIR, d->character->name);
				system(arg);
			}
			fBak = 0;
			fRestore = 0;
			break;

			/* RT code for breaking link */

		case CON_BREAK_CONNECT:
			switch( *argument )
			{
			case 'y' : case 'Y':
				for ( d_old = descriptor_list; d_old != NULL; d_old = d_next )
				{
					d_next = d_old->next;
					if (d_old == d || d_old->character == NULL)
						continue;

					if (str_cmp(ch->name,d_old->original ?
							d_old->original->name : d_old->character->name))
						continue;

					close_socket(d_old);
				}
				if (check_reconnect(d,ch->name,TRUE))
					return;
				write_to_buffer(d,"Reconnect attempt failed.\n\rName: ",0);
				if ( d->character != NULL )
				{
					free_char( d->character );
					d->character = NULL;
				}
				d->connected = CON_GET_NAME;
				break;

			case 'n' : case 'N':
				write_to_buffer(d,"Name: ",0);
				if ( d->character != NULL )
				{
					free_char( d->character );
					d->character = NULL;
				}
				d->connected = CON_GET_NAME;
				break;

			default:
				write_to_buffer(d,"Please type Y or N? ",0);
				break;
			}
			break;

			case CON_CONFIRM_NEW_NAME:
				switch ( *argument )
				{
				case 'y': case 'Y':
					if(IS_SET(ch->act, ACT_REINCARNATE))
					{
						write_to_buffer(d, Format("I'll need you to give me a password for this incarnation: %s", echo_off_str), 0);
					}
					else
					{
						
						ProtocolNoEcho( d, true );
						write_to_buffer( d, Format("New character.\n\rGive me a password for %s:", ch->name), 0 );
					}

					d->connected = CON_GET_NEW_PASSWORD;
					break;

				case 'n': case 'N':
					if(IS_SET(ch->act, ACT_REINCARNATE))
						write_to_buffer( d, "What name will you reincarnate to? ", 0);
					else
						write_to_buffer( d, "Ok, what IS it, then? ", 0 );
					free_char( d->character );
					d->character = NULL;
					d->connected = CON_GET_NAME;
					break;

				default:
					write_to_buffer( d, "Please type Yes or No? ", 0 );
					break;
				}
				break;

				case CON_GET_NEW_PASSWORD:
					get_new_password(d, argument);
					break;

				case CON_CONFIRM_NEW_PASSWORD:
					confirm_new_password(d, argument);
					break;

				case CON_GET_NEW_RACE:
					one_argument(argument,arg);

					if (!str_cmp(arg,"help"))
					{
						argument = one_argument(argument,arg);
						if (IS_NULLSTR(argument))
							do_function(ch, &do_help, "race" );
						else
							do_function(ch, &do_help, argument );
						write_to_buffer(d, "\tWWhat is your race (\tYhelp for more information\tW)?\tn\n\r",0);
						break;
					}

					race = race_lookup(argument);

					if (race == 0 || !race_table[race].pc_race)
					{
						write_to_buffer(d,"\tRThat is not a valid race.\tn\n\r",0);
						write_to_buffer(d,"\tWThe following races are available:\tn\n\r",0);
						for ( race = 1; pc_race_table[race].name != NULL; race++ )
						{
							if (!race_table[race].pc_race)
								break;
							write_to_buffer(d,pc_race_table[race].name,0);
							write_to_buffer(d,"\n\r",1);
						}
						write_to_buffer(d, "\n\r",0);
						write_to_buffer(d, "\tWWhat is your race (\tYhelp for more information\tW)?\tn\n\r",0);
						break;
					}

					ch->race = race;

					/* initialize stats */
					for (i = 0; i < MAX_STATS; i++)
						ch->perm_stat[i] = 1;
					ch->affected_by = ch->affected_by|race_table[race].aff;
					ch->imm_flags	= ch->imm_flags|race_table[race].imm;
					ch->res_flags	= ch->res_flags|race_table[race].res;
					ch->vuln_flags	= ch->vuln_flags|race_table[race].vuln;
					ch->form	= race_table[race].form;
					ch->parts	= race_table[race].parts;
					ch->size = pc_race_table[race].size;

					ch->auspice = number_range(0, 4);
					ch->breed = number_range(0, 2);

					if(ch->exp <= 20)
						ch->exp = 20;

					if(ch->race == race_lookup("werewolf"))
					{
						write_to_buffer( d, "\tWGaia has decided your auspice and breed.\tn\n\r", 0 );
						write_to_buffer( d, Format("\tGAuspice: %10s\n\rBreed: %10s\tn\n\r", auspice_table[ch->auspice].name, breed_table[ch->breed].name), 0 );
					}

					write_to_buffer( d, "What is your sex (M/F)?\n\r", 0 );
					d->connected = CON_GET_NEW_SEX;
					break;


				case CON_GET_NEW_SEX:
					switch ( argument[0] )
					{
						case 'm': case 'M': ch->sex = SEX_MALE;
						ch->pcdata->true_sex = SEX_MALE;
						break;
						case 'f': case 'F': ch->sex = SEX_FEMALE;
						ch->pcdata->true_sex = SEX_FEMALE;
						break;
						default:
							write_to_buffer( d, "\tRThat's not a sex.\n\rWhat IS your sex?\tn\n\r", 0 );
							return;
					}

					if(ch->race == race_lookup("werewolf")) {
						write_to_buffer( d, Format("\tGAuspice: %10s\n\rBreed: %10s\tn\n\r", auspice_table[ch->auspice].name, breed_table[ch->breed].name), 0 );
					}
					strncpy( buf, "\tWSelect a clan\tn\n\r[", sizeof(buf));
					for ( iClass = 1; iClass < MAX_CLAN; iClass++ )
					{
						if (IS_CLASS_AVAILABLE(ch->race,iClass)) {
							strncat( buf, capitalize(clan_table[iClass].name), sizeof(buf) );
							strncat( buf, ", ", sizeof(buf) );
						}
					}
					strncat( buf, "]: \n\r\n\r", sizeof(buf) );
					write_to_buffer( d, buf, 0 );
					d->connected = CON_GET_NEW_CLAN;
					break;

					case CON_GET_NEW_CLAN:
						iClass = clan_lookup(argument);

						if ( iClass == -1 )
						{
							write_to_buffer( d, "\tRThat's not a clan.\tn\n\r", 0 );
							strncpy( buf, "\tWAvailable clans are:\tn\n\r[", sizeof(buf));
							for ( iClass = 1; iClass < MAX_CLAN; iClass++ )
							{
								if (IS_CLASS_AVAILABLE(ch->race,iClass)) {
									strncat( buf, capitalize(clan_table[iClass].name), sizeof(buf) );
									strncat( buf, ", ", sizeof(buf) );
								}
							}
							strncat( buf, "]: \n\r\n\r\tWPlease select a clan.\tn", sizeof(buf) );
							write_to_buffer( d, buf, 0 );
							return;
						}

						ch->clan = iClass;
						init_clan_powers(ch, iClass);
						ch->gen_data = new_gen_data();
						ch->version = CURRENT_PFILE_VERSION;

						/* This is where Nosferatu get's ugly. */
						if (clan_lookup("nosferatu") == ch->clan)
							ch->perm_stat[STAT_APP] = 0;

						/* ----------------------------------------------------- */
						/* New code for Mortals to get a bump or specialization. */
						/* FBI get's 3 police influence to start. */
						if (clan_lookup("FBI") == ch->clan)
							ch->influences[INFL_POLICE] = 3;
						/* Society of Leopold get 3 church influence to start */
						if (clan_lookup("soc. of Leopold") == ch->clan)
							ch->influences[INFL_CHURCH] = 3;
						/* ----------------------------------------------------- */

						log_string( LOG_GAME, Format("%s@%s new player.", ch->name, d->host) );
						wiznet("\tY[WIZNET]\tn Newbie alert!  $N sighted.",ch,NULL,WIZ_NEWBIE,0,0);
						wiznet(log_buf,NULL,NULL,WIZ_SITES,0,get_trust(ch));

						write_to_buffer( d, "\n\r", 2 );

						/* @@@@@ Convert this portion for concept char gen. */
						if(ch->race == race_lookup("werewolf"))
						{
							write_to_buffer( d, Format("\tGAuspice: %10s\n\rBreed: %10s\tn\n\r", auspice_table[ch->auspice].name, breed_table[ch->breed].name), 0 );
						}

						write_to_buffer( d,	"List these atrribute sets in order of importance.\n\r", 0 );
						write_to_buffer( d,	"(P)hysical, (S)ocial or (M)ental (help for more info)\n\r", 0);
						write_to_buffer( d, "(Example psm, smp, PSM, etc.)\n\r", 0);
						d->connected = CON_CHOOSE_ATTRIB;
						break;

					case CON_CHOOSE_ATTRIB:
						one_argument(argument,arg);

						if (!str_cmp(arg,"help"))
						{
							argument = one_argument(argument,arg);
							if (IS_NULLSTR(argument))
								do_function(ch, &do_help, "attribute" );
							else
								do_function(ch, &do_help, argument );
							write_to_buffer(d,
									"What order of importance do you want for your attributes? ",0);
							break;
						}

						if (!str_cmp(arg,"psm") || !str_cmp(arg,"PSM"))
						{
							ch->gen_data->stat_dots[0] = 7;
							ch->gen_data->stat_dots[1] = 5;
							ch->gen_data->stat_dots[2] = 3;
						}
						else if (!str_cmp(arg,"pms") || !str_cmp(arg,"PMS"))
						{
							ch->gen_data->stat_dots[0] = 7;
							ch->gen_data->stat_dots[1] = 3;
							ch->gen_data->stat_dots[2] = 5;
						}
						else if (!str_cmp(arg,"smp") || !str_cmp(arg,"SMP"))
						{
							ch->gen_data->stat_dots[0] = 3;
							ch->gen_data->stat_dots[1] = 7;
							ch->gen_data->stat_dots[2] = 5;
						}
						else if (!str_cmp(arg,"spm") || !str_cmp(arg,"SPM"))
						{
							ch->gen_data->stat_dots[0] = 5;
							ch->gen_data->stat_dots[1] = 7;
							ch->gen_data->stat_dots[2] = 3;
						}
						else if (!str_cmp(arg,"msp") || !str_cmp(arg,"MSP"))
						{
							ch->gen_data->stat_dots[0] = 3;
							ch->gen_data->stat_dots[1] = 5;
							ch->gen_data->stat_dots[2] = 7;
						}
						else if (!str_cmp(arg,"mps") || !str_cmp(arg,"MPS"))
						{
							ch->gen_data->stat_dots[0] = 5;
							ch->gen_data->stat_dots[1] = 3;
							ch->gen_data->stat_dots[2] = 7;
						}
						else
						{
							write_to_buffer( d, "Invalid choice. (Please use lower case.)\n\r", 0);
							write_to_buffer( d, "What order of importance do you want for your attributes?",0);
							write_to_buffer( d, "(Example psm, smp, PSM, etc.)\n\r", 0);
							return;
						}
						write_to_buffer( d, "\n\r", 2 );
						if(ch->race == race_lookup("werewolf")) {
							write_to_buffer( d, Format("Auspice: %10s Breed: %10s\n\r", auspice_table[ch->auspice].name, breed_table[ch->breed].name), 0 );
						}
						write_to_buffer( d, "List these skill sets in order of importance.\n\r", 0 );
						write_to_buffer( d, "(T)alents, (S)kills or (K)nowledges (help for more info)\n\r", 0);
						write_to_buffer( d, "(Example tsk, skt, kst, etc.)\n\r", 0);
						d->connected = CON_CHOOSE_ABIL;
						break;

					case CON_CHOOSE_ABIL:
						one_argument(argument,arg);

						if (!str_cmp(arg,"help"))
						{
							argument = one_argument(argument,arg);
							if (IS_NULLSTR(argument))
								do_function(ch, &do_help, "abilities" );
							else
								do_function(ch, &do_help, argument );
							write_to_buffer(d,
									"What order of importance do you want for your abilities? ",0);
							break;
						}

						if (!str_cmp(arg,"tsk") || !str_cmp(arg,"TSK"))
						{
							ch->gen_data->skill_dots[0] = 13;
							ch->gen_data->skill_dots[1] = 9;
							ch->gen_data->skill_dots[2] = 5;
						}
						else if (!str_cmp(arg,"tks") || !str_cmp(arg,"TKS"))
						{
							ch->gen_data->skill_dots[0] = 13;
							ch->gen_data->skill_dots[1] = 5;
							ch->gen_data->skill_dots[2] = 9;
						}
						else if (!str_cmp(arg,"skt") || !str_cmp(arg,"SKT"))
						{
							ch->gen_data->skill_dots[0] = 5;
							ch->gen_data->skill_dots[1] = 13;
							ch->gen_data->skill_dots[2] = 9;
						}
						else if (!str_cmp(arg,"stk") || !str_cmp(arg,"STK"))
						{
							ch->gen_data->skill_dots[0] = 9;
							ch->gen_data->skill_dots[1] = 13;
							ch->gen_data->skill_dots[2] = 5;
						}
						else if (!str_cmp(arg,"kst") || !str_cmp(arg,"KST"))
						{
							ch->gen_data->skill_dots[0] = 5;
							ch->gen_data->skill_dots[1] = 9;
							ch->gen_data->skill_dots[2] = 13;
						}
						else if (!str_cmp(arg,"kts") || !str_cmp(arg,"KTS"))
						{
							ch->gen_data->skill_dots[0] = 9;
							ch->gen_data->skill_dots[1] = 5;
							ch->gen_data->skill_dots[2] = 13;
						}
						else {
							write_to_buffer(d,
									"Invalid Choice. (Please use lower case.)\n\rWhat order of importance do you want for your abilities? ",0);
							write_to_buffer( d, "(Example tsk, skt, kst, etc.)\n\r", 0);
							return;
						}
						write_to_buffer( d, "\n\r", 2 );
						if(ch->race == race_lookup("werewolf")) {
							write_to_buffer( d, Format("Auspice: %10s Breed: %10s\n\r", auspice_table[ch->auspice].name, breed_table[ch->breed].name), 0 );
						}
						write_to_buffer( d,
								"Time to assign some dots.\n\r", 0 );
						for(i = 0; i < 3; i++)
						{
							send_to_char(Format("%s : %d ",stat_table[i].name, ch->perm_stat[i]),ch);
						}
						send_to_char( "\n\rChoices are: list,premise,add,minus,help and done.\n\r" ,ch);
						send_to_char(Format("You have %d dots to spend.\n\r", ch->gen_data->stat_dots[0]),ch);
						d->connected = CON_ASSIGN_PHYS;
						break;

					case CON_ASSIGN_PHYS:
						send_to_char("\n\r",ch);
						if (!str_cmp(argument,"done"))
						{
							/*	    free_gen_data(ch->gen_data);
	    ch->gen_data = NULL; */
							if(ch->gen_data->stat_dots[0] != 0)
							{
								write_to_buffer(d, "But you still have dots left to spend!", 0);
								break;
							}
							else
							{
								if(ch->race == race_lookup("werewolf")) {
									write_to_buffer( d, Format("Auspice: %10s Breed: %10s\n\r", auspice_table[ch->auspice].name, breed_table[ch->breed].name), 0 );
								}
								write_to_buffer( d,
										"Time to assign some dots.\n\r", 0 );
								for(i = 3; i < 6; i++)
								{
									send_to_char(Format("%s : %d ",stat_table[i].name, ch->perm_stat[i]),ch);
								}
								send_to_char("\n\rChoices are: list,premise,add,minus,help and done.\n\r", ch);
								write_to_buffer( d, "\n\r", 2 );
								send_to_char(Format("You have %d dots to spend.\n\r", ch->gen_data->stat_dots[1]), ch);
								d->connected = CON_ASSIGN_SOC;
								break;
							}
						}

						if (!parse_gen_physical(ch,argument))
						{
							if(ch->race == race_lookup("werewolf")) {
								write_to_buffer( d, Format("Auspice: %10s Breed: %10s\n\r", auspice_table[ch->auspice].name, breed_table[ch->breed].name), 0 );
							}
							for(i = 0; i < 3; i++)
							{
								send_to_char(Format("%s : %d ",stat_table[i].name, ch->perm_stat[i]),ch);
							}
							send_to_char("\n\rChoices are: list,premise,add,minus,help and done.\n\r", ch);
						}

						if(ch->gen_data->stat_dots[0] != 0)
						{
							send_to_char(Format("You have %d dots remaining.\n\r", ch->gen_data->stat_dots[0]), ch);
						}
						else
							send_to_char(Format("You have no dots left. (Type done to continue.)\n\r"), ch);
						send_to_char("\n\rChoices are: list,premise,add,minus,help and done.\n\r", ch);
						break;

					case CON_ASSIGN_SOC:
						send_to_char("\n\r",ch);
						if (!str_cmp(argument,"done"))
						{
							/*	    free_gen_data(ch->gen_data);
	    ch->gen_data = NULL; */
							if(ch->gen_data->stat_dots[1] != 0)
							{
								write_to_buffer(d, "But you still have dots left to spend!", 0);
								break;
							}
							else
							{
								if(ch->race == race_lookup("werewolf")) {
									write_to_buffer( d, Format("Auspice: %10s Breed: %10s\n\r", auspice_table[ch->auspice].name, breed_table[ch->breed].name), 0 );
								}
								write_to_buffer( d,
										"Time to assign some dots.\n\r", 0 );
								for(i = 6; i < 9; i++)
								{
									send_to_char(Format("%s : %d ",stat_table[i].name, ch->perm_stat[i]),ch);
								}
								send_to_char("\n\rChoices are: list,premise,add,minus,help and done.\n\r", ch);
								write_to_buffer( d, "\n\r", 2 );
								send_to_char(Format("You have %d dots to spend.\n\r", ch->gen_data->stat_dots[2]), ch);
								d->connected = CON_ASSIGN_MENT;
								/*	    desc_gen( ch );
								 */            break;
							}
						}

						if (!parse_gen_social(ch,argument))
						{
							if(ch->race == race_lookup("werewolf"))
							{
								write_to_buffer( d, Format("Auspice: %10s Breed: %10s\n\r", auspice_table[ch->auspice].name, breed_table[ch->breed].name), 0 );
							}
							for(i = 3; i < 6; i++)
							{
								send_to_char(Format("%s : %d ",stat_table[i].name, ch->perm_stat[i]),ch);
							}
							send_to_char("\n\rChoices are: list,premise,add,minus,help and done.\n\r", ch);
						}

						if(ch->gen_data->stat_dots[1] != 0)
						{
							send_to_char(Format("You have %d dots remaining.\n\r", ch->gen_data->stat_dots[1]), ch);
						}
						else
						{
							send_to_char(Format("You have no dots left. (Type done to continue.)\n\r"), ch);
						}
						send_to_char("\n\rChoices are: list,premise,add,minus,help and done.\n\r", ch);
						break;

					case CON_ASSIGN_MENT:
						send_to_char("\n\r",ch);
						if (!str_cmp(argument,"done"))
						{
							/*	    free_gen_data(ch->gen_data);
	    ch->gen_data = NULL; */
							if(ch->gen_data->stat_dots[2] != 0)
							{
								write_to_buffer(d, "But you still have dots left to spend!", 0);
								break;
							}
							else
							{
								if(ch->race == race_lookup("werewolf")) {
									write_to_buffer( d, Format("Auspice: %10s Breed: %10s\n\r", auspice_table[ch->auspice].name, breed_table[ch->breed].name), 0 );
								}
								write_to_buffer( d,
										"Ability dots time.\n\r", 0 );
								for (iClass = 0; iClass < 13; iClass++)
									if (IS_ATTRIB_AVAILABLE(ch->race, iClass))
									{
										if((iClass)%5 == 0) send_to_char("\n\r", ch);
										send_to_char(Format("%15s", ability_table[iClass].name), ch);
									}
								write_to_buffer(d, "\n\r", 2);
								send_to_char("\n\rChoices are: list,premise,add,minus,help and done.\n\r", ch);
								write_to_buffer( d, "\n\r", 2 );
								for (iClass = 0; iClass < MAX_ABIL; iClass++)
									ch->ability[iClass].value = 0;
								send_to_char(Format("You have %d dots to spend.\n\r", ch->gen_data->skill_dots[0]), ch);
								d->connected = CON_ASSIGN_TAL;
								break;
							}
						}

						if (!parse_gen_mental(ch,argument))
						{
							if(ch->race == race_lookup("werewolf")) {
								write_to_buffer( d, Format("Auspice: %10s Breed: %10s\n\r", auspice_table[ch->auspice].name, breed_table[ch->breed].name), 0 );
							}
							for(i = 6; i < 9; i++)
							{
								send_to_char(Format("%s : %d ",stat_table[i].name, ch->perm_stat[i]),ch);
							}
							send_to_char("\n\rChoices are: list,premise,add,minus,help and done.\n\r", ch);
						}

						if(ch->gen_data->stat_dots[2] != 0)
						{
							send_to_char(Format("You have %d dots remaining.\n\r", ch->gen_data->stat_dots[2]), ch);
						}
						else
						{
							send_to_char(Format("You have no dots left. (Type done to continue.)\n\r"), ch);
						}

						for(i = 0; i < 9; i++)
						{
							ch->mod_stat[i] = ch->perm_stat[i];
						}

						send_to_char("\n\rChoices are: list,premise,add,minus,help and done.\n\r", ch);
						break;

					case CON_ASSIGN_TAL:
						send_to_char("\n\r",ch);
						if (!str_cmp(argument,"done"))
						{
							/*
							 * 	    free_gen_data(ch->gen_data);
							 * 	    ch->gen_data = NULL;
							 */

							if(ch->gen_data->skill_dots[0] != 0)
							{
								write_to_buffer(d, "But you still have dots left to spend!", 0);
								break;
							}
							else
							{
								write_to_buffer( d, "More dots to go.\n\r", 0 );
								for (iClass = 13; iClass < 24; iClass++)
									if (IS_ATTRIB_AVAILABLE(ch->race, iClass))
									{
										if((iClass-13)%5 == 0) send_to_char("\n\r", ch);
										send_to_char(Format("%15s", ability_table[iClass].name), ch);
									}
								send_to_char("\n\r", ch);
								send_to_char("\n\rChoices are: list,premise,add,minus,help and done.\n\r", ch);
								write_to_buffer( d, "\n\r", 2 );
								send_to_char(Format("You have %d dots to spend.\n\r", ch->gen_data->skill_dots[1]), ch);
								d->connected = CON_ASSIGN_SKILL;
								break;
							}
						}

						if (!parse_gen_talents(ch,argument))
						{
							for (iClass = 0; iClass < 13; iClass++)
								if (IS_ATTRIB_AVAILABLE(ch->race, iClass))
								{
									if((iClass)%5 == 0) send_to_char("\n\r", ch);
									send_to_char(Format("%15s", ability_table[iClass].name), ch);
								}
							write_to_buffer(d, "\n\r", 2);
							send_to_char("\n\rChoices are: list,premise,add,minus,help and done.\n\r", ch);
						}

						if(ch->gen_data->skill_dots[0] != 0)
						{
							send_to_char(Format("You have %d dots remaining.\n\r", ch->gen_data->skill_dots[0]), ch);
						}
						else
						{
							send_to_char(Format("You have no dots left. (Type done to continue.)\n\r"), ch);
						}

						send_to_char("\n\rChoices are: list,premise,add,minus,help and done.\n\r", ch);
						break;

					case CON_ASSIGN_SKILL:
						send_to_char("\n\r",ch);
						if (!str_cmp(argument,"done"))
						{
							/*	    free_gen_data(ch->gen_data);
	    ch->gen_data = NULL; */
							if(ch->gen_data->skill_dots[1] != 0)
							{
								write_to_buffer(d, "But you still have dots left to spend!\n\r", 0);
								break;
							}
							else
							{
								write_to_buffer( d,"More dots to go.\n\r", 0 );
								for (iClass = 24; ability_table[iClass].name != NULL; iClass++)
									if ( IS_ATTRIB_AVAILABLE(ch->race, iClass) )
									{
										if((iClass-24)%5 == 0) send_to_char("\n\r", ch);
										send_to_char(Format("%15s", ability_table[iClass].name), ch);
									}
								write_to_buffer(d, "\n\r", 2);
								send_to_char("\n\rChoices are: list,premise,add,minus,help and done.\n\r", ch);
								write_to_buffer( d, "\n\r", 2 );
								send_to_char(Format("You have %d dots to spend.\n\r", ch->gen_data->skill_dots[2]), ch);
								d->connected = CON_ASSIGN_KNOWLEDGES;
								break;
							}
						}

						if (!parse_gen_skills(ch,argument))
						{
							for (iClass = 13; iClass < 24; iClass++)
								if (IS_ATTRIB_AVAILABLE(ch->race, iClass))
								{
									if((iClass-13)%5 == 0) send_to_char("\n\r", ch);
									send_to_char(Format("%15s", ability_table[iClass].name), ch);
								}
							write_to_buffer(d, "\n\r", 2);
							send_to_char("\n\rChoices are: list,premise,add,minus,help and done.\n\r", ch);
						}

						if(ch->gen_data->skill_dots[1] != 0)
						{
							send_to_char(Format("You have %d dots remaining.\n\r", ch->gen_data->skill_dots[1]), ch);
						}
						else
						{
							send_to_char(Format("You have no dots left. (Type done to continue.)\n\r"), ch);
						}

						send_to_char("\n\rChoices are: list,premise,add,minus,help and done.\n\r", ch);
						break;

					case CON_ASSIGN_KNOWLEDGES:
						send_to_char("\n\r",ch);
						if (!str_cmp(argument,"done"))
						{
							/*
							 * 	    free_gen_data(ch->gen_data);
							 * 	    ch->gen_data = NULL;
							 */

							if(ch->gen_data->skill_dots[2] != 0)
							{
								write_to_buffer(d, "But you still have dots left to spend!\n\r", 0);
								break;
							}
							else
							{
								write_to_buffer( d, "More dots to go.\n\r", 0 );
								write_to_buffer(d, "Conscience Self-Control Courage\n\r", 0);
								write_to_buffer(d, "\n\r", 2);
								send_to_char("\n\rChoices are: list,premise,add,minus,help and done.\n\r", ch);
								write_to_buffer( d, "\n\r", 2 );
								ch->gen_data->virtue_dots = 7;
								ch->virtues[0] = 1; ch->virtues[1] = 1; ch->virtues[2] = 1;
								send_to_char(Format("You have %d dots to spend.\n\r", ch->gen_data->virtue_dots), ch);
								d->connected = CON_ASSIGN_VIRTUES;
								return;
							}
						}

						if (!parse_gen_knowledges(ch,argument))
						{
							for (iClass = 24; ability_table[iClass].name != NULL; iClass++)
								if (IS_ATTRIB_AVAILABLE(ch->race, iClass))
								{
									if((iClass-24)%5 == 0) send_to_char("\n\r", ch);
									send_to_char(Format("%15s", ability_table[iClass].name), ch);
								}
							write_to_buffer(d, "\n\r", 2);
							send_to_char("\n\rChoices are: list,premise,add,minus,help and done.\n\r", ch);
						}

						if(ch->gen_data->skill_dots[2] != 0)
						{
							send_to_char(Format("You have %d dots remaining.\n\r", ch->gen_data->skill_dots[2]), ch);
						}
						else
						{
							send_to_char(Format("You have no dots left. (Type done to continue.)\n\r"), ch);
						}

						send_to_char("\n\rChoices are: list,premise,add,minus,help and done.\n\r", ch);
						break;

					case CON_ASSIGN_VIRTUES:
						send_to_char("\n\r",ch);
						if (!str_cmp(argument,"done"))
						{
							/*	    free_gen_data(ch->gen_data);
	    							ch->gen_data = NULL;
	    					*/

							if(ch->gen_data->virtue_dots != 0)
							{
								write_to_buffer(d, "But you still have dots left to spend!\n\r", 0);
								break;
							}
							else
							{
								do_function(ch, &do_help, "motd" );
								write_to_buffer(d, "\n\r",2);
								d->connected = CON_READ_MOTD;
								do_function(ch, &do_outfit, "" );
								return;
							}
						}

						if (!parse_gen_virtues(ch,argument))
						{
							write_to_buffer(d, "Conscience Self-Control Courage\n\r", 0);
							write_to_buffer(d, "\n\r", 2);
							send_to_char("\n\rChoices are: list,premise,add,minus,help and done.\n\r", ch);
						}

						if(ch->gen_data->virtue_dots != 0)
						{
							send_to_char(Format("You have %d dots remaining.\n\r", ch->gen_data->virtue_dots), ch);
							send_to_char("\n\rChoices are: list,premise,add,minus,help and done.\n\r", ch);
						}
						else
						{
							send_to_char(Format("You have no dots left. (Type done to continue.)\n\r"), ch);
						}

						break;


					case CON_READ_IMOTD:
						do_function(ch, &do_help, "motd" );
						write_to_buffer(d, "\n\r",2);
						d->connected = CON_READ_MOTD;
						break;

					case CON_READ_MOTD:
						if ( ch->pcdata == NULL || ch->pcdata->pwd[0] == '\0')
						{
							write_to_buffer( d, "\tRWarning! Null password!\tn\n\r",0 );
							write_to_buffer( d,	"\tRType 'password null [new password]' to fix.\tn\n\r",0);
						}

						LINK_SINGLE(ch, next, char_list);
						d->connected	= CON_PLAYING;
						reset_char(ch);

						if(ch->ooc_xp_time > 60) ch->ooc_xp_time = 10;

						if(IS_SET(ch->act, ACT_REINCARNATE))
						{
							ch->position = P_STAND;
							ch->health = 7;
							ch->agghealth = 7;
							ch->home = -1;
							ch->condition[COND_HUNGER] = 48;
							ch->condition[COND_FULL] = 48;
							ch->condition[COND_THIRST] = 48;
							REMOVE_BIT(ch->act, ACT_REINCARNATE);
						}

						if(ch->version == TESTING_PFILE_VERSION)
						{
							/* Put anything in here that needs to be configured for testing */

							/* Testing stuff for new power infrastructure */
							if(ch->race == race_lookup("werewolf"))
							{
								/* Give 'em the new gift data */
								/* @@@@@ */
							}
						}

						if(ch->version < CURRENT_PFILE_VERSION)
						{
							ch->version = CURRENT_PFILE_VERSION;

							/* Rayal PFILE Updates */
							ch->exp +=10;

						}

						if(ch->race == race_lookup("werewolf"))
						{
							if(ch->backgrounds[RACE_STATUS] < 1)
								ch->backgrounds[RACE_STATUS] = 1;

							if(ch->disc[0] > ch->backgrounds[RACE_STATUS])
							{
								ch->exp+=compound(ch->disc[0]-ch->backgrounds[RACE_STATUS])*8;
								ch->disc[0] = ch->backgrounds[RACE_STATUS];
							}
							if(ch->disc[1] > ch->backgrounds[RACE_STATUS])
							{
								ch->exp+=compound(ch->disc[1]-ch->backgrounds[RACE_STATUS])*8;
								ch->disc[1] = ch->backgrounds[RACE_STATUS];
							}
							if(ch->disc[2] > ch->backgrounds[RACE_STATUS])
							{
								ch->exp+=compound(ch->disc[2]-ch->backgrounds[RACE_STATUS])*8;
								ch->disc[2] = ch->backgrounds[RACE_STATUS];
							}

							for(i = 0; i < ch->disc[0]; i++)
							{
								SET_BIT(ch->powers[i],
										group_gift_lookup(clan_table[ch->clan].name, i));
							}

							for(i = 0; i < ch->disc[1]; i++)
							{
								SET_BIT(ch->powers[i],
										group_gift_lookup(auspice_table[ch->auspice].name, i));
							}

							for(i = 0; i < ch->disc[2]; i++)
							{
								SET_BIT(ch->powers[i],
										group_gift_lookup(breed_table[ch->breed].name, i));
							}
						}

						if ( ( ch->trust == 0 ) || ( ch->trust > 1 ) )
						{
							ch->health	= MAX_HEALTH;
							ch->agghealth = MAX_HEALTH;
							ch->ooc_xp_time = 0;
							if(ch->trust == 0)
							{

								ch->dollars = 200;
								if(ch->ability[FINANCE].value > 0)
								{
									ch->dollars += 100 * ch->ability[FINANCE].value;
								}

								ch->in_room = get_room_index(ROOM_VNUM_START);


								if(ch->race == race_lookup("vampire"))
								{
									ch->max_GHB = ch->virtues[0] + ch->virtues[1];
									ch->max_RBPG = 23 - ch->gen;
									if(ch->max_RBPG < 10) ch->max_RBPG = 10;
									ch->max_willpower = ch->virtues[2];
									if(ch->clan != clan_lookup("None"))
									{
										ch->clan_powers[0] = clan_table[ch->clan].powers[0];
										ch->clan_powers[1] = clan_table[ch->clan].powers[1];
										ch->clan_powers[2] = clan_table[ch->clan].powers[2];
									}
									else
									{
										ch->clan_powers[0] = -1;
										ch->clan_powers[1] = -1;
										ch->clan_powers[2] = -1;
									}
								}
								else if(ch->race == race_lookup("werewolf"))
								{
									ch->max_GHB = breed_table[ch->breed].gnosis;
									ch->max_RBPG = auspice_table[ch->auspice].rage;
									ch->max_willpower = 2;
								}
								else if(ch->race == race_lookup("human"))
								{
									ch->max_GHB = 8;
									ch->max_RBPG = 0;
									ch->max_willpower = ch->virtues[2];
								}

								ch->warrants = 0;
							}

							if(ch->trust == 0)
								ch->trust = 1;
							ch->GHB = ch->max_GHB;
							ch->RBPG = ch->max_RBPG;
							ch->willpower = ch->max_willpower;

							if(!IS_ADMIN(ch))
							{
								set_title( ch, (char *)Format("is new to the city.") );
							}

							if(ch->in_room == NULL)
								char_to_room( ch, get_room_index( ROOM_VNUM_START ) );
							else
								char_to_room( ch, ch->in_room );
							send_to_char("\n\r",ch);

							if(!IS_ADMIN(ch))
								do_function(ch, &do_wear, "all" );
						}
						else if ( ch->in_room != NULL )
						{
							char_to_room( ch, ch->in_room );
						}

						write_to_buffer( d,	"\n\rWelcome to the city... life's about to get interesting.\n\r", 0);
						act( "$n stumbles a little getting their bearings.", ch, NULL, NULL, TO_ROOM, 0 );
						MXPSendTag( d, "<VERSION>" );  /* <--- Add this line */

						if(!IS_ADMIN(ch))
						{
							do_function(NULL, &do_echo, (char *)Format("[\tCConnection Alert\tn] %s has joined us.", ch->name) );
							log_to_file("44000", "OOCXML", Format("[Connection Alert] %s has joined in.", ch->name));
						}
						wiznet("\tY[WIZNET]\tn $N joins the party.", ch, NULL, WIZ_LOGINS, WIZ_SITES, get_trust(ch));

						if (ch->pet != NULL)
						{
							char_to_room(ch->pet,ch->in_room);
							act("$n follows $N along.",ch->pet,NULL,ch,TO_ROOM,0);
						}

						if(ch->health + ch->agghealth - 7 < 0)
							do_function(ch, &do_restore, ch->name );

						do_function(ch, &do_updatetime, "" );
						send_to_char("\n\r\n\r", ch);
						do_function(ch, &do_look, "auto" );
						send_to_char("\n\r\n\r", ch);
						do_function(ch, &do_unread, "" );
						do_function(ch, &do_save, "no_messages" );
						break;

					case CON_DECEASED:
						iClass = get_points(ch);
						send_to_char("Death has ended this character's existence.\n\r",ch);
						send_to_char("There is still hope in reincarnation.\n\r",ch);
						send_to_char("Do you wish to reincarnate? (n deletes character.) [y/n]",ch);
						d->connected = CON_ETHEREAL;
						break;

					case CON_ETHEREAL:
						one_argument(argument, arg);
						if (arg[0] == 'y' || arg[0] == 'Y')
						{
							write_to_buffer( d, "What name do you wish to reincarnate as? ", 0 );
							if(ch->trust == 1) ch->trust = 0;
							d->connected = CON_REINCARNATE;
						}
						else if (arg[0] == 'n' || arg[0] == 'N')
						{
							deaded_char(ch);
						}
						else send_to_char("Do you wish to reincarnate? [y/n]",ch);
						break;

	}

	return;
}


/*
 * Parse a name for acceptability.
 */
bool check_parse_name( char *name )
{
	int count = 0, clan = 0;
	DESCRIPTOR_DATA *d;
	DESCRIPTOR_DATA *dnext;

	/*
	 * Reserved words.
	 */
	if ( is_exact_name( name,
			"all auto immortal self someone something the you demise balance circle loner honor fuck sux sucks nosferatu malkavian jesus god none quit") )
		return FALSE;

	if ( is_exact_name( name, "accountsystem" ) )
		return TRUE;

	/* check clans */
	for (clan = 0; clan < MAX_CLAN; clan++)
	{
		if (LOWER(name[0]) == LOWER(clan_table[clan].name[0])
				&&  !str_cmp(name,clan_table[clan].name))
			return FALSE;
	}

	/*
	 * Length restrictions.
	 */

	if ( strlen(name) <  2 )
		return FALSE;

#if defined(MSDOS)
	if ( strlen(name) >  8 )
		return FALSE;
#endif

#if defined(macintosh) || defined(unix)
	if ( strlen(name) > 12 )
		return FALSE;
#endif

	/*
	 * Alphanumerics only.
	 * Lock out IllIll twits.
	 */
	{
		char *pc;
		bool fIll,adjcaps = FALSE,cleancaps = FALSE;
		int total_caps = 0;

		fIll = TRUE;
		for ( pc = name; *pc != '\0'; pc++ )
		{
			if ( !isalpha(*pc) )
				return FALSE;

			if ( isupper(*pc)) /* ugly anti-caps hack */
			{
				if (adjcaps)
					cleancaps = TRUE;
				total_caps++;
				adjcaps = TRUE;
			}
			else
				adjcaps = FALSE;

			if ( LOWER(*pc) != 'i' && LOWER(*pc) != 'l' )
				fIll = FALSE;
		}

		if ( fIll )
			return FALSE;

		if (cleancaps || (total_caps > (strlen(name)) / 2 && strlen(name) < 3))
			return FALSE;
	}

	/*
	 * Prevent players from naming themselves after mobs.
    {
	extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
	MOB_INDEX_DATA *pMobIndex;
	int iHash;

	for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	{
	    for ( pMobIndex  = mob_index_hash[iHash];
		  pMobIndex != NULL;
		  pMobIndex  = pMobIndex->next )
	    {
		if ( is_name( name, pMobIndex->player_name ) )
		    return FALSE;
	    }
	}
    }
	 */

	/*
	 * check names of people playing. Yes, this is necessary for multiple
	 * newbies with the same name (thanks Saro)
	 */
	if (descriptor_list) {
		count=0;
		for (d = descriptor_list; d != NULL; d = dnext)
		{
			dnext=d->next;
			if (d->connected!=CON_PLAYING&&d->character&&d->character->name
					&& d->character->name[0] && !str_cmp(d->character->name,name)
					&&!IS_SET(d->character->act, ACT_REINCARNATE))
			{
				count++;
				close_socket(d);
			}
		}
		if (count)
		{
			wiznet((char *)Format("\tY[WIZNET]\tn Double newbie alert (%s)",name),NULL,NULL,WIZ_LOGINS,0,0);

			return FALSE;
		}
	}

	return TRUE;
}



/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect( DESCRIPTOR_DATA *d, char *name, bool fConn )
{
	CHAR_DATA *ch;

	for ( ch = char_list; ch != NULL; ch = ch->next )
	{
		if ( !IS_NPC(ch)
				&&   (!fConn || ch->desc == NULL)
				&&   !str_cmp( d->character->name, ch->name ) )
		{
			if ( fConn == FALSE )
			{
				PURGE_DATA( d->character->pcdata->pwd );
				d->character->pcdata->pwd = str_dup( ch->pcdata->pwd );
			}
			else
			{
				if (d->character->pet) {
					CHAR_DATA *pet=d->character->pet;

					char_to_room(pet,get_room_index( ROOM_VNUM_LIMBO));
					stop_follower(pet);
					extract_char(pet, TRUE);
				}
				free_char( d->character );
				d->character = ch;
				ch->desc	 = d;
				ch->timer	 = 0;
				send_to_char("\tGReconnecting.\tn\n\r", ch );
				act( "\tG$n has reconnected.\tn", ch, NULL, NULL, TO_ROOM, 0 );

				log_string( LOG_CONNECT, Format("%s@%s reconnected.", ch->name, d->host) );
				wiznet("\tY[WIZNET]\tn $N groks the fullness of $S link.",	ch,NULL,WIZ_LINKS,0,0);
				d->connected = CON_PLAYING;
				MXPSendTag( d, "<VERSION>" );  /* <--- Add this line */
			}
			return TRUE;
		}
	}

	return FALSE;
}



/*
 * Check if already playing.
 */
bool check_playing( DESCRIPTOR_DATA *d, char *name )
{
	DESCRIPTOR_DATA *dold;

	for ( dold = descriptor_list; dold; dold = dold->next )
	{
		if ( dold != d
				&&   dold->character != NULL
				&&   dold->connected != CON_GET_NAME
				&&   dold->connected != CON_GET_OLD_PASSWORD
				&&   !str_cmp( name, dold->original
						? dold->original->name : dold->character->name ) )
		{
			write_to_buffer( d, "That character is already playing.\n\r",0);
			write_to_buffer( d, "Do you wish to connect anyway (Y/N)?",0);
			d->connected = CON_BREAK_CONNECT;
			return TRUE;
		}
	}

	return FALSE;
}



void stop_idling( CHAR_DATA *ch )
{
	if ( ch == NULL
			||   ch->desc == NULL
			||   ch->desc->connected != CON_PLAYING
			||   ch->was_in_room == NULL
			||   ch->in_room != get_room_index(ROOM_VNUM_LIMBO))
		return;

	ch->timer = 0;
	if(!IS_SET(ch->affected_by2, AFF2_EARTHMELD)) {
		char_from_room( ch );
		char_to_room( ch, ch->was_in_room );
		ch->was_in_room	= NULL;
		act( "$n has returned from the void.", ch, NULL, NULL, TO_ROOM, 0 );
	}
	return;
}



/*
 * Write to one char, new colour version, by Lope.
 */
/*void send_to_char( const char *txt, CHAR_DATA *ch )
{
	const	char 	*point;
	char 	*point2;
	char 	buf[ MSL*4 ]={'\0'};
	int	skip = 0;

	buf[0] = '\0';
	point2 = buf;

	if( txt && ch->desc )
	{
		if( IS_SET( ch->plr_flags, PLR_COLOUR ) )
		{
			for( point = txt ; *point ; point++ )
			{
				if( *point == '{' )
				{
					point++;
					skip = colour( *point, ch, point2 );
					while( skip-- > 0 )
						++point2;
					continue;
				}
				*point2 = *point;
				*++point2 = '\0';
			}
			*point2 = '\0';
			if( ch->desc != NULL )
				write_to_buffer( ch->desc, buf, point2 - buf );
			else
				trigger_test(buf, ch, NULL);
		}
		else
		{
			for( point = txt ; *point ; point++ )
			{
				if( *point == '{' )
				{
					point++;
					continue;
				}
				*point2 = *point;
				*++point2 = '\0';
			}
			*point2 = '\0';
			if( ch->desc != NULL )
				write_to_buffer( ch->desc, buf, point2 - buf );
			else
				trigger_test(buf, ch, NULL);
		}
	}
	return;
}*/

/*
 * Write to one char.
 */
void send_to_char( const char *txt, CHAR_DATA *ch )
{
    if ( txt != NULL && ch->desc != NULL )
        write_to_buffer( ch->desc, txt, strlen(txt) );
    return;
}

/*
 * Send a string to everyone in a room.
 */
void send_to_room( const char *argument, ROOM_INDEX_DATA *room )
{
    DESCRIPTOR_DATA *d;

    for ( d = descriptor_list; d; d = d->next )
    {
        if ( d->connected == CON_PLAYING
        &&   d->character->in_room == room )
        {
            send_to_char( argument, d->character );
        }
    }

    return;
}

/*
 * Page to one char, new colour version, by Lope.
 */
void page_to_char( const char *txt, CHAR_DATA *ch )
{
	DESCRIPTOR_DATA *d;
	if ( !ch || txt == NULL || ch->desc == NULL || IS_NULLSTR(txt) )
		return;
	d = ch->desc;
	if (ch->lines == 0 )
	{
		send_to_char(txt,ch);
		return;
	}
	/*
	 * If there is already some data being "paged" for this descriptor,
	 * append the new string.
	 */
	if( !IS_NULLSTR( d->showstr_head ) )
	{
		char *fub;
		int i = 0;
		int size_new = strlen( txt ) + strlen( d->showstr_head ) + 2;
		ALLOC_DATA(fub, char, size_new);
		fub[0] = '\0';
		strncat( fub, d->showstr_head, size_new );
		if (IS_NULLSTR(d->showstr_point))
			i = strlen(fub);
		else
			i = strlen( fub ) - strlen( d->showstr_point );
		strncat( fub, txt, size_new );
		PURGE_DATA(d->showstr_head);
		d->showstr_head = str_dup(fub);
		d->showstr_point = (char *)d->showstr_head + i;
		PURGE_DATA(fub);
		fub = NULL;
		return;
	}
	if(!IS_NULLSTR(d->showstr_head))
		PURGE_DATA(d->showstr_head);
	d->showstr_head = str_dup(txt);
	d->showstr_point = (char *)d->showstr_head;
	show_string(ch->desc,"");
}

/* string pager */
void show_string(struct descriptor_data *d, char *input)
{
	char buffer[4*MSL]={'\0'};
	char buf[MSL]={'\0'};
	register char *scan, *chk;
	int lines = 0, toggle = 1;
	int show_lines = 0;

	one_argument(input,buf);
	if (!IS_NULLSTR(buf))
	{
		if (d->showstr_head)
		{
			PURGE_DATA(d->showstr_head);
			d->showstr_head = 0;
		}
		d->showstr_point  = 0;
		return;
	}

	if (d->character)
		show_lines = d->character->lines;
	else
		show_lines = 0;

	for (scan = buffer; ; scan++, d->showstr_point++)
	{
		if (((*scan = *d->showstr_point) == '\n' || *scan == '\r')
				&& (toggle = -toggle) < 0)
			lines++;

		else if (!*scan || (show_lines > 0 && lines >= show_lines))
		{
			*scan = '\0';
			write_to_buffer(d,buffer,strlen(buffer));
			for (chk = d->showstr_point; isspace(*chk); chk++);
			{
				if (!*chk)
				{
					if (d->showstr_head)
					{
						PURGE_DATA(d->showstr_head);
						d->showstr_head = 0;
					}
					d->showstr_point  = 0;
				}
			}
			return;
		}
	}
	return;
}

/* quick sex fixer */
void fix_sex(CHAR_DATA *ch)
{
    if (ch->sex < 0 || ch->sex > 2)
    	ch->sex = IS_NPC(ch) ? 0 : ch->pcdata->true_sex;
}

/*
 * Colour version of act_new() function.
 * Colour by Lope, astral projection "listeners" by 1KMonkeys.
 */
void act_new( const char *format, CHAR_DATA *ch, const void *arg1,  const void *arg2, int type, int min_pos, int all_planes)
{
    static char * const he_she  [] = { "it",  "he",  "she" };
    static char * const him_her [] = { "it",  "him", "her" };
    static char * const his_her [] = { "its", "his", "her" };
 
    char buf[MSL] = {'\0'};
    char fname[MAX_INPUT_LENGTH] = {'\0'};
    CHAR_DATA *to;
    CHAR_DATA *also_to;
    CHAR_DATA *vch = (CHAR_DATA *) arg2;
    OBJ_DATA *obj1 = (OBJ_DATA  *) arg1;
    OBJ_DATA *obj2 = (OBJ_DATA  *) arg2;
    ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA  *) arg2;
    const char *str;
    const char *i = " <@@@> ";
    char *point;
    char 		*pbuff;
    char 		buffer[ MSL*2 ] = {'\0'};
    bool		fColour = FALSE;
 
    /*
     * Discard null and zero-length messages.
     */
    if ( format == NULL || format[0] == '\0' )
    	return;

    /* discard null rooms and chars */
    if (ch == NULL || ch->in_room == NULL)
    	return;

    if(type == TO_OROOM)
    {
    	if(room == NULL)
    	{
    		log_string(LOG_BUG, "Act: null room with TO_OROOM.");
    		return;
    	}

    	to = room->people;
    	also_to = room->listeners;
    }
    else
    {
    	if(IS_SET(ch->act2, ACT2_ASTRAL))
    	{
    		to = ch->listening->people;
    		also_to = ch->listening->listeners;
    	}
    	else
    	{
    		to = ch->in_room->people;
    		also_to = ch->in_room->listeners;
    	}
    }

    if ( type == TO_VICT )
    {
        if ( vch == NULL )
        {
            log_string(LOG_BUG, "Act: null vch with TO_VICT.");
            return;
        }

	if (!SAME_PLANE(ch, vch) && !all_planes)
	    return;

	if (vch->in_room == NULL)
	    return;

        to = vch->in_room->people;
        also_to = vch->in_room->listeners;
    }

    for ( ; to != NULL; to = to->next_in_room )
    {
        if (to->position < min_pos) continue;
        if (!SAME_PLANE(ch, to) && !all_planes && type != TO_OPLANE) continue;
	    if (!IS_NPC(to) && to->desc == NULL ) continue;
	    if ( IS_NPC(to) && !HAS_TRIGGER(to, TRIG_ACT) && !HAS_SCRIPT(to) ) continue;

        if ( (type == TO_CHAR) && to != ch )
            continue;
        if ( type == TO_VICT && ( to != vch || to == ch ) )
            continue;
        if ( type == TO_ROOM && to == ch )
            continue;
        if ( type == TO_NOTVICT && (to == ch || to == vch) )
            continue;
        if ( type == TO_GROUP && !is_same_group(to, ch) && to != ch )
            continue;
        if ( type == TO_OPLANE && to == ch )
            continue;
	if(ch != NULL && to != NULL)
	{
		// -- what if to->ignore is NULL ? Hmm?  Protection, we wrap it up :)
		if(!IS_NULLSTR(to->ignore)) {
			if(strstr(to->ignore, ch->name) != '\0')
				continue;
		}
	}

        point   = buf;
        str     = format;
        while ( *str != '\0' )
        {
            if ( *str != '$' )
            {
                *point++ = *str++;
                continue;
            }
	    fColour = TRUE;
            ++str;
 
            if ( arg2 == NULL && *str >= 'A' && *str <= 'Z' )
            {
                log_string(LOG_BUG, Format("Act: missing arg2 for code %d.", *str ));
                i = " <@@@> ";
            }
            else
            {
                switch ( *str )
                {
                default:  log_string(LOG_BUG, Format("Act: bad code %d.", *str ));
                          i = " <@@@> ";                                break;
                case '$': i = "$";
                            break;
                /* Thx alex for 't' idea */
                case 't': if (arg1) i = (char *) arg1;
			  else log_string(LOG_BUG, "Act: bad code $t for 'arg1'");
                            break;
                case 'T': if (arg2) i = (char *) arg2;
			  else log_string(LOG_BUG, "Act: bad code $T for 'arg2'");
                            break;
                case 'n': if (ch&&to) {
        		if(LOOKS_DIFFERENT(ch))
        		    i = ALT_PERS( ch, to );
        		else
        		    i = PERS( ch, to ); }
			  else log_string(LOG_BUG, "Act: bad code $n for 'ch' or 'to'");
			    break;
                case 'N': if (vch&&to) {
        		if(LOOKS_DIFFERENT(vch))
        		    i = ALT_PERS( vch, to );
        		else
        		    i = PERS( vch, to ); }
			  else log_string(LOG_BUG, "Act: bad code $N for 'vch' or 'to'");
			    break;
                case 'e': if (ch) i = he_she  [URANGE(0, ch  ->sex, 2)];
			  else log_string(LOG_BUG, "Act: bad code $e for 'ch'");
		            break;
                case 'E': if (vch) i = he_she  [URANGE(0, vch ->sex, 2)];
			  else log_string(LOG_BUG, "Act: bad code $E for 'vch'");
		            break;
                case 'm': if (ch) i = him_her [URANGE(0, ch  ->sex, 2)];
			  else log_string(LOG_BUG, "Act: bad code $m for 'ch'");
		            break;
                case 'M': if (vch) i = him_her [URANGE(0, vch ->sex, 2)];
			  else log_string(LOG_BUG, "Act: bad code $M for 'vch'");
		            break;
                case 's': if (ch) i = his_her [URANGE(0, ch  ->sex, 2)];
			  else log_string(LOG_BUG, "Act: bad code $s for 'ch'");
		            break;
                case 'S': if (vch) i = his_her [URANGE(0, vch ->sex, 2)];
			  else log_string(LOG_BUG, "Act: bad code $S for 'vch'");
		            break;

                case 'p':
                    if (obj1) {
			if ( can_see_obj( to, obj1 ) )
                            i = IS_SET(obj1->extra2, OBJ_PACKAGED)
				? "a package" : obj1->short_descr;
                        else
			    i = "something";
		    }
		    else 
		    	log_string(LOG_BUG, "Act: bad code $p for 'obj1'");
                    break;
 
                case 'P':
                    if (obj2) {
			if ( can_see_obj( to, obj2 ) )
                            i = IS_SET(obj2->extra2, OBJ_PACKAGED)
				? "a package" : obj2->short_descr;
                        else
			    i = "something";
		    }
		    else log_string(LOG_BUG, "Act: bad code $P for 'obj2'");
                    break;
 
                case 'd':
                    if ( arg2 == NULL || ((char *) arg2)[0] == '\0' )
                    {
                        i = "door";
                    }
                    else
                    {
                        one_argument( (char *) arg2, fname );
                        i = fname;
                    }
                    break;

		case 'i':
		    if ( is_number((char *)arg1) )
		    {
			snprintf(fname, sizeof(fname), "%d", atoi((char *)arg1));
			i = fname;
		    }
		    else log_string(LOG_BUG, Format("Act: bad code $i for 'arg1', which is %s", arg1));
		    break;
                }
            }
 
            ++str;
            while ( ( *point = *i ) != '\0' )
                ++point, ++i;
        }
 
        *point++ = '\n';
        *point++ = '\r';
        *point   = '\0';
        buf[0]   = UPPER(buf[0]);
        if (to->desc == NULL)
        {
        	trigger_test(buf, to, ch);
        }
        else
        {
        	pbuff	 = buffer;
        	write_to_buffer( to->desc, buf, 0 );
        }
    }

    for ( ; also_to != NULL; also_to = also_to->next_listener )
    {
        if (!SAME_PLANE(ch, to) && !all_planes && type != TO_OPLANE) continue;

        if ( (type == TO_CHAR) && also_to != ch )
            continue;
        if ( type == TO_VICT )
            continue;
        if ( type == TO_ROOM && also_to == ch )
            continue;
        if ( type == TO_NOTVICT && (also_to == ch || also_to == vch) )
            continue;
        if ( type == TO_GROUP && !is_same_group(also_to, ch) && also_to != ch )
            continue;
        if ( type == TO_OPLANE && also_to == ch )
            continue;
 
        point   = buf;
        str     = format;
        while ( *str != '\0' )
        {
            if ( *str != '$' )
            {
                *point++ = *str++;
                continue;
            }
            ++str;
 
            if ( arg2 == NULL && *str >= 'A' && *str <= 'Z' )
            {
                log_string(LOG_BUG, Format("Act: missing arg2 for code %d.", *str ));
                i = " <@@@> ";
            }
            else
            {
                switch ( *str )
                {
                default:  log_string(LOG_BUG, Format("Act: bad code %d.", *str ));
                          i = " <@@@> ";                                break;
                /* Thx alex for 't' idea */
                case 't': if (arg1) i = (char *) arg1;
			  else log_string(LOG_BUG, "Act: bad code $t for 'arg1'");
                            break;
                case 'T': if (arg2) i = (char *) arg2;
			  else log_string(LOG_BUG, "Act: bad code $T for 'arg2'");
                            break;
                case 'n': if (ch&&also_to) {
        		if(LOOKS_DIFFERENT(ch))
        		{
        		    if(can_see(also_to, ch))
        		        i = ch->alt_name;
	                    else
                		i = "someone";
        		}
        		else
        		    i = PERS( ch, also_to ); }
			  else log_string(LOG_BUG, "Act: bad code $n for 'ch' or 'also_to'");
			    break;
                case 'N': if (vch&&also_to) {
        		if(LOOKS_DIFFERENT(vch))
        		{
        		    if(can_see(also_to, vch))
        		        i = vch->alt_name;
	                    else
                		i = "someone";
        		}
        		else
        		    i = PERS( vch, also_to ); }
			  else log_string(LOG_BUG, "Act: bad code $N for 'vch' or 'also_to'");
			    break;
                case 'e': if (ch) i = he_she  [URANGE(0, ch  ->sex, 2)];
			  else log_string(LOG_BUG, "Act: bad code $e for 'ch'");
		            break;
                case 'E': if (vch) i = he_she  [URANGE(0, vch ->sex, 2)];
			  else log_string(LOG_BUG, "Act: bad code $E for 'vch'");
		            break;
                case 'm': if (ch) i = him_her [URANGE(0, ch  ->sex, 2)];
			  else log_string(LOG_BUG, "Act: bad code $m for 'ch'");
		            break;
                case 'M': if (vch) i = him_her [URANGE(0, vch ->sex, 2)];
			  else log_string(LOG_BUG, "Act: bad code $M for 'vch'");
		            break;
                case 's': if (ch) i = his_her [URANGE(0, ch  ->sex, 2)];
			  else log_string(LOG_BUG, "Act: bad code $s for 'ch'");
		            break;
                case 'S': if (vch) i = his_her [URANGE(0, vch ->sex, 2)];
			  else log_string(LOG_BUG, "Act: bad code $S for 'vch'");
		            break;
 
                case 'p':
                    if (obj1) i = can_see_obj( also_to, obj1 )
                            ? obj1->short_descr
                            : "something";
		    else log_string(LOG_BUG, "Act: bad code $p for 'obj1'");
                    break;
 
                case 'P':
                    if (obj2) i = can_see_obj( also_to, obj2 )
                            ? obj2->short_descr
                            : "something";
		    else log_string(LOG_BUG, "Act: bad code $p for 'obj2'");
                    break;
 
                case 'd':
                    if ( arg2 == NULL || ((char *) arg2)[0] == '\0' )
                    {
                        i = "door";
                    }
                    else
                    {
                        one_argument( (char *) arg2, fname );
                        i = fname;
                    }
                    break;
                }
            }

            ++str;
            while ( ( *point = *i ) != '\0' )
                ++point, ++i;
        }

        *point++ = '\n';
        *point++ = '\r';
        *point   = '\0';
        buf[0]   = UPPER(buf[0]);
        if (also_to->desc == NULL)
        {
        	trigger_test(buf, also_to, ch);
        }
        else
        {
        	pbuff	 = buffer;
        	write_to_buffer( also_to->desc, buffer, 0 );
        }
    }

    return;
}



/*
 * Macintosh support functions.
 */
#if defined(macintosh)
int gettimeofday( struct timeval *tp, void *tzp )
{
    tp->tv_sec  = time( NULL );
    tp->tv_usec = 0;
}
#endif

int colour( char type, CHAR_DATA *ch, char *string )
{
	char	code[ 20 ];
	char	*p = '\0';
	bool	colour = 0;

	if( IS_NPC( ch ) )
		return( 0 );

	if(IS_SET(ch->plr_flags, PLR_COLOUR))
		colour = 1;

	snprintf( code, sizeof(code), colour_lookup(type, colour));

	p = code;
	while( *p != '\0' )
	{
		*string = *p++;
		*++string = '\0';
	}

	return( strlen( code ) );
}

void logfmt (char * fmt, ...)
{
	char buf [2*MSL]={'\0'};
	va_list args;
	va_start (args, fmt);
	vsprintf (buf, fmt, args);
	va_end (args);

	log_string (LOG_ERR, buf);
}

/* this procedure handles the input parsing for the physical attribute generator */
bool parse_gen_physical(CHAR_DATA *ch,char *argument)
{
    char arg[MAX_INPUT_LENGTH]={'\0'};
    int sn = 0,i = 0;
 
    if (IS_NULLSTR(argument))
	return FALSE;

    argument = one_argument(argument,arg);

    if (!str_prefix(arg,"help"))
    {
	if (IS_NULLSTR(argument))
	{
	    do_function(ch, &do_help, "physical" );
	    return TRUE;
	}

	do_function(ch, &do_help, argument );
	return TRUE;
    }

    if (!str_prefix(arg,"add"))
    {
	if (IS_NULLSTR(argument))
	{
	    send_to_char("You must provide an ability name.\n\r",ch);
	    return TRUE;
	}

	if (ch->gen_data->stat_dots[0] == 0)
	{
	send_to_char("You have no more dots available.\n\r",ch);
	return TRUE;
	}

	sn = stat_lookup(argument, ch);

	if (sn != -1
	  && (sn >= 0 || sn < 3) )
	{
	    if (ch->perm_stat[sn] == 5)
	    {
		send_to_char("You cannot increase the attribute more.\n\r",ch);
		return TRUE;
	    }

	    ch->gen_data->stat_dots[0] -= 1;
	    ch->perm_stat[sn] += 1;
	    send_to_char(Format("%s : %d\n\r",stat_table[sn].name, ch->perm_stat[sn]),ch);
	    return TRUE;
	}

	send_to_char("No attributes by that name...\n\r",ch);
	return FALSE;
    }

    if (!str_cmp(arg,"minus"))
    {
	if (IS_NULLSTR(argument))
  	{
	    send_to_char("You must provide an attribute to subtract from.\n\r",ch);
	    return TRUE;
	}

	sn = stat_lookup(argument, ch);
	if (sn != -1 && ch->perm_stat[sn] > 1
	  && ( sn >= 0 || sn < 3) )
	{
	    ch->gen_data->stat_dots[0]++;
	    ch->perm_stat[sn]--;
	    send_to_char(Format("%s : %d\n\r",stat_table[sn].name, ch->perm_stat[sn]),ch);
	    return TRUE;
	}

	send_to_char("You can't subtract any dots from there.\n\r",ch);
	return TRUE;
    }

    if (!str_prefix(arg,"premise"))
    {
	do_function(ch, &do_help, "premise" );
	return TRUE;
    }

    if (!str_prefix(arg,"list"))
    {
	for(i = 0; i < 3; i++)
	    {
	    send_to_char(Format("%s : %d ",stat_table[i].name, ch->perm_stat[i]),ch);
	    }
	send_to_char("\n\r", ch);
	return TRUE;
    }

    return FALSE;
}

/* this procedure handles the input parsing for the social attribute generator */
bool parse_gen_social(CHAR_DATA *ch,char *argument)
{
	char arg[MAX_INPUT_LENGTH]={'\0'};
	int sn = 0,i = 0;

	if (IS_NULLSTR(argument))
		return FALSE;

	argument = one_argument(argument,arg);

	if (!str_prefix(arg,"help"))
	{
		if (IS_NULLSTR(argument))
		{
			do_function(ch, &do_help, "social" );
			return TRUE;
		}

		do_function(ch, &do_help, argument );
		return TRUE;
	}

	if (!str_prefix(arg,"add"))
	{
		if (IS_NULLSTR(argument))
		{
			send_to_char("You must provide an ability name.\n\r",ch);
			return TRUE;
		}

		if (ch->gen_data->stat_dots[1] == 0)
		{
			send_to_char("You have no more dots available.\n\r",ch);
			return TRUE;
		}

		if(stat_lookup(argument, ch) == stat_lookup("appearance", ch)
				&& ch->clan == clan_lookup("nosferatu"))
		{
			send_to_char("The nosferatu may not have a pleasing appearance.\n\r",ch);
			return TRUE;
		}

		sn = stat_lookup(argument, ch);
		if (sn != -1
				&& (sn >= 3 || sn < 6) )
		{
			if (ch->perm_stat[sn] == 5)
			{
				send_to_char("You cannot increase the attribute more.\n\r",ch);
				return TRUE;
			}

			ch->gen_data->stat_dots[1] -= 1;
			ch->perm_stat[sn] += 1;
			send_to_char(Format("%s : %d\n\r",stat_table[sn].name, ch->perm_stat[sn]),ch);
			return TRUE;
		}

		send_to_char("No attributes by that name...\n\r",ch);
		return TRUE;
	}

	if (!str_cmp(arg,"minus"))
	{
		if (IS_NULLSTR(argument))
		{
			send_to_char("You must provide an attribute to subtract from.\n\r",ch);
			return TRUE;
		}

		sn = stat_lookup(argument, ch);
		if (sn != -1 && ch->perm_stat[sn] > 1
				&& (sn >= 3 || sn < 6) )
		{
			ch->gen_data->stat_dots[1] += 1;
			ch->perm_stat[sn] -= 1;
			send_to_char(Format("%s : %d\n\r",stat_table[sn].name, ch->perm_stat[sn]),ch);
			return TRUE;
		}

		send_to_char("You can't subtract any dots from there.\n\r",ch);
		return TRUE;
	}

	if (!str_prefix(arg,"premise"))
	{
		do_function(ch, &do_help, "premise" );
		return TRUE;
	}

	if (!str_prefix(arg,"list"))
	{
		for(i = 3; i < 6; i++)
		{
			send_to_char(Format("%s : %d ",stat_table[i].name, ch->perm_stat[i]),ch);
		}
		send_to_char("\n\r", ch);
		return TRUE;
	}

	return FALSE;
}

/* this procedure handles the input parsing for the mental attribute generator */
bool parse_gen_mental(CHAR_DATA *ch,char *argument)
{
	char arg[MAX_INPUT_LENGTH]={'\0'};
	int sn = 0,i = 0;

	if (IS_NULLSTR(argument))
		return FALSE;

	argument = one_argument(argument,arg);

	if (!str_prefix(arg,"help"))
	{
		if (IS_NULLSTR(argument))
		{
			do_function(ch, &do_help, "mental" );
			return TRUE;
		}

		do_function(ch, &do_help, argument );
		return TRUE;
	}

	if (!str_prefix(arg,"add"))
	{
		if (IS_NULLSTR(argument))
		{
			send_to_char("You must provide an ability name.\n\r",ch);
			return TRUE;
		}

		if (ch->gen_data->stat_dots[2] == 0)
		{
			send_to_char("You have no more dots available.\n\r",ch);
			return TRUE;
		}

		sn = stat_lookup(argument, ch);
		if (sn != -1
				&& (sn >= 6 || sn < 9) )
		{
			if (ch->perm_stat[sn] == 5)
			{
				send_to_char("You cannot increase the attribute more.\n\r",ch);
				return TRUE;
			}

			ch->gen_data->stat_dots[2] -= 1;
			ch->perm_stat[sn] += 1;
			send_to_char(Format("%s : %d\n\r",stat_table[sn].name, ch->perm_stat[sn]),ch);
			return TRUE;
		}

		send_to_char("No attributes by that name...\n\r",ch);
		return TRUE;
	}

	if (!str_cmp(arg,"minus"))
	{
		if (IS_NULLSTR(argument))
		{
			send_to_char("You must provide an attribute to subtract from.\n\r",ch);
			return TRUE;
		}

		sn = stat_lookup(argument, ch);
		if (sn != -1 && ch->perm_stat[sn] > 1
				&& (sn >= 6 || sn < 9) )
		{
			ch->gen_data->stat_dots[2] += 1;
			ch->perm_stat[sn] -= 1;
			send_to_char(Format("%s : %d\n\r",stat_table[sn].name, ch->perm_stat[sn]),ch);
			return TRUE;
		}

		send_to_char("You can't subtract any dots from there.\n\r",ch);
		return TRUE;
	}

	if (!str_prefix(arg,"premise"))
	{
		do_function(ch, &do_help, "premise" );
		return TRUE;
	}

	if (!str_prefix(arg,"list"))
	{
		for(i = 6; i < 9; i++)
		{
			send_to_char(Format("%s : %d ",stat_table[i].name, ch->perm_stat[i]),ch);
		}
		send_to_char("\n\r", ch);
		return TRUE;
	}

	return FALSE;
}

/* this procedure handles the input parsing for the talent abilities generator */
bool parse_gen_talents(CHAR_DATA *ch,char *argument)
{
    char arg[MAX_INPUT_LENGTH]={'\0'};
    int sn = 0,i = 0;
 
    if (IS_NULLSTR(argument))
	return FALSE;

    argument = one_argument(argument,arg);

    if (!str_prefix(arg,"help"))
    {
	if (IS_NULLSTR(argument))
	{
	    do_function(ch, &do_help, "talents" );
	    return TRUE;
	}

	do_function(ch, &do_help, argument );
	return TRUE;
    }

    if (!str_prefix(arg,"add"))
    {
	if (IS_NULLSTR(argument))
	{
	    send_to_char("You must provide an ability name.\n\r",ch);
	    return TRUE;
	}

	if (ch->gen_data->skill_dots[0] == 0)
	{
	send_to_char("You have no more dots available.\n\r",ch);
	return TRUE;
	}

	sn = abil_lookup(argument, ch);
	if (sn != -1 && sn < 13)
	{
	    if (ch->ability[sn].value >= 5)
	    {
		send_to_char("You cannot increase the ability more.\n\r",ch);
		return TRUE;
	    }

	    if (IS_ATTRIB_AVAILABLE(ch->race, sn))
	    {
	    ch->gen_data->skill_dots[0] -= 1;
	    ch->ability[sn].value += 1;
	    send_to_char(Format("%s : %d\n\r",ability_table[sn].name, ch->ability[sn].value),ch);
	    return TRUE;
	    }
	}

	send_to_char("No ability by that name...\n\r",ch);
	return TRUE;
    }

    if (!str_cmp(arg,"minus"))
    {
	if (IS_NULLSTR(argument))
  	{
	    send_to_char("You must provide an ability to subtract from.\n\r",ch);
	    return TRUE;
	}

	sn = abil_lookup(argument, ch);
	if (sn != -1 && sn < 13 && ch->ability[sn].value > 0)
	{
	    ch->gen_data->skill_dots[0] += 1;
	    ch->ability[sn].value -= 1;
	    send_to_char(Format("%s : %d\n\r",ability_table[sn].name, ch->ability[sn].value),ch);
	    return TRUE;
	}

	send_to_char("You can't subtract any dots from there.\n\r",ch);
	return TRUE;
    }

    if (!str_prefix(arg,"premise"))
    {
	do_function(ch, &do_help, "premise" );
	return TRUE;
    }

    if (!str_prefix(arg,"list"))
    {
	for(i = 0; i < 13; i++)
	{
	    if(IS_ATTRIB_AVAILABLE(ch->race, i))
	    {
	    if(i%3 == 0) send_to_char("\n\r", ch);
	    send_to_char(Format("%15s : %d",ability_table[i].name, ch->ability[i].value),ch);
	    }
	}
	send_to_char("\n\r", ch);
	return TRUE;
    }

    return FALSE;
}

/* this procedure handles the input parsing for the skill abilities generator */
bool parse_gen_skills(CHAR_DATA *ch,char *argument)
{
	char arg[MAX_INPUT_LENGTH]={'\0'};
	int sn = 0,i = 0;
	
	if (IS_NULLSTR(argument))
		return FALSE;

	argument = one_argument(argument,arg);

	if (!str_prefix(arg,"help"))
	{
		if (IS_NULLSTR(argument))
		{
			do_function(ch, &do_help, "skills" );
			return TRUE;
		}

		do_function(ch, &do_help, argument );
		return TRUE;
	}

	if (!str_prefix(arg,"add"))
	{
		if (IS_NULLSTR(argument))
		{
			send_to_char("You must provide an ability name.\n\r",ch);
			return TRUE;
		}

		if (ch->gen_data->skill_dots[1] == 0)
		{
			send_to_char("You have no more dots available.\n\r",ch);
			return TRUE;
		}

		sn = abil_lookup(argument, ch);
		if ((sn != -1) && (sn >= 13) && (sn < 24))
		{
			if (ch->ability[sn].value == 5)
			{
				send_to_char("You cannot increase the ability more.\n\r",ch);
				return TRUE;
			}

			if (IS_ATTRIB_AVAILABLE(ch->race, sn))
			{
				ch->gen_data->skill_dots[1] -= 1;
				ch->ability[sn].value += 1;
				send_to_char(Format("%s : %d\n\r",ability_table[sn].name, ch->ability[sn].value),ch);
				return TRUE;
			}
		}

		send_to_char("No ability by that name...\n\r",ch);
		return TRUE;
	}

	if (!str_cmp(arg,"minus"))
	{
		if (IS_NULLSTR(argument))
		{
			send_to_char("You must provide an ability to subtract from.\n\r",ch);
			return TRUE;
		}

		sn = abil_lookup(argument, ch);
		if (sn != -1 && ch->ability[sn].value > 0
			&& (sn >= 13) && (sn < 24))
		{
			ch->gen_data->skill_dots[1] += 1;
			ch->ability[sn].value -= 1;
			send_to_char(Format("%s : %d\n\r",ability_table[sn].name, ch->ability[sn].value),ch);
			return TRUE;
		}

		send_to_char("You can't subtract any dots from there.\n\r",ch);
		return TRUE;
	}

	if (!str_prefix(arg,"premise"))
	{
		do_function(ch, &do_help, "premise" );
		return TRUE;
	}

	if (!str_prefix(arg,"list"))
	{
		for(i = 13; i < 24; i++)
		{
			if(IS_ATTRIB_AVAILABLE(ch->race,i))
			{
				if((i-13)%3 == 0) send_to_char("\n\r", ch);
				send_to_char(Format("%s : %d ",ability_table[i].name, ch->ability[i].value),ch);
			}
		}
		send_to_char("\n\r", ch);
		return TRUE;
	}

	return FALSE;
}

/* this procedure handles the input parsing for the knowledges abilities generator */
bool parse_gen_knowledges(CHAR_DATA *ch,char *argument)
{
    char arg[MAX_INPUT_LENGTH]={'\0'};
    int sn = 0,i = 0;
 
    if (IS_NULLSTR(argument))
	return FALSE;

    argument = one_argument(argument,arg);

    if (!str_prefix(arg,"help"))
    {
	if (IS_NULLSTR(argument))
	{
	    do_function(ch, &do_help, "knowledge" );
	    return TRUE;
	}

	do_function(ch, &do_help, argument );
	return TRUE;
    }

    if (!str_prefix(arg,"add"))
    {
	if (IS_NULLSTR(argument))
	{
	    send_to_char("You must provide an ability name.\n\r",ch);
	    return TRUE;
	}

	if (ch->gen_data->skill_dots[2] == 0)
	{
	send_to_char("You have no more dots available.\n\r",ch);
	return TRUE;
	}

	sn = abil_lookup(argument, ch);
	if (sn != -1
	  && (sn >= 24) && (sn < MAX_ABIL))
	{
	    if (ch->ability[sn].value == 5)
	    {
		send_to_char("You cannot increase the ability more.\n\r",ch);
		return TRUE;
	    }

	    if (IS_ATTRIB_AVAILABLE(ch->race, sn))
	    {
	    ch->gen_data->skill_dots[2] -= 1;
	    ch->ability[sn].value += 1;
	    send_to_char(Format("%s : %d\n\r",ability_table[sn].name, ch->ability[sn].value),ch);
	    return TRUE;
	    }
	}

	send_to_char("No ability by that name...\n\r",ch);
	return TRUE;
    }

    if (!str_cmp(arg,"minus"))
    {
	if (IS_NULLSTR(argument))
  	{
	    send_to_char("You must provide an ability to subtract from.\n\r",ch);
	    return TRUE;
	}

	sn = abil_lookup(argument, ch);
	if (sn != -1 && ch->ability[sn].value > 0
	  && (sn >= 24) && (sn < MAX_ABIL))
	{
	    ch->gen_data->skill_dots[2] += 1;
	    ch->ability[sn].value -= 1;
	    send_to_char(Format("%s : %d\n\r",ability_table[sn].name, ch->ability[sn].value),ch);
	    return TRUE;
	}

	send_to_char("You can't subtract any dots from there.\n\r",ch);
	return TRUE;
    }

    if (!str_prefix(arg,"premise"))
    {
	do_function(ch, &do_help, "premise" );
	return TRUE;
    }

    if (!str_prefix(arg,"list"))
    {
	for(i = 24; ability_table[i].name != NULL; i++)
	{
	    if(IS_ATTRIB_AVAILABLE(ch->race, i))
	    {
	    if((i-24)%3 == 0) send_to_char("\n\r", ch);
	    send_to_char(Format("%s : %d ",ability_table[i].name, ch->ability[i].value),ch);
	    }
	}
	send_to_char("\n\r", ch);
	return TRUE;
    }

    return FALSE;
}

/* this procedure handles the input parsing for the virtues generator */
bool parse_gen_virtues(CHAR_DATA *ch,char *argument)
{
	char arg[MAX_INPUT_LENGTH]={'\0'};
	int sn = 0,i = 0;

	if (IS_NULLSTR(argument))
		return FALSE;

	argument = one_argument(argument,arg);

	if (!str_prefix(arg,"help"))
	{
		if (IS_NULLSTR(argument))
		{
			do_function(ch, &do_help, "virtues" );
			return TRUE;
		}

		do_function(ch, &do_help, argument );
		return TRUE;
	}

	if (!str_prefix(arg,"add"))
	{
		if (IS_NULLSTR(argument))
		{
			send_to_char("You must provide a virtue name.\n\r",ch);
			return TRUE;
		}

		if (ch->gen_data->virtue_dots == 0)
		{
			send_to_char("You have no more dots available.\n\r",ch);
			return TRUE;
		}

		sn = virtue_lookup(argument);
		if (sn != -1)
		{
			if (ch->virtues[sn] == 5)
			{
				send_to_char("You cannot increase the virtue more.\n\r",ch);
				return TRUE;
			}

			ch->gen_data->virtue_dots -= 1;
			ch->virtues[sn] += 1;
			send_to_char(Format("%s : %d\n\r",virtue_table[sn].name, ch->virtues[sn]),ch);
			return TRUE;
		}

		send_to_char("No attributes by that name...\n\r",ch);
		return TRUE;
	}

	if (!str_cmp(arg,"minus"))
	{
		if (IS_NULLSTR(argument))
		{
			send_to_char("You must provide a virtue to subtract from.\n\r",ch);
			return TRUE;
		}

		sn = virtue_lookup(argument);
		if (sn != -1 && ch->virtues[sn] > 1)
		{
			ch->gen_data->virtue_dots += 1;
			ch->virtues[sn] -= 1;
			send_to_char(Format("%s : %d\n\r",virtue_table[sn].name, ch->virtues[sn]),ch);
			return TRUE;
		}

		send_to_char("You can't subtract any dots from there.\n\r",ch);
		return TRUE;
	}

	if (!str_prefix(arg,"premise"))
	{
		do_function(ch, &do_help, "premise" );
		return TRUE;
	}

	if (!str_prefix(arg,"list"))
	{
		for(i = 0; /*virtue_table[i].name != NULL*/i < 3; i++)
		{
			send_to_char(Format("%s : %d ",virtue_table[i].name, ch->virtues[i]),ch);
		}
		send_to_char("\n\r", ch);
		return TRUE;
	}

	return FALSE;
}

/*
 *  Random description generator.
 */
struct desc_type
{
	char * str;
};

const struct desc_type eye_list [] =
{
		{"brown"}, {"hazel"}, {"green"}, {"blue"}, {"blue-green"}, {"blue-grey"}, {"grey"}, {"slate grey"}, {"gun metal grey"}, {"ice blue"}, {"sea green"}
};

const struct desc_type hcolours [] =
{
		{"brown"}, {"black"}, {"blonde"}, {"chestnut"}, {"grey"}, {"silver"}, {"mousy"}, {"white"}, {"streaked"}, {"red"}, {"orange"}
};

const struct desc_type htypes [] =
{
		{"long curly"}, {"long straight"}, {"short straight"}, {"short curly"}, {"bald head"}, {"crew cut"}, {"thick braided"}, {"balding"}
};

const struct desc_type height_list [] =
{
		{"tall"}, {"short"}, {"average height"}
};

const struct desc_type weight_list [] =
{
		{"heavy set"}, {"thin"}, {"of average build"}, {"skinny"}, {"chubby"}
};

const struct desc_type skin_list [] =
{
		{"olive"}, {"evenly tanned"}, {"black"}, {"pale"}, {"white"}, {"quite pink"}
};

const struct desc_type fur_types [] =
{
		{"shaggy brown"}, {"shaggy white"}, {"shaggy grey"}, {"shaggy black"}, {"smooth black"}, {"smooth grey"}, {"smooth white"}, {"smooth brown"}, {"shiny grey"}, {"shiny black"}, {"shiny white"}, {"shiny brown"}
};

const struct desc_type nos_skin_colours [] =
{
		{"green"}, {"grey"}, {"pink"}, {"black"}, {"brown"}
};

const struct desc_type nos_skin_textures [] =
{
		{"calloused"}, {"blotchy"}, {"wrinkled"}, {"scaly"}, {"smooth"}, {"slimy"}
};

const struct desc_type nos_traits [] =
{
		{"hunch-back"}, {"club foot"}, {"three fingered hands"}, {"missing teeth"}, {"tusks"}, {"large flapping ears"}
};

const struct desc_type appearance [] =
{
		{"hideous"}, {"ugly"}, {"plain"}, {"average"}, {"attractive"}, {"radiant"}
};


void desc_gen(CHAR_DATA *ch)
{
	char buf[MSL]={'\0'};
    int eye = number_range(0,10);
    int hcolour = number_range(0,10);
    int htype = number_range(0,6);
    int height = number_range(0,2);
    int weight = number_range(0,4);
    int skin = number_range(0,5);
    int fur = number_range(0,11);
    int nos_skin_tex = number_range(0,4);
    int nos_skin_col = number_range(0,5);
    int nos_trait = number_range(0,5);
    int app = get_curr_stat(ch, STAT_APP);

    if(app > 5) app = 5;

    switch(ch->race)
    {
	case (RACE_VAMPIRE):
	    if (ch->clan == clan_lookup("nosferatu"))
		snprintf(buf, sizeof(buf),
"%s is a hideous creature with %s eyes and %s, %s skin stands here. It is %s and %s with %s.\n\r",
		IS_NPC(ch)?ch->short_descr:ch->name,
		eye_list[eye].str, nos_skin_textures[nos_skin_tex].str,
		nos_skin_colours[nos_skin_col].str,
		height_list[height].str,
		weight_list[weight].str, nos_traits[nos_trait].str);
	    else
		snprintf(buf, sizeof(buf),
"%s is somewhat pale featured but %s to look at, %s has %s eyes and %s, %s hair. %s is %s and %s with %s skin.\n\r",
		IS_NPC(ch)?ch->short_descr:ch->name,
		appearance[app].str,
		ch->sex==1?"she":"he",
		eye_list[eye].str, htypes[htype].str, hcolours[hcolour].str,
		ch->sex==1?"She":"He",
		height_list[height].str, weight_list[weight].str, skin_list[skin].str);
	    break;

	case (RACE_WEREWOLF):
	    snprintf(buf, sizeof(buf),
"%s has a wildness in them, but is %s to look at, %s has %s eyes and %s, %s hair. %s is %s and %s with %s skin.\n\r",
	    IS_NPC(ch)?ch->short_descr:ch->name,
	    appearance[app].str,
	    ch->sex==1?"she":"he",
	    eye_list[eye].str, htypes[htype].str, hcolours[hcolour].str,
	    ch->sex==1?"She":"He",
	    height_list[height].str, weight_list[weight].str, skin_list[skin].str);
	    break;

	case (RACE_CHANGELING):
	    snprintf(buf, sizeof(buf),
"%s is %s to look at, %s has %s eyes and %s, %s hair. %s is %s and %s with %s skin.\n\r",
	    IS_NPC(ch)?ch->short_descr:ch->name,
	    appearance[app].str,
	    ch->sex==1?"she":"he",
	    eye_list[eye].str, htypes[htype].str, hcolours[hcolour].str,
	    ch->sex==1?"She":"He",
	    height_list[height].str, weight_list[weight].str, skin_list[skin].str);
	    break;

	default:
	    snprintf(buf, sizeof(buf),
"%s is %s to look at, %s has %s eyes and %s, %s hair. %s is %s and %s with %s skin.\n\r",
	    IS_NPC(ch)?ch->short_descr:ch->name,
	    appearance[app].str,
	    ch->sex==1?"she":"he",
	    eye_list[eye].str, htypes[htype].str, hcolours[hcolour].str,
	    ch->sex==1?"She":"He",
	    height_list[height].str, weight_list[weight].str, skin_list[skin].str);
	    break;

    }

    fur = 5;

    PURGE_DATA(ch->description);
    ch->description = str_dup(buf);
}

/* printf_to_char originally by John Booth */
void printf_to_char (CHAR_DATA *ch, char *fmt, ...)
{
	char buf[MSL]={'\0'};

    va_list args;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);

    send_to_char(buf, ch);
}

/* Breaking out parts of the nanny code */
void get_new_password(DESCRIPTOR_DATA *d, char *argument)
{
	CHAR_DATA *ch = d->character;
	char *pwdnew;
	char *p;

#if defined(unix)
	write_to_buffer( d, "\n\r", 2 );
#endif

	if ( strlen(argument) < 5 )
	{
		write_to_buffer( d,	"Password must be at least five characters long.\n\rPassword: ", 0 );
		return;
	}

	pwdnew = crypt( argument, ch->name );
	for ( p = pwdnew; *p != '\0'; p++ )
	{
		if ( *p == '~' )
		{
			write_to_buffer( d,	"New password not acceptable, try again.\n\rPassword: ", 0 );
			return;
		}
	}

	PURGE_DATA( ch->pcdata->pwd );
	ch->pcdata->pwd	= str_dup( pwdnew );
	write_to_buffer( d, "Please retype password: ", 0 );
	d->connected = CON_CONFIRM_NEW_PASSWORD;

}

void confirm_new_password(DESCRIPTOR_DATA *d, char *argument)
{
	CHAR_DATA *ch = d->character;

#if defined(unix)
	write_to_buffer( d, "\n\r", 2 );
#endif

	if ( str_cmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
	{
		write_to_buffer( d, "Passwords don't match.\n\rRetype password: ", 0 );
		d->connected = CON_GET_NEW_PASSWORD;
		return;
	}

	ProtocolNoEcho( d, false );
	write_to_buffer(d,"\tWThe following races are available:\tn\n\r",0);
	write_to_buffer(d,"[*] Human - You are a human who belongs to one of the various organizations.\n\r",0);
	write_to_buffer(d,"[*] Vampire - One of the Kindred.\n\r",0);
	write_to_buffer(d,"[*] Werewolf - One of the Garou.\n\r",0);

	write_to_buffer(d,"\n\r",0);
	write_to_buffer(d,"\tWPlease select your race (\tYhelp for more information\tW)?\tn\n\r",0);
	d->connected = CON_GET_NEW_RACE;
}


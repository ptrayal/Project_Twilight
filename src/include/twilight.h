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
 ***************************************************************************/

#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
/* Change these as you need to. */

#define MYURL		"http://themudhost.net/~uriel/pt"
#define MYSERV		"themudhost.net port: 9080"
#define PT_ENGINE	"Project Twilight (Heavily Modified ROM2.4)"
#define WIZUTIL_VERSION "1.6"

/* Debugging flag - Controls output to STDERR */

#define DEBUG 0

#define args( list )			list
#define DECLARE_DO_FUN( fun )		DO_FUN    fun
#define DECLARE_SPEC_FUN( fun )		SPEC_FUN  fun
#define DECLARE_SPELL_FUN( fun )	SPELL_FUN fun
#define DECLARE_QUEST_FUN( fun )	QUEST_FUN fun
#define DECLARE_JOB_FUN( fun )		JOB_FUN fun
#define DECLARE_INFL_FUN( fun )		INFL_FUN fun
#define DECLARE_SPIRIT_FUN( fun )	SPIRIT_FUN fun
#define DECLARE_BLD_FUN( fun )		BLD_FUN fun

/* system calls */
int unlink();
int system();


/*
 * Short scalar types.
 * Diavolo reports AIX compiler has bugs with short types.
 */
#if	!defined(FALSE)
#define FALSE	 0
#endif

#if	!defined(TRUE)
#define TRUE	 1
#endif

#if	defined(_AIX)
#if	!defined(const)
#define const
#endif
typedef int				sh_int;

typedef int				bool;
#define unix
#else
typedef short   int			sh_int;
typedef unsigned char			bool;
#endif

/*
 * Standard Includes.
 */
#include <stdio.h>
#include <string.h>
#include "abilities.h"
#include "gsn.h"
#include "protocol.h"

/*
 * Structure types.
 */
typedef struct	affect_data		AFFECT_DATA;
typedef struct	area_data		AREA_DATA;
typedef struct	ban_data		BAN_DATA;
typedef struct 	buf_type	 	BUFFER;
typedef struct	char_data		CHAR_DATA;
typedef struct  body_data		BODY_DATA;
typedef struct	descriptor_data		DESCRIPTOR_DATA;
typedef struct	exit_data		EXIT_DATA;
typedef struct	extra_descr_data	EXTRA_DESCR_DATA;
typedef struct	help_data		HELP_DATA;
typedef struct	old_help_data		OLD_HELP_DATA;
typedef struct	mem_data		MEM_DATA;
typedef struct	mob_index_data		MOB_INDEX_DATA;
typedef struct	note_data		NOTE_DATA;
typedef struct	newspaper		NEWSPAPER;
typedef struct	stocks			STOCKS;
typedef struct	obj_data		OBJ_DATA;
typedef struct	obj_index_data		OBJ_INDEX_DATA;
typedef struct	pc_data			PC_DATA;
typedef struct  gen_data		GEN_DATA;
typedef struct	reset_data		RESET_DATA;
typedef struct	room_index_data		ROOM_INDEX_DATA;
typedef struct	shop_data		SHOP_DATA;
typedef struct	time_info_data		TIME_INFO_DATA;
typedef struct	weather_data		WEATHER_DATA;
typedef struct  abilities		ABILITIES;
typedef struct  pack_data		PACK_DATA;
typedef struct  org_data		ORG_DATA;
typedef struct  orgmem_data		ORGMEM_DATA;
typedef struct  survey_data		SURVEY_DATA;
typedef struct  trait_data		TRAIT_DATA;
typedef struct  liquid_data		LIQUID_DATA;
typedef struct  mprog_list		MPROG_LIST;
typedef struct  mprog_code		MPROG_CODE;

extern bool     timestop;		/* Time stopped flag.  */
extern int	reboot_countdown;	/* Reboot countdown timer */
extern char	reboot_initiator[50];	/* Reboot initiator */

extern BUFFER *buffer_list;
extern int top_buffer;

/*
 * Function types.
 */
typedef	void DO_FUN	args( ( CHAR_DATA *ch, char *argument ) );
typedef bool SPEC_FUN	args( ( CHAR_DATA *ch ) );
typedef void SPELL_FUN	args( ( int sn, int level, CHAR_DATA *ch, void *vo,
				int target ) );
typedef void QUEST_FUN  args( ( CHAR_DATA *ch, int flag ) );
typedef	int  JOB_FUN	args( ( CHAR_DATA *ch, char *argument ) );
typedef	int  INFL_FUN	args( ( CHAR_DATA *ch, char *argument ) );
typedef	int SPIRIT_FUN args(( CHAR_DATA *ch, char *argument, bool n_r,
				OBJ_DATA *obj ));
typedef	int  BLD_FUN	args( ( CHAR_DATA *ch, char *argument, int type,
				ORG_DATA *org ) );


/*
 * String and memory management parameters.
 */
#define	MAX_KEY_HASH		 1024
#define MAX_STRING_LENGTH	 4096
#define MAX_INPUT_LENGTH	 256
#define MAX_LOG			 1048576
#define PAGELEN			   22

#define MSL MAX_STRING_LENGTH
#define MIL MAX_INPUT_LENGTH

/*
 * Game parameters.
 * Increase the max'es if you add more of something.
 * Adjust the pulse numbers to suit yourself.
 */
#define MUD_PORT				9080
/* #define MAX_SOCIALS		  256 */
#define MAX_SKILL				150
#define MAX_ABIL				38
#define MAX_GROUP				30
#define MAX_IN_GROUP			15
#define MAX_ALIAS				5
#define MAX_CLAN				31
#define MAX_DAMAGE_MESSAGE		41
#define MAX_LEVEL				5
#define LEVEL_IMMORTAL			(MAX_LEVEL - 3)
#define MAX_HEALTH				7
#define MAX_VIRTUES				4
#define MAX_DISC				21
#define MAX_EXITS				6
#define MAX_NOMINEES			6
#define MAX_OFFICES				3
#define MAX_JUDGES				3
#define MAX_ALDERMEN			3
#define MAX_HALLUC				15
#define MAX_COLOURS				16
#define MAX_CAR_STOPS			20
#define PC_RACE					11
#define MAX_POWERS				5
#define MAX_ARTICLES			20
#define MAX_OWNED_ROOMS			20
#define MAX_VOTE_OPTIONS		10
#define MAX_RITE_STEPS			10
#define MAX_RITE_ACTIONS		19
#define WARRANT_THRESHOLD		50
#define START_DOTS				84
#define HOURS_PER_EXP			600
#define CURRENT_PFILE_VERSION	3
#define TESTING_PFILE_VERSION	3
#define NO_LOGS					0
#define MAX_JOB_SKILLS			50
#define CURRENT_AREA_VERSION	1

#define PULSE_PER_SECOND		4
#define PULSE_VIOLENCE			( 6 * PULSE_PER_SECOND)
#define PULSE_MOBILE			( 4 * PULSE_PER_SECOND)
#define PULSE_MUSIC				( 6 * PULSE_PER_SECOND)
#define PULSE_TICK				(60 * PULSE_PER_SECOND)
#define PULSE_AREA				(120 * PULSE_PER_SECOND)

#define IMPLEMENTOR			MAX_LEVEL
#define	MASTER				(MAX_LEVEL - 1)
#define STORYTELLER			(MAX_LEVEL - 2)
#define LINE_LENGTH			72

/*
 * Colour stuff by Lope of Loping Through The MUD
 */
#define CLEAR		"[0m"		/* Resets Colour	*/
#define C_RED		"[0;31m"	/* Normal Colours	*/
#define C_GREEN		"[0;32m"
#define C_YELLOW	"[0;33m"
#define C_BLUE		"[0;34m"
#define C_MAGENTA	"[0;35m"
#define C_CYAN		"[0;36m"
#define C_WHITE		"[0;37m"
#define C_D_GREY	"[1;30m"  	/* Light Colors		*/
#define C_B_RED		"[1;31m"
#define C_B_GREEN	"[1;32m"
#define C_B_YELLOW	"[1;33m"
#define C_B_BLUE	"[1;34m"
#define C_B_MAGENTA	"[1;35m"
#define C_B_CYAN	"[1;36m"
#define C_B_WHITE	"[1;37m"
#define C_DING		""

/* Web Colours */
#define WEBCLEAR		"</FONT>"		/* Resets Colour	*/
#define WEBC_RED		"<FONT COLOR=\"light red\">"/* Normal Colours	*/
#define WEBC_GREEN		"<FONT COLOR=\"light green\">"
#define WEBC_YELLOW		"<FONT COLOR=\"light yellow\">"
#define WEBC_BLUE		"<FONT COLOR=\"light blue\">"
#define WEBC_MAGENTA	"<FONT COLOR=\"light magenta\">"
#define WEBC_CYAN		"<FONT COLOR=\"light cyan\">"
#define WEBC_WHITE		"<FONT COLOR=\"white\">"
#define WEBC_D_GREY		"<FONT COLOR=\"grey\">" /* Light Colors		*/
#define WEBC_B_RED		"<FONT COLOR=\"red\">"
#define WEBC_B_GREEN	"<FONT COLOR=\"green\">"
#define WEBC_B_YELLOW	"<FONT COLOR=\"yellow\">"
#define WEBC_B_BLUE		"<FONT COLOR=\"blue\">"
#define WEBC_B_MAGENTA	"<FONT COLOR=\"magenta\">"
#define WEBC_B_CYAN		"<FONT COLOR=\"cyan\">"
#define WEBC_B_WHITE	"<FONT COLOR=\"white\">"

/* "Colours" for printing special characters */
#define CURLY_BRACKET	"{"
#define CR				"\n\r"
#define TILDE			"~"

/*
 * Site ban structure.
 */

#define BAN_SUFFIX		(A)
#define BAN_PREFIX		(B)
#define BAN_NEWBIES		(C)
#define BAN_ALL			(D)	
#define BAN_PERMIT		(E)
#define BAN_PERMANENT		(F)

struct	ban_data
{
	BAN_DATA *	next;
	sh_int	ban_flags;
	sh_int	level;
	char *	name;
};

struct buf_type
{
	BUFFER *next;
	bool        valid;
	sh_int      state;  /* error state of the buffer */
	sh_int      size;   /* size in k */
	char *      string; /* buffer's string */
	char *file;
	char *function;
	int line;
};



/*
 * Time and weather stuff.
 */
#define SUN_DARK		    0
#define SUN_RISE		    1
#define SUN_LIGHT		    2
#define SUN_SET			    3

#define MOON_NEW			0
#define MOON_CRESCENT		1
#define MOON_HALF			2
#define MOON_GIBBOUS		3
#define MOON_FULL			4

#define SKY_CLOUDLESS		0
#define SKY_CLOUDY			1
#define SKY_RAINING			2
#define SKY_LIGHTNING		3
#define SKY_FOGGY		    4
#define SKY_THICKFOG		5
#define SKY_PEASOUPFOG		6

struct	time_info_data
{
	int		min;
	int		hour;
	int		day;
	int		month;
	int		year;
	int		moon_phase;
};

struct	weather_data
{
	int		mmhg;
	int		change;
	int		sky;
	int		sunlight;
};

/*
 * Copyover stuff.
 */
#define COPYOVER_FILE       "copyover.data"
#define EXE_FILE            "../src/project"
#define CH(descriptor)  ((descriptor)->original ? (descriptor)->original : (descriptor)->character)

/*
 * Backup Stuff.
 */
extern bool backup;

/*
 * Non-redirected stderr logging.
 */
extern char logfile[MAX_INPUT_LENGTH];

/*
 * Connected state for a channel.
 */
#define CON_PLAYING			 0
#define CON_GET_NAME			 1
#define CON_GET_OLD_PASSWORD		 2
#define CON_CONFIRM_NEW_NAME		 3
#define CON_GET_NEW_PASSWORD		 4
#define CON_CONFIRM_NEW_PASSWORD	 5
#define CON_GET_NEW_RACE		 6
#define CON_GET_NEW_SEX			 7
#define CON_GET_NEW_CLAN		 8
#define CON_ASSIGN_PHYS			17
#define CON_ASSIGN_SOC			16
#define CON_ASSIGN_MENT			 9
#define CON_ASSIGN_TAL			10
#define CON_ASSIGN_SKILL		11
#define CON_ASSIGN_KNOWLEDGES		12
#define CON_READ_IMOTD			13
#define CON_READ_MOTD			14
#define CON_BREAK_CONNECT		15
#define CON_CHOOSE_ATTRIB		18
#define CON_CHOOSE_ABIL			19
#define CON_ASSIGN_VIRTUES		20
#define CON_DECEASED			25
#define CON_ETHEREAL			26
#define CON_REINCARNATE			27
#define CON_CONFIRM_REINCARNATE_NAME	28
#define CON_COPYOVER_RECOVER		29
#define CON_MUDLINK			30
#define CON_AIDESC			31
#define CON_GET_ACCT_NAME		32
#define CON_GET_ACCT_PASS		33
#define CON_ACCT_MENUCHOICE		34
#define MAX_CON_STATE			35

/*
 * Stuff for race availability functions.
 */
#define HU      (A)
#define WW      (B)
#define VA      (C)
#define FA      (D)

/*
 * Conceed flags.
 */
#define CEED_DIE		(A)
#define CEED_EMBRACE	(B)
#define CEED_FEED		(C)
#define CEED_GHOUL		(D)
#define CEED_ARREST		(E)

/*
 * Descriptor (channel) structure.
 */
struct	descriptor_data
{
    DESCRIPTOR_DATA *	next;
    DESCRIPTOR_DATA *	snoop_by;
    CHAR_DATA *		character;
    CHAR_DATA *		original;
    char *		host;
    sh_int		descriptor;
    sh_int		connected;
    bool		fcommand;
    char		inbuf		[4 * MAX_INPUT_LENGTH];
    char		incomm		[MAX_INPUT_LENGTH];
    char		inlast		[MAX_INPUT_LENGTH];
    int			repeat;
    char *		outbuf;
    int			outsize;
    int			outtop;
    char *		showstr_head;
    char *		showstr_point;
    void *		pEdit; /*OLC*/
    char **		pString; /*OLC*/
    int			editor; /*OLC*/
    protocol_t *        pProtocol;
};



/*
 * Attribute bonus structures.
 */
struct	str_app_type
{
    sh_int	tohit;
    sh_int	todam;
    sh_int	carry;
    sh_int	wield;
};

/*
 * TO types for act.
 */
#define TO_ROOM		    0
#define TO_NOTVICT	    1
#define TO_VICT		    2
#define TO_CHAR		    3
#define TO_ALL		    4
#define TO_OROOM	    5
#define TO_GROUP	    6
#define TO_OPLANE	    7



/*
 * Help table types.
 */
struct	help_data
{
    HELP_DATA *	next;
    sh_int	level;
    char *	keyword;
    char *	races;
    char *	clans;
    char *	topic;
    char *	syntax;
    char *	description;
    char *	see_also;
    char *	quote;
    char *	website;
    char *	unformatted;
};


/*
 * Shop types.
 */
#define MAX_TRADE	 5

struct	shop_data
{
    SHOP_DATA *	next;			/* Next shop in list		*/
    sh_int	keeper;			    /* Vnum of shop keeper mob	*/
    sh_int	buy_type [MAX_TRADE];	/* Item types shop will buy	*/
    sh_int	profit_buy;		    /* Cost multiplier for buying	*/
    sh_int	profit_sell;		/* Cost multiplier for selling	*/
    sh_int	open_hour;		    /* First opening hour		*/
    sh_int	close_hour;		    /* First closing hour		*/
    sh_int	raw_materials;		/* Flag for raw material sales. PT */
};


#define MATERIAL_NONE		0
#define MATERIAL_EDIBLE		(A)
#define MATERIAL_POISON		(B)
#define MATERIAL_METAL		(C)
#define MATERIAL_EXPLOSIVE	(D)
#define MATERIAL_LIQUID		(E)
#define MATERIAL_DRUGS		(F)
#define MATERIAL_ALCOHOL	(G)
#define MATERIAL_FABRIC		(H)
#define MATERIAL_STONE		(I)

/*
 * Stats stuff.
 */

#define MAX_STATS 	10
#define STAT_STR 	0
#define STAT_DEX	1
#define STAT_STA	2
#define STAT_CHA	3
#define STAT_MAN	4
#define STAT_APP	5
#define STAT_PER	6
#define STAT_INT	7
#define STAT_WIT	8
#define STAT_NONE	9

struct item_type
{
    int		type;
    char *	name;
};

struct weapon_type
{
    char *	name;
    sh_int	vnum;
    sh_int	type;
    int		*gsn;
};

struct wiznet_type
{
    char *	name;
    long 	flag;
    int		level;
};

struct attack_type
{
    char *	name;			/* name */
    char *	noun;			/* message */
    int   	damage;			/* damage class */
    bool   	agg;			/* aggravated? */
};

struct race_type
{
    char *	name;			/* call name of the race */
    bool	pc_race;		/* can be chosen by pcs */
    long	act;			/* act bits for the race */
    long	aff;			/* aff bits for the race */
    long	off;			/* off bits for the race */
    long	imm;			/* imm bits for the race */
    long	res;			/* res bits for the race */
    long	vuln;			/* vuln bits for the race */
    long	form;			/* default form flag for the race */
    long	parts;			/* default parts for the race */
};


struct pc_race_type  /* additional data for pc races */
{
    char *	name;			/* MUST be in race_type */
    char 	who_name[14];
    char *      clan_word;
    char *      GHB;
    char *      RBPG;
    sh_int	max_stats[MAX_STATS];	/* maximum stats */
    sh_int	size;			/* aff bits for the race */
};


struct spec_type
{
    char * 	name;			/* special function name */
    SPEC_FUN *	function;		/* the function */
};



/*
 * Data structure for notes.
 */

#define NOTE_NOTE	0
#define NOTE_BACKGROUND	1
#define NOTE_KNOWLEDGE	2
#define NOTE_ARTICLE	3
struct	note_data
{
    NOTE_DATA *	next;
    sh_int	type;
    char *	sender;
    char *	date;
    char *	to_list;
    char *	subject;
    char *	text;
    time_t  	date_stamp;
    /* For use with knowledges and backgrounds */
    int		race;
    int		successes;
};

extern NOTE_DATA *know_list;
extern NOTE_DATA *bg_list;
extern NOTE_DATA *news_list;
extern NEWSPAPER *paper_list;
extern SURVEY_DATA *survey_list;
extern SURVEY_DATA *survey_last;

/*
 * Structure for the stock market.
 */
struct stocks {
    char	*name;
    char	*ticker;
    long	last_change;
    int		cost;
    int		phase;
    int		upordown;
    STOCKS	*next;
};

extern STOCKS *stock_list;


/*
 * Structure for Newspapers
 */
struct newspaper {
    char	*name;
    sh_int	on_stands;
    long	articles[MAX_ARTICLES];
    int		cost;
    NEWSPAPER	*next;
};

extern NEWSPAPER *newspapers;


/*
 * An affect.
 */
struct	affect_data
{
    AFFECT_DATA *	next;
    sh_int		where;
    sh_int		type;
    sh_int		level;
    sh_int		duration;
    sh_int		location;
    sh_int		modifier;
    int			bitvector;
};

/* where definitions */
#define TO_AFFECTS	0
#define TO_OBJECT	1
#define TO_IMMUNE	2
#define TO_RESIST	3
#define TO_VULN		4
#define TO_WEAPON	5
#define TO_ROOM_AFF	6
#define TO_AFF2		7
#define TO_SKILLS	8



/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (Start of section ... start here)                     *
 *                                                                         *
 ***************************************************************************/

/*
 * Well known mob virtual numbers.
 * Defined in #MOBILES.
 */
#define MOB_VNUM_BLANKY		   5
#define MOB_VNUM_TENTACLES	  12 /* For Obtenebration 3:Arms of the Abyss */


/* RT ASCII conversions -- used so we can have letters in this file */

#define A		  	1
#define B			2
#define C			4
#define D			8
#define E			16
#define F			32
#define G			64
#define H			128

#define I			256
#define J			512
#define K		    1024
#define L		 	2048
#define M			4096
#define N		 	8192
#define O			16384
#define P			32768

#define Q			65536
#define R			131072
#define S			262144
#define T			524288
#define U			1048576
#define V			2097152
#define W			4194304
#define X			8388608

#define Y			16777216
#define Z			33554432
#define aa			67108864 	/* doubled due to conflicts */
#define bb			134217728
#define cc			268435456    
#define dd			536870912
#define ee			1073741824

/*
 * ACT bits for mobs.
 * Used in #MOBILES.
 */
#define ACT_IS_NPC			(A)		/* Auto set for mobs	*/
#define ACT_SENTINEL		(B)		/* Stays in one room	*/
#define ACT_SCAVENGER		(C)		/* Picks up objects	*/
#define ACT_AGGRESSIVE		(F)    	/* Attacks PC's		*/
#define ACT_STAY_AREA		(G)		/* Won't leave area	*/
#define ACT_WIMPY			(H)
#define ACT_PET				(I)		/* Auto set for pets	*/
#define ACT_INTELLIGENT		(J)		/* Has a personality	*/
#define ACT_VAMPIRE			(K)		/* Can practice PC's	*/
#define ACT_WEREWOLF		(O)	
#define ACT_CHANGELING		(Q)
#define ACT_CHIMERAE		(R)
#define ACT_FAST			(S)
#define ACT_UMBRA			(T)
#define ACT_DREAMING		(U)
#define ACT_NOPURGE			(V)
#define ACT_OUTDOORS		(W)
#define ACT_INDOORS			(Y)
#define ACT_REINCARNATE		(Z)
#define ACT_IS_HEALER		(aa)
#define ACT_SUMMON			(bb)
#define ACT_UPDATE_ALWAYS	(cc)
#define ACT_IS_CHANGER		(dd)
#define ACT_TWIN			(ee)

#define ACT2_POLICE			(A)
#define ACT2_COURT			(B)
#define ACT2_TWILIGHT		(C)
#define ACT2_FRENZY			(D)
#define ACT2_ASTRAL			(E)
#define ACT2_NO_HIRE		(F)
#define ACT2_EMPLOYER		(G)
#define ACT2_RESIST			(H)
#define ACT2_WYRM			(I)
#define ACT2_WEAVER			(J)
#define ACT2_WYLD			(K)
#define ACT2_SPIRIT			(L)
#define ACT2_STORY			(M)
#define ACT2_NEWS_STAND		(N)
#define ACT2_RP_ING			(O)
#define ACT2_GO_SHOP		(P)
#define ACT2_SHOPPER		(Q)
#define ACT2_GHOUL			(R)
#define ACT2_NOWARRANT		(S)
#define ACT2_HUNTER			(T)
#define ACT2_NOGIFT			(U)

#define ACT2_NOQUEST		(aa)
#define ACT2_NOQUESTAGGR	(bb)
#define ACT2_NOQUESTVICT	(cc)

/* damage classes */
#define DAM_NONE			0
#define DAM_BASH			1
#define DAM_PIERCE			2
#define DAM_SLASH			3
#define DAM_FIRE			4
#define DAM_COLD			5
#define DAM_LIGHTNING		6
#define DAM_ACID			7
#define DAM_POISON			8
#define DAM_NEGATIVE		9
#define DAM_HOLY			10
#define DAM_MENTAL			11
#define DAM_DISEASE			12
#define DAM_DROWNING		13
#define DAM_OTHER			14
#define DAM_SHOT			15
#define DAM_IRON			16
#define DAM_SILVER			17

/* OFF bits for mobiles */
#define OFF_AREA_ATTACK			(A)
#define OFF_BACKSTAB			(B)
#define OFF_BASH				(C)
#define OFF_TOUGH				(D)
#define OFF_DISARM				(E)
#define OFF_DODGE				(F)
#define OFF_FADE				(G)
#define OFF_FAST				(H)
#define OFF_KICK				(I)
#define OFF_KICK_DIRT			(J)
#define OFF_PARRY				(K)
#define OFF_RESCUE				(L)
#define OFF_TAIL				(M)
#define OFF_TRIP				(N)
#define OFF_CRUSH				(O)
#define ASSIST_ALL				(P)
#define OFF_STRONG				(Q)
#define ASSIST_RACE				(R)
#define ASSIST_PLAYERS			(S)
#define ASSIST_VNUM				(T)
#define BANDAGED				(U)
#define LOADED_DICE_POS			(V)
#define LOADED_DICE_NEG			(W)
#define LOADED_DICE_BPOS		(X)
#define LOADED_DICE_BNEG		(Y)

/* return values for check_imm */
#define IS_NORMAL			0
#define IS_IMMUNE			1
#define IS_RESISTANT		2
#define IS_VULNERABLE		3

/* IMM bits for mobs */
#define IMM_SUMMON              (A)
#define IMM_CHARM               (B)
#define IMM_MAGIC               (C)
#define IMM_WEAPON              (D)
#define IMM_BASH                (E)
#define IMM_PIERCE              (F)
#define IMM_SLASH               (G)
#define IMM_FIRE                (H)
#define IMM_COLD                (I)
#define IMM_LIGHTNING           (J)
#define IMM_ACID                (K)
#define IMM_POISON              (L)
#define IMM_NEGATIVE            (M)
#define IMM_HOLY                (N)
#define IMM_ENERGY              (O)
#define IMM_MENTAL              (P)
#define IMM_DISEASE             (Q)
#define IMM_DROWNING            (R)
#define IMM_LIGHT		(S)
#define IMM_SOUND		(T)
#define IMM_WOOD                (X)
#define IMM_SILVER              (Y)
#define IMM_IRON                (Z)
 
/* RES bits for mobs */
#define RES_SUMMON		(A)
#define RES_CHARM		(B)
#define RES_MAGIC               (C)
#define RES_WEAPON              (D)
#define RES_BASH                (E)
#define RES_PIERCE              (F)
#define RES_SLASH               (G)
#define RES_FIRE                (H)
#define RES_COLD                (I)
#define RES_LIGHTNING           (J)
#define RES_ACID                (K)
#define RES_POISON              (L)
#define RES_NEGATIVE            (M)
#define RES_HOLY                (N)
#define RES_ENERGY              (O)
#define RES_MENTAL              (P)
#define RES_DISEASE             (Q)
#define RES_DROWNING            (R)
#define RES_LIGHT		(S)
#define RES_SOUND		(T)
#define RES_WOOD                (X)
#define RES_SILVER              (Y)
#define RES_IRON                (Z)
 
/* VULN bits for mobs */
#define VULN_SUMMON		(A)
#define VULN_CHARM		(B)
#define VULN_MAGIC              (C)
#define VULN_WEAPON             (D)
#define VULN_BASH               (E)
#define VULN_PIERCE             (F)
#define VULN_SLASH              (G)
#define VULN_FIRE               (H)
#define VULN_COLD               (I)
#define VULN_LIGHTNING          (J)
#define VULN_ACID               (K)
#define VULN_POISON             (L)
#define VULN_NEGATIVE           (M)
#define VULN_HOLY               (N)
#define VULN_ENERGY             (O)
#define VULN_MENTAL             (P)
#define VULN_DISEASE            (Q)
#define VULN_DROWNING           (R)
#define VULN_LIGHT		(S)
#define VULN_SOUND		(T)
#define VULN_WOOD               (X)
#define VULN_SILVER             (Y)
#define VULN_IRON		(Z)
 
/* body form */
#define FORM_EDIBLE             (A)
#define FORM_POISON             (B)
#define FORM_MAGICAL            (C)
#define FORM_INSTANT_DECAY      (D)
#define FORM_OTHER              (E)  /* defined by material bit */
 
/* actual form */
#define FORM_BLOOD              (F)
#define FORM_ANIMAL             (G)
#define FORM_SENTIENT           (H)
#define FORM_UNDEAD             (I)
#define FORM_CONSTRUCT          (J)
#define FORM_MIST               (K)
#define FORM_INTANGIBLE         (L)
 
#define FORM_BIPED              (M)
#define FORM_CENTAUR            (N)
#define FORM_INSECT             (O)
#define FORM_SPIDER             (P)
#define FORM_CRUSTACEAN         (Q)
#define FORM_WORM               (R)
#define FORM_BLOB		(S)

#define FORM_SHADOW		(T)
#define FORM_HORRID		(U)

#define FORM_MAMMAL             (V)
#define FORM_BIRD               (W)
#define FORM_REPTILE            (X)
#define FORM_SNAKE              (Y)
#define FORM_DRAGON             (Z)
#define FORM_AMPHIBIAN          (aa)
#define FORM_FISH               (bb)
#define FORM_COLD_BLOOD		(cc)
#define FORM_PLANT		(dd)
#define FORM_ASH		(ee)

/* Shapes for transformations */
#define SHAPE_NONE		-1
#define SHAPE_HUMAN		0
#define SHAPE_WOLF		1
#define SHAPE_CRINOS		2
#define SHAPE_BAT		3
#define SHAPE_SNAKE		4

/* body parts */
#define PART_HEAD               (A)
#define PART_ARMS               (B)
#define PART_LEGS               (C)
#define PART_HEART              (D)
#define PART_BRAINS             (E)
#define PART_GUTS               (F)
#define PART_HANDS              (G)
#define PART_FEET               (H)
#define PART_FINGERS            (I)
#define PART_EAR                (J)
#define PART_EYE				(K)
#define PART_LONG_TONGUE        (L)
#define PART_EYESTALKS          (M)
#define PART_TENTACLES          (N)
#define PART_FINS               (O)
#define PART_WINGS              (P)
#define PART_TAIL               (Q)
#define PART_WEBBING            (R)
#define PART_BIGNOSE            (S)
#define PART_DISTORTION         (T)
/* for combat */
#define PART_CLAWS              (U)
#define PART_FANGS              (V)
#define PART_HORNS              (W)
#define PART_SCALES             (X)
#define PART_TUSKS				(Y)
#define PART_THREAD				(Z)
#define PART_SCALED_SKIN		(aa)

/*
 * Bits for 'affected_by'.
 * Used in #MOBILES.
 */
#define AFF_NONE			0
#define AFF_BLIND			(A)
#define AFF_INVISIBLE		(B)
#define AFF_CALM			(C)
#define AFF_DETECT_INVIS	(D)
#define AFF_ENCHANTED		(E)
#define AFF_DETECT_HIDDEN	(F)
#define AFF_LUNA			(G)
#define AFF_HSENSES			(H)
#define AFF_TRUE_SCENT		(I)
#define AFF_INFRARED		(J)
#define AFF_CURSE			(K)
#define AFF_AUSPEX4			(L)
#define AFF_POISON			(M)
#define AFF_PROTECT_EVIL	(N)
#define AFF_OBFUSCATE3		(O)
#define AFF_SNEAK			(P)
#define AFF_HIDE			(Q)
#define AFF_SLEEP			(R)
#define AFF_CHARM			(S)
#define AFF_FLYING			(T)
#define AFF_CLAWED			(U)
#define AFF_ODOUR			(V)
#define AFF_HEAT_METAL		(W)
#define AFF_PLAGUE			(X)
#define AFF_WEAKEN			(Y)
#define AFF_DARK_VISION		(Z)
#define AFF_SCLAWS			(aa)
#define AFF_SWIM			(bb)
#define AFF_REGENERATION	(cc)
#define AFF_SLOW			(dd)
#define AFF_RESIST_PAIN		(ee)

/*
 * Bits for 'affected_by'.
 * Used in #MOBILES.
 */
#define AFF2_NONE			0
#define AFF2_EARTHMELD		(A)
#define AFF2_VICISSITUDE1	(B)
#define AFF2_IMMOBILIZED	(C)

/*
 * Sex.
 * Used in #MOBILES.
 */
#define SEX_NEUTRAL		      0
#define SEX_MALE		      1
#define SEX_FEMALE		      2

/* AC types */
#define AC_LIGHT			0
#define AC_SOLID			1
#define AC_HARD				2
#define AC_EXOTIC			3

/* dice */
#define DICE_NUMBER			0
#define DICE_TYPE			1
#define DICE_BONUS			2

/* size */
#define SIZE_MINUTE			0
#define SIZE_TINY			1
#define SIZE_SMALL			2
#define SIZE_MEDIUM			3
#define SIZE_LARGE			4
#define SIZE_HUGE			5
#define SIZE_GIANT			6
#define SIZE_GARGANTUAN			7
#define MAX_SIZE			7



/*
 * Well known object virtual numbers.
 * Defined in #OBJECTS.
 */
#define OBJ_VNUM_MAP		      1

#define OBJ_VNUM_DOLLAR_ONE	      2
#define OBJ_VNUM_DOLLARS_SOME	      3
#define OBJ_VNUM_CENT_ONE	      4
#define OBJ_VNUM_CENTS_SOME	      5
#define OBJ_VNUM_COINS		      6

#define OBJ_VNUM_CORPSE_NPC	     10
#define OBJ_VNUM_CORPSE_PC	     11
#define OBJ_VNUM_SEVERED_HEAD	     12
#define OBJ_VNUM_TORN_HEART	     13
#define OBJ_VNUM_SLICED_ARM	     14
#define OBJ_VNUM_SLICED_LEG	     15
#define OBJ_VNUM_GUTS		     16
#define OBJ_VNUM_BRAINS		     17

#define OBJ_VNUM_GRUEL		     20
#define OBJ_VNUM_CAR_DOORS	     22
#define OBJ_VNUM_NEWSPAPER	     23
#define OBJ_VNUM_PUDDLE		     24
#define OBJ_VNUM_POOL		     25

#define OBJ_VNUM_HUNTER_RIFLE		    100
#define OBJ_VNUM_HUNTER_RIFLECLIP	    101
#define OBJ_VNUM_HUNTER_SILVERSWORD	    102
#define OBJ_VNUM_HUNTER_IRONPOKER	    103
#define OBJ_VNUM_HUNTER_FLAMETHROWER	    104
#define OBJ_VNUM_HUNTER_FLAMETFUELTANK	    105
#define OBJ_VNUM_HUNTER_TANKERBOOTS	    106
#define OBJ_VNUM_HUNTER_CARGOPANTS	    107
#define OBJ_VNUM_HUNTER_MESHJACKET	    108
#define OBJ_VNUM_HUNTER_TSHIRT		    109
#define OBJ_VNUM_HUNTER_DRESSSHOES	    110
#define OBJ_VNUM_HUNTER_DRESSPANTS	    111
#define OBJ_VNUM_HUNTER_DRESSSHIRT	    112
#define OBJ_VNUM_HUNTER_ROMANCOLLAR	    113
#define OBJ_VNUM_HUNTER_CREAMJACKET	    114

#define OBJ_VNUM_MALE1		   1000
#define OBJ_VNUM_MALE2		   1001
#define OBJ_VNUM_MALE3		   1002
#define OBJ_VNUM_MALE4		   1003
#define OBJ_VNUM_MALE5		   1004
#define OBJ_VNUM_FEMALE1	   1005
#define OBJ_VNUM_FEMALE2	   1006
#define OBJ_VNUM_FEMALE3	   1007
#define OBJ_VNUM_FEMALE4	   1008
#define OBJ_VNUM_MALE6		   1009

/*
 * Item types.
 * Used in #OBJECTS.
 */
#define ITEM_LIGHT		      1
#define ITEM_SCROLL		      2
#define ITEM_WEAPON		      5
#define ITEM_TREASURE		  8
#define ITEM_ARMOR		      9
#define ITEM_POTION		     10
#define ITEM_CLOTHING		 11
#define ITEM_FURNITURE		 12
#define ITEM_TRASH		     13
#define ITEM_CONTAINER		 15
#define ITEM_DRINK_CON		 17
#define ITEM_KEY		     18
#define ITEM_FOOD		     19
#define ITEM_MONEY		     20
#define ITEM_BOAT		     22
#define ITEM_CORPSE_NPC		 23
#define ITEM_CORPSE_PC		 24
#define ITEM_FOUNTAIN		 25
#define ITEM_PILL		     26
#define ITEM_MAP		     28
#define ITEM_PORTAL		     29
#define ITEM_TELEPHONE		 30
#define ITEM_AMMO		     31
#define ITEM_BOMB		     32
#define ITEM_LIQUID		     33
#define ITEM_RELIC			 34


/*
 * Extra flags.
 * Used in #OBJECTS.
 */
#define ITEM_GLOW		(A)
#define ITEM_ANTI_HUMAN		(B)
#define ITEM_DARK		(C)
#define ITEM_LOCK		(D)
#define ITEM_EVIL		(E)
#define ITEM_INVIS		(F)
#define ITEM_MAGIC		(G)
#define ITEM_NODROP		(H)
#define ITEM_BLESS		(I)
#define ITEM_ANTI_VAMPIRE	(J)
#define ITEM_ANTI_WEREWOLF	(K)
#define ITEM_ANTI_CHANGELING	(L)
#define ITEM_NOREMOVE		(M)
#define ITEM_INVENTORY		(N)
#define ITEM_NOPURGE		(O)
#define ITEM_ROT_DEATH		(P)
#define ITEM_VIS_DEATH		(Q)
#define ITEM_NONMETAL		(S)
#define ITEM_NOLOCATE		(T)
#define ITEM_MELT_DROP		(U)
#define ITEM_HAD_TIMER		(V)
#define ITEM_SELL_EXTRACT	(W)
#define ITEM_BURN_PROOF		(Y)
#define ITEM_NOUNCURSE		(Z)
#define ITEM_DRAGGABLE		(aa)
#define ITEM_UMBRAL		(bb)
#define ITEM_DREAM		(cc)
#define ITEM_CHIMERICAL		(dd)
#define ITEM_HIDDEN		(ee)

/*
 * Transient and additional Extra Object Flags (extra2)
 */
#define OBJ_BURNING		(A)
#define OBJ_SLOW_BURN		(B)
#define OBJ_FAST_BURN		(C)
#define OBJ_NO_DECAY		(D)
#define OBJ_PACKAGED		(E)
#define OBJ_WATERPROOF		(F)
#define OBJ_ATTUNED		(G)
#define OBJ_FURNISHING		(H)
#define OBJ_INTANGIBLE		(I)
#define OBJ_KEEP		(J)

/*
 * Wear flags.
 * Used in #OBJECTS.
 */
#define ITEM_TAKE		(A)
#define ITEM_WEAR_FINGER	(B)
#define ITEM_WEAR_NECK		(C)
#define ITEM_WEAR_BODY		(D)
#define ITEM_WEAR_HEAD		(E)
#define ITEM_WEAR_LEGS		(F)
#define ITEM_WEAR_FEET		(G)
#define ITEM_WEAR_HANDS		(H)
#define ITEM_WEAR_UNDERTOP	(I)
#define ITEM_WEAR_UNDERPANTS	(J)
#define ITEM_WEAR_ABOUT		(K)
#define ITEM_WEAR_WAIST		(L)
#define ITEM_WEAR_WRIST		(M)
#define ITEM_WIELD		(N)
#define ITEM_HOLD		(O)
#define ITEM_WEAR_FACE		(P)
#define ITEM_WEAR_ARMS		(Q)
#define ITEM_TWO_HANDS		(R)
#define ITEM_WEAR_HIP		(S)
#define ITEM_WEAR_ARMPIT	(T)

/* weapon class */
#define WEAPON_EXOTIC		0
#define WEAPON_FIREARM		1
#define WEAPON_BLADE		2
#define WEAPON_BLUNT		3
#define WEAPON_WHIP		4
#define WEAPON_GRAPPLE		5

/* weapon types */
#define WEAPON_FLAMING		(A)
#define WEAPON_FROST		(B)
#define WEAPON_VAMPIRIC		(C)
#define WEAPON_TWO_HANDS	(D)
#define WEAPON_SHOCKING		(E)
#define WEAPON_POISON		(F)

/* gate flags */
#define GATE_NORMAL_EXIT	(A)
#define GATE_NOCURSE		(B)
#define GATE_GOWITH		(C)
#define GATE_BUGGY		(D)
#define GATE_RANDOM		(E)
#define GATE_CAR_DOOR		(F)

/* furniture flags */
#define STAND_AT		(A)
#define STAND_ON		(B)
#define STAND_IN		(C)
#define SIT_AT			(D)
#define SIT_ON			(E)
#define SIT_IN			(F)
#define REST_AT			(G)
#define REST_ON			(H)
#define REST_IN			(I)
#define SLEEP_AT		(J)
#define SLEEP_ON		(K)
#define SLEEP_IN		(L)
#define PUT_AT			(M)
#define PUT_ON			(N)
#define PUT_IN			(O)
#define PUT_INSIDE		(P)




/*
 * Apply types (for affects).
 * Used in #OBJECTS.
 */
#define APPLY_NONE		      0
#define APPLY_STR		      1
#define APPLY_DEX		      2
#define APPLY_STA		      3
#define APPLY_CHA		      4
#define APPLY_MAN		      5
#define APPLY_APP		      6
#define APPLY_PER		      7
#define APPLY_INT		      8
#define APPLY_WIT		      9
#define APPLY_GHB		     10
#define APPLY_RBPG		     11
#define APPLY_WILLPOWER		     12
#define APPLY_CONSCIENCE	     13
#define APPLY_SELF_CONTROL	     14
#define APPLY_COURAGE		     15
#define APPLY_PAIN		     16
#define APPLY_ANGER		     17
#define APPLY_FEAR		     18
#define APPLY_FRENZY		     19
#define APPLY_HEALTH		     20
#define APPLY_DICE		     21
#define APPLY_DIFFICULTY	     22
#define APPLY_SEX		     23
#define APPLY_AGE		     24
#define APPLY_HEIGHT		     25
#define APPLY_WEIGHT		     26
#define APPLY_SPELL_AFFECT	     27
#define APPLY_SPECIAL		     28
#define APPLY_ROOM_LIGHT	     29
#define APPLY_GENERATION	     30
#define APPLY_SKILL		     31
#define APPLY_SIZE		     32
#define APPLY_DERANGEMENT	     32

/*
 * Values for containers (value[1]).
 * Used in #OBJECTS.
 */
#define CONT_CLOSEABLE		      1
#define CONT_PICKPROOF		      2
#define CONT_CLOSED		      4
#define CONT_LOCKED		      8
#define CONT_PUT_ON		     16

/*
 * Well known room virtual numbers.
 * Defined in #ROOMS.
 */
#define ROOM_VNUM_LIMBO             2
#define ROOM_VNUM_POLICE_STATION    3000
#define ROOM_VNUM_JAIL              3004
#define ROOM_VNUM_START             10000

/*
 * Room flags.
 * Used in #ROOMS.
 */
#define ROOM_DARK		(A)
#define ROOM_BANK		(B)
#define ROOM_NO_MOB		(C)
#define ROOM_INDOORS		(D)
#define ROOM_JAMMIN		(E)
#define ROOM_NOPHONE		(F)
#define ROOM_HOME		(G)

#define ROOM_OPENDOORS		(H)
#define ROOM_ELEVATOR		(I)
#define ROOM_TRAIN		(J)
#define ROOM_STOP		(K)
#define ROOM_SOLITARY		(L)
#define ROOM_PRIVATE		(M)
#define ROOM_NO_RECALL		(N)
#define ROOM_IMP_ONLY		(O)
#define ROOM_ADMIN		(P)
#define ROOM_POLLUTED		(Q)
#define ROOM_CORRUPTED		(R)
#define ROOM_VEHICLE		(S)
#define ROOM_NOWHERE		(T)



/*
 * Directions.
 * Used in #ROOMS.
 */
#define DIR_NORTH		      0
#define DIR_EAST		      1
#define DIR_SOUTH		      2
#define DIR_WEST		      3
#define DIR_UP			      4
#define DIR_DOWN		      5



/*
 * Exit flags.
 * Used in #ROOMS.
 */
#define EX_ISDOOR		      (A)
#define EX_CLOSED		      (B)
#define EX_LOCKED		      (C)
#define EX_PICKPROOF		      (F)
#define EX_NOPASS		      (G)
#define EX_EASY			      (H)
#define EX_HARD			      (I)
#define EX_INFURIATING		      (J)
#define EX_NOCLOSE		      (K)
#define EX_NOLOCK		      (L)
#define EX_HIDDEN		      (M)
#define EX_WINDOW		      (N)
#define EX_BROKEN		      (O)
#define EX_UMBRA		      (P)
#define EX_DREAM		      (Q)
#define EX_NOBREAK		      (R)
#define EX_FALLING		      (S)
#define EX_NARROW		      (T)
#define EX_SEALED		      (U)
#define EX_SQUEEZE		      (V)
#define EX_KEYCHOMP		      (W)

/*
 * Sector types.
 * Used in #ROOMS.
 */
#define SECT_INSIDE		      0
#define SECT_STREET		      1
#define SECT_FIELD		      2
#define SECT_FOREST		      3
#define SECT_HILLS		      4
#define SECT_MOUNTAIN		      5
#define SECT_WATER_SWIM		      6
#define SECT_WATER_NOSWIM	      7
#define SECT_POLLUTED		      8
#define SECT_AIR		      9
#define SECT_DESERT		     10
#define SECT_CITY		     11
#define SECT_MAX		     12



/*
 * Equpiment wear locations.
 * Used in #RESETS.
 */
#define WEAR_NONE		     -1
#define WEAR_FACE		      0
#define WEAR_FINGER_L		      1
#define WEAR_FINGER_R		      2
#define WEAR_NECK		      3
#define WEAR_UNDERTOP		      4
#define WEAR_BODY		      5
#define WEAR_HEAD		      6
#define WEAR_UNDERPANTS		      7
#define WEAR_LEGS		      8
#define WEAR_FEET		      9
#define WEAR_HANDS		     10
#define WEAR_ARMS		     11
#define WEAR_ABOUT		     12
#define WEAR_WAIST		     13
#define WEAR_WRIST_L		     14
#define WEAR_WRIST_R		     15
#define WEAR_WIELD		     16
#define WEAR_HOLD		     17
#define WEAR_RESET		     18
#define WEAR_LEFT_HIP		     19
#define WEAR_RIGHT_HIP		     20
#define WEAR_LEFT_PIT		     21
#define WEAR_RIGHT_PIT		     22
#define WEAR_HOME		     23
#define WEAR_SKINFLAP		     24
#define MAX_WEAR		     25


/*
 * Influence Types
 */
#define INFL_CHURCH		0
#define INFL_CRIMINAL	1
#define INFL_ECONOMIC	2
#define INFL_JUDICIAL	3
#define INFL_MEDIA		4
#define INFL_POLICE		5
#define INFL_POLITICAL	6
#define INFL_SCIENTIFIC	7
#define MAX_INFL		8
/*#define INFL_		() */

/*
 * Tags for backgrounds without other grouping.
 */
#define ALLIES          0
#define PUREBREED       1
#define CONTACTS        2
#define FETISHES        3
#define GENERATION      4
#define HERD            5
#define MENTOR          6
#define PASTLIFE        7
#define RESOURCES       8
#define RETAINERS       9
#define MAX_BACKGROUND		10
/*
 * Status array macros.
 */
#define MAX_STATUS	3
#define CLAN_STATUS	10
#define FAME_STATUS	11
#define RACE_STATUS 12

#define MAX_BG		(MAX_BACKGROUND + MAX_STATUS)


/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (End of this section ... stop here)                   *
 *                                                                         *
 ***************************************************************************/

/*
 * Conditions.
 */
#define COND_DRUNK		      0
#define COND_FULL		      1
#define COND_THIRST		      2
#define COND_HUNGER		      3
#define COND_HIGH		      4
#define COND_TRIPPING		      5
#define COND_FRENZY                   6
#define COND_ANGER                    7
#define COND_PAIN                     8
#define COND_FEAR                     9
#define COND_SANITY                  10


/*
 * Positions.
 */
#define P_DEAD		      0
#define P_MORT		      1
#define P_PETRF               2
#define P_TORPOR	      3
#define P_INCAP		      4
#define P_STUN		      5
#define P_SLEEP		      6
#define P_REST		      7
#define P_SIT		      8
#define P_FIGHT		      9
#define P_STAND		      10


/*
 * ACT bits for players.
 */
#define PLR_IS_NPC		(A)		/* Don't EVER set.	*/

/* RT auto flags */
#define PLR_AUTOEXIT		(C)

/* RT personal flags */
#define PLR_HOLYLIGHT		(D)
#define PLR_NOFOLLOW		(E)

/* penalty flags */
#define PLR_PERMIT		(F)
#define PLR_LOG			(G)
#define PLR_DENY		(H)
#define PLR_FREEZE		(I)
#define PLR_COLOUR		(J)

/* RP flag */
#define PLR_RP_OK		(K)

/* Elder flag */
#define PLR_ELDER		(L)

/* Arresting Flag */
#define PLR_ARREST		(M)

/* Email active Flag */
#define PLR_EMAIL		(N)


/* RT comm flags -- may be used on both mobs and chars */
#define COMM_QUIET              (A)
#define COMM_DEAF            	(B)
#define COMM_NOWIZ              (C)
#define COMM_NOAUCTION          (D)
#define COMM_NOGOSSIP           (E)
#define COMM_NOQUESTION         (F) /* UNUSED */
#define COMM_NOCLAN		(G)
#define COMM_OLC		(H)
#define COMM_DISCREET		(I)
#define COMM_JAM_LEAD		(J)
#define COMM_JAMMIN		(K)
#define COMM_NOPHONE		(Q)
#define COMM_BUILDER		(R)
#define COMM_OOC_OFF		(S)
#define COMM_THINK_ON		(X)
#define COMM_AFK		(Y)

/* display flags */
#define COMM_COMPACT		(L)
#define COMM_BRIEF		(M)
#define COMM_PROMPT		(N)
#define COMM_COMBINE		(O)
#define COMM_TELNET_GA		(P)

/* penalties */
#define COMM_NOEMOTE		(T)
#define COMM_NOSHOUT		(U)
#define COMM_NOTELL		(V)
#define COMM_NOCHANNELS		(W)

/* Unusual COMM flags */
#define COMM_SURVEY		(Z)
#define COMM_TIPS		(aa)

/* WIZnet flags */
#define WIZ_ON			(A)
#define WIZ_LOGINS		(C)
#define WIZ_SITES		(D)
#define WIZ_LINKS		(E)
#define WIZ_DEATHS		(F)
#define WIZ_RESETS		(G)
#define WIZ_MOBDEATHS		(H)
#define WIZ_FLAGS		(I)
#define WIZ_PENALTIES		(J)
#define WIZ_SECURE		(M)
#define WIZ_SWITCHES		(N)
#define WIZ_SNOOPS		(O)
#define WIZ_RESTORE		(P)
#define WIZ_LOAD		(Q)
#define WIZ_NEWBIE		(R)
#define WIZ_PREFIX		(S)
#define WIZ_SPAM		(T)
#define WIZ_COMM		(U)
#define WIZ_STAFF_COMM		(V)
#define WIZ_LOG (W)

/*
 * All the storytelling stuff
 */
typedef struct plot_data PLOT_DATA;
typedef struct event_data EVENT_DATA;
typedef struct script_data SCRIPT_DATA;
typedef struct actor_data ACTOR_DATA;

struct plot_data
{
    int vnum;
    char *author;
    char *title;
    EVENT_DATA *event_list;
    long races;
    PLOT_DATA *next;
};

struct actor_data
{
    int mob;
    ACTOR_DATA *next;
};

struct event_data
{
    int vnum;
    PLOT_DATA *plot;
    char *author;
    char *title;
    long races;
    int loc;
    int stop;
    ACTOR_DATA *actors;
    SCRIPT_DATA *script_list;
    EVENT_DATA *next;
    EVENT_DATA *next_in_plot;
};

struct script_data
{
    int vnum;
    EVENT_DATA *event;
    char *author;
    int actor;
    char *trigger;
    char *reaction;
    int  delay;
    int  active;
    int  first_script;
    SCRIPT_DATA *next;
    SCRIPT_DATA *next_in_event;
};

/*
 * Prototype for a mob.
 * This is the in-memory version of #MOBILES.
 */
struct	mob_index_data
{
    MOB_INDEX_DATA *	next;
    SPEC_FUN *		spec_fun;
    SHOP_DATA *		pShop;
    AREA_DATA *		area; /*OLC*/
    MPROG_LIST *	mprogs;
    SCRIPT_DATA *	triggers;
    sh_int		vnum;
    sh_int		group;
    sh_int		count;
    sh_int		killed;
    int			level;
    char *		player_name;
    char *		short_descr;
    char *		long_descr;
    char *		description;
    long		act;
    long		act2;
    long		affected_by;
    long		affected_by2;
    sh_int		hit[3];
    sh_int		health;
    sh_int 		dam_type;
    long		off_flags;
    long		imm_flags;
    long		res_flags;
    long		vuln_flags;
    sh_int		start_pos;
    sh_int		default_pos;
    sh_int		sex;
    sh_int		race;
    long		wealth;
    long		form;
    long		parts;
    sh_int		size;
    char *		material;
    sh_int		stat[MAX_STATS];
    ABILITIES		ability[MAX_ABIL];
    sh_int		disc[MAX_DISC];
    sh_int		virtues[MAX_VIRTUES];
    sh_int		influences[MAX_INFL];
    sh_int		backgrounds[MAX_BG];
    sh_int		GHB;
    sh_int		RBPG;
    sh_int		willpower;
    long		mprog_flags;
};



/*
 * New combat crap.
 * Coded by One Thousand Monkeys
 * Using ideas first suggested by KaVir
 */

#define MOVE_NONE	-1
#define MOVE_STAND	0
#define MOVE_FLIP	1
#define MOVE_ROLL	2
#define MOVE_JUMP	3
#define MOVE_LBLOW	4
#define MOVE_RBLOW	5
#define MOVE_LGRAB	6
#define MOVE_RGRAB	7
#define MOVE_KICK	8
#define MOVE_SKICK	9
#define MOVE_BITE	10
#define MOVE_LBLOCK	11
#define MOVE_RBLOCK	12
#define MOVE_FBLOCK	13
#define MOVE_SPIN	14
#define MOVE_SWEEP	15
#define MOVE_SHOOT	16
#define MOVE_DODGE	17
#define MOVE_TOUCH	18
#define MOVE_TONGUE	19

typedef enum
{
   AP_PART_NONE,
   AP_LEFT_HAND,
   AP_RIGHT_HAND,
   AP_HEAD,
   AP_LEGS,
   AP_FEET,
   AP_BODY,
   AP_MAX
} at_part_type;

typedef struct combat_move COMBAT_DATA;

struct combat_move
{
    COMBAT_DATA 	*next;
    int 		move;
    CHAR_DATA		*victim;
    at_part_type	part;
};

/*
 * New quest system stuff.
 */
typedef struct quest_data QUEST_DATA;
struct quest_data
{
    long		quest_flags;
    CHAR_DATA		*victim;
    CHAR_DATA		*aggressor;
    CHAR_DATA		*questor;
    int			time_limit;
    QUEST_DATA		*next;
    int			quest_type;
    int			state;
    OBJ_DATA		*obj;
};

/* Quest types */
#define Q_NONE		0
#define Q_COURIER	1
#define Q_HITMAN	2
#define Q_THIEF		3
#define Q_BODYGUARD	4
#define Q_GUARD		5
#define Q_RESCUE	6
#define MAX_QUESTS	7

/* Quest flags */
#define Q_SAFE		A	/* No chance of player death		  */
#define Q_EASY		B
#define Q_HARD		C
#define Q_PUZZLE	D	/* Limit info on quest			  */
#define Q_ARREST	E	/* Raises warrant level, chance of arrest */
#define Q_DEADLY	F	/* Quest has good risk of player death	  */
#define Q_HUNTER	G	/* Set hunter on player on quest complete */
#define Q_ACCEPTED	H

/* Goal types */
#define G_NONE		0
#define G_ITEM		1
#define G_HITMAN	2
#define G_GROUP		3
#define G_MONEY		4
#define G_GUARD		5
#define G_STATUS	6
#define G_TERRITORY	7
#define MAX_GOALS	8


/*
 * Electoral System Data Structures.
 */
typedef struct nominee_data NOMINEE_DATA;

struct nominee_data
{
    char *name;
    int  votes;
};

extern NOMINEE_DATA vote_tally[MAX_OFFICES][MAX_NOMINEES];
extern time_t next_election_result;
extern time_t last_election_result;
extern char *incumbent_mayor;
extern char *incumbent_alder[MAX_ALDERMEN];
extern char *incumbent_judge[MAX_JUDGES];
extern char *incumbent_pchief;

/*
 * Stuff for artificial intelligence.
 */
typedef struct react_data REACT_DATA;
typedef struct react REACT;
typedef struct persona PERSONA;
typedef struct memory MEMORY;

#define MAX_ATTITUDE 10

struct react_data {
    char *reaction;
    int priority;
    REACT_DATA *next;
};

struct react {
    char *trig;
    REACT_DATA *react;
    int attitude;
    PERSONA *persona;
    REACT *next_in_matrix_loc;
    REACT *next;
};

struct persona {
    REACT *matrix[MAX_ATTITUDE];
    char *name;
    int vnum;
    PERSONA *next;
};

/* memory settings */
#define MEM_CUSTOMER	A
#define MEM_SELLER	B
#define MEM_HOSTILE	C
#define MEM_AFRAID	D
#define MEM_FRIEND	E
/*#define MEM_	F
#define MEM_	G*/

/* memory for mobs */
struct memory {
    char	*name;
    long 	reaction;
    time_t 	when;
    int		attitude;
    MEMORY	*boss;
    MEMORY	*enemy;
    MEMORY	*next;
};


struct liquid_data
{
    int liquid;			/* Particular liquid			*/
    int quantity;		/* Quantity of liquid			*/
    LIQUID_DATA *next;		/* Next in list				*/
};


/*
 * Flags for new liquid objects.
 */
#define	LIQ_VAMPIRE	(A)
#define LIQ_WEREWOLF	(B)
#define LIQ_FAERIE	(C)
#define LIQ_MAGE	(D)
#define LIQ_ANIMAL	(E)
#define LIQ_HUMAN	(F)
#define LIQ_MUTAGEN	(G)


/*
 * One character (PC or NPC).
 */
struct	char_data
{
    CHAR_DATA *		next;
    CHAR_DATA *		next_in_room;
    CHAR_DATA *		next_listener;
    CHAR_DATA *		master;
    CHAR_DATA *		leader;
    CHAR_DATA *		rp_leader;
    CHAR_DATA *		summonner;
    CHAR_DATA *		fighting;
    COMBAT_DATA *	combo;
    int			combo_success;
    int			act_points;
    CHAR_DATA *		reply;
    CHAR_DATA *		calling;
    int			on_phone;
    CHAR_DATA *		pet;
    CHAR_DATA *		mprog_target;
    MEM_DATA *		memory;
    SPEC_FUN *		spec_fun;
    SCRIPT_DATA *	script;
    char *		aifile;
    MOB_INDEX_DATA *	pIndexData;
    DESCRIPTOR_DATA *	desc;
    AFFECT_DATA *	affected;
    NOTE_DATA *		pnote;
    OBJ_DATA *		carrying;
    OBJ_DATA *		on;
    ROOM_INDEX_DATA *	in_room;
    ROOM_INDEX_DATA *	was_in_room;
    ROOM_INDEX_DATA *	listening;
    AREA_DATA *		zone;
    PC_DATA *		pcdata;
    GEN_DATA *		gen_data;
    QUEST_DATA *	quest;
    PERSONA *		personality;
    MEMORY *		memories;
    EXTRA_DESCR_DATA *	extra_descr;
    TRAIT_DATA *	traits;
    int			attitude;
    char *		laston;
    char *		name;
    char *		surname;
    char *		married;
    char *		oldname;
    char *		nature;
    char *		demeanor;
    char *		colours[2];
    long		id;
    char *		sire;
    sh_int		version;
    sh_int		shape;
    int			warrants;
    int			hunter_vis;
    bool		hunted;
    char *		short_descr;
    char *		long_descr;
    char *		description;
    char *		alt_name;
    char *		alt_long_descr;
    char *		alt_description;
    char *		switch_desc;
    char *		prompt;
    char *		prefix;
    char *		ghouled_by;
    char *		profession;
    long		employer;
    int			pospts;
    int			markup;
    int			xp_job_acts;
    int			job_skill[MAX_JOB_SKILLS];
    sh_int              trust;
    sh_int		group;
    int			clan;
    int			clan_powers[3];
    sh_int		sex;
    sh_int		race;
    sh_int		auspice;
    sh_int		breed;
    int			played;
    int			lines;  /* for the pager */
    time_t		logon;
    sh_int		timer;
    sh_int		wait;
    sh_int		daze;
    sh_int		condition	[11];
    sh_int		torpor_timer;
    sh_int		power_timer;
    sh_int		infl_timer;
    sh_int		blood_timer;
    int			ooc_xp_time;
    sh_int		ooc_xp_count;
    sh_int		combat_flag;
    int			jump_timer;
    sh_int		jump_dir;
    int			falldam;
    int			herd_timer;
    int			dollars;
    int			cents;
    long		bank_account;
    sh_int		health;
    sh_int		agghealth;
    sh_int		RBPG;
    sh_int		max_RBPG;
    sh_int		GHB;
    sh_int		max_GHB;
    sh_int		willpower;
    sh_int		max_willpower;
    sh_int		virtues[MAX_VIRTUES];
    sh_int		max_virtues[MAX_VIRTUES];
    sh_int		max_traits[2];
    int			gen;
    int			char_age;
    int			balance;
    sh_int		disc[MAX_DISC];
    int			exp;
    int			oocxp;
    int			xpgift;
    ABILITIES		ability[MAX_ABIL];
    int			learned[MAX_SKILL];
    long		act;
    long		act2;
    long		comm;   /* RT added to pad the vector */
    long		wiznet; /* wiz stuff */
    char *		ignore;
    long		imm_flags;
    long		res_flags;
    long		vuln_flags;
    sh_int		invis_level;
    sh_int		incog_level;
    long		affected_by;
    long		affected_by2;
    sh_int		position;
    sh_int		carry_weight;
    sh_int		carry_number;
    sh_int		saving_throw;
    sh_int		armor[3];
    /* stats */
    sh_int		perm_stat[MAX_STATS];
    sh_int		mod_stat[MAX_STATS];
    /* parts stuff */
    long		form;
    long		parts;
    sh_int		size;
    char *		material;
    /* mobile stuff */
    long		off_flags;
    sh_int		damage[3];
    sh_int		dam_type;
    sh_int		start_pos;
    sh_int		default_pos;
    sh_int		mprog_delay;
    long		powers[MAX_POWERS];
    char *		to_learn;
    sh_int		train_success;
    sh_int		dice_mod;
    sh_int		diff_mod;
    sh_int		bg_timer;
    int			bg_count;
    sh_int		influences[MAX_INFL];
    sh_int		backgrounds[MAX_BG];
    long		plr_flags;
    int			concede;
    int			drives[2];
    int			stock_ticker;
    STOCKS		*stocks;
    char *		pack;
    int			totem_attitudes[2];
    SURVEY_DATA*	survey;
    int			home;
    ROOM_INDEX_DATA*	rooms[MAX_OWNED_ROOMS];
    int			BUILDER_MODE;
    /* Email stuff. */
    char *		email_addr;
    long		email_lock;
    int			current_tip;
    int			riteacts[MAX_RITE_STEPS];
    int			ritepoint;
    LIQUID_DATA	*	coating;
};



/*
 * Data which only PC's have.
 */
struct	pc_data
{
    PC_DATA *		next;
    BUFFER * 		buffer;
    char *		acct_login;
    char *		pwd;
    char *		bamfin;
    char *		bamfout;
    char *		title;
    char *		rpok_string;
    time_t              last_note;
    sh_int		true_sex;
    int			points;
    bool              	full_reset;
    bool              	confirm_delete;
    char *		alias[MAX_ALIAS];
    char * 		alias_sub[MAX_ALIAS];
    int			security; /*OLC - Builder security level */
    time_t		last_vote;
    char *		ignore_reject;
    char *		block_join;
    /* @@@@@ insert character linked list here for accounts */
};


/* Data for generating characters -- only used during generation */
struct gen_data
{
    GEN_DATA	*next;
    int		skill_chosen[MAX_SKILL];
    int		stat_dots[3];
    int		skill_dots[3];
    int		virtue_dots;
    int		points_chosen;
    int		freebies;
};

/* Trait stuff */
#define TRAIT_QUIRK		0
#define TRAIT_MERIT		1
#define TRAIT_FLAW		2
#define TRAIT_DERANGEMENT	3

struct trait_data {
	TRAIT_DATA	*next;
	int			type;
	char		*qualifier;
	char		*detail;
	int			value;
};

/* Totems and Packs */
#define	TOTEM_RESPECT	(A)
#define	TOTEM_WAR	(B)
#define	TOTEM_WISDOM	(C)
#define	TOTEM_CUNNING	(D)
#define	NOT_TOTEM	(E)

/* Data for packs. */
struct pack_data
{
    PACK_DATA	*next;
    char	*name;
    long	totem;
};

/* Data for organisations. (Player defined clans.) */
struct org_data
{
    ORG_DATA	*next;
    char	*name;
    char	*who_name;
    char	*file_name;
    long	step_point;
    long	funds;
    long	default_auths;
    int		type;
    char	*leader;
    char	*applicants;
    char	*commands[5];
    char	*title[6];
    char	*races;
    ORGMEM_DATA *members;
};

#define ORG_ANARCHISTIC		0
#define ORG_DICTATORIAL		1
#define ORG_DEMOCRATIC		2
#define ORG_NO_LEADER		3

/* Data for Org members. */
struct orgmem_data
{
    ORGMEM_DATA	*next;
    char	*name;
    int		status;
    long	auth_flags;
    int		status_votes;
};

#define AUTH_INDUCT	(A)
#define AUTH_BOOT	(B)
#define AUTH_BUILD	(C)
#define AUTH_CMD1	(D)
#define AUTH_CMD2	(E)
#define AUTH_CMD3	(F)
#define AUTH_CMD4	(G)
#define AUTH_CMD5	(H)
#define AUTH_MEMBERS	(I)
#define AUTH_VOTE	(J)
#define AUTH_STATUS	(K)
#define AUTH_NEWVOTE	(L)

/* Data for surveys. */
struct survey_data
{
    SURVEY_DATA	*next;
    bool	valid;
    bool	bnew;
    bool	under_edit;
    long	ID;
    char	*name;
    char	*question;
    char	*answers[MAX_VOTE_OPTIONS];
    int		tally[MAX_VOTE_OPTIONS];
    char	*voters[MAX_VOTE_OPTIONS];
    char	*file;
    int		type;
};

#define SURV_SURVEY	0
#define SURV_VOTE	1
#define SURV_STATUS	2


/*
 * Liquids.
 */
#define LIQ_WATER        0

struct	liq_type
{
    char *	liq_name;
    char *	liq_color;
    sh_int	liq_affect[5];
};



/*
 * Extra description data for a room or object.
 */
struct	extra_descr_data
{
    EXTRA_DESCR_DATA *next;	/* Next in list                       */
    char *keyword;              /* Keyword in look/examine            */
    char *description;          /* What to see                        */
    int  timer;                 /* For use with the "mark" command    */
    int  attribute;             /* Ability rolled to see desc (use)   */
    int  ability;               /* Ability rolled to see desc (use)   */
    int  difficulty;            /* Difficulty to see desc (use)       */
    int  successes;             /* Successes needed to see desc (use) */
};


/*
 * Prototype for an object.
 */
struct	obj_index_data
{
    OBJ_INDEX_DATA *	next;
    EXTRA_DESCR_DATA *	extra_descr;
    AFFECT_DATA *	affected;
    AREA_DATA *		area; /* OLC */
    char *		name;
    char *		short_descr;
    char *		description;
    char *		full_desc;
    char *		company; /* New Object Driven Stock Market Prices */
    sh_int		vnum;
    sh_int		reset_num;
    char *		material;
    sh_int		item_type;
    int			extra_flags;
    int			extra2;
    int			wear_flags;
    sh_int 		condition;
    sh_int		count;
    sh_int		weight;
    int			cost;
    int			value[6];
    int			fetish_flags;
    int			fetish_level;
    int			fetish_target;
    char *		to_user_none;
    char *		to_user_self;
    char *		to_user_other;
    char *		to_room_none;
    char *		to_room_self;
    char *		to_room_other;
    char *		to_vict_other;
    char *		to_room_used;
    char *		to_user_used;
    int			uses;
    int			quality;
    LIQUID_DATA	*	coating;
};



/*
 * One object.
 */
struct	obj_data
{
    OBJ_DATA *		next;
    OBJ_DATA *		next_content;
    OBJ_DATA *		contains;
    OBJ_DATA *		in_obj;
    OBJ_DATA *		on;
    CHAR_DATA *		carried_by;
    EXTRA_DESCR_DATA *	extra_descr;
    AFFECT_DATA *	affected;
    OBJ_INDEX_DATA *	pIndexData;
    ROOM_INDEX_DATA *	in_room;
    bool		enchanted;
    char *	        owner;
    char *		name;
    char *		short_descr;
    char *		description;
    char *		full_desc;
    sh_int		item_type;
    int			extra_flags;
    int			extra2;
    int			wear_flags;
    sh_int		wear_loc;
    sh_int		weight;
    int			cost;
    sh_int 		condition;
    char *		material;
    sh_int		timer;
    int			value	[5];
    long		fetish_flags;
    int			fetish_level;
    int			fetish_target;
    char *		to_user_none;
    char *		to_user_self;
    char *		to_user_other;
    char *		to_room_none;
    char *		to_room_self;
    char *		to_room_other;
    char *		to_vict_other;
    char *		to_room_used;
    char *		to_user_used;
    int			uses;
    long		reset_loc;
    int			BUILDER_MODE;
    int			quality;
    LIQUID_DATA	*	coating;
};



/*
 * Exit data.
 */
struct	exit_data
{
	union
	{
		ROOM_INDEX_DATA *	to_room;
		sh_int			vnum;
	} u1;
	union
	{
		ROOM_INDEX_DATA *	to_room;
		sh_int			vnum;
	} jumpto;

	int			exit_info;
	sh_int		key;
	char *		keyword;
	char *		description;
	EXIT_DATA *		next; /*OLC*/
	int			rs_flags; /*OLC*/
	int			orig_door; /*OLC*/
};



/*
 * Reset commands:
 *   '*': comment
 *   'M': read a mobile 
 *   'O': read an object
 *   'P': put object in object
 *   'G': give object to mobile
 *   'E': equip object to mobile
 *   'D': set state of door
 *   'R': randomize room exits
 *   'F': furniture reset
 *   'S': stop (end of list)
 */

/*
 * Area-reset definition.
 */
struct	reset_data
{
    RESET_DATA *	next;
    char		command;
    sh_int		arg1;
    sh_int		arg2;
    sh_int		arg3;
    sh_int		arg4;
};



/*
 * Area definition.
 */
struct	area_data
{
    AREA_DATA *		next;
    RESET_DATA *	reset_first;
    RESET_DATA *	reset_last;
    char *		file_name;
    char *		name;
    char *		credits;
    sh_int		age;
    sh_int		nplayer;
    sh_int		low_range;
    sh_int		high_range;
    sh_int 		min_vnum;
    sh_int		max_vnum;
    bool		empty;
    char *		builders; /* OLC - Listing of */
    int			vnum; /* OLC - Area vnum */
    int			area_flags; /*OLC*/
    int			security; /* OLC - values between 1-9 */
    int			pricemod; /* Multiplier for home purchase prices. */
};



/*
 * Room type.
 */
struct	room_index_data
{
    ROOM_INDEX_DATA *	next;
    CHAR_DATA *		people;
    CHAR_DATA *		listeners;
    OBJ_DATA *		contents;
    EXTRA_DESCR_DATA *	extra_descr;
    AREA_DATA *		area;
    EXIT_DATA *		exit	[6];
    EXIT_DATA * 	old_exit[6];
    RESET_DATA *	reset_first; /*OLC*/
    RESET_DATA *	reset_last; /*OLC*/
    AFFECT_DATA *	affects;
    EVENT_DATA *	event;
    char *		name;
    char *		description;
    char *		uname;
    char *		udescription;
    char *		dname;
    char *		ddescription;
    char *		owner;
    sh_int		vnum;
    int			room_flags;
    sh_int		light;
    sh_int		sector_type;
    sh_int		heal_rate;
    sh_int 		mana_rate;
    int			clan;
    int			car;
    int			stops[MAX_CAR_STOPS];
    int			going_to;
    int			at_stop;
    int			car_pause;
};



/*
 * Types of attacks.
 * Must be non-overlapping with spell/skill types,
 * but may be arbitrary beyond that.
 */
#define TYPE_UNDEFINED               -1
#define TYPE_HIT                     1000



/*
 *  Target types.
 */
#define TAR_IGNORE		    0
#define TAR_CHAR_OFFENSIVE	    1
#define TAR_CHAR_DEFENSIVE	    2
#define TAR_CHAR_SELF		    3
#define TAR_OBJ_INV		    4
#define TAR_OBJ_CHAR_DEF	    5
#define TAR_OBJ_CHAR_OFF	    6
#define TAR_ROOM		    7
#define TAR_ROOM_CHAR		    8

#define TARGET_CHAR		    0
#define TARGET_OBJ		    1
#define TARGET_ROOM		    2
#define TARGET_NONE		    3


/*
 * MOBprog definitions
 */
#define TRIG_ACT       (A)
#define TRIG_BRIBE     (B)
#define TRIG_DEATH     (C)
#define TRIG_ENTRY     (D)
#define TRIG_FIGHT     (E)
#define TRIG_GIVE      (F)
#define TRIG_GREET     (G)
#define TRIG_GRALL     (H)
#define TRIG_KILL      (I)
#define TRIG_HPCNT     (J)
#define TRIG_RANDOM    (K)
#define TRIG_SPEECH    (L)
#define TRIG_EXIT      (M)
#define TRIG_EXALL     (N)
#define TRIG_DELAY     (O)
#define TRIG_SURR      (P)
#define TRIG_ANTIOBJ   (Q)
#define TRIG_POWERDISP (R)

struct mprog_list
{
    int				trig_type;
    char *			trig_phrase;
    sh_int			vnum;
    char *			code;
    MPROG_LIST *	next;
};

struct mprog_code
{
    sh_int             vnum;
    char *             code;
    MPROG_CODE *       next;
};


/*
 * Power type declaration
 */
typedef struct power_type POWER_TYPE;
typedef struct power_effect POWER_EFFECT;

/*
 * Limitation flags for powers.
 */
#define LIM_ALONE		A	/* Power only works when alone */


/*
 * Powers as the first free-form command prototype.
 * Requires power_fun_table to exist and be appropriately populated.
 * Requires limitation flag handler as pre-processor to power func.
 * Requires availability handler as pre-processor to power func.
 * Requires re-write of teaching and learning functions.
 * @@@@@
 */
struct	power_type
{
    int		vnum;			/* Command vnum - weird huh?	*/
    char *	name;			/* Name of power		*/
    char *	command;		/* Activating command		*/
    int		power_fun_id;		/* Function pointer table id	*/
    int		limits;			/* Limitation flags		*/

    int		available[PC_RACE];	/* Available for race?		*/
    int		stat_avail_type;	/* Stat category for availability */
    int		stat_id;		/* Specific stat in category	*/
    int		stat_lev;		/* Stat value needed		*/

    sh_int	target;			/* Legal targets		*/
    sh_int	minimum_position;	/* Position for user		*/
    sh_int	slot;			/* Slot for #OBJECT loading	*/
    sh_int	beats;			/* Waiting time after use	*/

    char *	taught_by;		/* Who can teach?		*/

    char *	noun_damage;		/* Damage message		*/
    char *	msg_off;		/* Wear off message		*/
    char *	msg_obj;		/* Wear off message for obects	*/
    char *	msg_fail_user;		/* Fail message to user		*/
    char *	msg_fail_room;		/* Fail message to room		*/
    char *	msg_fail_vict;		/* Fail message to vict		*/
    char *	msg_botch_user;		/* Botch message to user	*/
    char *	msg_botch_room;		/* Botch message to room	*/
    char *	msg_go_user;		/* Success message to user	*/
    char *	msg_go_room;		/* Success message to room	*/
    char *	msg_go_vict;		/* Success message to vict	*/
    int		valid;
    POWER_TYPE *	next;
};


/*
 * Skills include spells as a particular case.
 */
struct	skill_type
{
    char *	name;			/* Name of skill		*/
    int		available[PC_RACE];	/* Available for race?		*/
    SPELL_FUN *	spell_fun;		/* Spell pointer (for spells)	*/
    sh_int	target;			/* Legal targets		*/
    sh_int	minimum_position;	/* Position for caster / user	*/
    sh_int *	pgsn;			/* Pointer to associated gsn	*/
    sh_int	slot;			/* Slot for #OBJECT loading	*/
    sh_int	beats;			/* Waiting time after use	*/
    char *	noun_damage;		/* Damage message		*/
    char *	msg_off;		/* Wear off message		*/
    char *	msg_obj;		/* Wear off message for obects	*/
};


/* Abilities: GENERAL (extra one fer mobs) */
#define GENERAL			0

/* Abilities: Talents */
#define ACTING			0
#define ALERTNESS		1
#define ATHLETICS		2
#define BRAWL			3
#define DODGE			4
#define EMPATHY			5
#define EXPRESSION		6
#define INTIMIDATION		7
#define KENNING			8
#define LEADERSHIP		9
#define PRIMAL_URGE		10
#define STREETWISE		11
#define SUBTERFUGE		12

/* Abilities: Skills */
#define ANIMAL_KEN		13
#define CRAFTS			14
#define DRIVE			15
#define ETIQUETTE		16
#define FIREARMS		17
#define MELEE			18
#define PERFORMANCE		19
#define REPAIR			20
#define LARCENY			21
#define STEALTH			22
#define SURVIVAL		23
/* Abilities: Knowledges */
#define ACADEMICS		24
#define COMPUTER		25
#define ENIGMAS			26
#define FINANCE			27
#define INVESTIGATION	28
#define LAW				29
#define TECHNOLOGY		30
#define MEDICINE		31
#define MYTHLORE		32
#define OCCULT			33
#define POLITICS		34
#define RITUALS			35
#define SCIENCE			36

#define MAX_ABILITY		38

#define DISC_ANIMALISM		0
#define DISC_AUSPEX			1
#define DISC_CELERITY		2
#define DISC_DOMINATE		3
#define DISC_FORTITUDE		4
#define DISC_OBFUSCATE		5
#define DISC_PRESENCE		6
#define DISC_POTENCE		7
#define DISC_PROTEAN		8
#define DISC_THAUMATURGY	9
#define DISC_NECROMANCY		10
#define DISC_OBEAH			11
#define DISC_CHIMERSTRY		12
#define DISC_MELPOMINEE		13
#define DISC_QUIETUS		14
#define DISC_SERPENTIS		15
#define DISC_THANATOSIS		16
#define DISC_DEMENTATION	17
#define DISC_OBTENEBRATION	18
#define DISC_VICISSITUDE	19
#define DISC_MISSING		20

#define DISC_TRIBE		0
#define DISC_AUSPICE		1
#define DISC_BREED		2

#define DISC_REALM		0
#define DISC_ART		1
#define DISC_ACTOR		2

/*
 * Utility macros.
 */
#define IS_VALID(data)		((data) != NULL && (data)->valid)
#define VALIDATE(data)		((data)->valid = TRUE)
#define INVALIDATE(data)	((data)->valid = FALSE)
#define UMIN(a, b)		((a) < (b) ? (a) : (b))
#define UMAX(a, b)		((a) > (b) ? (a) : (b))
#define URANGE(a, b, c)		((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
#define LOWER(c)		((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))
#define UPPER(c)		((c) >= 'a' && (c) <= 'z' ? (c)+'A'-'a' : (c))
#define IS_SET(flag, bit)	((flag) & (bit))
#define SET_BIT(var, bit)	((var) |= (bit))
#define REMOVE_BIT(var, bit)    ((var) &= ~(bit))
#define IS_NULLSTR(str)                ((str) == NULL || (str)[0] == '\0')
#define ENTRE(min,num,max)     ( ((min) < (num)) && ((num) < (max)) )
#define CHECK_POS(a, b, c)     {                       \
                               (a) = (b);              \
                               if ( (a) < 0 )          \
                               bug( "[*****] BUG: CHECK_POS : " c " == %d < 0", a ); \
                               }

#define IS_NIGHT()		(time_info.hour < 7 || time_info.hour > 19)

/*
 * BUILDER_MODE macros.
 */
#define IS_BUILDERMODE(data)	((data) != NULL && (data)->BUILDER_MODE)
#define SET_BUILDERMODE(data)	((data)->BUILDER_MODE = TRUE)
#define CLEAR_BUILDERMODE(data)	((data)->BUILDER_MODE = FALSE)

/*
 * Character macros.
 */
#define IS_NPC(ch)		(IS_SET((ch)->act, ACT_IS_NPC))
#define IS_ADMIN(ch)		(get_trust(ch) >= LEVEL_IMMORTAL)
#define IS_TRUSTED(ch,level)	(get_trust((ch)) >= (level))
#define IS_AFFECTED(ch, sn)	(IS_SET((ch)->affected_by, (sn)))
#define IS_AFFECTED2(ch, sn)	(IS_SET((ch)->affected_by2, (sn)))
#define IS_ROOM_AFFECTED(ch, sn)	(IS_SET((ch)->affects, (sn)))
#define IS_INTELLIGENT(ch)	(IS_SET((ch)->act, ACT_INTELLIGENT))

#define GET_AGE(ch)		((int) (17 + ((ch)->played \
				    + current_time - (ch)->logon )/72000))

#define IS_AWAKE(ch)		(ch->position > P_SLEEP)
#define GET_AC(ch,type)		((ch)->armor[type] + ( IS_AWAKE(ch) \
			? dex_app[get_curr_stat(ch,STAT_DEX)].defensive : 0 ))  

#define IS_OUTSIDE(ch)		(!IS_SET(				    \
				    (ch)->in_room->room_flags,		    \
				    ROOM_INDOORS))

#define NEAR_FRENZY(ch)		(ch->condition[COND_FRENZY] > 60)
#define IS_DRUNK(ch)		(ch->condition[COND_DRUNK] > 15)
#define IS_HIGH(ch)		(ch->condition[COND_HIGH] > 15)
#define IS_TRIPPING(ch)		(ch->condition[COND_TRIPPING] > 15)
#define IS_WANTED(ch)		(ch->warrants >= 50)

#define WAIT_STATE(ch, npulse)	((ch)->wait = UMAX((ch)->wait, (npulse)))
#define DAZE_STATE(ch, npulse)  ((ch)->daze = UMAX((ch)->daze, (npulse)))
#define get_carry_weight(ch)	((ch)->carry_weight + (ch)->cents/50 +  \
						      (ch)->dollars * 2 / 100)

#define HAS_TRIGGER(ch,trig)   (IS_SET((ch)->pIndexData->mprog_flags,(trig)))
#define HAS_SCRIPT(ch)		   ((ch)->pIndexData->triggers != NULL)
#define IS_SWITCHED( ch )       ( ch->desc && ch->desc->original )
#define IS_BUILDER(ch, Area)   ( !IS_NPC(ch) && !IS_SWITCHED( ch ) &&    \
                               ( ch->pcdata->security >= Area->security  \
                               || strstr( Area->builders, ch->name )     \
                               || strstr( Area->builders, "All" ) ) )

#define IS_REJECTED(ch, vict)   (strstr(vict->pcdata->ignore_reject, ch->name))

#define IS_BLOCKED(ch, vict)    (strstr(vict->pcdata->block_join, ch->name ) )

#define IS_VAMPIRE(ch)		(ch->race == race_lookup("vampire"))
#define IS_WEREWOLF(ch)     (ch->race == race_lookup("werewolf"))
#define IS_FAERIE(ch)       (ch->race == race_lookup("faerie"))

/* macro.c */
bool IS_ATTRIB_AVAILABLE	args( (int race, int sn) );
bool IS_CLASS_AVAILABLE		args( (int race, int sn) );
bool IS_ALONE			args( (CHAR_DATA *ch) );
bool IS_GROUP_ALONE		args( (CHAR_DATA *ch) );
bool IS_ANIMAL			args( (CHAR_DATA *ch) );
bool SAME_PLANE			args( (CHAR_DATA *ch, CHAR_DATA *vch) );
bool can_comprehend		args( (CHAR_DATA *ch, CHAR_DATA *vch) );
int  affect_level		args( (CHAR_DATA *ch, int sn) );
void add_warrant		args( (CHAR_DATA *ch, int warrant, bool caught) );
bool part_in_use		args( (CHAR_DATA *ch, int move) );

#define act(format,ch,arg1,arg2,type,all_planes)\
		act_new((format),(ch),(arg1),(arg2),(type),P_REST,(all_planes))

/*
 * Object macros.
 */
#define CAN_WEAR(obj, part)	(IS_SET((obj)->wear_flags,  (part)))
#define IS_OBJ_STAT(obj, stat)	(IS_SET((obj)->extra_flags, (stat)))
#define IS_WEAPON_STAT(obj,stat)(IS_SET((obj)->value[4],(stat)))
#define WEIGHT_MULT(obj)	((obj)->item_type == ITEM_CONTAINER ? \
	(obj)->value[4] : 100)

/*
 * quest.c
 */
void gen_quest			args( ( bool forced, CHAR_DATA *vch) );

/*
 * Description macros.
 */
extern const char *hallucination_table[MAX_HALLUC];
#define PERS(ch, looker)	( can_see( (looker), (ch) ) ?		\
				( IS_NPC(ch) ? (ch)->short_descr	\
				: (ch)->name ) : "someone" )
#define ALT_PERS(ch, looker)	( can_see( (looker), (ch) ) ?		\
				( IS_NPC(ch) ? (ch)->short_descr	\
				: (ch)->alt_name ) : "someone" )

/*
 * Structure for a social in the socials table.
 */
struct	social_type
{
    char *    name;
    char *    char_no_arg;
    char *    others_no_arg;
    char *    char_found;
    char *    others_found;
    char *    vict_found;
    char *    char_not_found;
    char *      char_auto;
    char *      others_auto;
};


/* Global Variables */
extern	AREA_DATA *	area_first;
extern	AREA_DATA *	area_last;
extern	PLOT_DATA *	plot_list;
extern	EVENT_DATA *	event_list;
extern	SCRIPT_DATA *	script_first;
extern	SCRIPT_DATA *	script_last;
extern	PERSONA *	persona_first;
extern	PERSONA *	persona_last;
extern	REACT *		react_first;
extern	REACT *		react_last;
extern	REACT_DATA *	react_data_first;
extern	REACT_DATA *	react_data_last;

extern	int	top_affect;
extern	int	top_area;
extern	int	top_ed;
extern	int	top_exit;
extern	int	top_help;
extern	int	top_tip;
extern	int	top_mob_index;
extern	int	top_obj_index;
extern	int	top_mprog_index;
extern	int	top_reset;
extern	int	top_room;
extern	int	top_shop;

extern	int	top_vnum_mob;
extern	int	top_vnum_obj;
extern	int	top_vnum_room;

extern	char	str_empty	[1];

extern	MOB_INDEX_DATA *	mob_index_hash	[MAX_KEY_HASH];
extern	OBJ_INDEX_DATA *	obj_index_hash	[MAX_KEY_HASH];
extern	ROOM_INDEX_DATA *	room_index_hash	[MAX_KEY_HASH];



/*
 * Global variables.
 */
extern		HELP_DATA	  *	help_list;
extern		HELP_DATA	  *	tip_list;
extern		SHOP_DATA	  *	shop_list;
extern		CHAR_DATA	  *	char_list;
extern		DESCRIPTOR_DATA   *	descriptor_list;
extern		OBJ_DATA	  *	object_list;
extern		QUEST_DATA	  *	quest_list;
extern		MPROG_CODE	  *	mprog_list;

extern		char			bug_buf		[];
extern		time_t			current_time;
extern		bool			fLogAll;
extern		FILE *			fpReserve;
extern		char			log_buf		[];
extern		TIME_INFO_DATA		time_info;
extern		WEATHER_DATA		weather_info;
extern		int			tax_rate;
extern		ORG_DATA	  *	org_list;
extern		PACK_DATA	  *	pack_list;
extern		bool			MOBtrigger;

/*
 * OS-dependent declarations.
 * These are all very standard library functions,
 *   but some systems have incomplete or non-ansi header files.
 */
#if	defined(_AIX)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(apollo)
int	atoi		args( ( const char *string ) );
void *	calloc		args( ( unsigned nelem, size_t size ) );
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(hpux)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(linux)
//char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(macintosh)
#define NOCRYPT
#ifdef unix
#undef	unix
#endif
#endif

#if	defined(MIPS_OS)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(MSDOS)
#define NOCRYPT
#ifdef unix
#undef	unix
#endif
#endif

#if	defined(NeXT)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(sequent)
char *	crypt		args( ( const char *key, const char *salt ) );
int	fclose		args( ( FILE *stream ) );
int	fprintf		args( ( FILE *stream, const char *format, ... ) );
int	fread		args( ( void *ptr, int size, int n, FILE *stream ) );
int	fseek		args( ( FILE *stream, long offset, int ptrname ) );
void	perror		args( ( const char *s ) );
int	ungetc		args( ( int c, FILE *stream ) );
#endif

#if	defined(sun)
char *	crypt		args( ( const char *key, const char *salt ) );
int	fclose		args( ( FILE *stream ) );
int	fprintf		args( ( FILE *stream, const char *format, ... ) );
#if	defined(SYSV)
siz_t	fread		args( ( void *ptr, size_t size, size_t n, 
			    FILE *stream) );
#elif !defined(__SVR4)
int	fread		args( ( void *ptr, int size, int n, FILE *stream ) );
#endif
int	fseek		args( ( FILE *stream, long offset, int ptrname ) );
void	perror		args( ( const char *s ) );
int	ungetc		args( ( int c, FILE *stream ) );
#endif

#if	defined(ultrix)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif



/*
 * The crypt(3) function is not available on some operating systems.
 * In particular, the U.S. Government prohibits its export from the
 *   United States to foreign countries.
 * Turn on NOCRYPT to keep passwords in plain text.
 */
#define NOCRYPT
#if	defined(NOCRYPT)
#define crypt(s1, s2)	(s1)
#endif



/*
 * Data files used by the server.
 *
 * AREA_LIST contains a list of areas to boot.
 * All files are read in completely at bootup.
 * Most output files (bug, idea, typo, shutdown) are append-only.
 *
 * The NULL_FILE is held open so that we have a stream handle in reserve,
 *   so players can go ahead and telnet to all the other descriptors.
 * Then we close it whenever we need to open a file (e.g. a save file).
 */
#if defined(macintosh)
#define PLAYER_DIR	""			/* Player files	*/
#define PLAYER_BACKUP_DIR  ""  /* Player backup files */
#define NPC_DIR		""			/* NPC files	*/
#define AREA_DIR	""			/* Area files	*/
#define ORG_DIR		""			/* Org files	*/
#define SURVEY_DIR	""			/* Survey files	*/
#define LOG_DIR		""			/* Log files	*/
#define TEMP_FILE	"mudtmp"
#define NULL_FILE	"proto.are"		/* To reserve one stream */

#elif  defined(MSDOS)
#define PLAYER_DIR	"../player/"		/* Player files */
#define PLAYER_BACKUP_DIR  "../player/backup/"  /* Player backup files */
#define NPC_DIR		"../npc/"		/* NPC files	*/
#define AREA_DIR	"../area/"		/* Area files	*/
#define ORG_DIR		"../orgs/"		/* Org files	*/
#define SURVEY_DIR	"../data/survey/"	/* Survey files	*/
#define LOG_DIR		"../log/"		/* Log files	*/
#define TEMP_FILE	"mudtmp"
#define NULL_FILE	"nul"			/* To reserve one stream */

#elif  defined(unix)
#define PLAYER_DIR      "../player/"        	/* Player files */
#define PLAYER_BACKUP_DIR  "../player/backup/"  /* Player backup files */
#define NPC_DIR		"../npc/"		/* NPC files	*/
#define AREA_DIR	"../area/"		/* Area files	*/
#define ORG_DIR		"../data/orgs/"		/* Org files	*/
#define SURVEY_DIR	"../data/survey/"	/* Survey files	*/
#define LOG_DIR		"../log/"		/* Log files	*/
#define TEMP_FILE	"../player/mudtmp"
#define NULL_FILE	"/dev/null"		/* To reserve one stream */
#else
#define PLAYER_DIR      "../player/"            /* Player files */
#define PLAYER_BACKUP_DIR  "../player/backup/"  /* Player backup files */
#define NPC_DIR     "../npc/"       /* NPC files    */
#define AREA_DIR    "../area/"      /* Area files   */
#define ORG_DIR     "../data/orgs/"     /* Org files    */
#define SURVEY_DIR  "../data/survey/"   /* Survey files */
#define LOG_DIR     "../log/"       /* Log files    */
#define TEMP_FILE   "../player/mudtmp"
#define NULL_FILE   "/dev/null"     /* To reserve one stream */
#endif

#define AREA_LIST		"area.lst"				/* List of areas */
#define ORG_LIST		"org.lst"				/* List of orgs */
#define BUG_FILE		"../data/bugs.txt"				/* For 'bug' and bug() */
#define SHUTDOWN_FILE	"shutdown.txt"			/* For 'shutdown' */
#define BAN_FILE		"ban.txt"				/* For 'ban' */
#define NOTE_FILE		"../data/notes.txt"		/* For 'notes' */
#define BG_FILE			"../data/bg.txt"		/* For 'backgrounds' */
#define KNOW_FILE		"../data/know.txt"		/* For 'backgrounds' */
#define NEWS_FILE		"../data/news.txt"		/* For 'articles' */
#define PAPER_FILE		"../data/paper.txt"		/* For 'newspapers' */
#define STOCK_FILE		"../data/stock.txt"		/* For 'stocks' */
#define VOTES_FILE      "../data/votes.txt"     /* for 'votes' */

#define GLOBAL_XML_IN	"glob"			/* RSS global activity log */
#define GLOBAL_XML_OUT	"../../public_html/global.xml" /* RSS output for globals */

#if defined(windows)
#ifndef SHUT_RD
#define SHUT_RD SD_RECEIVE
#endif
#ifndef SHUT_WR
#define SHUT_WR SD_SEND
#endif
#ifndef SHUT_RDWR
#define SHUT_RDWR SD_BOTH
#endif
#endif


// Log types
#define LOG_CRIT 1
#define LOG_ERR 2
#define LOG_BUG 4
#define LOG_SECURITY 8
#define LOG_CONNECT 16
#define LOG_GAME 32
#define LOG_COMMAND 64
#define LOG_ALL 127 // All the others added up

#define SMTP_SITE	""
#define SMTP_PORT	""

#define UPDATE_FILE	"../../public_html/news/data/news.txt"

/*
 * Our function prototypes.
 * One big lump ... this is every function in Merc.
 */
#define CD	CHAR_DATA
#define MID	MOB_INDEX_DATA
#define OD	OBJ_DATA
#define OID	OBJ_INDEX_DATA
#define RID	ROOM_INDEX_DATA
#define SF	SPEC_FUN
#define AD	AFFECT_DATA
#define MPC	MPROG_CODE

/* act_comm.c */
void  	check_sex	args( ( CHAR_DATA *ch) );
void	add_follower	args( ( CHAR_DATA *ch, CHAR_DATA *master ) );
void	stop_follower	args( ( CHAR_DATA *ch ) );
void	die_follower	args( ( CHAR_DATA *ch ) );
bool	is_same_group	args( ( CHAR_DATA *ach, CHAR_DATA *bch ) );
void	do_yell		args( ( CHAR_DATA *ch, char *argument) );
void	do_say		args( ( CHAR_DATA *ch, char *argument) );
bool	HAS_PHONE	args( ( CHAR_DATA * ch ) );
bool	OWNS_PHONE	args( ( CHAR_DATA * ch ) );
void printf_to_char (CHAR_DATA *ch, char *fmt, ...);

/* act_enter.c */
RID  *get_random_room   args ( (CHAR_DATA *ch) );

/* act_info.c */
void	set_title	args( ( CHAR_DATA *ch, char *title ) );
void	do_peek		args( ( CHAR_DATA *ch, char *argument ) );

/* act_move.c */
void	move_char	args( ( CHAR_DATA *ch, int door, bool follow ) );
void	do_visible	args( ( CHAR_DATA *ch, char * argument) );
void	do_hide		args( ( CHAR_DATA *ch, char * argument) );
void walk_to_summonner	args( ( CHAR_DATA *ch ) );
void	arrest_player	args( ( CHAR_DATA *cop ) );
int	find_dir	args( ( CHAR_DATA *ch, char *arg ) );
void free_help(HELP_DATA *pHelp);
void free_mob_index( MOB_INDEX_DATA *pMob );
void free_obj_index( OBJ_INDEX_DATA *pObj );
void free_room_index( ROOM_INDEX_DATA *pRoom );
void free_area( AREA_DATA *pArea );
void free_shop( SHOP_DATA *pShop );
void free_plot(PLOT_DATA *plot);
void free_event(EVENT_DATA *event);


/* act_obj.c */
void    wear_obj        args( ( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace,
				char *wear_loc ) );
void    get_obj         args( ( CHAR_DATA *ch, OBJ_DATA *obj,
                            OBJ_DATA *container ) );
void	do_remove	args( ( CHAR_DATA *ch, char *argument ) );

/* act_wiz.c */
void wiznet		args( (char *string, CHAR_DATA *ch, OBJ_DATA *obj,
			       long flag, long flag_skip, int min_level ) );
void copyover_recover	args( ( void ) );
void do_echo		args( (CHAR_DATA *ch, char *argument) );

/* alias.c */
void 	substitute_alias args( (DESCRIPTOR_DATA *d, char *input) );

/* ban.c */
bool	check_ban	args( ( char *site, int type) );


/* comm.c */
void	show_string	args( ( struct descriptor_data *d, char *input) );
void	close_socket	args( ( DESCRIPTOR_DATA *dclose ) );
void	write_to_buffer	args( ( DESCRIPTOR_DATA *d, const char *txt,
			    int length ) );
void	send_to_char	args( ( const char *txt, CHAR_DATA *ch ) );
void	send_to_room	args( ( const char *txt, ROOM_INDEX_DATA *room ) );
void	page_to_char	args( ( const char *txt, CHAR_DATA *ch ) );
void	act		args( ( const char *format, CHAR_DATA *ch,
			    const void *arg1, const void *arg2, int type ) );
void	act_new		args( ( const char *format, CHAR_DATA *ch,
			    const void *arg1, const void *arg2, int type,
			    int min_pos, int all_planes) );
int	colour		args( ( char type, CHAR_DATA *ch, char *string ) );
void	reincarnate	args( ( CHAR_DATA *ch, int freebies ) );
bool write_to_descriptor args( (int desc, char *txt, int length) );
void	logfmt		args( ( char * fmt, ... ) );

/* db.c */
char *	print_flags	args( ( int flag ));
void	boot_db		args( ( void ) );
void	area_update	args( ( void ) );
CD *	create_mobile	args( ( MOB_INDEX_DATA *pMobIndex ) );
void	clone_mobile	args( ( CHAR_DATA *parent, CHAR_DATA *clone) );
OD *	create_object	args( ( OBJ_INDEX_DATA *pObjIndex ) );
void	clone_object	args( ( OBJ_DATA *parent, OBJ_DATA *clone ) );
void	clear_char	args( ( CHAR_DATA *ch ) );
char *	get_extra_descr	args( ( const char *name, EXTRA_DESCR_DATA *ed ) );
MID *	get_mob_index	args( ( int vnum ) );
OID *	get_obj_index	args( ( int vnum ) );
RID *	get_room_index	args( ( int vnum ) );
MPC *	get_mprog_index	args( ( int vnum ) );
char	fread_letter	args( ( FILE *fp ) );
int	fread_number	args( ( FILE *fp ) );
long 	fread_flag	args( ( FILE *fp ) );
char *	fread_string	args( ( FILE *fp ) );
char *  fread_string_eol args(( FILE *fp ) );
void	fread_to_eol	args( ( FILE *fp ) );
char *	_fread_word	args( ( FILE *fp, const char *file, const char *function, int line ) );
#define fread_word(fp) _fread_word(fp, __FILE__, __FUNCTION__, __LINE__)
char *	fread_chunk	args( ( FILE *fp, char delimiter ) );
long	flag_convert	args( ( char letter) );
void	openReserve args ( (void ) );
void	closeReserve args ( (void ) );

int	number_fuzzy	args( ( int number ) );
int	number_range	args( ( int from, int to ) );
int	number_percent	args( ( void ) );
int	number_door	args( ( void ) );
int	number_bits	args( ( int width ) );
long     number_mm       args( ( void ) );
int	dice		args( ( int number, int size ) );
int	interpolate	args( ( int level, int value_00, int value_32 ) );
void	smash_tilde	args( ( char *str ) );
//bool	str_cmp		args( ( const char *astr, const char *bstr ) );
bool	str_prefix	args( ( const char *astr, const char *bstr ) );
bool	str_infix	args( ( const char *astr, const char *bstr ) );
bool	str_suffix	args( ( const char *astr, const char *bstr ) );
char *	capitalize	args( ( const char *str ) );
void	append_file	args( ( CHAR_DATA *ch, char *file, char *str ) );
void	bug		args( ( const char *str, int param ) );
void log_string args( (int type, const char *fmt, ... ) );
void	tail_chain	args( ( void ) );
char *	allcaps		args( ( const char *str ) );
void	log_to_file	args( ( char *file, char *extension,
						const char *string ) );
const char *Upper args( (const char *str) );
const char *Lower args( (const char *str) );
char *_str_dup args( (const char *str, const char *file, const char *function, int line) );
char* strmove args(char* s1,const char* s2) 
const char *Format args( (const char *fmt, ...) );
char *CapitalSentence args( (const char *str) );


/* effect.c */
void	acid_effect	args( (void *vo, int level, int dam, int target) );
void	cold_effect	args( (void *vo, int level, int dam, int target) );
void	fire_effect	args( (void *vo, int level, int dam, int target) );
void	poison_effect	args( (void *vo, int level, int dam, int target) );
void	shock_effect	args( (void *vo, int level, int dam, int target) );

/* lookup.c */
int	abil_lookup	args( (const char * name, CHAR_DATA * ch) );
int	stat_lookup	args( (const char * name, CHAR_DATA * ch) );
int	get_points	args( (CHAR_DATA *ch) );
int	exit_lookup	args( (CHAR_DATA *ch, const char *dir) );
int	virtue_lookup	args( (const char *name) );
int	disc_lookup	args( ( CHAR_DATA *ch, const char *dir) );

/* fight.c */
void    end_life        args( ( CHAR_DATA *ch ) );
int	dice_rolls	args( ( CHAR_DATA *ch, int number, int difficulty ) );
int	range		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
int	get_abil	args( ( CHAR_DATA *ch, int TSK ) );
void    do_mob_hit      args( ( CHAR_DATA *ch, CHAR_DATA *victim, int weapon ) );
void	deaded_char	args( ( CHAR_DATA *ch ) );
void	strike		args( ( CHAR_DATA *ch, CHAR_DATA *vch ) );
COMBAT_DATA * go_combo  args( ( CHAR_DATA *ch ) );
void   shooting args( ( CHAR_DATA *ch, CHAR_DATA *victim, BODY_DATA *target ) );
int	do_soak		args( ( CHAR_DATA *victim, int damage, int aggravated) );

/* fight_old.c */
void	violence_update	args( ( void ) );
void	multi_hit	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
int	damage		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
			int dt, int bclass, bool show, int agg, int combo ) );
void	update_pos	args( ( CHAR_DATA *victim, int agg ) );
void	stop_fighting	args( ( CHAR_DATA *ch, bool fBoth ) );
void	dam_message	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt, bool immune, int combo ) );
void	set_fighting	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	one_hit		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
void	raw_kill	args( ( CHAR_DATA *victim, int absolute ) );

/* handler.c */
AD  	*affect_find args( (AFFECT_DATA *paf, int sn));
void	affect_check	args( (CHAR_DATA *ch, int where, int vector) );
int	count_users	args( (OBJ_DATA *obj) );
void 	deduct_cost	args( (CHAR_DATA *ch, int cost) );
void	affect_enchant	args( (OBJ_DATA *obj) );
int 	check_immune	args( (CHAR_DATA *ch, int dam_type) );
int	liq_lookup	args( ( const char *name) );
int 	material_lookup args( ( const char *name) );
char 	*weapon_name	args( ( int weapon_Type) );
int	item_lookup	args( ( const char *name) );
char	*item_name	args( ( int item_type) ); 
int	attack_lookup	args( ( const char *name) );
int	race_lookup	args( ( const char *name) );
int	auspice_lookup	args( ( const char *name) );
int	breed_lookup	args( ( const char *name) );
long	wiznet_lookup	args( ( const char *name) );
int	clan_lookup	args( ( const char *name) );
bool	is_clan		args( (CHAR_DATA *ch) );
bool	is_same_clan	args( (CHAR_DATA *ch, CHAR_DATA *victim));
int	get_skill	args( ( CHAR_DATA *ch, int sn ) );
int	get_weapon_sn	args( ( CHAR_DATA *ch ) );
int	get_weapon_skill args(( CHAR_DATA *ch, int sn ) );
int     get_age         args( ( CHAR_DATA *ch ) );
void	reset_char	args( ( CHAR_DATA *ch )  );
int	get_trust	args( ( CHAR_DATA *ch ) );
int	get_curr_stat	args( ( CHAR_DATA *ch, int stat ) );
int 	get_max_train	args( ( CHAR_DATA *ch, int stat ) );
int	can_carry_n	args( ( CHAR_DATA *ch ) );
int	can_carry_w	args( ( CHAR_DATA *ch ) );
bool	is_name		args( ( char *str, char *namelist ) );
bool	is_exact_name	args( ( char *str, char *namelist ) );
void	affect_to_char	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	affect_to_obj	args( ( OBJ_DATA *obj, AFFECT_DATA *paf ) );
void	affect_to_room	args( ( ROOM_INDEX_DATA *room, AFFECT_DATA *paf ) );
void	affect_remove	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	affect_remove_obj args( (OBJ_DATA *obj, AFFECT_DATA *paf ) );
void	affect_remove_room args( (ROOM_INDEX_DATA *room, AFFECT_DATA *paf ) );
void	affect_strip	args( ( CHAR_DATA *ch, int sn ) );
bool	is_affected	args( ( CHAR_DATA *ch, int sn ) );
bool	is_affected_obj	args( ( OBJ_DATA *obj, int sn ) );
void	affect_join	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	char_from_room	args( ( CHAR_DATA *ch ) );
void	char_to_room	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex ) );
void	char_from_listeners	args( ( CHAR_DATA *ch ) );
void	char_to_listeners args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex ) );
void	obj_to_char	args( ( OBJ_DATA *obj, CHAR_DATA *ch ) );
void	obj_from_char	args( ( OBJ_DATA *obj ) );
int	apply_ac	args( ( OBJ_DATA *obj, int iWear, int type ) );
OD *	get_eq_char	args( ( CHAR_DATA *ch, int iWear ) );
void	equip_char	args( ( CHAR_DATA *ch, OBJ_DATA *obj, int iWear ) );
void	unequip_char	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
int	count_obj_list	args( ( OBJ_INDEX_DATA *obj, OBJ_DATA *list ) );
void	obj_from_room	args( ( OBJ_DATA *obj ) );
void	obj_to_room	args( ( OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex ) );
void	obj_to_obj	args( ( OBJ_DATA *obj, OBJ_DATA *obj_to ) );
void	obj_from_obj	args( ( OBJ_DATA *obj ) );
void	extract_obj	args( ( OBJ_DATA *obj ) );
void	extract_char	args( ( CHAR_DATA *ch, bool fPull ) );
CD *	get_char_dir	args( ( CHAR_DATA *ch, int dir, char *argument ) );
CD *	get_char_room	args( ( CHAR_DATA *ch, char *argument ) );
CD *	get_char_oroom	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *room,
				char *argument ) );
CD *	get_char_world	args( ( CHAR_DATA *ch, char *argument ) );
CD *	get_char_area	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_type	args( ( OBJ_INDEX_DATA *pObjIndexData ) );
OD *	get_obj_list	args( ( CHAR_DATA *ch, char *argument,
			    OBJ_DATA *list ) );
OD *	get_obj_carry	args( ( CHAR_DATA *ch, char *argument, 
			    CHAR_DATA *viewer ) );
OD *	get_obj_wear	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_here	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_world	args( ( CHAR_DATA *ch, char *argument ) );
OD *	create_money	args( ( int dollars, int cents ) );
int	get_obj_number	args( ( OBJ_DATA *obj ) );
int	get_obj_weight	args( ( OBJ_DATA *obj ) );
int	get_true_weight	args( ( OBJ_DATA *obj ) );
bool	room_is_dark	args( ( ROOM_INDEX_DATA *pRoomIndex ) );
bool	room_is_dim	args( ( ROOM_INDEX_DATA *pRoomIndex ) );
bool	is_room_owner	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *room) );
bool	room_is_private	args( ( ROOM_INDEX_DATA *pRoomIndex ) );
bool	can_see		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	can_see_main	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	can_see_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
bool   can_see_obj_main args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
bool	can_see_room	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex) );
bool	can_drop_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
void	trigger_test	args( ( const char *string, CD *ch, CD *vict ) );
bool	is_online	args( ( char *argument ) );
bool	pc_in_char_list	args( ( char *argument ) );
CD	*get_player	args( ( char *argument ) );
/*CD *	get_char_area	args( ( CHAR_DATA *ch, char *argument ) );
*/
char    *health_string  	args( ( CHAR_DATA *ch ) );
char	*appearance_string	args( ( CHAR_DATA *ch ) );
char	*gender_string		args( ( CHAR_DATA *ch ) );
bool	IS_FEARFUL			args( ( CHAR_DATA *ch ) );
int		factorial			args( ( int n ) );
int		compound			args( ( int n ) );
bool    IS_NATURAL			args( ( CHAR_DATA *ch ) );
int		count_words			args( ( char *string ) );
char	*HTML_TOKEN_STRIP	args( ( char *string ) );
bool	bare_earth			args( ( ROOM_INDEX_DATA *rm ) );
int	gen_room	args( ( CHAR_DATA *ch, ORG_DATA *org ) );
int	gen_exit	args( ( CHAR_DATA *ch, int door, int value ) );
char *	insult		args( ( CHAR_DATA *ch ) );
CD *	gen_hunter	args( ( CHAR_DATA *ch ) );
bool	IS_HUNTED	args( ( CHAR_DATA *ch ) );
int	QUALITY_OF_WORK args( ( int fail ) );
bool	is_in_group	args( ( CHAR_DATA *ch ) );
bool	add_voter	args( ( SURVEY_DATA *survey, CHAR_DATA *ch ) );
void	extract_mem	args( ( ORG_DATA *org, ORGMEM_DATA *mem ) );
void	clear_orgs	args( ( CHAR_DATA *ch) );
int	current_dots	args( ( CHAR_DATA *ch) );
int	xp_cost_mod	args( ( CHAR_DATA *ch, int cost, int newdot) );
int	trait_count	args( ( CHAR_DATA *ch, int type) );
CD *	sat_on		args( ( OBJ_DATA *obj ) );
void	obj_to_plane	args( ( OBJ_DATA *obj, int plane ) );
bool	LOOKS_DIFFERENT	args( ( CHAR_DATA *ch ) );
bool	coat_obj_in_liquid args( ( OBJ_DATA *obj, OBJ_DATA *liquid ) );
bool	coat_char_in_liquid args( ( CHAR_DATA *ch, OBJ_DATA *liquid ) );
bool	coat_obj_in_liquid_by_name args( ( OBJ_DATA *obj, char *liquid ) );
bool	coat_char_in_liquid_by_name args( ( CHAR_DATA *ch, char *liquid ) );
OD * create_puddle_from_obj args( ( OBJ_DATA *obj, int vnum, bool scrap_obj ) );
OD * create_puddle_from_liq args( ( char *liq, int vnum, int volume ) );
int	has_trait	args( ( CHAR_DATA *ch, TRAIT_DATA *p ) );

void	cleanse_builder_stuff args( (CHAR_DATA *ch ) );
int	count_multi_args args( ( char *argument, char *cEnd ) );
void purgeExtractedWorldData  args ( (void) );


/* interp.c */
void	interpret	args( ( CHAR_DATA *ch, char *argument ) );
void	mob_interpret	args( ( CHAR_DATA *ch, char *argument ) );
bool	is_number	args( ( char *arg ) );
int	number_argument	args( ( char *argument, char *arg ) );
int	mult_argument	args( ( char *argument, char *arg) );
char *	one_argument	args( ( char *argument, char *arg_first ) );
char *	exact_argument	args( ( char *argument, char *arg_first ) );
char *	one_chunk	args( ( char *argument, char *arg_first, char cEnd ) );
char *	one_line	args( ( char *argument, char *arg ) );

// crazy like pinapple - David Simmerson
#define str_cmp(astr, bstr) __strcmp(astr, bstr, __FILE__, __FUNCTION__, __LINE__)

/*Thanks goto markanth for the BufPrintf function, and for the singly linked code.
 LINK_SINGLE/UNLINK_SINGLE are awesome functions for handling your singly linked
 lists, i use them for all of mine, courtesy of the firstmud mudbase. */
#define UNLINK_SINGLE(pdata,pnext,type,list)   \
		do                                         \
		{                                          \
			assert(pdata); \
			if (list == pdata)                      \
			{                                       \
				list = pdata->pnext;                \
			}                                       \
			else                                    \
			{                                       \
				type *prev;                         \
				for (prev = list; prev != NULL; prev = prev->pnext) \
				{                                   \
					if (prev->pnext == pdata)       \
					{                               \
						prev->pnext = pdata->pnext;  \
						break;                      \
					}                               \
				}                                   \
				if (prev == NULL)                   \
				{                                   \
					logfmt(#pdata " not found in " #list "."); \
				}                                   \
			}                                       \
		} while(0)

#define LINK_SINGLE(pdata,pnext,list) \
		do \
		{  \
			assert(pdata); \
			pdata->pnext = list;  \
			list = pdata;  \
		}  \
		while (0)


/* magic.c */
int	find_spell	args( ( CHAR_DATA *ch, const char *name) );
int	skill_lookup	args( ( const char *name ) );
int	slot_lookup	args( ( int slot ) );
void	obj_cast_spell	args( ( int sn, int level, CHAR_DATA *ch,
				    CHAR_DATA *victim, OBJ_DATA *obj ) );
bool	saves_spell	args( ( int level, CHAR_DATA *victim, int dam_type ) );
void init_clan_powers	args( (CHAR_DATA *ch, int clan) );
void	clear_rite	args( (CHAR_DATA *ch) );

/* mob_prog.c */
void   program_flow    args( ( sh_int vnum, char *source, CHAR_DATA *mob,
			CHAR_DATA *ch, const void *arg1, const void *arg2 ) );
void   mp_act_trigger  args( ( char *argument, CHAR_DATA *mob, CHAR_DATA *ch,
                               const void *arg1, const void *arg2, int type ) );
bool   mp_percent_trigger args( ( CHAR_DATA *mob, CHAR_DATA *ch,
                               const void *arg1, const void *arg2, int type ) );
void   mp_bribe_trigger  args( ( CHAR_DATA *mob, CHAR_DATA *ch, int amount ) );
bool   mp_exit_trigger   args( ( CHAR_DATA *ch, int dir ) );
void   mp_give_trigger   args( ( CHAR_DATA *mob, CHAR_DATA *ch, OBJ_DATA *obj));
void   mp_greet_trigger  args( ( CHAR_DATA *ch ) );
void   mp_hprct_trigger  args( ( CHAR_DATA *mob, CHAR_DATA *ch ) );
void   mp_antiobj_trigger  args( (CHAR_DATA *mob,CHAR_DATA *ch,OBJ_DATA *obj) );

/* mob_cmds.c */
void   mprog_interpret args( ( CHAR_DATA *ch, char *argument ) );


/* save.c */
void	save_char_obj	args( ( CHAR_DATA *ch ) );
bool	load_char_obj	args( (DESCRIPTOR_DATA *d, char *name, bool log_load,
				bool load_concept, bool load_backup) );
void save_area		args(( AREA_DATA *pArea ));

/* skills.c */
bool 	parse_gen_groups args( ( CHAR_DATA *ch,char *argument ) );
void 	list_group_costs args( ( CHAR_DATA *ch ) );
void    list_group_known args( ( CHAR_DATA *ch ) );
void 	check_improve	args( ( CHAR_DATA *ch, int sn, bool success, 
				    int multiplier ) );
int 	group_lookup	args( (const char *name) );
void	gn_add		args( ( CHAR_DATA *ch, int gn) );
void 	gn_remove	args( ( CHAR_DATA *ch, int gn) );
void 	group_add	args( ( CHAR_DATA *ch, const char *name, bool deduct) );
void	group_remove	args( ( CHAR_DATA *ch, const char *name) );

/* social-edit.c */
void	load_social_table	args( ( ) );
void	save_social_table	args( ( ) );

/* special.c */
SF *	spec_lookup	args( ( const char *name ) );
char *	spec_name	args( ( SPEC_FUN *function ) );

/* teleport.c */
RID *	room_by_name	args( ( char *target, int level, bool error) );

/* update.c */
void	gain_condition	args( ( CHAR_DATA *ch, int iCond, int value ) );
void	update_handler	args( ( void ) );
int	update_news_stands args( ( void ) );

#undef	CD
#undef	MID
#undef	OD
#undef	OID
#undef	RID
#undef	SF
#undef AD
/*
 * OLC
 */
#define OBJ_VNUM_DUMMY 21

/* Area Flags */
#define		AREA_NONE	0
#define		AREA_CHANGED	(A) /* Area has been modified. */
#define		AREA_ADDED		(B) /* Area has been added to. */
#define		AREA_LOADING	(C) /* Used for counting in db.c */
#define		AREA_SUBURB		(D) /* Area is a suburb for player homes. */
#define		AREA_SLUM		(E)
#define		AREA_BOHEMIAN	(F)
#define		AREA_DOWNTOWN	(G)
#define		AREA_WAREHOUSE	(H)
#define		AREA_PATROLLED	(I)


#define MAX_DIR 6
#define NO_FLAG -99 /*Must not be used in flags or stats. */

/*act_wiz.c*/
/*
ROOM_INDEX_DATA *find_location( CHAR_DATA *ch, char *arg );
*/

/*db.c*/
void	reset_area      args( ( AREA_DATA * pArea ) );
void	reset_room	args( ( ROOM_INDEX_DATA *pRoom ) );
void	forced_reset_area      args( ( AREA_DATA * pArea ) );
void	forced_reset_room	args( ( ROOM_INDEX_DATA *pRoom ) );

/*string.c*/
void	string_edit	args( ( CHAR_DATA *ch, char **pString ) );
void    string_append   args( ( CHAR_DATA *ch, char **pString ) );
char *	string_replace	args( ( char * orig, char * old, char * bnew ) );
void    string_add      args( ( CHAR_DATA *ch, char *argument ) );
char *  format_string   args( ( char *oldstring /*, bool fSpace */ ) );
char *  first_arg       args( ( char *argument, char *arg_first, bool fCase ) );
char *	string_unpad	args( ( char * argument ) );
char *	string_proper	args( ( char * argument ) );
char *  desc_pretty	args( ( char *string, int start, int lines,
				bool no_free ) );

/*olc.c*/
bool	run_olc_editor	args( ( DESCRIPTOR_DATA *d ) );
char	*olc_ed_name	args( ( CHAR_DATA *ch ) );
char	*olc_ed_vnum	args( ( CHAR_DATA *ch ) );

/*olc_save.c*/
void	fwrite_votes	args( ( void ) );

/* websvr.c*/
/*
void init_web(int port);
void handle_web(void);
void shutdown_web(void);
*/

/* bit.c */
/*
extern const struct flag_type 	area_flags[];
extern const struct flag_type	sex_flags[];
extern const struct flag_type	exit_flags[];
extern const struct flag_type	door_resets[];
extern const struct flag_type	room_flags[];
extern const struct flag_type	sector_flags[];
extern const struct flag_type	type_flags[];
extern const struct flag_type	extra_flags[];
extern const struct flag_type	wear_flags[];
extern const struct flag_type	act_flags[];
extern const struct flag_type	affect_flags[];
extern const struct flag_type	affect_flags2[];
extern const struct flag_type	apply_flags[];
extern const struct flag_type	wear_loc_strings[];
extern const struct flag_type	wear_loc_flags[];
extern const struct flag_type	weapon_flags[];
extern const struct flag_type	container_flags[];

extern const struct flag_type   material_type[];
extern const struct flag_type   form_flags[];
extern const struct flag_type   part_flags[];
extern const struct flag_type   ac_type[];
extern const struct flag_type   size_flags[];
extern const struct flag_type   off_flags[];
extern const struct flag_type   imm_flags[];
extern const struct flag_type   res_flags[];
extern const struct flag_type   vuln_flags[];
extern const struct flag_type   position_flags[];
extern const struct flag_type   weapon_class[];
extern const struct flag_type   weapon_type2[];
extern const struct flag_type   furniture_flags[];
extern const struct flag_type   concede_flags[];
extern const struct flag_type   plot_races[];
*/

// Macro's by David Simmerson (Darien/Omega)
extern bool logFail;				// so we can set this to log our purge_data failure
// changed from calloc to malloc to see if this will affect it.
#define ALLOC_DATA(pointer, datatype, elements)   \
		do { \
			if (!((pointer) = (datatype *) calloc ((elements), sizeof(datatype)))) \
			{ \
				logfmt("Unable to calloc new data from file: %s function: %s, line: %d\t *** ABORTING ***", __FILE__, __FUNCTION__, __LINE__); \
				abort(); \
			} \
		} \
		while(0)

#define PURGE_DATA(data) \
		do { \
			if(data) \
			{ \
				free((void *)data); \
			} \
			else { \
				if(logFail) { \
					log_string(LOG_ERR, (char *)Format("clear_free_lists: unable to purge_data as it was NULL from: %s %s %d", __FILE__, __FUNCTION__, __LINE__)); \
				} \
			} \
			data = NULL; \
			} while(0)

#define ReplaceString(pointer, str) \
      do { \
    		 const char *_Iu_Temp__String_RepStr = str; \
                 PURGE_DATA(pointer); \
                 pointer = str_dup(_Iu_Temp__String_RepStr); \
        } while(0)

#define Escape(data) \
	do { \
		if(!data) \
			return; \
	} while(0)

#define CopyTo(to, from) \
        do { \
                  if(!IS_NULLSTR(from)) \
                        to = str_dup(from); \
        } while(0)

#define str_dup(str) _str_dup(str, __FILE__, __FUNCTION__, __LINE__)

/*
 *
 * How and why this works: We utilize several parts of this code to enforce a brand new standard when dealing with asserting.
 * We can effectively drop a log to the cerr, as well as drop a real assert should we so please.
 *
 * If we want the system to crash, we can make it come to a boiling point(ie, where it will blow up), or if we want it to use
 * the real-assert, we can.
 *
 *      Ultimately, we can get a somewhat data-trace from the assert, to the file, function, and line of code where we failed the assertion.
 *      This will give us plenty of information, and hopefully lead to some answers to bad memory.
 *
 */
//#define ASSERT_CRASH
//#define REAL_ASSERT
#ifdef ASSERT
void AssertFailed ( const char *expression, const char *msg, const char *file, const char *baseFile, const char *function, int line );
#if !defined (REAL_ASSERT)
#define Assert(exp, errormsg) \
		do { \
			if (exp) \
			; \
			else \
			AssertFailed( #exp, #errormsg, __FILE__, __BASE_FILE__, __FUNCTION__, __LINE__ ); \
		} while ( 0 )
#else
// this is a-little troubling
#define Assert(exp, errormsg) \
		do { \
			if(exp) \
			; \
			else { \
				AssertFailed( #exp, #errormsg, __FILE__, __BASE_FILE__, __FUNCTION__, __LINE__); \
				assert ( exp );
\
} \
} while ( 0 )
#endif
#else
#if defined (REAL_ASSERT)
#define Assert(exp, errormsg) assert(exp)
#else
#if defined(ASSERT_CRASH)
#define Assert(exp, errormsg)
#else
#define Assert(exp, errormsg) \
		do { \
			if(!(exp)) \
			abort(); \
		}while(0)
#endif
#endif
#endif

bool __strcmp(const char *astr, const char *bstr, const char *_file, const char *_function, int _line);


// default delimiter
#define DELIMITER "~"

// String Values
#define WriteToFile(fp, delimiter, key, value) \
                do { \
                        if(fp && !IS_NULLSTR(key)) { \
                                if(!IS_NULLSTR(value)) \
                                        fprintf(fp,"%s %s%s\n", key, value, delimiter ? DELIMITER:""); \
                        } \
                } while(0)

// integer value's
#define WriteNumber(fp, key, value) \
do { \
if(fp && !IS_NULLSTR(key)) \
fprintf(fp, "%s %d\n",key, value); \
} \
while(0)

// long values
#define WriteLong(fp, key, value) \
do { \
if(fp && !IS_NULLSTR(key)) \
fprintf(fp, "%s %ld\n",key, value); \
} \
while(0)

#define GetName(Ch) (IS_NPC(Ch) ? (!IS_NULLSTR(Ch->short_descr) ? Ch->short_descr : "bad mob") : (!IS_NULLSTR(Ch->name) ? Ch->name : "Bad Name") )

//This checks to make sure there is a character for whatever the command is.
#define CheckCH(ch) \
do { \
    if(!ch) { \
        log_string(LOG_ERR, Format("%s: NULL character data supplied!", __PRETTY_FUNCTION__)); \
        return; \
    } \
} \
while(0)

// This is to replace checks like.  It's a more elegant way of doing it and logs it as well.
//  if ( IS_NPC(ch) )
//        return;
#define CheckChNPC(ch) \
   do { \
      CheckCH(ch); \
      if(IS_NPC(ch)) { \
        send_to_char("No!",ch); \
        log_string(LOG_ERR, Format("(NPCBLOCK)%s attempted to use %s!", GetName(ch), __PRETTY_FUNCTION__ ) ); \
        return; \
    } \
} \
while(0)

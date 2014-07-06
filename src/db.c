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
*   ROM 2.4 is copyright 1993-1998 Russ Taylor                             *
*   ROM has been brought to you by the ROM consortium                      *
*       Russ Taylor (rtaylor@hypercube.org)                                *
*       Gabrielle Taylor (gtaylor@hypercube.org)                           *
*       Brian Moore (zump@rom.org)                                         *
*   By using this code, you have agreed to follow the terms of the         *
*   ROM license, in the file Rom24/doc/rom.license                         *
***************************************************************************/

#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <stdarg.h>
#include <unistd.h>
#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#endif

#include <stddef.h>
#include <dirent.h>

#define IN_DB_C
#include "twilight.h"
#undef IN_DB_C
#include "tables.h"
#include "db.h"
#include "recycle.h"
#include "lookup.h"
#include "olc.h"


bool logFail = FALSE;           // this can be turned into a global value, it doesn't really need to be true
                                // however if you want to log which pointers are NULL when cleared
                                // then be my guest and change this value to true.

const char *Upper(const char *str)
{
    static char up[MSL * 2]={'\0'};
    up[0] = '\0';
    if (str == NULL || str[0] == '\0')
        return "";
    strncpy(up, str, sizeof(up));
    up[0] = toupper(up[0]);
    return up;
}

const char *Lower(const char *str)
{
    static char low[MSL * 2]={'\0'};
    low[0] = '\0';
    if (str == NULL || str[0] == '\0')
        return "";
    strncpy(low, str, sizeof(low));
    low[0] = tolower(low[0]);
    return low;
}



#if !defined(macintosh)
extern  int _filbuf     args( (FILE *) );
#endif

void new_reset  args(( ROOM_INDEX_DATA *pR, RESET_DATA *pReset ));

/*
 * Globals.
 */
HELP_DATA *     help_list;
HELP_DATA *     tip_list;

SHOP_DATA *     shop_list;

MPROG_CODE *    mprog_list;

char            bug_buf     [2*MAX_INPUT_LENGTH];
CHAR_DATA *     char_list;

char *          help_greeting;
char            log_buf     [2*MAX_INPUT_LENGTH];
NOTE_DATA *     note_list;
OBJ_DATA *      object_list = NULL;
TIME_INFO_DATA  time_info;
WEATHER_DATA    weather_info;
int             tax_rate;
ORG_DATA *      org_list;
ORG_DATA *      org_last;
PACK_DATA *     pack_list;

/*
 * Locals.
 */
MOB_INDEX_DATA *    mob_index_hash      [MAX_KEY_HASH];
OBJ_INDEX_DATA *    obj_index_hash      [MAX_KEY_HASH];
ROOM_INDEX_DATA *   room_index_hash     [MAX_KEY_HASH];
char *          string_hash     [MAX_KEY_HASH];

AREA_DATA *     area_first;
AREA_DATA *     area_last;
AREA_DATA *     current_area;

PLOT_DATA *     plot_list;
EVENT_DATA *        event_list;

SCRIPT_DATA *       script_first;
SCRIPT_DATA *       script_last;
PERSONA *       persona_first;
PERSONA *       persona_last;
REACT *         react_first;
REACT *         react_last;
REACT_DATA *        react_data_first;
REACT_DATA *        react_data_last;

STOCKS *        stock_list;

char *          top_string;
char            str_empty   [1];

int         top_affect;
int         top_area;
int         top_ed;
int         top_exit;
int         top_help;
int         top_tip;
int         top_mob_index;
int         top_obj_index;
int         top_reset;
int         top_room;
int         top_shop;
int                     top_vnum_room;  /* OLC */
int                     top_vnum_mob;   /* OLC */
int                     top_vnum_obj;   /* OLC */
int                     top_mprog_index;   /* OLC */
int             mobile_count = 0;
int         newmobs = 0;
int         newobjs = 0;

int         top_web_desc; /* Web server */

int             reboot_countdown = -1;  /* Reboot countdown timer */
char            reboot_initiator[50];  /* Reboot countdown timer */
int         maxSocial;

/*
 * Memory management.
 * Increase MAX_STRING if you have to.
 * Tune the others only if you understand what you're doing.
 */
#define         MAX_STRING  1413120
#define         MAX_PERM_BLOCK  131072
#define         MAX_MEM_LIST    11

void *          rgFreeList  [MAX_MEM_LIST];
const int       rgSizeList  [MAX_MEM_LIST]  =
{
    16, 32, 64, 128, 256, 1024, 2048, 4096, 8192, 16384, 32768-64
};

int         nAllocString;
int         sAllocString;
int         nAllocPerm;
int         sAllocPerm;



/*
 * Semi-locals.
 */
bool            fBootDb;
FILE *          fpArea;
char            strArea[MAX_INPUT_LENGTH];



/*
 * Local booting procedures.
*/
void    init_mm         args( ( void ) );
void    load_area   args( ( FILE *fp ) );
void    new_load_area   args( ( FILE *fp ) );   /* OLC */
void    load_helps  args( ( FILE *fp, int type ) );
void    load_old_helps  args( ( FILE *fp, int type ) );
void    load_mobiles    args( ( FILE *fp ) );
void    load_objects    args( ( FILE *fp ) );
void    load_resets args( ( FILE *fp ) );
void    load_rooms  args( ( FILE *fp ) );
void    load_shops  args( ( FILE *fp ) );
void    load_notes  args( ( void ) );
void    load_papers args( ( void ) );
void    load_bans   args( ( void ) );
void    load_plots  args( ( FILE *fp ) );
void    load_personas   args( ( FILE *fp ) );
void    load_votes  args( ( void ) );
void    load_stocks args( ( void ) );
void    load_org    args( ( FILE *fp ) );
void    load_org_members args( ( FILE *fp ) );
void    load_mobprogs   args( ( FILE *fp ) );

void    fix_exits   args( ( void ) );
void    fix_mobprogs    args( ( void ) );
void    prep_vehicles   args( ( void ) );

void    reset_area  args( ( AREA_DATA * pArea ) );

void    rand_name   args( (CHAR_DATA *ch) );
HELP_DATA * new_help    args( (void) );
void    apply_newsstands args( (void) );
void    openReserve     args( (void) );
void    closeReserve    args( (void) );

/*
 * Big mama top level function.
 */
void boot_db()
{
    /*
     * Init some data space stuff.
     */

    fBootDb = TRUE;

    /*
     * Init random number generator.
     */
    {
        init_mm( );
    }

    /*
     * Set reboot_initiator to blank.
     */
    {
        reboot_initiator[0] = '\0';
    }

    /*
     * Set time and weather.
     */
    {
        long lhour, lday, lmonth;

        lhour       = (current_time - 650336715)
                    / (PULSE_TICK / PULSE_PER_SECOND);
        time_info.hour  = lhour  % 24;
        lday        = lhour  / 24;
        time_info.day   = lday   % 35;
        lmonth      = lday   / 35;
        time_info.month = lmonth % 17;
        time_info.year  = lmonth / 17;

        if ( time_info.hour <  5 ) weather_info.sunlight = SUN_DARK;
        else if ( time_info.hour <  6 ) weather_info.sunlight = SUN_RISE;
        else if ( time_info.hour < 19 ) weather_info.sunlight = SUN_LIGHT;
        else if ( time_info.hour < 20 ) weather_info.sunlight = SUN_SET;
        else                            weather_info.sunlight = SUN_DARK;

        weather_info.change = 0;
        weather_info.mmhg   = 960;
        if ( time_info.month >= 7 && time_info.month <=12 )
            weather_info.mmhg += number_range( 1, 50 );
        else
            weather_info.mmhg += number_range( 1, 80 );

        if ( weather_info.mmhg <=  980 ) weather_info.sky = SKY_LIGHTNING;
        else if ( weather_info.mmhg <= 1000 ) weather_info.sky = SKY_RAINING;
        else if ( weather_info.mmhg <= 1020 ) weather_info.sky = SKY_CLOUDY;
        else                                  weather_info.sky = SKY_CLOUDLESS;

    }

    /*
     * Assign gsn's for skills which have them.
     */
    {
        int sn;

        for ( sn = 0; sn < MAX_SKILL; sn++ )
        {
            if ( skill_table[sn].pgsn != NULL )
                *skill_table[sn].pgsn = sn;
        }
    }

    /*
     * Read in all the area files.
     */
    {
        FILE *fpList;

        if ( ( fpList = fopen( AREA_LIST, "r" ) ) == NULL )
        {
            perror( AREA_LIST );
            exit( 1 );
        }

        for ( ; ; )
        {
            strncpy( strArea, fread_word( fpList ), sizeof(strArea) );
            if ( strArea[0] == '$' )
                break;

            if ( strArea[0] == '-' )
            {
                fpArea = stdin;
            }
            else
            {
                if ( ( fpArea = fopen( strArea, "r" ) ) == NULL )
                {
                    perror( strArea );
                    exit( 1 );
                }
            }

            current_area = NULL;

            for ( ; ; )
            {
                char *word;

                if ( fread_letter( fpArea ) != '#' )
                {
                    bug( "`Boot_db: # not found.", 0 );
                    exit( 1 );
                }

                word = fread_word( fpArea );

                if ( word[0] == '$'               )                 break;
                else if ( !str_cmp( word, "AREA"     ) ) load_area    (fpArea);
                /* OLC */     else if ( !str_cmp( word, "AREADATA" ) ) new_load_area(fpArea);
                else if ( !str_cmp( word, "HELPS"    ) ) load_helps (fpArea,0);
                else if ( !str_cmp( word, "TIPS"     ) ) load_helps (fpArea,1);
                else if ( !str_cmp( word, "HELP"  ) ) load_old_helps (fpArea,0);
                else if ( !str_cmp( word, "TIP"   ) ) load_old_helps (fpArea,1);
                else if ( !str_cmp( word, "MOBILES"  ) ) load_mobiles (fpArea);
                else if ( !str_cmp( word, "MOBPROGS" ) ) load_mobprogs (fpArea);
                else if ( !str_cmp( word, "OBJECTS"  ) ) load_objects (fpArea);
                else if ( !str_cmp( word, "PERSONA"  ) ) load_personas(fpArea);
                else if ( !str_cmp( word, "RESETS"   ) ) load_resets  (fpArea);
                else if ( !str_cmp( word, "ROOMS"    ) ) load_rooms   (fpArea);
                else if ( !str_cmp( word, "SHOPS"    ) ) load_shops   (fpArea);
                else if ( !str_cmp( word, "PLOTS"    ) ) load_plots   (fpArea);
                else
                {
                    bug( "Boot_db: bad section name.", 0 );
                    exit( 1 );
                }
            }

            if ( fpArea != stdin )
                fclose( fpArea );
            fpArea = NULL;
            system((char *)Format("cp %s bak/", strArea));

        }
        fclose( fpList );
    }

    /*
     * Read in all the org files.
     */
    {
        FILE *fpList;

        if ( ( fpList = fopen( (char *)Format("%s%s", ORG_DIR, ORG_LIST), "r" ) ) == NULL )
        {
            perror( ORG_LIST );
            exit( 1 );
        }

        for ( ; ; )
        {
            strncpy( strArea, fread_word( fpList ), sizeof(strArea) );
            if ( strArea[0] == '$' )
                break;

            if ( strArea[0] == '-' )
            {
                fpArea = stdin;
            }
            else
            {
                if ( ( fpArea = fopen( (char *)Format("%s%s", ORG_DIR, strArea), "r" ) ) == NULL )
                {
                    perror( strArea );
                    exit( 1 );
                }
            }

            for ( ; ; )
            {
                char *word;

                if ( fread_letter( fpArea ) != '#' )
                {
                    bug( "Boot_db (Org): # not found.", 0 );
                    exit( 1 );
                }

                word = fread_word( fpArea );

                if ( word[0] == '$'               )                 break;
                else if ( !str_cmp( word, "ORG"     ) ) load_org    (fpArea);
                else if ( !str_cmp( word, "MEMBERS"))load_org_members(fpArea);
                else
                {
                    bug( "Boot_db (Org): bad section name.", 0 );
                    exit( 1 );
                }
            }

            if ( fpArea != stdin )
                fclose( fpArea );
            fpArea = NULL;
        }
        fclose( fpList );
    }

    /*
     * Generate shop for news stands and apply it to news stand mob indices.
     */
    apply_newsstands();

    /*
     * Fix up exits.
     * Declare db booting over.
     * Reset all areas once.
     * Load up the notes and ban files.
     */
    {
        fix_exits();
        fix_mobprogs();
        prep_vehicles();
        fBootDb = FALSE;
        area_update();
        load_notes();
        load_papers();
        load_bans();
        //load_votes();
        load_stocks();
        load_social_table();
    }

    return;
}


/*
 * Spam out news stands.
 */
void apply_newsstands()
{
    SHOP_DATA *pShop;
    MOB_INDEX_DATA *step, *pMobIndex;
    int ispot = 0;

    for ( ispot = 0; ispot < MAX_KEY_HASH; ispot++ )
    {

        if((step = mob_index_hash[ispot]) != NULL)
            for ( pMobIndex = step; pMobIndex; pMobIndex = pMobIndex->next )
            {

                if(IS_SET(pMobIndex->act2, ACT2_NEWS_STAND))
                {
                    //ALLOC_DATA(pShop, SHOP_DATA, 1);
                    pShop = new_shop();
                    pMobIndex->pShop        = pShop;
                    pShop->keeper       = pMobIndex->vnum;
                    if ( pShop->keeper == 0 )
                        break;
                    pShop->buy_type[0]      = ITEM_MAP;
                    pShop->profit_buy       = 100;
                    pShop->profit_sell      = 100;
                    pShop->open_hour        = 0;
                    pShop->close_hour       = 24;

                    LINK_SINGLE(pShop, next, shop_list);
                    top_shop++;
                }
            }
    }

    return;
}


/*
 * Snarf an 'area' header line.
 */
void load_area( FILE *fp )
{
    AREA_DATA *pArea;

    pArea = new_area();
    /*  pArea->reset_first  = NULL;
    pArea->reset_last   = NULL; */
    pArea->file_name    = fread_string(fp);

    pArea->area_flags   = AREA_LOADING;         /* OLC */
    pArea->security     = 9;                    /* OLC */ /* 9 -- Hugin */
    PURGE_DATA(pArea->builders);
    pArea->builders     = str_dup( "None" );    /* OLC */
    pArea->vnum         = top_area;             /* OLC */

    pArea->name     = fread_string( fp );
    pArea->credits  = fread_string( fp );
    pArea->min_vnum = fread_number( fp );
    pArea->max_vnum = fread_number( fp );
    /* pArea->area_version = fread_number( fp ); */
    pArea->age      = 15;
    pArea->nplayer  = 0;
    pArea->empty    = FALSE;

    if ( !area_first )
        area_first = pArea;
    if ( area_last )
    {
        area_last->next = pArea;
        REMOVE_BIT(area_last->area_flags, AREA_LOADING);        /* OLC */
    }
    area_last   = pArea;
    pArea->next = NULL;
    current_area = pArea;

    top_area++;
    return;
}

/*
 * Snarf an 'org' header line.
 */
void load_org( FILE *fp )
{
    ORG_DATA *org;
    int i = 0;

    org                  = new_org();
    PURGE_DATA(org->name);
    PURGE_DATA(org->who_name);
    PURGE_DATA(org->file_name);
    PURGE_DATA(org->leader);
    PURGE_DATA(org->applicants);
    PURGE_DATA(org->races);
    org->name            = fread_string(fp);
    org->who_name        = fread_string(fp);
    org->file_name       = str_dup(strArea);
    org->leader          = fread_string(fp);
    org->step_point      = fread_number(fp);
    org->type            = fread_number(fp);
    org->funds           = fread_number(fp);
    org->applicants      = fread_string(fp);
    org->races           = fread_string(fp);
    org->default_auths   = fread_flag(fp);
    for(i = 0; i < 5; i++)
    {
        PURGE_DATA(org->commands[i]);
        org->commands[i] = fread_string(fp);
    }

    for(i = 0; i < 6; i++)
    {
        PURGE_DATA(org->title[i]);
        org->title[i] = fread_string(fp);
    }

    if ( !org_list )
        org_list = org;
    if ( org_last )
    {
        org_last->next = org;
    }
    org_last    = org;
    org->next   = NULL;

    return;
}

/*
 * OLC
 * Use these macros to load any new area formats that you choose to
 * support on your MUD.  See the new_load_area format below for
 * a short example.
 */
#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )                \
                if ( !str_cmp( word, literal ) )    \
                {                                   \
                    field  = value;                 \
                    fMatch = TRUE;                  \
                    break;                          \
                                }

#define SKEY( string, field )                       \
                if ( !str_cmp( word, string ) )     \
                {                                   \
                    PURGE_DATA( field );           \
                    field = fread_string( fp );     \
                    fMatch = TRUE;                  \
                    break;                          \
                                }



/* OLC
 * Snarf an 'area' header line.   Check this format.  MUCH better.  Add fields
 * too.
 *
 * #AREAFILE
 * Name   { All } Locke    Newbie School~
 * Repop  A teacher pops in the room and says, 'Repop coming!'~
 * Recall 3001
 * End
 */
void new_load_area( FILE *fp )
{
    AREA_DATA *pArea;
    const char      *word;
    bool      fMatch;

    pArea = new_area();
    pArea->age          = 15;
    pArea->nplayer      = 0;

    PURGE_DATA( pArea->file_name );
    pArea->file_name     = str_dup( strArea );
    pArea->vnum         = top_area;

    PURGE_DATA( pArea->name );
    PURGE_DATA( pArea->builders );

    pArea->name         = str_dup( "New Area" );
    pArea->builders     = NULL;
    pArea->security     = 9;                    /* 9 -- Hugin */
    pArea->min_vnum     = 0;
    pArea->max_vnum     = 0;
    pArea->area_flags   = 0;
    /* pArea->area_version = CURRENT_AREA_VERSION; */
    /*  pArea->recall       = ROOM_VNUM_TEMPLE;        ROM OLC */

    for ( ; ; )
    {
        word   = feof( fp ) ? "End" : fread_word( fp );
        fMatch = FALSE;

        switch ( UPPER(word[0]) )
        {
        case 'N':
            SKEY( "Name", pArea->name );
            break;
        case 'S':
            KEY( "Security", pArea->security, fread_number( fp ) );
            break;
        case 'V':
            if ( !str_cmp( word, "VNUMs" ) )
            {
                pArea->min_vnum = fread_number( fp );
                pArea->max_vnum = fread_number( fp );
            }
            break;
        case 'E':
            if ( !str_cmp( word, "End" ) )
            {
                fMatch = TRUE;
                if ( area_first == NULL )
                    area_first = pArea;
                if ( area_last  != NULL )
                    area_last->next = pArea;
                area_last   = pArea;
                pArea->next = NULL;
                top_area++;
                return;
            }
            break;
        case 'F':
            KEY( "Flags", pArea->area_flags, fread_flag( fp ) );
            break;
        case 'B':
            SKEY( "Builders", pArea->builders );
            break;
        case 'C':
            SKEY( "Credits", pArea->credits );
            break;
        case 'P':
            KEY( "Price", pArea->pricemod, fread_number( fp ) );
            break;
        }
    }
}

/*
 * Sets vnum range for area using OLC protection features.
 */
void assign_area_vnum( int vnum )
{
    if ( area_last->min_vnum == 0 || area_last->max_vnum == 0 )
        area_last->min_vnum = area_last->max_vnum = vnum;
    if ( vnum != URANGE( area_last->min_vnum, vnum, area_last->max_vnum ) )
    {
        if ( vnum < area_last->min_vnum )
            area_last->min_vnum = vnum;
        else
            area_last->max_vnum = vnum;
    }
    return;
}


/* Conversion functions for the new help format. */
void colourstrip( char *txt, char *out )
{
    const       char    *point;
    char buffer[MSL*10]={'\0'};
    char *buf = buffer;

    for( point = txt ; *point ; point++ )
    {
        if( *point == '{' )
        {
            point++;
            continue;
        }
        *buf = *point;
/*        *buf = *buf++;
*/
    (*buf)++;
    }
    *buf = '\0';

    /* Removed to try to fix name changing bug.
    return str_dup(buffer);
    */
    strcpy(out, buffer);
    
    return;
}


/*
 * For loading new style helps.
 */
void fread_help( HELP_DATA *help, FILE *fp )
{
    const char *word;
    bool fMatch = FALSE;

    for ( ; ; )
    {
        word   = feof( fp ) ? "End" : fread_word( fp );
        fMatch = FALSE;

        switch ( UPPER(word[0]) )
        {
        case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;

    case 'E':
             if ( !str_cmp( word, "End" ) )
             {
                 fMatch = TRUE;
                 return;
            }
            break;

        case 'D':
            if (!str_cmp(word,"Desc"))
            {
                help->description = fread_string( fp ) ;
                fMatch = TRUE;
                break;
            }
        break;

        case 'Q':
            if (!str_cmp(word,"Quote"))
            {
                help->quote = fread_string( fp ) ;
                fMatch = TRUE;
                break;
            }
        break;

        case 'S':
            if (!str_cmp(word,"See"))
            {
                help->see_also = fread_string( fp ) ;
                fMatch = TRUE;
                break;
            }
            if (!str_cmp(word,"Syntax"))
            {
                help->syntax = fread_string( fp ) ;
                fMatch = TRUE;
                break;
            }
        break;

        case 'T':
            if (!str_cmp(word,"Topic"))
            {
                help->topic = fread_string( fp ) ;
                fMatch = TRUE;
                break;
            }
        break;

        case 'U':
            if (!str_cmp(word,"Unformatted"))
            {
                help->unformatted = fread_string( fp ) ;
                fMatch = TRUE;
                break;
            }
        break;

        case 'W':
            if (!str_cmp(word,"Web"))
            {
                help->website = fread_string( fp ) ;
                fMatch = TRUE;
                break;
            }
        break;
    }

        if ( !fMatch )
        {
            bug( "Fread_help: no match. Dumping into unformatted.", 0 );
            help->unformatted = fread_string( fp ) ;
        }
    }
}

const char *help_section [] =
{"Topic:", "Syntax:", "Description:", "See also:"};


/*
 * Snarf a help section.
 */
void load_helps( FILE *fp, int type )
{
    HELP_DATA *pHelp;

    for ( ; ; )
    {
    pHelp       = new_help();
    pHelp->level    = fread_number( fp );
    pHelp->keyword  = fread_string( fp );

    if(pHelp->keyword == NULL) {
        free_help(pHelp);
        return;
    }

    if ( pHelp->keyword[0] == '$' ) {
        free_help(pHelp);
            break;
    }

    pHelp->races    = fread_string( fp );
    pHelp->clans    = fread_string( fp );
    fread_help( pHelp, fp );

    if ( !str_cmp( pHelp->keyword, "greeting" ) )
        help_greeting = pHelp->unformatted;

    if(!type)
    {
      LINK_SINGLE(pHelp, next, help_list);
      top_help++;
    }
    else
    {
      LINK_SINGLE(pHelp, next, tip_list);
      top_tip++;
    }
    }

    return;
}


/*
 * Snarf a help section.
 */
void load_old_helps( FILE *fp, int type )
{
    HELP_DATA *pHelp;

    for ( ; ; )
    {
    pHelp       = new_help();
    pHelp->level    = fread_number( fp );
    pHelp->keyword  = fread_string( fp );
    if ( pHelp->keyword[0] == '$' ) {
        free_help(pHelp);
        break;
    }

    pHelp->unformatted  = fread_string( fp );

    if ( !str_cmp( pHelp->keyword, "greeting" ) )
        help_greeting = pHelp->unformatted;

    if(!type)
    {
      LINK_SINGLE(pHelp, next, help_list);
      top_help++;
    }
    else
    {
      LINK_SINGLE(pHelp, next, tip_list);
      top_tip++;
    }
    }

    return;
}


/*
 * Snarf a reset section.
 */
void load_resets( FILE *fp )
{
    RESET_DATA *pReset;
    int     iLastRoom = 0;
    int     iLastObj = 0;

    if(!area_last)
    {
        bug( "Load_resets: no #AREA seen yet.", 0 );
        exit( 1 );
    }

    for ( ; ; )
    {
        ROOM_INDEX_DATA *pRoomIndex;
        char letter;
        /* OBJ_INDEX_DATA *temp_index;
    int temp; */

        if ( ( letter = fread_letter( fp ) ) == 'S' )
            break;

        if ( letter == '*' )
        {
            fread_to_eol( fp );
            continue;
        }
        //ALLOC_DATA(pReset, RESET_DATA, 1);
        pReset = new_reset_data();
        pReset->command = letter;
        /* if_flag */     fread_number( fp );
        pReset->arg1    = fread_number( fp );
        pReset->arg2    = fread_number( fp );
        pReset->arg3    = (letter == 'G' || letter == 'R')
                        ? 0 : fread_number( fp );
        pReset->arg4    = (letter == 'P' || letter == 'M')
                        ? fread_number(fp) : 0;
        fread_to_eol( fp );

        /*
         * Validate parameters.
         * We're calling the index functions for the side effect.
         */
        switch ( letter )
        {
        default:
            bug( "Load_resets: bad command '%c'.", letter );
            exit( 1 );
            break;

        case 'F':
            /*temp_index = get_obj_index  ( pReset->arg1 );
        temp_index->reset_num++;
        get_room_index ( pReset->arg3 );*/
            /* No longer necessary.
        get_obj_index (pReset->arg1);*/
            if(( pRoomIndex = get_room_index( pReset->arg3)))
            {
                new_reset(pRoomIndex, pReset);
                iLastObj = pReset->arg3;
            } else { free_reset_data(pReset); }
            break;

        case 'M':
            /* No longer necessary.
                get_mob_index  ( pReset->arg1 );*/
            if ((pRoomIndex = get_room_index ( pReset->arg3 )))
            {
                new_reset(pRoomIndex, pReset);
                iLastRoom = pReset->arg3;
            }  else { free_reset_data(pReset); }
            break;

        case 'O':
            /*temp_index = get_obj_index  ( pReset->arg1 );
        temp_index->reset_num++;
        get_room_index ( pReset->arg3 );*/
            /* No longer necessary.
        get_obj_index (pReset->arg1);*/
            if(( pRoomIndex = get_room_index( pReset->arg3)))
            {
                new_reset(pRoomIndex, pReset);
                iLastObj = pReset->arg3;
            }  else { free_reset_data(pReset); }
            break;

        case 'P':
            /*temp_index = get_obj_index  ( pReset->arg1 );
        temp_index->reset_num++;
        get_obj_index  ( pReset->arg3 );*/
            /* No longer necessary.
        get_obj_index ( pReset->arg1 );*/
            if(( pRoomIndex = get_room_index (iLastObj)))
            {
                new_reset(pRoomIndex, pReset);
            }  else { free_reset_data(pReset); }
            break;

        case 'G':
        case 'E':
            /*temp_index = get_obj_index  ( pReset->arg1 );
        temp_index->reset_num++;*/
            /* No longer necessary.
        get_obj_index(pReset->arg1);*/
            if((pRoomIndex = get_room_index(iLastRoom)))
            {
                new_reset(pRoomIndex, pReset);
                iLastObj = iLastRoom;
            }  else { free_reset_data(pReset); }
            break;

        case 'D':
            log_string("load_resets: old-style door reset found! (D)");
            free_reset_data(pReset);
            break;

        case 'R':
            pRoomIndex      = get_room_index( pReset->arg1 );

            if ( pReset->arg2 < 0 || pReset->arg2 > MAX_DIR )
            {
                bug( "Load_resets: 'R': bad exit %d.", pReset->arg2 );
                exit( 1 );
            }

            if(pRoomIndex)
                new_reset(pRoomIndex, pReset);
             else { free_reset_data(pReset); }
            break;
        }

        /*  if ( area_last->reset_first == NULL )
        area_last->reset_first  = pReset;
    if ( area_last->reset_last  != NULL )
        area_last->reset_last->next = pReset;

    area_last->reset_last   = pReset;
    pReset->next        = NULL;
    top_reset++; */
    }

    return;
}


/*
 * Snarf a room section.
 */
void load_rooms( FILE *fp )
{
    ROOM_INDEX_DATA *pRoomIndex;

    if ( area_last == NULL )
    {
        bug( "Load_resets: no #AREA seen yet.", 0 );
        exit( 1 );
    }

    if ( !area_last )   /* OLC */
    {
        bug( "Load_rooms: no #AREA seen yet.", 0 );
        exit( 1 );
    }

    for ( ; ; )
    {
        sh_int vnum;
        char letter;
        int door;
        int iHash;

        letter              = fread_letter( fp );
        if ( letter != '#' )
        {
            bug( "Load_rooms: # not found.", 0 );
            exit( 1 );
        }

        vnum                = fread_number( fp );
        if ( vnum == 0 )
            break;

        fBootDb = FALSE;
        if ( get_room_index( vnum ) != NULL )
        {
            bug( (char *)Format("Load_rooms: vnum %d duplicated.", vnum),  0 );
            exit( 1 );
        }
        fBootDb = TRUE;

        pRoomIndex          = new_room_index();
        pRoomIndex->area        = area_last;
        pRoomIndex->vnum        = vnum;
        pRoomIndex->name        = fread_string( fp );
        pRoomIndex->description     = fread_string( fp );
        /* Area number */         fread_number( fp );
        pRoomIndex->room_flags      = fread_flag( fp );
        pRoomIndex->sector_type     = fread_number( fp );
        pRoomIndex->light       = 0;
        for ( door = 0; door <= MAX_DIR; door++ )
            pRoomIndex->exit[door] = NULL;

        /* defaults */
        pRoomIndex->heal_rate = 100;

        for ( ; ; )
        {
            letter = fread_letter( fp );

            if ( letter == 'S' )
                break;

            if ( letter == 'H') /* healing room */
                pRoomIndex->heal_rate = fread_number(fp);

            else if ( letter == 'M') /* mana room */
                pRoomIndex->mana_rate = fread_number(fp);

            else if ( letter == 'C') /* clan */
            {
                if (pRoomIndex->clan)
                {
                    bug("Load_rooms: duplicate clan fields.",0);
                    exit(1);
                }
                pRoomIndex->clan = clan_lookup(fread_string(fp));
            }

            else if ( letter == 'D' )
            {
                EXIT_DATA *pexit;

                door = fread_number( fp );
                if ( door < 0 || door > 5 )
                {
                    bug( "Fread_rooms: vnum %d has bad door number.", vnum );
                    exit( 1 );
                }

                pexit = new_exit();
                pexit->description  = fread_string( fp );
                pexit->keyword      = fread_string( fp );
                pexit->exit_info    = fread_flag( fp );
                pexit->rs_flags     = pexit->exit_info;     /* OLC */
                pexit->key      = fread_number( fp );
                pexit->u1.vnum      = fread_number( fp );
                pexit->orig_door    = door;         /* OLC */

                pRoomIndex->exit[door]  = pexit;
                pRoomIndex->old_exit[door] = pexit;
                top_exit++;
            }

            else if ( letter == 'E' )
            {
                EXTRA_DESCR_DATA *ed;

                ed          = new_extra_descr();
                ed->keyword     = fread_string( fp );
                ed->description     = fread_string( fp );
                ed->timer       = -1;

                if(!str_cmp(ed->keyword, "XDUMB1"))
                {
                    if(!IS_NULLSTR(ed->description))
                        pRoomIndex->udescription = str_dup(ed->description);
                    free_extra_descr(ed);
                }
                else if(!str_cmp(ed->keyword, "XNUMB1"))
                {
                    if(!IS_NULLSTR(ed->description))
                        pRoomIndex->uname = str_dup(ed->description);
                    free_extra_descr(ed);
                }
                else if(!str_cmp(ed->keyword, "XNDREAM"))
                {
                    if(!IS_NULLSTR(ed->description))
                        pRoomIndex->dname = str_dup(ed->description);
                    free_extra_descr(ed);
                }
                else if(!str_cmp(ed->keyword, "XDDREAM"))
                {
                    if(!IS_NULLSTR(ed->description))
                        pRoomIndex->ddescription = str_dup(ed->description);
                    free_extra_descr(ed);
                }
                else
                {
                    LINK_SINGLE(ed, next, pRoomIndex->extra_descr);
                    top_ed++;
                }
            }

            else if ( letter == 'J' )
            {
                EXIT_DATA *pexit;

                door = fread_number( fp );
                if ( door < 0 || door > 5 )
                {
                    bug( "Fread_rooms: vnum %d has bad jumpto.", vnum );
                }
                else
                {
                    pexit       = pRoomIndex->exit[door];
                    pexit->jumpto.vnum  = fread_number( fp );
                }
            }

            else if (letter == 'O')
            {
                if (pRoomIndex->owner[0] != '\0')
                {
                    bug("Load_rooms: duplicate owner.",0);
                    exit(1);
                }

                PURGE_DATA(pRoomIndex->owner);
                pRoomIndex->owner = fread_string(fp);
            }

            else if (letter == 'V') /* Vehicle/Train/Elevator */
            {
                if(!IS_SET(pRoomIndex->room_flags, ROOM_STOP)
                        && !IS_SET(pRoomIndex->room_flags, ROOM_VEHICLE)
                        && !IS_SET(pRoomIndex->room_flags, ROOM_ELEVATOR))
                {
                    bug("Load_rooms: Unnecessary vehicle data.", 0);
                    fread_number(fp);
                    fread_number(fp);
                    continue;
                }
                else
                {
                    pRoomIndex->car = fread_number(fp);
                    pRoomIndex->stops[0] = fread_number(fp);
                }
            }

            else
            {
                bug( "Load_rooms: vnum %d has flag not 'DES'.", vnum );
                exit( 1 );
            }
        }

        iHash           = vnum % MAX_KEY_HASH;
        LINK_SINGLE(pRoomIndex, next, room_index_hash[iHash]);
        top_room++;
        top_vnum_room = top_vnum_room < vnum ? vnum : top_vnum_room; /* OLC */
        assign_area_vnum( vnum );                                    /* OLC */
    }

    return;
}



/*
 * Snarf a shop section.
 */
void load_shops( FILE *fp )
{
    SHOP_DATA *pShop;

    for ( ; ; )
    {
        MOB_INDEX_DATA *pMobIndex;
        int iTrade;

        pShop = new_shop();
        pShop->keeper       = fread_number( fp );
        if ( pShop->keeper == 0 ) {
            free_shop(pShop);
            break;
        }
        for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ )
            pShop->buy_type[iTrade] = fread_number( fp );
        pShop->profit_buy   = fread_number( fp );
        pShop->profit_sell  = fread_number( fp );
        pShop->open_hour    = fread_number( fp );
        pShop->close_hour   = fread_number( fp );
        pShop->raw_materials    = fread_flag( fp );
        fread_to_eol( fp );
        pMobIndex       = get_mob_index( pShop->keeper );
        pMobIndex->pShop    = pShop;

        LINK_SINGLE(pShop, next, shop_list);

        top_shop++;
    }

    return;
}



/*
 * Load mobprogs section
 */
void load_mobprogs( FILE *fp )
{
    MPROG_CODE *pMprog;

    if ( area_last == NULL )
    {
        bug( "Load_mobprogs: no #AREA seen yet.", 0 );
        exit( 1 );
    }

    for ( ; ; )
    {
        sh_int vnum;
        char letter;

        letter            = fread_letter( fp );
        if ( letter != '#' )
        {
            bug( "Load_mobprogs: # not found.", 0 );
            exit( 1 );
        }

        vnum             = fread_number( fp );
        if ( vnum == 0 )
            break;

        fBootDb = FALSE;
        if ( get_mprog_index( vnum ) != NULL )
        {
            bug( "Load_mobprogs: vnum %d duplicated.", vnum );
            exit( 1 );
        }
        fBootDb = TRUE;

        pMprog = new_mpcode();
        pMprog->vnum    = vnum;
        pMprog->code    = fread_string( fp );
    LINK_SINGLE(pMprog, next, mprog_list); 
        top_mprog_index++;
    }
    return;
}

/*
 *  Translate mobprog vnums pointers to real code
 */
void fix_mobprogs( void )
{
    MOB_INDEX_DATA *pMobIndex;
    MPROG_LIST        *list;
    MPROG_CODE        *prog;
    int iHash = 0;

    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
        for ( pMobIndex   = mob_index_hash[iHash];
              pMobIndex   != NULL;
              pMobIndex   = pMobIndex->next )
        {
            for( list = pMobIndex->mprogs; list != NULL; list = list->next )
            {
                if ( ( prog = get_mprog_index( list->vnum ) ) != NULL )
                    list->code = str_dup(prog->code);
                else
                {
                    bug( "Fix_mobprogs: code vnum %d not found.", list->vnum );
                    exit( 1 );
                }
            }
        }
    }
}


/*
 * Translate all room exits from virtual to real.
 * Has to be done after all rooms are read in.
 * Check for bad reverse exits.
 */
void fix_exits( void )
{
    extern const sh_int rev_dir [];
    ROOM_INDEX_DATA *pRoomIndex;
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev;
    int iHash = 0;
    int door = 0;

    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
    for ( pRoomIndex  = room_index_hash[iHash];
          pRoomIndex != NULL;
          pRoomIndex  = pRoomIndex->next )
    {
        bool fexit;

        fexit = FALSE;
        for ( door = 0; door <= 5; door++ )
        {
        if ( ( pexit = pRoomIndex->exit[door] ) != NULL )
        {
            if ( pexit->u1.vnum <= 0
            || get_room_index(pexit->u1.vnum) == NULL)
            pexit->u1.to_room = NULL;
            else
            {
            fexit = TRUE;
            pexit->u1.to_room = get_room_index( pexit->u1.vnum );
            }
        }
        }
        if (!fexit)
        SET_BIT(pRoomIndex->room_flags,ROOM_NO_MOB);
    }
    }

    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
    for ( pRoomIndex  = room_index_hash[iHash];
          pRoomIndex != NULL;
          pRoomIndex  = pRoomIndex->next )
    {
        for ( door = 0; door <= 5; door++ )
        {
        if ( ( pexit     = pRoomIndex->exit[door]       ) != NULL
        &&   ( to_room   = pexit->u1.to_room            ) != NULL
        &&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
        &&   pexit_rev->u1.to_room != pRoomIndex
        &&   !IS_SET(pexit->exit_info, EX_WINDOW))
        {
            bug( (char *)Format("Fix_exits: %d:%d -> %d:%d -> %d.",
            pRoomIndex->vnum, door, to_room->vnum, rev_dir[door], (pexit_rev->u1.to_room == NULL) ? 0 : pexit_rev->u1.to_room->vnum), 0 );
        }
        }
    }
    }

    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
    for ( pRoomIndex  = room_index_hash[iHash];
          pRoomIndex != NULL;
          pRoomIndex  = pRoomIndex->next )
    {
        for ( door = 0; door <= 5; door++ )
        {
        if ( ( pexit = pRoomIndex->exit[door] ) != NULL )
        {
            if ( pexit->jumpto.vnum <= 0
            || get_room_index(pexit->jumpto.vnum) == NULL)
            pexit->jumpto.to_room = NULL;
            else
            {
             pexit->jumpto.to_room = get_room_index(pexit->jumpto.vnum);
            }
        }
        }
    }
    }

    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
    for ( pRoomIndex  = room_index_hash[iHash];
          pRoomIndex != NULL;
          pRoomIndex  = pRoomIndex->next )
    {
        for ( door = 0; door <= 5; door++ )
        {
        if ( ( pexit     = pRoomIndex->exit[door]       ) != NULL
        &&   ( to_room   = pexit->jumpto.to_room            ) != NULL
        &&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
        &&   pexit_rev->jumpto.to_room != pRoomIndex
        &&   !IS_SET(pexit->exit_info, EX_WINDOW))
        {
            bug( (char *)Format("Fix_exits (Jump to): %d:%d -> %d:%d -> %d.",
            pRoomIndex->vnum, door, to_room->vnum,    rev_dir[door], (pexit_rev->jumpto.to_room == NULL) ? 0 : pexit_rev->jumpto.to_room->vnum), 0 );
        }
        }
    }
    }

    return;
}



/*
 * Repopulate areas periodically.
 */
void area_update( void )
{
    AREA_DATA *pArea;

    for ( pArea = area_first; pArea != NULL; pArea = pArea->next )
    {

        if ( ++pArea->age < 3 )
            continue;

        /*
         * Check age and reset.
         * Note: Mud School resets every 3 minutes (not 15).
         */
        if ( (!pArea->empty && (pArea->nplayer == 0 || pArea->age >= 10)) || pArea->age >= 3)
        {
            ROOM_INDEX_DATA *pRoomIndex;

            reset_area( pArea );
            wiznet((char *)Format("\tY[WIZNET]\tn %s has just been reset.",pArea->name),NULL,NULL,WIZ_RESETS,0,0);

            pArea->age = number_range( 0, 3 );
            pRoomIndex = get_room_index( ROOM_VNUM_START );
            if ( pRoomIndex != NULL && pArea == pRoomIndex->area )
                pArea->age = 15 - 2;
            else if (pArea->nplayer == 0)
                pArea->empty = TRUE;
        }
    }

    return;
}

/*
 * Reset one room.  Called by forced_reset_area and olc.
 */
void forced_reset_room( ROOM_INDEX_DATA *pRoom )
{
    RESET_DATA  *pReset;
    CHAR_DATA   *pMob;
    CHAR_DATA   *mob;
    OBJ_DATA    *pObj;
    CHAR_DATA   *LastMob = NULL;
    OBJ_DATA    *LastObj = NULL;
    int iExit = 0;
    bool last = 0;

    if ( !pRoom )
        return;

    pMob        = NULL;
    last        = FALSE;

    for ( iExit = 0;  iExit < MAX_DIR;  iExit++ )
    {
        EXIT_DATA *pExit;
        if ( ( pExit = pRoom->exit[iExit] )
      /*  && !IS_SET( pExit->exit_info, EX_BASHED )   ROM OLC */ )  
        {
            pExit->exit_info = pExit->rs_flags;
            if ( ( pExit->u1.to_room != NULL )
              && ( ( pExit = pExit->u1.to_room->exit[rev_dir[iExit]] ) ) )
            {
                /* nail the other side */
                pExit->exit_info = pExit->rs_flags;
            }
        }
    }

    for ( pReset = pRoom->reset_first; pReset != NULL; pReset = pReset->next )
    {
        MOB_INDEX_DATA  *pMobIndex;
        OBJ_INDEX_DATA  *pObjIndex;
        OBJ_INDEX_DATA  *pObjToIndex;
        ROOM_INDEX_DATA *pRoomIndex;
    int count = 0,limit=0;

        switch ( pReset->command )
        {
        default:
                bug( "Reset_room: bad command %c.", pReset->command );
                break;

    case 'F':
        if ( !( pObjIndex = get_obj_index( pReset->arg1 ) )
        && pReset->arg1 != 0)
        {
        bug( "Reset_area: 'F': bad vnum %d.", pReset->arg1 );
        bug(Format("%d %d %d %d",pReset->arg1, pReset->arg2, pReset->arg3, pReset->arg4),1);
        continue;
        }

        if ( !( pRoomIndex = get_room_index( pReset->arg3 ) ) )
        {
            bug( "Reset_area: 'F': bad vnum %d.", pReset->arg3 );
            bug(Format("%d %d %d %d",pReset->arg1,pReset->arg2,pReset->arg3, pReset->arg4),1);
            continue;
        }

        if ( pRoom->area->nplayer > 0
        || ( pObjIndex != NULL
          && count_obj_list( pObjIndex, pRoom->contents ) > 0)
        || ( pObjIndex == NULL
          && pRoom->owner[0] != '\0'
          && !is_online(pRoom->owner)) )
        {
        pObj = pRoom->contents;
        while(pObj != NULL)
        {
            pObj = pRoom->contents;
            if(pObj != NULL)
            extract_obj(pObj);
        }
        }

        if( pObjIndex != NULL )
        {
            pObj       = create_object( pObjIndex );
            pObj->cost = 0;
            obj_to_room( pObj, pRoom );
        SET_BIT(pObj->extra2, OBJ_FURNISHING);
        }
        else
        {
        if(!is_online(pRoom->owner))
        {
            if((mob = get_player(pRoom->owner)) != NULL)
            {
            /* @@@@@ place_home_room_obj(mob, pRoom); */
            free_char(mob);
            }
        }
        }
        last = TRUE;
        break;

        case 'M':
            if ( !( pMobIndex = get_mob_index( pReset->arg1 ) ) )
            {
                bug( "Reset_room: 'M': bad vnum %d.", pReset->arg1 );
                continue;
            }

        if ( ( pRoomIndex = get_room_index( pReset->arg3 ) ) == NULL )
        {
        bug( "Reset_area: 'R': bad vnum %d.", pReset->arg3 );
        continue;
        }

            if ( pMobIndex->count >= pReset->arg2 )
            {
        mob = pRoom->people;
        }

        count = 0;
        for (mob = pRoomIndex->people; mob != NULL; mob = mob->next_in_room)
        if (mob->pIndexData == pMobIndex)
        {
            count++;
            if (count >= pReset->arg4)
            {
                last = FALSE;
                break;
            }
        }

        if (count > pReset->arg4)
        break;

        pMob = create_mobile( pMobIndex );

        char_to_room( pMob, pRoomIndex );
        LastMob = pMob;
        last = TRUE;
        break;

    case 'O':
        if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) )
        {
        bug( "Reset_area: 'O': bad vnum %d.", pReset->arg1 );
        bug(Format("%d %d %d %d",pReset->arg1, pReset->arg2, pReset->arg3, pReset->arg4),1);
        continue;
        }

        if ( !( pRoomIndex = get_room_index( pReset->arg3 ) ) )
        {
        bug( "Reset_area: 'R': bad vnum %d.", pReset->arg3 );
        bug(Format("%d %d %d %d",pReset->arg1,pReset->arg2,pReset->arg3, pReset->arg4),1);
        continue;
        }

        if ( pRoom->area->nplayer > 0
        ||   count_obj_list( pObjIndex, pRoom->contents ) > 0 )
        {
        pObj = pRoom->contents;
        while(pObj != NULL)
        {
            pObj = pRoom->contents;
            if(pObj != NULL)
            extract_obj(pObj);
        }
        }

        pObj       = create_object( pObjIndex );
        pObj->cost = 0;
        obj_to_room( pObj, pRoom );
        last = TRUE;
        break;

    case 'P':
        if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) )
        {
        bug( "Reset_area: 'P': bad vnum %d.", pReset->arg1 );
        continue;
        }

        if ( !( pObjToIndex = get_obj_index( pReset->arg3 ) ) )
        {
        bug( "Reset_area: 'P': bad vnum %d.", pReset->arg3 );
        continue;
        }

            if (pReset->arg2 > 50) /* old format */
                limit = 6;
            else if (pReset->arg2 == -1) /* no limit */
                limit = 999;
            else
                limit = pReset->arg2;

        if ((LastObj = get_obj_type( pObjToIndex ) ) == NULL
        || (LastObj->in_room == NULL && !last)
        || ( pObjIndex->count >= limit /* && number_range(0,4) != 0 */ )
        || (count = count_obj_list(pObjIndex,LastObj->contains)) 
        > pReset->arg4 )
        {
        last = FALSE;
        break;
        }

        while (count < pReset->arg4)
        {
            pObj = create_object( pObjIndex );
            obj_to_obj( pObj, LastObj );
        count++;
        if (pObjIndex->count >= limit)
            break;
        }
        /* fix object lock state! */
        LastObj->value[1] = LastObj->pIndexData->value[1];
        last = TRUE;
        break;

    case 'G':
    case 'E':
        if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) )
        {
        bug( "Reset_area: 'E' or 'G': bad vnum %d.", pReset->arg1 );
        continue;
        }

        if ( !last )
        break;

        if ( !LastMob )
        {
        bug( "Reset_area: 'E' or 'G': null mob for vnum %d.",
            pReset->arg1 );
        last = FALSE;
        break;
        }

        if ( LastMob->pIndexData->pShop )
        {
        pObj = create_object( pObjIndex );
        SET_BIT( pObj->extra_flags, ITEM_INVENTORY );
        }

        else
        {
        int limit;
        if (pReset->arg2 > 50) /* old format */
            limit = 6;
        else if (pReset->arg2 == -1 || pReset->arg2 == 0 )
            limit = 999;
        else
            limit = pReset->arg2;

        if (pObjIndex->count < limit || number_range(0,4) == 0)
        {
            pObj=create_object(pObjIndex);
        }
        else
            break;
        }
        obj_to_char( pObj, LastMob );
        if ( pReset->command == 'E' )
        equip_char( LastMob, pObj, pReset->arg3 );
        last = TRUE;
        break;

    case 'D':
        break;

    case 'R':
        if ( !( pRoomIndex = get_room_index( pReset->arg1 ) ) )
        {
        bug( "Reset_area: 'R': bad vnum %d.", pReset->arg1 );
        continue;
        }

        {
        EXIT_DATA *pExit;
        int d0;
        int d1;

        for ( d0 = 0; d0 < pReset->arg2 - 1; d0++ )
        {
            d1                   = number_range( d0, pReset->arg2-1 );
            pExit                = pRoomIndex->exit[d0];
            pRoomIndex->exit[d0] = pRoomIndex->exit[d1];
            pRoomIndex->exit[d1] = pExit;
        }
        }
        break;
    }
    }

    return;
}

/*
 * OLC
 * Reset one Area.
 */
void forced_reset_area( AREA_DATA *pArea )
{
    ROOM_INDEX_DATA *pRoom;
    int vnum = 0;

    for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
    {
    if ( ( pRoom = get_room_index(vnum) ) )
        forced_reset_room(pRoom);
    }

    return;
}


/*
 * Reset one room.  Called by reset_area.
 */
void reset_room( ROOM_INDEX_DATA *pRoom )
{
    RESET_DATA  *pReset;
    CHAR_DATA   *pMob;
    CHAR_DATA   *mob;
    OBJ_DATA    *pObj;
    CHAR_DATA   *LastMob = NULL;
    OBJ_DATA    *LastObj = NULL;
    int iExit = 0;
    bool last = FALSE;

    if ( !pRoom )
        return;

    pMob        = NULL;

    for ( iExit = 0;  iExit < MAX_DIR;  iExit++ )
    {
        EXIT_DATA *pExit;
        if ( ( pExit = pRoom->exit[iExit] )
                /*  && !IS_SET( pExit->exit_info, EX_BASHED )   ROM OLC */ )
        {
            pExit->exit_info = pExit->rs_flags;
            if ( ( pExit->u1.to_room != NULL )
                    && ( ( pExit = pExit->u1.to_room->exit[rev_dir[iExit]] ) ) )
            {
                /* nail the other side */
                pExit->exit_info = pExit->rs_flags;
            }
        }
    }

    for ( pReset = pRoom->reset_first; pReset != NULL; pReset = pReset->next )
    {
        MOB_INDEX_DATA  *pMobIndex;
        OBJ_INDEX_DATA  *pObjIndex;
        OBJ_INDEX_DATA  *pObjToIndex;
        ROOM_INDEX_DATA *pRoomIndex;
        int count,limit=0;

        switch ( pReset->command )
        {
        default:
            bug( "Reset_room: bad command %c.", pReset->command );
            break;

        case 'F':
            if ( !( pObjIndex = get_obj_index( pReset->arg1 ) )
                    && pReset->arg1 != 0)
            {
                bug( "Reset_area: 'F': bad vnum %d.", pReset->arg1 );
                bug(Format("%d %d %d %d",pReset->arg1, pReset->arg2, pReset->arg3, pReset->arg4),1);
                continue;
            }

            if ( !( pRoomIndex = get_room_index( pReset->arg3 ) ) )
            {
                bug( "Reset_area: 'F': bad vnum %d.", pReset->arg3 );
                bug(Format("%d %d %d %d",pReset->arg1,pReset->arg2,pReset->arg3, pReset->arg4),1);
                continue;
            }

            if ( pRoom->area->nplayer > 0
                    || ( pObjIndex != NULL
                            && count_obj_list( pObjIndex, pRoom->contents ) > 0)
                            || ( pObjIndex == NULL
                                    && pRoom->owner[0] != '\0'
                                            && is_online(pRoom->owner)) )
            {
                last = FALSE;
                break;
            }

            if( pObjIndex != NULL )
            {
                pObj       = create_object( pObjIndex );
                pObj->cost = 0;
                obj_to_room( pObj, pRoom );
                SET_BIT(pObj->extra2, OBJ_FURNISHING);
            }
            else
            {
                if(!is_online(pRoom->owner))
                {
                    if((mob = get_player(pRoom->owner)) != NULL)
                    {
                        /* @@@@@ place_home_room_obj(mob, pRoom); */
                        free_char(mob);
                    }
                }
            }
            last = TRUE;
            break;

        case 'M':
            if ( !( pMobIndex = get_mob_index( pReset->arg1 ) ) )
            {
                bug( "Reset_room: 'M': bad vnum %d.", pReset->arg1 );
                continue;
            }

            if ( ( pRoomIndex = get_room_index( pReset->arg3 ) ) == NULL )
            {
                bug( "Reset_area: 'R': bad vnum %d.", pReset->arg3 );
                continue;
            }
            if ( pMobIndex->count >= pReset->arg2 )
            {
                last = FALSE;
                break;
            }

            count = 0;
            for (mob = pRoomIndex->people; mob != NULL; mob = mob->next_in_room)
                if (mob->pIndexData == pMobIndex)
                {
                    count++;
                    if (count >= pReset->arg4)
                    {
                        last = FALSE;
                        break;
                    }
                }

            if (count > pReset->arg4)
                break;

            pMob = create_mobile( pMobIndex );

            char_to_room( pMob, pRoomIndex );
            LastMob = pMob;
            last = TRUE;
            break;

        case 'O':
            if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) )
            {
                bug( "Reset_area: 'O': bad vnum %d.", pReset->arg1 );
                bug((char *)Format("%d %d %d %d",pReset->arg1, pReset->arg2, pReset->arg3, pReset->arg4),1);
                continue;
            }

            if ( !( pRoomIndex = get_room_index( pReset->arg3 ) ) )
            {
                bug( "Reset_area: 'R': bad vnum %d.", pReset->arg3 );
                bug((char *)Format("%d %d %d %d",pReset->arg1,pReset->arg2,pReset->arg3, pReset->arg4),1);
                continue;
            }

            if ( pRoom->area->nplayer > 0
                    ||   count_obj_list( pObjIndex, pRoom->contents ) > 0 )
            {
                last = FALSE;
                break;
            }

            pObj       = create_object( pObjIndex );
            pObj->cost = 0;
            obj_to_room( pObj, pRoom );
            last = TRUE;
            break;

        case 'P':
            if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) )
            {
                bug( "Reset_area: 'P': bad vnum %d.", pReset->arg1 );
                continue;
            }

            if ( !( pObjToIndex = get_obj_index( pReset->arg3 ) ) )
            {
                bug( "Reset_area: 'P': bad vnum %d.", pReset->arg3 );
                continue;
            }

            if (pReset->arg2 > 50) /* old format */
                limit = 6;
            else if (pReset->arg2 == -1) /* no limit */
                limit = 999;
            else
                limit = pReset->arg2;

            if (pRoom->area->nplayer > 0
                    || (LastObj = get_obj_type( pObjToIndex ) ) == NULL
                    || (LastObj->in_room == NULL && !last)
                    || ( pObjIndex->count >= limit /* && number_range(0,4) != 0 */ )
                    || (count = count_obj_list(pObjIndex,LastObj->contains))
                    > pReset->arg4 )
            {
                last = FALSE;
                break;
            }

            while (count < pReset->arg4)
            {
                pObj = create_object( pObjIndex );
                obj_to_obj( pObj, LastObj );
                count++;
                if (pObjIndex->count >= limit)
                    break;
            }
            /* fix object lock state! */
            LastObj->value[1] = LastObj->pIndexData->value[1];
            last = TRUE;
            break;

        case 'G':
        case 'E':
            if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) )
            {
                bug( "Reset_area: 'E' or 'G': bad vnum %d.", pReset->arg1 );
                continue;
            }

            if ( !last )
                break;

            if ( !LastMob )
            {
                bug( "Reset_area: 'E' or 'G': null mob for vnum %d.", pReset->arg1 );
                last = FALSE;
                break;
            }

            if ( LastMob->pIndexData->pShop )
            {
                pObj = create_object( pObjIndex );
                SET_BIT( pObj->extra_flags, ITEM_INVENTORY );
            }

            else
            {
                int limit;
                if (pReset->arg2 > 50) /* old format */
                    limit = 6;
                else if (pReset->arg2 == -1 || pReset->arg2 == 0 )
                    limit = 999;
                else
                    limit = pReset->arg2;

                if (pObjIndex->count < limit || number_range(0,4) == 0)
                {
                    pObj=create_object(pObjIndex);
                }
                else
                    break;
            }
            obj_to_char( pObj, LastMob );
            if ( pReset->command == 'E' )
                equip_char( LastMob, pObj, pReset->arg3 );
            last = TRUE;
            break;

        case 'D':
            break;

        case 'R':
            if ( !( pRoomIndex = get_room_index( pReset->arg1 ) ) )
            {
                bug( "Reset_area: 'R': bad vnum %d.", pReset->arg1 );
                continue;
            }

            {
                EXIT_DATA *pExit;
                int d0;
                int d1;

                for ( d0 = 0; d0 < pReset->arg2 - 1; d0++ )
                {
                    d1                   = number_range( d0, pReset->arg2-1 );
                    pExit                = pRoomIndex->exit[d0];
                    pRoomIndex->exit[d0] = pRoomIndex->exit[d1];
                    pRoomIndex->exit[d1] = pExit;
                }
            }
            break;
        }
    }

    return;
}

/*
 * OLC
 * Reset one Area.
 */
void reset_area( AREA_DATA *pArea )
{
    ROOM_INDEX_DATA *pRoom;
    int vnum = 0;

    for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
    {
        if ( ( pRoom = get_room_index(vnum) ) )
            reset_room(pRoom);
    }

    return;
}


/*
 * Snarf a mob section.  new style
 */
void load_mobiles( FILE *fp )
{
    MOB_INDEX_DATA *pMobIndex;

    if ( !area_last )   /* OLC */
    {
        bug( "Load_mobiles: no #AREA seen yet.", 0 );
        exit( 1 );
    }

    for ( ; ; )
    {
        sh_int vnum;
        char letter;
        int iHash;

        letter                          = fread_letter( fp );
        if ( letter != '#' )
        {
            bug( "Load_mobiles: # not found.", 0 );
            exit( 1 );
        }

        vnum                            = fread_number( fp );
        if ( vnum == 0 )
            break;

        fBootDb = FALSE;
        if ( get_mob_index( vnum ) != NULL )
        {
            bug( "Load_mobiles: vnum %d duplicated.", vnum );
            exit( 1 );
        }
        fBootDb = TRUE;

        pMobIndex                       = new_mob_index();
        pMobIndex->vnum                 = vnum;
        pMobIndex->area                 = area_last;               /* OLC */
        newmobs++;
        pMobIndex->player_name          = fread_string( fp );
        pMobIndex->short_descr          = fread_string( fp );
        pMobIndex->long_descr           = fread_string( fp );
        pMobIndex->description          = fread_string( fp );
        pMobIndex->race                 = race_lookup(fread_string( fp ));

        if(!IS_NULLSTR(pMobIndex->long_descr))
        {
            ReplaceString(pMobIndex->long_descr, Upper(pMobIndex->long_descr));
        }
        if(!IS_NULLSTR(pMobIndex->description))
        {
            ReplaceString(pMobIndex->description, Upper(pMobIndex->description));
        }

        pMobIndex->act                  = fread_flag( fp ) | ACT_IS_NPC | race_table[pMobIndex->race].act;
        pMobIndex->affected_by          = fread_flag( fp ) | race_table[pMobIndex->race].aff;
        pMobIndex->pShop                = NULL;

        pMobIndex->level        = fread_number( fp );

        /* read damage dice */
        fread_number( fp ); /* Spare */
        fread_number( fp ); /* Spare */
        fread_number( fp ); /* Spare */
        pMobIndex->dam_type     = attack_lookup(fread_word(fp));

        /* read flags and add in data from the race table */
        pMobIndex->off_flags        = fread_flag( fp ) | race_table[pMobIndex->race].off;
        pMobIndex->imm_flags        = fread_flag( fp ) | race_table[pMobIndex->race].imm;
        pMobIndex->res_flags        = fread_flag( fp ) | race_table[pMobIndex->race].res;
        pMobIndex->vuln_flags       = fread_flag( fp ) | race_table[pMobIndex->race].vuln;

        /* vital statistics */
        pMobIndex->start_pos        = fread_number(fp);
        pMobIndex->default_pos      = fread_number(fp);
        pMobIndex->sex          = fread_number(fp);

        pMobIndex->wealth       = fread_number( fp );

        pMobIndex->form         = fread_flag( fp ) | race_table[pMobIndex->race].form;
        pMobIndex->parts        = fread_flag( fp ) | race_table[pMobIndex->race].parts;

        CHECK_POS( pMobIndex->size, size_lookup(fread_word(fp)), "size" );
        PURGE_DATA( pMobIndex->material );
        pMobIndex->material     = str_dup(fread_word( fp ));

        for ( ; ; )
        {
            letter = fread_letter( fp );

            if (letter == 'A')
            {
                pMobIndex->act2 = fread_flag(fp);
            }

        else if (letter == 'I')
        {
            pMobIndex->affected_by2 = fread_flag( fp );
        }

        else if (letter == 'F')
        {
            char *word;
            long vector;

            word                    = fread_word(fp);
            vector          = fread_flag(fp);

            if (!str_prefix(word,"act"))
                REMOVE_BIT(pMobIndex->act,vector);
            else if (!str_prefix(word,"aff"))
                REMOVE_BIT(pMobIndex->affected_by,vector);
            else if (!str_prefix(word,"aff2"))
                REMOVE_BIT(pMobIndex->affected_by2,vector);
            else if (!str_prefix(word,"off"))
                REMOVE_BIT(pMobIndex->off_flags,vector);
            else if (!str_prefix(word,"imm"))
                REMOVE_BIT(pMobIndex->imm_flags,vector);
            else if (!str_prefix(word,"res"))
                REMOVE_BIT(pMobIndex->res_flags,vector);
            else if (!str_prefix(word,"vul"))
                REMOVE_BIT(pMobIndex->vuln_flags,vector);
            else if (!str_prefix(word,"for"))
                REMOVE_BIT(pMobIndex->form,vector);
            else if (!str_prefix(word,"par"))
                REMOVE_BIT(pMobIndex->parts,vector);
            else
            {
                bug("Flag remove: flag not found.",0);
                exit(1);
            }
        }

        else if (letter == 'M')
        {
            MPROG_LIST *pMprog;
            char *word;
            int trigger = 0;

            pMprog = new_mprog();
        word                = fread_word( fp );
            if ( !(trigger = flag_lookup( word, mprog_flags )) )
            {
                bug("MOBprogs: invalid trigger.",0);
                exit(1);
            }
            SET_BIT( pMobIndex->mprog_flags, trigger );
            pMprog->trig_type   = trigger;
            pMprog->vnum        = fread_number( fp );
            pMprog->trig_phrase = fread_string( fp );
        LINK_SINGLE(pMprog, next, pMobIndex->mprogs);
        }

        else if (letter == 'T')
        {
            /* Read in scripts */
            SCRIPT_DATA *ps;

            ps = new_script();
            if((letter = fread_letter(fp)) == 'r')
                ps->delay = fread_number(fp);
            else
                ungetc(letter,fp);
            ps->trigger = fread_string(fp);
            ps->reaction = fread_string(fp);

        LINK_SINGLE(ps, next_in_event, pMobIndex->triggers);
        }

        else
        {
            ungetc(letter,fp);
            break;
        }
        }

    iHash                   = vnum % MAX_KEY_HASH;
    LINK_SINGLE(pMobIndex, next, mob_index_hash[iHash]);
    top_vnum_mob = top_vnum_mob < vnum ? vnum : top_vnum_mob; /* OLC */
    assign_area_vnum( vnum );   /* OLC */

    }

    return;
}

/*
 * Snarf an obj section. new style
 */
void load_objects( FILE *fp )
{
    OBJ_INDEX_DATA *pObjIndex;
    if ( !area_last )  /* OLC */
    {
        bug( "Load_objects: no #AREA seen yet.", 0 );
        exit( 1 );
    }
    for ( ; ; )
    {
        sh_int vnum;
        char letter;
        int iHash;
        letter = fread_letter( fp );
        if ( letter != '#' )
        {
            bug( "Load_objects: # not found.", 0 );
            exit( 1 );
        }

        vnum = fread_number( fp );

        if ( vnum == 0 )
            break;
        fBootDb = FALSE;

        if ( get_obj_index( vnum ) != NULL )
        {
            bug( "Load_objects: vnum %d duplicated.", vnum );
            exit( 1 );
        }

        fBootDb = TRUE;
        pObjIndex           = new_obj_index();
        pObjIndex->vnum     = vnum;
        pObjIndex->area     = area_last;            /* OLC */
        pObjIndex->reset_num = 0;
        newobjs++;
        PURGE_DATA(pObjIndex->name);
        PURGE_DATA(pObjIndex->short_descr);
        PURGE_DATA(pObjIndex->description);
        PURGE_DATA(pObjIndex->full_desc);
        PURGE_DATA(pObjIndex->material);
        pObjIndex->name             = fread_string( fp );
        pObjIndex->short_descr      = fread_string( fp );
        pObjIndex->description      = fread_string( fp );
        pObjIndex->full_desc        = fread_string( fp );
        pObjIndex->material         = fread_string( fp );
        pObjIndex->item_type        = fread_number( fp );

        pObjIndex->extra_flags      = fread_flag( fp );
        pObjIndex->wear_flags       = fread_flag( fp );
        switch(pObjIndex->item_type)
        {
        case ITEM_WEAPON:
            pObjIndex->value[0] = fread_number(fp);
            pObjIndex->value[1] = fread_number(fp);
            pObjIndex->value[2] = fread_number(fp);
            pObjIndex->value[3] = attack_lookup(fread_word(fp));
            pObjIndex->value[4] = fread_flag(fp);
            pObjIndex->value[5] = fread_number(fp);
            break;
        case ITEM_CONTAINER:
            pObjIndex->value[0] = fread_number(fp);
            pObjIndex->value[1] = fread_flag(fp);
            pObjIndex->value[2] = fread_number(fp);
            pObjIndex->value[3] = fread_number(fp);
            pObjIndex->value[4] = fread_number(fp);
            pObjIndex->value[5] = fread_number(fp);
            break;
        case ITEM_DRINK_CON:
        case ITEM_FOUNTAIN:
            pObjIndex->value[0]        = fread_number(fp);
            pObjIndex->value[1]        = fread_number(fp);
            CHECK_POS(pObjIndex->value[2],liq_lookup(fread_word(fp)), "liq_lookup");
            pObjIndex->value[3]        = fread_number(fp);
            pObjIndex->value[4]        = fread_number(fp);
            pObjIndex->value[5] = fread_number(fp);
            break;
        case ITEM_POTION:
        case ITEM_PILL:
        case ITEM_SCROLL:
            pObjIndex->value[0] = fread_number(fp);
            pObjIndex->value[1] = skill_lookup(fread_word(fp));
            pObjIndex->value[2] = skill_lookup(fread_word(fp));
            pObjIndex->value[3] = skill_lookup(fread_word(fp));
            pObjIndex->value[4] = skill_lookup(fread_word(fp));
            pObjIndex->value[5] = fread_number(fp);
            break;
        default:
            pObjIndex->value[0]            = fread_flag( fp );
            pObjIndex->value[1]            = fread_flag( fp );
            pObjIndex->value[2]            = fread_flag( fp );
            pObjIndex->value[3]            = fread_flag( fp );
            pObjIndex->value[4]    = fread_flag( fp );
            pObjIndex->value[5]    = fread_flag( fp );
            break;
        }
        pObjIndex->weight              = fread_number( fp );
        pObjIndex->cost                = fread_number( fp );
        pObjIndex->condition = fread_number( fp );
        if(pObjIndex->condition <= 0)
            pObjIndex->condition            = 100;
        for ( ; ; )
        {
            char letter;
            letter = fread_letter( fp );
            if ( letter == 'A' )
            {
                AFFECT_DATA *paf;
                ALLOC_DATA(paf, AFFECT_DATA, 1);
                paf->where = TO_OBJECT;
                paf->type              = -1;
                paf->duration          = -1;
                paf->location          = fread_number( fp );
                paf->modifier          = fread_number( fp );
                paf->bitvector          = 0;

                LINK_SINGLE(paf, next, pObjIndex->affected);
                top_affect++;
            }
            else if (letter == 'F')
            {
                AFFECT_DATA *paf;
                ALLOC_DATA(paf, AFFECT_DATA, 1);
                letter = fread_letter(fp);
                switch (letter)
                {
                case 'A':
                    paf->where          = TO_AFFECTS;
                    break;
                case 'I':
                    paf->where = TO_IMMUNE;
                    break;
                case 'R':
                    paf->where = TO_RESIST;
                    break;
                case 'V':
                    paf->where = TO_VULN;
                    break;
                default:
                    bug( "Load_objects: Bad where on flag set.", 0 );
                    exit( 1 );
                }
                paf->type              = -1;
                paf->duration          = -1;
                paf->location          = fread_number(fp);
                paf->modifier          = fread_number(fp);
                paf->bitvector          = fread_flag(fp);

                LINK_SINGLE(paf, next, pObjIndex->affected);
                top_affect++;
            }
            else if ( letter == 'E' )
            {
                EXTRA_DESCR_DATA *ed = NULL;
                ed                    = new_extra_descr();
                assert(ed); // crash if our pointer is messed up.
                ed->keyword            = fread_string( fp );
                ed->description        = fread_string( fp );
                if(!str_cmp(ed->keyword, "--TUNONE")) {
                    PURGE_DATA(pObjIndex->to_user_none);
                    pObjIndex->to_user_none = str_dup(ed->description);
                    free_extra_descr(ed);
                    if(pObjIndex->uses < -1) pObjIndex->uses = -1;
                }
                else if(!str_cmp(ed->keyword, "--TUSELF")) {
                    PURGE_DATA(pObjIndex->to_user_self);
                    pObjIndex->to_user_self = str_dup(ed->description);
                    free_extra_descr(ed);
                    if(pObjIndex->uses < -1) pObjIndex->uses = -1;
                }
                else if(!str_cmp(ed->keyword, "--TUOTH")) {
                    PURGE_DATA(pObjIndex->to_user_other);
                    pObjIndex->to_user_other = str_dup(ed->description);
                    free_extra_descr(ed);
                    if(pObjIndex->uses < -1) pObjIndex->uses = -1;
                }
                else if(!str_cmp(ed->keyword, "--TRNONE")) {
                    PURGE_DATA(pObjIndex->to_room_none);
                    pObjIndex->to_room_none = str_dup(ed->description);
                    free_extra_descr(ed);
                    if(pObjIndex->uses < -1) pObjIndex->uses = -1;
                }
                else if(!str_cmp(ed->keyword, "--TRSELF")) {
                    PURGE_DATA(pObjIndex->to_room_self);
                    pObjIndex->to_room_self = str_dup(ed->description);
                    free_extra_descr(ed);
                    if(pObjIndex->uses < -1) pObjIndex->uses = -1;
                }
                else if(!str_cmp(ed->keyword, "--TROTH")) {
                    PURGE_DATA(pObjIndex->to_room_other);
                    pObjIndex->to_room_other = str_dup(ed->description);
                    free_extra_descr(ed);
                    if(pObjIndex->uses < -1) pObjIndex->uses = -1;
                }
                else if(!str_cmp(ed->keyword, "--TVOTH")) {
                    PURGE_DATA(pObjIndex->to_vict_other);
                    pObjIndex->to_vict_other = str_dup(ed->description);
                    free_extra_descr(ed);
                    if(pObjIndex->uses < -1) pObjIndex->uses = -1;
                }
                else if(!str_cmp(ed->keyword, "--TRUSE")) {
                    PURGE_DATA(pObjIndex->to_room_used);
                    pObjIndex->to_room_used = str_dup(ed->description);
                    free_extra_descr(ed);
                    if(pObjIndex->uses < -1) pObjIndex->uses = -1;
                }
                else if(!str_cmp(ed->keyword, "--TUUSE")) {
                    PURGE_DATA(pObjIndex->to_user_used);
                    pObjIndex->to_user_used = str_dup(ed->description);
                    free_extra_descr(ed);
                    if(pObjIndex->uses < -1) pObjIndex->uses = -1;
                }
                else if(!str_cmp(ed->keyword, "--USES")) {
                    if(ed->description == NULL || ed->description[0] == '\0') {
                        logfmt("Description is empty for extra descr data received on uses for vnum: ", vnum);
                        exit(1);
                    }
                    if(!is_number(ed->description)) {
                        logfmt("Description is not a number for vnum: %d", vnum);
                        exit(1);
                    }
                    pObjIndex->uses = atoi(ed->description);
                    free_extra_descr(ed);
                }
                else if(!str_cmp(ed->keyword, "--COMPANY")) {
                    PURGE_DATA(pObjIndex->company);
                    pObjIndex->company = str_dup(ed->description);
                    free_extra_descr(ed);
                }
                else {
                    LINK_SINGLE(ed, next, pObjIndex->extra_descr);
                    top_ed++;
                }
            }
            else if ( letter == 'S' )
            {
                pObjIndex->fetish_flags = fread_flag(fp);
                pObjIndex->fetish_flags = fread_number(fp);
                pObjIndex->fetish_target = fread_number(fp);
            }
            else if ( letter == 'Q' )
            {
                pObjIndex->quality = fread_number(fp);
            }
            else
            {
                ungetc( letter, fp );
                break;
            }
        }
        iHash                  = vnum % MAX_KEY_HASH;
        LINK_SINGLE(pObjIndex, next, obj_index_hash[iHash]);
        top_vnum_obj = top_vnum_obj < vnum ? vnum : top_vnum_obj; /* OLC */
        assign_area_vnum( vnum ); /* OLC */
    }
    return;
}

PLOT_DATA *get_plot_index args( (int vnum) );
EVENT_DATA *get_event_index args( (int vnum) );
SCRIPT_DATA *get_script_index args( (int vnum) );
PERSONA *get_persona_index args( (int vnum) );

/*
 * Snarf a script section.
 */
void load_scripts( FILE *fp )
{
    SCRIPT_DATA *pScript;
 
    if ( !event_list )
    {
        bug( "Load_scripts: no event seen yet.", 0 );
        exit( 1 );
    }

    for ( ; ; )
    {
        sh_int vnum;
        char letter;
 
        letter                          = fread_letter( fp );
        if ( letter != 'S' )
        {
            bug( "Load_scripts: S not found.", 0 );
            exit( 1 );
        }
 
        vnum                            = fread_number( fp );
        if ( vnum == 0 )
            break;
 
        fBootDb = FALSE;
        if ( get_script_index( vnum ) != NULL )
        {
            bug( "Load_scripts: vnum %d duplicated.", vnum );
            exit( 1 );
        }
        fBootDb = TRUE;
 
        ALLOC_DATA(pScript,SCRIPT_DATA, 1);
        pScript->vnum = vnum;
        pScript->author = fread_string( fp );
        pScript->actor = fread_number( fp );
        pScript->delay = fread_number( fp );
        pScript->first_script = fread_number( fp );
        pScript->trigger = fread_string( fp );
        pScript->reaction = fread_string( fp );

    pScript->event = event_list;

    LINK_SINGLE(pScript, next_in_event, event_list->script_list);

    LINK_SINGLE(pScript, next, script_first);
    }
 
    return;
}

/*
 * Snarf a script section.
 */
void load_actors( FILE *fp )
{
    ACTOR_DATA *pActor;

    if ( !event_list )
    {
        bug( "Load_actors: no event seen yet.", 0 );
        exit( 1 );
    }

    for ( ; ; )
    {
        sh_int vnum;
        char letter;

        letter                          = fread_letter( fp );
        if ( letter != 'A' )
        {
            bug( "Load_actors: A not found.", 0 );
            exit( 1 );
        }

        vnum                            = fread_number( fp );
        if ( vnum == 0 )
            break;

        ALLOC_DATA(pActor, ACTOR_DATA, 1);
        pActor->mob = vnum;

    LINK_SINGLE(pActor, next, event_list->actors);
    }
 
    return;
}

/*
 * Snarf a event section.
 */
void load_events( FILE *fp )
{
    EVENT_DATA *pEvent;

    if ( !plot_list )
    {
        bug( "Load_events: no plot seen yet.", 0 );
        exit( 1 );
    }

    for ( ; ; )
    {
        sh_int vnum;
        char letter;

        letter                          = fread_letter( fp );
        if ( letter != 'E' )
        {
            bug( "Load_events: E not found.", 0 );
            exit( 1 );
        }

        vnum                            = fread_number( fp );
        if ( vnum == 0 )
            break;

        fBootDb = FALSE;
        if ( get_event_index( vnum ) != NULL )
        {
            bug( "Load_events: vnum %d duplicated.", vnum );
            exit( 1 );
        }
        fBootDb = TRUE;

        ALLOC_DATA(pEvent, EVENT_DATA, 1);
        pEvent->vnum = vnum;
        //event_last = pEvent;
        pEvent->title = fread_string( fp );
        pEvent->author = fread_string( fp );
        pEvent->races = fread_flag( fp );
        pEvent->loc = fread_number(fp);

                LINK_SINGLE(pEvent, next_in_plot, plot_list->event_list);
                LINK_SINGLE(pEvent, next, event_list);


        for ( ; ; )
        {
            letter = fread_letter( fp );

            if (letter == 'A')
            {
                ungetc(letter,fp);
                load_actors(fp);
            }
            else if (letter == 'S')
            {
                ungetc(letter,fp);
                load_scripts(fp);
            }
            else
            {
                ungetc(letter,fp);
                break;
            }
        }

    }

    return;
}

/*
 * Snarf a plot section.
 */
void load_plots( FILE *fp )
{
    PLOT_DATA *pPlot;

    for ( ; ; )
    {
        sh_int vnum;
        char letter;

        letter                          = fread_letter( fp );
        if ( letter != '#' )
        {
            bug( "Load_plots: # not found.", 0 );
            exit( 1 );
        }

        vnum                            = fread_number( fp );
        if ( vnum == 0 )
            break;

        fBootDb = FALSE;
        if ( get_plot_index( vnum ) != NULL )
        {
            bug( "Load_plots: vnum %d duplicated.", vnum );
            exit( 1 );
        }
        fBootDb = TRUE;

        ALLOC_DATA(pPlot, PLOT_DATA, 1);
        pPlot->vnum = vnum;
        //plot_last = pPlot;
        pPlot->title = fread_string( fp );
        pPlot->author = fread_string( fp );
        pPlot->races = fread_flag( fp );

                LINK_SINGLE(pPlot, next, plot_list);
        for ( ; ; )
        {
            letter = fread_letter( fp );

            if (letter == 'E')
            {
                ungetc(letter,fp);
                load_events(fp);
            }
            else
            {
                ungetc(letter,fp);
                break;
            }
        }
    }

    return;
}

/*
 * Snarf a trigger/reaction set.
 */
void load_reacts( FILE *fp )
{
    REACT *pr;
    REACT_DATA *prd;

    if ( !persona_last )
    {
        bug( "Load_reacts: no persona seen yet.", 0 );
        exit( 1 );
    }

    for ( ; ; )
    {
        sh_int vnum;
        char letter;

        letter                          = fread_letter( fp );
        if ( letter != 'T' )
        {
            bug( "Load_reacts: T not found.", 0 );
            exit( 1 );
        }

        vnum                            = fread_number( fp );
        if ( vnum == 0 )
            break;

        fBootDb = FALSE;
        if ( vnum < 0 || vnum > MAX_ATTITUDE )
        {
            bug( "Load_reacts: Invalid attitude %d.", vnum );
            exit( 1 );
        }
        fBootDb = TRUE;

        ALLOC_DATA(pr, REACT, 1);
        pr->attitude    = vnum;
        pr->trig = fread_string( fp );
        for ( ; ; )
        {
            letter = fread_letter( fp );

            if (letter == 'R')
            {
                ALLOC_DATA(prd, REACT_DATA, 1);
                prd->priority = fread_number( fp );
                prd->reaction = fread_string( fp );
                prd->next = pr->react;
                pr->react = prd;
            }
            else if (letter == 'E')
            {
                ungetc(letter,fp);
                if(!str_cmp(fread_word(fp), "End"))
                    break;
            }
            else
            {
                ungetc(letter,fp);
                break;
            }
        }

        pr->next_in_matrix_loc = persona_last->matrix[pr->attitude];
        persona_last->matrix[pr->attitude] = pr;
        pr->next  = react_first;
        react_first   = pr;
        persona_last    = NULL;
    }

    return;
}

/*
 * Snarf a personality section.
 */
void load_personas( FILE *fp )
{
    PERSONA *pPersona;
    for ( ; ; )
    {
        int vnum;
        char letter;

        letter                          = fread_letter( fp );
        if ( letter != '#' )
        {
            bug( "Load_personas: # not found.", 0 );
            exit( 1 );
        }

        vnum                            = fread_number( fp );
        if ( vnum == 0 )
            break;

        fBootDb = FALSE;
        if ( get_persona_index( vnum ) != NULL )
        {
            bug( "Load_personas: vnum %d duplicated.", vnum );
            exit( 1 );
        }
        fBootDb = TRUE;

        ALLOC_DATA(pPersona, PERSONA, 1);
        pPersona->vnum = vnum;
        persona_last = pPersona;
        pPersona->name = fread_string( fp );
        for ( ; ; )
        {
            letter = fread_letter( fp );

            if (letter == 'T')
            {
                ungetc(letter,fp);
                load_reacts(fp);
            }
            else
            {
                ungetc(letter,fp);
                break;
            }
        }

        pPersona->next  = persona_first;
        persona_first   = pPersona;
        persona_last    = NULL;
    }

    return;
}


/*
 * Loading system for election data.
 */
void load_votes(void)
{
    FILE *fp;
    char *word;
    char tmp[MSL]={'\0'};
    int mcount = 0, acount = 0, jcount = 0;
    int i = 0, j = 0;
    PACK_DATA *pack;

    last_election_result = current_time;
    next_election_result = current_time + 14*24*60*60;
    tax_rate = 0;
    for(i=0;i<MAX_OFFICES;i++)
    {
        for(j=0;j<MAX_NOMINEES;j++)
        {
            vote_tally[i][j].name = str_dup("None");
            vote_tally[i][j].votes = 0;
        }
    }

    incumbent_mayor = str_dup("None");
    for(j=0;j<MAX_JUDGES;j++)
    {
        incumbent_judge[j] = str_dup("None");
    }
    for(j=0;j<MAX_ALDERMEN;j++)
    {
        incumbent_alder[j] = str_dup("None");
    }
    incumbent_pchief = str_dup("None");

    i=0;j=0;

    if ( ( fp = fopen( "votes.txt", "r" ) ) == NULL )
    {
    bug( "Load_votes: file open fail.", 0 );
    return;
    }

    for ( ; ; )
    {
    strncpy( tmp, fread_word( fp ), sizeof(tmp) );
    if ( tmp[1] == '$' )
        break;

    if ( tmp[0] == '#' )
    {
        switch(tmp[1])
        {
        case 'A':
            if(acount < MAX_NOMINEES)
            {
                PURGE_DATA(vote_tally[2][acount].name);
            vote_tally[2][acount].name = str_dup(fread_word(fp));
            vote_tally[2][acount].votes = fread_number(fp);
            acount++;
            }
            break;
        case 'I':
            word = fread_word(fp);
            if(!str_cmp(word, "mayor"))
            {
            incumbent_mayor = fread_string(fp);
            }
            else if(!str_cmp(word, "alderman"))
            {
            if(i < MAX_ALDERMEN)
            {
            incumbent_alder[i] = fread_string(fp);
            i++;
            }
            }
            else if(!str_cmp(word, "judge"))
            {
            if(j < MAX_JUDGES)
            {
            incumbent_judge[j] = fread_string(fp);
            j++;
            }
            }
            else if(!str_cmp(word, "pchief"))
            incumbent_pchief = fread_string(fp);
            break;
        case 'J':
            if(jcount < MAX_NOMINEES)
            {
                PURGE_DATA(vote_tally[1][jcount].name);
            vote_tally[1][jcount].name = str_dup(fread_word(fp));
            vote_tally[1][jcount].votes = fread_number(fp);
            jcount++;
            }
            break;
        case 'M':
            if(mcount < MAX_NOMINEES)
            {
                PURGE_DATA(vote_tally[0][mcount].name);
            vote_tally[0][mcount].name = str_dup(fread_word(fp));
            vote_tally[0][mcount].votes = fread_number(fp);
            mcount++;
            }
            break;
        case 'P':
            pack         = new_pack();
            pack->name       = fread_string(fp);
            pack->totem      = fread_flag(fp);
            LINK_SINGLE(pack, next, pack_list);
            break;
        case 'R':
            last_election_result = fread_number(fp);
            next_election_result = fread_number(fp);
            break;
        case 'T':
            tax_rate         = fread_number(fp);
            break;
        default:
            break;
        }
    }
    }

    fclose(fp);
}



/*
 * Snarf a clan section.
 */
void load_org_members( FILE *fp )
{
    ORGMEM_DATA *mem;
 
    if ( !org_last )   /* OLC */
    {
        bug( "Load_org members: no #ORG seen yet.", 0 );
        exit( 1 );
    }

    for ( ; ; )
    {
        char letter;
 
        letter                          = fread_letter( fp );
        if ( letter != '#' )
        {
            bug( "Load_org_members: # not found.", 0 );
            exit( 1 );
        }
        letter = fread_letter( fp );
        if ( letter == '$' )
        {
        break;
        }
    else
    {
        ungetc(letter,fp);
    }

    mem             = new_orgmem(); 
        mem->name           = fread_string( fp );
        mem->status         = fread_number( fp );
    mem->auth_flags         = fread_flag( fp );
    mem->status_votes       = fread_number( fp );
 
    for ( ; ; )
        {
            letter = fread_letter( fp );

        if (letter == 'A')
        {
        }

        else
        {
        ungetc(letter,fp);
        break;
        }
    }

        mem->next     = org_last->members;
        org_last->members = mem;
    }
 
    return;
}


/*
 * Adds a reset to a room. OLC
 * Similar to add_reset in olc.c
 */
void new_reset( ROOM_INDEX_DATA *pR, RESET_DATA *pReset )
{
    RESET_DATA *pr;

    if(!pR) {
    logfmt("new_reset called with NULL room!");
    free_reset_data(pReset);
        return;
    }
    if(!pReset) {
    logfmt("new_reset called with NULL pReset");
    return;
    }

    pr = pR->reset_last;

    if(!pr)
    {
    pR->reset_first = pReset;
    pR->reset_last = pReset;
    }
    else
    {
    pR->reset_last->next = pReset;
    pR->reset_last       = pReset;
    pR->reset_last->next = NULL;
    }

    top_reset++;
    return;
}


/*
 * Create an instance of a mobile.
 */
CHAR_DATA *create_mobile( MOB_INDEX_DATA *pMobIndex )
{
    CHAR_DATA *mob;
    int i = 0;
    AFFECT_DATA af;

    mobile_count++;

    if ( pMobIndex == NULL )
    {
        pMobIndex = get_mob_index(MOB_VNUM_BLANKY);
    }

    mob = new_char();

    mob->pIndexData = pMobIndex;

    mob->timer      = -1;

    PURGE_DATA( mob->name );
    PURGE_DATA( mob->short_descr );
    PURGE_DATA( mob->long_descr );
    PURGE_DATA( mob->description );
    mob->name       = str_dup( pMobIndex->player_name );    /* OLC */
    mob->short_descr    = str_dup( pMobIndex->short_descr );    /* OLC */
    if(!IS_NULLSTR(pMobIndex->long_descr))
        mob->long_descr = str_dup( pMobIndex->long_descr );     /* OLC */

    if(!IS_NULLSTR(pMobIndex->description))
        mob->description    = str_dup( pMobIndex->description );    /* OLC */

    mob->id             = get_mob_id();
    mob->spec_fun       = pMobIndex->spec_fun;
    mob->prompt         = NULL;
    mob->mprog_target   = NULL;

    if (pMobIndex->wealth == 0)
    {
        mob->dollars = 0;
        mob->cents   = 0;
    }
    else
    {
        long wealth;

        wealth = number_range(pMobIndex->wealth/2, 3 * pMobIndex->wealth/2);
        mob->dollars = number_range(wealth/2,wealth);
        mob->cents = 0;
    } 

    /* read from prototype */
    mob->group      = pMobIndex->group;
    mob->act        = pMobIndex->act;
    mob->act2       = pMobIndex->act2;
    mob->comm       = COMM_NOCHANNELS|COMM_NOSHOUT|COMM_NOTELL;
    mob->plr_flags      = PLR_AUTOEXIT;
    mob->affected_by    = pMobIndex->affected_by;
    mob->affected_by2   = pMobIndex->affected_by2;
    mob->health     = pMobIndex->health;
    mob->dam_type       = pMobIndex->dam_type;
        if (mob->dam_type == 0)
            switch(number_range(1,3))
            {
                case (1): mob->dam_type = 3;        break;  /* slash */
                case (2): mob->dam_type = 7;        break;  /* pound */
                case (3): mob->dam_type = 11;       break;  /* pierce */
            }
    mob->off_flags      = pMobIndex->off_flags;
    mob->imm_flags      = pMobIndex->imm_flags;
    mob->res_flags      = pMobIndex->res_flags;
    mob->vuln_flags     = pMobIndex->vuln_flags;
    mob->start_pos      = pMobIndex->start_pos;
    mob->default_pos    = pMobIndex->default_pos;
    mob->sex        = pMobIndex->sex;
        if (mob->sex == 3) /* random sex */
            mob->sex = number_range(1,2);
    mob->race       = pMobIndex->race;
    mob->form       = pMobIndex->form;
    mob->parts      = pMobIndex->parts;
    mob->size       = pMobIndex->size;
    PURGE_DATA( mob->material );
    mob->material       = str_dup(pMobIndex->material);

    /* computed on the spot */

        for (i = 0; i < MAX_STATS; i ++)
            mob->perm_stat[i] = UMAX(number_fuzzy(pMobIndex->level/25) + 1, 1);
    
        if (IS_SET(mob->off_flags,OFF_FAST) && mob->perm_stat[STAT_DEX] <= 3)
            mob->perm_stat[STAT_DEX] += 2;
            
        if (IS_SET(mob->off_flags,OFF_TOUGH) && mob->perm_stat[STAT_STA] <= 3)
            mob->perm_stat[STAT_STA] += 2;
            
        if (IS_SET(mob->off_flags,OFF_STRONG) && mob->perm_stat[STAT_STR] <= 3)
            mob->perm_stat[STAT_STR] += 2;
            
        mob->perm_stat[STAT_STR] += mob->size - SIZE_MEDIUM;
        mob->perm_stat[STAT_STA] += (mob->size - SIZE_MEDIUM) / 2;
        mob->perm_stat[STAT_DEX] -= (mob->size - SIZE_MEDIUM) / 2;

    mob->perm_stat[STAT_STR] = UMAX(mob->perm_stat[STAT_STR], 1);
    mob->perm_stat[STAT_DEX] = UMAX(mob->perm_stat[STAT_DEX], 1);
    mob->perm_stat[STAT_STA] = UMAX(mob->perm_stat[STAT_STA], 1);

        for (i = 0; i < MAX_ABIL; i++)
    {
        mob->ability[i].value = dice_rolls(mob, 7, 10 - pMobIndex->level / 25);
        if (mob->ability[i].value > 5)
        mob->ability[i].value = 5;
        if (mob->ability[i].value < 0)
        mob->ability[i].value = 0;
    }

    /* let's get some spell action */
    if (IS_AFFECTED(mob,AFF_PROTECT_EVIL))
    {
        af.where     = TO_AFFECTS;
        af.type  = skill_lookup("protection evil");
        af.level     = 1;
        af.duration  = -1;
        af.location  = APPLY_STA;
        af.modifier  = -1;
        af.bitvector = AFF_PROTECT_EVIL;
        affect_to_char(mob,&af);
    }

    mob->position = mob->start_pos;

    /* Randomise the mob name */
    rand_name(mob);

    /* link the mob to the world list */
    LINK_SINGLE(mob, next, char_list);

    pMobIndex->count++;
    return mob;
}

/* duplicate a mobile exactly -- except inventory */
void clone_mobile(CHAR_DATA *parent, CHAR_DATA *clone)
{
    int i = 0;
    AFFECT_DATA *paf;

    if ( parent == NULL || clone == NULL || !IS_NPC(parent))
    return;
    
    /* start fixing values */
    PURGE_DATA( clone->name );
    clone->name     = str_dup(parent->name);
    clone->version  = parent->version;
    PURGE_DATA( clone->short_descr );
    PURGE_DATA( clone->long_descr );
    PURGE_DATA( clone->description );
    clone->short_descr  = str_dup(parent->short_descr);
    clone->long_descr   = str_dup(parent->long_descr);
    clone->description  = str_dup(parent->description);
    clone->group    = parent->group;
    clone->sex      = parent->sex;
    clone->clan     = parent->clan;
    clone->race     = parent->race;
    clone->trust    = 0;
    clone->timer    = parent->timer;
    clone->wait     = parent->wait;
    clone->health   = parent->health;
    clone->dollars  = parent->dollars;
    clone->cents    = parent->cents;
    clone->exp      = parent->exp;
    clone->act      = parent->act;
    clone->act2     = parent->act2;
    clone->comm     = parent->comm;
    clone->imm_flags    = parent->imm_flags;
    clone->res_flags    = parent->res_flags;
    clone->vuln_flags   = parent->vuln_flags;
    clone->invis_level  = parent->invis_level;
    clone->affected_by  = parent->affected_by;
    clone->affected_by2 = parent->affected_by2;
    clone->position = parent->position;
    clone->saving_throw = parent->saving_throw;
    clone->form     = parent->form;
    clone->parts    = parent->parts;
    clone->size     = parent->size;
    PURGE_DATA( clone->material );
    clone->material = str_dup(parent->material);
    clone->off_flags    = parent->off_flags;
    clone->dam_type = parent->dam_type;
    clone->start_pos    = parent->start_pos;
    clone->default_pos  = parent->default_pos;
    clone->spec_fun = parent->spec_fun;
    
    for (i = 0; i < 4; i++)
        clone->armor[i] = parent->armor[i];

    for (i = 0; i < MAX_STATS; i++)
    {
    clone->perm_stat[i] = parent->perm_stat[i];
    clone->mod_stat[i]  = parent->mod_stat[i];
    }

    for (i = 0; i < 3; i++)
    clone->damage[i]    = parent->damage[i];

    /* now add the affects */
    for (paf = parent->affected; paf != NULL; paf = paf->next)
        affect_to_char(clone,paf);

}


/*
 * Create an instance of an object.
 */
OBJ_DATA *create_object( OBJ_INDEX_DATA *pObjIndex )
{
    AFFECT_DATA *paf;
    OBJ_DATA *obj, *pool;

    if ( pObjIndex == NULL )
    {
        bug( "Create_object: NULL pObjIndex.", 0 );
        exit( 1 );
    }

    obj = new_obj();

    obj->pIndexData = pObjIndex;
    obj->in_room    = NULL;
    obj->enchanted  = FALSE;

    obj->wear_loc   = -1;

    PURGE_DATA( obj->name );
    PURGE_DATA( obj->short_descr );
    PURGE_DATA( obj->description );
    PURGE_DATA( obj->full_desc );
    PURGE_DATA( obj->material );
    PURGE_DATA( obj->owner );
    obj->name       = str_dup( pObjIndex->name );           /* OLC */
    obj->short_descr    = str_dup( pObjIndex->short_descr );    /* OLC */
    if(!IS_NULLSTR(pObjIndex->description))
        CopyTo( obj->description, pObjIndex->description );
    else
        obj->description = str_dup("This object needs a long description.");
    
    if(!IS_NULLSTR(pObjIndex->full_desc))
        CopyTo( obj->full_desc, pObjIndex->full_desc );
    else
        obj->full_desc = str_dup("This object needs a full description.");
    obj->material   = str_dup(pObjIndex->material);
    obj->owner      = NULL;
    obj->item_type  = pObjIndex->item_type;
    obj->extra_flags    = pObjIndex->extra_flags;
    obj->extra2     = pObjIndex->extra2;
    obj->wear_flags = pObjIndex->wear_flags;
    obj->value[0]   = pObjIndex->value[0];
    obj->value[1]   = pObjIndex->value[1];
    obj->value[2]   = pObjIndex->value[2];
    obj->value[3]   = pObjIndex->value[3];
    obj->value[4]   = pObjIndex->value[4];
    obj->weight     = pObjIndex->weight;
    obj->condition  = pObjIndex->condition;
    obj->quality    = pObjIndex->quality;

    obj->cost       = pObjIndex->cost;
    obj->fetish_flags   = pObjIndex->fetish_flags;
    obj->fetish_level   = pObjIndex->fetish_level;
    obj->fetish_target  = pObjIndex->fetish_target;

    PURGE_DATA( obj->to_user_none );
    CopyTo(obj->to_user_none,pObjIndex->to_user_none);
    PURGE_DATA( obj->to_user_self );
    CopyTo(obj->to_user_self, pObjIndex->to_user_self);
    PURGE_DATA( obj->to_user_other );
    CopyTo(obj->to_user_other, pObjIndex->to_user_other);
    PURGE_DATA( obj->to_room_none );
    CopyTo(obj->to_room_none, pObjIndex->to_room_none );
    PURGE_DATA( obj->to_room_self );
    CopyTo(obj->to_room_self, pObjIndex->to_room_self );
    PURGE_DATA( obj->to_room_other );
    CopyTo(obj->to_room_other, pObjIndex->to_room_other );
    PURGE_DATA( obj->to_vict_other );
    CopyTo( obj->to_vict_other, pObjIndex->to_vict_other );
    PURGE_DATA( obj->to_room_used );
    CopyTo( obj->to_room_used, pObjIndex->to_room_used );
    PURGE_DATA( obj->to_user_used );
    CopyTo( obj->to_user_used, pObjIndex->to_user_used );
    obj->uses           = pObjIndex->uses;
    obj->timer          = -1;

    /*
     * Mess with object properties.
     */
     switch ( obj->item_type )
     {
     default:
         bug( "Read_object: vnum %d bad type.", pObjIndex->vnum );
         break;

     case ITEM_LIGHT:
         if (obj->value[2] == 999)
             obj->value[2] = -1;
         break;

         /* ADD CONVERSION FROM LIQUIDS TO LIQUID OBJECTS HERE @@@@@ */
     case ITEM_DRINK_CON:
         pool = create_puddle_from_liq(liq_table[obj->value[2]].liq_name, OBJ_VNUM_POOL,
                 obj->value[1]);
         obj->item_type = ITEM_CONTAINER;
         SET_BIT(obj->extra2, OBJ_WATERPROOF);
         obj->value[0] = obj->value[1]
                                    * material_table[material_lookup(liq_table[obj->value[2]].liq_name)].weight;
         obj->value[1] = 0;
         obj->value[3] = obj->value[0];
         obj->value[2] = 0;
         obj_to_obj(pool, obj);
         break;

     case ITEM_FURNITURE:
     case ITEM_TRASH:
     case ITEM_CONTAINER:
     case ITEM_KEY:
     case ITEM_FOOD:
     case ITEM_BOAT:
     case ITEM_CORPSE_NPC:
     case ITEM_CORPSE_PC:
     case ITEM_FOUNTAIN:
     case ITEM_MAP:
     case ITEM_CLOTHING:
     case ITEM_PORTAL:
     case ITEM_TELEPHONE:
         break;

     case ITEM_TREASURE:
     case ITEM_RELIC:
         break;

     case ITEM_SCROLL:
         obj->value[0]  = number_fuzzy( obj->value[0] );
         break;

     case ITEM_WEAPON:
         break;

     case ITEM_ARMOR:
         obj->value[0] = number_fuzzy(2);
         break;

     case ITEM_POTION:
     case ITEM_PILL:
         obj->value[0] = number_fuzzy( number_fuzzy( obj->value[0] ) );
         break;

     case ITEM_MONEY:
     case ITEM_AMMO:
     case ITEM_BOMB:
         break;
     }

     for (paf = pObjIndex->affected; paf != NULL; paf = paf->next)
         if ( paf->location == APPLY_SPELL_AFFECT )
             affect_to_obj(obj,paf);

    LINK_SINGLE(obj, next, object_list);
     pObjIndex->count++;

     return obj;
}

/* duplicate an object exactly -- except contents */
void clone_object(OBJ_DATA *parent, OBJ_DATA *clone)
{
    int i = 0;
    AFFECT_DATA *paf;
    EXTRA_DESCR_DATA *ed,*ed_new;

    if (parent == NULL || clone == NULL)
        return;

    /* start fixing the object */
    PURGE_DATA( clone->name );
    PURGE_DATA( clone->short_descr );
    PURGE_DATA( clone->description );
    clone->name     = str_dup(parent->name);
    clone->short_descr  = str_dup(parent->short_descr);
    clone->description  = str_dup(parent->description);
    clone->item_type    = parent->item_type;
    clone->extra_flags  = parent->extra_flags;
    clone->extra2   = parent->extra2;
    clone->wear_flags   = parent->wear_flags;
    clone->weight   = parent->weight;
    clone->cost     = parent->cost;
    clone->condition    = parent->condition;
    PURGE_DATA( clone->material );
    clone->material = str_dup(parent->material);
    clone->timer    = parent->timer;

    for (i = 0;  i < 5; i ++)
        clone->value[i] = parent->value[i];

    /* affects */
    clone->enchanted    = parent->enchanted;

    for (paf = parent->affected; paf != NULL; paf = paf->next)
        affect_to_obj(clone,paf);

    /* extended desc */
    for (ed = parent->extra_descr; ed != NULL; ed = ed->next)
    {
        ed_new                  = new_extra_descr();
        PURGE_DATA( ed_new->keyword );
        PURGE_DATA( ed_new->description );
        ed_new->keyword     = str_dup( ed->keyword);
        ed_new->description     = str_dup( ed->description );
        ed_new->next            = clone->extra_descr;
        clone->extra_descr      = ed_new;
    }

}


/*
 * Clear a new character.
 */
void clear_char( CHAR_DATA *ch )
{
    int i = 0;

    ch->name            = NULL;
    ch->short_descr     = NULL;
    ch->long_descr      = NULL;
    ch->description     = NULL;
    ch->prompt                  = NULL;
    ch->logon           = current_time;
    ch->lines           = PAGELEN;
    for (i = 0; i < 4; i++)
        ch->armor[i]        = 100;
    ch->position        = P_STAND;
    ch->health          = 7;
    ch->on          = NULL;
    ch->warrants        = 0;
    ch->hunter_vis      = 0;
    for (i = 0; i < MAX_STATS; i ++)
    {
        ch->perm_stat[i] = 13;
        ch->mod_stat[i] = 0;
    }
    return;
}

/*
 * Get an extra description from a list.
 */
char *get_extra_descr( const char *name, EXTRA_DESCR_DATA *ed )
{
    for ( ; ed != NULL; ed = ed->next )
    {
        if ( is_name( (char *) name, ed->keyword ) )
            return ed->description;
    }
    return NULL;
}



/*
 * Translates mob virtual number to its mob index struct.
 * Hash table lookup.
 */
MOB_INDEX_DATA *get_mob_index( int vnum )
{
    MOB_INDEX_DATA *pMobIndex;

    for ( pMobIndex  = mob_index_hash[vnum % MAX_KEY_HASH];
            pMobIndex != NULL;
            pMobIndex  = pMobIndex->next )
    {
        if ( pMobIndex->vnum == vnum )
            return pMobIndex;
    }

    if ( fBootDb )
    {
        bug( "Get_mob_index: bad vnum %d.", vnum );
        exit( 1 );
    }

    return NULL;
}


/*
 * Translates mob virtual number to its obj index struct.
 * Hash table lookup.
 */
OBJ_INDEX_DATA *get_obj_index( int vnum )
{
    OBJ_INDEX_DATA *pObjIndex;

    for ( pObjIndex  = obj_index_hash[vnum % MAX_KEY_HASH];
            pObjIndex != NULL;
            pObjIndex  = pObjIndex->next )
    {
        if ( pObjIndex->vnum == vnum )
            return pObjIndex;
    }

    if ( fBootDb )
    {
        bug( "Get_obj_index: bad vnum %d.", vnum );
        exit( 1 );
    }

    return NULL;
}



/*
 * Translates mob virtual number to its room index struct.
 * Hash table lookup.
 */
ROOM_INDEX_DATA *get_room_index( int vnum )
{
    ROOM_INDEX_DATA *pRoomIndex;

    // this simply won't exist!
    if(vnum == -1)
        return NULL;

    for ( pRoomIndex  = room_index_hash[vnum % MAX_KEY_HASH];
            pRoomIndex != NULL;
            pRoomIndex  = pRoomIndex->next )
    {
        assert(pRoomIndex);

        if ( pRoomIndex->vnum == vnum )
            return pRoomIndex;
    }

    if ( fBootDb )
    {
        bug( "Get_room_index: bad vnum %d.", vnum );
        exit( 1 );
    }

    return NULL;
}



/*
 * Read a letter from a file.
 */
char fread_letter( FILE *fp )
{
    char c;

    do
    {
        c = getc( fp );
    }
    while ( isspace(c) );

    return c;
}



/*
 * Read a number from a file.
 */
int fread_number( FILE *fp )
{
    int number = 0;
    bool sign = FALSE;
    char c;

    do
    {
    c = getc( fp );
    }
    while ( isspace(c) );

    number = 0;

    if ( c == '+' )
    {
    c = getc( fp );
    }
    else if ( c == '-' )
    {
    sign = TRUE;
    c = getc( fp );
    }

    if ( !isdigit(c) )
    {
    bug( "Fread_number: bad format.", 0 );
    exit( 1 );
    }

    while ( isdigit(c) )
    {
    number = number * 10 + c - '0';
    c      = getc( fp );
    }

    if ( sign )
    number = 0 - number;

    if ( c == '|' )
    number += fread_number( fp );
    else if ( c != ' ' )
    ungetc( c, fp );

    return number;
}

long fread_flag( FILE *fp)
{
    int number = 0;
    char c;
    bool negative = FALSE;

    do
    {
    c = getc(fp);
    }
    while ( isspace(c));

    if (c == '-')
    {
    negative = TRUE;
    c = getc(fp);
    }

    number = 0;

    if (!isdigit(c))
    {
    while (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z'))
    {
        number += flag_convert(c);
        c = getc(fp);
    }
    }

    while (isdigit(c))
    {
    number = number * 10 + c - '0';
    c = getc(fp);
    }

    if (c == '|')
    number += fread_flag(fp);

    else if  ( c != ' ')
    ungetc(c,fp);

    if (negative)
    return -1 * number;

    return number;
}

long flag_convert(char letter )
{
    long bitsum = 0;
    char i;

    if ('A' <= letter && letter <= 'Z') 
    {
    bitsum = 1;
    for (i = letter; i > 'A'; i--)
        bitsum *= 2;
    }
    else if ('a' <= letter && letter <= 'z')
    {
    bitsum = 67108864; /* 2^26 */
    for (i = letter; i > 'a'; i --)
        bitsum *= 2;
    }

    return bitsum;
}


/*
 * Read and allocate space for a string from a file.
 * These strings are read-only and shared.
 * Strings are hashed:
 *   each string prepended with hash pointer to prev string,
 *   hash code is simply the string length.
 *   this function takes 40% to 50% of boot-up time.
 */
char *fread_string( FILE *fp )
{

       static char buf[MSL *5]={'\0'};
    
       /*
        * extra 2 bytes on the end for \0
        * and 1b slack 
        */
       size_t i = 0;
       register char c;
       bool sFull = FALSE;
       FILE *gfp = fp;

       //MakeZero(buf);

       /*
        * skip blanks 
        */
       do
       {
          c = getc( gfp );
       }
       while( isspace( c ) );
    
       /*
        * empty string 
        */
       if( c == '~' )
          return NULL;
    
       buf[i++] = c;
    
       for( ;; )
       {
          if( i >= MSL *5 && !sFull )
          {
             //sys.bugf("String [%20.20s...] exceeded [%d] MSL *5", buf, MSL *5);
             sFull = TRUE;
             abort();
          }
          switch ( c = getc( gfp ) )
          {
             default:
                if( !sFull )
                {
                   buf[i++] = c;
                }
                break;
    
             case '\0':
                abort(  );
                break;
    
             case '\n':
                if( !sFull )
                {
                   buf[i++] = '\n';
                   buf[i++] = '\r';
                }
                break;
    
             case '\r':
                break;
    
             case '~':
                buf[i] = '\0';
    
            if(buf[0] == '\0')
            return NULL;
    
                if( !str_cmp( buf, "(null)" ) )
                   return NULL;
    
                return str_dup(buf);
          }
       }

}

char *fread_string_eol( FILE *fp )
{
       char buf[MSL * 5]={'\0'};
       char *ptr = buf;
       FILE *gfp = fp;
       int c = 0;
       do
       {
          c = getc( gfp );
          *ptr = c;
       }
       while( isspace( c ) );
       if( ( *ptr++ = c ) == '\n' )
          return NULL;
       for( ;; )
          switch ( *ptr = getc( gfp ) )
          {
             default:
                ptr++;
                break;
             case EOF:
                //sys.bugf("read_string: EOF");
                abort();
                break;
             case '\n':
             case '\r':
                *ptr = '\0';
                if( fBootDb )
                {
                   if( !str_cmp( buf, "(null)" ) )
                      return NULL;
    
                   ptr = (char *)str_dup( buf );
                   return ptr;
                }
    
                if( !str_cmp( buf, "(null)" ) )
                   return NULL;
    
                return str_dup( buf );
          }
}



/*
 * Read to end of line (for comments).
 */
void fread_to_eol( FILE *fp )
{
    char c;

    do
    {
        c = getc( fp );
    }
    while ( c != '\n' && c != '\r' );

    do
    {
        c = getc( fp );
    }
    while ( c == '\n' || c == '\r' );

    ungetc( c, fp );
    return;
}


/*
 * Read one word (into static buffer).
 */
char *_fread_word( FILE *fp, const char *file, const char *function, int line )
{
    static char word[MAX_INPUT_LENGTH]={'\0'};
    char *pword;
    char cEnd;

    do
    {
        cEnd = getc( fp );
    }
    while ( isspace( cEnd ) );

    if ( cEnd == '\'' || cEnd == '"' )
    {
        pword   = word;
    }
    else
    {
        word[0] = cEnd;
        pword   = word+1;
        cEnd    = ' ';
    }

    for ( ; pword < word + MAX_INPUT_LENGTH; pword++ )
    {
        *pword = getc( fp );
        if ( cEnd == ' ' ? isspace(*pword) : *pword == cEnd )
        {
            if ( cEnd == ' ' )
                ungetc( *pword, fp );
            *pword = '\0';
            return word;
        }
    }


    logfmt("fread_word: word too long: %s:%s:%d",file, function, line); 
    bug( "Fread_word: word too long.", 0 );
    exit( 1 );
    return NULL;
}

char *fread_chunk( FILE *fp, char delimiter )
{
    static char word[10*MAX_INPUT_LENGTH]={'\0'};
    char *pword;
    char cEnd;

    do
    {
        cEnd = getc( fp );
    }
    while ( cEnd == delimiter );

    if ( cEnd == '\'' || cEnd == '"' )
    {
        pword   = word;
    }
    else
    {
        word[0] = cEnd;
        pword   = word+1;
        cEnd    = delimiter;
    }

    for ( ; pword < word + 10*MAX_INPUT_LENGTH; pword++ )
    {
        *pword = getc( fp );
        if ( cEnd == delimiter ? *pword == delimiter : *pword == cEnd )
        {
            if ( cEnd == delimiter )
                ungetc( *pword, fp );
            *pword = '\0';
            return word;
        }
    }

    bug( "Fread_word: word too long.", 0 );
    exit( 1 );
    return NULL;
}

void do_areas( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea1;
    AREA_DATA *pArea2;
    int iArea = 0;
    int iAreaHalf = 0;

    if (!IS_NULLSTR(argument))
    {
        send_to_char("No argument is used with this command.\n\r",ch);
        return;
    }

    iAreaHalf = (top_area + 1) / 2;
    pArea1    = area_first;
    pArea2    = area_first;
    for ( iArea = 0; iArea < iAreaHalf; iArea++ )
        pArea2 = pArea2->next;

    for ( iArea = 0; iArea < iAreaHalf; iArea++ )
    {
        send_to_char( Format("%-15s%-24s%-15s%-24s\n\r", pArea1->credits, pArea1->name, (pArea2 != NULL) ? pArea2->credits : "", (pArea2 != NULL) ? pArea2->name : ""), ch );
        pArea1 = pArea1->next;
        if ( pArea2 != NULL )
            pArea2 = pArea2->next;
    }

    return;
}


void do_memory( CHAR_DATA *ch, char *argument )
{
    send_to_char( "\r\n", ch );
    send_to_char( Format("Affects %5d\n\r", top_affect), ch );
    send_to_char( Format("Areas   %5d\n\r", top_area), ch );
    send_to_char( Format("ExDes   %5d\n\r", top_ed), ch );
    send_to_char( Format("Exits   %5d\n\r", top_exit), ch );
    send_to_char( Format("Helps   %5d\n\r", top_help), ch );
    send_to_char( Format("Tips    %5d\n\r", top_tip), ch );
    send_to_char( Format("Socials %5d\n\r", maxSocial), ch );
    send_to_char( Format("Mobs    %d\n\r", newmobs), ch );
    send_to_char( Format("(in use)%5d\n\r", mobile_count), ch );
    send_to_char( Format("Objs    %d\n\r", newobjs), ch );
    send_to_char( Format("Resets  %5d\n\r", top_reset), ch );
    send_to_char( Format("Rooms   %5d\n\r", top_room), ch );
    send_to_char( Format("Shops   %5d\n\r", top_shop), ch );

    return;
}

/*
 * Stick a little fuzz on a number.
 */
int number_fuzzy( int number )
{
    switch ( number_bits( 2 ) )
    {
    case 0:  number -= 1; break;
    case 3:  number += 1; break;
    }

    return UMAX( 1, number );
}



/*
 * Generate a random number.
 */
int number_range( int from, int to )
{
    int power = 0;
    int number = 0;

    if (from == 0 && to == 0)
        return 0;

    if ( ( to = to - from + 1 ) <= 1 )
        return from;

    for ( power = 2; power < to; power <<= 1 )
        ;

    while ( ( number = number_mm() & (power -1 ) ) >= to )
        ;

    return from + number;
}



/*
 * Generate a percentile roll.
 */
int number_percent( void )
{
    int percent = 0;

    while ( (percent = number_mm() & (128-1) ) > 99 )
    ;

    return 1 + percent;
}



/*
 * Generate a random door.
 */
int number_door( void )
{
    int door = 0;

    while ( ( door = number_mm() & (8-1) ) > 5)
    ;

    return door;
}

int number_bits( int width )
{
    return number_mm( ) & ( ( 1 << width ) - 1 );
}

void init_mm( )
{
    srandom(time(NULL)^getpid());
    return;
}
 
 
 
long number_mm( void )
{
#if defined (OLD_RAND)
    int *piState = 0;
    int iState1 = 0;
    int iState2 = 0;
    int iRand = 0;

    piState             = &rgiState[2];
    iState1             = piState[-2];
    iState2             = piState[-1];
    iRand               = (piState[iState1] + piState[iState2])
                                & ((1 << 30) - 1);
    piState[iState1]    = iRand;
    if ( ++iState1 == 55 )
        iState1 = 0;
    if ( ++iState2 == 55 )
        iState2 = 0;
    piState[-2]         = iState1;
    piState[-1]         = iState2;
    return iRand >> 6;
#else
    return random() >> 6;
#endif
}


/*
 * Roll some dice.
 */
int dice( int number, int size )
{
    int idice = 0;
    int sum = 0;

    switch ( size )
    {
    case 0: return 0;
    case 1: return number;
    }

    for ( idice = 0, sum = 0; idice < number; idice++ )
        sum += number_range( 1, size );

    return sum;
}



/*
 * Simple linear interpolation.
 */
int interpolate( int level, int value_00, int value_32 )
{
    return value_00 + level * (value_32 - value_00) / 32;
}



/*
 * Removes the tildes from a string.
 * Used for player-entered strings that go into disk files.
 */
void smash_tilde( char *str )
{
    for ( ; *str != '\0'; str++ )
    {
        if ( *str == '~' )
            *str = '-';
    }

    return;
}



/*
 * Compare strings, case insensitive.
 * Return TRUE if different
 *   (compatibility with historical functions).
 */
bool __strcmp(const char *astr, const char *bstr, const char *_file, const char *_function, int _line)
{
    if ( astr == NULL )
    {
        bug( Format("str_cmp: null astr from %s, %s %d",_file,_function, _line), 0 );
        return TRUE;
    }
    if ( bstr == NULL )
    {
        bug( Format ("Str_cmp: null bstr from %s, %s %d",_file,_function, _line), 0 );
        return TRUE;
    }
    for ( ; *astr || *bstr; astr++, bstr++ )
    {
        if ( LOWER(*astr) != LOWER(*bstr) )
            return TRUE;
    }
    return FALSE;
}



/*
 * Compare strings, case insensitive, for prefix matching.
 * Return TRUE if astr not a prefix of bstr
 *   (compatibility with historical functions).
 */
bool str_prefix( const char *astr, const char *bstr )
{
    if ( astr == NULL )
    {
        bug( "Strn_cmp: null astr.", 0 );
        return TRUE;
    }

    if ( bstr == NULL )
    {
        bug( "Strn_cmp: null bstr.", 0 );
        return TRUE;
    }

    for ( ; *astr; astr++, bstr++ )
    {
        if ( LOWER(*astr) != LOWER(*bstr) )
            return TRUE;
    }

    return FALSE;
}

/*
 * Needed a method of testing if two lists of numbers were the same
 */
bool num_array_comp( const int *astr, const int *bstr )
{
    if ( astr == NULL )
    {
        bug( "Strn_cmp: null astr.", 0 );
        return TRUE;
    }

    if ( bstr == NULL )
    {
        bug( "Strn_cmp: null bstr.", 0 );
        return TRUE;
    }

    for ( ; *astr; astr++, bstr++ )
    {
        if ( *astr != *bstr )
            return TRUE;
    }

    return FALSE;
}

/*
 * Compare strings, case insensitive, for match anywhere.
 * Returns TRUE is astr not part of bstr.
 *   (compatibility with historical functions).
 */
bool str_infix( const char *astr, const char *bstr )
{
    int sstr1 = 0;
    int sstr2 = 0;
    int ichar = 0;
    char c0;

    if ( ( c0 = LOWER(astr[0]) ) == '\0' )
        return FALSE;

    sstr1 = strlen(astr);
    sstr2 = strlen(bstr);

    for ( ichar = 0; ichar <= sstr2 - sstr1; ichar++ )
    {
        if ( c0 == LOWER(bstr[ichar]) && !str_prefix( astr, bstr + ichar ) )
            return FALSE;
    }

    return TRUE;
}



/*
 * Compare strings, case insensitive, for suffix matching.
 * Return TRUE if astr not a suffix of bstr
 *   (compatibility with historical functions).
 */
bool str_suffix( const char *astr, const char *bstr )
{
    int sstr1 = 0;
    int sstr2 = 0;

    sstr1 = strlen(astr);
    sstr2 = strlen(bstr);
    if ( sstr1 <= sstr2 && !str_cmp( astr, bstr + sstr2 - sstr1 ) )
    return FALSE;
    else
    return TRUE;
}

/*
 * Randomised name creation.
 */
void name_replacer(char *format, char *first_name, char *last_name, char *out)
{
    // Changed some of the naming in order to make things clearer.
    // Added toFree to help shutdown a memory leak.
    char buf[MSL]={'\0'};
    char *str;
    char *i;
    char *point;
    char *toFree;

    if(format == NULL)
        return;

    point = buf;
    str = str_dup(format);
    toFree = str;

    // we should never reach this if format is null.
    if(str == NULL)
        return;

    /* replace $f with first_name and $l with last_name. */
    while ( *str != '\0' )
    {
        if ( *str != '$' )
        {
            *point++ = *str++;
            continue;
        }
        ++str;

        switch ( *str )
        {
            default:  bug( "Rand_name: bad code %d.", *str );
            i = " Dave ";                                break;
            case 'f': i = first_name;
            break;
            case 'l': i = last_name;
            break;
            case ' ': i = "$";
            break;
            case '$': i = "$";
            break;
        }

        ++str;
        while ( ( *point = *i ) != '\0' )
            ++point, ++i;
    }
    *point++ = '\0';

    strcpy(out, buf);

    free(toFree);

    return;
}

#define MAX_NAMES 103

char* male_names [MAX_NAMES] =
{
        "Michael", "Matthew", "Nicholas", "Christopher", "Joshua", "Austin", "Tyler", "Brandon",
        "Joseph", "Andrew", "Daniel", "Ryan", "Zachary", "Anthony", "Dylan", "David", "Jonathan",
        "William", "James", "John", "Alexander", "Justin", "Jordan", "Noah", "Robert", "Brian",
        "Jose", "Steven", "Kyle", "Kevin", "Christian", "Samuel", "Eric", "Benjamin", "Thomas",
        "Hunter", "Ethan", "Sean", "Devin", "Cameron", "Nathan", "Aaron", "Caleb", "Connor", "Cody",
        "Jason", "Luis", "Timothy", "Isaac", "Adam", "Jared", "Charles", "Richard", "Gabriel",
        "Mark", "Jack", "Isaiah", "Juan", "Logan", "Alex", "Antonio", "Carlos", "Elijah", "Jesse",
        "Brendan", "Patrick", "Luke", "Angel", "Dakota", "Lucas", "Chase", "Cole", "Seth",
        "Nathaniel", "Jeremy", "Alejandro", "Ian", "Tristan", "Jeffrey", "Jake", "Garrett",
        "Victor", "Jackson", "Marcus", "Trevor", "Bradley", "Brett", "Bryce", "Adrian", "Evan",
        "Gavin", "Paul", "Tanner", "Colton", "Dalton", "Kenneth", "Spencer", "Xavier", "Peter",
        "Dave", "Mick", "Matt", "Bernard"
};

char* female_names [MAX_NAMES] =
{
        "Emily", "Sarah", "Brianna", "Samantha", "Hailey", "Ashley", "Kaitlyn", "Madison", "Hannah",
        "Alexis", "Jessica", "Alyssa", "Abigail", "Kayla", "Megan", "Katherine", "Taylor",
        "Elizabeth", "Makayla", "Rachel", "Jasmine", "Olivia", "Victoria", "Emma", "Anna",
        "Rebecca", "Natalie", "Courtney", "Lauren", "Sydney", "Amanda", "Nicole", "Destiny", "Stephanie", 
        "Morgan", "Julia", "Sierra", "Brittany", "Grace", "Danielle", "Jennifer",
        "Kaylee", "Mackenzie", "Alexandra", "Savannah", "Amber", "Jordan", "Kylie", "Shelby",
        "Christina", "Jacqueline", "Gabriella", "Maria", "Erica", "Gabrielle", "Madeline", "Erin",
        "Mary", "Marissa", "Andrea", "Vanessa", "Kimberly", "Michelle", "Riley", "Brooke", "Bailey",
        "Kiara", "Cassandra", "Chloe", "Sophia", "Alexandria", "Cassidy", "Kelly", "Melissa",
        "Kelsey", "Laura", "Miranda", "Jade", "Paige", "Sabrina", "Casey", "Molly", "Katie",
        "Alicia", "Isabel", "Leslie", "Lily", "Alexa", "Caroline", "Mariah", "Jenna", "Angela",
        "Veronica", "Chelsea", "Autumn", "Diana", "Faith", "Shannon", "Bethany", "Bianca",
        "Stacey", "Magrat", "Sarah"
};

char* last_names [MAX_NAMES] =
{
        "Smith", "Johnson", "Williams", "Brown", "Davis", "Miller", "Wilson", "Moore", "Taylor",
        "Morrison", "Black", "White", "Grossman", "Thall", "Finklestein", "Bevill", "McGuinn",
        "Olson", "Wiater", "Convey", "Sabaitis", "Brothers", "Kotiw", "Carraway", "Jennings",
        "Major", "Watts", "Bendig", "Abbott", "Abene", "Zurek", "Zarov", "Yurchak", "Vogt", "Ulum",
        "Tzanetopoulos", "Turner", "Swanson", "Strong", "Stoffer", "Rozen", "Roy", "Rosenthal",
        "RoonieQuinn", "Qazi", "Putman", "Pulman", "Pruitt", "Overstreet", "Owen", "Nutile",
        "Norman", "Noelle", "Musso", "Mulvihill", "Morgan", "Mitchels", "Lynn", "Luber", "Liva",
        "Krueger", "Kozlowski", "Koffmann", "June", "Jones", "Jensen", "Isztok", "Hutchins",
        "Hughes", "Howard", "Griffin", "Green", "Greenan", "Gould", "Giles", "Froemel", "Flynn",
        "Fishman", "Esposito", "Emerson", "Edgeson", "Drozd", "Downs", "Dohra", "Curley", "Crosby",
        "Coote", "Collins", "Burger", "Bruns", "Blunt", "Blackmond", "Barnett", "Fitzgerald",
        "Wood", "Smyth", "Stryker", "Sweet", "Sutherland", "Cohen",
        "Dredd", "O'Reily", "Kilborn"
};

char *name_select(char *table[MAX_NAMES])
{
    int i = 0;
    char *a;

    i = number_range(0, MAX_NAMES - 1);

    a = table[i];
    return a;
}

void rand_name(CHAR_DATA *ch)
{
    char *first_name;
    char *last_name;
    char out[4*MSL]={'\0'};

    if(ch->sex == SEX_FEMALE)
    {
        first_name = name_select(female_names);
        last_name = name_select(last_names);
    }
    else
    {
        first_name = name_select(male_names);
        last_name = name_select(last_names);
    }

    PURGE_DATA(ch->name);
    name_replacer(ch->pIndexData->player_name, first_name, last_name, out);
    ch->name = str_dup(out);
    PURGE_DATA(ch->short_descr);
    name_replacer(ch->pIndexData->short_descr, first_name, last_name, out);
    ch->short_descr = str_dup(out);
    PURGE_DATA(ch->long_descr);
    name_replacer(ch->pIndexData->long_descr, first_name, last_name, out);
    ch->long_descr = str_dup(out);
    PURGE_DATA(ch->description);
    name_replacer(ch->pIndexData->description, first_name, last_name, out);
    ch->description = str_dup(out);
}

/*
 * Set all elevators, trains and other assorted vehicles in starting positions.
 */
void prep_vehicles (void)
{
    ROOM_INDEX_DATA *pr;
    ROOM_INDEX_DATA *car;
    int i = 0;

    for(i = 0; i < MAX_KEY_HASH; i++)
    {
        for(pr = room_index_hash[i]; pr != NULL; pr = pr->next)
        {
            if(IS_SET(pr->room_flags, ROOM_STOP) && pr->car < 0)
            {
                log_string(Format("Room %d set as stop with no car attached.", pr->vnum));
                bug(Format("Room %d set as stop with no car attached.", pr->vnum), 0);
                continue;
            }

            if(pr->car > 0)
            {
                if((car = get_room_index(pr->car)) == NULL)
                {
                    log_string(Format("Room %d has invalid car %d attached.", pr->vnum, pr->car));
                    bug(Format("Room %d has invalid car %d attached.", pr->vnum, pr->car), 0);
                    exit(1);
                }

                car->stops[pr->stops[0]] = pr->vnum;
            }
        }
    }
}

/*
 * Returns an initial-capped string.
 */
char *capitalize( const char *str )
{
    static char strcap[MSL]={'\0'};
    int i = 0;

    for ( i = 0; str[i] != '\0'; i++ )
    strcap[i] = LOWER(str[i]);
    strcap[i] = '\0';
    strcap[0] = UPPER(strcap[0]);
    return strcap;
}

/*
 * Returns an all-capped string.
 */
char *allcaps( const char *str )
{
    static char strcap[MSL]={'\0'};
    int i = 0;

    for ( i = 0; str[i] != '\0'; i++ )
    strcap[i] = UPPER(str[i]);
    strcap[i] = '\0';
    return strcap;
}


/*
 * Append a string to a file.
 */
void append_file( CHAR_DATA *ch, char *file, char *str )
{
    FILE *fp;

    if ( ch != NULL && (IS_NPC(ch) || str[0] == '\0') )
        return;

    closeReserve();
    if ( ( fp = fopen( file, "a" ) ) == NULL )
    {
        perror( file );
        send_to_char( "Could not open the file!\n\r", ch );
    }
    else
    {
        if(ch != NULL)
            fprintf( fp, "[%5d] %s: %s - %s",
                    ch->in_room ? ch->in_room->vnum : 0,
                            ch->name, str, (char *) ctime( &current_time ) );
        else
            fprintf( fp, "%s\n", str );
        fclose( fp );
    }

    openReserve();
    return;
}



/*
 * Reports a bug.
 */
void bug( const char *str, int param )
{
    char buf[MSL]={'\0'};

    if ( fpArea != NULL )
    {
        int iLine;
        int iChar;

        if ( fpArea == stdin )
        {
            iLine = 0;
        }
        else
        {
            iChar = ftell( fpArea );
            fseek( fpArea, 0, 0 );
            for ( iLine = 0; ftell( fpArea ) < iChar; iLine++ )
            {
                while ( getc( fpArea ) != '\n' )
                    ;
            }
            fseek( fpArea, iChar, 0 );
        }

        log_string( Format("[*****] FILE: %s LINE: %d", strArea, iLine) );
    }

    strncpy( buf, "[*****] BUG: ", sizeof(buf) );
    snprintf( buf + strlen(buf), sizeof(buf), str, param );
    log_string( buf );

    return;
}


char *sys_time()
{
    char *strtime;

    strtime = ctime( &current_time );
    strtime[strlen(strtime)-1] = '\0';
    return strtime;
}


void log_to_file(char *file, char *extension, const char *string)
{
    int fd = 0, length = 0;
    char stack[MSL]={'\0'};

    if (DEBUG) fprintf( stderr, "%s\n\r", string );

    /* these should be defined by your system... they're the bits that will
      be set for the file's file permissions. If you're using DOS or Win95
      or some god-awful OS like that, replace the snprintf() above with:
      snprintf(stack, sizeof(stack), "logs\\%s.log", file); and the call to open() below
      to: fd = open(stack, O_CREAT | O_WRONLY); Or, you can write some
      preprocessor directives to choose which to use, if you're interested
      in portability. I for one don't need a Win95 system to test on,
      since I'm running NetBSD/mac68k on my Mac IIci ;-) -- end OS rant */

    if((fd = open((char *)Format("%s%s.%s", LOG_DIR, file, extension), O_CREAT | O_WRONLY | O_SYNC, S_IRUSR | S_IWUSR)) == -1)
        perror( "Log failure" );
    length = lseek(fd, 0, SEEK_END);

    /* this bit will turn over your log file(s) if they go over MAX_LOG
      bytes. */

    if (length > MAX_LOG)
    {
        int i = atoi(file) + 1;

        close(fd);
        fd=open((char *)Format("%s%d.log", LOG_DIR, i), O_CREAT | O_WRONLY | O_SYNC | O_TRUNC, S_IRUSR | S_IWUSR);
    }


    /* sys_time is a function that returns the real time, formatted like:
       05:24:52 - 12/22/97 in my program's case */

    snprintf(stack, sizeof(stack), "%s - %s\n", sys_time(), string);

    /* you can set this to be TRUE (a global for me == 1) to turn off
      logging */

    if (NO_LOGS == TRUE)
        printf(stack);
    write(fd, stack, strlen(stack));
    close(fd);

    fprintf(stdout, "%s", stack);

}


char logfile[MAX_INPUT_LENGTH];


/*
 * Writes a string to the log. Thanks Darien
 */
void log_string( const char *str )
{
    FILE *fp = fopen("../log/mylog.log", "a");
    if(fp)
    {
        fprintf(fp, "%s\n", str);
    }

    fprintf(stdout, "%s\n", str);
    fclose(fp);
    fp = NULL; return;
}


MPROG_CODE *get_mprog_index( int vnum )
{
    MPROG_CODE *prg;
    for( prg = mprog_list; prg; prg = prg->next )
    {
        if ( prg->vnum == vnum )
            return prg;
    }

    return NULL;
}


/*
 * This function is here to aid in debugging.
 * If the last expression in a function is another function call,
 *   gcc likes to generate a JMP instead of a CALL.
 * This is called "tail chaining."
 * It hoses the debugger call stack for that call.
 * So I make this the last call in certain critical functions,
 *   where I really need the call stack to be right for debugging!
 *
 * If you don't understand this, then LEAVE IT ALONE.
 * Don't remove any calls to tail_chain anywhere.
 *
 * -- Furey
 */
void tail_chain( void )
{
    return;
}

// Improved strdup capabilitys for logging / debugging by David Simmerson
char *_str_dup(const char *str, const char *file, const char *function, int line)
{
    if(IS_NULLSTR(str))
    {
        log_string(Format("_str_dup: %s:%s:%d called with NULL values!", file, function, line ) );
        return NULL;
    }
    return strdup(str);  // relay to our actual strdup!
}

// Format by David Simmerson
const char *Format(const char *fmt, ...)
{

    static char textString[MSL*5] = {'\0'};

    // -- empty the buffer properly to ensure no leaks.
    memset(textString, '\0', sizeof(textString));

    va_list args;
    va_start ( args, fmt );
    int length = vsnprintf ( textString, MSL*5, fmt, args );
    va_end ( args );

    if(length == 0) {
        log_string("Format had a boo-boo! 0 length string returned!  Suspect will cause corruption and/or crash!");
    }

    return textString;
}

// CapitalSentence by David Simmerson
char *CapitalSentence(const char *str)
{
    static char _buf[MSL*5]={'\0'};
    bool didPeriod= false;
    memset(_buf, 0, sizeof(_buf));
    int _x = 0;
    for(_x = 0; str[_x] != '\0'; _x++)
    {
        if(_x == 0) // first letter of the string is capitalized.
            _buf[0] = UPPER(str[0]);
        else
        {
            if(str[_x] == '.')
            {
                _buf[_x] = str[_x];
                didPeriod = true;
                continue;
            }
            // are we a letter that needs to be capitalized.
            if(didPeriod && isalpha(str[_x]))
            {
                _buf[_x] = UPPER(str[_x]);
                didPeriod = false;
                continue;
            }
            // just a letter/number/etc, push it to the new buffer.
            _buf[_x] = str[_x];
        }
    }
    return (_buf);
}

#ifdef ASSERT
void AssertLog(const std::string &str) {
        closeReserve();
FILE *fp = fopen("Assert.log", "a");
if(fp) {
fprintf(fp, "%s\n", str );
} else {
                fprintf(stdout, "%s\n", str);
}
fclose(fp); // just in-case.
        openReserve();
}
void AssertFailed ( const char *expression, const char *msg, const char *file, const char *baseFile, const char *function, int line )
{
if ( !str_cmp ( file, baseFile ) ) {
AssertLog(Format("Assert(%s)(%s) failed in file(%s), function(%s), line(%d)", expression, msg, file, function.c_str(), line ) );
} else {
AssertLog(Format("Assert(%s)(%s) failed in file(%s), included from file(%s), function(%s), line(%d)", expression, msg, file, baseFile, function, line ) );
}
// what this does: if ASSERT_CRASH is defined, we let the assert log, THEN continue! as if nothing bad is about to happen!
// then we crash at the actual instance so we can get ourselves a nice core-file!
// however, if ASSERT_CRASH is not defined, we will ABORT and simply log.
// Note: some systems will leave a crash-log when aborting (dump core file)
#if !defined (ASSERT_CRASH)
// if REAL_ASSERT is defined we skip the abort because it will assert the expression (double logging, we know)
// but it gives us a great opportunity to check out differences in results.  As well as create a new standard
// to hold the code to.
#if !defined(REAL_ASSERT)
abort();
#endif
#endif
}
#endif

void openReserve(void)
{
    if(fpReserve) return;
    if ((fpReserve = fopen (NULL_FILE, "r")) == NULL)
    {
        perror (Format("%s : %s", __FUNCTION__, NULL_FILE ) );
        exit (1);
    }
}
void closeReserve(void)
{
    if(fpReserve)
    {
          fclose(fpReserve);
    }
    fpReserve = NULL;
}

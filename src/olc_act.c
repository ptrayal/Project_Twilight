/***************************************************************************
 *  File: olc_act.c                                                        *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/



#if defined(Macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include "twilight.h"
#include "tables.h"
#include "olc.h"
#include "recycle.h"
#include "lookup.h"

char * mprog_type_to_name ( int type );
int	make_event_vnum		args( () );
int	make_script_vnum	args( () );

/* Return TRUE if area changed, FALSE if not. */
#define REDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )
#define OEDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )
#define MEDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )
#define AEDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )
#define HEDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )
#define PEDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )
#define EEDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )
#define SEDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )
#define AIEDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )
#define RSEDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )
#define NEDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )

PLOT_DATA *get_plot_index	args( ( int vnum ) );
EVENT_DATA *get_event_index	args( ( int vnum ) );
SCRIPT_DATA *get_script_index	args( ( int vnum ) );
PERSONA *get_persona_index	args( ( int vnum ) );

struct olc_help_type
{
    char *command;
    const void *structure;
    char *desc;
};



bool show_version( CHAR_DATA *ch, char *argument )
{
    send_to_char( VERSION, ch );
    send_to_char( "\n\r", ch );
    send_to_char( AUTHOR, ch );
    send_to_char( "\n\r", ch );
    send_to_char( DATE, ch );
    send_to_char( "\n\r", ch );
    send_to_char( CREDITS, ch );
    send_to_char( "\n\r", ch );

    return FALSE;
}

/*
 * This table contains help commands and a brief description of each.
 * ------------------------------------------------------------------
 */
const struct olc_help_type help_table[] =
{
    {	"area",		area_flags,	 "Area attributes."		 },
    {	"room",		room_flags,	 "Room attributes."		 },
    {	"sector",	sector_flags,	 "Sector types, terrain."	 },
    {	"exit",		exit_flags,	 "Exit types."			 },
    {	"type",		type_flags,	 "Types of objects."		 },
    {	"extra",	extra_flags,	 "Object attributes."		 },
    {	"extra2",	extra2_flags,	 "More object attributes."	 },
    {	"wear",		wear_flags,	 "Where to wear object."	 },
    {	"sex",		sex_flags,	 "Sexes."			 },
    {	"act",		act_flags,	 "Mobile attributes."		 },
    {	"act2",		act2_flags,	 "Mobile attributes."		 },
    {	"affect",	affect_flags,	 "Mobile affects."		 },
    {	"wear-loc",	wear_loc_flags,	 "Where mobile wears object."	 },
    {	"spells",	skill_table,	 "Names of current spells." 	 },
    {	"weapon",	weapon_flags,	 "Type of weapon." 		 },
    {	"container",	container_flags, "Container status."		 },
    {   "damtype",	attack_table,	 "Damage Types"			 },
    {   "apply",	apply_flags,	 "Apply flags"			 },
    {	"form",		form_flags,	 "Mobile body form."	         },
    {	"part",		part_flags,	 "Mobile body parts."		 },
    {	"imm",		imm_flags,	 "Mobile immunity."		 },
    {	"res",		res_flags,	 "Mobile resistance."	         },
    {	"vuln",		vuln_flags,	 "Mobile vlnerability."	         },
    {	"off",		off_flags,	 "Mobile offensive behaviour."	 },
    {	"size",		size_flags,	 "Mobile size."			 },
    {   "position",     position_flags,  "Mobile positions."             },
    {	"material",	material_type,	 "Material mob/obj is made from."},
    {   "wclass",       weapon_class,    "Weapon class."                 },
    {   "wtype",        weapon_type2,    "Special weapon type."          },
    {	"portal",	portal_flags,	 "Portal types."		 },
    {	"furniture",	furniture_flags, "Furniture types."		 },
    {	"praces",	plot_races, 	 "Plot race flags."		 },
    {	"liquid",	liquid_flags, 	 "Liquids."			 },
    {	"spirits",	spirit_table, 	 "Spirits."			 },
    {	"quality",	quality_flags, 	 "Object quality."		 },
    {	"rawmaterials",	raw_material_table,  "Raw material class."	 },
    {   "mprog",        mprog_flags,     "MobProgram flags."             },
    {	NULL,		NULL,		 NULL				 }
};



/*****************************************************************************
 Name:		show_flag_cmds
 Purpose:	Displays settable flags and stats.
 Called by:	show_help(olc_act.c).
 ****************************************************************************/
void show_flag_cmds( CHAR_DATA *ch, const struct flag_type *flag_table )
{
	char buf1 [ MSL ]={'\0'};
	int  flag = 0;
	int  col = 0;

	buf1[0] = '\0';
	col = 0;
	for (flag = 0; flag_table[flag].name != NULL; flag++)
	{
		if ( flag_table[flag].settable )
		{
			strncat( buf1, Format("%-19.18s", flag_table[flag].name), sizeof(buf1) - strlen(buf1) - 1 );
			if ( ++col % 4 == 0 )
				strncat( buf1, "\n\r", sizeof(buf1) - strlen(buf1) - 1 );
		}
	}

	if ( col % 4 != 0 )
		strncat( buf1, "\n\r", sizeof(buf1) - strlen(buf1) - 1 );

	send_to_char( buf1, ch );
	return;
}


/*****************************************************************************
 Name:		show_skill_cmds
 Purpose:	Displays all skill functions.
 		Does remove those damn immortal commands from the list.
 		Could be improved by:
 		(1) Adding a check for a particular class.
 		(2) Adding a check for a level range.
 Called by:	show_help(olc_act.c).
 ****************************************************************************/
void show_skill_cmds( CHAR_DATA *ch, int tar )
{
	char buf1 [ MSL*2 ]={'\0'};
	int  sn = 0;
	int  col = 0;

	buf1[0] = '\0';
	col = 0;
	for (sn = 0; sn < MAX_SKILL; sn++)
	{
		if ( !skill_table[sn].name )
			break;

		if ( !str_cmp( skill_table[sn].name, "reserved" )
				|| skill_table[sn].spell_fun == spell_null )
			continue;

		if ( tar == -1 || skill_table[sn].target == tar )
		{
			strncat( buf1, Format("%-19.18s", skill_table[sn].name), sizeof(buf1) - strlen(buf1) - 1 );
			if ( ++col % 4 == 0 )
				strncat( buf1, "\n\r", sizeof(buf1) - strlen(buf1) - 1 );
		}
	}

	if ( col % 4 != 0 )
		strncat( buf1, "\n\r", sizeof(buf1) - strlen(buf1) - 1 );

	send_to_char( buf1, ch );
	return;
}

/*****************************************************************************
 Name:		show_help
 Purpose:	Displays help for many tables used in OLC.
 Called by:	olc interpreters.
 ****************************************************************************/
bool show_help( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH]={'\0'};
    char spell[MAX_INPUT_LENGTH]={'\0'};
    int cnt = 0;

    argument = one_argument( argument, arg );
    one_argument( argument, spell );

    /*
     * Display syntax.
     */
    if ( IS_NULLSTR(arg) )
    {
	send_to_char( "Syntax:  ? [command]\n\r\n\r", ch );
	send_to_char( "[command]  [description]\n\r", ch );
	for (cnt = 0; help_table[cnt].command != NULL; cnt++)
	{
	    send_to_char( Format("%-10.10s -%s\n\r", capitalize( help_table[cnt].command ), help_table[cnt].desc), ch );
	}
	return FALSE;
    }

    /*
     * Dirty hack to allow ? races to work properly.
     * - Dsarky
     */
    if(!str_prefix(arg, "races"))
    {

	send_to_char( "Available races are:", ch );

	for ( cnt = 0; race_table[cnt].name != NULL; cnt++ )
	{
	    if ( ( cnt % 3 ) == 0 )
		send_to_char( "\n\r", ch );
	    send_to_char( Format(" %-15s", race_table[cnt].name), ch );
	}

	send_to_char( "\n\r", ch );
	return FALSE;
    }

    /*
     * Find the command, show changeable data.
     * ---------------------------------------
     */
    for (cnt = 0; help_table[cnt].command != NULL; cnt++)
    {
        if (  arg[0] == help_table[cnt].command[0]
          && !str_prefix( arg, help_table[cnt].command ) )
	{
	    if ( help_table[cnt].structure == skill_table )
	    {

		if ( IS_NULLSTR(spell) )
		{
		    send_to_char( "Syntax:  ? spells "
		        "[ignore/attack/defend/self/object/all]\n\r", ch );
		    return FALSE;
		}

		if ( !str_prefix( spell, "all" ) )
		    show_skill_cmds( ch, -1 );
		else if ( !str_prefix( spell, "ignore" ) )
		    show_skill_cmds( ch, TAR_IGNORE );
		else if ( !str_prefix( spell, "attack" ) )
		    show_skill_cmds( ch, TAR_CHAR_OFFENSIVE );
		else if ( !str_prefix( spell, "defend" ) )
		    show_skill_cmds( ch, TAR_CHAR_DEFENSIVE );
		else if ( !str_prefix( spell, "self" ) )
		    show_skill_cmds( ch, TAR_CHAR_SELF );
		else if ( !str_prefix( spell, "object" ) )
		    show_skill_cmds( ch, TAR_OBJ_INV );
		else
		    send_to_char( "Syntax:  ? spell "
		        "[ignore/attack/defend/self/object/all]\n\r", ch );

		return FALSE;
	    }
	    else
	    {
		show_flag_cmds( ch, (struct flag_type *)help_table[cnt].structure );
		return FALSE;
	    }
	}
    }

    show_help( ch, "" );
    return FALSE;
}

REDIT( redit_rlist )
{
    ROOM_INDEX_DATA	*pRoomIndex;
    AREA_DATA		*pArea;
    BUFFER		*buf1;
    char		arg  [ MAX_INPUT_LENGTH    ];
    bool found = FALSE;
    sh_int vnum = 0;
    int  col = 0;

    one_argument( argument, arg );

    pArea = ch->in_room->area;
    buf1=new_buf();

    for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
    {
    	if ( ( pRoomIndex = get_room_index( vnum ) ) )
    	{
    		found = TRUE;
    		add_buf( buf1, (char *)Format("[%5d] %-17.16s", vnum, capitalize( pRoomIndex->name )) );
    		if ( ++col % 3 == 0 )
    			add_buf( buf1, "\n\r" );
    	}
    }

    if ( !found )
    {
    	send_to_char( "Room(s) not found in this area.\n\r", ch);
    	return FALSE;
    }

    if ( col % 3 != 0 )
    	add_buf( buf1, "\n\r" );

    page_to_char( buf_string(buf1), ch );
    free_buf(buf1);
    return FALSE;
}

REDIT( redit_flags )
{
    ROOM_INDEX_DATA *pRoom;
    int value = 0;

    if ( !IS_NULLSTR(argument) )
    {
        EDIT_ROOM( ch, pRoom );

        if ( ( value = flag_value( room_flags, argument ) ) != NO_FLAG )
        {
            pRoom->room_flags ^= value;
            send_to_char( "Room flag toggled.\n\r", ch);
            return TRUE;
        }
    }

    send_to_char( "Syntax: flags [flag]\n\r"
                  "Type '? room' for a list of flags.\n\r", ch );
    return FALSE;
}

REDIT( redit_mlist )
{
	MOB_INDEX_DATA	*pMobIndex;
	AREA_DATA		*pArea;
	BUFFER		*buf1;
	char		arg  [ MAX_INPUT_LENGTH    ];
	bool fAll, found;
	int vnum = 0;
	int  col = 0;

	one_argument( argument, arg );
	if ( IS_NULLSTR(arg) )
	{
		send_to_char( "Syntax:  mlist <all/name>\n\r", ch );
		return FALSE;
	}

	buf1=new_buf();
	pArea = ch->in_room->area;
	/*    buf1[0] = '\0'; */
	fAll    = !str_cmp( arg, "all" );
	found   = FALSE;

	for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
	{
		if ( ( pMobIndex = get_mob_index( vnum ) ) != NULL )
		{
			if ( fAll || is_name( arg, pMobIndex->player_name ) )
			{
				found = TRUE;
				add_buf( buf1, (char *)Format("[%5d] %-17.16s", pMobIndex->vnum, capitalize( pMobIndex->short_descr )) );
				if ( ++col % 3 == 0 )
					add_buf( buf1, "\n\r" );
			}
		}
	}

	if ( !found )
	{
		send_to_char( "Mobile(s) not found in this area.\n\r", ch);
		return FALSE;
	}

	if ( col % 3 != 0 )
		add_buf( buf1, "\n\r" );

	page_to_char( buf_string(buf1), ch );
	free_buf(buf1);
	return FALSE;
}



REDIT( redit_olist )
{
	OBJ_INDEX_DATA	*pObjIndex;
	AREA_DATA		*pArea;
	BUFFER		*buf1;
	char		arg  [ MAX_INPUT_LENGTH    ];
	bool fAll, found;
	int vnum = 0;
	int  col = 0;

	one_argument( argument, arg );
	if ( IS_NULLSTR(arg) )
	{
		send_to_char( "Syntax:  olist <all/name/item_type>\n\r", ch );
		return FALSE;
	}

	pArea = ch->in_room->area;
	buf1=new_buf();
	/*    buf1[0] = '\0'; */
	fAll    = !str_cmp( arg, "all" );
	found   = FALSE;

	for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
	{
		if ( ( pObjIndex = get_obj_index( vnum ) ) )
		{
			if ( fAll || is_name( arg, pObjIndex->name )
					|| flag_value( type_flags, arg ) == pObjIndex->item_type )
			{
				found = TRUE;
				add_buf( buf1, (char *)Format("[%5d] %-17.16s", pObjIndex->vnum, capitalize( pObjIndex->short_descr )) );
				if ( ++col % 3 == 0 )
					add_buf( buf1, "\n\r" );
			}
		}
	}

	if ( !found )
	{
		send_to_char( "Object(s) not found in this area.\n\r", ch);
		return FALSE;
	}

	if ( col % 3 != 0 )
		add_buf( buf1, "\n\r" );

	page_to_char( buf_string(buf1), ch );
	free_buf(buf1);
	return FALSE;
}



REDIT( redit_mshow )
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( IS_NULLSTR(argument) )
    {
	send_to_char( "Syntax:  mshow <vnum>\n\r", ch );
	return FALSE;
    }

    if ( !is_number( argument ) )
    {
       send_to_char( "Syntax:  mshow <vnum>\n\r", ch);
       return FALSE;
    }

    if ( is_number( argument ) )
    {
	value = atoi( argument );
	if ( !( pMob = get_mob_index( value ) ))
	{
	    send_to_char( "REdit:  That mobile does not exist.\n\r", ch );
	    return FALSE;
	}

	ch->desc->pEdit = (void *)pMob;
    }

    medit_show( ch, argument );
    ch->desc->pEdit = (void *)ch->in_room;
    return FALSE;
}



REDIT( redit_oshow )
{
    OBJ_INDEX_DATA *pObj;
    int value;

    if ( IS_NULLSTR(argument) )
    {
	send_to_char( "Syntax:  oshow <vnum>\n\r", ch );
	return FALSE;
    }

    if ( !is_number( argument ) )
    {
       send_to_char( "Syntax:  mshow <vnum>\n\r", ch);
       return FALSE;
    }

    if ( is_number( argument ) )
    {
	value = atoi( argument );
	if ( !( pObj = get_obj_index( value ) ))
	{
	    send_to_char( "REdit:  That object does not exist.\n\r", ch );
	    return FALSE;
	}

	ch->desc->pEdit = (void *)pObj;
    }

    oedit_show( ch, argument );
    ch->desc->pEdit = (void *)ch->in_room;
    return FALSE;
}



/*****************************************************************************
 Name:		check_range( lower vnum, upper vnum )
 Purpose:	Ensures the range spans only one area.
 Called by:	aedit_vnum(olc_act.c).
 ****************************************************************************/
bool check_range( int lower, int upper )
{
    AREA_DATA *pArea;
    int cnt = 0;

    for ( pArea = area_first; pArea; pArea = pArea->next )
    {
	/*
	 * lower < area < upper
	 */
        if ( ( lower <= pArea->min_vnum && pArea->min_vnum <= upper )
	||   ( lower <= pArea->max_vnum && pArea->max_vnum <= upper ) )
	    ++cnt;

	if ( cnt > 1 )
	    return FALSE;
    }
    return TRUE;
}



AREA_DATA *get_vnum_area( int vnum )
{
    AREA_DATA *pArea;

    for ( pArea = area_first; pArea; pArea = pArea->next )
    {
        if ( vnum >= pArea->min_vnum
          && vnum <= pArea->max_vnum )
            return pArea;
    }

    return 0;
}



/*
 * Area Editor Functions.
 */
AEDIT( aedit_show )
{
    AREA_DATA *pArea;

    EDIT_AREA(ch, pArea);

    send_to_char( Format("Name:     [%5d] %s\n\r", pArea->vnum, pArea->name), ch );

#if 0  /* ROM OLC */
    send_to_char( Format("Recall:   [%5d] %s\n\r", pArea->recall, get_room_index( pArea->recall ) ? get_room_index( pArea->recall )->name : "none"), ch );
#endif /* ROM */

    send_to_char( Format("File:      %s\n\r", pArea->file_name), ch );
    send_to_char( Format("Vnums:     [%d-%d]\n\r", pArea->min_vnum, pArea->max_vnum), ch );
    send_to_char( Format("Age:       [%d]\n\r",	pArea->age), ch );
    send_to_char( Format("Players:   [%d]\n\r", pArea->nplayer), ch );
    send_to_char( Format("Security:  [%d]\n\r", pArea->security), ch );
    send_to_char( Format("Builders:  [%s]\n\r", pArea->builders), ch );
    send_to_char( Format("Credits :  [%s]\n\r", pArea->credits), ch );
    send_to_char( Format("Price Mod: [%d]\n\r", pArea->pricemod), ch);
    send_to_char( Format("Flags:    [%s]\n\r", flag_string( area_flags, pArea->area_flags )), ch );

    return FALSE;
}



AEDIT( aedit_reset )
{
    AREA_DATA *pArea;

    EDIT_AREA(ch, pArea);

    forced_reset_area( pArea );
    send_to_char( "Area reset.\n\r", ch );

    return FALSE;
}



AEDIT( aedit_create )
{
    AREA_DATA *pArea;

    pArea               =   new_area();
    area_last->next     =   pArea;
    area_last		=   pArea;	/* Thanks, Walker. */
    ch->desc->pEdit     =   (void *)pArea;

    SET_BIT( pArea->area_flags, AREA_ADDED );
    send_to_char( "Area Created.\n\r", ch );
    return FALSE;
}



AEDIT( aedit_name )
{
    AREA_DATA *pArea;

    EDIT_AREA(ch, pArea);

    if ( IS_NULLSTR(argument) )
    {
	send_to_char( "Syntax:   name [$name]\n\r", ch );
	return FALSE;
    }

    PURGE_DATA( pArea->name );
    pArea->name = str_dup( argument );

    send_to_char( "Name set.\n\r", ch );
    return TRUE;
}

AEDIT( aedit_credits )
{
	AREA_DATA *pArea;

	EDIT_AREA(ch, pArea);

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Syntax:   credits [$credits]\n\r", ch );
		return FALSE;
	}

	PURGE_DATA( pArea->credits );
	pArea->credits = str_dup( argument );

	send_to_char( "Credits set.\n\r", ch );
	return TRUE;
}

AEDIT( aedit_flags )
{
    AREA_DATA *pArea;
    int value;

    if ( !IS_NULLSTR(argument) )
    {
        EDIT_AREA( ch, pArea );

        if ( ( value = flag_value( area_flags, argument ) ) != NO_FLAG )
        {
            pArea->area_flags ^= value;
            send_to_char( "Area flag toggled.\n\r", ch);
            return TRUE;
        }
    }

    send_to_char( "Syntax: flags [flag]\n\r", ch );
    return FALSE;
}


AEDIT( aedit_file )
{
	AREA_DATA *pArea;
	AREA_DATA *tArea;
	char file[MSL]={'\0'};
	char file2[MSL]={'\0'};
	int i, length;
	bool found = FALSE;
	file2[0] = '\0';

	EDIT_AREA(ch, pArea);

	one_argument( argument, file );	/* Forces Lowercase */

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Syntax:  filename [$file]\n\r", ch );
		return FALSE;
	}

	/*
	 * Simple Syntax Check.
	 */
	length = strlen( argument );
	if ( length > 15 )
	{
		send_to_char( "No more than eight characters allowed.\n\r", ch );
		return FALSE;
	}

	/*
	 * Allow only letters and numbers.
	 */
	for ( i = 0; i < length; i++ )
	{
		if ( !isalnum( file[i] ) )
		{
			send_to_char( "Only letters and numbers are valid.\n\r", ch );
			return FALSE;
		}
	}

	/* Prevent filename overwrite by Takeda (takeda@mathlab.sunysb.edu) */
	strncat( file2, file, sizeof(file2) - strlen(file2) - 1 );
	strncat( file2, ".are", sizeof(file2) - strlen(file2) - 1 );
	for ( tArea = area_first; tArea; tArea = tArea->next )
	{
		if(!str_cmp(tArea->file_name,file2)) {
			found = TRUE;
			continue;
		}
	}
	if(found == TRUE) {
		send_to_char("There is a file with the same name!!\n",ch);
		return FALSE;
	}

	PURGE_DATA( pArea->file_name );
	strncat( file, ".are", sizeof(file2) - strlen(file2) - 1 );
	pArea->file_name = str_dup( file );

	send_to_char( "Filename set.\n\r", ch );
	return TRUE;
}



AEDIT( aedit_age )
{
    AREA_DATA *pArea;
    char age[MSL]={'\0'};

    EDIT_AREA(ch, pArea);

    one_argument( argument, age );

    if ( !is_number( age ) || IS_NULLSTR(age) )
    {
	send_to_char( "Syntax:  age [#xage]\n\r", ch );
	return FALSE;
    }

    pArea->age = atoi( age );

    send_to_char( "Age set.\n\r", ch );
    return TRUE;
}


#if 0 /* ROM OLC */
AEDIT( aedit_recall )
{
    AREA_DATA *pArea;
    char room[MSL]={'\0'};
    int  value;

    EDIT_AREA(ch, pArea);

    one_argument( argument, room );

    if ( !is_number( argument ) || IS_NULLSTR(argument) )
    {
	send_to_char( "Syntax:  recall [#xrvnum]\n\r", ch );
	return FALSE;
    }

    value = atoi( room );

    if ( !get_room_index( value ) )
    {
	send_to_char( "AEdit:  Room vnum does not exist.\n\r", ch );
	return FALSE;
    }

    pArea->recall = value;

    send_to_char( "Recall set.\n\r", ch );
    return TRUE;
}
#endif /* ROM OLC */


AEDIT( aedit_security )
{
	AREA_DATA *pArea;
	char sec[MSL]={'\0'};
	int  value;

	EDIT_AREA(ch, pArea);

	one_argument( argument, sec );

	if ( !is_number( sec ) || IS_NULLSTR(sec) )
	{
		send_to_char( "Syntax:  security [#xlevel]\n\r", ch );
		return FALSE;
	}

	value = atoi( sec );

	if ( value > ch->pcdata->security || value < 0 )
	{
		if ( ch->pcdata->security != 0 )
		{
			send_to_char( Format("Security is 0-%d.\n\r", ch->pcdata->security), ch );
		}
		else
			send_to_char( "Security is 0 only.\n\r", ch );
		return FALSE;
	}

	pArea->security = value;

	send_to_char( "Security set.\n\r", ch );
	return TRUE;
}



AEDIT( aedit_builder )
{
    AREA_DATA *pArea;
    char name[MSL]={'\0'};
    char buf[MSL]={'\0'};

    EDIT_AREA(ch, pArea);

    one_argument( argument, name );

    if ( IS_NULLSTR(name) )
    {
	send_to_char( "Syntax:  builder [$name]  -toggles builder\n\r", ch );
	send_to_char( "Syntax:  builder All      -allows everyone\n\r", ch );
	return FALSE;
    }

    name[0] = UPPER( name[0] );

    if ( strstr( pArea->builders, name ) != NULL )
    {
	pArea->builders = string_replace( pArea->builders, name, "\0" );
	pArea->builders = string_unpad( pArea->builders );

	if ( pArea->builders[0] == '\0' )
	{
		PURGE_DATA( pArea->builders );
	    pArea->builders = str_dup( "None" );
	}
	send_to_char( "Builder removed.\n\r", ch );
	return TRUE;
    }
    else
    {
	buf[0] = '\0';
	if ( strstr( pArea->builders, "None" ) != NULL )
	{
	    pArea->builders = string_replace( pArea->builders, "None", "\0" );
	    pArea->builders = string_unpad( pArea->builders );
	}

	if (pArea->builders[0] != '\0' )
	{
	    strncat( buf, pArea->builders, sizeof(buf) - strlen(buf) - 1 );
	    strncat( buf, " ", sizeof(buf) - strlen(buf) - 1 );
	}
	strncat( buf, name, sizeof(buf) - strlen(buf) - 1);
	PURGE_DATA( pArea->builders );
	pArea->builders = string_proper( str_dup( buf ) );

	send_to_char( "Builder added.\n\r", ch );
	send_to_char( pArea->builders,ch);
	return TRUE;
    }

    return FALSE;
}



AEDIT( aedit_vnum )
{
    AREA_DATA *pArea;
    char lower[MSL]={'\0'};
    char upper[MSL]={'\0'};
    int  ilower;
    int  iupper;

    EDIT_AREA(ch, pArea);

    argument = one_argument( argument, lower );
    one_argument( argument, upper );

    if ( !is_number( lower ) || IS_NULLSTR(lower)
    || !is_number( upper ) || IS_NULLSTR(upper) )
    {
	send_to_char( "Syntax:  vnum [#xlower] [#xupper]\n\r", ch );
	return FALSE;
    }

    if ( ( ilower = atoi( lower ) ) > ( iupper = atoi( upper ) ) )
    {
	send_to_char( "AEdit:  Upper must be larger then lower.\n\r", ch );
	return FALSE;
    }

    if ( !check_range( atoi( lower ), atoi( upper ) ) )
    {
	send_to_char( "AEdit:  Range must include only this area.\n\r", ch );
	return FALSE;
    }

    if ( get_vnum_area( ilower )
    && get_vnum_area( ilower ) != pArea )
    {
	send_to_char( "AEdit:  Lower vnum already assigned.\n\r", ch );
	return FALSE;
    }

    pArea->min_vnum = ilower;
    send_to_char( "Lower vnum set.\n\r", ch );

    if ( get_vnum_area( iupper )
    && get_vnum_area( iupper ) != pArea )
    {
	send_to_char( "AEdit:  Upper vnum already assigned.\n\r", ch );
	return TRUE;	/* The lower value has been set. */
    }

    pArea->max_vnum = iupper;
    send_to_char( "Upper vnum set.\n\r", ch );

    return TRUE;
}



AEDIT( aedit_lvnum )
{
    AREA_DATA *pArea;
    char lower[MSL]={'\0'};
    int  ilower;
    int  iupper;

    EDIT_AREA(ch, pArea);

    one_argument( argument, lower );

    if ( !is_number( lower ) || IS_NULLSTR(lower) )
    {
	send_to_char( "Syntax:  min_vnum [#xlower]\n\r", ch );
	return FALSE;
    }

    if ( ( ilower = atoi( lower ) ) > ( iupper = pArea->max_vnum ) )
    {
	send_to_char( "AEdit:  Value must be less than the max_vnum.\n\r", ch );
	return FALSE;
    }

    if ( !check_range( ilower, iupper ) )
    {
	send_to_char( "AEdit:  Range must include only this area.\n\r", ch );
	return FALSE;
    }

    if ( get_vnum_area( ilower )
    && get_vnum_area( ilower ) != pArea )
    {
	send_to_char( "AEdit:  Lower vnum already assigned.\n\r", ch );
	return FALSE;
    }

    pArea->min_vnum = ilower;
    send_to_char( "Lower vnum set.\n\r", ch );
    return TRUE;
}



AEDIT( aedit_uvnum )
{
    AREA_DATA *pArea;
    char upper[MSL]={'\0'};
    int  ilower;
    int  iupper;

    EDIT_AREA(ch, pArea);

    one_argument( argument, upper );

    if ( !is_number( upper ) || IS_NULLSTR(upper) )
    {
	send_to_char( "Syntax:  max_vnum [#xupper]\n\r", ch );
	return FALSE;
    }

    if ( ( ilower = pArea->min_vnum ) > ( iupper = atoi( upper ) ) )
    {
	send_to_char( "AEdit:  Upper must be larger then lower.\n\r", ch );
	return FALSE;
    }

    if ( !check_range( ilower, iupper ) )
    {
	send_to_char( "AEdit:  Range must include only this area.\n\r", ch );
	return FALSE;
    }

    if ( get_vnum_area( iupper )
    && get_vnum_area( iupper ) != pArea )
    {
	send_to_char( "AEdit:  Upper vnum already assigned.\n\r", ch );
	return FALSE;
    }

    pArea->max_vnum = iupper;
    send_to_char( "Upper vnum set.\n\r", ch );

    return TRUE;
}



/*
 * Room Editor Functions.
 */
REDIT( redit_show )
{
	ROOM_INDEX_DATA	*pRoom;
	char		buf  [MSL]={'\0'};
	char		buf1 [2*MSL]={'\0'};
	OBJ_DATA		*obj;
	CHAR_DATA		*rch;
	int			door;
	bool		fcnt;

	EDIT_ROOM(ch, pRoom);

	buf1[0] = '\0';

	strncat( buf1, Format("\tWArea:\tn       [%5d] %s\n\r", pRoom->area->vnum, pRoom->area->name), sizeof(buf1) - strlen(buf1) - 1 );
	strncat( buf1, Format("\tWVnum:\tn       [%5d]\n\r", pRoom->vnum ), sizeof(buf1) - strlen(buf1) - 1 );
	strncat( buf1, Format("\tWName:\tn       [%s]\n\r", pRoom->name), sizeof(buf1) - strlen(buf1) - 1 );
	strncat( buf1, Format("\tWDescription:\tn\n\r%s\n\r", pRoom->description), sizeof(buf1) - strlen(buf1) - 1 );
	strncat( buf1, Format("\tWSector:\tn     [%s]\n\r", flag_string( sector_flags, pRoom->sector_type )), sizeof(buf1) - strlen(buf1) - 1);
	strncat( buf1, Format("\tYUmbra Name:\tn       [%s]\n\r", pRoom->uname), sizeof(buf1) - strlen(buf1) - 1 );
	strncat( buf1, Format("\tYUmbra Description:\tn\n\r%s\n\r", pRoom->udescription), sizeof(buf1) - strlen(buf1) - 1 );
	strncat( buf1, Format("\tWRoom flags:\tn [%s]\n\r", flag_string( room_flags, pRoom->room_flags )), sizeof(buf1) - strlen(buf1) - 1 );
	strncat( buf1, Format("\tWHealth recovery:\tn [%d]\n\r", pRoom->heal_rate), sizeof(buf1) - strlen(buf1) - 1 );
	strncat( buf1, Format("\tWClan:\tn [%d] %s\n\r" , pRoom->clan, ((pRoom->clan > 0) ? clan_table[pRoom->clan].name : "none" )), sizeof(buf1) - strlen(buf1) - 1 );

	strncat( buf1, "\tWCharacters in Room:\tn [" , sizeof(buf1) - strlen(buf1) - 1);
	fcnt = FALSE;
	for ( rch = pRoom->people; rch; rch = rch->next_in_room )
	{
		one_argument( rch->name, buf );
		strncat( buf1, buf, sizeof(buf1) - strlen(buf1) - 1 );
		strncat( buf1, " ", sizeof(buf1) - strlen(buf1) - 1 );
		fcnt = TRUE;
	}

	if ( fcnt )
	{
		int end = 0;

		end = strlen(buf1) - 1;
		buf1[end] = ']';
		strncat( buf1, "\n\r", sizeof(buf1) - strlen(buf1) - 1 );
	}
	else
	{
		strncat( buf1, "none]\n\r", sizeof(buf1) - strlen(buf1) - 1 );
	}

	strncat( buf1, "\tWObjects in Room:\tn    [", sizeof(buf1) - strlen(buf1) - 1 );
	fcnt = FALSE;
	for ( obj = pRoom->contents; obj; obj = obj->next_content )
	{
		one_argument( obj->name, buf );
		strncat( buf1, buf, sizeof(buf1) - strlen(buf1) - 1 );
		strncat( buf1, " ", sizeof(buf1) - strlen(buf1) - 1 );
		fcnt = TRUE;
	}

	if ( fcnt )
	{
		int end = 0;

		end = strlen(buf1) - 1;
		buf1[end] = ']';
		strncat( buf1, "\n\r", sizeof(buf1) - strlen(buf1) - 1 );
	}
	else
	{
		strncat( buf1, "none]\n\r", sizeof(buf1) - strlen(buf1) - 1 );
	}

	// Doors
	strncat( buf1, "\n\r\tG<----\tWDoors\tG---->\tn\n\r", sizeof(buf1) - strlen(buf1) - 1 );
	for ( door = 0; door < MAX_DIR; door++ )
	{
		EXIT_DATA *pexit;

		if ( ( pexit = pRoom->exit[door] ) )
		{
			char word[MAX_INPUT_LENGTH]={'\0'};
			char reset_state[MSL]={'\0'};
			char *state;
			int i, length;

			strncat( buf1, Format("-%-5s to [%5d] Key: [%5d] ",
				capitalize(dir_name[door]), pexit->u1.to_room ? pexit->u1.to_room->vnum : 0, pexit->key), sizeof(buf1) - strlen(buf1) - 1 );

			/*
			 * Format up the exit info.
			 * Capitalize all flags that are not part of the reset info.
			 */
			strncpy( reset_state, flag_string( exit_flags, pexit->rs_flags ), MSL );
			state = flag_string( exit_flags, pexit->exit_info );
			strncat( buf1, " Exit flags: [", sizeof(buf1) - strlen(buf1) - 1 );
			for (; ;)
			{
				state = one_argument( state, word );

				if ( word[0] == '\0' )
				{
					int end;

					end = strlen(buf1) - 1;
					buf1[end] = ']';
					strncat( buf1, "\n\r",sizeof(buf1) - strlen(buf1) - 1 );
					break;
				}

				if ( str_infix( word, reset_state ) )
				{
					length = strlen(word);
					for (i = 0; i < length; i++)
						word[i] = UPPER(word[i]);
				}
				strncat( buf1, word, sizeof(buf1) - strlen(buf1) - 1 );
				strncat( buf1, " ", sizeof(buf1) - strlen(buf1) - 1 );
			}

			if ( pexit->keyword && pexit->keyword[0] != '\0' )
			{
				strncat( buf1, Format("Kwds: [%s]\n\r", pexit->keyword), sizeof(buf1) - strlen(buf1) - 1 );
			}
			if ( pexit->description && pexit->description[0] != '\0' )
			{
				strncat( buf1, Format("%s", pexit->description), sizeof(buf1) - strlen(buf1) - 1 );
			}

			if(pexit->jumpto.to_room)
			{
				strncat( buf1, Format("-%-5s has jumpto link to [%5d]\n\r",
					capitalize(dir_name[door]), pexit->jumpto.to_room?pexit->jumpto.to_room->vnum:0), sizeof(buf1) - strlen(buf1) - 1 );
			}
		}
	}

	// Begin showing optional flags
	strncat( buf1, "\n\r\tG<----\tWOptional Room Settings\tG---->\tn\n\r", sizeof(buf1) - strlen(buf1) - 1 );
	// Is the room owned by someone?
	if(pRoom->owner != NULL && pRoom->owner[0] != '\0')
	{
		strncat( buf1, Format("\tWOwner:\tn [%s]\n\r" , pRoom->owner), sizeof(buf1) - strlen(buf1) - 1 );
	}

	// Are there extra descriptions being used?
	if ( pRoom->extra_descr )
	{
		EXTRA_DESCR_DATA *ed;

		strncat( buf1, "\tWExtra Desc Keywords:\tn  [", sizeof(buf1) - strlen(buf1) - 1 );
		for ( ed = pRoom->extra_descr; ed; ed = ed->next )
		{
			strncat( buf1, ed->keyword, sizeof(buf1) - strlen(buf1) - 1 );
			if ( ed->next )
			{
				strncat( buf1, " ", sizeof(buf1) - strlen(buf1) - 1 );
			}
		}
		strncat( buf1, "]\n\r", sizeof(buf1) - strlen(buf1) - 1 );
	}

	// Is this room a room stop for elevators?
	if(IS_SET(pRoom->room_flags, ROOM_STOP))
	{
		strncat( buf1, "\tWStop for elevator:\tn  [Room vnum: ", sizeof(buf1) - strlen(buf1) - 1 );
		strncat( buf1, Format("%d Stop number: %d", pRoom->car, pRoom->stops[0]), sizeof(buf1) - strlen(buf1) - 1 );
		strncat( buf1, "]\n\r", sizeof(buf1) - strlen(buf1) - 1 );
	}

	// Is this room a vehicle?
	if(IS_SET(pRoom->room_flags, ROOM_VEHICLE))
	{
		strncat( buf1, "\tWInitially Parked:\tn  [", sizeof(buf1) - strlen(buf1) - 1 );
		strncat( buf1, Format("%d", pRoom->stops[0]), sizeof(buf1) - strlen(buf1) - 1 );
		strncat( buf1, "]\n\r", sizeof(buf1) - strlen(buf1) - 1 );
	}

	// Is this a stop for train or elevators?
	if(IS_SET(pRoom->room_flags, ROOM_TRAIN) || IS_SET(pRoom->room_flags, ROOM_ELEVATOR))
	{
		strncat( buf1, Format("Stops (1-%d):  ", MAX_CAR_STOPS), sizeof(buf1) - strlen(buf1) - 1);
		for(door = 1; door < MAX_CAR_STOPS; door++)
		{
			if(pRoom->stops[door] > 0) {
				strncat( buf1, Format("[%d] %d ", door, pRoom->stops[door]), sizeof(buf1) - strlen(buf1) - 1 );
			}
		}
		strncat( buf1, "\n\r", sizeof(buf1) - strlen(buf1) - 1 );
	}

	send_to_char( buf1, ch );
	return FALSE;
}



/* Local function. */
bool change_exit( CHAR_DATA *ch, char *argument, int door, int thissideonly )
{
    ROOM_INDEX_DATA *pRoom;
    char command[MAX_INPUT_LENGTH]={'\0'};
    char arg[MAX_INPUT_LENGTH]={'\0'};
    int  value = 0;

    EDIT_ROOM(ch, pRoom);

    /*
     * Set the exit flags, needs full argument.
     * ----------------------------------------
     * (Fixed to allow keywords to be flags.)
     */
    if ( ( value = flag_value( exit_flags, argument ) ) != NO_FLAG
	&& str_prefix("name", argument) )
    {
	ROOM_INDEX_DATA *pToRoom;
	sh_int rev;                                    /* ROM OLC */

	if ( !pRoom->exit[door] )
	   {
	   	send_to_char("That exit does not exist.\n\r",ch);
	   	return FALSE;
	   }
	 /*   pRoom->exit[door] = new_exit(); */

	/*
	 * This room.
	 */
	TOGGLE_BIT(pRoom->exit[door]->rs_flags,  value);
	/* Don't toggle exit_info because it can be changed by players. */
	pRoom->exit[door]->exit_info = pRoom->exit[door]->rs_flags;

	/*
	 * Connected room.
	 */
	pToRoom = pRoom->exit[door]->u1.to_room;     /* ROM OLC */
	rev = rev_dir[door];

	if (pToRoom->exit[rev] != NULL && !thissideonly)
	{
	   TOGGLE_BIT(pToRoom->exit[rev]->rs_flags,  value);
	   TOGGLE_BIT(pToRoom->exit[rev]->exit_info, value);
	}

	send_to_char( "Exit flag toggled.\n\r", ch );
	return TRUE;
    }

    /*
     * Now parse the arguments.
     */
    argument = one_argument( argument, command );
    one_argument( argument, arg );

    if ( IS_NULLSTR(command) && IS_NULLSTR(argument) )	/* Move command. */
    {
	move_char( ch, door, TRUE );                    /* ROM OLC */
	return FALSE;
    }

    if ( command[0] == '?' )
    {
	do_help( ch, "EXIT" );
	return FALSE;
    }

    if ( !str_cmp( command, "delete" ) )
    {
	ROOM_INDEX_DATA *pToRoom;
	sh_int rev;                                     /* ROM OLC */

	if ( !pRoom->exit[door] )
	{
	    send_to_char( "REdit:  Cannot delete a null exit.\n\r", ch );
	    return FALSE;
	}

	/*
	 * Remove ToRoom Exit.
	 */
	rev = rev_dir[door];
	pToRoom = pRoom->exit[door]->u1.to_room;       /* ROM OLC */

	if ( pToRoom->exit[rev] )
	{
	    free_exit( pToRoom->exit[rev] );
	    pToRoom->exit[rev] = NULL;
	}

	/*
	 * Remove this exit.
	 */
	free_exit( pRoom->exit[door] );
	pRoom->exit[door] = NULL;

	send_to_char( "Exit unlinked.\n\r", ch );
	return TRUE;
    }

    if ( !str_cmp( command, "link" ) )
    {
	EXIT_DATA *pExit;

	if ( IS_NULLSTR(arg) || !is_number( arg ) )
	{
	    send_to_char( "Syntax:  [direction] link [vnum]\n\r", ch );
	    return FALSE;
	}

	value = atoi( arg );

	if ( !get_room_index( value ) )
	{
	    send_to_char( "REdit:  Cannot link to non-existant room.\n\r", ch );
	    return FALSE;
	}

	if ( !IS_BUILDER( ch, get_room_index( value )->area ) )
	{
	    send_to_char( "REdit:  Cannot link to that area.\n\r", ch );
	    return FALSE;
	}

	if ( get_room_index( value )->exit[rev_dir[door]] )
	{
	    send_to_char( "REdit:  Remote side's exit already exists.\n\r", ch );
	    return FALSE;
	}

	if ( !pRoom->exit[door] )
	{
	    pRoom->exit[door] = new_exit();
	}

	pRoom->exit[door]->u1.to_room = get_room_index( value );   /* ROM OLC */
	pRoom->exit[door]->orig_door = door;

/*	pRoom->exit[door]->vnum = value;                Can't set vnum in ROM */

	pRoom                   = get_room_index( value );
	door                    = rev_dir[door];
	pExit                   = new_exit();
	pExit->u1.to_room       = ch->in_room;
/*	pExit->vnum             = ch->in_room->vnum;    Can't set vnum in ROM */
	pExit->orig_door	= door;
	pRoom->exit[door]       = pExit;

	send_to_char( "Two-way link established.\n\r", ch );
	return TRUE;
    }

    if ( !str_cmp( command, "jumpto" ) )
    {
	if ( IS_NULLSTR(arg) || !is_number( arg ) )
	{
	    send_to_char( "Syntax:  [direction] jumpto [vnum]\n\r", ch );
	    return FALSE;
	}

	value = atoi( arg );

	if ( !get_room_index( value ) )
	{
	    send_to_char("REdit:  Cannot set jumpto to non-existant room.\n\r",
		ch );
	    return FALSE;
	}

	if ( !IS_BUILDER( ch, get_room_index( value )->area ) )
	{
	    send_to_char( "REdit:  Cannot set jumpto to that area.\n\r", ch );
	    return FALSE;
	}

	if ( pRoom->exit[door] == NULL )
	{
	    send_to_char( "REdit:  No exit exists in that direction.\n\r", ch );
	    return FALSE;
	}

	pRoom->exit[door]->jumpto.to_room = get_room_index( value );/* PT OLC */

	send_to_char( "Jump-to target established.\n\r", ch );
	return TRUE;
    }

    if ( !str_cmp( command, "deljumpto" ) )
    {
	if ( !pRoom->exit[door] )
	{
	    send_to_char( "No exit in that direction.\n\r", ch );
	    return FALSE;
	}

	if ( pRoom->exit[door]->jumpto.to_room == NULL )
	{
	    send_to_char( "No jumpto target in that direction.\n\r", ch );
	    return FALSE;
	}

	pRoom->exit[door]->jumpto.to_room = NULL;/* PT OLC */

	send_to_char( "Jump-to target removed.\n\r", ch );
	return TRUE;
    }

    if ( !str_cmp( command, "dig" ) )
    {

	if ( IS_NULLSTR(arg) || !is_number( arg ) )
	{
	    send_to_char( "Syntax: [direction] dig <vnum>\n\r", ch );
	    return FALSE;
	}

	redit_create( ch, arg );
	change_exit( ch, (char *)Format("link %s", arg), door, 0);
	return TRUE;
    }

    if ( !str_cmp( command, "room" ) )
    {
	if ( IS_NULLSTR(arg) || !is_number( arg ) )
	{
	    send_to_char( "Syntax:  [direction] room [vnum]\n\r", ch );
	    return FALSE;
	}

	if ( !pRoom->exit[door] )
	{
	    pRoom->exit[door] = new_exit();
	}

	value = atoi( arg );

	if ( !get_room_index( value ) )
	{
	    send_to_char( "REdit:  Cannot link to non-existant room.\n\r", ch );
	    return FALSE;
	}

	pRoom->exit[door]->u1.to_room = get_room_index( value );    /* ROM OLC */
	pRoom->exit[door]->orig_door = door;
/*	pRoom->exit[door]->vnum = value;                 Can't set vnum in ROM */

	send_to_char( "One-way link established.\n\r", ch );
	return TRUE;
    }

    if ( !str_cmp( command, "key" ) )
    {
	if ( IS_NULLSTR(arg) || !is_number( arg ) )
	{
	    send_to_char( "Syntax:  [direction] key [vnum]\n\r", ch );
	    return FALSE;
	}

	if ( !pRoom->exit[door] )
	{
	    pRoom->exit[door] = new_exit();
	}

	value = atoi( arg );

	if ( !get_obj_index( value ) )
	{
	    send_to_char( "REdit:  Item doesn't exist.\n\r", ch );
	    return FALSE;
	}

	if ( get_obj_index( atoi( argument ) )->item_type != ITEM_KEY )
	{
	    send_to_char( "REdit:  Key doesn't exist.\n\r", ch );
	    return FALSE;
	}

	pRoom->exit[door]->key = value;

	send_to_char( "Exit key set.\n\r", ch );
	return TRUE;
    }

    if ( !str_cmp( command, "name" ) )
    {
	if ( IS_NULLSTR(arg) )
	{
	    send_to_char( "Syntax:  [direction] name [string]\n\r", ch );
	    return FALSE;
	}

	if ( !pRoom->exit[door] )
	{
	    pRoom->exit[door] = new_exit();
	}

	PURGE_DATA( pRoom->exit[door]->keyword );
	pRoom->exit[door]->keyword = str_dup( arg );

	send_to_char( "Exit name set.\n\r", ch );
	return TRUE;
    }

    if ( !str_prefix( command, "description" ) )
    {
	if ( IS_NULLSTR(arg) )
	{
	    if ( !pRoom->exit[door] )
	    {
	        pRoom->exit[door] = new_exit();
	    }
	    string_append( ch, &pRoom->exit[door]->description );
	    return TRUE;
	}

	send_to_char( "Syntax:  [direction] desc\n\r", ch );
	return FALSE;
    }

    return FALSE;
}



REDIT( redit_north )
{
    if ( change_exit( ch, argument, DIR_NORTH, 0 ) )
	return TRUE;

    return FALSE;
}


REDIT( redit_south )
{
    if ( change_exit( ch, argument, DIR_SOUTH, 0 ) )
	return TRUE;

    return FALSE;
}


REDIT( redit_east )
{
    if ( change_exit( ch, argument, DIR_EAST, 0 ) )
	return TRUE;

    return FALSE;
}


REDIT( redit_west )
{
    if ( change_exit( ch, argument, DIR_WEST, 0 ) )
	return TRUE;

    return FALSE;
}


REDIT( redit_up )
{
    if ( change_exit( ch, argument, DIR_UP, 0 ) )
	return TRUE;

    return FALSE;
}


REDIT( redit_down )
{
    if ( change_exit( ch, argument, DIR_DOWN, 0 ) )
	return TRUE;

    return FALSE;
}


REDIT( redit_nearnorth )
{
    if ( change_exit( ch, argument, DIR_NORTH, 0 ) )
	return TRUE;

    return FALSE;
}


REDIT( redit_nearsouth )
{
    if ( change_exit( ch, argument, DIR_SOUTH, 0 ) )
	return TRUE;

    return FALSE;
}


REDIT( redit_neareast )
{
    if ( change_exit( ch, argument, DIR_EAST, 0 ) )
	return TRUE;

    return FALSE;
}


REDIT( redit_nearwest )
{
    if ( change_exit( ch, argument, DIR_WEST, 0 ) )
	return TRUE;

    return FALSE;
}


REDIT( redit_nearup )
{
    if ( change_exit( ch, argument, DIR_UP, 0 ) )
	return TRUE;

    return FALSE;
}


REDIT( redit_neardown )
{
    if ( change_exit( ch, argument, DIR_DOWN, 0 ) )
	return TRUE;

    return FALSE;
}



REDIT( redit_ed )
{
    ROOM_INDEX_DATA *pRoom;
    EXTRA_DESCR_DATA *ed;
    char command[MAX_INPUT_LENGTH]={'\0'};
    char keyword[MAX_INPUT_LENGTH]={'\0'};

    EDIT_ROOM(ch, pRoom);

    argument = one_argument( argument, command );
    one_argument( argument, keyword );

    if ( IS_NULLSTR(command) || IS_NULLSTR(keyword) )
    {
	send_to_char( "Syntax:  ed add [keyword]\n\r", ch );
	send_to_char( "         ed edit [keyword]\n\r", ch );
	send_to_char( "         ed delete [keyword]\n\r", ch );
	send_to_char( "         ed format [keyword]\n\r", ch );
	return FALSE;
    }

    if ( !str_cmp( command, "add" ) )
    {
	if ( IS_NULLSTR(keyword) )
	{
	    send_to_char( "Syntax:  ed add [keyword]\n\r", ch );
	    return FALSE;
	}

	ed			=   new_extra_descr();
	PURGE_DATA( ed->keyword );
	PURGE_DATA( ed->description );
	ed->keyword		=   str_dup( keyword );
	ed->description		=   NULL;
	ed->next		=   pRoom->extra_descr;
	pRoom->extra_descr	=   ed;

	string_append( ch, &ed->description );

	return TRUE;
    }


    if ( !str_cmp( command, "edit" ) )
    {
	if ( IS_NULLSTR(keyword) )
	{
	    send_to_char( "Syntax:  ed edit [keyword]\n\r", ch );
	    return FALSE;
	}

	for ( ed = pRoom->extra_descr; ed; ed = ed->next )
	{
	    if ( is_name( keyword, ed->keyword ) )
		break;
	}

	if ( !ed )
	{
	    send_to_char( "REdit:  Extra description keyword not found.\n\r", ch );
	    return FALSE;
	}

	string_append( ch, &ed->description );

	return TRUE;
    }


    if ( !str_cmp( command, "delete" ) )
    {
	EXTRA_DESCR_DATA *ped = NULL;

	if ( IS_NULLSTR(keyword) )
	{
	    send_to_char( "Syntax:  ed delete [keyword]\n\r", ch );
	    return FALSE;
	}

	for ( ed = pRoom->extra_descr; ed; ed = ed->next )
	{
	    if ( is_name( keyword, ed->keyword ) )
		break;
	    ped = ed;
	}

	if ( !ed )
	{
	    send_to_char( "REdit:  Extra description keyword not found.\n\r", ch );
	    return FALSE;
	}

	if ( !ped )
	    pRoom->extra_descr = ed->next;
	else
	    ped->next = ed->next;

	free_extra_descr( ed );

	send_to_char( "Extra description deleted.\n\r", ch );
	return TRUE;
    }


    if ( !str_cmp( command, "format" ) )
    {
	if ( IS_NULLSTR(keyword) )
	{
	    send_to_char( "Syntax:  ed format [keyword]\n\r", ch );
	    return FALSE;
	}

	for ( ed = pRoom->extra_descr; ed; ed = ed->next )
	{
	    if ( is_name( keyword, ed->keyword ) )
		break;
	}

	if ( !ed )
	{
	    send_to_char( "REdit:  Extra description keyword not found.\n\r", ch );
	    return FALSE;
	}

	ed->description = format_string( ed->description );

	send_to_char( "Extra description formatted.\n\r", ch );
	return TRUE;
    }

    redit_ed( ch, "" );
    return FALSE;
}



REDIT( redit_create )
{
    AREA_DATA *pArea;
    ROOM_INDEX_DATA *pRoom;
    int value;
    int iHash;

    EDIT_ROOM(ch, pRoom);

    value = atoi( argument );

    if ( IS_NULLSTR(argument) || value <= 0 )
    {
	send_to_char( "Syntax:  create [vnum > 0]\n\r", ch );
	return FALSE;
    }

    pArea = get_vnum_area( value );
    if ( !pArea )
    {
	send_to_char( "REdit:  That vnum is not assigned an area.\n\r", ch );
	return FALSE;
    }

    if ( !IS_BUILDER( ch, pArea ) )
    {
	send_to_char( "REdit:  Vnum in an area you cannot build in.\n\r", ch );
	return FALSE;
    }

    if ( get_room_index( value ) )
    {
	send_to_char( "REdit:  Room vnum already exists.\n\r", ch );
	return FALSE;
    }

    pRoom			= new_room_index();
    pRoom->area			= pArea;
    pRoom->vnum			= value;

    if ( value > top_vnum_room )
        top_vnum_room = value;

    iHash			= value % MAX_KEY_HASH;
    pRoom->next			= room_index_hash[iHash];
    room_index_hash[iHash]	= pRoom;
    ch->desc->pEdit		= (void *)pRoom;

    send_to_char( "Room created.\n\r", ch );
    return TRUE;
}



REDIT( redit_stop )
{
	ROOM_INDEX_DATA *pRoom;
	ROOM_INDEX_DATA *pR;
	ROOM_INDEX_DATA *pOldR;
	int value1, value2;
	int i;
	char arg[MAX_INPUT_LENGTH]={'\0'};

	EDIT_ROOM(ch, pRoom);

	argument = one_argument(argument, arg);

	if ( IS_NULLSTR(arg) || IS_NULLSTR(argument) )
	{
		if(IS_SET(pRoom->room_flags, ROOM_STOP))
		{
			send_to_char( "Stop for elevator:  [Room vnum: ", ch );
			send_to_char( Format("%d Stop number: %d]\n\r", pRoom->car, pRoom->stops[0]), ch );
		}

		else if(IS_SET(pRoom->room_flags, ROOM_VEHICLE))
		{
			send_to_char( "Initially Parked:  [", ch );
			send_to_char( Format("%d]\n\r", pRoom->stops[0]), ch );
		}

		else if(IS_SET(pRoom->room_flags, ROOM_TRAIN)
				|| IS_SET(pRoom->room_flags, ROOM_ELEVATOR))
		{
			send_to_char( Format("Stops (1-%d):  ", MAX_CAR_STOPS), ch);
			for(i = 1; i < MAX_CAR_STOPS; i++)
			{
				if(pRoom->stops[i] > 0) {
					send_to_char( Format("[%d] %d ", i, pRoom->stops[i]), ch );
				}
			}
			send_to_char( "\n\r", ch );
		}
		return FALSE;
	}

	if(!is_number(arg) || !is_number(argument))
	{
		send_to_char( Format("Syntax:  stop [1-%d] [vnum]\n\r", MAX_CAR_STOPS), ch );
		return FALSE;
	}

	value1 = atoi( argument );
	value2 = atoi( arg );

	if ( 0 > value2 || value2 > MAX_CAR_STOPS
			|| (pR = get_room_index(value1)) == NULL
			|| (value2 == 0 && IS_SET(pRoom->room_flags, ROOM_ELEVATOR)))
	{
		send_to_char( Format("Syntax:  stop [0-%d] [vnum]\n\r", MAX_CAR_STOPS), ch );
		send_to_char( "(0 is only available for vehicles.)", ch );
		return FALSE;
	}

	if(!IS_SET(pRoom->room_flags, ROOM_ELEVATOR)
			&& !IS_SET(pRoom->room_flags, ROOM_TRAIN)
			&& !IS_SET(pRoom->room_flags, ROOM_VEHICLE))
	{
		send_to_char("Register the stop from its respective elevator or train.\n\r", ch);
		return FALSE;
	}

	if(IS_SET(pR->room_flags, ROOM_STOP) && pR->car != pRoom->vnum)
	{
		send_to_char("That room is already a stop for something else.\n\r", ch);
		return FALSE;
	}

	if(IS_SET(pR->room_flags, ROOM_ELEVATOR)
			|| IS_SET(pR->room_flags, ROOM_TRAIN)
			|| IS_SET(pR->room_flags, ROOM_VEHICLE))
	{
		send_to_char("That room is already set up as a vehicle of some type.\n\r", ch);
		return FALSE;
	}

	if(IS_SET(pRoom->room_flags, ROOM_VEHICLE) && value2 > 0)
	{
		send_to_char("A vehicle can have only one stop. (Stop 0)\n\r", ch);
		return FALSE;
	}

    if(pRoom->stops[value2] != pR->vnum && pRoom->stops[value2] > 0
    		&& !IS_SET(pRoom->room_flags, ROOM_VEHICLE))
    {
    	if((pOldR = get_room_index(pRoom->stops[value2])) != NULL)
    	{
    		send_to_char(Format("%d configured as stop %d for room %d\n\r", pOldR->vnum, pOldR->stops[0], pRoom->vnum), ch);
    		if(IS_SET(pOldR->room_flags, ROOM_STOP))
    		{
    			REMOVE_BIT(pOldR->room_flags, ROOM_STOP);
    		}
    		pOldR->stops[0] = 0;
    		pOldR->car = 0;
    	}
    }
    pR->stops[0] = value2;
    pRoom->stops[value2] = pR->vnum;
    pR->car = pRoom->vnum;
    if(!IS_SET(pR->room_flags, ROOM_STOP)
    		&& !IS_SET(pRoom->room_flags, ROOM_VEHICLE))
    	SET_BIT(pR->room_flags, ROOM_STOP);
    send_to_char(Format("%d configured as stop %d for room %d\n\r", pRoom->stops[value2], value2, pRoom->vnum), ch);
    return TRUE;
}



REDIT( redit_name )
{
	ROOM_INDEX_DATA *pRoom;

	EDIT_ROOM(ch, pRoom);

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Syntax:  name [name]\n\r", ch );
		return FALSE;
	}

	PURGE_DATA( pRoom->name );
	pRoom->name = str_dup( argument );

	send_to_char( "Name set.\n\r", ch );
	return TRUE;
}


REDIT( redit_desc )
{
	ROOM_INDEX_DATA *pRoom;

	EDIT_ROOM(ch, pRoom);

	if ( IS_NULLSTR(argument) )
	{
		string_append( ch, &pRoom->description );
		return TRUE;
	}

	send_to_char( "Syntax:  desc\n\r", ch );
	return FALSE;
}


REDIT( redit_uname )
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch, pRoom);

    if ( IS_NULLSTR(argument) )
    {
	send_to_char( "Syntax:  uname [name]\n\r", ch );
	return FALSE;
    }

    PURGE_DATA( pRoom->uname );
    pRoom->uname = str_dup( argument );

    send_to_char( "Name set.\n\r", ch );
    return TRUE;
}


REDIT( redit_udesc )
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch, pRoom);

    if ( IS_NULLSTR(argument) )
    {
	string_append( ch, &pRoom->udescription );
	return TRUE;
    }

    send_to_char( "Syntax:  udesc\n\r", ch );
    return FALSE;
}


REDIT( redit_heal )
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch, pRoom);

    if (is_number(argument))
       {
          pRoom->heal_rate = atoi ( argument );
          send_to_char ( "Heal rate set.\n\r", ch);
          return TRUE;
       }

    send_to_char ( "Syntax : heal <#xnumber>\n\r", ch);
    return FALSE;
}

REDIT( redit_mana )
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch, pRoom);

    if (is_number(argument))
       {
          pRoom->mana_rate = atoi ( argument );
          send_to_char ( "Mana rate set.\n\r", ch);
          return TRUE;
       }

    send_to_char ( "Syntax : mana <#xnumber>\n\r", ch);
    return FALSE;
}

REDIT( redit_owner )
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch, pRoom);

    pRoom->owner = str_dup(argument);

    send_to_char ( "Owner set.\n\r", ch);
    return TRUE;
}

REDIT( redit_clan )
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch, pRoom);

    pRoom->clan = clan_lookup(argument);

    send_to_char ( "Clan set.\n\r", ch);
    return TRUE;
}

REDIT( redit_sector )
{
    ROOM_INDEX_DATA *pRoom;
    int i;

    EDIT_ROOM(ch, pRoom);

    if((i = flag_lookup(argument, sector_flags)) < 0)
    {
	send_to_char("No such sector type.\n\r", ch);
	return FALSE;
    }

    pRoom->sector_type = i;

    send_to_char ( "Room sector type set.\n\r", ch);
    return TRUE;
}

REDIT( redit_format )
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch, pRoom);

    pRoom->description = format_string( pRoom->description );
    if(pRoom->udescription != NULL)
    pRoom->udescription = format_string( pRoom->udescription );
    if(pRoom->ddescription != NULL)
    pRoom->ddescription = format_string( pRoom->ddescription );

    send_to_char( "String formatted.\n\r", ch );
    return TRUE;
}



REDIT( redit_mreset )
{
	ROOM_INDEX_DATA	*pRoom;
	MOB_INDEX_DATA	*pMobIndex;
	CHAR_DATA		*newmob;
	char		arg [ MAX_INPUT_LENGTH ];
	char		arg2 [ MAX_INPUT_LENGTH ];

	RESET_DATA		*pReset;

	EDIT_ROOM(ch, pRoom);

	argument = one_argument( argument, arg );
	argument = one_argument( argument, arg2 );

	if ( IS_NULLSTR(arg) || !is_number( arg ) )
	{
		send_to_char ( "Syntax:  mreset <vnum> <max #x> <mix #x>\n\r", ch );
		return FALSE;
	}

	if ( !( pMobIndex = get_mob_index( atoi( arg ) ) ) )
	{
		send_to_char( "REdit: No mobile has that vnum.\n\r", ch );
		return FALSE;
	}

	if ( pMobIndex->area != pRoom->area )
	{
		send_to_char( "REdit: No such mobile in this area.\n\r", ch );
		return FALSE;
	}

	/*
	 * Create the mobile reset.
	 */
	pReset              = new_reset_data();
	pReset->command	= 'M';
	pReset->arg1	= pMobIndex->vnum;
	pReset->arg2	= is_number( arg2 ) ? atoi( arg2 ) : MAX_MOB;
	pReset->arg3	= pRoom->vnum;
	pReset->arg4	= is_number( argument ) ? atoi (argument) : 1;
	add_reset( pRoom, pReset, 0/* Last slot*/ );

	/*
	 * Create the mobile.
	 */
	newmob = create_mobile( pMobIndex );
	char_to_room( newmob, pRoom );

	send_to_char( Format("%s (%d) has been loaded and added to resets.\n\rThere will be a maximum of %d loaded to this room.\n\r",
			capitalize( pMobIndex->short_descr ), pMobIndex->vnum, pReset->arg2), ch );
	act( "$n has created $N!", ch, NULL, newmob, TO_ROOM, 0 );
	return TRUE;
}



struct wear_type
{
    int	wear_loc;
    int	wear_bit;
};



const struct wear_type wear_table[] =
{
    {	WEAR_NONE,	ITEM_TAKE		},
    {	WEAR_FINGER_L,	ITEM_WEAR_FINGER	},
    {	WEAR_FINGER_R,	ITEM_WEAR_FINGER	},
    {	WEAR_NECK,	ITEM_WEAR_NECK		},
    {	WEAR_BODY,	ITEM_WEAR_BODY		},
    {	WEAR_HEAD,	ITEM_WEAR_HEAD		},
    {	WEAR_LEGS,	ITEM_WEAR_LEGS		},
    {	WEAR_FEET,	ITEM_WEAR_FEET		},
    {	WEAR_HANDS,	ITEM_WEAR_HANDS		},
    {	WEAR_ARMS,	ITEM_WEAR_ARMS		},
    {	WEAR_UNDERTOP,	ITEM_WEAR_UNDERTOP	},
    {	WEAR_UNDERPANTS, ITEM_WEAR_UNDERPANTS	},
    {	WEAR_ABOUT,	ITEM_WEAR_ABOUT		},
    {	WEAR_WAIST,	ITEM_WEAR_WAIST		},
    {	WEAR_WRIST_L,	ITEM_WEAR_WRIST		},
    {	WEAR_WRIST_R,	ITEM_WEAR_WRIST		},
    {	WEAR_WIELD,	ITEM_WIELD		},
    {	WEAR_HOLD,	ITEM_HOLD		},
    {	WEAR_FACE,	ITEM_WEAR_FACE		},
    {	NO_FLAG,	NO_FLAG			}
};



/*****************************************************************************
 Name:		wear_loc
 Purpose:	Returns the location of the bit that matches the count.
 		1 = first match, 2 = second match etc.
 Called by:	oedit_reset(olc_act.c).
 ****************************************************************************/
int wear_loc(int bits, int count)
{
    int flag;

    for (flag = 0; wear_table[flag].wear_bit != NO_FLAG; flag++)
    {
        if ( IS_SET(bits, wear_table[flag].wear_bit) && --count < 1)
            return wear_table[flag].wear_loc;
    }

    return NO_FLAG;
}



/*****************************************************************************
 Name:		wear_bit
 Purpose:	Converts a wear_loc into a bit.
 Called by:	redit_oreset(olc_act.c).
 ****************************************************************************/
int wear_bit(int loc)
{
    int flag;

    for (flag = 0; wear_table[flag].wear_loc != NO_FLAG; flag++)
    {
        if ( loc == wear_table[flag].wear_loc )
            return wear_table[flag].wear_bit;
    }

    return 0;
}



REDIT( redit_oreset )
{
	ROOM_INDEX_DATA	*pRoom;
	OBJ_INDEX_DATA	*pObjIndex;
	OBJ_DATA		*newobj;
	OBJ_DATA		*to_obj;
	CHAR_DATA		*to_mob;
	char		arg1 [ MAX_INPUT_LENGTH ];
	char		arg2 [ MAX_INPUT_LENGTH ];
	int			olevel = 0;
	RESET_DATA		*pReset;

	EDIT_ROOM(ch, pRoom);

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( IS_NULLSTR(arg1) || !is_number( arg1 ) )
	{
		send_to_char ( "Syntax:  oreset <vnum> <args>\n\r", ch );
		send_to_char ( "        -no_args               = into room\n\r", ch );
		send_to_char ( "        -<obj_name>            = into obj\n\r", ch );
		send_to_char ( "        -<mob_name> <wear_loc> = into mob\n\r", ch );
		return FALSE;
	}

	if ( !( pObjIndex = get_obj_index( atoi( arg1 ) ) ) )
	{
		send_to_char( "REdit: No object has that vnum.\n\r", ch );
		return FALSE;
	}

	if ( pObjIndex->area != pRoom->area )
	{
		send_to_char( "REdit: No such object in this area.\n\r", ch );
		return FALSE;
	}

	/*
	 * Load into room.
	 */
	if ( IS_NULLSTR(arg2) )
	{
		pReset		= new_reset_data();
		pReset->command	= 'O';
		pReset->arg1	= pObjIndex->vnum;
		pReset->arg2	= 0;
		pReset->arg3	= pRoom->vnum;
		pReset->arg4	= 0;
		add_reset( pRoom, pReset, 0/* Last slot*/ );

		newobj = create_object( pObjIndex );
		obj_to_room( newobj, pRoom );

		send_to_char( Format("%s (%d) has been loaded and added to resets.\n\r", capitalize( pObjIndex->short_descr ), pObjIndex->vnum), ch );
	}
	else
		/*
		 * Load into object's inventory.
		 */
		if ( IS_NULLSTR(argument) && ( ( to_obj = get_obj_list( ch, arg2, pRoom->contents ) ) != NULL ) )
		{
			pReset		= new_reset_data();
			pReset->command	= 'P';
			pReset->arg1	= pObjIndex->vnum;
			pReset->arg2	= 0;
			pReset->arg3	= to_obj->pIndexData->vnum;
			pReset->arg4	= 1;
			add_reset( pRoom, pReset, 0/* Last slot*/ );

			newobj = create_object( pObjIndex );
			newobj->cost = 0;
			obj_to_obj( newobj, to_obj );

			send_to_char( Format("%s (%d) has been loaded into %s (%d) and added to resets.\n\r",
					capitalize( newobj->short_descr ), newobj->pIndexData->vnum, to_obj->short_descr, to_obj->pIndexData->vnum), ch );
		}
		else
			/*
			 * Load into mobile's inventory.
			 */
			if ( ( to_mob = get_char_room( ch, arg2 ) ) != NULL )
			{
				int	wear_loc;

				/*
				 * Make sure the location on mobile is valid.
				 */
				if ( (wear_loc = flag_value( wear_loc_flags, argument )) == NO_FLAG )
				{
					send_to_char( "REdit: Invalid wear_loc.  '? wear-loc'\n\r", ch );
					return FALSE;
				}

				/*
				 * Disallow loading a sword(WEAR_WIELD) into WEAR_HEAD.
				 */
				if ( !IS_SET( pObjIndex->wear_flags, wear_bit(wear_loc) ) )
				{
					send_to_char( Format("%s (%d) has wear flags: [%s]\n\r", capitalize( pObjIndex->short_descr ), pObjIndex->vnum, flag_string( wear_flags, pObjIndex->wear_flags )), ch );
					return FALSE;
				}

				/*
				 * Can't load into same position.
				 */
				if ( get_eq_char( to_mob, wear_loc ) )
				{
					send_to_char( "REdit:  Object already equipped.\n\r", ch );
					return FALSE;
				}

				pReset		= new_reset_data();
				pReset->arg1	= pObjIndex->vnum;
				pReset->arg2	= wear_loc;
				if ( pReset->arg2 == WEAR_NONE )
					pReset->command = 'G';
				else
					pReset->command = 'E';
				pReset->arg3	= wear_loc;

				add_reset( pRoom, pReset, 0/* Last slot*/ );

				newobj = create_object( pObjIndex );

				if ( to_mob->pIndexData->pShop )	/* Shop-keeper? */
				{
					switch ( pObjIndex->item_type )
					{
					default:		olevel = 0;				break;
					case ITEM_ARMOR:	olevel = number_range(  5, 15 );	break;
					case ITEM_WEAPON:	if ( pReset->command == 'G' )
						olevel = number_range( 5, 15 );
					else
						olevel = number_fuzzy( olevel );
					break;
					}

					newobj = create_object( pObjIndex );
					if ( pReset->arg2 == WEAR_NONE )
						SET_BIT( newobj->extra_flags, ITEM_INVENTORY );
				}
				else
					newobj = create_object( pObjIndex );

				obj_to_char( newobj, to_mob );
				if ( pReset->command == 'E' )
					equip_char( to_mob, newobj, pReset->arg3 );

				send_to_char( Format("%s (%d) has been loaded %s of %s (%d) and added to resets.\n\r",
						capitalize( pObjIndex->short_descr ), pObjIndex->vnum, flag_string( wear_loc_strings, pReset->arg3 ),
						to_mob->short_descr, to_mob->pIndexData->vnum), ch );
			}
			else	/* Display Syntax */
			{
				send_to_char( "REdit:  That mobile isn't here.\n\r", ch );
				return FALSE;
			}

	act( "$n has created $p!", ch, newobj, NULL, TO_ROOM, 0 );
	return TRUE;
}



/*
 * Object Editor Functions.
 */
void show_obj_values( CHAR_DATA *ch, OBJ_INDEX_DATA *obj )
{

    switch( obj->item_type )
    {
	default:	/* No values. */
	    break;

	case ITEM_LIGHT:
		if ( obj->value[2] == -1 || obj->value[2] == 999 ) /* ROM OLC */
		{
			send_to_char("[v2] Light:  Infinite[-1]\n\r", ch);
		}
		else
		{
			send_to_char(Format("[v2] Light:  [%d]\n\r", obj->value[2]), ch);
		}
		break;

	case ITEM_PORTAL:
	    send_to_char( Format("[v0] Charges:        [%d]\n\r[v1]", obj->value[0]), ch);
	    send_to_char( Format("Exit Flags:     %s\n\r", flag_string( exit_flags, obj->value[1])), ch);
	    send_to_char( Format("[v2] Portal Flags:   %s\n\r", flag_string( portal_flags, obj->value[2]) ), ch);
	    send_to_char( Format("[v3] Goes to (vnum): [%d]\n\r", obj->value[3]), ch);
	    break;

	case ITEM_FURNITURE:
	    send_to_char( Format("[v0] Max people:      [%d]\n\r", obj->value[0]), ch);
	    send_to_char( Format("[v1] Max weight:      [%d]\n\r", obj->value[1]), ch);
	    send_to_char( Format("[v2] Furniture Flags: %s\n\r", flag_string( furniture_flags, obj->value[2])), ch);
	    send_to_char( Format("[v3] Heal bonus:      [%d]\n\r", obj->value[3]), ch);
	    break;

	case ITEM_SCROLL:
	case ITEM_POTION:
	case ITEM_PILL:
 	    send_to_char( Format("[v0] Level:  [%d]\n\r", obj->value[0]), ch);
	    send_to_char( Format("[v1] Spell:  %s\n\r", obj->value[1] != -1 ? skill_table[obj->value[1]].name : "none"), ch);
	    send_to_char( Format("[v2] Spell:  %s\n\r", obj->value[2] != -1 ? skill_table[obj->value[2]].name : "none"), ch);
	    send_to_char( Format("[v3] Spell:  %s\n\r", obj->value[3] != -1 ? skill_table[obj->value[3]].name : "none"), ch);
	    send_to_char( Format("[v4] Spell:  %s\n\r", obj->value[4] != -1 ? skill_table[obj->value[4]].name : "none"), ch);
	    break;

/* ARMOR for PT */

        case ITEM_ARMOR:
	    send_to_char( Format("[v0] Ac Normal       [%d]\n\r", obj->value[0]), ch);
	    send_to_char( Format("[v1] Ac Aggro        [%d]\n\r", obj->value[1]), ch);
	    send_to_char( Format("[v2] DO NOT USE      [%d]\n\r", obj->value[2]), ch);
	    send_to_char( Format("[v3] DO NOT USE      [%d]\n\r", obj->value[3]), ch);
	    break;

/* WEAPON changed in ROM: */
/* I had to split the output here, I have no idea why, but it helped -- Hugin */
/* It somehow fixed a bug in showing scroll/pill/potions too ?! */
        case ITEM_WEAPON:
        	send_to_char( Format("[v0] Weapon class:   %s\n\r", flag_string( weapon_class, obj->value[0] )), ch );
        	if(obj->value[0] != WEAPON_FIREARM)
        	{
        		send_to_char( Format("[v1] Difficulty to use [4-10]: [%d]\n\r", obj->value[1]), ch );
        		send_to_char( Format("[v2] Damage bonus [1-3]:   [%d]\n\r", obj->value[2]), ch );
        		send_to_char( Format("[v3] Type:           %s\n\r", flag_string( weapon_flags, obj->value[3] )), ch );
        		send_to_char( Format("[v4] Special type:   %s\n\r", flag_string( weapon_type2,  obj->value[4] )), ch );
        	}
        	else
        	{
        		send_to_char( Format("[v1] Difficulty to use [1-10]: [%d]\n\r", obj->value[1]), ch );
        		send_to_char( Format("[v2] Ammo Vnum:                [%d]\n\r", obj->value[2]), ch );
        		send_to_char( Format("[v3] Starting Ammo:            [%d]\n\r", obj->value[3]), ch );
        		send_to_char( Format("[v4] Max Ammo:                 [%d]\n\r", obj->value[4]), ch );
        		send_to_char( Format("[v5] Ammo Ref:   %d\n\r", obj->value[5]), ch );
        	}
        	break;

        case ITEM_AMMO:
        	send_to_char( Format("[v0] Number of rounds:   %d\n\r", obj->value[0]), ch );
        	send_to_char( Format("[v1] Damage:            [%d]\n\r", obj->value[1]), ch );
        	send_to_char( Format("[v2] Range:             [%d]\n\r", obj->value[2]), ch );
        	send_to_char( Format("[v3] Type:           %s\n\r", flag_string( weapon_flags, obj->value[3] )), ch );
        	send_to_char( Format("[v4] Special type:   %s\n\r", flag_string( weapon_type2,  obj->value[4] )), ch );
        	send_to_char( Format("[v5] Ammo Ref:   %d\n\r", obj->value[5]), ch );
        	break;

        case ITEM_BOMB:
        	send_to_char( Format("[v0] Fuse burn time:    %d\n\r", obj->value[0]), ch );
        	send_to_char( Format("[v1] Unused:            [%d]\n\r", obj->value[1]), ch );
        	send_to_char( Format("[v2] Unused:            [%d]\n\r", obj->value[2]), ch );
        	send_to_char( Format("[v3] Unused:            [%d]\n\r", obj->value[3]), ch );
        	send_to_char( Format("[v4] Unused:            [%d]\n\r", obj->value[4]), ch );
        	break;

        case ITEM_CONTAINER:
        	send_to_char( Format("[v0] Weight:     [%d kg]\n\r", obj->value[0]), ch);
        	send_to_char( Format("[v1] Flags:      [%s]\n\r", flag_string( container_flags, obj->value[1] )), ch);
        	send_to_char( Format("[v2] Key:     %s [%d]\n\r", get_obj_index(obj->value[2]) ? get_obj_index(obj->value[2])->short_descr : "none", obj->value[2]), ch);
        	send_to_char( Format("[v3] Capacity    [%d]\n\r", obj->value[3]), ch);
        	send_to_char( Format("[v4] Weight Mult [%d]\n\r", obj->value[4]), ch);
        	break;

	case ITEM_DRINK_CON:
	    send_to_char( Format("[v0] Liquid Total: [%d]\n\r", obj->value[0]), ch);
	    send_to_char( Format("[v1] Liquid Left:  [%d]\n\r", obj->value[1]), ch);
	    send_to_char( Format("[v2] Liquid:       %s\n\r", liq_table[obj->value[2]].liq_name), ch);
	    send_to_char( Format("[v3] Poisoned:     %s\n\r", obj->value[3] != 0 ? "Yes" : "No"), ch);
	    break;

	case ITEM_LIQUID:
		send_to_char( Format("[v0] Liquid strength: [%d]\n\r", obj->value[0]), ch);
		send_to_char( Format("[v1] Flags:           [%s]\n\r", flag_string( liquid_special_flags, obj->value[1] )), ch);
		break;

	case ITEM_FOUNTAIN:
	    send_to_char( Format("[v0] Liquid Total: [%d]\n\r", obj->value[0]), ch);
	    send_to_char( Format("[v1] Liquid Left:  [%d]\n\r", obj->value[1]), ch);
	    send_to_char( Format("[v2] Liquid:	    %s\n\r", liq_table[obj->value[2]].liq_name), ch);
	    break;

	case ITEM_FOOD:
	    send_to_char( Format("[v0] Food hours: [%d]\n\r", obj->value[0]), ch);
	    send_to_char( Format("[v1] Full hours: [%d]\n\r", obj->value[1]), ch);
	    send_to_char( Format("[v3] Poisoned:   %s\n\r", obj->value[3] != 0 ? "Yes" : "No"), ch);
	    break;

	case ITEM_MONEY:
		send_to_char( Format("[v0] Gold:   [%d]\n\r", obj->value[0]), ch );
		break;

	case ITEM_KEY:
		send_to_char( Format("[v0] Reset Room: [%d]\n\r[v1] Reset Mob:  [%d]\n\r", obj->value[0], obj->value[1]), ch );
		break;
    }

    return;
}



bool set_obj_values( CHAR_DATA *ch, OBJ_INDEX_DATA *pObj, int value_num, char *argument)
{
	int i;

	switch( pObj->item_type )
	{
	default:
		send_to_char("You can't make me do it!\n\r", ch);
		send_to_char("No settable values.\n\r", ch);
		break;

	case ITEM_LIGHT:
		switch ( value_num )
		{
		default:
			do_help( ch, "ITEM_LIGHT" );
			return FALSE;
		case 2:
			send_to_char( "HOURS OF LIGHT SET.\n\r\n\r", ch );
			pObj->value[2] = atoi( argument );
			break;
		}
		break;

		case ITEM_SCROLL:
		case ITEM_POTION:
		case ITEM_PILL:
			switch ( value_num )
			{
			default:
				do_help( ch, "ITEM_SCROLL_POTION_PILL" );
				return FALSE;
			case 0:
				send_to_char( "SPELL LEVEL SET.\n\r\n\r", ch );
				pObj->value[0] = atoi( argument );
				break;
			case 1:
				send_to_char( "SPELL TYPE 1 SET.\n\r\n\r", ch );
				pObj->value[1] = skill_lookup( argument );
				break;
			case 2:
				send_to_char( "SPELL TYPE 2 SET.\n\r\n\r", ch );
				pObj->value[2] = skill_lookup( argument );
				break;
			case 3:
				send_to_char( "SPELL TYPE 3 SET.\n\r\n\r", ch );
				pObj->value[3] = skill_lookup( argument );
				break;
			case 4:
				send_to_char( "SPELL TYPE 4 SET.\n\r\n\r", ch );
				pObj->value[4] = skill_lookup( argument );
				break;
			}
			break;

/* ARMOR for PT: */

        case ITEM_ARMOR:
	    switch ( value_num )
	    {
	        default:
		    do_help( ch, "ITEM_ARMOR" );
		    return FALSE;
	        case 0:
		    if((i = atoi(argument)) < 3 && i > -2)
		    {
			send_to_char( "SOAK VS. NORMAL DAM SET.\n\r\n\r", ch );
			pObj->value[0] = atoi( argument );
		    }
		    else
			send_to_char("Value must be between -2 and 3.\n\r", ch);
		    break;
	        case 1:
		    if((i = atoi(argument)) < 3 && i > -2)
		    {
			send_to_char("SOAK VS. AGGRAVATED DAM SET.\n\r\n\r",ch);
			pObj->value[1] = atoi( argument );
		    }
		    else
			send_to_char("Value must be between -2 and 3.\n\r", ch);
		    break;
/* Removed until I can find a real use for them.
	        case 2:
		    send_to_char( "AC SLASH SET.\n\r\n\r", ch );
		    pObj->value[2] = atoi( argument );
		    break;
	        case 3:
		    send_to_char( "AC EXOTIC SET.\n\r\n\r", ch );
		    pObj->value[3] = atoi( argument );
		    break;
*/
	    }
	    break;

/* WEAPONS changed in PT */

        case ITEM_WEAPON:
	    switch ( value_num )
	    {
	        default:
		    do_help( ch, "ITEM_WEAPON" );
	            return FALSE;
	        case 0:
		    send_to_char( "WEAPON CLASS SET.\n\r\n\r", ch );
		    pObj->value[0] = flag_value( weapon_class, argument );
		    break;
	        case 1:
		    if((i = atoi(argument)) < 4 || i > 10)
		    send_to_char("Weapon difficulty must be from 4-10.\n\r",ch);
		    else {
	            send_to_char( "WEAPON DIFFICULTY SET.\n\r\n\r", ch );
	            pObj->value[1] = i;
		    }
	            break;
	        case 2:
		    if(pObj->value[0] == WEAPON_FIREARM)
		      if(get_obj_index(atoi(argument))) {
		      send_to_char( "AMMO OBJECT SET.\n\r", ch );
		      pObj->value[2] = atoi(argument);
		      } else {
		      send_to_char( "NO SUCH OBJECT.\n\r\n\r", ch );
		      break;
		      }
		    else
	          send_to_char("Weapon dam dice now automatically set.\n\r",ch);
	            break;
	        case 3:
		    if(pObj->value[0] == WEAPON_FIREARM) {
	            send_to_char( "STARTING AMMO SET.\n\r\n\r", ch );
	            pObj->value[3] = atoi( argument );
		    } else {
	            send_to_char( "WEAPON TYPE SET.\n\r\n\r", ch );
	            pObj->value[3] = flag_value( weapon_flags, argument );
		    }
	            break;
	        case 4:
		    if(pObj->value[0] == WEAPON_FIREARM) {
	            send_to_char( "MAX AMMO SET.\n\r\n\r", ch );
	            pObj->value[4] = atoi( argument );
		    } else {
                    send_to_char( "SPECIAL WEAPON TYPE SET.\n\r\n\r", ch );
		    pObj->value[4] = flag_value( weapon_type2, argument );
		    }
		    break;
		    case 5:
		    if(pObj->value[0] == WEAPON_FIREARM) {
	            send_to_char( "AMMO REFERENCE SET.\n\r\n\r", ch );
	            pObj->value[5] = atoi( argument );
		    }
		    break;
	    }
            break;

        case ITEM_AMMO:
	    switch ( value_num )
	    {
	        default:
		    do_help( ch, "ITEM_AMMO" );
	            return FALSE;
	        case 0:
		    send_to_char( "NUMBER OF ROUNDS SET.\n\r\n\r", ch );
		    pObj->value[0] = atoi( argument );
		    break;
	        case 1:
	            send_to_char( "AMMO DAMAGE DICE SET.\n\r\n\r", ch );
	            pObj->value[1] = atoi( argument );
	            break;
	        case 2:
	            send_to_char( "AMMO RANGE SET.\n\r\n\r", ch );
	            pObj->value[2] = atoi( argument );
	            break;
	        case 3:
	            send_to_char( "DAMAGE TYPE SET.\n\r\n\r", ch );
	            pObj->value[3] = flag_value( weapon_flags, argument );
	            break;
	        case 4:
                    send_to_char( "SPECIAL WEAPON TYPE SET.\n\r\n\r", ch );
		    pObj->value[4] = flag_value( weapon_type2, argument );
		    break;
		    case 5:
		    if(pObj->value[0] == WEAPON_FIREARM) {
	            send_to_char( "AMMO REFERENCE SET.\n\r\n\r", ch );
	            pObj->value[5] = atoi( argument );
		    }
		    break;
	    }
            break;

        case ITEM_BOMB:
	    switch ( value_num )
	    {
	        default:
		    do_help( ch, "ITEM_BOMB" );
	            return FALSE;
	        case 0:
		    send_to_char( "TIMER SET.\n\r\n\r", ch );
		    pObj->value[0] = atoi( argument );
		    break;
	        case 1:
	            break;
	        case 2:
	            break;
	        case 3:
	            break;
	        case 4:
		    break;
	    }
            break;

	case ITEM_PORTAL:
	    switch ( value_num )
	    {
	        default:
	            do_help(ch, "ITEM_PORTAL" );
	            return FALSE;

	    	case 0:
	    	    send_to_char( "CHARGES SET.\n\r\n\r", ch);
	    	    pObj->value[0] = atoi ( argument );
	    	    break;
	    	case 1:
	    	    send_to_char( "EXIT FLAGS SET.\n\r\n\r", ch);
	    	    pObj->value[1] = flag_value( exit_flags, argument );
	    	    break;
	    	case 2:
	    	    send_to_char( "PORTAL FLAGS SET.\n\r\n\r", ch);
	    	    pObj->value[2] = flag_value( portal_flags, argument );
	    	    break;
	    	case 3:
	    	    send_to_char( "EXIT VNUM SET.\n\r\n\r", ch);
	    	    pObj->value[3] = atoi ( argument );
	    	    break;
	   }
	   break;

	case ITEM_FURNITURE:
	    switch ( value_num )
	    {
	        default:
	            do_help( ch, "ITEM_FURNITURE" );
	            return FALSE;

	        case 0:
	            send_to_char( "NUMBER OF PEOPLE SET.\n\r\n\r", ch);
	            pObj->value[0] = atoi ( argument );
	            break;
	        case 1:
	            send_to_char( "MAX WEIGHT SET.\n\r\n\r", ch);
	            pObj->value[1] = atoi ( argument );
	            break;
	        case 2:
	            send_to_char( "FURNITURE FLAGS SET.\n\r\n\r", ch);
		    if(flag_value( furniture_flags, argument ) == NO_FLAG)
			break;
	            pObj->value[2] = flag_value( furniture_flags, argument );
	            break;
	        case 3:
	            send_to_char( "HEAL BONUS SET.\n\r\n\r", ch);
	            pObj->value[3] = atoi ( argument );
	            break;
	        case 4:
	            send_to_char( "MANA BONUS SET.\n\r\n\r", ch);
	            pObj->value[4] = atoi ( argument );
	            break;
	    }
	    break;

        case ITEM_CONTAINER:
	    switch ( value_num )
	    {
		int value;

		default:
		    do_help( ch, "ITEM_CONTAINER" );
	            return FALSE;
		case 0:
	            send_to_char( "WEIGHT CAPACITY SET.\n\r\n\r", ch );
	            pObj->value[0] = atoi( argument );
	            break;
		case 1:
	            if ( ( value = flag_value( container_flags, argument ) )
	              != NO_FLAG )
	        	TOGGLE_BIT(pObj->value[1], value);
		    else
		    {
			do_help ( ch, "ITEM_CONTAINER" );
			return FALSE;
		    }
	            send_to_char( "CONTAINER TYPE SET.\n\r\n\r", ch );
	            break;
		case 2:
		    if ( atoi(argument) != 0 )
		    {
			if ( !get_obj_index( atoi( argument ) ) )
			{
			    send_to_char( "THERE IS NO SUCH ITEM.\n\r\n\r", ch );
			    return FALSE;
			}

			if ( get_obj_index( atoi( argument ) )->item_type != ITEM_KEY )
			{
			    send_to_char( "THAT ITEM IS NOT A KEY.\n\r\n\r", ch );
			    return FALSE;
			}
		    }
		    send_to_char( "CONTAINER KEY SET.\n\r\n\r", ch );
		    pObj->value[2] = atoi( argument );
		    break;
		case 3:
		    send_to_char( "CONTAINER MAX WEIGHT SET.\n\r", ch);
		    pObj->value[3] = atoi( argument );
		    break;
		case 4:
		    send_to_char( "WEIGHT MULTIPLIER SET.\n\r\n\r", ch );
		    pObj->value[4] = atoi ( argument );
		    break;
	    }
	    break;

	case ITEM_DRINK_CON:
	    switch ( value_num )
	    {
	        default:
		    do_help( ch, "ITEM_DRINK" );
/* OLC		    do_help( ch, "liquids" );    */
	            return FALSE;
	        case 0:
	            send_to_char( "MAXIMUM AMOUT OF LIQUID HOURS SET.\n\r\n\r", ch );
	            pObj->value[0] = atoi( argument );
	            break;
	        case 1:
	            send_to_char( "CURRENT AMOUNT OF LIQUID HOURS SET.\n\r\n\r", ch );
	            pObj->value[1] = atoi( argument );
	            break;
	        case 2:
	            send_to_char( "LIQUID TYPE SET.\n\r\n\r", ch );
	            pObj->value[2] = ( liq_lookup(argument) != -1 ?
	            		       liq_lookup(argument) : 0 );
	            break;
	        case 3:
	            send_to_char( "POISON VALUE TOGGLED.\n\r\n\r", ch );
	            pObj->value[3] = ( pObj->value[3] == 0 ) ? 1 : 0;
	            break;
	    }
            break;

	case ITEM_LIQUID:
	    switch ( value_num )
	    {
		int value;

	        default:
		    do_help( ch, "ITEM_LIQUID" );
/* OLC		    do_help( ch, "liquids" );    */
	            return FALSE;
	        case 0:
	            send_to_char( "LIQUID STRENGTH SET.\n\r\n\r", ch );
	            pObj->value[0] = atoi( argument );
	            break;
	        case 1:
	            if ( ( value = flag_value(liquid_special_flags, argument) )
	              != NO_FLAG )
	        	TOGGLE_BIT(pObj->value[1], value);
		    else
		    {
			do_help ( ch, "ITEM_LIQUID" );
			return FALSE;
		    }
	            send_to_char( "LIQUID SPECIALS SET.\n\r\n\r", ch );
	            break;
	        case 2:
	            break;
	        case 3:
	            break;
	    }
            break;

	case ITEM_FOUNTAIN:
	    switch (value_num)
	    {
	    	default:
		    do_help( ch, "ITEM_FOUNTAIN" );
/* OLC		    do_help( ch, "liquids" );    */
	            return FALSE;
	        case 0:
	            send_to_char( "MAXIMUM AMOUT OF LIQUID HOURS SET.\n\r\n\r", ch );
	            pObj->value[0] = atoi( argument );
	            break;
	        case 1:
	            send_to_char( "CURRENT AMOUNT OF LIQUID HOURS SET.\n\r\n\r", ch );
	            pObj->value[1] = atoi( argument );
	            break;
	        case 2:
	            send_to_char( "LIQUID TYPE SET.\n\r\n\r", ch );
	            pObj->value[2] = ( liq_lookup( argument ) != -1 ?
	            		       liq_lookup( argument ) : 0 );
	            break;
            }
	break;

	case ITEM_FOOD:
	    switch ( value_num )
	    {
	        default:
		    do_help( ch, "ITEM_FOOD" );
	            return FALSE;
	        case 0:
	            send_to_char( "HOURS OF FOOD SET.\n\r\n\r", ch );
	            pObj->value[0] = atoi( argument );
	            break;
	        case 1:
	            send_to_char( "HOURS OF FULL SET.\n\r\n\r", ch );
	            pObj->value[1] = atoi( argument );
	            break;
	        case 3:
	            send_to_char( "POISON VALUE TOGGLED.\n\r\n\r", ch );
	            pObj->value[3] = ( pObj->value[3] == 0 ) ? 1 : 0;
	            break;
	    }
            break;

	case ITEM_MONEY:
	    switch ( value_num )
	    {
	        default:
		    do_help( ch, "ITEM_MONEY" );
	            return FALSE;
	        case 0:
	            send_to_char( "GOLD AMOUNT SET.\n\r\n\r", ch );
	            pObj->value[0] = atoi( argument );
	            break;
		case 1:
		    send_to_char( "SILVER AMOUNT SET.\n\r\n\r", ch );
		    pObj->value[1] = atoi( argument );
		    break;
	    }
            break;
    }

    show_obj_values( ch, pObj );

    return TRUE;
}



OEDIT( oedit_show )
{
    OBJ_INDEX_DATA *pObj;
    AFFECT_DATA *paf;
    int cnt;
    int material;

    EDIT_OBJ(ch, pObj);

    send_to_char("\n\r\tW---------------------------------<\tG OBJ OLC \tW>----------------------------------\tn\n\r", ch);
    send_to_char( Format("Name:         [%s]\n\r", pObj->name), ch );
    send_to_char( Format("Area:         [%5d] %s\n\r", !pObj->area ? -1        : pObj->area->vnum, !pObj->area ? "No Area" : pObj->area->name), ch );
    send_to_char( Format("Vnum:         [%5d]\n\r", pObj->vnum), ch );
    send_to_char( Format("Type:         [%s]\n\r", flag_string( type_flags, pObj->item_type )), ch );
    send_to_char( Format("Wear flags:   [%s]\n\r", flag_string( wear_flags, pObj->wear_flags )), ch );
    send_to_char( Format("Extra flags:  [%s]\n\r", flag_string( extra_flags, pObj->extra_flags )), ch );
    send_to_char( Format("Extra2 flags: [%s]\n\r", flag_string( extra2_flags, pObj->extra2 )), ch );

    send_to_char("\tW--------------------------------<\tGDescription\tW>---------------------------------\tn\n\r", ch);
    send_to_char( Format("Short desc:  [%s]\n\r", pObj->short_descr), ch );
    send_to_char( Format("Long desc:\n\r  [%s]\n\r", pObj->description), ch );
	send_to_char( Format("Description:\n\r\tW%s\tn\n\r", pObj->full_desc), ch );

    if ( pObj->extra_descr )
    {
    	EXTRA_DESCR_DATA *ed;

    	send_to_char( "Ex desc kwd: ", ch );

    	for ( ed = pObj->extra_descr; ed; ed = ed->next )
    	{
    		send_to_char( "[", ch );
    		send_to_char( ed->keyword, ch );
    		send_to_char( "]", ch );
    	}

    	send_to_char( "\n\r", ch );
    }

    send_to_char("\tW---------------------------------<\tGMaterials\tW>----------------------------------\tn\n\r", ch);
    send_to_char( Format("Material:    [%s]\n\r", pObj->material), ch );

    material = material_lookup(pObj->material);
    if(material >= 0)
    {
    send_to_char( Format("(Full: %d Hunger: %d Thirst: %d Drunk: %d High: %d Trip: %d)\n\r",
	pObj->weight * material_table[material].full,
	pObj->weight * material_table[material].hunger,
	pObj->weight * material_table[material].thirst,
	pObj->weight * material_table[material].drunk,
	pObj->weight * material_table[material].high,
	pObj->weight * material_table[material].trip), ch );

    send_to_char( Format("(Liquid: %s Edible: %s Explosive: %s Flammable: %s Metal: %s Poison: %s)\n\r",
	material_table[material].is_liquid ? "Yes" : "No",
	material_table[material].is_edible ? "Yes" : "No",
	material_table[material].is_explosive ? "Yes" : "No",
	material_table[material].is_flammable ? "Yes" : "No",
	material_table[material].is_metal ? "Yes" : "No",
	material_table[material].is_poison ? "Yes" : "No"), ch );

    send_to_char(Format("(Value of material is above %d)\n\r", pObj->weight * material_table[material].value * 100), ch);
    }

    send_to_char( Format("Condition:   [%5d]\n\r", pObj->condition), ch );

    send_to_char( Format("Weight Mult: [%5d]    (Actual Weight: %5d)\n\r", pObj->weight, pObj->weight * material_table[material].weight), ch);
    send_to_char( Format("Cost:        [%5d]\n\r", pObj->cost), ch);

    send_to_char( Format("Fetish flags: [%s]\n\r", flag_string( spirit_table, pObj->fetish_flags )), ch );
    send_to_char( Format("Fetish level: [%d]\n\r", pObj->fetish_level), ch );
    send_to_char( Format("Quality: [%d] %s quality.\n\r", pObj->quality, quality_flags[pObj->quality].name), ch );

    for ( cnt = 0, paf = pObj->affected; paf; paf = paf->next )
    {
    	if ( cnt == 0 )
    	{
    		send_to_char( "Number Modifier Affects\n\r", ch );
    		send_to_char( "------ -------- -------\n\r", ch );
    	}
    	send_to_char( Format("[%4d] %-8d %s\n\r", cnt, paf->modifier, flag_string( apply_flags, paf->location )), ch );
    	cnt++;
    }

    send_to_char("\tW---------------------------------<\tG  Other  \tW>----------------------------------\tn\n\r", ch);

    if(pObj->to_user_none != NULL && pObj->to_user_none[0] != '\0')
    {
    	send_to_char(Format("Use - ToUserNone: %s\n\r", pObj->to_user_none), ch);
    }
    if(pObj->to_room_none != NULL && pObj->to_room_none[0] != '\0')
    {
    	send_to_char(Format("Use - ToRoomNone: %s\n\r", pObj->to_room_none), ch);
    }
    if(pObj->to_user_self != NULL && pObj->to_user_self[0] != '\0')
    {
    	send_to_char(Format("Use - ToUserSelf: %s\n\r", pObj->to_user_self), ch);
    }
    if(pObj->to_room_self != NULL && pObj->to_room_self[0] != '\0')
    {
    	send_to_char(Format("Use - ToRoomSelf: %s\n\r", pObj->to_room_self), ch);
    }
    if(pObj->to_user_other != NULL && pObj->to_user_other[0] != '\0')
    {
    	send_to_char(Format("Use - ToUserOther: %s\n\r", pObj->to_user_other), ch);
    }
    if(pObj->to_room_other != NULL && pObj->to_room_other[0] != '\0')
    {
    	send_to_char(Format("Use - ToRoomOther: %s\n\r", pObj->to_room_other), ch);
    }
    if(pObj->to_vict_other != NULL && pObj->to_vict_other[0] != '\0')
    {
    	send_to_char(Format("Use - ToVictOther: %s\n\r", pObj->to_vict_other), ch);
    }
    if(pObj->to_user_used != NULL && pObj->to_user_used[0] != '\0')
    {
    	send_to_char(Format("Use - ToUserUsed: %s\n\r", pObj->to_user_used), ch);
    }
    if(pObj->to_room_used != NULL && pObj->to_room_used[0] != '\0')
    {
    	send_to_char(Format("Use - ToRoomUsed: %s\n\r", pObj->to_room_used), ch);
    }

    show_obj_values( ch, pObj );

    return FALSE;
}


/*
 * Need to issue warning if flag isn't valid. -- does so now -- Hugin.
 */
OEDIT( oedit_addaffect )
{
    int value;
    OBJ_INDEX_DATA *pObj;
    AFFECT_DATA *pAf;
    char loc[MSL]={'\0'};
    char mod[MSL]={'\0'};

    EDIT_OBJ(ch, pObj);

    argument = one_argument( argument, loc );
    one_argument( argument, mod );

    if ( loc[0] == '\0' || mod[0] == '\0' || !is_number( mod ) )
    {
	send_to_char( "Syntax:  addaffect [location] [#mod]\n\r", ch );
	return FALSE;
    }

    if ( ( value = flag_value( apply_flags, loc ) ) == NO_FLAG ) /* Hugin */
    {
        send_to_char( "Valid affects are:\n\r", ch );
	show_help( ch, "? affect" );
	return FALSE;
    }

    pAf             =   new_affect();
    pAf->location   =   value;
    pAf->modifier   =   atoi( mod );
    pAf->type       =   -1;
    pAf->duration   =   -1;
    pAf->bitvector  =   0;
    pAf->next       =   pObj->affected;
    pObj->affected  =   pAf;

    send_to_char( "Affect added.\n\r", ch);
    return TRUE;
}

/* OEDIT( oedit_addaffect )
{
    int value,value3;
    OBJ_INDEX_DATA *pObj;
    AFFECT_DATA *pAf;
    char loc[MSL]={'\0'};
    char mod[MSL]={'\0'};
    char value2[MSL]={'\0'};

    EDIT_OBJ(ch, pObj);

    argument = one_argument( argument, loc );
    argument = one_argument( argument, mod );
    argument = one_argument( argument, value2 );

    if ( loc[0] == '\0' || mod[0] == '\0' || !is_number( mod ) )
    {
	send_to_char( "Syntax:  addaffect [location] [#xmod]\n\r", ch );
	return FALSE;
    }

    if ( ( value = flag_value( apply_flags, loc ) ) == NO_FLAG )
    {
        send_to_char( "Valid affects are:\n\r", ch );
	show_help( ch, "apply" );
	return FALSE;
    }

    value3 = flag_value( affect_flags, value2 );

    pAf             =   new_affect();
    pAf->location   =   value;
    pAf->modifier   =   atoi( mod );
    pAf->type       =   (value3 == NO_FLAG) ? -1 : skill_lookup(value2);
    pAf->duration   =   -1;
    pAf->bitvector  =   (value3 == NO_FLAG) ? 0 : value3;
    pAf->where	    =	0;
    pAf->next       =   pObj->affected;
    pObj->affected  =   pAf;

    send_to_char( "Affect added.\n\r", ch);
    return TRUE;
} */


OEDIT( oedit_desc )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( IS_NULLSTR(argument) )
    {
	string_append( ch, &pObj->full_desc );
	return TRUE;
    }

    send_to_char( "Syntax:  desc    - line edit\n\r", ch );
    return FALSE;
}



/*
 * My thanks to Hans Hvidsten Birkeland and Noam Krendel(Walker)
 * for really teaching me how to manipulate pointers.
 */
OEDIT( oedit_delaffect )
{
    OBJ_INDEX_DATA *pObj;
    AFFECT_DATA *pAf;
    AFFECT_DATA *pAf_next;
    char affect[MSL]={'\0'};
    int  value;
    int  cnt = 0;

    EDIT_OBJ(ch, pObj);

    one_argument( argument, affect );

    if ( !is_number( affect ) || affect[0] == '\0' )
    {
	send_to_char( "Syntax:  delaffect [#xaffect]\n\r", ch );
	return FALSE;
    }

    value = atoi( affect );

    if ( value < 0 )
    {
	send_to_char( "Only non-negative affect-numbers allowed.\n\r", ch );
	return FALSE;
    }

    if ( !( pAf = pObj->affected ) )
    {
	send_to_char( "OEdit:  Non-existant affect.\n\r", ch );
	return FALSE;
    }

    if( value == 0 )	/* First case: Remove first affect */
    {
	pAf = pObj->affected;
	pObj->affected = pAf->next;
	free_affect( pAf );
    }
    else		/* Affect to remove is not the first */
    {
	while ( ( pAf_next = pAf->next ) && ( ++cnt < value ) )
	     pAf = pAf_next;

	if( pAf_next )		/* See if it's the next affect */
	{
	    pAf->next = pAf_next->next;
	    free_affect( pAf_next );
	}
	else                                 /* Doesn't exist */
	{
	     send_to_char( "No such affect.\n\r", ch );
	     return FALSE;
	}
    }

    send_to_char( "Affect removed.\n\r", ch);
    return TRUE;
}


OEDIT( oedit_name )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( IS_NULLSTR(argument) )
    {
	send_to_char( "Syntax:  name [string]\n\r", ch );
	return FALSE;
    }

    PURGE_DATA( pObj->name );
    pObj->name = str_dup( argument );

    send_to_char( "Name set.\n\r", ch);
    return TRUE;
}




OEDIT( oedit_short )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( IS_NULLSTR(argument) )
    {
	send_to_char( "Syntax:  short [string]\n\r", ch );
	return FALSE;
    }

    PURGE_DATA( pObj->short_descr );
    pObj->short_descr = str_dup( argument );
    pObj->short_descr[0] = LOWER( pObj->short_descr[0] );

    send_to_char( "Short description set.\n\r", ch);
    return TRUE;
}



OEDIT( oedit_long )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( IS_NULLSTR(argument) )
    {
	send_to_char( "Syntax:  long [string]\n\r", ch );
	return FALSE;
    }

    PURGE_DATA( pObj->description );
    pObj->description = str_dup( argument );
    pObj->description[0] = UPPER( pObj->description[0] );

    send_to_char( "Long description set.\n\r", ch);
    return TRUE;
}



bool set_value( CHAR_DATA *ch, OBJ_INDEX_DATA *pObj, char *argument, int value )
{
	if ( IS_NULLSTR(argument) )
	{
		set_obj_values( ch, pObj, -1, "" );  /* '\0' changed to "" -- Hugin */
		return FALSE;
	}

	if ( set_obj_values( ch, pObj, value, argument ) )
		return TRUE;

	return FALSE;
}



/*****************************************************************************
 Name:		oedit_values
 Purpose:	Finds the object and sets its value.
 Called by:	The four valueX functions below. (now five -- Hugin )
 ****************************************************************************/
bool oedit_values( CHAR_DATA *ch, char *argument, int value )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( set_value( ch, pObj, argument, value ) )
        return TRUE;

    return FALSE;
}


OEDIT( oedit_value0 )
{
    if ( oedit_values( ch, argument, 0 ) )
        return TRUE;

    return FALSE;
}



OEDIT( oedit_value1 )
{
    if ( oedit_values( ch, argument, 1 ) )
        return TRUE;

    return FALSE;
}



OEDIT( oedit_value2 )
{
    if ( oedit_values( ch, argument, 2 ) )
        return TRUE;

    return FALSE;
}



OEDIT( oedit_value3 )
{
    if ( oedit_values( ch, argument, 3 ) )
        return TRUE;

    return FALSE;
}



OEDIT( oedit_value4 )
{
    if ( oedit_values( ch, argument, 4 ) )
        return TRUE;

    return FALSE;
}



OEDIT( oedit_weight )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( IS_NULLSTR(argument) || !is_number( argument ) )
    {
	send_to_char( "Syntax:  weight [number]\n\r", ch );
	return FALSE;
    }

    pObj->weight = atoi( argument );

    send_to_char( "Weight set.\n\r", ch);
    return TRUE;
}

OEDIT( oedit_cost )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( IS_NULLSTR(argument) || !is_number( argument ) )
    {
	send_to_char( "Syntax:  cost [number]\n\r", ch );
	return FALSE;
    }

    pObj->cost = atoi( argument );

    send_to_char( "Cost set.\n\r", ch);
    return TRUE;
}



OEDIT( oedit_create )
{
	OBJ_INDEX_DATA *pObj;
	AREA_DATA *pArea;
	int  value;
	int  iHash;

	value = atoi( argument );
	if ( IS_NULLSTR(argument) || value == 0 )
	{
		send_to_char( "Syntax:  oedit create [vnum]\n\r", ch );
		return FALSE;
	}

	pArea = get_vnum_area( value );
	if ( !pArea )
	{
		send_to_char( "OEdit:  That vnum is not assigned an area.\n\r", ch );
		return FALSE;
	}

	if ( !IS_BUILDER( ch, pArea ) )
	{
		send_to_char( "OEdit:  Vnum in an area you cannot build in.\n\r", ch );
		return FALSE;
	}

	if ( get_obj_index( value ) )
	{
		send_to_char( "OEdit:  Object vnum already exists.\n\r", ch );
		return FALSE;
	}

    pObj			= new_obj_index();
    pObj->vnum			= value;
    pObj->area			= pArea;

    if ( value > top_vnum_obj )
	top_vnum_obj = value;

    iHash			= value % MAX_KEY_HASH;
    pObj->next			= obj_index_hash[iHash];
    obj_index_hash[iHash]	= pObj;
    ch->desc->pEdit		= (void *)pObj;

    send_to_char( "Object Created.\n\r", ch );
    return TRUE;
}



OEDIT( oedit_ed )
{
    OBJ_INDEX_DATA *pObj;
    EXTRA_DESCR_DATA *ed;
    char command[MAX_INPUT_LENGTH]={'\0'};
    char keyword[MAX_INPUT_LENGTH]={'\0'};

    EDIT_OBJ(ch, pObj);

    argument = one_argument( argument, command );
    one_argument( argument, keyword );

    if ( IS_NULLSTR(command) )
    {
	send_to_char( "Syntax:  ed add [keyword]\n\r", ch );
	send_to_char( "         ed delete [keyword]\n\r", ch );
	send_to_char( "         ed edit [keyword]\n\r", ch );
	send_to_char( "         ed format [keyword]\n\r", ch );
	return FALSE;
    }

    if ( !str_cmp( command, "add" ) )
    {
	if ( IS_NULLSTR(keyword) )
	{
	    send_to_char( "Syntax:  ed add [keyword]\n\r", ch );
	    return FALSE;
	}

	ed                  =   new_extra_descr();
	PURGE_DATA( ed->keyword );
	ed->keyword         =   str_dup( keyword );
	ed->next            =   pObj->extra_descr;
	pObj->extra_descr   =   ed;

	string_append( ch, &ed->description );

	return TRUE;
    }

    if ( !str_cmp( command, "edit" ) )
    {
	if ( IS_NULLSTR(keyword) )
	{
	    send_to_char( "Syntax:  ed edit [keyword]\n\r", ch );
	    return FALSE;
	}

	for ( ed = pObj->extra_descr; ed; ed = ed->next )
	{
	    if ( is_name( keyword, ed->keyword ) )
		break;
	}

	if ( !ed )
	{
	    send_to_char( "OEdit:  Extra description keyword not found.\n\r", ch );
	    return FALSE;
	}

	string_append( ch, &ed->description );

	return TRUE;
    }

    if ( !str_cmp( command, "delete" ) )
    {
	EXTRA_DESCR_DATA *ped = NULL;

	if ( IS_NULLSTR(keyword) )
	{
	    send_to_char( "Syntax:  ed delete [keyword]\n\r", ch );
	    return FALSE;
	}

	for ( ed = pObj->extra_descr; ed; ed = ed->next )
	{
	    if ( is_name( keyword, ed->keyword ) )
		break;
	    ped = ed;
	}

	if ( !ed )
	{
	    send_to_char( "OEdit:  Extra description keyword not found.\n\r", ch );
	    return FALSE;
	}

	if ( !ped )
	    pObj->extra_descr = ed->next;
	else
	    ped->next = ed->next;

	free_extra_descr( ed );

	send_to_char( "Extra description deleted.\n\r", ch );
	return TRUE;
    }

    if ( !str_cmp( command, "format" ) )
    {

    	if ( IS_NULLSTR(keyword) )
    	{
    		send_to_char( "Syntax:  ed format [keyword]\n\r", ch );
    		return FALSE;
    	}

    	for ( ed = pObj->extra_descr; ed; ed = ed->next )
    	{
			EXTRA_DESCR_DATA *ped = NULL;
    		if ( is_name( keyword, ed->keyword ) )
    			break;
    		ped = ed;
    	}

    	if ( !ed )
    	{
    		send_to_char( "OEdit:  Extra description keyword not found.\n\r", ch );
    		return FALSE;
    	}

    	ed->description = format_string( ed->description );

    	send_to_char( "Extra description formatted.\n\r", ch );
    	return TRUE;
    }

    oedit_ed( ch, "" );
    return FALSE;
}


OEDIT( oedit_fetish )
{
	OBJ_INDEX_DATA *pObj;
	int value;

	if ( !IS_NULLSTR(argument) )
	{
		EDIT_OBJ(ch, pObj);

		if ( ( value = flag_value( spirit_table, argument ) ) != NO_FLAG )
		{
			TOGGLE_BIT(pObj->fetish_flags, value);

			send_to_char( "Bound spirit flag toggled.\n\r", ch);
			return TRUE;
		}
	}

	send_to_char( "Syntax:  fetish [flag]\n\r", ch);
	send_to_char( "Type '? spirit' for a list of flags.\n\r", ch );
	return FALSE;
}


OEDIT( oedit_fetlevel )
{
	OBJ_INDEX_DATA *pObj;

	if ( !IS_NULLSTR(argument) && is_number(argument))
	{
		EDIT_OBJ(ch, pObj);

		pObj->fetish_level = atoi(argument);
		act("Fetish level set to $i.", ch, argument, NULL, TO_CHAR, 1);
		return TRUE;
	}

	send_to_char("Syntax:  fetishlevel [number]\n\r", ch);
	return FALSE;
}


OEDIT( oedit_quality )
{
	OBJ_INDEX_DATA *pObj;
	int quality;

	if ( !IS_NULLSTR(argument) && is_number(argument))
	{
		EDIT_OBJ(ch, pObj);

		quality = atoi(argument);
		if( 1 > quality || quality > 15)
		{
			send_to_char("Quality must be between 1 and 15.\n\r", ch);
			return FALSE;
		}
		pObj->quality = quality;
		act("Quality set to $i ($T).", ch, argument, quality_flags[quality].name, TO_CHAR, 1);
		return TRUE;
	}

	send_to_char("Syntax:  quality [1-15]\n\r", ch);
	return FALSE;
}



/* ROM object functions : Many altered and updated for PT */

OEDIT( oedit_extra )      /* Moved out of oedit() due to naming conflicts -- Hugin */
{
	OBJ_INDEX_DATA *pObj;
	int value;

	if ( !IS_NULLSTR(argument) )
	{
		EDIT_OBJ(ch, pObj);

		if ( ( value = flag_value( extra_flags, argument ) ) != NO_FLAG )
		{
			TOGGLE_BIT(pObj->extra_flags, value);

			send_to_char( "Extra flag toggled.\n\r", ch);
			return TRUE;
		}
	}

	send_to_char( "Syntax:  extra [flag]\n\rType '? extra' for a list of flags.\n\r", ch );
	return FALSE;
}


OEDIT( oedit_extra2 )      /* Moved out of oedit() due to naming conflicts -- Hugin */
{
	OBJ_INDEX_DATA *pObj;
	int value;

	if ( !IS_NULLSTR(argument) )
	{
		EDIT_OBJ(ch, pObj);

		if ( ( value = flag_value( extra2_flags, argument ) ) != NO_FLAG )
		{
			TOGGLE_BIT(pObj->extra2, value);

			send_to_char( "Extra2 flag toggled.\n\r", ch);
			return TRUE;
		}
	}

	send_to_char( "Syntax:  extra2 [flag]\n\rType '? extra2' for a list of flags.\n\r", ch );
	return FALSE;
}


OEDIT( oedit_wear )      /* Moved out of oedit() due to naming conflicts -- Hugin */
{
	OBJ_INDEX_DATA *pObj;
	int value;

	if ( !IS_NULLSTR(argument) )
	{
		EDIT_OBJ(ch, pObj);

		if ( ( value = flag_value( wear_flags, argument ) ) != NO_FLAG )
		{
			TOGGLE_BIT(pObj->wear_flags, value);

			send_to_char( "Wear flag toggled.\n\r", ch);
			return TRUE;
		}
	}

	send_to_char( "Syntax:  wear [flag]\n\r"
		"Type '? wear' for a list of flags.\n\r", ch );
	return FALSE;
}


OEDIT( oedit_type )      /* Moved out of oedit() due to naming conflicts -- Hugin */
{
    OBJ_INDEX_DATA *pObj;
    int value;

    if ( !IS_NULLSTR(argument) )
    {
	EDIT_OBJ(ch, pObj);

	if ( ( value = flag_lookup( argument, type_flags ) ) != NO_FLAG )
	{
	    pObj->item_type = value;

	    send_to_char( "Type set.\n\r", ch);

	    /*
	     * Clear the values.
	     */
	    pObj->value[0] = 0;
	    pObj->value[1] = 0;
	    pObj->value[2] = 0;
	    pObj->value[3] = 0;
	    pObj->value[4] = 0;
	    pObj->value[5] = 0;

	    return TRUE;
	}
    }

    send_to_char( "Syntax:  type [flag]\n\r"
		  "Type '? type' for a list of flags.\n\r", ch );
    return FALSE;
}

OEDIT( oedit_material )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( IS_NULLSTR(argument) )
    {
	send_to_char( "Syntax:  material [string]\n\r", ch );
	return FALSE;
    }

    PURGE_DATA( pObj->material );
    pObj->material = str_dup( argument );

    send_to_char( "Material set.\n\r", ch);
    return TRUE;
}

/*
OEDIT( oedit_material )
{
    OBJ_INDEX_DATA *pObj;
    int value;

    EDIT_OBJ(ch, pObj);

    if ( !IS_NULLSTR(argument) )
    {

	if ( ( value = flag_value( material_type, argument ) ) != NO_FLAG )
	{
	    pObj->material = material_name(value);
	    send_to_char( "Material type set.\n\r", ch);
	    return TRUE;
	}
    }

    send_to_char( "Syntax:  material [material-name]\n\r"
		  "Type '? material' for a list of materials.\n\r", ch );
    return FALSE;
}
*/

OEDIT( oedit_condition )
{
    OBJ_INDEX_DATA *pObj;
    int value;

    if ( !IS_NULLSTR(argument)
    && ( value = atoi (argument ) ) >= 0
    && ( value <= 100 ) )
    {
	EDIT_OBJ( ch, pObj );

	pObj->condition = value;
	send_to_char( "Condition set.\n\r", ch );

	return TRUE;
    }

    send_to_char( "Syntax:  condition [number]\n\r"
		  "Where number can range from 0 (ruined) to 100 (perfect).\n\r",
		  ch );
    return FALSE;
}


OEDIT( oedit_use_strings )
{
    OBJ_INDEX_DATA *pObj;
    char arg1[MAX_INPUT_LENGTH]={'\0'};
    char arg2[MAX_INPUT_LENGTH]={'\0'};
    bool check = FALSE;

    EDIT_OBJ(ch, pObj);

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if ( IS_NULLSTR(arg1)
    || ((IS_NULLSTR(arg2) || IS_NULLSTR(argument)) && !is_number(arg1)) )
    {
	send_to_char(
	"Syntax:  use <seen by> <target> [string]\n\r", ch);
	send_to_char(
	"         use <number of uses>\n\r", ch);
	send_to_char("Seen by: user, room, or victim.\n\r", ch);
	send_to_char("Target: none, self, other, used.\n\r", ch);
	return FALSE;
    }

    if(is_number(arg1)) {
	pObj->uses = atoi(arg1);
	check = TRUE;
    }

    if(!str_prefix(arg1, "user") || !str_prefix(arg1, "self")) {
	if(!str_prefix(arg2, "none")) {
		PURGE_DATA( pObj->to_user_none );
	    pObj->to_user_none = str_dup( argument );
	    send_to_char( "Use string for user without argument set.\n\r", ch);
	    check = TRUE;
	}
	else if(!str_prefix(arg2, "self") || !str_prefix(arg2, "user")) {
		PURGE_DATA( pObj->to_user_self );
	    pObj->to_user_self = str_dup( argument );
	    send_to_char("Use string for user with user as target set.\n\r",ch);
	    check = TRUE;
	}
	else if(!str_prefix(arg2, "other") || !str_prefix(arg2, "victim")) {
		PURGE_DATA( pObj->to_user_other );
	    pObj->to_user_other = str_dup( argument );
	    send_to_char("Use string for user with victim set.\n\r",ch);
	    check = TRUE;
	}
	else if(!str_prefix(arg2, "used")) {
		PURGE_DATA( pObj->to_user_used );
	    pObj->to_user_used = str_dup( argument );
	    send_to_char("Use string for user of used-up object set.\n\r",ch);
	    check = TRUE;
	}
    }
    else if(!str_prefix(arg1, "room")) {
	if(!str_prefix(arg2, "none")) {
		PURGE_DATA( pObj->to_room_none );
	    pObj->to_room_none = str_dup( argument );
	    send_to_char( "Use string for room without argument set.\n\r", ch);
	    check = TRUE;
	}
	else if(!str_prefix(arg2, "self") || !str_prefix(arg2, "user")) {
		PURGE_DATA( pObj->to_room_self );
	    pObj->to_room_self = str_dup( argument );
	    send_to_char("Use string for room with user as target set.\n\r",ch);
	    check = TRUE;
	}
	else if(!str_prefix(arg2, "other") || !str_prefix(arg2, "victim")) {
		PURGE_DATA( pObj->to_room_other );
	    pObj->to_room_other = str_dup( argument );
	    send_to_char("Use string for room with victim set.\n\r",ch);
	    check = TRUE;
	}
	else if(!str_prefix(arg2, "used")) {
		PURGE_DATA( pObj->to_room_used );
	    pObj->to_room_used = str_dup( argument );
	    send_to_char("Use string for room from used-up object set.\n\r",ch);
	    check = TRUE;
	}
    }
    else if(!str_prefix(arg1, "victim") || !str_prefix(arg1, "other")) {
    	PURGE_DATA( pObj->to_vict_other );
	    pObj->to_vict_other = str_dup( (char *)Format("%s %s", arg2, argument) );
	    send_to_char("Use string for victim with victim set.\n\r",ch);
	    check = TRUE;
    }

    if (check == FALSE)
    {
    	send_to_char( "Syntax:  use <seen by> <target> [string]\n\r", ch);
    	send_to_char( "         use <number of uses>\n\r", ch);
    	send_to_char("Seen by: user, room, or victim.\n\r", ch);
    	send_to_char("Target: none, self, other, used.\n\r", ch);
    }
    else
    {
    	if(pObj->uses < -1) pObj->uses = -1;
    }

    return check;
}


OEDIT( oedit_company )
{
	OBJ_INDEX_DATA *pObj;
	char arg1[MAX_INPUT_LENGTH]={'\0'};
	STOCKS *st;
	bool check = FALSE;

	EDIT_OBJ(ch, pObj);

	argument = one_argument(argument, arg1);

	if ( IS_NULLSTR(arg1) )
	{
		send_to_char( "Syntax:  company <tickerID>\n\r", ch);
		return FALSE;
	}

	if((st = stock_lookup(argument)) != NULL) {
		PURGE_DATA( pObj->company );
		pObj->company = str_dup( st->ticker );
		send_to_char( "Company set.\n\r", ch);
		check = TRUE;
	} else {
		send_to_char( "Company does not exist.\n\r", ch);
		check = FALSE;
	}
	if (check == FALSE) {
		send_to_char( "Syntax:  company <tickerID>\n\r", ch);
	}

	return check;
}



/*
 * Mobile Editor Functions.
 */
MEDIT( medit_show )
{
    MOB_INDEX_DATA *pMob;
    SCRIPT_DATA *ps;
    MPROG_LIST *list;

    EDIT_MOB(ch, pMob);

    send_to_char( Format("Name:        [%s]\n\r", pMob->player_name), ch );
    send_to_char( Format("Area:        [%5d] %s\n\r", !pMob->area ? -1        : pMob->area->vnum, !pMob->area ? "No Area" : pMob->area->name), ch);

    send_to_char( Format("Act:         [%s]\n\r", flag_string( act_flags, pMob->act )), ch );

    send_to_char( Format("Act2:         [%s]\n\r", flag_string( act2_flags, pMob->act2 )), ch );

    send_to_char( Format("Vnum:        [%5d]\n\r", pMob->vnum), ch);
    send_to_char( Format("Sex:         [%s]\n\r", pMob->sex == SEX_MALE    ? "male"   : pMob->sex == SEX_FEMALE  ? "female" : 	pMob->sex == 3           ? "random" : "neutral"), ch);

    send_to_char( Format("Race:        [%s]\n\r", race_table[pMob->race].name), ch );
    send_to_char( Format("Level:       [%d]\n\r", pMob->level), ch );
    send_to_char( Format("Dam Type:    [%s]\n\r", attack_table[pMob->dam_type].name), ch );
    send_to_char( Format("Affected by: [%s]\n\r", flag_string( affect_flags, pMob->affected_by )), ch );
    send_to_char( Format("Affected by 2(aff2): [%s]\n\r", flag_string( affect_flags2, pMob->affected_by2 )), ch );
    send_to_char( Format("Form:        [%s]\n\r", flag_string( form_flags, pMob->form )), ch );
    send_to_char( Format("Parts:       [%s]\n\r", flag_string( part_flags, pMob->parts )), ch );
    send_to_char( Format("Imm:         [%s]\n\r", flag_string( imm_flags, pMob->imm_flags )), ch );
    send_to_char( Format("Res:         [%s]\n\r", flag_string( res_flags, pMob->res_flags )), ch );
    send_to_char( Format("Vuln:        [%s]\n\r", flag_string( vuln_flags, pMob->vuln_flags )), ch );
    send_to_char( Format("Off:         [%s]\n\r", flag_string( off_flags,  pMob->off_flags )), ch );
    send_to_char( Format("Size:        [%s]\n\r", flag_string( size_flags, pMob->size )), ch );
    send_to_char( Format("Material:    [%s]\n\r", pMob->material), ch );
    send_to_char( Format("Start pos.   [%s]\n\r", flag_string( position_flags, pMob->start_pos )), ch );
    send_to_char( Format("Default pos  [%s]\n\r", flag_string( position_flags, pMob->default_pos )), ch );
    send_to_char( Format("Avg Dollars:      [%5ld]\n\r", pMob->wealth), ch );
    send_to_char( Format("Short descr: %s\n\r", pMob->short_descr), ch);
    send_to_char( Format("Long descr:\n\r%s\n\r", pMob->long_descr), ch);
    send_to_char( Format("Description:\n\r%s\n\r", pMob->description), ch );

    if ( pMob->pShop )
    {
    	SHOP_DATA *pShop;
    	int iTrade;

    	pShop = pMob->pShop;

    	send_to_char(Format("Shop data for [%5d]:\n\r", pShop->keeper), ch);
    	send_to_char(Format("  Markup for purchaser: %d%%\n\r", pShop->profit_buy), ch);
    	send_to_char(Format("  Markdown for seller:  %d%%\n\r", pShop->profit_sell), ch);

    	send_to_char( Format("  Hours: %d to %d.\n\r", pShop->open_hour, pShop->close_hour), ch );
    	if(pShop->raw_materials)
    	{
    		send_to_char( Format("Raw materials sold: %s.\n\r", flag_string(raw_material_table, pShop->raw_materials)), ch );
    	}

    	for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ )
    	{
    		if ( pShop->buy_type[iTrade] != 0 )
    		{
    			if ( iTrade == 0 ) {
    				send_to_char( "  Number Trades Type\n\r", ch );
    				send_to_char( "  ------ -----------\n\r", ch );
    			}
    			send_to_char( Format("  [%4d] %s\n\r", iTrade, flag_string( type_flags, pShop->buy_type[iTrade] )), ch );
    		}
    	}
    }

    send_to_char("Scripts: [", ch);
    for(ps = pMob->triggers; ps; ps = ps->next_in_event)
    {
    	send_to_char(Format(" %s,", ps->trigger), ch);
    }
    send_to_char(" ]\n\r", ch);

    if ( pMob->mprogs )
    {
        int cnt;

        send_to_char( Format("\n\rMOBPrograms for [%5d]:\n\r", pMob->vnum), ch );

        for (cnt=0, list=pMob->mprogs; list; list=list->next)
        {
                if (cnt ==0)
                {
                        send_to_char ( " Number Vnum Trigger Phrase\n\r", ch );
                        send_to_char ( " ------ ---- ------- ------\n\r", ch );
                }

                send_to_char( Format("[%5d] %4d %7s %s\n\r", cnt, list->vnum,mprog_type_to_name(list->trig_type), list->trig_phrase), ch );
                cnt++;
        }
    }

    return FALSE;
}



MEDIT( medit_create )
{
    MOB_INDEX_DATA *pMob;
    AREA_DATA *pArea;
    int  value;
    int  iHash;

    value = atoi( argument );
    if ( IS_NULLSTR(argument) || value == 0 )
    {
	send_to_char( "Syntax:  medit create [vnum]\n\r", ch );
	return FALSE;
    }

    pArea = get_vnum_area( value );

    if ( !pArea )
    {
	send_to_char( "MEdit:  That vnum is not assigned an area.\n\r", ch );
	return FALSE;
    }

    if ( !IS_BUILDER( ch, pArea ) )
    {
	send_to_char( "MEdit:  Vnum in an area you cannot build in.\n\r", ch );
	return FALSE;
    }

    if ( get_mob_index( value ) )
    {
	send_to_char( "MEdit:  Mobile vnum already exists.\n\r", ch );
	return FALSE;
    }

    pMob			= new_mob_index();
    pMob->vnum			= value;
    pMob->area			= pArea;

    if ( value > top_vnum_mob )
	top_vnum_mob = value;

    pMob->act			= ACT_IS_NPC;
    pMob->act2			= 0;
    iHash			= value % MAX_KEY_HASH;
    pMob->next			= mob_index_hash[iHash];
    mob_index_hash[iHash]	= pMob;
    ch->desc->pEdit		= (void *)pMob;
    pMob->start_pos		= P_STAND;
    pMob->default_pos		= P_STAND;
    pMob->sex			= 0;
    pMob->size			= 2;

    send_to_char( "Mobile Created.\n\r", ch );
    return TRUE;
}



MEDIT( medit_spec )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( IS_NULLSTR(argument) )
    {
	send_to_char( "Syntax:  spec [special function]\n\r", ch );
	return FALSE;
    }


    if ( !str_cmp( argument, "none" ) )
    {
        pMob->spec_fun = NULL;

        send_to_char( "Spec removed.\n\r", ch);
        return TRUE;
    }

    send_to_char( "MEdit: No such special function.\n\r", ch );
    return FALSE;
}

MEDIT( medit_damtype )
{
    MOB_INDEX_DATA *pMob;
    int value = NO_FLAG;

    EDIT_MOB(ch, pMob);

    if ( IS_NULLSTR(argument) )
    {
	send_to_char( "Syntax:  damtype [damage message]\n\r", ch );
	send_to_char( "for a list of weapon types, type '? weapon'.\n\r", ch );
	return FALSE;
    }

    if ( ( value = flag_value( weapon_flags, argument ) ) != NO_FLAG )
    {
	pMob->dam_type = value;
	send_to_char( "Damage type set.\n\r", ch);
	return TRUE;
    }

    send_to_char( "MEdit: No such damage type.\n\r", ch );
    return FALSE;
}


MEDIT( medit_level )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( IS_NULLSTR(argument) || !is_number( argument ) )
    {
	send_to_char( "Syntax:  level [number]\n\r", ch );
	return FALSE;
    }

    pMob->level = atoi( argument );

    send_to_char( "Level set.\n\r", ch);
    return TRUE;
}


MEDIT( medit_desc )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( IS_NULLSTR(argument) )
    {
	string_append( ch, &pMob->description );
	return TRUE;
    }

    send_to_char( "Syntax:  desc    - line edit\n\r", ch );
    return FALSE;
}




MEDIT( medit_long )
{
	MOB_INDEX_DATA *pMob;

	EDIT_MOB(ch, pMob);

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Syntax:  long [string]\n\r", ch );
		return FALSE;
	}

	PURGE_DATA( pMob->long_descr );
	strncat( argument, "\n\r", sizeof(*argument) );
	pMob->long_descr = str_dup( argument );
	pMob->long_descr[0] = UPPER( pMob->long_descr[0]  );

	send_to_char( "Long description set.\n\r", ch);
	return TRUE;
}



MEDIT( medit_short )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( IS_NULLSTR(argument) )
    {
	send_to_char( "Syntax:  short [string]\n\r", ch );
	return FALSE;
    }

    PURGE_DATA( pMob->short_descr );
    pMob->short_descr = str_dup( argument );

    send_to_char( "Short description set.\n\r", ch);
    return TRUE;
}


MEDIT( medit_name )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( IS_NULLSTR(argument) )
    {
	send_to_char( "Syntax:  name [string]\n\r", ch );
	return FALSE;
    }

    PURGE_DATA( pMob->player_name );
    pMob->player_name = str_dup( argument );

    send_to_char( "Name set.\n\r", ch);
    return TRUE;
}
/* ROM medit functions: */


MEDIT( medit_shop )
{
    MOB_INDEX_DATA *pMob;
    char command[MAX_INPUT_LENGTH]={'\0'};
    char arg1[MAX_INPUT_LENGTH]={'\0'};

    argument = one_argument( argument, command );
    argument = one_argument( argument, arg1 );

    EDIT_MOB(ch, pMob);

    if ( IS_NULLSTR(command) )
    {
	send_to_char( "Syntax:  shop hours [#xopening] [#xclosing]\n\r", ch );
	send_to_char( "         shop profit [#xbuying%] [#xselling%]\n\r", ch );
	send_to_char( "         shop type [#x0-4] [item type]\n\r", ch );
	send_to_char( "         shop raw [rawmaterial flag]\n\r", ch );
	send_to_char( "         shop delete [#x0-4]\n\r", ch );
	return FALSE;
    }


    if ( !str_cmp( command, "hours" ) )
    {
	if ( IS_NULLSTR(arg1) || !is_number( arg1 )
	|| IS_NULLSTR(argument) || !is_number( argument ) )
	{
	    send_to_char( "Syntax:  shop hours [#xopening] [#xclosing]\n\r", ch );
	    return FALSE;
	}

	if ( !pMob->pShop )
	{
	    pMob->pShop         = new_shop();
	    pMob->pShop->keeper = pMob->vnum;

	    LINK_SINGLE(pMob->pShop, next, shop_list);
	}

	pMob->pShop->open_hour = atoi( arg1 );
	pMob->pShop->close_hour = atoi( argument );

	send_to_char( "Shop hours set.\n\r", ch);
	return TRUE;
    }


    if ( !str_cmp( command, "profit" ) )
    {
	if ( IS_NULLSTR(arg1) || !is_number( arg1 )
	|| IS_NULLSTR(argument) || !is_number( argument ) )
	{
	    send_to_char( "Syntax:  shop profit [#xbuying%] [#xselling%]\n\r", ch );
	    return FALSE;
	}

	if ( !pMob->pShop )
	{
	    pMob->pShop         = new_shop();
	    pMob->pShop->keeper = pMob->vnum;
	    LINK_SINGLE(pMob->pShop, next, shop_list);
	}

	pMob->pShop->profit_buy     = atoi( arg1 );
	pMob->pShop->profit_sell    = atoi( argument );

	send_to_char( "Shop profit set.\n\r", ch);
	return TRUE;
    }


    if ( !str_cmp( command, "type" ) )
    {
    	int value;

    	if ( IS_NULLSTR(arg1) || !is_number( arg1 )
    			|| IS_NULLSTR(argument) )
    	{
    		send_to_char( "Syntax:  shop type [#x0-4] [item type]\n\r", ch );
    		return FALSE;
    	}

    	if ( atoi( arg1 ) >= MAX_TRADE )
    	{
    		send_to_char( Format("REdit:  May sell %d items max.\n\r", MAX_TRADE), ch );
    		return FALSE;
    	}

    	if ( ( value = flag_value( type_flags, argument ) ) == NO_FLAG )
    	{
    		send_to_char( "REdit:  That type of item is not known.\n\r", ch );
    		return FALSE;
    	}

    	if ( !pMob->pShop )
    	{
    		pMob->pShop         = new_shop();
    		pMob->pShop->keeper = pMob->vnum;
		LINK_SINGLE(pMob->pShop, next, shop_list);
    	}

    	pMob->pShop->buy_type[atoi( arg1 )] = value;

    	send_to_char( "Shop type set.\n\r", ch);
    	return TRUE;
    }

    if ( !str_cmp( command, "raw" ) )
    {
	int value;
	if(arg1[0] != '\0')
	{
	    if ( ( value = flag_value( raw_material_table, arg1 ) ) != NO_FLAG )
            {
		if ( !pMob->pShop )
		{
		    pMob->pShop         = new_shop();
		    pMob->pShop->keeper = pMob->vnum;

		    LINK_SINGLE(pMob->pShop, next, shop_list);
		}

        	pMob->pShop->raw_materials ^= value;
		if(!IS_NULLSTR(argument))
		  if((value = flag_value(raw_material_table,argument))!=NO_FLAG)
        		pMob->pShop->raw_materials ^= value;
        	send_to_char( "Raw material flag toggled.\n\r", ch);
		return TRUE;
            }
	}

	send_to_char( "Syntax: shop raw [flag]\n\r"
                  "Type '? rawmaterials' for a list of flags.\n\r", ch );
	return FALSE;
    }


    if ( !str_cmp( command, "delete" ) )
    {
	SHOP_DATA *pShop;
	SHOP_DATA *pShop_next;
	int value;
	int cnt = 0;

	if ( IS_NULLSTR(arg1) || !is_number( arg1 ) )
	{
	    send_to_char( "Syntax:  shop delete [#x0-4]\n\r", ch );
	    return FALSE;
	}

	value = atoi( argument );

	if ( !pMob->pShop )
	{
	    send_to_char( "REdit:  Non-existant shop.\n\r", ch );
	    return FALSE;
	}

	if ( value == 0 )
	{
	    pShop = pMob->pShop;
	    pMob->pShop = pMob->pShop->next;
	    free_shop( pShop );
	}
	else
	for ( pShop = pMob->pShop, cnt = 0; pShop; pShop = pShop_next, cnt++ )
	{
	    pShop_next = pShop->next;
	    if ( cnt+1 == value )
	    {
		pShop->next = pShop_next->next;
		free_shop( pShop_next );
		break;
	    }
	}

	send_to_char( "Shop deleted.\n\r", ch);
	return TRUE;
    }

    medit_shop( ch, "" );
    return FALSE;
}




MEDIT( medit_sex )          /* Moved out of medit() due to naming conflicts -- Hugin */
{
	MOB_INDEX_DATA *pMob;
	int value;

	if ( !IS_NULLSTR(argument) )
	{
		EDIT_MOB( ch, pMob );

		if ( ( value = flag_value( sex_flags, argument ) ) != NO_FLAG )
		{
			pMob->sex = value;

			send_to_char( "Sex set.\n\r", ch);
			return TRUE;
		}
	}

	send_to_char( "Syntax: sex [sex]\n\r"
			"Type '? sex' for a list of flags.\n\r", ch );
	return FALSE;
}


MEDIT( medit_act )          /* Moved out of medit() due to naming conflicts -- Hugin */
{
	MOB_INDEX_DATA *pMob;
	int value;

	if ( !IS_NULLSTR(argument) )
	{
		EDIT_MOB( ch, pMob );

		if ( ( value = flag_value( act_flags, argument ) ) != NO_FLAG )
		{
			pMob->act ^= value;
			SET_BIT( pMob->act, ACT_IS_NPC );

			send_to_char( "Act flag toggled.\n\r", ch);
			return TRUE;
		}
	}

	send_to_char( "Syntax: act [flag]\n\r"
			"Type '? act' for a list of flags.\n\r", ch );
	return FALSE;
}

MEDIT( medit_act2 )
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( !IS_NULLSTR(argument) )
    {
	EDIT_MOB( ch, pMob );

	if ( ( value = flag_value( act2_flags, argument ) ) != NO_FLAG )
	{
	    pMob->act2 ^= value;

	    send_to_char( "Act flag toggled.\n\r", ch);
	    return TRUE;
	}
    }

    send_to_char( "Syntax: act2 [flag]\n\r"
		  "Type '? act2' for a list of flags.\n\r", ch );
    return FALSE;
}


MEDIT( medit_affect )      /* Moved out of medit() due to naming conflicts -- Hugin */
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( !IS_NULLSTR(argument) )
    {
	EDIT_MOB( ch, pMob );

	if ( ( value = flag_value( affect_flags, argument ) ) != NO_FLAG )
	{
	    pMob->affected_by ^= value;

	    send_to_char( "Affect flag toggled.\n\r", ch);
	    return TRUE;
	}
    }

    send_to_char( "Syntax: affect [flag]\n\r"
		  "Type '? affect' for a list of flags.\n\r", ch );
    return FALSE;
}


MEDIT( medit_affect2 )
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( !IS_NULLSTR(argument) )
    {
	EDIT_MOB( ch, pMob );

	if ( ( value = flag_value( affect_flags2, argument ) ) != NO_FLAG )
	{
	    pMob->affected_by2 ^= value;

	    send_to_char( "Affect (2) flag toggled.\n\r", ch);
	    return TRUE;
	}
    }

    send_to_char( "Syntax: aff2 [flag]\n\r"
		  "Type '? aff2' for a list of flags.\n\r", ch );
    return FALSE;
}



MEDIT( medit_form )
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( !IS_NULLSTR(argument) )
    {
	EDIT_MOB( ch, pMob );

	if ( ( value = flag_value( form_flags, argument ) ) != NO_FLAG )
	{
	    pMob->form ^= value;
	    send_to_char( "Form toggled.\n\r", ch );
	    return TRUE;
	}
    }

    send_to_char( "Syntax: form [flags]\n\r"
		  "Type '? form' for a list of flags.\n\r", ch );
    return FALSE;
}

MEDIT( medit_part )
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( !IS_NULLSTR(argument) )
    {
	EDIT_MOB( ch, pMob );

	if ( ( value = flag_value( part_flags, argument ) ) != NO_FLAG )
	{
	    pMob->parts ^= value;
	    send_to_char( "Parts toggled.\n\r", ch );
	    return TRUE;
	}
    }

    send_to_char( "Syntax: part [flags]\n\r"
		  "Type '? part' for a list of flags.\n\r", ch );
    return FALSE;
}

MEDIT( medit_imm )
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( !IS_NULLSTR(argument) )
    {
	EDIT_MOB( ch, pMob );

	if ( ( value = flag_value( imm_flags, argument ) ) != NO_FLAG )
	{
	    pMob->imm_flags ^= value;
	    send_to_char( "Immunity toggled.\n\r", ch );
	    return TRUE;
	}
    }

    send_to_char( "Syntax: imm [flags]\n\r"
		  "Type '? imm' for a list of flags.\n\r", ch );
    return FALSE;
}

MEDIT( medit_res )
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( !IS_NULLSTR(argument) )
    {
	EDIT_MOB( ch, pMob );

	if ( ( value = flag_value( res_flags, argument ) ) != NO_FLAG )
	{
	    pMob->res_flags ^= value;
	    send_to_char( "Resistance toggled.\n\r", ch );
	    return TRUE;
	}
    }

    send_to_char( "Syntax: res [flags]\n\r"
		  "Type '? res' for a list of flags.\n\r", ch );
    return FALSE;
}

MEDIT( medit_vuln )
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( !IS_NULLSTR(argument) )
    {
	EDIT_MOB( ch, pMob );

	if ( ( value = flag_value( vuln_flags, argument ) ) != NO_FLAG )
	{
	    pMob->vuln_flags ^= value;
	    send_to_char( "Vulnerability toggled.\n\r", ch );
	    return TRUE;
	}
    }

    send_to_char( "Syntax: vuln [flags]\n\r"
		  "Type '? vuln' for a list of flags.\n\r", ch );
    return FALSE;
}

MEDIT( medit_material )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( IS_NULLSTR(argument) )
    {
	send_to_char( "Syntax:  material [string]\n\r", ch );
	return FALSE;
    }

    PURGE_DATA( pMob->material );
    pMob->material = str_dup( argument );

    send_to_char( "Material set.\n\r", ch);
    return TRUE;
}

MEDIT( medit_triggers )
{
	MOB_INDEX_DATA *pMobIndex;
	SCRIPT_DATA *ps, *step;
	char arg[MSL]={'\0'};

	EDIT_MOB(ch, pMobIndex);

	argument = one_argument(argument, arg);

	if(IS_NULLSTR(argument))
	{
		send_to_char("Syntax: trigger <delay> <keyword/string>\n\r", ch);
		send_to_char("        trigger edit <keyword/string>\n\r", ch);
		send_to_char("        trigger delete <keyword/string>\n\r", ch);
		/* Insert listing code here */
		if ( pMobIndex->triggers )
		{
			SCRIPT_DATA *ps;

			snprintf( arg, sizeof(arg), "Triggers:  [ _ " );
			for ( ps = pMobIndex->triggers; ps; ps = ps->next_in_event )
			{
				strncat( arg, ps->trigger, sizeof(arg) - strlen(arg) - 1 );
				if ( ps->next_in_event )
					strncat( arg, " _ ", sizeof(arg) - strlen(arg) - 1 );
			}
			strncat( arg, "]\n\r", sizeof(arg) - strlen(arg) - 1 );
			send_to_char(arg, ch);
		}
		return FALSE;
	}

	if(!str_cmp(arg, "delete"))
	{
		for(ps = pMobIndex->triggers; ps; ps = ps->next_in_event)
		{
			if(ps->next_in_event)
				if(!str_cmp(ps->next_in_event->trigger, argument)
						|| strstr(ps->next_in_event->trigger, argument))
				{
					step = ps->next_in_event;
					ps->next_in_event = step->next_in_event;
					free_script(step);
					send_to_char("Trigger deleted.\n\r", ch);
					return TRUE;
				}
		}
		send_to_char("Cannot find trigger to delete.\n\r", ch);
		return FALSE;
	}

	if(!str_cmp(arg, "edit"))
	{
		for(ps = pMobIndex->triggers; ps; ps = ps->next_in_event)
		{
			if(strstr(argument, ps->trigger))
			{
				string_append(ch, &ps->reaction);
				return TRUE;
			}
		}

		send_to_char("Trigger not found.\n\r", ch);
		return FALSE;
	}

	if(!is_number(arg))
	{
		send_to_char("You need to provide a delay.\n\r", ch);
		return FALSE;
	}

	if(atoi(arg) < 0)
	{
		send_to_char("Delays should be non-negative numbers.\n\r", ch);
		return FALSE;
	}

	for(ps = pMobIndex->triggers; ps; ps = ps->next_in_event)
	{
		if(strstr(argument, ps->trigger))
		{
			string_append(ch, &ps->reaction);
			ps->delay = atoi(arg);
			return TRUE;
		}
	}

	ps = new_script();
	PURGE_DATA(ps->trigger);
	ps->trigger = str_dup(argument);
	ps->delay = atoi(arg);
	ps->next_in_event = pMobIndex->triggers;
	pMobIndex->triggers = ps;
	string_append(ch, &ps->reaction);
	return TRUE;
}


MEDIT( medit_off )
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( !IS_NULLSTR(argument) )
    {
	EDIT_MOB( ch, pMob );

	if ( ( value = flag_value( off_flags, argument ) ) != NO_FLAG )
	{
	    pMob->off_flags ^= value;
	    send_to_char( "Offensive behaviour toggled.\n\r", ch );
	    return TRUE;
	}
    }

    send_to_char( "Syntax: off [flags]\n\r"
		  "Type '? off' for a list of flags.\n\r", ch );
    return FALSE;
}

MEDIT( medit_size )
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( !IS_NULLSTR(argument) )
    {
	EDIT_MOB( ch, pMob );

	if ( ( value = flag_value( size_flags, argument ) ) != NO_FLAG )
	{
	    pMob->size = value;
	    send_to_char( "Size set.\n\r", ch );
	    return TRUE;
	}
    }

    send_to_char( "Syntax: size [size]\n\r"
		  "Type '? size' for a list of sizes.\n\r", ch );
    return FALSE;
}

MEDIT( medit_hitdice )
{
    static char syntax[] = "Syntax:  hitdice <number> d <type> + <bonus>\n\r";
    char *num, *type, *bonus, *cp;
    MOB_INDEX_DATA *pMob;

    EDIT_MOB( ch, pMob );

    if ( IS_NULLSTR(argument) )
    {
	send_to_char( syntax, ch );
	return FALSE;
    }

    num = cp = argument;

    while ( isdigit( *cp ) ) ++cp;
    while ( *cp != '\0' && !isdigit( *cp ) )  *(cp++) = '\0';

    type = cp;

    while ( isdigit( *cp ) ) ++cp;
    while ( *cp != '\0' && !isdigit( *cp ) ) *(cp++) = '\0';

    bonus = cp;

    while ( isdigit( *cp ) ) ++cp;
    if ( *cp != '\0' ) *cp = '\0';

    if ( ( !is_number( num   ) || atoi( num   ) < 1 )
    ||   ( !is_number( type  ) || atoi( type  ) < 1 )
    ||   ( !is_number( bonus ) || atoi( bonus ) < 0 ) )
    {
	send_to_char( syntax, ch );
	return FALSE;
    }

    pMob->hit[DICE_NUMBER] = atoi( num   );
    pMob->hit[DICE_TYPE]   = atoi( type  );
    pMob->hit[DICE_BONUS]  = atoi( bonus );

    send_to_char( "Hitdice set.\n\r", ch );
    return TRUE;
}

MEDIT( medit_race )
{
    MOB_INDEX_DATA *pMob;
    int race;

    if (!IS_NULLSTR(argument)
    && ( race = race_lookup( argument ) ) != 0 )
    {
	EDIT_MOB( ch, pMob );

	pMob->race = race;
	pMob->off_flags   |= race_table[race].off;
	pMob->imm_flags   |= race_table[race].imm;
	pMob->res_flags   |= race_table[race].res;
	pMob->vuln_flags  |= race_table[race].vuln;
	pMob->form        |= race_table[race].form;
	pMob->parts       |= race_table[race].parts;

	send_to_char( "Race set.\n\r", ch );
	return TRUE;
    }

    if ( argument[0] == '?' )
    {
    	send_to_char( "Available races are:", ch );

    	for ( race = 0; race_table[race].name != NULL; race++ )
    	{
    		if ( ( race % 3 ) == 0 )
    			send_to_char( "\n\r", ch );
    		send_to_char( Format(" %-15s", race_table[race].name), ch );
    	}

    	send_to_char( "\n\r", ch );
    	return FALSE;
    }

    send_to_char( "Syntax:  race [race]\n\r", ch);
    send_to_char( "Type 'race ?' for a list of races.\n\r", ch );
    return FALSE;
}


MEDIT( medit_position )
{
    MOB_INDEX_DATA *pMob;
    char arg[MAX_INPUT_LENGTH]={'\0'};
    int value;

    argument = one_argument( argument, arg );

    switch ( arg[0] )
    {
    default:
	break;

    case 'S':
    case 's':
	if ( str_prefix( arg, "start" ) )
	    break;

	if ( ( value = flag_value( position_flags, argument ) ) == NO_FLAG )
	    break;

	EDIT_MOB( ch, pMob );

	pMob->start_pos = value;
	send_to_char( "Start position set.\n\r", ch );
	return TRUE;

    case 'D':
    case 'd':
	if ( str_prefix( arg, "default" ) )
	    break;

	if ( ( value = flag_value( position_flags, argument ) ) == NO_FLAG )
	    break;

	EDIT_MOB( ch, pMob );

	pMob->default_pos = value;
	send_to_char( "Default position set.\n\r", ch );
	return TRUE;
    }

    send_to_char( "Syntax:  position [start/default] [position]\n\r"
		  "Type '? position' for a list of positions.\n\r", ch );
    return FALSE;
}


MEDIT( medit_gold )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( IS_NULLSTR(argument) || !is_number( argument ) )
    {
	send_to_char( "Syntax:  wealth [number]\n\r", ch );
	return FALSE;
    }

    pMob->wealth = atoi( argument );

    send_to_char( "Wealth set.\n\r", ch);
    return TRUE;
}


MEDIT( medit_stat )
{
    MOB_INDEX_DATA *pMob;
    char arg[MAX_INPUT_LENGTH]={'\0'};
    char *output = "<@@@>";
    int a = -1, b = -1, c = -1, d = -1, e = -1, f = -1, g = -1;

    char *extra_names[7] = { "banality", "glamour", "gnosis",
	"humanity", "rage", "willpower", "health" };

    EDIT_MOB(ch, pMob);

    argument = one_argument(argument, arg);

    if ( IS_NULLSTR(argument) || IS_NULLSTR(arg) )
    {
	send_to_char( "Syntax:  stat [stat name] [number]\n\r", ch );
	send_to_char( "(Stats can be abilities, attributes, health, willpower,\n\r", ch );
	send_to_char( "rage, gnosis, blood, glamour, power ratings, backgrounds, etc.)\n\r", ch );
	send_to_char( "\n\rThis mob currently has:\n\r", ch);

	send_to_char( Format("Str: %3d  Cha: %3d  Per: %3d\n\r", pMob->stat[STAT_STR], pMob->stat[STAT_CHA], pMob->stat[STAT_PER]), ch );
	send_to_char( Format("Dex: %3d  Man: %3d  Int: %3d\n\r", pMob->stat[STAT_DEX], pMob->stat[STAT_MAN], pMob->stat[STAT_INT] ), ch );
	send_to_char( Format("Sta: %3d  App: %3d  Wit: %3d\n\r", pMob->stat[STAT_STA], pMob->stat[STAT_APP], pMob->stat[STAT_WIT] ), ch );

	return FALSE;
    }

    if((a = stat_lookup(arg, ch)) == -1)
	if((b = abil_lookup(arg, ch)) == -1)
	    if((c = disc_lookup(ch, arg)) == -1)
		if((d = virtue_lookup(arg)) == -1)
		    if((e = influence_lookup(arg)) == -1)
			if((f = background_lookup(arg)) == -1);

    if(a >= 0)
    	pMob->stat[a] = atoi(argument);
    else if(b >= 0)
    	pMob->ability[b].value = atoi(argument);
    else if(c >= 0)
    	pMob->disc[c] = atoi(argument);
    else if(d >= 0)
    	pMob->virtues[d] = atoi(argument);
    else if(e >= 0)
    {
    	pMob->influences[e] = atoi(argument);
    }
    else if(f >= 0)
    	pMob->backgrounds[f] = atoi(argument);
    else
    {
	switch(LOWER(arg[0]))
	{
	    case 'b':
		if(!str_prefix(arg, "banality"))
		{
		    pMob->GHB = atoi(argument);
		    g = 0;
		    break;
		}
	    case 'g':
		if(!str_prefix(arg, "glamour"))
		{
		    pMob->RBPG = atoi(argument);
		    g = 1;
		    break;
		}
		if(!str_prefix(arg, "gnosis"))
		{
		    pMob->GHB = atoi(argument);
		    g = 2;
		    break;
		}
	    case 'h':
		if(!str_prefix(arg, "humanity"))
		{
		    pMob->GHB = atoi(argument);
		    g = 3;
		    break;
		}
		if(!str_prefix(arg, "health"))
		{
		    pMob->health = atoi(argument);
		    g = 6;
		    break;
		}
	    case 'r':
		if(!str_prefix(arg, "rage"))
		{
		    pMob->RBPG = atoi(argument);
		    g = 4;
		    break;
		}
	    case 'w':
		if(!str_prefix(arg, "willpower"))
		{
		    pMob->willpower = atoi(argument);
		    g = 5;
		    break;
		}
	    default:
		send_to_char( "Syntax:  stat [stat name] [number]\n\r", ch );
		send_to_char(
		"(Stats can be abilities, attributes, health, willpower,\n\r",
		    ch );
		send_to_char(
		"rage, gnosis, blood, glamour, power ratings, backgrounds, etc.)\n\r",
		    ch );
		return FALSE;
		break;
	}
    }

    if(a >= 0)
	output = stat_table[a].name;
    else if(b >= 0)
	output = ability_table[a].name;
    else if(c >= 0)
    {
	if(pMob->race == race_lookup("human"))
	    output = disc_table[a].hname;
	else if(pMob->race == race_lookup("werewolf"))
	    output = disc_table[a].wname;
	else if(pMob->race == race_lookup("vampire"))
	    output = disc_table[a].vname;
    }
    else if(d >= 0)
	output = virtue_table[a].name;
    else if(e >= 0)
	output = background_table[a].name;
    else if(f >= 0)
	output = influence_table[a].name;
    else
	output = extra_names[g];

    act( "$t set.\n\r", ch, output, NULL, TO_CHAR, 1);
    return TRUE;
}


MEDIT ( medit_addmprog )
{
  int value;
  MOB_INDEX_DATA *pMob;
  MPROG_LIST *list;
  MPROG_CODE *code;
  char trigger[MSL]={'\0'};
  char phrase[MSL]={'\0'};
  char num[MSL]={'\0'};

  EDIT_MOB(ch, pMob);
  argument=one_argument(argument, num);
  argument=one_argument(argument, trigger);
  argument=one_argument(argument, phrase);

  if (!is_number(num) || IS_NULLSTR(trigger) || IS_NULLSTR(phrase) )
  {
        send_to_char("Syntax:   addmprog [vnum] [trigger] [phrase]\n\r",ch);
        return FALSE;
  }

  if ( (value = flag_value (mprog_flags, trigger) ) == NO_FLAG)
  {
        send_to_char("Valid flags are:\n\r",ch);
        show_help( ch, "mprog");
        return FALSE;
  }

  if ( ( code =get_mprog_index (atoi(num) ) ) == NULL)
  {
        send_to_char("No such MOBProgram.\n\r",ch);
        return FALSE;
  }

  list                  = new_mprog();
  list->vnum            = atoi(num);
  list->trig_type       = value;
  list->trig_phrase     = str_dup(phrase);
  list->code            = code->code;
  SET_BIT(pMob->mprog_flags,value);
  list->next            = pMob->mprogs;
  pMob->mprogs          = list;

  send_to_char( "Mprog Added.\n\r",ch);
  return TRUE;
}

MEDIT ( medit_delmprog )
{
    MOB_INDEX_DATA *pMob;
    MPROG_LIST *list;
    MPROG_LIST *list_next;
    char mprog[MSL]={'\0'};
    int value;
    int cnt = 0;

    EDIT_MOB(ch, pMob);

    one_argument( argument, mprog );
    if (!is_number( mprog ) || mprog[0] == '\0' )
    {
       send_to_char("Syntax:  delmprog [#mprog]\n\r",ch);
       return FALSE;
    }

    value = atoi ( mprog );

    if ( value < 0 )
    {
        send_to_char("Only non-negative mprog-numbers allowed.\n\r",ch);
        return FALSE;
    }

    if ( !(list= pMob->mprogs) )
    {
        send_to_char("MEdit:  Non existant mprog.\n\r",ch);
        return FALSE;
    }

    if ( value == 0 )
    {
        REMOVE_BIT(pMob->mprog_flags, pMob->mprogs->trig_type);
        list = pMob->mprogs;
        pMob->mprogs = list->next;
        free_mprog( list );
    }
    else
    {
        while ( (list_next = list->next) && (++cnt < value ) )
                list = list_next;

        if ( list_next )
        {
                REMOVE_BIT(pMob->mprog_flags, list_next->trig_type);
                list->next = list_next->next;
                free_mprog(list_next);
        }
        else
        {
                send_to_char("No such mprog.\n\r",ch);
                return FALSE;
        }
    }

    send_to_char("Mprog removed.\n\r", ch);
    return TRUE;
}


OEDIT( oedit_liqlist )
{
	int liq;
	BUFFER *buffer;

	buffer = new_buf();

	for ( liq = 0; liq_table[liq].liq_name != NULL; liq++)
	{
		if ( (liq % 21) == 0 )
			add_buf(buffer,"Name                 Color          Proof Full Thirst Food Size\n\r");

		add_buf(buffer, (char *)Format("%-20s %-14s %5d %4d %6d %4d %5d\n\r",
				liq_table[liq].liq_name,liq_table[liq].liq_color,
				liq_table[liq].liq_affect[0],liq_table[liq].liq_affect[1],
				liq_table[liq].liq_affect[2],liq_table[liq].liq_affect[3],
				liq_table[liq].liq_affect[4]));
	}

	page_to_char(buf_string(buffer),ch);
	free_buf(buffer);

	return TRUE;
}

/* Storyteller utility stuff... starting with plot generation */
void show_plot_races( CHAR_DATA *ch, long races )
{
    send_to_char(Format("Races: [  %s  ]\n\r", flag_string(plot_races, races)), ch);
}

void pedit_show_events ( CHAR_DATA *ch, PLOT_DATA *plot )
{
	EVENT_DATA *pe;

	for(pe = plot->event_list; pe; pe = pe->next_in_plot)
	{
		send_to_char(Format("Event(%d): %s, by %s\n\r", pe->vnum, pe->title, pe->author), ch);
	}
}

int make_plot_vnum()
{
    PLOT_DATA *plot;
    int i;

    for(i = 1; i < 400; i++)
    {
	if((plot = get_plot_index(i)) == NULL)
		return i;
    }

    return -1;
}

PEDIT( pedit_create )
{
	PLOT_DATA *pPlot;
	int  value;

	value = make_plot_vnum();

	if(value == -1)
	{
		send_to_char("PEdit: No vnums available, please kick Rayal to add more.\n\r",ch);
		return FALSE;
	}

	pPlot			= new_plot();
	pPlot->vnum			= value;
	pPlot->author		= str_dup(ch->name);

	LINK_SINGLE(pPlot, next, plot_list);

	ch->desc->pEdit		= (void *)pPlot;

	send_to_char( "Plot Created.\n\r", ch );
	return TRUE;
}

PEDIT( pedit_show )
{
    PLOT_DATA *pPlot;

    EDIT_PLOT(ch, pPlot);

    send_to_char(Format("Title: [%s]\n\r", pPlot->title), ch);
    send_to_char(Format("Vnum: [%d]\n\r", pPlot->vnum), ch);
    send_to_char(Format("Author: [%s]\n\r", pPlot->author), ch);

    show_plot_races(ch, pPlot->races);
    pedit_show_events(ch, pPlot);
    return FALSE;
}

PEDIT( pedit_title )
{
	PLOT_DATA *pPlot;

	EDIT_PLOT(ch, pPlot);

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Syntax:  title [string]\n\r", ch );
		return FALSE;
	}

	PURGE_DATA( pPlot->title );
	pPlot->title = str_dup( argument );
	pPlot->title[0] = UPPER( pPlot->title[0]  );

	send_to_char( "Title set.\n\r", ch);
	return TRUE;
}

PEDIT( pedit_remove )
{
	PLOT_DATA *plot;
	PLOT_DATA *pp;

	EDIT_PLOT(ch, plot);

	for(pp = plot_list; pp; pp = pp->next)
	{
		if(pp->next == plot)
		{
			pp->next = plot->next;
			free_plot(plot);
			return TRUE;
		}
	}

	send_to_char("No such plot.\n\r", ch);
	return FALSE;
}

PEDIT( pedit_races )
{
    PLOT_DATA *pPlot;
    int value;

    if ( !IS_NULLSTR(argument) )
    {
	EDIT_PLOT( ch, pPlot );

	if ( ( value = flag_value( plot_races, argument ) ) != NO_FLAG )
	{
	    pPlot->races ^= value;

	    send_to_char( "Race toggled.\n\r", ch);
	    return TRUE;
	}
    }

    send_to_char( "Syntax: race [flag]\n\r", ch);
    send_to_char("Type '? races' for a list of flags.\n\r", ch );
    return FALSE;
}

PEDIT( pedit_addevent )
{
	PLOT_DATA *pPlot;
	EVENT_DATA *pEvent;
	int  value;

	EDIT_PLOT(ch, pPlot);

	value = make_event_vnum();

	if(value == -1)
	{
		send_to_char("EEdit: No vnums available, please kick Rayal to add more.\n\r",ch);
		return FALSE;
	}

	pEvent			= new_event();
	pEvent->vnum		= value;
	pEvent->author		= str_dup(ch->name);

	LINK_SINGLE(pEvent, next, event_list);
	pEvent->plot		= (PLOT_DATA *)ch->desc->pEdit;

	LINK_SINGLE(pEvent, next_in_plot, pPlot->event_list);

	edit_done( ch );
	do_eedit(ch, (char *)Format("%d", pEvent->vnum));

	send_to_char( "Event Created.\n\r", ch );
	return TRUE;
}

PEDIT( pedit_delevent )
{
	EVENT_DATA *event;
	EVENT_DATA *pe;

	EDIT_EVENT(ch, event);

	for(pe = event_list; pe; pe = pe->next)
	{
		if(pe->next == event)
		{
			pe->next = event->next;
			free_event(event);
			return TRUE;
		}
	}

	send_to_char("No such event.\n\r", ch);
	return FALSE;
}


PEDIT( pedit_assignevent )
{
	EVENT_DATA *pEvent;
	char name[MSL]={'\0'};
	char buf[MSL]={'\0'};

	EDIT_EVENT(ch, pEvent);

	one_argument( argument, name );

	if ( IS_NULLSTR(name) )
	{
		send_to_char( "Syntax:  assignevent [$name]  -toggles author\n\r", ch );
		send_to_char( "Syntax:  assignevent All      -allows everyone\n\r", ch );
		return FALSE;
	}

	name[0] = UPPER( name[0] );

	if ( strstr( pEvent->author, name ) != NULL )
	{
		pEvent->author = string_replace( pEvent->author, name, "\0" );
		pEvent->author = string_unpad( pEvent->author );

		if ( pEvent->author[0] == '\0' )
		{
			PURGE_DATA( pEvent->author );
			pEvent->author = str_dup( "None" );
		}
		send_to_char( "Author removed.\n\r", ch );
		return TRUE;
	}
	else
	{
		buf[0] = '\0';
		if ( strstr( pEvent->author, "None" ) != NULL )
		{
			pEvent->author = string_replace( pEvent->author, "None", "\0" );
			pEvent->author = string_unpad( pEvent->author );
		}

		if (pEvent->author[0] != '\0' )
		{
			strncat( buf, pEvent->author, sizeof(buf) - strlen(buf) - 1 );
			strncat( buf, " ", sizeof(buf) - strlen(buf) - 1 );
		}
		strncat( buf, name, sizeof(buf) - strlen(buf) - 1 );
		PURGE_DATA( pEvent->author );
		pEvent->author = string_proper( str_dup( buf ) );

		send_to_char( "Author added.\n\r", ch );
		send_to_char( pEvent->author,ch);
		return TRUE;
	}

	return FALSE;
}


int make_event_vnum()
{
    EVENT_DATA *event;
    int i;

    for(i = 1; i < 400; i++)
    {
	if((event = get_event_index(i)) == NULL)
		return i;
    }

    return -1;
}

EEDIT( eedit_create )
{
    EVENT_DATA *pEvent;
    int  value;

    value = make_event_vnum();

    if(value == -1)
    {
	send_to_char("EEdit: No vnums available, please kick Rayal to add more.\n\r",ch);
	return FALSE;
    }

    pEvent			= new_event();
    pEvent->vnum		= value;
    pEvent->author		= str_dup(ch->name);
    LINK_SINGLE(pEvent, next, event_list);

    ch->desc->pEdit		= (void *)pEvent;
    pEvent->loc			= 10000;

    send_to_char( "Event Created.\n\r", ch );
    return TRUE;
}

void eedit_show_actors ( CHAR_DATA *ch, EVENT_DATA *event )
{
	ACTOR_DATA *a;
	MOB_INDEX_DATA *mob;

	send_to_char("Actors: ", ch);
	for(a = event->actors; a; a = a->next)
	{
		if((mob = get_mob_index(a->mob)) != NULL)
		{
			send_to_char(Format("(%d) %s", a->mob, mob->short_descr), ch);
		}
	}
	send_to_char("\n\r", ch);
}

void eedit_show_scripts ( CHAR_DATA *ch, EVENT_DATA *event )
{
	SCRIPT_DATA *ps;

	for(ps = event->script_list; ps; ps = ps->next_in_event)
	{
		send_to_char( Format("Script(%d):\n\r%s - %s, by %s\n\r", ps->vnum, ps->trigger, ps->reaction, ps->author), ch);
	}
}

EEDIT( eedit_show )
{
	EVENT_DATA *pEvent;
	ROOM_INDEX_DATA *room;

	EDIT_EVENT(ch, pEvent);

	send_to_char(Format("Title: [%s]\n\r", pEvent->title), ch);
	send_to_char(Format("Vnum: [%d]\n\r", pEvent->vnum), ch);
	send_to_char(Format("Author: [%s]\n\r", pEvent->author), ch);
	room = get_room_index(pEvent->loc);
	send_to_char(Format("Location: [%s] (%d)\n\r", room->name, pEvent->loc), ch);
	show_plot_races(ch, pEvent->races);
	eedit_show_actors(ch, pEvent);
	eedit_show_scripts(ch, pEvent);
	return FALSE;
}

EEDIT( eedit_location )
{
	EVENT_DATA *pEvent;
	int value;

	EDIT_EVENT(ch, pEvent);

	if(!is_number(argument))
	{
		send_to_char("Where do you want the event to take place?\n\r", ch);
		return FALSE;
	}

	value = atoi(argument);
	if(get_room_index(value) == NULL)
	{
		send_to_char("No such location.", ch);
		return FALSE;
	}

	pEvent->loc = value;
	return TRUE;
}

EEDIT( eedit_title )
{
	EVENT_DATA *pEvent;

	EDIT_EVENT(ch, pEvent);

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Syntax:  title [string]\n\r", ch );
		return FALSE;
	}

	PURGE_DATA( pEvent->title );
	pEvent->title = str_dup( argument );
	pEvent->title[0] = UPPER( pEvent->title[0]  );

	send_to_char( "Title set.\n\r", ch);
	return TRUE;
}

EEDIT( eedit_remove )
{
	EVENT_DATA *event;
	EVENT_DATA *pe;

	EDIT_EVENT(ch, event);

	for(pe = event_list; pe; pe = pe->next)
	{
		if(pe->next == event)
		{
			pe->next = event->next;
			free_event(event);
			return TRUE;
		}
	}

	send_to_char("No such event.\n\r", ch);
	return FALSE;
}

EEDIT( eedit_races )
{
	EVENT_DATA *pEvent;
	int value;

	if ( !IS_NULLSTR(argument) )
	{
		EDIT_EVENT( ch, pEvent );

		if ( ( value = flag_value( plot_races, argument ) ) != NO_FLAG )
		{
			pEvent->races ^= value;

			send_to_char( "Race toggled.\n\r", ch);
			return TRUE;
		}
	}

	send_to_char( "Syntax: race [flag]\n\r", ch);
	send_to_char( "Type '? races' for a list of flags.\n\r", ch );
	return FALSE;
}

EEDIT( eedit_scripts )
{
	EVENT_DATA *pEvent;
	SCRIPT_DATA *pScript;
	int  value;

	EDIT_EVENT(ch, pEvent);

	value = make_script_vnum();

	if(value == -1)
	{
		send_to_char("SEDIT: No vnums available, please kick Rayal to add more.\n\r",ch);
		return FALSE;
	}

	pScript			= new_script();
	pScript->vnum		= value;
	PURGE_DATA(pScript->author);
	pScript->author		= str_dup(ch->name);

	pScript->next		= script_first;
	script_first		= pScript;
	pScript->event		= (EVENT_DATA *)ch->desc->pEdit;

	pScript->next_in_event	= pEvent->script_list;
	pEvent->script_list		= pScript;

	edit_done( ch );
	do_sedit( ch, (char *)Format("%d", pScript->vnum) );

	send_to_char( "Script Created.\n\r", ch );
	return TRUE;
}

EEDIT( eedit_delscript )
{
	SCRIPT_DATA *script;
	SCRIPT_DATA *ps;

	EDIT_SCRIPT(ch, script);

	for(ps = script_first; ps; ps = ps->next)
	{
		if(ps->next == script)
		{
			ps->next = script->next;
			free_script(script);
			return TRUE;
		}
	}

	send_to_char("No such script.\n\r", ch);
	return FALSE;
}

SEDIT( sedit_show )
{
    SCRIPT_DATA *pScript;

    EDIT_SCRIPT(ch, pScript);

    send_to_char(Format("Vnum: [%d]\n\r", pScript->vnum), ch);
    send_to_char(Format("Author: [%s]\n\r", pScript->author), ch);
    send_to_char(Format("Trigger: [%s]\n\r", pScript->trigger), ch);
    send_to_char(Format("Reaction: [%s]\n\r", pScript->reaction), ch);
    send_to_char(Format("Actor: [%d]\n\r", pScript->actor > 0 ? pScript->actor : -1), ch);
    send_to_char(Format("Delay: [%d]\n\r", pScript->delay), ch);
    send_to_char(Format("Event: [%s] (Vnum: %d)\n\r", pScript->event ? pScript->event->title : "None",
    		pScript->event ? pScript->event->vnum : -1), ch);

    return FALSE;
}

int make_script_vnum()
{
	SCRIPT_DATA *script;
	int i;

	for(i = 1; i < 400; i++)
	{
		if((script = get_script_index(i)) == NULL)
			return i;
	}

	return -1;
}

SEDIT( sedit_create )
{
	SCRIPT_DATA *pScript;
	int  value;

	value = make_script_vnum();

	if(value == -1)
	{
		send_to_char("SEDIT: No vnums available, please kick Rayal to add more.\n\r",ch);
		return FALSE;
	}

	pScript			= new_script();
	pScript->vnum		= value;
	PURGE_DATA(pScript->author);
	pScript->author		= str_dup(ch->name);

	pScript->next		= script_first;
	script_first		= pScript;
	ch->desc->pEdit		= (void *)pScript;
	pScript->first_script	= FALSE;

	send_to_char( "Script Created.\n\r", ch );
	return TRUE;
}


SEDIT( sedit_delay )
{
	SCRIPT_DATA *pScript;

	EDIT_SCRIPT(ch, pScript);

	if ( IS_NULLSTR(argument) || !is_number( argument ) )
	{
		send_to_char( "Syntax:  delay [number]\n\r", ch );
		return FALSE;
	}

	pScript->delay = atoi( argument );

	send_to_char( "Delay set.\n\r", ch);
	return TRUE;
}

SEDIT( sedit_eventfirst )
{
	SCRIPT_DATA *pScript;
	SCRIPT_DATA *ps;

	EDIT_SCRIPT(ch, pScript);

	if(pScript->event == NULL)
	{
		send_to_char("That script is not in an event.\n\r", ch);
		return FALSE;
	}

	pScript->first_script = TRUE;

	for(ps = pScript->event->script_list; ps; ps = ps->next_in_event)
	{
		if(ps == pScript)
			continue;

		if(ps->first_script)
			ps->first_script = FALSE;
	}

	return TRUE;
}

SEDIT( sedit_trigger )
{
	SCRIPT_DATA *pScript;

	EDIT_SCRIPT(ch, pScript);

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Syntax:  trigger [string]\n\r", ch );
		return FALSE;
	}

	PURGE_DATA( pScript->trigger );
	pScript->trigger = str_dup( argument );

	send_to_char( "Trigger set.\n\r", ch);
	return TRUE;
}

SEDIT( sedit_reaction )
{
	SCRIPT_DATA *pScript;

	EDIT_SCRIPT(ch, pScript);

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Syntax:  reaction [string]\n\r", ch );
		return FALSE;
	}

	PURGE_DATA( pScript->reaction );
	pScript->reaction = str_dup( argument );

	send_to_char( "Reaction set.\n\r", ch);
	return TRUE;
}

SEDIT( sedit_remove )
{
	SCRIPT_DATA *script;
	SCRIPT_DATA *ps;

	EDIT_SCRIPT(ch, script);

	for(ps = script_first; ps; ps = ps->next)
	{
		if(ps->next == script)
		{
			ps->next = script->next;
			free_script(script);
			return TRUE;
		}
	}

	send_to_char("No such script.\n\r", ch);
	return FALSE;
}

SEDIT( sedit_event )
{
	EVENT_DATA *event;
	SCRIPT_DATA *ps;

	EDIT_SCRIPT(ch, ps);

	if(!is_number(argument))
	{
		send_to_char("Add script to which event?\n\r", ch);
		return FALSE;
	}
	if((event = get_event_index(atoi(argument))) == NULL)
	{
		send_to_char("No such event.\n\r", ch);
		return FALSE;
	}

	ps->event = event;

	ps->next = event->script_list;
	event->script_list = ps;

	send_to_char("Script added to event.\n\r", ch);
	return TRUE;
}

SEDIT( sedit_actor )
{
	MOB_INDEX_DATA *pMob;
	SCRIPT_DATA *script;

	EDIT_SCRIPT(ch, script);

	if(!is_number(argument))
	{
		send_to_char("Which mob will be the actor?\n\r", ch);
		return FALSE;
	}

	if((pMob = get_mob_index(atoi(argument))) == NULL)
	{
		send_to_char("Mobile does not exist.\n\r", ch);
		return FALSE;
	}
	script->actor = pMob->vnum;
	send_to_char("Actor set.\n\r", ch);
	return TRUE;
}


EEDIT( eedit_actors )
{
	MOB_INDEX_DATA *pMob;
	EVENT_DATA *event;
	ACTOR_DATA *actor, *actor_next;
	char arg[MSL]={'\0'};

	EDIT_EVENT(ch, event);

	argument = one_argument(argument, arg);

	if(!is_number(arg) && str_cmp(arg, "clear"))
	{
		send_to_char("Which mob will be the actor?\n\r", ch);
		return FALSE;
	}

	if(!str_cmp(arg, "clear"))
	{
		for(actor = event->actors; actor != NULL; actor = actor_next)
		{
			actor_next = actor->next;
			free_actor(actor);
			event->actors = NULL;
		}
		return TRUE;
	}

	if((pMob = get_mob_index(atoi(arg))) == NULL)
	{
		send_to_char("Mobile does not exist.\n\r", ch);
		return FALSE;
	}
	actor = new_actor();
	actor->mob = pMob->vnum;
	actor->next = event->actors;
	event->actors = actor;
	if(!IS_NULLSTR(argument))
		eedit_actors(ch, argument);
	return TRUE;
}

/* Personality crap */
AIEDIT( aiedit_show )
{
    PERSONA *pPersona;

    EDIT_PERSONA(ch, pPersona);

    send_to_char(Format("Name: [%s]\n\r", pPersona->name), ch);
    send_to_char(Format("Vnum: [%d]\n\r", pPersona->vnum), ch);
    return FALSE;
}

int make_persona_vnum()
{
	PERSONA *persona;
	int i;

	for(i = 1; i < 400; i++)
	{
		if((persona = get_persona_index(i)) == NULL)
			return i;
	}

	return -1;
}

AIEDIT( aiedit_create )
{
	PERSONA *pPersona;
	int value;

	value = make_persona_vnum();

	if(value == -1)
	{
		send_to_char("AIEdit: No vnums available, please kick Rayal to add more.\n\r",ch);
		return FALSE;
	}

	pPersona		=   new_persona();
	pPersona->vnum	=   value;
	pPersona->next	=   persona_first;
	persona_first	=   pPersona;
	ch->desc->pEdit     =   (void *)pPersona;

	send_to_char( "Personality Created.\n\r", ch );
	return TRUE;
}

AIEDIT( aiedit_remove )
{
	PERSONA *persona;
	PERSONA *pp;

	EDIT_PERSONA(ch, persona);

	for(pp = persona_first; pp; pp = pp->next)
	{
		if(pp->next == persona)
		{
			pp->next = persona->next;
			free_persona(persona);
			return TRUE;
		}
	}

	send_to_char("No such personality.\n\r", ch);
	return FALSE;
}

AIEDIT( aiedit_name )
{
	PERSONA *pPersona;

	EDIT_PERSONA(ch, pPersona);

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Syntax:  name [string]\n\r", ch );
		return FALSE;
	}

	PURGE_DATA( pPersona->name );
	pPersona->name = str_dup( argument );

	send_to_char( "Name set.\n\r", ch);
	return TRUE;
}

AIEDIT( aiedit_trigger )
{
	REACT *pReact;
	PERSONA *pPersona;
	int a;

	EDIT_PERSONA(ch, pPersona);

	if(!is_number(argument))
	{
		send_to_char("Syntax: trigger [attitude]\n\r", ch);
		return FALSE;
	}

	a = atoi(argument);
	if(a > MAX_ATTITUDE || a < 0)
	{
		send_to_char("Attitude must be between 1 and 10.\n\r", ch);
		return FALSE;
	}

	pReact              =   new_react();
	pReact->attitude	=   a;
	pReact->next_in_matrix_loc = pPersona->matrix[a-1];
	pPersona->matrix[a-1] = pReact;
	pReact->next        =   react_first;
	react_first         =   pReact;
	ch->desc->pEdit     =   (void *)pReact;

	send_to_char( "Trigger/Reaction Set Created.\n\r", ch );
	return TRUE;
}

void rsedit_show_reactions ( CHAR_DATA *ch, REACT *react )
{
	REACT_DATA *pe;
	int i = 0;

	send_to_char("Reactions:\n\r", ch);
	for(pe = react->react; pe; pe = pe->next)
	{
		i++;
		send_to_char(Format("(%d): %s\n\r", i, pe->reaction), ch);
	}
	if(i == 0)
	{
		send_to_char("No reactions found.\n\r", ch);
	}
}

RSEDIT( rsedit_show )
{
    REACT *pReact;

    EDIT_REACT(ch, pReact);

    send_to_char(Format("Persona:  [%s]\n\r", pReact->persona ? pReact->persona->name : "None"), ch);
    send_to_char(Format("Attitude: [%d]\n\r", pReact->attitude), ch);
    send_to_char(Format("Trigger: [%s]\n\r", pReact->trig), ch);
    rsedit_show_reactions(ch, pReact);
    return FALSE;
}

RSEDIT( rsedit_create )
{
    REACT *pReact;

    pReact		=   new_react();
    pReact->next	=   react_first;
    react_first		=   pReact;
    ch->desc->pEdit     =   (void *)pReact;

    send_to_char( "Trigger/Reaction Set Created.\n\r", ch );
    return TRUE;
}

RSEDIT( rsedit_remove )
{
	REACT *react;
	REACT *pr;

	EDIT_REACT(ch, react);

	for(pr = react_first; pr; pr = pr->next)
	{
		if(pr->next == react)
		{
			pr->next = react->next;
			free_react(react);
			return TRUE;
		}
	}

	send_to_char("No such trigger reaction set.\n\r", ch);
	return FALSE;
}

RSEDIT( rsedit_trigger )
{
    REACT *pReact;

    EDIT_REACT(ch, pReact);

    if ( IS_NULLSTR(argument) )
    {
	send_to_char( "Syntax:  trigger [string]\n\r", ch );
	return FALSE;
    }

    PURGE_DATA( pReact->trig );
    pReact->trig = str_dup( argument );
    pReact->trig[0] = UPPER( pReact->trig[0]  );

    send_to_char( "Trigger set.\n\r", ch);
    return TRUE;
}

RSEDIT( rsedit_reaction )
{
    REACT *pReact;
    REACT_DATA *pr;
    char arg[MSL]={'\0'};

    EDIT_REACT(ch, pReact);

    argument = one_argument(argument, arg);

    if(!is_number(arg) || IS_NULLSTR(argument))
    {
	send_to_char("Syntax: reaction [weight (1-10)] [reaction string]\n\r", ch);
	return FALSE;
    }

    pr = new_react_data();
    pr->priority = atoi(arg);
    PURGE_DATA(pr->reaction);
    pr->reaction = str_dup(argument);
    pr->next = pReact->react;
    pReact->react = pr;
    send_to_char("Reaction added for trigger.\n\r", ch);
    return TRUE;
}

RSEDIT( rsedit_delreaction )
{
    REACT *react;
    REACT_DATA *pr;
    REACT_DATA *r;
    int i;

    EDIT_REACT(ch, react);

    if(!is_number(argument))
    {
	send_to_char("Which reaction do you wish to delete?\n\r", ch);
	return FALSE;
    }

    i = atoi(argument);

    for(pr = react->react; pr; pr = pr->next)
    {
	i--;
	if(i <= 0)
	{
	    break;
	}
    }

    if(pr == NULL)
    {
	send_to_char("No such reaction.\n\r", ch);
	return FALSE;
    }

    for(r = react->react; r; r = r->next)
    {
	if(r->next == pr)
	{
	    r->next = pr->next;
	    break;
	}
    }
    free_react_data(pr);

    return TRUE;
}

RSEDIT( rsedit_persona )
{
REACT *pr;

    EDIT_REACT(ch, pr);

    if(pr->attitude == 0)
    {
	send_to_char("You have to set an attitude before adding to a persona.\n\r", ch);
	return FALSE;
    }

    if(!is_number(argument))
    {
	send_to_char("Syntax: persona [vnum]\n\r", ch);
	return FALSE;
    }

    if(pr->persona != NULL)
    {
	send_to_char("This trigger set already belongs to a persona.\n\r", ch);
	return FALSE;
    }

    if((pr->persona = get_persona_index(atoi(argument))) == NULL)
    {
	send_to_char("No such personality.\n\r", ch);
	return FALSE;
    }

    pr->next_in_matrix_loc = pr->persona->matrix[pr->attitude - 1];
    pr->persona->matrix[pr->attitude - 1] = pr;

return TRUE;
}

RSEDIT( rsedit_attitude )
{
REACT *pReact;
int i;

    EDIT_REACT(ch, pReact);

    if(!is_number(argument))
    {
	send_to_char("What attitude do you want to place this trigger set into?\n\r", ch);
	return FALSE;
    }

    i = atoi(argument);
    if(i > MAX_ATTITUDE || i < 0)
    {
	send_to_char("Attitude must be between 1 and 10.\n\r", ch);
	return FALSE;
    }

    pReact->attitude = i;
    return TRUE;
}

/*
 * Help Editors
 */
HEDIT( hedit_show )
{
    HELP_DATA *help;

    EDIT_HELP(ch, help);

    send_to_char( "Essential Fields.\n\r", ch );

    send_to_char( Format("Trust: [%5d]  Keywords: %s\n\r", help->level, help->keyword), ch );
    send_to_char( Format("Races:  %s\n\r", help->races), ch );
    send_to_char( Format("Clans:  %s\n\r", help->clans), ch );

    send_to_char( "\n\rFormatted fields:\n\r", ch );
    send_to_char( Format("Topic:  %s\n\r", help->topic), ch );
    send_to_char( Format("Syntax:  %s\n\r", help->syntax), ch );
    send_to_char( Format("Body:\n\r%s\n\r", help->description), ch );
    send_to_char( Format("See also:  %s\n\r", help->see_also), ch );
    send_to_char( Format("Website:  %s\n\r", help->website), ch );

    send_to_char("\n\rDo not use in conjunction with formatted fields.\n\r",ch);
    send_to_char( Format("Unformatted:\n\r%s\n\r", help->unformatted), ch );

    return FALSE;
}


HEDIT( hedit_trust )
{
    HELP_DATA *help;

    EDIT_HELP(ch, help);

    if ( IS_NULLSTR(argument) || !is_number( argument ) )
    {
	send_to_char( "Syntax:  trust [number]\n\r", ch );
	return FALSE;
    }

    help->level = atoi( argument );

    send_to_char( "Trust set.\n\r", ch);
    return TRUE;
}


HEDIT( hedit_keyword )
{
    HELP_DATA *help;

    EDIT_HELP(ch, help);

    if ( IS_NULLSTR(argument) )
    {
	send_to_char( "Syntax:  keywords [string]\n\r", ch );
	return FALSE;
    }

    PURGE_DATA( help->keyword );
    help->keyword = str_dup( argument );

    send_to_char( "Keywords set.\n\r", ch);
    return TRUE;
}


HEDIT( hedit_body )
{
    HELP_DATA *help;

    EDIT_HELP(ch, help);

    if ( IS_NULLSTR(argument) )
    {
	string_append( ch, &help->description );
	return TRUE;
    }

    send_to_char( "Syntax:  body\n\r", ch );
    return FALSE;
}


HEDIT( hedit_topic )
{
    HELP_DATA *help;

    EDIT_HELP(ch, help);

    if ( IS_NULLSTR(argument) )
    {
	send_to_char( "Syntax:  topic [string]\n\r", ch );
	return FALSE;
    }

    PURGE_DATA( help->topic );
    help->topic = str_dup( argument );

    send_to_char( "Topic set.\n\r", ch);
    return TRUE;
}


HEDIT( hedit_website )
{
    HELP_DATA *help;

    EDIT_HELP(ch, help);

    if ( IS_NULLSTR(argument) )
    {
	send_to_char( "Syntax:  website [string]\n\r", ch );
	return FALSE;
    }

    PURGE_DATA( help->website );
    help->website = str_dup( argument );

    send_to_char( "Website set.\n\r", ch);
    return TRUE;
}


HEDIT( hedit_syntax )
{
    HELP_DATA *help;

    EDIT_HELP(ch, help);

    if ( IS_NULLSTR(argument) )
    {
	string_append( ch, &help->syntax );
	return TRUE;
    }

    send_to_char( "Syntax:  syntax\n\r", ch );
    return FALSE;
}


HEDIT( hedit_see_also )
{
    HELP_DATA *help;

    EDIT_HELP(ch, help);

    if ( IS_NULLSTR(argument) )
    {
	string_append( ch, &help->see_also );
	return TRUE;
    }

    send_to_char( "Syntax:  see\n\r", ch );
    return FALSE;
}


HEDIT( hedit_quote )
{
    HELP_DATA *help;

    EDIT_HELP(ch, help);

    if ( IS_NULLSTR(argument) )
    {
	string_append( ch, &help->quote );
	return TRUE;
    }

    send_to_char( "Syntax:  quote\n\r", ch );
    return FALSE;
}


HEDIT( hedit_no_format )
{
    HELP_DATA *help;

    EDIT_HELP(ch, help);

    if ( IS_NULLSTR(argument) )
    {
	string_append( ch, &help->unformatted );
	return TRUE;
    }

    send_to_char( "Syntax:  unformatted\n\r", ch );
    return FALSE;
}


HEDIT( hedit_clans )
{
	HELP_DATA *help;
	char name[MSL]={'\0'};
	char buf[MSL]={'\0'};

	EDIT_HELP(ch, help);

	one_argument( argument, name );

	if ( IS_NULLSTR(name) )
	{
		send_to_char( "Syntax:  clans [$clan]  -toggles clan\n\r", ch );
		send_to_char( "Syntax:  clans All      -allows everyone\n\r", ch );
		return FALSE;
	}

	name[0] = UPPER( name[0] );

	if ( strstr( help->clans, name ) != NULL )
	{
		help->clans = string_replace( help->clans, name, "\0" );
		help->clans = string_unpad( help->clans );

		if ( help->clans[0] == '\0' )
		{
			PURGE_DATA( help->clans );
			help->clans = str_dup( "None" );
		}
		send_to_char( "Race removed.\n\r", ch );
		return TRUE;
	}
	else
	{
		buf[0] = '\0';
		if ( strstr( help->clans, "None" ) != NULL )
		{
			help->clans = string_replace( help->clans, "None", "\0" );
			help->clans = string_unpad( help->clans );
		}

		if (help->clans[0] != '\0' )
		{
			strncat( buf, help->clans, sizeof(buf) - strlen(buf) - 1 );
			strncat( buf, " ", sizeof(buf) - strlen(buf) - 1 );
		}
		strncat( buf, name, sizeof(buf) - strlen(buf) - 1 );
		PURGE_DATA( help->clans );
		help->clans = string_proper( str_dup( buf ) );

		send_to_char( "Clan added.\n\r", ch );
		send_to_char( help->clans,ch);
		return TRUE;
	}

	return FALSE;
}


HEDIT( hedit_races )
{
	HELP_DATA *help;
	char name[MSL]={'\0'};
	char buf[MSL]={'\0'};

	EDIT_HELP(ch, help);

	one_argument( argument, name );

	if ( IS_NULLSTR(name) )
	{
		send_to_char( "Syntax:  races [$race]  -toggles race\n\r", ch );
		send_to_char( "Syntax:  races All      -allows everyone\n\r", ch );
		return FALSE;
	}

	name[0] = UPPER( name[0] );

	if ( strstr( help->races, name ) != NULL )
	{
		help->races = string_replace( help->races, name, "\0" );
		help->races = string_unpad( help->races );

		if ( help->races[0] == '\0' )
		{
			PURGE_DATA( help->races );
			help->races = str_dup( "None" );
		}
		send_to_char( "Race removed.\n\r", ch );
		return TRUE;
	}
	else
	{
		buf[0] = '\0';
		if ( strstr( help->races, "None" ) != NULL )
		{
			help->races = string_replace( help->races, "None", "\0" );
			help->races = string_unpad( help->races );
		}

		if (help->races[0] != '\0' )
		{
			strncat( buf, help->races, sizeof(buf) - strlen(buf) - 1 );
			strncat( buf, " ", sizeof(buf) - strlen(buf) - 1 );
		}
		strncat( buf, name, sizeof(buf) - strlen(buf) - 1 );
		PURGE_DATA( help->races );
		help->races = string_proper( str_dup( buf ) );

		send_to_char( "Race added.\n\r", ch );
		send_to_char( help->races,ch);
		return TRUE;
	}

	return FALSE;
}


HEDIT( hedit_create )
{
    HELP_DATA *help;

    help = new_help();
    PURGE_DATA(help->keyword);
    help->keyword		= str_dup(argument);
    PURGE_DATA(help->races);
    help->races			= NULL;
    PURGE_DATA(help->clans);
    help->clans			= NULL;
    PURGE_DATA(help->topic);
    help->topic			= NULL;
    PURGE_DATA(help->syntax);
    help->syntax		= NULL;
    PURGE_DATA(help->description);
    help->description		= NULL;
    PURGE_DATA(help->see_also);
    help->see_also		= NULL;
    PURGE_DATA(help->quote);
    help->quote			= NULL;
    PURGE_DATA(help->website);
    help->website		= NULL;
    PURGE_DATA(help->unformatted);
    help->unformatted		= NULL;
    help->level			= 0;

    LINK_SINGLE(help, next, help_list);
    top_help++;
    ch->desc->pEdit		= (void *)help;

    send_to_char( "Help Created.\n\r", ch );
    return TRUE;
}

/*
 * Tip editing. (Tips use hedit commands beside creation.)
 */
HEDIT( tipedit_create )
{
    HELP_DATA *help;

    help = new_help();
    PURGE_DATA(help->keyword);
    help->keyword		= str_dup(argument);
    PURGE_DATA(help->races);
    help->races			= NULL;
    PURGE_DATA(help->clans);
    help->clans			= NULL;
    PURGE_DATA(help->topic);
    help->topic			= NULL;
    PURGE_DATA(help->syntax);
    help->syntax		= NULL;
    PURGE_DATA(help->description);
    help->description		= NULL;
    PURGE_DATA(help->see_also);
    help->see_also		= NULL;
    PURGE_DATA(help->quote);
    help->quote			= NULL;
    PURGE_DATA(help->website);
    help->website		= NULL;
    PURGE_DATA(help->unformatted);
    help->unformatted		= NULL;
    help->level			= 0;
    LINK_SINGLE(help, next, tip_list);
    top_tip++;
    ch->desc->pEdit		= (void *)help;

    send_to_char( "Tip Created.\n\r", ch );
    return TRUE;
}

/*
 * Backgrounds and Knowledges editor functions.
 */
NEDIT( bedit_create )
{
	NOTE_DATA *note;

	note = new_note();
	PURGE_DATA(note->to_list);
	note->to_list		= str_dup(argument);
	PURGE_DATA(note->text);
	note->text			= NULL;
	note->type			= NOTE_BACKGROUND;
	PURGE_DATA( note->sender );
	note->sender		= str_dup( ch->name );
	PURGE_DATA( note->date );
	note->date			= str_dup( ctime( &current_time ) );
	note->date_stamp		= current_time;

	LINK_SINGLE(note, next, bg_list);

	ch->desc->pEdit		= (void *)note;

	send_to_char( "Background Created.\n\r", ch );
	return TRUE;
}

NEDIT( kedit_create )
{
	NOTE_DATA *note;

	note = new_note();
	PURGE_DATA(note->to_list);
	note->to_list		= str_dup(argument);
	PURGE_DATA(note->text);
	note->text			= NULL;
	note->type			= NOTE_KNOWLEDGE;
	PURGE_DATA( note->sender );
	note->sender		= str_dup( ch->name );
	PURGE_DATA( note->date );
	note->date			= str_dup( ctime( &current_time ) );
	note->date_stamp		= current_time;

	LINK_SINGLE(note, next, know_list);
	ch->desc->pEdit		= (void *)note;

	send_to_char( "Fact Created.\n\r", ch );
	return TRUE;
}

NEDIT( kbedit_create )
{
	char arg[MAX_INPUT_LENGTH]={'\0'};

	argument = one_argument(argument, arg);

	if(!str_prefix(arg, "knowledge"))
	{
		kedit_create(ch, argument);
		return TRUE;
	}
	else if(!str_prefix(arg, "background"))
	{
		bedit_create(ch, argument);
		return TRUE;
	}

	send_to_char("Syntax: edit create [knowledge/background]\n\r", ch);
	return FALSE;
}

void parse_note( CHAR_DATA *ch, char *argument, int type );

NEDIT( kbedit_show )
{
	NOTE_DATA *note;

	EDIT_NOTE(ch, note);

	send_to_char( Format("Difficulty: [%5s] Successes: [%d]\n\rKeywords: %s\n\r", note->subject, note->successes, note->to_list), ch );
	show_plot_races(ch, note->race);

	send_to_char( Format("Text:\n\r%s\n\r", note->text), ch );

	return FALSE;
}


NEDIT( kbedit_body )
{
	NOTE_DATA *note;

	EDIT_NOTE(ch, note);

	if ( IS_NULLSTR(argument) )
	{
		string_append( ch, &note->text );
		return TRUE;
	}

	send_to_char( "Syntax:  text\n\r", ch );
	return FALSE;
}


NEDIT( kbedit_diff )
{
	NOTE_DATA *note;

	EDIT_NOTE(ch, note);

	if ( IS_NULLSTR(argument) || !is_number(argument) )
	{
		send_to_char("Syntax: difficulty <number>\n\r", ch);
		return FALSE;
	}

	PURGE_DATA( note->subject );
	note->subject = str_dup(argument);

	return TRUE;
}


NEDIT( kbedit_keyword )
{
	NOTE_DATA *note;

	EDIT_NOTE(ch, note);

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Syntax:  keywords [string]\n\r", ch );
		return FALSE;
	}

	PURGE_DATA( note->to_list );
	note->to_list = str_dup( argument );

	send_to_char( "Keywords set.\n\r", ch);
	return TRUE;
}

NEDIT( kbedit_races )
{
	NOTE_DATA *pNote;
	int value;

	if ( !IS_NULLSTR(argument) )
	{
		EDIT_NOTE( ch, pNote );

		if ( ( value = flag_value( plot_races, argument ) ) != NO_FLAG )
		{
			pNote->race ^= value;

			send_to_char( "Race toggled.\n\r", ch);
			return TRUE;
		}
	}

	send_to_char( "Syntax: race [flag]\n\r"
			"Type '? races' for a list of flags.\n\r", ch );
	return FALSE;
}

NEDIT( kbedit_successes )
{
	NOTE_DATA *pNote;

	EDIT_NOTE(ch, pNote);

	if ( IS_NULLSTR(argument) || !is_number( argument ) )
	{
		send_to_char( "Syntax:  success [number]\n\r", ch );
		return FALSE;
	}

	pNote->successes = atoi( argument );

	send_to_char( "Successes set.\n\r", ch);
	return TRUE;
}

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

#include <sys/types.h>
#include <time.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "twilight.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
#include "subcmds.h"
#include "olc.h"
#include "interp.h"
#include "grid.h"

bool timestop;
int close	args( ( int fd ) );
int flag_lookup2 (const char *name, const struct flag_type *flag_table);

SCRIPT_DATA *	get_script_index	args( (int vnum) );
char *flag_string       args ( (const struct flag_type *flag_table,int bits) );
int flag_value( const struct flag_type *flag_table, char *argument);
void fwrite_org(ORG_DATA *org);
void save_org_list();
#define MUDCMD(name) void name(CHAR_DATA *ch, char *argument)

/*
 * Local functions.
 */
ROOM_INDEX_DATA *	find_location	args( ( CHAR_DATA *ch, char *arg ) );

void do_wiznet( CHAR_DATA *ch, char *argument )
{
	char buf[MSL]={'\0'};
	int flag = 0;

	CheckCH(ch);

	if ( IS_NULLSTR(argument) )
	{
		if (IS_SET(ch->wiznet,WIZ_ON))
		{
			send_to_char("Signing off of Wiznet.\n\r",ch);
			REMOVE_BIT(ch->wiznet,WIZ_ON);
		}
		else
		{
			send_to_char("Welcome to Wiznet!\n\r",ch);
			SET_BIT(ch->wiznet,WIZ_ON);
		}
		return;
	}

	if (!str_prefix(argument,"on"))
	{
		send_to_char("Welcome to Wiznet!\n\r",ch);
		SET_BIT(ch->wiznet,WIZ_ON);
		return;
	}

	if (!str_prefix(argument,"off"))
	{
		send_to_char("Signing off of Wiznet.\n\r",ch);
		REMOVE_BIT(ch->wiznet,WIZ_ON);
		return;
	}

	/* show wiznet status */
	if (!str_prefix(argument,"status"))
	{

		send_to_char("Wiznet Status:\n\r", ch);

		if (!IS_SET(ch->wiznet,WIZ_ON))
			send_to_char("OFF\n\r", ch);

		for (flag = 0; wiznet_table[flag].name != NULL; flag++)
			if (IS_SET(ch->wiznet,wiznet_table[flag].flag))
			{
				strncat(buf,wiznet_table[flag].name, sizeof(buf) - strlen(buf) - 1);
				strncat(buf," ", sizeof(buf) - strlen(buf) - 1);
			}

		strncat(buf,"\n\r", sizeof(buf) - strlen(buf) - 1);
		send_to_char(buf,ch);
		return;
	}

	if (!str_prefix(argument,"show"))
		/* list of all wiznet options */
	{
		buf[0] = '\0';

		for (flag = 0; wiznet_table[flag].name != NULL; flag++)
		{
			if (wiznet_table[flag].level <= get_trust(ch))
			{
				strncat(buf,IS_SET(ch->wiznet,wiznet_table[flag].flag)?allcaps(wiznet_table[flag].name):wiznet_table[flag].name, sizeof(buf) - strlen(buf) - 1);
				strncat(buf," ", sizeof(buf) - strlen(buf) - 1);
			}
		}

		strncat(buf,"\n\r", sizeof(buf) - strlen(buf) - 1);

		send_to_char("Wiznet options available to you are:\n\r",ch);
		send_to_char(buf,ch);
		return;
	}

	flag = wiznet_lookup(argument);

	if (flag == -1 || get_trust(ch) < wiznet_table[flag].level)
	{
		send_to_char("No such option.\n\r",ch);
		return;
	}

	if (IS_SET(ch->wiznet,wiznet_table[flag].flag))
	{
		send_to_char(Format("You will no longer see %s on wiznet.\n\r", wiznet_table[flag].name), ch);
		REMOVE_BIT(ch->wiznet,wiznet_table[flag].flag);
		return;
	}
	else
	{
		send_to_char(Format("You will now see %s on wiznet.\n\r", wiznet_table[flag].name),ch);
		SET_BIT(ch->wiznet,wiznet_table[flag].flag);
		return;
	}

}

void wiznet(char *string, CHAR_DATA *ch, OBJ_DATA *obj, long flag, long flag_skip, int min_level)
{
	DESCRIPTOR_DATA *d;

	for ( d = descriptor_list; d != NULL; d = d->next )
	{
		if(d->character ==NULL)
			continue;
		
		if (d->connected == CON_PLAYING
				&&  IS_ADMIN(d->character)
		&&  IS_SET(d->character->wiznet,WIZ_ON)
		&&  (!flag || IS_SET(d->character->wiznet,flag))
		&&  (!flag_skip || !IS_SET(d->character->wiznet,flag_skip))
		&&  get_trust(d->character) >= min_level
		&&  d->character != ch)
		{
			if (IS_SET(d->character->wiznet,WIZ_PREFIX))
				send_to_char("--> ",d->character);
			act_new(string,d->character,obj,ch,TO_CHAR,P_DEAD,1);
		}
	}

	return;
}

/* equips a character */
void do_outfit ( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj;
	/*int i,sn,vnum;*/

	CheckCH(ch);

	obj = create_object(get_obj_index(OBJ_VNUM_MAP));
	obj_to_char(obj, ch);
	obj = create_object(get_obj_index(50));
	obj_to_char(obj, ch);

	if(ch->sex == SEX_MALE)
	{
		obj = create_object(get_obj_index(OBJ_VNUM_MALE1));
		obj_to_char(obj, ch);
		obj = create_object(get_obj_index(OBJ_VNUM_MALE2));
		obj_to_char(obj, ch);
		obj = create_object(get_obj_index(OBJ_VNUM_MALE3));
		obj_to_char(obj, ch);
		obj = create_object(get_obj_index(OBJ_VNUM_MALE4));
		obj_to_char(obj, ch);
		obj = create_object(get_obj_index(OBJ_VNUM_MALE5));
		obj_to_char(obj, ch);
		obj = create_object(get_obj_index(OBJ_VNUM_MALE6));
		obj_to_char(obj, ch);
	}
	if(ch->sex == SEX_FEMALE)
	{
		obj = create_object(get_obj_index(OBJ_VNUM_FEMALE1));
		obj_to_char(obj, ch);
		obj = create_object(get_obj_index(OBJ_VNUM_FEMALE2));
		obj_to_char(obj, ch);
		obj = create_object(get_obj_index(OBJ_VNUM_FEMALE3));
		obj_to_char(obj, ch);
		obj = create_object(get_obj_index(OBJ_VNUM_FEMALE4));
		obj_to_char(obj, ch);
	}

	return;
}


/* RT nochannels command, for those spammers */
void do_nochannels( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	char arg[MIL]={'\0'};

	CheckCH(ch);

	one_argument( argument, arg );

	if ( IS_NULLSTR(arg) )
	{
		send_to_char( "Nochannel whom?", ch );
		return;
	}

	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	if ( get_trust( victim ) >= get_trust( ch ) )
	{
		send_to_char( "You failed.\n\r", ch );
		return;
	}

	if ( IS_SET(victim->comm, COMM_NOCHANNELS) )
	{
		REMOVE_BIT(victim->comm, COMM_NOCHANNELS);
		send_to_char( "The gods have restored your channel priviliges.\n\r", victim );
		send_to_char( "NOCHANNELS removed.\n\r", ch );
		wiznet((char *)Format("\tY[WIZNET]\tn $N restores channels to %s",victim->name),ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
	}
	else
	{
		SET_BIT(victim->comm, COMM_NOCHANNELS);
		send_to_char( "You no longer have access to channels.\n\r", victim );
		send_to_char( "NOCHANNELS set.\n\r", ch );
		wiznet((char *)Format("\tY[WIZNET]\tn $N revokes %s's channels.",victim->name),ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
	}

	return;
}


/* RT dice loading command, for those times when you just need an edge */
void do_loaddice( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	char arg[MIL]={'\0'};
	int pos = 0;

	CheckCH(ch);

	argument = one_argument( argument, arg );

	if ( IS_NULLSTR(arg) )
	{
		send_to_char( "Whose dice are you loading?", ch );
		return;
	}

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Are you loading to the positive or negative?", ch );
		return;
	}

	if ( !str_prefix( argument, "positive" ) )  
		pos = LOADED_DICE_POS;
	else if ( !str_prefix( argument, "negative" ) ) 
		pos = LOADED_DICE_NEG;
	if ( !str_prefix( argument, "bigpositive" ) )  
		pos = LOADED_DICE_BPOS;
	else if ( !str_prefix( argument, "bignegative" ) ) 
		pos = LOADED_DICE_BNEG;
	else if ( !str_prefix( argument, "none" ) || !str_prefix( argument, "remove" ) )
		pos = 0;
	else
	{
		send_to_char( "Are you loading to the positive or negative?", ch );
		return;
	}

	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	if ( get_trust( victim ) >= get_trust( ch ) && ch != victim )
	{
		send_to_char( "You failed.\n\r", ch );
		return;
	}

	if ( pos == 0 )
	{
		REMOVE_BIT(victim->off_flags, LOADED_DICE_POS);
		REMOVE_BIT(victim->off_flags, LOADED_DICE_NEG);
		REMOVE_BIT(victim->off_flags, LOADED_DICE_BPOS);
		REMOVE_BIT(victim->off_flags, LOADED_DICE_BNEG);
		send_to_char( "Dice load flags removed.\n\r", ch );
		wiznet((char *)Format("\tY[WIZNET]\tn $N unloads the dice of %s",victim->name),ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
	}

	if ( IS_SET(victim->off_flags, pos) )
	{
		REMOVE_BIT(victim->off_flags, pos);
		send_to_char( "Dice load flag removed.\n\r", ch );
		wiznet((char *)Format("\tY[WIZNET]\tn $N unloads the dice of %s",victim->name),ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
	}
	else
	{
		SET_BIT(victim->off_flags, pos);
		send_to_char( "Dice load flag set.\n\r", ch );
		if(pos == LOADED_DICE_POS)
		{
			wiznet((char *)Format("\tY[WIZNET]\tn $N positively loads %s's dice.",victim->name),ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
		}
		else if(pos == LOADED_DICE_BPOS)
		{
			wiznet((char *)Format("\tY[WIZNET]\tn $N really positively loads %s's dice.",victim->name),ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
		}
		else if(pos == LOADED_DICE_NEG)
		{
			wiznet((char *)Format("\tY[WIZNET]\tn $N negatively loads %s's dice.",victim->name),ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
		}
		else
		{
			wiznet((char *)Format("\tY[WIZNET]\tn $N really negatively loads %s's dice.",victim->name),ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
		}
	}

	return;
}


/* RT nogiftxp command, for those cheating up their xp */
void do_nogiftxp( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	char arg[MIL]={'\0'};

	CheckCH(ch);

	one_argument( argument, arg );

	if ( IS_NULLSTR(arg) )
	{
		send_to_char( "Nogiftxp whom?", ch );
		return;
	}

	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	if ( get_trust( victim ) >= get_trust( ch ) )
	{
		send_to_char( "You failed.\n\r", ch );
		return;
	}

	if ( IS_SET(victim->act2, ACT2_NOGIFT) )
	{
		REMOVE_BIT(victim->act2, ACT2_NOGIFT);
		send_to_char( "You may now once again share roleplaying xp.\n\r", victim );
		send_to_char( "NOGIFTXP removed.\n\r", ch );
		wiznet((char *)Format("\tY[WIZNET]\tn $N allows %s to join in gift xp again.",victim->name),ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
	}
	else
	{
		SET_BIT(victim->act2, ACT2_NOGIFT);
		send_to_char( "You may no longer share roleplaying xp.\n\r", victim );
		send_to_char( "NOGIFTXP set.\n\r", ch );
		wiznet((char *)Format("\tY[WIZNET]\tn $N revokes %s's ability to share gift xp.",victim->name),ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
	}

	return;
}


void do_bamfin( CHAR_DATA *ch, char *argument )
{
	CheckCH(ch);

	if ( !IS_NPC(ch) )
	{
		smash_tilde( argument );

		if (IS_NULLSTR(argument))
		{
			send_to_char(Format("You enter with: %s\n\r",ch->pcdata->bamfin),ch);
			return;
		}

		if ( strstr(argument,ch->name) == NULL)
		{
			send_to_char("You must include your name.\n\r",ch);
			return;
		}

		PURGE_DATA( ch->pcdata->bamfin );
		ch->pcdata->bamfin = str_dup( argument );

		send_to_char(Format("You now enter with: %s\n\r",ch->pcdata->bamfin),ch);
	}
	return;
}


void do_bamfout( CHAR_DATA *ch, char *argument )
{
	CheckCH(ch);

	if ( !IS_NPC(ch) )
	{
		smash_tilde( argument );

		if (IS_NULLSTR(argument))
		{
			send_to_char(Format("You exit with: %s\n\r",ch->pcdata->bamfout),ch);
			return;
		}

		if ( strstr(argument,ch->name) == NULL)
		{
			send_to_char("You must include your name.\n\r",ch);
			return;
		}

		PURGE_DATA( ch->pcdata->bamfout );
		ch->pcdata->bamfout = str_dup( argument );

		send_to_char(Format("You now exit with: %s\n\r",ch->pcdata->bamfout),ch);
	}
	return;
}


void do_deny( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	char arg[MIL]={'\0'};

	CheckCH(ch);

	one_argument( argument, arg );
	if ( IS_NULLSTR(arg) )
	{
		send_to_char( "Deny whom?\n\r", ch );
		return;
	}

	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	if ( IS_NPC(victim) )
	{
		send_to_char( "Not on NPC's.\n\r", ch );
		return;
	}

	if ( get_trust( victim ) >= get_trust( ch ) )
	{
		send_to_char( "You failed.\n\r", ch );
		return;
	}

	SET_BIT(victim->plr_flags, PLR_DENY);
	send_to_char( "You are denied access!\n\r", victim );
	wiznet((char *)Format("\tY[WIZNET]\tn $N denies access to %s",victim->name), ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
	send_to_char( "OK.\n\r", ch );
	save_char_obj(victim);
	stop_fighting(victim,TRUE);
	do_function(victim, &do_quit, "" );

	return;
}


void do_disconnect( CHAR_DATA *ch, char *argument )
{
	DESCRIPTOR_DATA *d;
	CHAR_DATA *victim;
	char arg[MIL]={'\0'};

	CheckCH(ch);

	one_argument( argument, arg );
	if ( IS_NULLSTR(arg) )
	{
		send_to_char( "Disconnect whom?\n\r", ch );
		return;
	}

	if (is_number(arg))
	{
		int desc = 0;

		desc = atoi(arg);
		for ( d = descriptor_list; d != NULL; d = d->next )
		{
			if ( d->descriptor == desc )
			{
				if(d->character->trust < ch->trust)
				{
					close_socket( d );
					send_to_char( "Ok.\n\r", ch );
					return;
				}
				else
				{
					send_to_char("Not to people with the same or more authority.\n\r", ch);
					return;
				}
			}
		}
	}

	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	if( victim->trust >= ch->trust )
	{
		send_to_char("Not to people with the same or more authority.\n\r", ch);
		return;
	}

	if ( victim->desc == NULL )
	{
		act( "$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR, 1 );
		return;
	}

	for ( d = descriptor_list; d != NULL; d = d->next )
	{
		if ( d == victim->desc )
		{
			close_socket( d );
			send_to_char( "Ok.\n\r", ch );
			return;
		}
	}

	log_string(LOG_BUG, "Do_disconnect: desc not found.");
	send_to_char( "Descriptor not found!\n\r", ch );
	return;
}

void do_echo( CHAR_DATA *ch, char *argument )
{
	DESCRIPTOR_DATA *d;
	int trust = 7;

	// CheckCH(ch);

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Global echo what?\n\r", ch );
		return;
	}

	if(ch)
		trust = get_trust(ch);

	for ( d = descriptor_list; d; d = d->next )
	{
		if ( d->connected == CON_PLAYING )
		{
			if (get_trust(d->character) >= trust)
				send_to_char( "> ",d->character);
			send_to_char( argument, d->character );
			send_to_char( "\n\r",   d->character );
		}
	}

	return;
}



void do_recho( CHAR_DATA *ch, char *argument )
{
	DESCRIPTOR_DATA *d;

	CheckCH(ch);

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Local echo what?\n\r", ch );

		return;
	}

	for ( d = descriptor_list; d; d = d->next )
	{
		if ( d->connected == CON_PLAYING
				&&   d->character->in_room == ch->in_room )
		{
			if (get_trust(d->character) >= get_trust(ch))
				send_to_char( "local> ",d->character);
			send_to_char( argument, d->character );
			send_to_char( "\n\r",   d->character );
		}
	}

	return;
}

void do_pecho( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	char arg[MIL]={'\0'};

	CheckCH(ch);

	argument = one_argument(argument, arg);

	if ( IS_NULLSTR(argument) || IS_NULLSTR(arg) )
	{
		send_to_char("Personal echo what?\n\r", ch);
		return;
	}

	if  ( (victim = get_char_world(ch, arg) ) == NULL )
	{
		send_to_char("Target not found.\n\r",ch);
		return;
	}

	if (get_trust(victim) >= get_trust(ch) && get_trust(ch) != MAX_LEVEL)
		send_to_char( "personal> ",victim);

	send_to_char(argument,victim);
	send_to_char("\n\r",victim);
	send_to_char( "personal> ",ch);
	send_to_char(argument,ch);
	send_to_char("\n\r",ch);
}


ROOM_INDEX_DATA *find_location( CHAR_DATA *ch, char *arg )
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;

	if ( is_number(arg) )
		return get_room_index( atoi( arg ) );

	if ( ( victim = get_char_world( ch, arg ) ) != NULL && IS_ADMIN(ch) )
		return victim->in_room;

	if ( victim != NULL && !IS_NPC(victim) && !IS_BLOCKED(ch, victim) )
		return victim->in_room;

	if ( ( obj = get_obj_world( ch, arg ) ) != NULL )
		return obj->in_room;

	return NULL;
}



void do_transfer( CHAR_DATA *ch, char *argument )
{
	ROOM_INDEX_DATA *location;
	DESCRIPTOR_DATA *d;
	CHAR_DATA *victim;
	char arg1[MIL]={'\0'};
	char arg2[MIL]={'\0'};

	CheckCH(ch);

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( IS_NULLSTR(arg1) )
	{
		send_to_char( "Transfer whom (and where)?\n\r", ch );
		return;
	}

	if ( !str_cmp( arg1, "all" ) )
	{
		for ( d = descriptor_list; d != NULL; d = d->next )
		{
			if ( d->connected == CON_PLAYING
					&&   d->character != ch
					&&   d->character->in_room != NULL
					&&   can_see( ch, d->character ) )
			{
				do_function(ch, &do_transfer, (char *)Format("%s %s", d->character->name, arg2) );
			}
		}
		return;
	}

	/*
	 * Thanks to Grodyn for the optional location parameter.
	 */
	if ( IS_NULLSTR(arg2) )
	{
		location = ch->in_room;
	}
	else
	{
		if ( ( location = find_location( ch, arg2 ) ) == NULL )
		{
			send_to_char( "No such location.\n\r", ch );
			return;
		}

		if ( !is_room_owner(ch,location) && room_is_private( location )
				&&  get_trust(ch) < MAX_LEVEL)
		{
			send_to_char( "That room is private right now.\n\r", ch );
			return;
		}
	}

	if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	if ( victim->in_room == NULL )
	{
		send_to_char( "They are in limbo.\n\r", ch );
		return;
	}

	if ( victim->fighting != NULL )
		stop_fighting( victim, TRUE );
	act( "$n disappears in a mushroom cloud.", victim, NULL, NULL, TO_ROOM, 0 );
	char_from_room( victim );
	char_to_room( victim, location );
	act( "$n arrives from a puff of smoke.", victim, NULL, NULL, TO_ROOM, 0 );
	if ( ch != victim )
		act( "$n has transferred you.", ch, NULL, victim, TO_VICT, 1 );
	do_function(victim, &do_look, "auto" );
	send_to_char( "Ok.\n\r", ch );
}


void do_at( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    OBJ_DATA *on;
    CHAR_DATA *wch;
    char arg[MIL]={'\0'};

    CheckCH(ch);

    argument = one_argument( argument, arg );

    if ( IS_NULLSTR(arg) || IS_NULLSTR(argument) )
    {
	send_to_char( "At where what?\n\r", ch );
	return;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if (!is_room_owner(ch,location) && room_is_private( location )
    &&  get_trust(ch) < MAX_LEVEL)
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }

    original = ch->in_room;
    on = ch->on;
    char_from_room( ch );
    char_to_room( ch, location );
    interpret( ch, argument );

    /*
     * See if 'ch' still exists before continuing!
     * Handles 'at XXXX quit' case.
     */
    for ( wch = char_list; wch != NULL; wch = wch->next )
    {
	if ( wch == ch )
	{
	    char_from_room( ch );
	    char_to_room( ch, original );
	    ch->on = on;
	    break;
	}
    }

    return;
}


void do_goto( CHAR_DATA *ch, char *argument )
{
	ROOM_INDEX_DATA *location;
	CHAR_DATA *rch;
	int count = 0;

	CheckCH(ch);

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Goto where?\n\r", ch );
		return;
	}

	if ( ( location = find_location( ch, argument ) ) == NULL )
	{
		send_to_char( "No such location.\n\r", ch );
		return;
	}

	count = 0;
	for ( rch = location->people; rch != NULL; rch = rch->next_in_room )
		count++;

	if (!is_room_owner(ch,location) && room_is_private(location)
			&&  (count > 1 || !IS_TRUSTED(ch,STORYTELLER)))
	{
		send_to_char( "That room is private right now.\n\r", ch );
		return;
	}

	if ( ch->fighting != NULL )
		stop_fighting( ch, TRUE );

	for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
	{
		if (get_trust(rch) >= ch->invis_level)
		{
			if (ch->pcdata != NULL && !IS_NULLSTR(ch->pcdata->bamfout))
				act("$t",ch,ch->pcdata->bamfout,rch,TO_VICT,0);
			else
				act("$n leaves in a swirling mist.",ch,NULL,rch,TO_VICT,0);
		}
	}

	char_from_room( ch );
	char_to_room( ch, location );

	for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
	{
		if (get_trust(rch) >= ch->invis_level)
		{
			if (ch->pcdata != NULL && !IS_NULLSTR(ch->pcdata->bamfin))
				act("$t",ch,ch->pcdata->bamfin,rch,TO_VICT,0);
			else
				act("$n appears in a swirling mist.",ch,NULL,rch,TO_VICT,0);
		}
	}

	do_function(ch, &do_look, "auto" );
	return;
}

void do_rstat( CHAR_DATA *ch, char *argument )
{
	ROOM_INDEX_DATA *location;
	OBJ_DATA *obj;
	CHAR_DATA *rch;
	char buf[MSL]={'\0'};
	char arg[MIL]={'\0'};
	int door;

	CheckCH(ch);

	one_argument( argument, arg );
	location = ( IS_NULLSTR(arg) ) ? ch->in_room : find_location( ch, arg );
	if ( location == NULL )
	{
		send_to_char( "No such location.\n\r", ch );
		return;
	}

	if (!is_room_owner(ch,location) && ch->in_room != location &&  room_is_private( location ) && !IS_TRUSTED(ch,MASTER))
	{
		send_to_char( "That room is private right now.\n\r", ch );
		return;
	}

	send_to_char( Format("Name: '%s'\n\rArea: '%s'\n\r", location->name, location->area->name), ch );
	send_to_char( Format("Vnum: %d  Sector: %d  Light: %d  Healing: %d  Mana: %d\n\r", location->vnum, location->sector_type, location->light, location->heal_rate, location->mana_rate), ch );
	send_to_char( Format("Room flags: %d.\n\rDescription:\n\r%s", location->room_flags, location->description), ch );

	if ( location->extra_descr != NULL )
	{
		EXTRA_DESCR_DATA *ed;

		send_to_char( "Extra description keywords: '", ch );
		for ( ed = location->extra_descr; ed; ed = ed->next )
		{
			send_to_char( ed->keyword, ch );
			if ( ed->next != NULL )
				send_to_char( " ", ch );
		}
		send_to_char( "'.\n\r", ch );
	}

	send_to_char( "Characters:", ch );
	for ( rch = location->people; rch; rch = rch->next_in_room )
	{
		if (can_see(ch,rch))
		{
			send_to_char( " ", ch );
			one_argument( rch->name, buf );
			send_to_char( buf, ch );
		}
	}

	send_to_char( ".\n\rObjects:   ", ch );
	for ( obj = location->contents; obj; obj = obj->next_content )
	{
		send_to_char( " ", ch );
		one_argument( obj->name, buf );
		send_to_char( buf, ch );
	}
	send_to_char( ".\n\r", ch );

	for ( door = 0; door <= 5; door++ )
	{
		EXIT_DATA *pexit;

		if ( ( pexit = location->exit[door] ) != NULL )
		{

			send_to_char( Format("Door: %d.  ", door), ch );
			send_to_char( Format("To: %d.  ", (pexit->u1.to_room == NULL ? -1 : pexit->u1.to_room->vnum)), ch );
			send_to_char( Format("Key: %d.  ", pexit->key), ch );
			send_to_char( Format("Exit flags: %d.\n\r", pexit->exit_info), ch );
			send_to_char( Format("Keywords: %s.  ", pexit->keyword), ch );
		}
	}

	return;
}


void do_ostat( CHAR_DATA *ch, char *argument )
{
	AFFECT_DATA *paf;
	OBJ_DATA *obj;
	char arg[MIL]={'\0'};

	CheckCH(ch);

	one_argument( argument, arg );

	if ( IS_NULLSTR(arg) )
	{
		send_to_char( "Stat what?\n\r", ch );
		return;
	}

	if ( ( obj = get_obj_world( ch, argument ) ) == NULL )
	{
		send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
		return;
	}

    send_to_char( Format("Name(s): %s\n\r", obj->name), ch );
    send_to_char( Format("Vnum: %d  Type: %s  Resets: %d\n\r", obj->pIndexData->vnum, item_name(obj->item_type), obj->pIndexData->reset_num), ch );
    send_to_char( Format("Short description: %s\n\rLong description: %s\n\r", obj->short_descr, obj->description), ch );
    send_to_char( Format("Wear bits: %s\n\r", flag_string( wear_flags, obj->wear_flags )), ch );
    send_to_char( Format("Extra bits: %s\n\r", flag_string( extra_flags, obj->extra_flags )), ch );
	send_to_char( Format("Material: %s\n\r", obj->material), ch);
    send_to_char( Format("Number: %d/%d  Weight: %d/%d/%d (10th pounds)\n\r", 1, get_obj_number( obj ), obj->weight, get_obj_weight( obj ),get_true_weight(obj)), ch );
    send_to_char( Format("Cost: %d  Condition: %d  Timer: %d  Quality: %d\n\r", obj->cost, obj->condition, obj->timer, obj->quality), ch );

    send_to_char( Format("In room: %d  In object: %s  Carried by: %s  Wear_loc: %d\n\r",
	obj->in_room    == NULL    ?        0 : obj->in_room->vnum,
	obj->in_obj     == NULL    ? "(none)" : obj->in_obj->short_descr,
	obj->carried_by == NULL    ? "(none)" :  can_see(ch,obj->carried_by) ? obj->carried_by->name : "someone",
	obj->wear_loc), ch );

    send_to_char( Format("Values: %d %d %d %d %d\n\r", obj->value[0], obj->value[1], obj->value[2], obj->value[3], obj->value[4]), ch );

    /* now give out vital statistics as per identify */

    switch ( obj->item_type )
    {
    	case ITEM_SCROLL:
    	case ITEM_POTION:
    	case ITEM_PILL:
	    send_to_char( Format("Level %d spells of:", obj->value[0]), ch );

	    if ( obj->value[1] >= 0 && obj->value[1] < MAX_SKILL )
	    {
	    	send_to_char( " '", ch );
	    	send_to_char( skill_table[obj->value[1]].name, ch );
	    	send_to_char( "'", ch );
	    }

	    if ( obj->value[2] >= 0 && obj->value[2] < MAX_SKILL )
	    {
	    	send_to_char( " '", ch );
	    	send_to_char( skill_table[obj->value[2]].name, ch );
	    	send_to_char( "'", ch );
	    }

	    if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
	    {
	    	send_to_char( " '", ch );
	    	send_to_char( skill_table[obj->value[3]].name, ch );
	    	send_to_char( "'", ch );
	    }

	    if (obj->value[4] >= 0 && obj->value[4] < MAX_SKILL)
	    {
	    	send_to_char(" '",ch);
	    	send_to_char(skill_table[obj->value[4]].name,ch);
	    	send_to_char("'",ch);
	    }

	    send_to_char( ".\n\r", ch );
	break;

	case ITEM_DRINK_CON:
	    send_to_char(Format("It holds %s-colored %s.\n\r", liq_table[obj->value[2]].liq_color, liq_table[obj->value[2]].liq_name), ch);
	    break;


    	case ITEM_WEAPON:
 	    send_to_char("Weapon type is ",ch);
	    switch (obj->value[0])
	    {
	    	case(WEAPON_EXOTIC):
		    send_to_char("exotic\n\r",ch);
		    break;
	    	case(WEAPON_FIREARM):
		    send_to_char("\n\r",ch);
		    break;
	    	case(WEAPON_BLADE):
		    send_to_char("blade\n\r",ch);
		    break;
	    	case(WEAPON_BLUNT):
		    send_to_char("blunt\n\r",ch);
		    break;
	    	case(WEAPON_GRAPPLE):
		    send_to_char("grapple\n\r",ch);
		    break;
	    	case(WEAPON_WHIP):
		    send_to_char("whip\n\r",ch);
		    break;
	    	default:
		    send_to_char("unknown\n\r",ch);
		    break;
 	    }

	    send_to_char( Format("Difficulty is %d Damage is at %d\n\r", obj->value[1],obj->value[2]), ch );

	    send_to_char(Format("Damage noun is %s.\n\r",
		(obj->value[3] > 0 && obj->value[3] < MAX_DAMAGE_MESSAGE) ? attack_table[obj->value[3]].noun : "undefined"), ch);

	    if (obj->value[4])  /* weapon flags */
	    {
	    	send_to_char(Format("Weapons flags: %s\n\r", flag_string(weapon_flags, obj->value[4])), ch);
	    }
	    break;

    	case ITEM_ARMOR:
	    send_to_char( Format("Armor class is %d pierce, %d bash, %d slash, and %d vs. magic\n\r", obj->value[0], obj->value[1], obj->value[2], obj->value[3]), ch );
	break;

        case ITEM_CONTAINER:
            send_to_char(Format("Capacity: %d#  Maximum weight: %d#  flags: %s\n\r", obj->value[0], obj->value[3], flag_string(container_flags, obj->value[1])), ch);
            if (obj->value[4] != 100)
            {
                send_to_char(Format("Weight multiplier: %d%%\n\r", obj->value[4]), ch);
            }
        break;
    }


    if ( obj->extra_descr != NULL || obj->pIndexData->extra_descr != NULL )
    {
	EXTRA_DESCR_DATA *ed;

	send_to_char( "Extra description keywords: '", ch );

	for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL )
	    	send_to_char( " ", ch );
	}

	for ( ed = obj->pIndexData->extra_descr; ed != NULL; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL )
		send_to_char( " ", ch );
	}

	send_to_char( "'\n\r", ch );
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
    	send_to_char(Format("Affects %s by %d, level %d", flag_string( apply_flags, paf->location ), paf->modifier,paf->level), ch);
    	if ( paf->duration > -1)
    	{
    		send_to_char( Format(", %d hours.\n\r", paf->duration), ch );
    	}
    	else
    	{
    		send_to_char( Format(".\n\r"), ch );
    	}

    	if (paf->bitvector)
    	{
    		switch(paf->where)
    		{
    		case TO_AFFECTS:
    			send_to_char(Format("Adds %s affect.\n", flag_string(affect_flags, paf->bitvector)), ch);
    			break;
    		case TO_WEAPON:
    			send_to_char(Format("Adds %s weapon flags.\n", flag_string(weapon_flags, paf->bitvector)), ch);
    			break;
    		case TO_OBJECT:
    			send_to_char(Format("Adds %s object flag.\n", flag_string( extra_flags, paf->bitvector )), ch);
    			break;
    		case TO_IMMUNE:
    			send_to_char(Format("Adds immunity to %s.\n", flag_string(imm_flags, paf->bitvector)), ch);
    			break;
    		case TO_RESIST:
    			send_to_char(Format("Adds resistance to %s.\n\r", flag_string(imm_flags, paf->bitvector)), ch);
    			break;
    		case TO_VULN:
    			send_to_char(Format("Adds vulnerability to %s.\n\r", flag_string(imm_flags, paf->bitvector)), ch);
    			break;
    		default:
    			send_to_char(Format("Unknown bit %d: %d\n\r", paf->where,paf->bitvector), ch);
    			break;
    		}
    	}
    }

    if (!obj->enchanted)
    	for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    	{
    		send_to_char( Format("Affects %s by %d, level %d.\n\r", flag_string( apply_flags, paf->location ), paf->modifier,paf->level), ch );
    		if (paf->bitvector)
    		{
    			switch(paf->where)
    			{
    			case TO_AFFECTS:
    				send_to_char(Format("Adds %s affect.\n", flag_string(affect_flags, paf->bitvector)), ch);
    				break;
    			case TO_OBJECT:
    				send_to_char(Format("Adds %s object flag.\n", flag_string( extra_flags, paf->bitvector )), ch);
    				break;
    			case TO_IMMUNE:
    				send_to_char(Format("Adds immunity to %s.\n", flag_string(imm_flags, paf->bitvector)), ch);
    				break;
    			case TO_RESIST:
    				send_to_char(Format("Adds resistance to %s.\n\r", flag_string(imm_flags, paf->bitvector)), ch);
    				break;
    			case TO_VULN:
    				send_to_char(Format("Adds vulnerability to %s.\n\r", flag_string(imm_flags, paf->bitvector)), ch);
    				break;
    			default:
    				send_to_char(Format("Unknown bit %d: %d\n\r", paf->where,paf->bitvector), ch);
    				break;
    			}
    		}
    	}

    return;
}


void do_mstat( CHAR_DATA *ch, char *argument )
{
	AFFECT_DATA *paf;
	CHAR_DATA *victim;
	char arg[MIL]={'\0'};

	CheckCH(ch);

	one_argument( argument, arg );

	if ( IS_NULLSTR(arg) )
	{
		send_to_char( "Stat whom?\n\r", ch );
		return;
	}

	if ( ( victim = get_char_world( ch, argument ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	send_to_char( Format("Name: %s\n\r", victim->name), ch );

	if(IS_ADMIN(victim))
	{
		send_to_char( Format("Administrator: %s\n\r", IS_ADMIN(victim) ? "Yes": "No"), ch);
		send_to_char(Format("Admin Level: %s\n\r", staff_status[victim->trust].name), ch);
	}

	send_to_char( Format("Vnum: %d    Room: %d\n\r", IS_NPC(victim) ? victim->pIndexData->vnum : 0, victim->in_room == NULL ? 0 : victim->in_room->vnum), ch);
	send_to_char( Format("Race: %s   Sex: %s\n\r", race_table[victim->race].name, sex_table[victim->sex].name), ch);
	send_to_char( Format("Group: %d\n\r", IS_NPC(victim) ? victim->group : 0), ch);

	send_to_char( Format("Str: %d  Dex: %d  Sta: %d\n\r",
			get_curr_stat(victim,STAT_STR), get_curr_stat(victim,STAT_DEX), get_curr_stat(victim,STAT_STA)), ch );

	send_to_char( Format("Cha: %d  Man: %d  App: %d\n\r",
			get_curr_stat(victim,STAT_CHA), get_curr_stat(victim,STAT_MAN), get_curr_stat(victim,STAT_APP)), ch );

	send_to_char( Format("Per: %d  Int: %d  Wit: %d\n\r",
			get_curr_stat(victim,STAT_PER), get_curr_stat(victim,STAT_INT), get_curr_stat(victim,STAT_WIT)), ch );

	send_to_char( Format("Health: %s\n\r", health_string(victim)), ch );

	send_to_char( Format("Clan: %s  Dollars: %d  Cents: %d  Exp: %d\n\r",
			IS_NPC(victim) ? "mobile" : clan_table[victim->clan].name, victim->dollars, victim->cents, victim->exp), ch );

	send_to_char( Format("Saves: %d  Size: %s  Position: %s\n\r",
			victim->saving_throw, size_table[victim->size].name, position_table[victim->position].name), ch );

	send_to_char( Format("Fighting: %s\n\r", victim->fighting ? victim->fighting->name : "(none)"), ch );

	if (IS_NPC(victim))
	{
		send_to_char(Format("Damage: %dd%d  Message:  %s\n\r",
				victim->damage[DICE_NUMBER],victim->damage[DICE_TYPE], attack_table[victim->dam_type].noun), ch);
	}

	if (IS_NPC(victim))
	{
		send_to_char(Format("Count: %d  Killed: %d\n\r", victim->pIndexData->count,victim->pIndexData->killed), ch);
	}

	if ( !IS_NPC(victim) )
	{
		send_to_char( Format("Thirst: %d  Hunger: %d  Full: %d  Drunk: %d  High: %d  Trip: %d\n\r",
				victim->condition[COND_THIRST],
				victim->condition[COND_HUNGER],
				victim->condition[COND_FULL],
				victim->condition[COND_DRUNK],
				victim->condition[COND_HIGH],
				victim->condition[COND_TRIPPING]), ch );
	}

	send_to_char( Format("Carry number: %d  Carry weight: %d\n\r", victim->carry_number, get_carry_weight(victim) / 10), ch );


	if (!IS_NPC(victim))
	{
		send_to_char( Format("Age: %d  Played: %d  Timer: %d\n\r", get_age(victim), (int) (victim->played + current_time - victim->logon) / 3600, victim->timer), ch );
		send_to_char(Format("Influence Timer: %d\r\n", victim->infl_timer), ch);
		send_to_char(Format("Herd Timer: %d\r\n", victim->herd_timer), ch);
		send_to_char(Format("Power Timer: %d\r\n", victim->power_timer), ch);
	}

	send_to_char(Format("Act: %s\n\r",flag_string(act_flags, victim->act)), ch);
	send_to_char(Format("Act2: %s\n\r",flag_string(act2_flags, victim->act2)), ch);


	if (victim->comm)
	{
		send_to_char(Format("Comm: %s\n\r",flag_string(comm_flags, victim->comm)),ch);
	}

	if (IS_NPC(victim) && victim->off_flags)
	{
		send_to_char(Format("Offense: %s\n\r", flag_string(off_flags, victim->off_flags)),ch);
	}

	if (victim->imm_flags)
	{
		send_to_char(Format("Immune: %s\n\r", flag_string(imm_flags, victim->imm_flags)),ch);
	}

	if (victim->res_flags)
	{
		send_to_char(Format("Resist: %s\n\r", flag_string(imm_flags, victim->res_flags)),ch);
	}

	if (victim->vuln_flags)
	{
		send_to_char(Format("Vulnerable: %s\n\r", flag_string(imm_flags, victim->vuln_flags)),ch);
	}

	send_to_char(Format("Form: %s\r\n", flag_string(form_flags, victim->form)), ch);
	send_to_char(Format("Parts: %s\r\n", flag_string(part_flags, victim->parts)), ch);
	send_to_char(Format("Material: %s\r\n", victim->material), ch);

	if (victim->affected_by)
	{
		send_to_char(Format("Affected by %s\n\r", flag_string(affect_flags, victim->affected_by)), ch);
	}

	if (victim->affected_by2)
	{
		send_to_char(Format("Affected by %s\n\r", flag_string(affect_flags2, victim->affected_by2)), ch);
	}

	send_to_char( Format("Master: %s  Leader: %s  Pet: %s\n\r",
			victim->master      ? victim->master->name   : "(none)",
			victim->leader      ? victim->leader->name   : "(none)",
			victim->pet 	    ? victim->pet->name	     : "(none)"), ch );

	if (!IS_NPC(victim))
	{
		send_to_char( Format("Security: %d.\n\r", victim->pcdata->security), ch );
	}

	send_to_char( Format("Short description: %s\n\r", victim->short_descr), ch );
	send_to_char( Format("Long description: %s", !IS_NULLSTR(victim->long_descr) ? victim->long_descr : "(none)\n\r"), ch);

	for ( paf = victim->affected; paf != NULL; paf = paf->next )
	{
		send_to_char( Format("Spell: '%s' modifies %s by %d for %d hours with bits %s, level %d.\n\r",
				paf->type>=0?skill_table[(int) paf->type].name:"Unknown", flag_string( apply_flags, paf->location ),
						paf->modifier, paf->duration, flag_string( affect_flags, paf->bitvector ), paf->level), ch );
	}

	return;
}

/* ofind and mfind replaced with vnum, vnum skill also added */
void do_vnum(CHAR_DATA *ch, char *argument)
{
	char arg[MIL]={'\0'};
	char *string;

	CheckCH(ch);

	string = one_argument(argument,arg);

	if (IS_NULLSTR(arg))
	{
		send_to_char("Syntax:\n\r",ch);
		send_to_char("  vnum obj [name]\n\r",ch);
		send_to_char("  vnum mob [name]\n\r",ch);
		send_to_char("  vnum skill [skill or spell]\n\r",ch);
		return;
	}

	if (!str_cmp(arg,"obj"))
	{
		do_function(ch, &do_ofind, string );
		return;
	}

	if (!str_cmp(arg,"mob") || !str_cmp(arg,"char"))
	{
		return;
	}

	if (!str_cmp(arg,"skill") || !str_cmp(arg,"spell"))
	{
		do_function(ch, &do_slookup, string );
		return;
	}
	/* do both */
	do_function(ch, &do_mfind, argument );
	do_function(ch, &do_ofind, argument );
}


/*
 * Combination of functionality from old ofind/mfind and OLC redit_olist/mlist
 */
void do_mfind(CHAR_DATA *ch, char *argument)
{
    extern int top_mob_index;
    extern int newmobs;
    MOB_INDEX_DATA      *pMobIndex;
    BUFFER              *buf1;
    char                arg  [ MIL ]={'\0'};
    int vnum;
    int col = 0;
    int nMatch;
    bool fAll, found;

    CheckCH(ch);

    one_argument( argument, arg );
    if ( IS_NULLSTR(arg) )
    {
        send_to_char( "\tCSyntax:  mfind [all/name]\tn\n\r", ch );
        return;
    }

    buf1=new_buf();
/*    buf1[0] = '\0'; */
    fAll    = !str_cmp( arg, "all" );
    found   = FALSE;
    nMatch      = 0;

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_mob_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
    for ( vnum = 0; nMatch < newmobs; vnum++ )
    {
        if ( ( pMobIndex = get_mob_index( vnum ) ) != NULL )
        {
            nMatch++;
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
        send_to_char( "No mobile(s) found.\n\r", ch);
        return;
    }

    if ( col % 3 != 0 )
        add_buf( buf1, "\n\r" );

    page_to_char( buf_string(buf1), ch );
    free_buf(buf1);
    return;
}


void do_ofind(CHAR_DATA *ch, char *argument)
{
	// extern int top_obj_index;
	extern int newobjs;
	OBJ_INDEX_DATA      *pObjIndex;
	BUFFER              *buf1;
	char                arg  [ MIL ]={'\0'};
	bool fAll;
	bool fBigger;
	bool found = FALSE;
	long largest = 0;
	int vnum = 0;
	int lsize = 0;
	// int col = 0;
	int nMatch = 0;
	int count = 0;

	CheckCH(ch);

	// log_string( LOG_COMMAND, Format("TRACKING 1 - do_ofind argument %s", argument));
	one_argument( argument, arg );
	// log_string( LOG_COMMAND, Format("TRACKING 2 - do_ofind arg %s", arg));

	if ( IS_NULLSTR(arg) )
	{
		send_to_char("\tCSyntax:  ofind [all/name/item_type/item_size/oversize]\tn\n\r", ch );
		return;
	}

	buf1 = new_buf();
	fAll    = !str_cmp( arg, "all" );
	fBigger = !str_cmp( arg, "oversize" );
	// log_string( LOG_COMMAND, Format("TRACKING 3: fAll = %s. fBigger = %s.", 
	// 	fAll ? "TRUE" : "FALSE",
	// 	fBigger ? "TRUE" : "FALSE"));


	/*
	 * Yeah, so iterating over all vnum's takes 10,000 loops.
	 * Get_obj_index is fast, and I don't feel like threading another link.
	 * Do you?
	 * -- Furey
	 */
	// for ( vnum = 0; nMatch < top_obj_index; vnum++ )
	for ( vnum = 0; nMatch < newobjs; vnum++ )
	{
		// log_string( LOG_COMMAND, Format("OFIND TRACKING A: fAll = %s. fBigger = %s.", 
		// 	fAll ? "TRUE" : "FALSE",
		// 	fBigger ? "TRUE" : "FALSE"));
		if ( ( pObjIndex = get_obj_index( vnum ) ) )
		{
			// log_string( LOG_COMMAND, Format("OFIND TRACKING B: fAll = %s. fBigger = %s.", 
			// 	fAll ? "TRUE" : "FALSE",
			// 	fBigger ? "TRUE" : "FALSE"));
			nMatch++;
			if ( fAll || is_name( arg, pObjIndex->name )
				|| flag_value( type_flags, arg ) == pObjIndex->item_type
				|| flag_value( size_flags, arg ) == pObjIndex->weight
				|| (fBigger && pObjIndex->weight > MAX_SIZE) )
			{
				found = TRUE;
				// log_string( LOG_COMMAND, Format("TRACKING # - FOUND do_ofind arg %s", arg));
				add_buf( buf1, (char *)Format("[%5d] %-s\n\r", pObjIndex->vnum, capitalize( pObjIndex->short_descr )) );
				count++;
				
				// if ( ++col % 3 == 0 )
				// {
				// 	add_buf( buf1, "\n\r" );
				// }
				
				if(pObjIndex->weight > lsize) 
				{
					lsize = pObjIndex->weight;
					largest = pObjIndex->vnum;
				}
			}
		}
	}

	if ( !found )
	{
		send_to_char( "No object(s) found.\n\r", ch);
		return;
	}

	// if ( col % 3 != 0 )
	// {
	// 	add_buf( buf1, "\n\r" );
	// }

	if ( fBigger )
	{
		add_buf( buf1, (char *)Format("The largest object is %ld at size %d.\n\r", largest, lsize) );
	}

	add_buf(buf1, (char *)Format("\tJNumber of Objects Found: %d\tn.\n\r", count));
	page_to_char( buf_string(buf1), ch );
	free_buf(buf1);
	return;
}


void do_owhere(CHAR_DATA *ch, char *argument )
{
	BUFFER *buffer;
	OBJ_DATA *obj;
	OBJ_DATA *in_obj;
	bool found = FALSE;
	int number = 0;
	int max_found = 200;

	CheckCH(ch);

	buffer = new_buf();

	if (IS_NULLSTR(argument))
	{
		send_to_char("Find what?\n\r",ch);
		return;
	}

	for ( obj = object_list; obj != NULL; obj = obj->next )
	{
		if ( !can_see_obj( ch, obj ) || !is_name( argument, obj->name ))
		{
			continue;
		}

		found = TRUE;
		number++;

		for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
			;

		if ( in_obj->carried_by != NULL && can_see(ch,in_obj->carried_by) && in_obj->carried_by->in_room != NULL)
		{
			add_buf(buffer, (char *)Format("%3d) %s is carried by %s [Room %d]\n\r",
				number, CapitalSentence(obj->short_descr), PERS(in_obj->carried_by, ch), in_obj->carried_by->in_room->vnum ) );
		}
		else if (in_obj->in_room != NULL && can_see_room(ch,in_obj->in_room))
		{
			add_buf(buffer, (char *)Format("%3d) %s is in %s [Room %d]\n\r",
				number, CapitalSentence(obj->short_descr),in_obj->in_room->name, in_obj->in_room->vnum) );
		}
		else
		{
			add_buf(buffer, (char *)Format("%3d) %s is somewhere\n\r", number, CapitalSentence(obj->short_descr) ));
		}
		if (number >= max_found)
		{
			break;
		}
	}

	if ( !found )
	{
		send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
	}
	else
	{
		page_to_char(buf_string(buffer),ch);
	}

	free_buf(buffer);
}


void do_mwhere( CHAR_DATA *ch, char *argument )
{
	char buf[MSL]={'\0'};
	BUFFER *buffer;
	CHAR_DATA *victim;
	bool found;
	int count = 0;

	CheckCH(ch);

	if ( IS_NULLSTR(argument) )
	{
		DESCRIPTOR_DATA *d;

		/* show characters logged */

		buffer = new_buf();
		for (d = descriptor_list; d != NULL; d = d->next)
		{
			if (d->character != NULL && d->connected == CON_PLAYING
					&&  d->character->in_room != NULL && can_see(ch,d->character)
			&&  can_see_room(ch,d->character->in_room))
			{
				victim = d->character;
				count++;
				if (d->original != NULL)
				{
					snprintf(buf, sizeof(buf),
							"%3d) %s (in the body of %s) is in %s [%d]\n\r",
							count, d->original->name,victim->short_descr,
							victim->in_room->name,victim->in_room->vnum);
				}
				else
				{
					snprintf(buf, sizeof(buf), "%3d) %s is in %s [%d]\n\r",
							count, victim->name,victim->in_room->name,
							victim->in_room->vnum);
				}
				add_buf(buffer,buf);
			}
		}

		page_to_char(buf_string(buffer),ch);
		free_buf(buffer);
		return;
	}

    found = FALSE;
    buffer = new_buf();
    for ( victim = char_list; victim != NULL; victim = victim->next )
    {
    	if ( victim->in_room != NULL &&   is_name( argument, victim->name ) )
    	{
    		found = TRUE;
    		count++;
    		add_buf(buffer, (char *)Format("%3d) [%5d] %-28s [%5d] %s\n\r", count,
    				IS_NPC(victim) ? victim->pIndexData->vnum : 0, IS_NPC(victim) ? victim->short_descr : victim->name, victim->in_room->vnum, 	victim->in_room->name));
    	}
    }

    /* all the mobs without a room */
    if (!str_cmp(argument,"nowhere")) {
        buffer = new_buf();
        found=FALSE;
        count=0;
        for ( victim = char_list; victim != NULL; victim = victim->next )
        {
            if (victim->in_room==NULL)
            {
                found = TRUE;
                count++;
                add_buf(buffer, (char *)Format("%3d) [%5d] %-28s %lx\n\r", count, IS_NPC(victim) ? victim->pIndexData->vnum : 0,
                    IS_NPC(victim) ? victim->short_descr : victim->name, (unsigned long)victim));
            }
        }
        if (found)
            page_to_char(buf_string(buffer),ch);
        else
            send_to_char("No mobs without rooms found.\n\r",ch);
        free_buf(buffer);
        return;
    }

    if ( !found )
	act( "You didn't find any $T.", ch, NULL, argument, TO_CHAR, 0 );
    else
    	page_to_char(buf_string(buffer),ch);

    free_buf(buffer);

    return;
}


void do_reboo( CHAR_DATA *ch, char *argument )
{
    CheckCH(ch);
    send_to_char( "Type the whole command.\n\r", ch );
    return;
}


void do_reboot( CHAR_DATA *ch, char *argument )
{
	extern bool merc_down;
	DESCRIPTOR_DATA *d,*d_next;
	CHAR_DATA *vch;
	int a = 0;
	CheckCH(ch);

	if (!IS_NULLSTR(argument) && (!is_number(argument) && str_cmp(argument, "now") && str_cmp(argument, "cancel")))
	{
		send_to_char("Syntax: reboot [now/# of minutes/cancel]\n\r", ch);
		return;
	}

	if (IS_NULLSTR(argument))
	{
		reboot_countdown = 60*5*PULSE_PER_SECOND;
		do_function(ch, &do_echo, (char *)Format("Reboot in 5 minutes.") );
		if (ch != NULL)
			snprintf(reboot_initiator, sizeof(reboot_initiator), "%s", ch->name);
		return;
	}

	if (!str_cmp(argument, "cancel"))
	{
		reboot_countdown = -1;
		do_function(ch, &do_echo, (char *)Format("Reboot timer disabled.") );
		return;
	}

	if (is_number(argument) && (a = atoi(argument)) < 0)
	{
		send_to_char("# of minutes must be 0 or higher. (0 = now)\n\r", ch);
		return;
	}

	if (is_number(argument) && a > 0)
	{
		reboot_countdown = a*60*PULSE_PER_SECOND;
		do_function(ch, &do_echo, (char *)Format("Reboot in %d minutes.", a) );
		if (ch != NULL)
			snprintf(reboot_initiator, sizeof(reboot_initiator), "%s", ch->name);
		return;
	}

	if (ch != NULL && ch->invis_level < LEVEL_IMMORTAL)
	{
		do_function(ch, &do_echo, (char *)Format("Reboot by %s.", ch->name) );
	}
	else if (!IS_NULLSTR(reboot_initiator))
	{
		do_function(NULL, &do_echo, (char *)Format("Reboot by %s.", reboot_initiator) );
	}
	else
	{
		do_function(NULL, &do_echo, (char *)Format("Automatic reboot.") );
	}

	merc_down = TRUE;
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
		d_next = d->next;
		vch = d->original ? d->original : d->character;
		if (vch != NULL)
			save_char_obj(vch);
		close_socket(d);
	}

	return;
}


void do_shutdow( CHAR_DATA *ch, char *argument )
{
    CheckCH(ch);
    send_to_char( "Type the whole command.\n\r", ch );
    return;
}


void do_shutdown( CHAR_DATA *ch, char *argument )
{
	DESCRIPTOR_DATA *d,*d_next;
	CHAR_DATA *vch;
	char buf[MSL]={'\0'};
	extern bool merc_down;

	CheckCH(ch);

	if (ch->invis_level < LEVEL_IMMORTAL)
	{
		snprintf( buf, sizeof(buf), "Shutdown by %s.", ch->name );
	}

	append_file( ch, SHUTDOWN_FILE, buf );
	strncat( buf, "\n\r" , sizeof(buf) - strlen(buf) - 1);

	if (ch->invis_level < LEVEL_IMMORTAL)
	{
		do_function(ch, &do_echo, buf );
	}

	merc_down = TRUE;

	for ( d = descriptor_list; d != NULL; d = d_next)
	{
		d_next = d->next;
		vch = d->original ? d->original : d->character;
		if (vch != NULL)
		{
			save_char_obj(vch);
		}
		// close_socket(d);
	}

	return;
}

void do_snoop( CHAR_DATA *ch, char *argument )
{
	DESCRIPTOR_DATA *d;
	CHAR_DATA *victim;
	char arg[MIL]={'\0'};

	CheckCH(ch);

	one_argument( argument, arg );

	if ( IS_NULLSTR(arg) )
	{
		send_to_char( "Snoop whom?\n\r", ch );
		return;
	}

	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	if ( victim->desc == NULL )
	{
		send_to_char( "No descriptor to snoop.\n\r", ch );
		return;
	}

	if ( victim == ch )
	{
		send_to_char( "Cancelling all snoops.\n\r", ch );
		wiznet("\tY[WIZNET]\tn $N stops being such a snoop.", ch,NULL,WIZ_SNOOPS,WIZ_SECURE,get_trust(ch));
		for ( d = descriptor_list; d != NULL; d = d->next )
		{
			if ( d->snoop_by == ch->desc )
				d->snoop_by = NULL;
		}
		return;
	}

	if ( victim->desc->snoop_by != NULL )
	{
		send_to_char( "Busy already.\n\r", ch );
		return;
	}

	if (!is_room_owner(ch,victim->in_room) && ch->in_room != victim->in_room
			&&  room_is_private(victim->in_room) && !IS_TRUSTED(ch,STORYTELLER))
	{
		send_to_char("That character is in a private room.\n\r",ch);
		return;
	}

	if ( get_trust( victim ) >= get_trust( ch ) )
	{
		send_to_char( "You failed.\n\r", ch );
		return;
	}

	if ( ch->desc != NULL )
	{
		for ( d = ch->desc->snoop_by; d != NULL; d = d->snoop_by )
		{
			if ( d->character == victim || d->original == victim )
			{
				send_to_char( "No snoop loops.\n\r", ch );
				return;
			}
		}
	}

	victim->desc->snoop_by = ch->desc;
	wiznet((char *)Format("\tY[WIZNET]\tn $N starts snooping on %s", (IS_NPC(ch) ? victim->short_descr : victim->name)), ch,NULL,WIZ_SNOOPS,WIZ_SECURE,get_trust(ch));
	send_to_char( "Ok.\n\r", ch );
	return;
}


void do_switch( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	char arg[MIL]={'\0'};

	CheckCH(ch);

	one_argument( argument, arg );

	if ( IS_NULLSTR(arg) )
	{
		send_to_char( "Switch into whom?\n\r", ch );
		return;
	}

	if ( ch->desc == NULL )
		return;

	if ( ch->desc->original != NULL )
	{
		send_to_char( "You are already switched.\n\r", ch );
		return;
	}

	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	if ( victim == ch )
	{
		send_to_char( "Ok.\n\r", ch );
		return;
	}

	if (!IS_NPC(victim))
	{
		send_to_char("You can't switch into players.\n\r",ch);
		return;
	}

	if (!is_room_owner(ch,victim->in_room) && ch->in_room != victim->in_room
		&&  room_is_private(victim->in_room) && !IS_TRUSTED(ch,MASTER))
	{
		send_to_char("That character is in a private room.\n\r",ch);
		return;
	}

	if ( victim->desc != NULL )
	{
		send_to_char( "Character in use.\n\r", ch );
		return;
	}

	wiznet((char *)Format("\tY[WIZNET]\tn $N switches into %s",victim->short_descr),ch,NULL,WIZ_SWITCHES,WIZ_SECURE,get_trust(ch));

	ch->desc->character = victim;
	ch->desc->original  = ch;
	victim->desc        = ch->desc;
	ch->desc            = NULL;

	/* change communications to match */
	if (ch->prompt != NULL)
	{
		PURGE_DATA( victim->prompt );
		victim->prompt = str_dup(ch->prompt);
	}
	victim->comm = ch->comm;
	victim->lines = ch->lines;
	send_to_char( "Ok.\n\r", victim );
	return;
}


void do_return( CHAR_DATA *ch, char *argument )
{
	CheckCH(ch);

	if ( ch->desc == NULL )
		return;

	if ( ch->desc->original == NULL )
	{
		send_to_char( "You aren't switched.\n\r", ch );
		return;
	}

	send_to_char( "You return to your original body.\n\r", ch );
	if (ch->prompt != NULL)
	{
		PURGE_DATA(ch->prompt);
		ch->prompt = NULL;
	}

	wiznet((char *)Format("\tY[WIZNET]\tn $N returns from %s.",ch->short_descr), ch->desc->original,0,WIZ_SWITCHES,WIZ_SECURE,get_trust(ch));
	ch->desc->character       = ch->desc->original;
	ch->desc->original        = NULL;
	ch->desc->character->desc = ch->desc;
	ch->desc                  = NULL;
	return;
}

void do_as( CHAR_DATA *ch, char *argument )
{
	char arg[MIL]={'\0'};
	CHAR_DATA *victim;
	DESCRIPTOR_DATA *desc;

	CheckCH(ch);

	argument = one_argument( argument, arg );

	if ( IS_NULLSTR(arg) )
	{
		send_to_char( "Run a command as whom?\n\r", ch );
		return;
	}

	if ( IS_NULLSTR(argument) )
	{
		send_to_char("Run which command?\n\r", ch);
		return;
	}

	/* Block to stop bad freakiness with descriptors. */
	if (!str_cmp(argument,"delete") || !str_cmp(argument, "quit")
			|| !str_prefix(argument, "switch") || !str_prefix(argument, "being")
			|| !str_prefix(argument,"mob"))
	{
		send_to_char("That will NOT be done.\n\r",ch);
		return;
	}

	if ( ch->desc == NULL )
		return;

	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	if ( victim == ch )
	{
		send_to_char( "Ok.\n\r", ch );
		return;
	}

	if (!is_room_owner(ch,victim->in_room) && ch->in_room != victim->in_room
    &&  room_is_private(victim->in_room) && !IS_TRUSTED(ch,MASTER))
    {
	send_to_char("That character is in a private room.\n\r",ch);
	return;
    }

    if (ch->trust < victim->trust)
    {
	send_to_char("Not on those with more authority.\n\r", ch);
    }

    wiznet((char *)Format("\tY[WIZNET]\tn $N acts as %s: %s", IS_NPC(victim) ? victim->short_descr : victim->name, argument),ch,NULL,WIZ_SWITCHES,WIZ_SECURE,get_trust(ch));

    ch->desc->original	= ch;
    ch->desc->character = victim;
    desc		= victim->desc;
    victim->desc        = ch->desc;

    interpret(victim, argument);

    ch->desc->character = ch;
    victim->desc        = desc;
    ch->desc->original	= NULL;
    send_to_char( "Ok.\n\r", ch );
    return;
}



/* trust levels for load and clone */
bool obj_check (CHAR_DATA *ch, OBJ_DATA *obj)
{
    if (IS_TRUSTED(ch,LEVEL_IMMORTAL))
	return TRUE;
    else
	return FALSE;
}

/* for clone, to insure that cloning goes many levels deep */
void recursive_clone(CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *clone)
{
    OBJ_DATA *c_obj, *t_obj;


    for (c_obj = obj->contains; c_obj != NULL; c_obj = c_obj->next_content)
    {
	if (obj_check(ch,c_obj))
	{
	    t_obj = create_object(c_obj->pIndexData);
	    clone_object(c_obj,t_obj);
	    obj_to_obj(t_obj,clone);
	    recursive_clone(ch,c_obj,t_obj);
	}
    }
}

/* command that is similar to load */
void do_clone(CHAR_DATA *ch, char *argument )
{
	char arg[MIL]={'\0'};
	char *rest;
	CHAR_DATA *mob;
	OBJ_DATA  *obj;

	CheckCH(ch);

	rest = one_argument(argument,arg);

	if (IS_NULLSTR(arg))
	{
		send_to_char("Clone what?\n\r",ch);
		return;
	}

	if (!str_prefix(arg,"object"))
	{
		mob = NULL;
		obj = get_obj_here(ch,rest);
		if (obj == NULL)
		{
			send_to_char("You don't see that here.\n\r",ch);
			return;
		}
	}
	else if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character"))
	{
		obj = NULL;
		mob = get_char_room(ch,rest);
		if (mob == NULL)
		{
			send_to_char("You don't see that here.\n\r",ch);
			return;
		}
	}
	else /* find both */
	{
		mob = get_char_room(ch,argument);
		obj = get_obj_here(ch,argument);
		if (mob == NULL && obj == NULL)
		{
			send_to_char("You don't see that here.\n\r",ch);
			return;
		}
	}

	/* clone an object */
	if (obj != NULL)
	{
		OBJ_DATA *clone;

		if (!obj_check(ch,obj))
		{
			send_to_char( "Your powers are not great enough for such a task.\n\r",ch);
			return;
		}

		clone = create_object(obj->pIndexData);
		clone_object(obj,clone);
		if (obj->carried_by != NULL)
			obj_to_char(clone,ch);
		else
			obj_to_room(clone,ch->in_room);
		recursive_clone(ch,obj,clone);

		act("$n has created $p.",ch,clone,NULL,TO_ROOM,0);
		act("You clone $p.",ch,clone,NULL,TO_CHAR,0);
		wiznet("\tY[WIZNET]\tn $N clones $p.",ch,clone,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
		return;
	}
	else if (mob != NULL)
	{
		CHAR_DATA *clone;
		OBJ_DATA *new_obj;

		if (!IS_NPC(mob))
		{
			send_to_char("You can only clone mobiles.\n\r",ch);
			return;
		}

		if (!IS_TRUSTED(ch,LEVEL_IMMORTAL))
		{
			send_to_char( "Your powers are not great enough for such a task.\n\r",ch);
			return;
		}

		clone = create_mobile(mob->pIndexData);
		clone_mobile(mob,clone);

		for (obj = mob->carrying; obj != NULL; obj = obj->next_content)
		{
			if (obj_check(ch,obj))
			{
				new_obj = create_object(obj->pIndexData);
				clone_object(obj,new_obj);
				recursive_clone(ch,obj,new_obj);
				obj_to_char(new_obj,clone);
				new_obj->wear_loc = obj->wear_loc;
			}
		}
		char_to_room(clone,ch->in_room);
		act("$n has created $N.",ch,NULL,clone,TO_ROOM,0);
		act("You clone $N.",ch,NULL,clone,TO_CHAR,0);
		wiznet((char *)Format("\tY[WIZNET]\tn $N clones %s.",clone->short_descr),ch,NULL,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
		return;
	}
}

/* RT to replace the two load commands */

void do_load(CHAR_DATA *ch, char *argument )
{
   char arg[MIL]={'\0'};

   CheckCH(ch);

   argument = one_argument(argument,arg);

    if (IS_NULLSTR(arg))
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  load mob <vnum>\n\r",ch);
	send_to_char("  load obj <vnum> <level>\n\r",ch);
	return;
    }

    if (!str_cmp(arg,"mob") || !str_cmp(arg,"char"))
    {
	do_function(ch, &do_mload, argument );
	return;
    }

    if (!str_cmp(arg,"obj"))
    {
	do_function(ch, &do_oload, argument );
	return;
    }
    /* echo syntax */
    do_function(ch, &do_load, "" );
}


void do_mload( CHAR_DATA *ch, char *argument )
{
    char arg[MIL]={'\0'};
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA *victim;

    CheckCH(ch);

    one_argument( argument, arg );

    if ( IS_NULLSTR(arg) || !is_number(arg) )
    {
	send_to_char( "Syntax: load mob <vnum>.\n\r", ch );
	return;
    }

    if ( ( pMobIndex = get_mob_index( atoi( arg ) ) ) == NULL )
    {
	send_to_char( "No mob has that vnum.\n\r", ch );
	return;
    }

    if ( !IS_BUILDER(ch, pMobIndex->area) && !IS_ADMIN(ch) )
    {
	send_to_char( "You can only load mobs from your own areas.\n\r", ch);
	return;
    }

    victim = create_mobile( pMobIndex );

    if(IS_SET(ch->comm, COMM_BUILDER))
    {
	SET_BUILDERMODE(victim);
	victim->master = ch;
    }

    char_to_room( victim, ch->in_room );
    act( "$n has created $N!", ch, NULL, victim, TO_ROOM, 0 );
    wiznet((char *)Format("\tY[WIZNET]\tn $N loads %s.",victim->short_descr),ch,NULL,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_oload( CHAR_DATA *ch, char *argument )
{
    char arg1[MIL]={'\0'};
    char arg2[MIL]={'\0'};
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;

    CheckCH(ch);

    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );

    if ( IS_NULLSTR(arg1) || !is_number(arg1))
    {
	send_to_char( "Syntax: load obj <vnum>.\n\r", ch );
	return;
    }

    if ( ( pObjIndex = get_obj_index( atoi( arg1 ) ) ) == NULL )
    {
	send_to_char( "No object has that vnum.\n\r", ch );
	return;
    }

    if ( !IS_BUILDER(ch, pObjIndex->area) && !IS_ADMIN(ch) )
    {
	send_to_char( "You can only load objects from your own areas.\n\r", ch);
	return;
    }

    obj = create_object( pObjIndex );

    if(IS_SET(ch->comm, COMM_BUILDER))
    {
	SET_BUILDERMODE(obj);
	PURGE_DATA(obj->owner);
	obj->owner = str_dup(ch->name);
    }

    if ( CAN_WEAR(obj, ITEM_TAKE) )
	obj_to_char( obj, ch );
    else
	obj_to_room( obj, ch->in_room );
    act( "$n has created $p!", ch, obj, NULL, TO_ROOM, 0 );
    wiznet("\tY[WIZNET]\tn $N loads $p.",ch,obj,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_purge( CHAR_DATA *ch, char *argument )
{
	char arg[MIL]={'\0'};
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	DESCRIPTOR_DATA *d;

	CheckCH(ch);

	one_argument( argument, arg );

	if(!IS_ADMIN(ch) && !IS_BUILDER(ch, ch->in_room->area))
	{
		send_to_char( "You can only use the purge command in your own areas.\n\r", ch);
		return;
	}

	if ( IS_NULLSTR(arg) )
	{
		/* 'purge' */
		CHAR_DATA *vnext;
		OBJ_DATA  *obj_next;

		for ( victim = ch->in_room->people; victim != NULL; victim = vnext )
		{
			vnext = victim->next_in_room;
			if ( IS_NPC(victim) && !IS_SET(victim->act,ACT_NOPURGE)
					&&   victim != ch /* safety precaution */ )
				extract_char( victim, TRUE );
		}

		for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
		{
			obj_next = obj->next_content;
			if (!IS_OBJ_STAT(obj,ITEM_NOPURGE))
				extract_obj( obj );
		}

		act( "The room empties.", ch, NULL, NULL, TO_ROOM, 1);
		send_to_char( "Ok.\n\r", ch );
		return;
	}

	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	if ( !IS_NPC(victim) )
	{

		if (ch == victim)
		{
			send_to_char("Try quitting.\n\r",ch);
			return;
		}

		if (get_trust(ch) <= get_trust(victim))
		{
			send_to_char("Maybe that wasn't a good idea...\n\r",ch);
			send_to_char(Format("%s tried to purge you!\n\r",ch->name),victim);
			return;
		}

		act("$n disintegrates $N.",ch,0,victim,TO_NOTVICT,0);

		if (victim->trust > 0)
			save_char_obj( victim );
		d = victim->desc;
		extract_char( victim, TRUE );
		if ( d != NULL )
			close_socket( d );

		return;
	}

	act( "$n purges $N.", ch, NULL, victim, TO_NOTVICT, 0 );
	extract_char( victim, TRUE );
	return;
}



void do_trust( CHAR_DATA *ch, char *argument )
{
	char arg1[MIL]={'\0'};
	char arg2[MIL]={'\0'};
	CHAR_DATA *victim;
	int level;

	CheckCH(ch);

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( IS_NULLSTR(arg1) || IS_NULLSTR(arg2) )
	{
		send_to_char( "Syntax: trust <char> <access level>.\n\r", ch );
		return;
	}

	if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
	{
		send_to_char( "That player is not here.\n\r", ch);
		return;
	}

	if(is_number(arg2))
	{
		if ( ( level = atoi( arg2 ) ) < 0 || level > MAX_LEVEL )
		{
			send_to_char( Format("Level must be 0 (reset) or 1 to %d.\n\r", MAX_LEVEL), ch );
			return;
		}
	}
	else
	{
		if((level = flag_lookup2 (arg2, staff_status)) == -1)
		{
			send_to_char("Options are: Player, Watcher, Storyteller, Master and Implementor", ch);
			return;
		}
	}

	if ( level > get_trust( ch ) )
	{
		send_to_char( "Limited to your trust.\n\r", ch );
		return;
	}

	act("$N trusted as $t.", ch, staff_status[level].name, victim, TO_CHAR, 1);
	victim->trust = level;
	return;
}



void do_restore( CHAR_DATA *ch, char *argument )
{
	char arg[MIL]={'\0'};
	CHAR_DATA *victim;
	CHAR_DATA *vch;
	DESCRIPTOR_DATA *d;

	CheckCH(ch);

	one_argument( argument, arg );
	if (IS_NULLSTR(arg) || !str_cmp(arg,"room"))
	{
		/* cure room */

		for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
		{
			affect_strip(vch,gsn_plague);
			affect_strip(vch,gsn_poison);
			affect_strip(vch,gsn_blindness);
			affect_strip(vch,gsn_sleep);
			affect_strip(vch,gsn_curse);

			vch->health	= MAX_HEALTH;
			vch->agghealth = MAX_HEALTH;

			if(vch->race == race_lookup("vampire"))
			{
				vch->max_RBPG = 23 - vch->gen;
				if(vch->max_RBPG < 10) vch->max_RBPG = 10;
			}
			vch->RBPG = vch->max_RBPG;
			vch->GHB = vch->max_GHB;
			vch->willpower = vch->max_willpower;
			if(vch->condition[COND_HUNGER] > 0)
				vch->condition[COND_HUNGER] = 48;
			if(vch->condition[COND_THIRST] > 0)
				vch->condition[COND_THIRST] = 48;
			if(vch->condition[COND_FULL] > 0)
				vch->condition[COND_FULL] = 48;
			update_pos( vch, 0 );
			act("$n has restored you.",ch,NULL,vch,TO_VICT,1);
		}

		wiznet((char *)Format("\tY[WIZNET]\tn $N restored room %d.",ch->in_room->vnum),ch,NULL,WIZ_RESTORE,WIZ_SECURE,get_trust(ch));

		send_to_char("Room restored.\n\r",ch);
		return;

	}

	if ( get_trust(ch) >=  MAX_LEVEL - 1 && !str_cmp(arg,"all"))
	{
		/* cure all */

		for (d = descriptor_list; d != NULL; d = d->next)
		{
			victim = d->character;

			if (victim == NULL || IS_NPC(victim))
				continue;

			affect_strip(victim,gsn_plague);
			affect_strip(victim,gsn_poison);
			affect_strip(victim,gsn_blindness);
			affect_strip(victim,gsn_sleep);
			affect_strip(victim,gsn_curse);

			victim->health 	= MAX_HEALTH;
			victim->agghealth 	= MAX_HEALTH;

			if(victim->race == race_lookup("vampire"))
			{
				victim->max_RBPG = 23 - victim->gen;
				if(victim->max_RBPG < 10) victim->max_RBPG = 10;
			}
			victim->RBPG = victim->max_RBPG;
			victim->GHB = victim->max_GHB;
			victim->willpower = victim->max_willpower;
			if(victim->condition[COND_HUNGER] > 0)
				victim->condition[COND_HUNGER] = 48;
			if(victim->condition[COND_THIRST] > 0)
				victim->condition[COND_THIRST] = 48;
			if(victim->condition[COND_FULL] > 0)
				victim->condition[COND_FULL] = 48;
			update_pos( victim, 0 );
			if (victim->in_room != NULL)
				act("$n has restored you.",ch,NULL,victim,TO_VICT,1);
		}
		send_to_char("All active players restored.\n\r",ch);
		return;
	}

	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	affect_strip(victim,gsn_plague);
	affect_strip(victim,gsn_poison);
	affect_strip(victim,gsn_blindness);
	affect_strip(victim,gsn_sleep);
	affect_strip(victim,gsn_curse);
	victim->health  = MAX_HEALTH;
	victim->agghealth  = MAX_HEALTH;

	if(victim->race == race_lookup("vampire"))
	{
		victim->max_RBPG = 23 - victim->gen;
		if(victim->max_RBPG < 10) victim->max_RBPG = 10;
	}
	victim->RBPG = victim->max_RBPG;
	victim->GHB = victim->max_GHB;
	victim->willpower = victim->max_willpower;
	if(victim->condition[COND_HUNGER] > 0)
		victim->condition[COND_HUNGER] = 48;
	if(victim->condition[COND_THIRST] > 0)
		victim->condition[COND_THIRST] = 48;
	if(victim->condition[COND_FULL] > 0)
		victim->condition[COND_FULL] = 48;
	update_pos( victim, 0 );

	if(ch != victim)
		act( "$n has restored you.", ch, NULL, victim, TO_VICT, 1 );
	wiznet((char *)Format("\tY[WIZNET]\tn $N restored %s", IS_NPC(victim) ? victim->short_descr : victim->name),ch,NULL,WIZ_RESTORE,WIZ_SECURE,get_trust(ch));
	if(ch != victim)
		send_to_char( "Ok.\n\r", ch );
	return;
}


void do_freeze( CHAR_DATA *ch, char *argument )
{
	char arg[MIL]={'\0'};
	CHAR_DATA *victim;

	CheckCH(ch);

	one_argument( argument, arg );

	if ( IS_NULLSTR(arg) )
	{
		send_to_char( "Freeze whom?\n\r", ch );
		return;
	}

	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	if ( IS_NPC(victim) )
	{
		send_to_char( "Not on NPC's.\n\r", ch );
		return;
	}

	if ( get_trust( victim ) >= get_trust( ch ) )
	{
		send_to_char( "You failed.\n\r", ch );
		return;
	}

    if ( IS_SET(victim->plr_flags, PLR_FREEZE) )
    {
    	REMOVE_BIT(victim->plr_flags, PLR_FREEZE);
    	send_to_char( "You can play again.\n\r", victim );
    	send_to_char( "FREEZE removed.\n\r", ch );
    	wiznet((char *)Format("\tY[WIZNET]\tn $N thaws %s.",victim->name),ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
    	SET_BIT(victim->plr_flags, PLR_FREEZE);
    	send_to_char( "You can't do ANYthing!\n\r", victim );
    	send_to_char( "FREEZE set.\n\r", ch );
    	wiznet((char *)Format("\tY[WIZNET]\tn $N puts %s in the deep freeze.",victim->name),ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

    save_char_obj( victim );

    return;
}



void do_log( CHAR_DATA *ch, char *argument )
{
	char arg[MIL]={'\0'};
	CHAR_DATA *victim;

	CheckCH(ch);

	one_argument( argument, arg );

	if ( IS_NULLSTR(arg) )
	{
		send_to_char( "Log whom?\n\r", ch );
		return;
	}

	if ( !str_cmp( arg, "all" ) )
	{
		if ( fLogAll )
		{
			fLogAll = FALSE;
			send_to_char( "Log ALL off.\n\r", ch );
		}
		else
		{
			fLogAll = TRUE;
			send_to_char( "Log ALL on.\n\r", ch );
		}
		return;
	}

	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	if ( IS_NPC(victim) )
	{
		send_to_char( "Not on NPC's.\n\r", ch );
		return;
	}

	/*
	 * No level check, gods can log anyone.
	 */
	if ( IS_SET(victim->plr_flags, PLR_LOG) )
	{
		REMOVE_BIT(victim->plr_flags, PLR_LOG);
		log_string( LOG_SECURITY, Format("%s has had their LOG flag removed.", ch->name) );
		send_to_char( "LOG removed.\n\r", ch );
	}
	else
	{
		SET_BIT(victim->plr_flags, PLR_LOG);
		log_string( LOG_SECURITY, Format("%s has had their LOG flag set.", ch->name) );
		send_to_char( "LOG set.\n\r", ch );
	}

	return;
}



void do_noemote( CHAR_DATA *ch, char *argument )
{
    char arg[MIL]={'\0'};
    CHAR_DATA *victim;

    CheckCH(ch);

    one_argument( argument, arg );

    if ( IS_NULLSTR(arg) )
    {
	send_to_char( "Noemote whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }


    if ( get_trust( victim ) >= get_trust( ch ) )
    {
    	send_to_char( "You failed.\n\r", ch );
    	return;
    }

    if ( IS_SET(victim->comm, COMM_NOEMOTE) )
    {
    	REMOVE_BIT(victim->comm, COMM_NOEMOTE);
    	send_to_char( "You can emote again.\n\r", victim );
    	send_to_char( "NOEMOTE removed.\n\r", ch );
    	wiznet((char *)Format("\tY[WIZNET]\tn $N restores emotes to %s.",victim->name),ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
    	SET_BIT(victim->comm, COMM_NOEMOTE);
    	send_to_char( "You can't emote!\n\r", victim );
    	send_to_char( "NOEMOTE set.\n\r", ch );
    	wiznet((char *)Format("\tY[WIZNET]\tn $N revokes %s's emotes.",victim->name),ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

    return;
}



void do_notell( CHAR_DATA *ch, char *argument )
{
	char arg[MIL]={'\0'};
	CHAR_DATA *victim;

	CheckCH(ch);

	one_argument( argument, arg );

	if ( IS_NULLSTR(arg) )
	{
		send_to_char( "Notell whom?", ch );
		return;
	}

	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	if ( get_trust( victim ) >= get_trust( ch ) )
	{
		send_to_char( "You failed.\n\r", ch );
		return;
	}

	if ( IS_SET(victim->comm, COMM_NOTELL) )
	{
		REMOVE_BIT(victim->comm, COMM_NOTELL);
		send_to_char( "You can tell again.\n\r", victim );
		send_to_char( "NOTELL removed.\n\r", ch );
		wiznet((char *)Format("\tY[WIZNET]\tn $N restores tells to %s.",victim->name),ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
	}
	else
	{
		SET_BIT(victim->comm, COMM_NOTELL);
		send_to_char( "You can't tell!\n\r", victim );
		send_to_char( "NOTELL set.\n\r", ch );
		wiznet((char *)Format("\tY[WIZNET]\tn $N revokes %s's tells.",victim->name),ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
	}

	return;
}



void do_peace( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;

    CheckCH(ch);

    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
	if ( rch->fighting != NULL )
	    stop_fighting( rch, TRUE );
	if (IS_NPC(rch) && IS_SET(rch->act,ACT_AGGRESSIVE))
	    REMOVE_BIT(rch->act,ACT_AGGRESSIVE);
    }

    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_wizlock( CHAR_DATA *ch, char *argument )
{
	extern bool wizlock;
	wizlock = !wizlock;

	CheckCH(ch);

	if ( wizlock )
	{
		wiznet("\tY[WIZNET]\tn $N has wizlocked the game.",ch,NULL,0,0,0);
		send_to_char( "Game wizlocked.\n\r", ch );
	}
	else
	{
		wiznet("\tY[WIZNET]\tn $N removes wizlock.",ch,NULL,0,0,0);
		send_to_char( "Game un-wizlocked.\n\r", ch );
	}

	return;
}

/* RT anti-newbie code */

void do_newlock( CHAR_DATA *ch, char *argument )
{
	extern bool newlock;
	newlock = !newlock;

	CheckCH(ch);

	if ( newlock )
	{
		wiznet("\tY[WIZNET]\tn $N locks out new characters.",ch,NULL,0,0,0);
		send_to_char( "New characters have been locked out.\n\r", ch );
	}
	else
	{
		wiznet("\tY[WIZNET]\tn $N allows new characters back in.",ch,NULL,0,0,0);
		send_to_char( "Newlock removed.\n\r", ch );
	}

	return;
}


void do_slookup( CHAR_DATA *ch, char *argument )
{
	char arg[MIL]={'\0'};
	int sn = 0;

	CheckCH(ch);

	one_argument( argument, arg );
	if ( IS_NULLSTR(arg) )
	{
		send_to_char( "Lookup which skill or spell?\n\r", ch );
		return;
	}

	if ( !str_cmp( arg, "all" ) )
	{
		for ( sn = 0; sn < MAX_SKILL; sn++ )
		{
			if ( skill_table[sn].name == NULL )
				break;
			send_to_char( Format("Sn: %3d  Slot: %3d  Skill/spell: '%s'\n\r", sn, skill_table[sn].slot, skill_table[sn].name), ch );
		}
	}
	else
	{
		if ( ( sn = skill_lookup( arg ) ) < 0 )
		{
			send_to_char( "No such skill or spell.\n\r", ch );
			return;
		}

		send_to_char( Format("Sn: %3d  Slot: %3d  Skill/spell: '%s'\n\r", sn, skill_table[sn].slot, skill_table[sn].name), ch );
	}

	return;
}

/* RT set replaces sset, mset, oset, and rset */

void do_set( CHAR_DATA *ch, char *argument )
{
	char arg[MIL]={'\0'};

	CheckCH(ch);

	argument = one_argument(argument,arg);

	if (IS_NULLSTR(arg))
	{
		send_to_char("Syntax:\n\r",ch);
		send_to_char("  set mob   <name> <field> <value>\n\r",ch);
		send_to_char("  set obj   <name> <field> <value>\n\r",ch);
		send_to_char("  set room  <room> <field> <value>\n\r",ch);
		send_to_char("  set skill <name> <spell or skill> <value>\n\r",ch);
		return;
	}

	if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character"))
	{
		do_function(ch, &do_mset, argument );
		return;
	}

	if (!str_prefix(arg,"skill") || !str_prefix(arg,"spell"))
	{
		do_function(ch, &do_sset, argument );
		return;
	}

	if (!str_prefix(arg,"object"))
	{
		do_function(ch, &do_oset, argument );
		return;
	}

	if (!str_prefix(arg,"room"))
	{
		do_function(ch, &do_rset, argument );
		return;
	}
	/* echo syntax */
	do_function(ch, &do_set, "" );
}


void do_sset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MIL]={'\0'};
    char arg2 [MIL]={'\0'};
    char arg3 [MIL]={'\0'};
    CHAR_DATA *victim;
    int value;
    int sn;
    bool fAll;

    CheckCH(ch);

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( IS_NULLSTR(arg1) || IS_NULLSTR(arg2) || IS_NULLSTR(arg3) )
    		{
    	send_to_char( "Syntax:\n\r",ch);
    	send_to_char( "  set skill <name> <spell or skill> <value>\n\r", ch);
    	send_to_char( "  set skill <name> all <value>\n\r",ch);
    	send_to_char("   (use the name of the skill, not the number)\n\r",ch);
    	return;
    		}

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    fAll = !str_cmp( arg2, "all" );
    sn   = 0;
    if ( !fAll && ( sn = skill_lookup( arg2 ) ) < 0 )
    {
	send_to_char( "No such skill or spell.\n\r", ch );
	return;
    }

    /*
     * Snarf the value.
     */
    if ( !is_number( arg3 ) )
    {
	send_to_char( "Value must be numeric.\n\r", ch );
	return;
    }

    value = atoi( arg3 );
    if ( value < 0 || value > 100 )
    {
	send_to_char( "Value range is 0 to 100.\n\r", ch );
	return;
    }

    if ( fAll )
    {
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name != NULL )
		victim->learned[sn]	= value;
	}
    }
    else
    {
	victim->learned[sn] = value;
    }

    return;
}


void do_mset( CHAR_DATA *ch, char *argument )
{
	char arg1 [MIL]={'\0'};
	char arg2 [MIL]={'\0'};
	char arg3 [MIL]={'\0'};
	CHAR_DATA *victim;
	int value = 0, rbpg = 0;

	CheckCH(ch);

	smash_tilde( argument );
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	strncpy( arg3, argument, sizeof(arg3) );

	if ( IS_NULLSTR(arg1) || IS_NULLSTR(arg2) || IS_NULLSTR(arg3) )
	{
		send_to_char("Syntax:\n\r",ch);
		send_to_char("  set char <name> <field> <value>\n\r",ch);
		send_to_char( "  Field being one of:\n\r",			ch );
		send_to_char( "    str dex sta cha man app per int wit\n\r",	ch );
		send_to_char( "    sex clan exp race security faith bgtime\n\r",ch );
		send_to_char( "    group dollars cents hp mana move giftexp\n\r",ch );
		send_to_char( "    thirst hunger drunk full high tripping\n\r",	ch );
		send_to_char( "    gnosis humanity banality selfcontrol\n\r",	ch );
		send_to_char( "    rage blood glamour willpower conscience\n\r",ch );
		send_to_char( "    courage frenzy anger pain fear torpor\n\r",	ch );
		send_to_char( "    nature demeanor auspice breed gen\n\r", ch );
		send_to_char( "    ability power home\n\r", ch );
		return;
	}

	if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	/* clear zones for mobs */
	victim->zone = NULL;

	/*
	 * To allow for setting blood pools above 10.
	 */
	if(ch->race == race_lookup("vampire"))
		rbpg = 20;
	else
		rbpg = 10;

	/*
	 * Snarf the value (which need not be numeric).
	 */
	value = is_number( arg3 ) ? atoi( arg3 ) : -1;

	/*
	 * Set something.
	 */

	if ( !str_cmp( arg2, "exp" ) )
	{
		victim->exp = value;
		return;
	}

	if ( !str_cmp( arg2, "giftexp" ) )
	{
		victim->xpgift = value;
		return;
	}

	if ( !str_cmp( arg2, "cha" ) )
	{
		if ( !(value >= 1 && value <= 10) )
		{
			send_to_char(Format("Charisma range is 1 to %d.\n\r", 10),ch);
			return;
		}
		victim->perm_stat[STAT_CHA] = value;
		return;
	}

	if ( !str_cmp( arg2, "man" ) )
	{
		if ( !(value >= 1 && value <= 10) )
		{
			send_to_char(Format("Manipulation range is 1 to %d.\n\r", 10),ch);
			return;
		}
		victim->perm_stat[STAT_MAN] = value;
		return;
	}

	if ( !str_cmp( arg2, "app" ) )
	{
		if ( !(value >= 1 && value <= 10) )
		{
			send_to_char(Format("Appearance range is 1 to %d.\n\r", 10),ch);
			return;
		}
		victim->perm_stat[STAT_APP] = value;
		return;
	}

	if ( !str_cmp( arg2, "per" ) )
	{
		if ( !(value >= 1 && value <= 10) )
		{
			send_to_char(Format("Perception range is 1 to %d.\n\r", 10),ch);
			return;
		}
		victim->perm_stat[STAT_PER] = value;
		return;
	}

	if ( !str_cmp( arg2, "int" ) )
	{
		if ( !(value >= 1 && value <= 10) )
		{
			send_to_char(Format("Intelligence range is 1 to %d.\n\r", 10),ch);
			return;
		}

		victim->perm_stat[STAT_INT] = value;
		return;
	}

	if ( !str_cmp( arg2, "wit" ) )
	{
		if ( !(value >= 1 && value <= 10) )
		{
			send_to_char( Format("Wits range is 1 to %d.\n\r",10), ch );
			return;
		}

		victim->perm_stat[STAT_WIT] = value;
		return;
	}

	if ( !str_cmp( arg2, "str" ) )
	{
		if ( !(value >= 1 && value <= 10) )
		{
			send_to_char(Format("Strength range is 1 to %d.\n\r", 10),ch);
			return;
		}
		victim->perm_stat[STAT_STR] = value;
		return;
	}
	if ( !str_cmp( arg2, "dex" ) )
	{
		if ( !(value >= 1 && value <= 10) )
		{
			send_to_char( Format("Dexterity ranges is 1 to %d.\n\r", 10), ch );
			return;
		}

		victim->perm_stat[STAT_DEX] = value;
		return;
	}

	if ( !str_cmp( arg2, "sta" ) )
	{
		if ( !(value >= 1 && value <= 10) )
		{
			send_to_char( Format("Stamina range is 1 to %d.\n\r", 10), ch );
			return;
		}

		victim->perm_stat[STAT_STA] = value;
		return;
	}

	if ( !str_cmp( arg2, "conscience" ) )
	{
		if ( !(value >= 1 && value <= 10) )
		{
			send_to_char( Format("Conscience range is 1 to %d.\n\r", 10), ch );
			return;
		}

		victim->virtues[0] = value;
		return;
	}

	if ( !str_cmp( arg2, "selfcontrol" ) )
	{
		if ( !(value >= 1 && value <= 10) )
		{
			send_to_char( Format("Self-control range is 1 to %d.\n\r", 10), ch );
			return;
		}

		victim->virtues[1] = value;
		return;
	}

	if ( !str_cmp( arg2, "courage" ) )
	{
		if ( !(value >= 1 && value <= 10) )
		{
			send_to_char( Format("Courage range is 1 to %d.\n\r", 10), ch );
			return;
		}

		victim->virtues[2] = value;
		return;
	}

	if ( !str_cmp( arg2, "willpower" ) )
	{
		if ( !(value >= 1 && value <= 10) )
		{
			send_to_char( Format("Willpower range is 1 to %d.\n\r", 10), ch );
			return;
		}

		victim->willpower = value;
		victim->max_willpower = value;
		return;
	}

    if ( !str_cmp( arg2, "home" ) )
    {
	ROOM_INDEX_DATA *rm;
	if ( IS_NPC(victim) )
	{
	    send_to_char("Not on NPCs.\n\r", ch);
	    return;
	}

	if ( (rm = get_room_index(value)) == NULL )
	{
	    send_to_char( Format("Home cleared for %s\n\r", victim->name), ch );
	    victim->home = -1;
	    return;
	}

	victim->home = rm->vnum;
	send_to_char( Format("%s's home set to %s [%d]\n\r", victim->name, rm->name, rm->vnum), ch );
	return;
    }

    if ( !str_cmp( arg2, "gnosis" )
	|| !str_cmp( arg2, "humanity" )
	|| !str_cmp( arg2, "banality" ) )
    {
	if ( !(value >= 1 && value <= 10) )
	{
		send_to_char( Format("Range is 1 to %d.\n\r", 10), ch );
		return;
	}

	victim->GHB = value;
	victim->max_GHB = value;
	return;
    }

    if ( !str_cmp( arg2, "rage" )
	|| !str_cmp( arg2, "blood" )
	|| !str_cmp( arg2, "faith" )
	|| !str_cmp( arg2, "glamour" ) )
    {
	if ( !(value >= 1 && value <= rbpg) )
	{
	    send_to_char( Format("Range is 1 to %d.\n\r", rbpg), ch );
	    return;
	}

	victim->RBPG = value;
	victim->max_RBPG = value;
	return;
    }

    if( !str_cmp( arg2, "security" )) /* OLC */
    {
	if( IS_NPC( victim ))
	{
	    send_to_char( "Not available on NPC's.\n\r", ch );
	    return;
	}

	if (value > ch->pcdata->security || value < 0 )
	{
	    if( ch->pcdata->security != 0 )
	    {
		send_to_char( Format("Valid security is 0-%d.\n\r", ch->pcdata->security), ch );
	    }
	    else
	    {
		send_to_char( "Valid security is 0 only.\n\r", ch );
	    }
	    return;
	}
	victim->pcdata->security = value;
	return;
    }

    if ( !str_prefix( arg2, "sex" ) )
    {
	if ( value < 0 || value > 2 )
	{
	    send_to_char( "Sex range is 0 to 2.\n\r", ch );
	    return;
	}
	victim->sex = value;
	if (!IS_NPC(victim))
	    victim->pcdata->true_sex = value;
	return;
    }

    if ( !str_prefix( arg2, "clan" ) )
    {
	int clan;

	clan = clan_lookup(arg3);
	if ( clan == -1 )
	{
		char buf[MSL]={'\0'};

        	strncpy( buf, "Possible tribes, clans and kith are: ", sizeof(buf) );
        	for ( clan = 0; clan < MAX_CLAN; clan++ )
        	{
            	    if ( clan > 0 )
                    	strncat( buf, " ", sizeof(buf) - strlen(buf) - 1 );
            	    strncat( buf, clan_table[clan].name, sizeof(buf) - strlen(buf) - 1 );
        	}
            strncat( buf, ".\n\r", sizeof(buf) - strlen(buf) - 1 );

	    send_to_char(buf,ch);
	    return;
	}

	victim->clan = clan;
	return;
    }

    if ( !str_prefix( arg2, "dollars" ) )
    {
	victim->dollars = value;
	return;
    }

    if ( !str_prefix(arg2, "cents" ) )
    {
	victim->cents = value;
	return;
    }

    if ( !str_prefix( arg2, "thirst" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -50 || value > 100 )
	{
	    send_to_char( "Thirst range is -50 to 100.\n\r", ch );
	    return;
	}

	victim->condition[COND_THIRST] = value;
	return;
    }

    if ( !str_prefix( arg2, "drunk" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -50 || value > 100 )
	{
	    send_to_char( "Drunk range is -50 to 100.\n\r", ch );
	    return;
	}

	victim->condition[COND_DRUNK] = value;
	return;
    }

    if ( !str_prefix( arg2, "tripping" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -50 || value > 100 )
	{
	    send_to_char( "Tripping range is -50 to 100.\n\r", ch );
	    return;
	}

	victim->condition[COND_TRIPPING] = value;
	return;
    }

    if ( !str_prefix( arg2, "high" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -50 || value > 100 )
	{
	    send_to_char( "High range is -50 to 100.\n\r", ch );
	    return;
	}

	victim->condition[COND_HIGH] = value;
	return;
    }

    if ( !str_prefix( arg2, "frenzy" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -50 || value > 100 )
	{
	    send_to_char( "Frenzy range is -50 to 100.\n\r", ch );
	    return;
	}

	victim->condition[COND_FRENZY] = value;
	return;
    }

    if ( !str_prefix( arg2, "fear" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -50 || value > 100 )
	{
	    send_to_char( "Fear range is -50 to 100.\n\r", ch );
	    return;
	}

	victim->condition[COND_FEAR] = value;
	return;
    }

    if ( !str_prefix( arg2, "anger" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -50 || value > 100 )
	{
	    send_to_char( "Anger range is -50 to 100.\n\r", ch );
	    return;
	}

	victim->condition[COND_ANGER] = value;
	return;
    }

    if ( !str_prefix( arg2, "pain" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -50 || value > 100 )
	{
	    send_to_char( "Pain range is -50 to 100.\n\r", ch );
	    return;
	}

	victim->condition[COND_PAIN] = value;
	return;
    }

    if ( !str_prefix( arg2, "full" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -50 || value > 100 )
	{
	    send_to_char( "Full range is -50 to 100.\n\r", ch );
	    return;
	}

	victim->condition[COND_FULL] = value;
	return;
    }

    if ( !str_prefix( arg2, "hunger" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < -50 || value > 100 )
        {
            send_to_char( "Full range is -50 to 100.\n\r", ch );
            return;
        }

        victim->condition[COND_HUNGER] = value;
        return;
    }

    if (!str_prefix( arg2, "race" ) )
    {
	int race;

	race = race_lookup(arg3);

	if ( race == 0)
	{
	    send_to_char("That is not a valid race.\n\r",ch);
	    return;
	}

	if (!IS_NPC(victim) && !race_table[race].pc_race)
	{
	    send_to_char("That is not a valid player race.\n\r",ch);
	    return;
	}

	victim->race = race;
	return;
    }

    if (!str_prefix(arg2,"group"))
    {
	if (!IS_NPC(victim))
	{
	    send_to_char("Only on NPCs.\n\r",ch);
	    return;
	}
	victim->group = value;
	return;
    }

    if(!str_prefix(arg2, "nature"))
    {
	if(flag_lookup(arg3, personality_archetypes) >= 0)
	{
		PURGE_DATA(victim->nature);
	    victim->nature = str_dup(arg3);
	}
	return;
    }

    if(!str_prefix(arg2, "demeanor"))
    {
	if(flag_lookup(arg3, personality_archetypes) >= 0)
	{
		PURGE_DATA(victim->nature);
	    victim->demeanor = str_dup(arg3);
	}
	return;
    }

    if (!str_prefix( arg2, "breed" ) )
    {
    	int breed;

    	if ((breed = breed_lookup(arg3)) < 0)
    	{
    		send_to_char("That is not a valid breed.\n\r",ch);
    		return;
    	}

    	victim->breed = breed;
    	return;
    }

    if (!str_prefix( arg2, "auspice" ) )
    {
    	int auspice;

    	if ((auspice = auspice_lookup(arg3)) < 0)
    	{
    		send_to_char("That is not a valid auspice.\n\r",ch);
    		return;
    	}

    	victim->auspice = auspice;
    	return;
    }

    if ( !str_cmp( arg2, "gen" ) )
    {
    	if ( !(value >= 5 && value <= 13) )
    	{
    		send_to_char("Generation range is 5 to 13.\n\r",ch);
    		return;
    	}
    	victim->gen = value;
    	return;
    }

    if ( !str_cmp( arg2, "bgtime") )
    {
    	if ( !(value > 0 && value < 200) )
    	{
    		send_to_char("Timer range is 0 to 200.\n\r",ch);
    		return;
    	}
    	victim->bg_timer = value;
    	return;
    }

    if ( !str_prefix( arg2, "abilities" ) || !str_prefix( arg2, "ability" ) )
    {
    	argument = one_argument(argument, arg3);
    	if( (rbpg = abil_lookup(arg3, victim)) < 0 )
    	{
    		send_to_char("That's not an ability.\n\r", ch);
    		return;
    	}
    	value = atoi(argument);
    	if(!is_number(argument) || value > 10 || value < 0 )
    	{
    		send_to_char("Abilities must be set to a value from 1-10\n\r", ch);
    		return;
    	}
    	victim->ability[rbpg].value = value;
    	return;
    }

    if ( !str_prefix( arg2, "powers" ) || !str_prefix( arg2, "disciplines" ) )
    {
    	argument = one_argument(argument, arg3);
    	if( (rbpg = disc_lookup(victim, arg3)) < 0 )
    	{
    		send_to_char("That's not a power.\n\r", ch);
    		return;
    	}
    	value = atoi(argument);
    	if(!is_number(argument) || value > 10 || value < 0 )
    	{
    		send_to_char("Powers must be set to a value from 1-10\n\r", ch);
    		return;
    	}
    	victim->disc[rbpg] = value;
    	return;
    }

    if ( !str_prefix( arg2, "backgrounds" ) )
    {
	argument = one_argument(argument, arg3);
	if( (rbpg = background_lookup(arg3)) < 0 )
	{
	    send_to_char("That's not a background.\n\r", ch);
	    return;
	}
	value = atoi(argument);
	if(!is_number(argument) || value > 5 || value < 0 )
	{
	    send_to_char("Backgrounds must be set to a value from 1-5\n\r", ch);
	    return;
	}
	victim->backgrounds[rbpg] = value;
	return;
    }

    if ( !str_prefix( arg2, "warrants" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 0 || value > 1000 )
        {
            send_to_char( "Warrant range is 0 to 1000.\n\r", ch );
            return;
        }

        victim->warrants = value;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    /*
     * Generate usage message.
     */
    do_function(ch, &do_mset, "" );
    return;
}

void do_string( CHAR_DATA *ch, char *argument )
{
	char type [MIL]={'\0'};
	char arg1 [MIL]={'\0'};
	char arg2 [MIL]={'\0'};
	char arg3 [MIL]={'\0'};
	CHAR_DATA *victim;
	OBJ_DATA *obj;

	CheckCH(ch);

	smash_tilde( argument );
	argument = one_argument( argument, type );
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	strncpy( arg3, argument, sizeof(arg3) );

	if ( IS_NULLSTR(type) || IS_NULLSTR(arg1) || IS_NULLSTR(arg2) || IS_NULLSTR(arg3) )
	{
		send_to_char("Syntax:\n\r",ch);
		send_to_char("  string char <name> <field> <string>\n\r",ch);
		send_to_char("    fields: name short long desc title spec ghouled_by\n\r", ch);
		send_to_char("  string obj  <name> <field> <string>\n\r",ch);
		send_to_char("    fields: name short long extended\n\r",ch);
		return;
	}

    if (!str_prefix(type,"character") || !str_prefix(type,"mobile"))
    {
    	if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
    	}

	/* clear zone for mobs */
	victim->zone = NULL;

	/* string something */

     	if ( !str_prefix( arg2, "name" ) )
    	{
	    if ( !IS_NPC(victim) )
	    {
	    	send_to_char( "Not on PC's.\n\r", ch );
	    	return;
	    }
	    PURGE_DATA( victim->name );
	    victim->name = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "description" ) )
    	{
    		PURGE_DATA(victim->description);
    	    victim->description = str_dup(arg3);
    	    return;
    	}

    	if ( !str_prefix( arg2, "short" ) )
    	{
    		PURGE_DATA( victim->short_descr );
    		victim->short_descr = str_dup( arg3 );
    		return;
    	}

    	if ( !str_prefix( arg2, "long" ) )
    	{
    		PURGE_DATA( victim->long_descr );
    		strncat(arg3,"\n\r", sizeof(arg3) - strlen(arg3) - 1 );
    		victim->long_descr = str_dup( arg3 );
    		return;
    	}

    	if ( !str_prefix( arg2, "title" ) )
    	{
	    if ( IS_NPC(victim) )
	    {
	    	send_to_char( "Not on NPC's.\n\r", ch );
	    	return;
	    }

	    set_title( victim, arg3 );
	    return;
    	}

    }

    if (!str_prefix(type,"object"))
    {
    	/* string an obj */

   	if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
    	{
	    send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
	    return;
    	}

        if ( !str_prefix( arg2, "name" ) )
    	{
        	PURGE_DATA( obj->name );
	    obj->name = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "short" ) )
    	{
    		PURGE_DATA( obj->short_descr );
	    obj->short_descr = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "long" ) )
    	{
    		PURGE_DATA( obj->description );
	    obj->description = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "ed" ) || !str_prefix( arg2, "extended"))
    	{
	    EXTRA_DESCR_DATA *ed;

	    argument = one_argument( argument, arg3 );
	    if ( argument == NULL )
	    {
	    	send_to_char( "Syntax: oset <object> ed <keyword> <string>\n\r",
		    ch );
	    	return;
	    }

 	    strncat(argument,"\n\r", sizeof(*argument));

	    ed = new_extra_descr();

	    PURGE_DATA( ed->keyword );
	    PURGE_DATA( ed->description );
	    ed->keyword		= str_dup( arg3     );
	    ed->description	= str_dup( argument );
	    ed->next		= obj->extra_descr;
	    obj->extra_descr	= ed;
	    return;
    	}
    }


    /* echo bad use message */
    do_function(ch, &do_string, "" );
}



void do_oset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MIL]={'\0'};
    char arg2 [MIL]={'\0'};
    char arg3 [MIL]={'\0'};
    OBJ_DATA *obj;
    int value = 0;

    CheckCH(ch);

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strncpy( arg3, argument, sizeof(arg3) );

    if ( IS_NULLSTR(arg1) || IS_NULLSTR(arg2) || IS_NULLSTR(arg3) )
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  set obj <object> <field> <value>\n\r",ch);
	send_to_char("  Field being one of:\n\r",				ch );
	send_to_char("    value0 value1 value2 value3 value4 (v1-v4)\n\r",	ch );
	send_to_char("    extra wear level weight cost timer\n\r",		ch );
	return;
    }

    if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
	return;
    }

    /*
     * Snarf the value (which need not be numeric).
     */
    value = atoi( arg3 );

    /*
     * Set something.
     */
    if ( !str_cmp( arg2, "value0" ) || !str_cmp( arg2, "v0" ) )
    {
	obj->value[0] = UMIN(50,value);
	return;
    }

    if ( !str_cmp( arg2, "value1" ) || !str_cmp( arg2, "v1" ) )
    {
	obj->value[1] = value;
	return;
    }

    if ( !str_cmp( arg2, "value2" ) || !str_cmp( arg2, "v2" ) )
    {
	obj->value[2] = value;
	return;
    }

    if ( !str_cmp( arg2, "value3" ) || !str_cmp( arg2, "v3" ) )
    {
	obj->value[3] = value;
	return;
    }

    if ( !str_cmp( arg2, "value4" ) || !str_cmp( arg2, "v4" ) )
    {
	obj->value[4] = value;
	return;
    }

    if ( !str_prefix( arg2, "extra" ) )
    {
	obj->extra_flags = value;
	return;
    }

    if ( !str_prefix( arg2, "wear" ) )
    {
	if( ( value = flag_value( wear_flags, argument ) ) != NO_FLAG )
	{
	    TOGGLE_BIT(obj->wear_flags, value);
	    send_to_char("Wear flag toggled.\n\r", ch);
	}
	else
	    send_to_char("Wear flag(s) not found.\n\r", ch);
	return;
    }

    if ( !str_prefix( arg2, "weight" ) )
    {
	obj->weight = value;
	return;
    }

    if ( !str_prefix( arg2, "cost" ) )
    {
	obj->cost = value;
	return;
    }

    if ( !str_prefix( arg2, "timer" ) )
    {
	obj->timer = value;
	return;
    }

    /*
     * Generate usage message.
     */
    do_function(ch, &do_oset, "" );
    return;
}



void do_rset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MIL]={'\0'};
    char arg2 [MIL]={'\0'};
    char arg3 [MIL]={'\0'};
    ROOM_INDEX_DATA *location;
    int value = 0;

    CheckCH(ch);

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strncpy( arg3, argument, sizeof(arg3) );

    if ( IS_NULLSTR(arg1) || IS_NULLSTR(arg2) || IS_NULLSTR(arg3) )
    {
	send_to_char( "Syntax:\n\r",ch);
	send_to_char( "  set room <location> <field> <value>\n\r",ch);
	send_to_char( "  Field being one of:\n\r",			ch );
	send_to_char( "    flags sector\n\r",				ch );
	return;
    }

    if ( ( location = find_location( ch, arg1 ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if (!is_room_owner(ch,location) && ch->in_room != location
    &&  room_is_private(location) && !IS_TRUSTED(ch,MASTER))
    {
        send_to_char("That room is private right now.\n\r",ch);
        return;
    }

    /*
     * Snarf the value.
     */
    if ( !is_number( arg3 ) && !str_prefix( arg2, "owner" ) )
    {
	send_to_char( "Value must be numeric.\n\r", ch );
	return;
    }
    if( is_number( arg3 ) ) value = atoi( arg3 );

    /*
     * Set something.
     */
    if ( !str_prefix( arg2, "flags" ) )
    {
	location->room_flags	= value;
	return;
    }

    if ( !str_prefix( arg2, "sector" ) )
    {
	location->sector_type	= value;
	return;
    }

    if ( !str_prefix( arg2, "owner" ) )
    {
	/* @@@@@ Needs to be re-worked with get_player, online checks and
	 * all the fun stuff that goes with loading fake players and dropping
	 * them off again. Currently not checking argument.
	 */
    	PURGE_DATA(location->owner);
	location->owner		= str_dup(arg3);
	send_to_char("In order for this update to stick, the area needs to be saved.\n\r", ch);
	return;
    }

    /*
     * Generate usage message.
     */
    do_function(ch, &do_rset, "" );
    return;
}


void do_sockets( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA       *vch;
	DESCRIPTOR_DATA *d;
	char            buf2 [ MSL ]={'\0'};
	int             count = 0;
	char *          st;
	char            s[100]={'\0'};
	char            idle[10]={'\0'};

	CheckCH(ch);

	strncat( buf2, "\n\r\tW[Num Connected_State Login@ Idl] Player Name Host\tn\n\r", sizeof(buf2) - strlen(buf2) - 1 );
	strncat( buf2, "\tW--------------------------------------------------------------------------\tn\n\r", sizeof(buf2) - strlen(buf2) - 1 );
	for ( d = descriptor_list; d; d = d->next )
	{
		if ( d->character && can_see( ch, d->character ) )
		{
			if(d->connected > MAX_CON_STATE) st = "   !UNKNOWN!   ";
			else st = con_state_flags[d->connected].name;
			count++;

			/* Format "login" value... */
			vch = d->original ? d->original : d->character;
			strftime( s, 100, "%I:%M%p", localtime( &vch->logon ) );

			if ( vch->timer > 0 )
				snprintf( idle, sizeof(idle), "%-2d", vch->timer );
			else
				snprintf( idle, sizeof(idle), "  " );

			strncat( buf2, Format("\tY[%3d %s %7s %2s] %-12s %-32.32s\tn\n\r",
					d->descriptor, st, s, idle, ( d->original ) ? d->original->name : ( d->character )  ? d->character->name : "(None!)",
									  d->host), sizeof(buf2) - strlen(buf2) - 1 );

		}
	}

	strncat( buf2, Format("\n\r\tY%d user%s\tn\n\r", count, count == 1 ? "" : "s"), sizeof(buf2) - strlen(buf2) - 1 );
	send_to_char( buf2, ch );
	return;
}


void do_protocol( CHAR_DATA *ch, char *argument )
{
	DESCRIPTOR_DATA *d;

	CheckCH(ch);

	send_to_char( Format("%-15s | %-15s | %-15s | %-15s\n\r", "User", "Client", "MXP", "MSDP"), ch);
	send_to_char("---------------------------------------------------------------------\n\r", ch);

	for ( d = descriptor_list; d != NULL; d = d->next )
	{
		send_to_char( Format("%-15s | ", ( d->original ) ? d->original->name : ( d->character )  ? d->character->name : "(None!)"), ch);
		send_to_char( Format("%-15s | ", d->pProtocol->pVariables[eMSDP_CLIENT_ID]->pValueString), ch);

		if (d !=NULL && d->pProtocol && d->pProtocol->pVariables[eMSDP_MXP]->ValueInt == 1)
			send_to_char( Format("%-15s | ", "Yes"), ch);
		else
			send_to_char( Format("%-15s | ", "No"), ch);

		if (d !=NULL && d->pProtocol && d->pProtocol->bMSDP == 1)
			send_to_char( Format("%-15s\n\r", "Yes"), ch);
		else
			send_to_char( Format("%-15s\n\r", "No"), ch);

	}

	return;
}

/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
void do_force( CHAR_DATA *ch, char *argument )
{
	char buf[MSL]={'\0'};
	char arg[MIL]={'\0'};
	char arg2[MIL]={'\0'};

	CheckCH(ch);

	argument = one_argument( argument, arg );

	if ( IS_NULLSTR(arg) || IS_NULLSTR(argument) )
	{
		send_to_char( "Force whom to do what?\n\r", ch );
		return;
	}

	one_argument(argument,arg2);

	if (!str_cmp(argument,"delete")
			|| !str_prefix(argument, "switch")
			|| !str_prefix(argument, "being")
			|| !str_prefix(argument,"mob"))
	{
		send_to_char("That will NOT be done.\n\r",ch);
		return;
	}

	snprintf( buf, sizeof(buf), "$n forces you to '%s'.", argument );

	if ( !str_cmp( arg, "all" ) )
	{
		DESCRIPTOR_DATA *desc,*desc_next;

		if (get_trust(ch) < MAX_LEVEL - 1) {
			send_to_char("Not at your level!\n\r",ch);
			return;
		}

		for ( desc = descriptor_list; desc != NULL; desc = desc_next ) {
			desc_next = desc->next;

			if (desc->connected==CON_PLAYING &&
					get_trust( desc->character ) < get_trust( ch ) ) {
				act( buf, ch, NULL, desc->character, TO_VICT, 1 );
				interpret( desc->character, argument );
			}
		}
	}
	else if (!str_cmp(arg,"players"))
	{
		DESCRIPTOR_DATA *desc,*desc_next;

		if (get_trust(ch) < MAX_LEVEL - 2) {
			send_to_char("Not at your level!\n\r",ch);
			return;
		}

		for ( desc = descriptor_list; desc != NULL; desc = desc_next ) {
			desc_next = desc->next;

			if (desc->connected==CON_PLAYING &&
					get_trust( desc->character ) < get_trust( ch ) ) {
				act( buf, ch, NULL, desc->character, TO_VICT, 1 );
				interpret( desc->character, argument );
			}
		}
	}
	else if (!str_cmp(arg,"gods"))
	{
		DESCRIPTOR_DATA *desc,*desc_next;

		if (get_trust(ch) < MAX_LEVEL) {
			send_to_char("Not at your level!\n\r",ch);
			return;
		}

		for ( desc = descriptor_list; desc != NULL; desc = desc_next ) {
			desc_next = desc->next;

			if (desc->connected==CON_PLAYING &&
					get_trust( desc->character ) < get_trust( ch ) ) {
				act( buf, ch, NULL, desc->character, TO_VICT, 1 );
				interpret( desc->character, argument );
			}
		}
	}
	else
	{
		CHAR_DATA *victim;

		if ( ( victim = get_char_world( ch, arg ) ) == NULL )
		{
			send_to_char( "They aren't here.\n\r", ch );
			return;
		}

		if ( victim == ch )
		{
			send_to_char( "Massochist! Just type the command.\n\r", ch );
			return;
		}

		if (!is_room_owner(ch,victim->in_room)
				&&  ch->in_room != victim->in_room
				&&  room_is_private(victim->in_room) && !IS_TRUSTED(ch,MASTER))
		{
			send_to_char("That character is in a private room.\n\r",ch);
			return;
		}

		if ( get_trust( victim ) >= get_trust( ch ) )
		{
			send_to_char( "Do it yourself!\n\r", ch );
			return;
		}

		if ( !IS_NPC(victim) && get_trust(ch) < MAX_LEVEL -3)
		{
			send_to_char("Not at your level!\n\r",ch);
			return;
		}

		act( buf, ch, NULL, victim, TO_VICT, 1 );
		interpret( victim, argument );
	}

	send_to_char( "Ok.\n\r", ch );
	return;
}



/*
 * New routines by Dionysos.
 */
void do_invis( CHAR_DATA *ch, char *argument )
{
	int level = 0;
	char arg[MSL]={'\0'};

	CheckCH(ch);

	/* RT code for taking a level argument */
	one_argument( argument, arg );

	if ( IS_NULLSTR(arg) )
		/* take the default path */

		if ( ch->invis_level)
		{
			ch->invis_level = 0;
			act( "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM, 0 );
			send_to_char( "You slowly fade back into existence.\n\r", ch );
		}
		else
		{
			ch->invis_level = get_trust(ch);
			act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM, 0 );
			send_to_char( "You slowly vanish into thin air.\n\r", ch );
		}
	else
		/* do the level thing */
	{
		if(is_number(arg))
			level = atoi(arg);
		else
			level = flag_lookup(arg, staff_status);
		if (level < 2 || level > get_trust(ch))
		{
			send_to_char("Invis level must be between 2 and your level.\n\r",ch);
			return;
		}
		else
		{
			ch->reply = NULL;
			ch->invis_level = level;
			act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM, 0 );
			send_to_char( "You slowly vanish into thin air.\n\r", ch );
		}
	}

	return;
}


void do_incognito( CHAR_DATA *ch, char *argument )
{
	int level = 0;
	char arg[MSL]={'\0'};

	CheckCH(ch);

	/* RT code for taking a level argument */
	one_argument( argument, arg );

	if ( IS_NULLSTR(arg) )
		/* take the default path */

		if ( ch->incog_level)
		{
			ch->incog_level = 0;
			act( "$n is no longer cloaked.", ch, NULL, NULL, TO_ROOM, 0 );
			send_to_char( "You are no longer cloaked.\n\r", ch );
		}
		else
		{
			ch->incog_level = get_trust(ch);
			act( "$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM, 0 );
			send_to_char( "You cloak your presence.\n\r", ch );
		}
	else
		/* do the level thing */
	{
		if(is_number(arg))
			level = atoi(arg);
		else
			level = flag_lookup2(arg, staff_status);
		if (level < 2 || level > get_trust(ch))
		{
			send_to_char("Incog level must be between 2 and your level.\n\r",ch);
			return;
		}
		else
		{
			ch->reply = NULL;
			ch->incog_level = level;
			act( "$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM, 0 );
			send_to_char( "You cloak your presence.\n\r", ch );
		}
	}

	return;
}



void do_holylight( CHAR_DATA *ch, char *argument )
{
	CheckCH(ch);
	CheckChNPC(ch);

	if ( IS_SET(ch->plr_flags, PLR_HOLYLIGHT) )
	{
		REMOVE_BIT(ch->plr_flags, PLR_HOLYLIGHT);
		send_to_char( "Holy light mode off.\n\r", ch );
	}
	else
	{
		SET_BIT(ch->plr_flags, PLR_HOLYLIGHT);
		send_to_char( "Holy light mode on.\n\r", ch );
	}

	return;
}

/* prefix command: it will put the string typed on each line typed */

void do_prefi (CHAR_DATA *ch, char *argument)
{
	CheckCH(ch);
	send_to_char("You cannot abbreviate the prefix command.\r\n",ch);
	return;
}

void do_prefix (CHAR_DATA *ch, char *argument)
{
	CheckCH(ch);

	if (IS_NULLSTR(argument))
	{
		if (ch->prefix[0] == '\0')
		{
			send_to_char("You have no prefix to clear.\r\n",ch);
			return;
		}

		send_to_char("Prefix removed.\r\n",ch);
		PURGE_DATA(ch->prefix);
		ch->prefix = NULL;
		return;
	}

	if (!IS_NULLSTR(ch->prefix))
	{
		send_to_char(Format("Prefix changed to %s.\r\n",argument), ch);
		PURGE_DATA(ch->prefix);
	}
	else
	{
		send_to_char(Format("Prefix set to %s.\r\n",argument), ch);
	}

	PURGE_DATA( ch->prefix );
	ch->prefix = str_dup(argument);
}


void do_addlag(CHAR_DATA *ch, char *argument)
{

	CHAR_DATA *victim;
	char arg1[MSL]={'\0'};
	int x = 0;

	CheckCH(ch);

	argument = one_argument(argument, arg1);

	if (IS_NULLSTR(arg1))
	{
		send_to_char("addlag to who?", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg1)) == NULL)
	{
		send_to_char("They're not here.", ch);
		return;
	}

	if ((x = atoi(argument)) <= 0)
	{
		send_to_char("That makes a LOT of sense.", ch);
		return;
	}

	WAIT_STATE(victim, x);
	send_to_char("\tOAdding lag now...\tn\n\r", ch);
	return;
}

extern int port,control; /* db.c */

void do_copyover (CHAR_DATA *ch, char * argument)
{
	FILE *fp;
	DESCRIPTOR_DATA *d, *d_next;
	char buf [100]={'\0'};
	extern int port, control;

	CheckCH(ch);

	fp = fopen (COPYOVER_FILE, "w");

	if (!fp)
	{
		send_to_char ("Transition file not writeable, aborted.\n\r",ch);
		log_string(LOG_ERR, Format("Could not write to transition file: %s", COPYOVER_FILE));
		perror ("do_copyover:fopen");
		return;
	}

	//	 Consider changing all saved areas here, if you use OLC
	do_asave (NULL, "changed"); //- autosave changed areas

	snprintf (buf, sizeof(buf), "\n\r *** Warm Boot initiated by %s.\n\rWarm fuzzy feelings all around!", ch->name);

	//	 For each playing descriptor, save its state
	for (d = descriptor_list; d ; d = d_next)
	{
		CHAR_DATA * och = CH (d);
		d_next = d->next;  //We delete from the list , so need to save this

		if (!d->character || d->connected > CON_PLAYING) // drop those logging on
		{
			write_to_descriptor (d->descriptor, "\n\rSorry, we are rebooting. Come back in a few minutes.\n\r", 0);
			close_socket (d);  //throw'em out
		}
		else
		{
			fprintf (fp, "%d %s %s\n", d->descriptor, och->name, d->host);

			save_char_obj (och);
			write_to_descriptor (d->descriptor, buf, 0);
		}
	}

	fprintf (fp, "-1\n");
	fclose (fp);

	//	 Close reserve and other always-open files and release other resources
	closeReserve();

	//	shutdown_web();

	//	 exec - descriptors are inherited
	execl (EXE_FILE, "Project", (char *)Format("%d", port), Format("%d", atoi(logfile) + 1), "copyover", (char *)Format("%d", control),  (char *) NULL);

	//	 Failed - sucessful exec will not return
	perror ("do_copyover: execl");
	send_to_char ("Warm reboot FAILED!\n\r",ch);

	//	 Here you might want to reopen fpReserve
	openReserve();
}

// Recover from a copyover - load players
void copyover_recover ()
{
	DESCRIPTOR_DATA *d;
	FILE *fp;
	char name [100]={'\0'};
	char host[MSL]={'\0'};
	int desc = 0;
	bool fOld = FALSE;

	log_string(LOG_CONNECT,"Warm reboot recovery initiated");

	fp = fopen (COPYOVER_FILE, "r");

	if (!fp)  //there are some descriptors open which will hang forever then ?
	{
		perror ("copyover_recover:fopen");
		log_string(LOG_ERR, "Unable to complete the hotreboot process, suspect not a hotreboot.  Will continue without looking for players.");
		return;
	}

	unlink (COPYOVER_FILE);  //In case something crashes - doesn't prevent reading

	for (;;)
	{
		if (fscanf (fp, "%d %s %s\n", &desc, name, host) == -1)
        {
            log_string(LOG_ERR,"copyover_recover: error");
            return;
        }

		// fscanf (fp, "%d %s %s\n", &desc, name, host);
		if (desc == -1)
			break;

	//	 Write something, and check if it goes error-free
		if (!write_to_descriptor (desc, "\n\rSystem loading...\n\r",0))
		{
			close (desc); // nope
			continue;
		}

		d = new_descriptor();
		d->descriptor = desc;

		PURGE_DATA( d->host );
		d->host = str_dup (host);

		LINK_SINGLE(d, next, descriptor_list);

		d->connected = CON_COPYOVER_RECOVER;  //-15, so close_socket frees the char


	//	 Now, find the pfile

		fOld = load_char_obj (d, name, TRUE, FALSE, FALSE);

		if (!fOld) // Player file not found?!
		{
			write_to_descriptor (desc, "\n\rSomehow, your character was lost in the transition. Sorry.\n\r", 0);
			close_socket (d);
		}
		else  //ok!
		{
			write_to_descriptor (desc, "\n\rWarm boot completed.\n\rSystem functioning normally.\n\r\n\r",0);

			// Just In Case
			if (!d->character->in_room)
				d->character->in_room = get_room_index (ROOM_VNUM_START);

			// Insert in the char_list
			LINK_SINGLE(d->character, next, char_list);

			char_to_room (d->character, d->character->in_room);
			d->connected = CON_PLAYING;

			if(d->character->pet != NULL)
			{
				char_to_room(d->character->pet, d->character->in_room);
			}
			do_function(d->character, &do_look, "" );
		}

	}

	fclose (fp);
	fp = NULL;
}

void do_timestop ( CHAR_DATA *ch, char * argument )
{
	CheckCH(ch);

	if (timestop)
	{
		send_to_char("You release time to pass as it should.\n\r", ch);
		timestop = FALSE;
	}
	else
	{
		send_to_char("You slow time to a stop!\n\r", ch);
		timestop = TRUE;
	}
}

const char wizutil_id [] = "$Id: wizutil.c,v 1.6 1996/01/04 21:30:45 root Exp root $";

/*
===========================================================================
This snippet was written by Erwin S. Andreasen, erwin@pip.dknet.dk. You may
use this code freely, as long as you retain my name in all of the files. You
also have to mail me telling that you are using it. I am giving this,
hopefully useful, piece of source code to you for free, and all I require
from you is some feedback.

Please mail me if you find any bugs or have any new ideas or just comments.

All my snippets are publically available at:

http://pip.dknet.dk/~pip1773/

If you do not have WWW access, try ftp'ing to pip.dknet.dk and examine
the /pub/pip1773 directory.
===========================================================================

  Various administrative utility commands.
  Version: 3 - Last update: January 1996.

  To use these 2 commands you will have to add a filename field to AREA_DATA.
  This value can be found easily in load_area while booting - the filename
  of the current area boot_db is reading from is in the strArea global.

  Since version 2 following was added:

  A rename command which renames a player. Search for do_rename to see
  more info on it.

  A FOR command which executes a command at/on every player/mob/location.

  Fixes since last release: None.


*/


/* To have VLIST show more than vnum 0 - 9900, change the number below: */

#define MAX_SHOW_VNUM   999999 /* show only 1 - 100*100 */

#define NUL '\0'


extern ROOM_INDEX_DATA *       room_index_hash        [MAX_KEY_HASH]; /* db.c */

/* opposite directions */
const sh_int opposite_dir [6] = { DIR_SOUTH, DIR_WEST, DIR_NORTH, DIR_EAST, DIR_DOWN, DIR_UP };


/* get the 'short' name of an area (e.g. MIDGAARD, MIRROR etc. */
/* assumes that the filename saved in the AREA_DATA struct is something like midgaard.are */
char * area_name (AREA_DATA *pArea)
{
	static char buffer[64]={'\0'}; /* short filename */
	char  *period;

	// assert (pArea != NULL);

	strncpy (buffer, pArea->file_name, 64); /* copy the filename */
	period = strchr (buffer, '.'); /* find the period (midgaard.are) */
	if (period) /* if there was one */
		*period = '\0'; /* terminate the string there (midgaard) */

	return buffer;
}

typedef enum {exit_from, exit_to, exit_both} exit_status;

/* depending on status print > or < or <> between the 2 rooms */
void room_pair (ROOM_INDEX_DATA* left, ROOM_INDEX_DATA* right, exit_status ex, char *buffer)
{
	char *sExit;

	switch (ex)
	{
		default:
		sExit = "??"; break; /* invalid usage */
		case exit_from:
		sExit = "< "; break;
		case exit_to:
		sExit = " >"; break;
		case exit_both:
		sExit = "<>"; break;
	}

	// snprintf (buffer, sizeof(buffer), "%d %-26s %s%d %-26s(%-8.8s)\n\r",
	// 		left->vnum, left->name, sExit, right->vnum, right->name, area_name(right->area)
	// snprintf (buffer, sizeof(buffer), "%d %s %d (%-8.8s)\n\r", left->vnum, sExit, right->vnum, area_name(right->area));
	sprintf (buffer, "%d %s %d (%-8.8s)\n\r", left->vnum, sExit, right->vnum, area_name(right->area));
}

/* for every exit in 'room' which leads to or from pArea but NOT both, print it */
void checkexits (ROOM_INDEX_DATA *room, AREA_DATA *pArea, char* buffer)
{
	EXIT_DATA *exit;
	ROOM_INDEX_DATA *to_room;
	char buf[MSL]={'\0'};
	int i = 0;

	strncpy (buffer, "", sizeof(*buffer));
	for (i = 0; i < 6; i++)
	{
		exit = room->exit[i];
		if (!exit)
		{
			continue;
		}
		else
		{
			to_room = exit->u1.to_room;
		}

		if (to_room)  /* there is something on the other side */
		{
			if ( (room->area == pArea) && (to_room->area != pArea) )
			{ /* an exit from our area to another area */
			  /* check first if it is a two-way exit */

				if ( to_room->exit[opposite_dir[i]] &&
					to_room->exit[opposite_dir[i]]->u1.to_room == room )
					room_pair (room,to_room,exit_both,buf); /* <> */
				else
					room_pair (room,to_room,exit_to,buf); /* > */

				strncat (buffer, buf, sizeof(*buffer));
			}
			else
			if ( (room->area != pArea) && (exit->u1.to_room->area == pArea) )
			{ /* an exit from another area to our area */

				if  (!
			    	 (to_room->exit[opposite_dir[i]] &&
				      to_room->exit[opposite_dir[i]]->u1.to_room == room )
					)
				/* two-way exits are handled in the other if */
				{
					room_pair (to_room,room,exit_from,buf);
					strncat (buffer, buf, sizeof(*buffer));
				}

			} /* if room->area */
		}
	} /* for */

}

/* for now, no arguments, just list the current area */
// This command has been replaced by do_arealinks.
// void do_exlist (CHAR_DATA *ch, char * argument)
// {
// 	AREA_DATA* pArea;
// 	ROOM_INDEX_DATA* room;
// 	int i = 0;
// 	char buffer[MSL]={'\0'};

// 	CheckCH(ch);

// 	pArea = ch->in_room->area;			/* this is the area we want info on */
// 	for (i = 0; i < MAX_KEY_HASH; i++)	/* room index hash table */
// 		for (room = room_index_hash[i]; room != NULL; room = room->next)
// 			/* run through all the rooms on the MUD */
// 		{
// 			checkexits (room, pArea, buffer);
// 			send_to_char (buffer, ch);
// 		}
// }

/* show a list of all used VNUMS */

#define COLUMNS 		5   /* number of columns */
#define MAX_ROW 		((MAX_SHOW_VNUM / COLUMNS)+1) /* rows */

void do_vlist (CHAR_DATA *ch, char *argument)
{
	ROOM_INDEX_DATA *room;
	char buffer[MAX_ROW*100]={'\0'}; /* should be plenty */
	int i = 0;
	int j = 0;
	int vnum = 0;

	CheckCH(ch);

	for (i = 0; i < MAX_ROW; i++)
	{
		strncpy (buffer, "", sizeof(buffer)); /* clear the buffer for this row */

		for (j = 0; j < COLUMNS; j++) /* for each column */
		{
			vnum = ((j*MAX_ROW) + i); /* find a vnum whih should be there */
			if (vnum < MAX_SHOW_VNUM)
			{
				room = get_room_index (vnum * 100 + 1); /* each zone has to have a XXX01 room */
				strncat (buffer, Format("%3d %-8.8s  ", vnum, room ? area_name(room->area) : "-"), sizeof(buffer));
			}
		} /* for columns */

		send_to_char (buffer,ch);
		send_to_char ("\n\r",ch);
	} /* for rows */
}

/* Super-AT command:

FOR ALL <action>
FOR MORTALS <action>
FOR GODS <action>
FOR MOBS <action>
FOR EVERYWHERE <action>


Executes action several times, either on ALL players (not including yourself),
MORTALS (including trusted characters), GODS (characters with level higher than
L_HERO), MOBS (Not recommended) or every room (not recommended either!)

If you insert a # in the action, it will be replaced by the name of the target.

If # is a part of the action, the action will be executed for every target
in game. If there is no #, the action will be executed for every room containg
at least one target, but only once per room. # cannot be used with FOR EVERY-
WHERE. # can be anywhere in the action.

Example:

FOR ALL SMILE -> you will only smile once in a room with 2 players.
FOR ALL TWIDDLE # -> In a room with A and B, you will twiddle A then B.

Destroying the characters this command acts upon MAY cause it to fail. Try to
avoid something like FOR MOBS PURGE (although it actually works at my MUD).

FOR MOBS TRANS 3054 (transfer ALL the mobs to Midgaard temple) does NOT work
though :)

The command works by transporting the character to each of the rooms with
target in them. Private rooms are not violated.

*/

/* Expand the name of a character into a string that identifies THAT
   character within a room. E.g. the second 'guard' -> 2. guard
*/
const char * name_expand (CHAR_DATA *ch)
{
	CHAR_DATA *rch;
	int count = 1;
	char name[MIL]={'\0'}; /*  HOPEFULLY no mob has a name longer than THAT */

	static char outbuf[MIL]={'\0'};

	if (!IS_NPC(ch))
		return ch->name;

	one_argument (ch->name, name); /* copy the first word into name */

	if (!name[0]) /* weird mob .. no keywords */
	{
		strncpy (outbuf, "", sizeof(outbuf)); /* Do not return NULL, just an empty buffer */
		return outbuf;
	}

	for (rch = ch->in_room->people; rch && (rch != ch);rch = rch->next_in_room)
		if (is_name (name, rch->name))
			count++;


	snprintf (outbuf, sizeof(outbuf), "%d.%s", count, name);
	return outbuf;
}


void do_for (CHAR_DATA *ch, char *argument)
{
	ROOM_INDEX_DATA *room, *old_room;
	CHAR_DATA *p, *p_next;
	bool fGods = FALSE, fMortals = FALSE, fMobs = FALSE, fEverywhere = FALSE, found = FALSE;
	int i = 0;
	char range[MIL]={'\0'};
	char buf[MSL]={'\0'};

	CheckCH(ch);

	argument = one_argument (argument, range);

	if (IS_NULLSTR(range) || IS_NULLSTR(argument)) /* invalid usage? */
	{
		send_to_char("Syntax: for <admin/players/mobs/npcs/all/everywhere> <action>\n\r", ch);
		return;
	}

	if (!str_prefix("quit", argument))
	{
		send_to_char ("Are you trying to crash the MUD or something?\n\r",ch);
		return;
	}


	if (!str_cmp (range, "all"))
	{
		fMortals = TRUE;
		fGods = TRUE;
	}
	else if (!str_prefix(range, "admin") || !str_prefix(range, "staff"))
		fGods = TRUE;
	else if (!str_prefix(range, "mobs") || !str_prefix(range, "npcs")
			|| !str_prefix(range, "mobiles"))
		fMobs = TRUE;
	else if (!str_prefix(range, "mortals") || !str_prefix(range, "players"))
		fMortals = TRUE;
	else if (!str_cmp(range, "everywhere"))
		fEverywhere = TRUE;
	else
		send_to_char("Syntax: for <admin/players/mobs/npcs/all/everywhere> <action>\n\r", ch); /* show syntax */

	/* do not allow # to make it easier */
	if (fEverywhere && strchr (argument, '#'))
	{
		send_to_char ("Cannot use FOR EVERYWHERE with the # thingie.\n\r",ch);
		return;
	}

	if (strchr (argument, '#')) /* replace # ? */
	{
		for (p = char_list; p ; p = p_next)
		{
			p_next = p->next; /* In case someone DOES try to AT MOBS SLAY # */
			found = FALSE;

			if (!(p->in_room) || room_is_private(p->in_room) || (p == ch))
				continue;

			if (IS_NPC(p) && fMobs)
				found = TRUE;
			else if (!IS_NPC(p) && p->trust >= LEVEL_IMMORTAL && fGods)
				found = TRUE;
			else if (!IS_NPC(p) && p->trust < LEVEL_IMMORTAL && fMortals)
				found = TRUE;

			/* It looks ugly to me.. but it works :) */
			if (found) /* p is 'appropriate' */
			{
				char *pSource = argument; /* head of buffer to be parsed */
				char *pDest = buf; /* parse into this */

				while (*pSource)
				{
					if (*pSource == '#') /* Replace # with name of target */
					{
						const char *namebuf = name_expand (p);

						if (namebuf) /* in case there is no mob name ?? */
							while (*namebuf) /* copy name over */
								*(pDest++) = *(namebuf++);

						pSource++;
					}
					else
						*(pDest++) = *(pSource++);
				} /* while */
				*pDest = '\0'; /* Terminate */

				/* Execute */
				old_room = ch->in_room;
				char_from_room (ch);
				char_to_room (ch,p->in_room);
				interpret (ch, buf);
				char_from_room (ch);
				char_to_room (ch,old_room);

			} /* if found */
		} /* for every char */
	}
	else /* just for every room with the appropriate people in it */
	{
		for (i = 0; i < MAX_KEY_HASH; i++) /* run through all the buckets */
			for (room = room_index_hash[i] ; room ; room = room->next)
			{
				found = FALSE;

				/* Anyone in here at all? */
				if (fEverywhere) /* Everywhere executes always */
					found = TRUE;
				else if (!room->people) /* Skip it if room is empty */
					continue;


				/* Check if there is anyone here of the requried type */
				/* Stop as soon as a match is found or there are no more ppl in room */
				for (p = room->people; p && !found; p = p->next_in_room)
				{

					if (p == ch) /* do not execute on oneself */
						continue;

					if (IS_NPC(p) && fMobs)
						found = TRUE;
					else if (!IS_NPC(p) && (p->trust >= LEVEL_IMMORTAL) && fGods)
						found = TRUE;
					else if (!IS_NPC(p) && (p->trust <= LEVEL_IMMORTAL) && fMortals)
						found = TRUE;
				} /* for everyone inside the room */

				if (found && !room_is_private(room)) /* Any of the required type here AND room not private? */
				{
					/* This may be ineffective. Consider moving character out of old_room
					   once at beginning of command then moving back at the end.
					   This however, is more safe?
					 */

					old_room = ch->in_room;
					char_from_room (ch);
					char_to_room (ch, room);
					interpret (ch, argument);
					char_from_room (ch);
					char_to_room (ch, old_room);
				} /* if found */
			} /* for every room in a bucket */
	} /* if strchr */
} /* do_for */

void get_actors (EVENT_DATA *event)
{
	ACTOR_DATA *pa;
	CHAR_DATA *a;

	for(pa = event->actors; pa; pa = pa->next)
	{
		for(a = char_list; a; a = a->next)
		{
			if(a->pIndexData)
				if(a->pIndexData->vnum == pa->mob)
				{
					char_from_room(a);
					char_to_room(a, get_room_index(event->loc));
					break;
				}
		}
	}
}

void run_script (SCRIPT_DATA *s)
{
	CHAR_DATA *actor;

	for(actor = char_list; actor; actor = actor->next)
	{
		if(IS_NPC(actor) && actor->in_room->vnum == s->event->loc)
		{
			if(actor->pIndexData->vnum == s->actor)
			{
				interpret(actor, s->reaction);
			}
		}

	}
}

EVENT_DATA *get_event_index args( (int vnum) );

void do_run_event (CHAR_DATA *ch, char *argument)
{
	int value = 0;
	EVENT_DATA *event;
	SCRIPT_DATA *script;

	CheckCH(ch);

	if(!is_number(argument))
	{
		send_to_char("Run which event?\n\r", ch);
		return;
	}

	value = atoi(argument);

	if((event = get_event_index(value)) == NULL)
	{
		send_to_char("No such event.\n\r", ch);
		return;
	}

	do_function(ch, &do_goto, (char *)Format("%d", event->loc) );
	do_function(ch, &do_timestop, "" );
	get_actors(event);
	for(script = event->script_list; script != NULL; script = script->next_in_event)
	{
		if(script->first_script)
			break;
	}

	ch->in_room->event = event;
	event->stop = FALSE;
	if(script != NULL)
		run_script(script);
	else
		send_to_char("No scripts in event or first script not set.\n\r", ch);
}

void do_run_script (CHAR_DATA *ch, char *argument)
{
	SCRIPT_DATA *script;

	CheckCH(ch);

	if(!is_number(argument))
	{
		send_to_char("Run which script?\n\r", ch);
		return;
	}

	if((script = get_script_index(atoi(argument))) == NULL)
	{
		send_to_char("No such script.\n\r", ch);
		return;
	}

	run_script(script);
}

void do_flag(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	long *flag, old = 0, bnew = 0, marked = 0, pos;
	char arg1[MIL]={'\0'};
	char arg2[MIL]={'\0'};
	char arg3[MIL]={'\0'};
	char word[MIL]={'\0'};
	char type;
	const struct flag_type *flag_table;

	CheckCH(ch);

	argument = one_argument(argument,arg1);
	argument = one_argument(argument,arg2);
	argument = one_argument(argument,arg3);

	type = argument[0];

	if (type == '=' || type == '-' || type == '+')
		argument = one_argument(argument,word);

	if (IS_NULLSTR(arg1))
	{
		send_to_char("Syntax:\n\r",ch);
		send_to_char("  flag mob  <name> <field> <flags>\n\r",ch);
		send_to_char("  flag char <name> <field> <flags>\n\r",ch);
		send_to_char("  mob  flags: act,aff,off,imm,res,vuln,form,part\n\r",ch);
		send_to_char("  char flags: plr,comm,aff,imm,res,vuln,\n\r",ch);
		send_to_char("  +: add flag, -: remove flag, = set equal to\n\r",ch);
		send_to_char("  otherwise flag toggles the flags listed.\n\r",ch);
		return;
	}

	if (IS_NULLSTR(arg2))
	{
		send_to_char("What do you wish to set flags on?\n\r",ch);
		return;
	}

	if (IS_NULLSTR(arg3))
	{
		send_to_char("You need to specify a flag to set.\n\r",ch);
		return;
	}

	if (IS_NULLSTR(argument))
	{
		send_to_char("Which flags do you wish to change?\n\r",ch);
		return;
	}

	if (!str_prefix(arg1,"mob") || !str_prefix(arg1,"char"))
	{
		victim = get_char_world(ch,arg2);
		if (victim == NULL)
		{
			send_to_char("You can't find them.\n\r",ch);
			return;
		}

		/* select a flag to set */
		if (!str_prefix(arg3,"act"))
		{
			if (!IS_NPC(victim))
			{
				send_to_char("Use plr for PCs.\n\r",ch);
				return;
			}

			flag = &victim->act;
			flag_table = act_flags;
		}

		else if (!str_prefix(arg3,"plr"))
		{
			if (IS_NPC(victim))
			{
				send_to_char("Use act for NPCs.\n\r",ch);
				return;
			}

			flag = &victim->act;
			flag_table = plr_flags;
		}

		else if (!str_prefix(arg3,"aff"))
		{
			flag = &victim->affected_by;
			flag_table = affect_flags;
		}

		else if (!str_prefix(arg3,"aff2"))
		{
			flag = &victim->affected_by2;
			flag_table = affect_flags2;
		}

		else if (!str_prefix(arg3,"immunity"))
		{
			flag = &victim->imm_flags;
			flag_table = imm_flags;
		}

		else if (!str_prefix(arg3,"resist"))
		{
			flag = &victim->res_flags;
			flag_table = imm_flags;
		}

		else if (!str_prefix(arg3,"vuln"))
		{
			flag = &victim->vuln_flags;
			flag_table = imm_flags;
		}

		else if (!str_prefix(arg3,"form"))
		{
			if (!IS_NPC(victim))
			{
				send_to_char("Form can't be set on PCs.\n\r",ch);
				return;
			}

			flag = &victim->form;
			flag_table = form_flags;
		}

		else if (!str_prefix(arg3,"parts"))
		{
			if (!IS_NPC(victim))
			{
				send_to_char("Parts can't be set on PCs.\n\r",ch);
				return;
			}

			flag = &victim->parts;
			flag_table = part_flags;
		}

		else if (!str_prefix(arg3,"comm"))
		{
			if (IS_NPC(victim))
			{
				send_to_char("Comm can't be set on NPCs.\n\r",ch);
				return;
			}

			flag = &victim->comm;
			flag_table = comm_flags;
		}

		else
		{
			send_to_char("That's not an acceptable flag.\n\r",ch);
			return;
		}

		old = *flag;
		victim->zone = NULL;

		if (type != '=')
			bnew = old;

		/* mark the words */
		for (; ;)
		{
			argument = one_argument(argument,word);

			if (IS_NULLSTR(word))
				break;

			pos = flag_lookup(word,flag_table);
			if (pos == 0)
			{
				send_to_char("That flag doesn't exist!\n\r",ch);
				return;
			}
			else
				SET_BIT(marked,pos);
		}

		for (pos = 0; flag_table[pos].name != NULL; pos++)
		{
			if (!flag_table[pos].settable && IS_SET(old,flag_table[pos].bit))
			{
				SET_BIT(bnew,flag_table[pos].bit);
				continue;
			}

			if (IS_SET(marked,flag_table[pos].bit))
			{
				switch(type)
				{
				case '=':
				case '+':
					SET_BIT(bnew,flag_table[pos].bit);
					break;
				case '-':
					REMOVE_BIT(bnew,flag_table[pos].bit);
					break;
				default:
					if (IS_SET(bnew,flag_table[pos].bit))
						REMOVE_BIT(bnew,flag_table[pos].bit);
					else
						SET_BIT(bnew,flag_table[pos].bit);
					break;
				}
			}
		}
		*flag = bnew;
		return;
	}
}

void do_interrupt (CHAR_DATA *ch, char *argument)
{
	EVENT_DATA *e;

	CheckCH(ch);

	if(!str_cmp(argument, "all"))
	{
		if(ch->trust == MAX_LEVEL)
		{
			for(e = event_list; e; e = e->next)
			{
				if(!e->stop)
				{
					e->stop = TRUE;
				}
			}
			send_to_char("You shut off all running events.\n\r", ch);
		}
		else
		{
			do_function(ch, &do_interrupt, "" );
		}
	}
	else
	{
		if(ch->in_room->event != NULL)
		{
			if(is_name(ch->name, ch->in_room->event->author) || ch->trust == MAX_LEVEL)
			{
				ch->in_room->event->stop = TRUE;
				send_to_char("Event stopped.\n\r", ch);
				return;
			}
			else
			{
				send_to_char("You have no authority over this event.\n\r", ch);
				return;
			}
		}
		else
		{
			send_to_char("No event running here.\n\r", ch);
			return;
		}
	}
}

void do_end_event(CHAR_DATA *ch, char *argument)
{
	CheckCH(ch);

	if(ch->in_room->event == NULL)
	{
		send_to_char("No event to finish here.\n\r", ch);
		return;
	}

	if(is_name(ch->name, ch->in_room->event->author) || ch->trust == MAX_LEVEL)
	{
		ch->in_room->event->stop = TRUE;
		ch->in_room->event = NULL;
		do_function(ch, &do_timestop, "" );
		send_to_char("Event stopped and cleared from room.\n\r", ch);
		return;
	}
	else
	{
		send_to_char("You have no authority over this event.\n\r", ch);
		return;
	}
}

void do_clear_timers(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;

	CheckCH(ch);

	log_string( LOG_COMMAND, Format("Timer clear argument is %s.", argument));

	if(IS_NULLSTR(argument))
	{
		send_to_char("Whose timers do you wish to clear?\n\r", ch);
		return;
	}

	if ( ( victim = get_char_world( ch, argument ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	if(IS_NPC(victim))
	{
		send_to_char("Not on NPC's.\n\r", ch);
		return;
	}

	send_to_char("Ok.\n\r", ch);
	act("$n has cleared all of your timers.", ch, NULL, victim, TO_VICT, 1);
	victim->power_timer = 0;
	victim->infl_timer = 0;
}

void do_resetoocxp(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;

	CheckCH(ch);

	if(IS_NULLSTR(argument) || (victim = get_char_room(ch, argument)) == NULL)
	{
		send_to_char("Whose experience timer do you wish to clear?\n\r", ch);
		return;
	}

	if(IS_NPC(victim))
	{
		send_to_char("Not on NPC's.\n\r", ch);
		return;
	}

	send_to_char("Ok.\n\r", ch);
	act("$n clears your ooc xp lock.", ch, NULL, victim, TO_VICT, 1);
	victim->ooc_xp_time = 0;
	victim->ooc_xp_count = 0;
}

void do_storyteller (CHAR_DATA *ch, char *argument)
{
	CheckCH(ch);

	if(IS_SET(ch->comm, COMM_BUILDER))
	{
		send_to_char("You must turn off buildermode first.\n\r", ch);
		return;
	}

	if(IS_SET(ch->act2, ACT2_STORY))
	{
		REMOVE_BIT(ch->act2, ACT2_STORY);
		send_to_char("You step out of storyteller mode.\n\r", ch);
		cleanse_builder_stuff(ch);
	}
	else
	{
		SET_BIT(ch->act2, ACT2_STORY);
		send_to_char("You step into storyteller mode.\n\r", ch);
	}
}

void run_election();

void do_election (CHAR_DATA *ch, char *argument)
{
	CheckCH(ch);
	run_election();
	// announce_election();
	send_to_char("Election complete.\n\r", ch);
}

void do_newspaper (CHAR_DATA *ch, char *argument)
{
	int cmd = 0;
	char arg[MIL]={'\0'};

	CheckCH(ch);

	if(IS_NULLSTR(argument))
	{
		send_to_char("What are you trying to do with the news?\n\r", ch);
		send_to_char("Do you mean to use \t<send href='newspaper commands'>newspaper commands\t</send>?\n\r", ch);
		return;
	}

	argument = one_argument(argument, arg);

	cmd = news_cmd_lookup(arg);

	if(cmd <= -1
			|| ch->trust < news_cmd_table[cmd].level)
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	(*news_cmd_table[cmd].func) ( ch, argument );
	return;
}

void do_articles(CHAR_DATA *ch, char *arg)
{
	CheckCH(ch);
	newspaper_articles(ch, arg);
	return;
}

void do_elder(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	CheckCH(ch);

	if((victim = get_char_world(ch, argument)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	send_to_char("Ok.\n\r", ch);

	if(!IS_SET(ch->plr_flags, PLR_ELDER))
	{
		send_to_char("You are now officially an elder.\n\r", victim);
		SET_BIT(ch->plr_flags, PLR_ELDER);
		return;
	}

	send_to_char("You no longer have the restrictions of being an elder.\n\r", victim);
	REMOVE_BIT(ch->plr_flags, PLR_ELDER);
	return;
}

void do_telepath(CHAR_DATA *ch, char *argument)
{
	CheckCH(ch);

	if(IS_SET(ch->comm, COMM_THINK_ON))
	{
		send_to_char("You will no longer hear the thoughts of others.\n\r", ch);
		REMOVE_BIT(ch->comm, COMM_THINK_ON);
	}
	else
	{
		send_to_char("You will now hear the thoughts of others.\n\r", ch);
		SET_BIT(ch->comm, COMM_THINK_ON);
	}
}

void do_backup(CHAR_DATA *ch, char *argument)
{
	CheckCH(ch);

	if(backup)
	{
		send_to_char("Backup manipulation already in progress.\n\r", ch);
		return;
	}

	int systemRet = system("~/ptmud/scripts/backup &");
	if (systemRet == -1)
	{
		log_string(LOG_BUG, "Error in do_backup.  Unable to run backup command.");
	}
	// system("~/ptmud/scripts/backup &");
	send_to_char("Backup under way.\n\r", ch);
	backup = TRUE;
	wiznet("\tY[WIZNET]\tn $N starts a backup cycle.",	ch,NULL,WIZ_LOAD,WIZ_SECURE,3);
}

void do_unpak(CHAR_DATA *ch, char *argument)
{
	CheckCH(ch);

	if(backup)
	{
		send_to_char("Backup manipulation already in progress.\n\r", ch);
		return;
	}

	int systemRet = system("~/ptmud/scripts/unpak &");
	if (systemRet == -1)
	{
		log_string(LOG_BUG, "Error in do_unpak.  Unable to run unpak command.");
	}
	// system("~/ptmud/scripts/unpak &");
	send_to_char("Unpak under way.\n\r", ch);
	backup = TRUE;
	wiznet("\tY[WIZNET]\tn $N starts an unpak cycle.",	ch,NULL,WIZ_LOAD,WIZ_SECURE,3);
}

void do_stockmarket (CHAR_DATA *ch, char *argument)
{
	int cmd = 0;
	char arg[MIL]={'\0'};

	CheckCH(ch);

	if(IS_NULLSTR(argument))
	{
		send_to_char("What are you trying to do with the market?\n\r", ch);
		return;
	}

	argument = one_argument(argument, arg);

	cmd = smarket_cmd_lookup(arg);

	if(cmd <= -1 || ch->trust < smarket_cmd_table[cmd].level)
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	(*smarket_cmd_table[cmd].func) ( ch, argument );

	return;
}

void do_sayat( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *rch;
	ROOM_INDEX_DATA *location;
	char arg[MIL]={'\0'};

	CheckCH(ch);

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Sayat where?\n\r", ch );
		return;
	}

	argument = one_argument(argument, arg);
	if ( IS_NULLSTR(arg) )
	{
		send_to_char( "Sayat what?\n\r", ch );
		return;
	}

	if((location = find_location(ch, arg)) == NULL)
	{
		send_to_char("No such location.\n\r", ch);
		return;
	}

	for ( rch = location->people; rch; rch = rch->next_in_room )
	{
		if ( rch != ch )
		{
			act(Format("\tC$n says in a disembodied voice, '\tY$t\tC'\tn"), ch, argument, rch, TO_VICT, 1 );
		}
	}

	act( Format("\tCYou say in a disembodied voice '\tY$T\tC'\tn"), ch, NULL, argument, TO_CHAR, 1 );

	return;
}

void do_marry(CHAR_DATA *ch, char *argument)
{
	CheckCH(ch);
	judge_marry(ch, argument);
}

void do_fixchar(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *vch;

	CheckCH(ch);

	if(IS_NULLSTR(argument))
	{
		send_to_char("Whose character are you attempting to reset?\n\r", ch);
		return;
	}

	if((vch = get_char_world(ch, argument)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	if(IS_NPC(vch))
	{
		send_to_char("Not on NPCs.\n\r", ch);
		return;
	}

	vch->pcdata->full_reset = TRUE;
	reset_char(vch);
	vch->pcdata->full_reset = FALSE;
	send_to_char("Ok.\n\r", ch);
}

void do_org(CHAR_DATA *ch, char *argument)
{
	ORG_DATA *org;
	ORGMEM_DATA *mem;
	CHAR_DATA *vch;
	int online = FALSE;
	int in_char_list;
	char arg[MIL]={'\0'};

	CheckCH(ch);

	if(IS_NULLSTR(argument))
	{
		send_to_char( "Syntax: org create <name> <who_name> - create a new organisation\n\r", ch);
		send_to_char("        org list - list all organisations\n\r", ch);
		/*	send_to_char("        org <name> <democracy/dictatorial>\n\r", ch); */
		send_to_char("        org <name> leader <argument>\n\r", ch);
		send_to_char("        org <name> step <vnum>\n\r", ch);
		send_to_char("        org <name> name <argument>\n\r", ch);
		send_to_char("        org <name> whoname <argument>\n\r", ch);
		send_to_char("        org <name> type <argument>\n\r", ch);
		send_to_char("        org <name> startingauth <argument>\n\r", ch);
		send_to_char("        org <name> funds <number>\n\r", ch);
		send_to_char("        org <name> races <race list>\n\r", ch);
		send_to_char("        org <name> clans <clan list>\n\r", ch);
		send_to_char("        org <name> show\n\r", ch);
		return;
	}

	argument = one_argument(argument, arg);

	if(!str_prefix(arg, "create"))
	{
		argument = one_argument(argument, arg);
		if(IS_NULLSTR(arg) || IS_NULLSTR(argument))
		{
			send_to_char("What do you want to name the organisation?\n\r", ch);
			return;
		}

		if(strlen(argument) > 5)
		{
			send_to_char("The who name can only be 5 characters long.\n\r", ch);
			return;
		}

		org = new_org();
		PURGE_DATA(org->name);
		PURGE_DATA(org->who_name);
		PURGE_DATA(org->file_name);
		org->name = str_dup(arg);
		org->who_name = str_dup(argument);
		org->file_name = str_dup(Format("%ld.org", (long)current_time));
		org->step_point = 0;
		org->type = 0;

		LINK_SINGLE(org, next, org_list);

		act("You create the $t organisation.", ch, org->name, NULL, TO_CHAR, 1);
		fwrite_org(org);
		save_org_list();
		return;
	}
	else if(!str_prefix(arg, "list"))
	{
		online = FALSE;
		send_to_char("\tWORG Who | ORG Name\tn\n\r", ch);
		send_to_char("\tW------------------------------------------------------\tn\n\r", ch);
		for(org = org_list; org; org = org->next)
		{
			send_to_char(Format("\tY%5s   | %s\tn\n\r", org->who_name, org->name), ch);
			online = TRUE;
		}
		if(!online) send_to_char("No organisations exist.\n\r", ch);
		return;
	}


	if((org = org_lookup(arg)) == NULL)
	{
		send_to_char("No such organisation.\n\r", ch);
		return;
	}

	argument = one_argument(argument, arg);
	if(!str_prefix(arg, "leader"))
	{
		if(IS_NULLSTR(argument))
		{
			send_to_char(Format("Current Leader: %s\n\r", org->leader?org->leader:""), ch);
			return;
		}

		online = is_online(argument);
		in_char_list = pc_in_char_list(argument);

		if((vch = get_player(argument)) == NULL)
		{
			send_to_char("No such player.\n\r", ch);
			return;
		}

		if((mem = mem_lookup(org, vch->name)) == NULL)
		{
			mem = new_orgmem();
			mem->next = org->members;
			org->members = mem;
			mem->name = str_dup(vch->name);
		}
		mem->status = 5;
		mem->auth_flags = A|B|C|D|E|F|G|H|I|J|K;

		if(is_name(vch->name, org->applicants))
		{
			org->applicants = string_replace( org->applicants, vch->name, "" );
			org->applicants = string_unpad( org->applicants );
		}

		PURGE_DATA(org->leader);
		org->leader = str_dup(vch->name);

		if(!online && !in_char_list)
		{
			free_char(vch);
		}
	}

	else if(!str_prefix(arg, "name"))
	{
		if(IS_NULLSTR(argument))
		{
			send_to_char("You need to supply a new name.\n\r", ch);
			return;
		}

		PURGE_DATA(org->name);
		org->name = str_dup(argument);
	}

	else if(!str_prefix(arg, "whoname"))
	{
		if(IS_NULLSTR(argument))
		{
			send_to_char("You need to supply a new who name.\n\r", ch);
			return;
		}

		PURGE_DATA(org->who_name);
		org->who_name = str_dup(argument);
	}

	else if(!str_prefix(arg, "filename"))
	{
		int length, i;

		if(IS_NULLSTR(argument))
		{
			send_to_char("You need to supply a new file name.\n\r", ch);
			return;
		}

		one_argument(argument, arg);

		/*
		 * Simple Syntax Check.
		 */
		length = strlen( argument );
		if ( length > 15 )
		{
			send_to_char( "No more than eight characters allowed.\n\r", ch );
			return;
		}

		/*
		 * Allow only letters and numbers.
		 */
		for ( i = 0; i < length; i++ )
		{
			if ( !isalnum( arg[i] ) )
			{
				send_to_char( "Only letters and numbers are valid.\n\r", ch );
				return;
			}
		}

		unlink( (char *)Format("%s%s", ORG_DIR, org->file_name));

		strncat(arg, ".org", sizeof(arg) - strlen(arg) - 1 );
		PURGE_DATA(org->file_name);
		org->file_name = str_dup(arg);
		save_org_list();
		send_to_char("Filename set.\n\r", ch);
	}

	else if(!str_prefix(arg, "step"))
	{
		if(IS_NULLSTR(argument) || !is_number(argument))
		{
			send_to_char("You need to supply the vnum of the step point.\n\r", ch);
			return;
		}

		if(!get_room_index(atol(argument)))
		{
			send_to_char("No such room.\n\r", ch);
			return;
		}

		org->step_point = atol(argument);
	}

	else if(!str_prefix(arg, "show"))
	{
		ROOM_INDEX_DATA *rm = get_room_index(org->step_point);
		send_to_char("\tW***** \tCOrganization Info\tW *****\tn\n\r", ch);
		send_to_char(Format("\tW***** [ \tC%s\tn\tW ] *****\tn\n\r", org->name), ch);
		send_to_char(Format("Org Who Name : %5s\n\r", org->who_name), ch);
		send_to_char(Format("Leader       : %s\n\r", org->leader?org->leader:""), ch);
		send_to_char(Format("Step point   : (%ld) %s\n\r", org->step_point?org->step_point:0, rm?rm->name:"None"), ch);
		send_to_char(Format("Current Funds: $%ld\n\r", org->funds), ch);
		send_to_char(Format("Type         : %s\n\r", capitalize(org_type[org->type].name)), ch);
		send_to_char("Acceptible groups:\n\r", ch);
		send_to_char(org->races, ch);
		send_to_char(Format("\n\rDefault Auths: %s\n\r",  flag_string(org_auths, org->default_auths)), ch);
	}

	else if(!str_prefix(arg, "type"))
	{
		if(IS_NULLSTR(argument))
		{
			int i;

			send_to_char("You need to supply an organisational type.\n\r", ch);
			for(i=0; org_type[i].name; i++)
			{
				send_to_char(Format("%15s", org_type[i].name), ch);
			}

			return;
		}

		if((org->type = flag_lookup(argument, org_type)) < 0)
		{
			send_to_char("No such org type.\n\r", ch);
			org->type = 0;
			return;
		}
	}

	else if(!str_prefix(arg, "startingauth"))
	{
		int j;
		if(IS_NULLSTR(argument))
		{
			int i;

			send_to_char("You need to supply a flag.\n\r", ch);
			for(i=0; org_auths[i].name; i++)
			{
				send_to_char(Format("%9s", org_auths[i].name), ch);
				if((i+1)%6 == 0) send_to_char("\n\r", ch);
			}

			return;
		}

		if((j = flag_lookup(argument, org_auths)) < 0)
		{
			send_to_char("No such org type.\n\r", ch);
			return;
		}

		org->default_auths ^= j;
	}

	else if(!str_prefix(arg, "funds"))
	{
		if(IS_NULLSTR(argument) || !is_number(argument))
		{
			send_to_char("You need to supply a number for the funds.\n\r", ch);
			return;
		}

		org->funds += atol(argument);
	}

	else if(!str_prefix(arg, "races"))
	{
		char name[MSL]={'\0'};
		char buf[MSL]={'\0'};
		int race;

		if(IS_NULLSTR(argument))
		{
			send_to_char( "Syntax:  org races [$race] -toggles race\n\r", ch );
			send_to_char( "Syntax:  org races All     -allows everyone\n\r", ch );
			send_to_char( "Currently acceptible races and clans:\n\r", ch);
			send_to_char(org->races, ch);
			return;
		}

		one_argument( argument, name );

		if((race = race_lookup(argument)) == 0 && str_cmp(argument, "All"))
		{
			send_to_char("That is not a valid race.\n\r", ch);
			return;
		}

		if(race > 0)
		{
			send_to_char(Format("%s", capitalize(race_table[race].name)), ch);
		}
		else
		{
			send_to_char("All", ch);
		}

		if (!str_cmp(name, "All"))
		{
			PURGE_DATA(org->races);
			org->races = str_dup( "All" );
			send_to_char("All races and clans are now acceptible.\n\r", ch);
		}
		else if ( strstr( org->races, name ) != NULL )
		{
			org->races = string_replace( org->races, name, "\0" );
			org->races = string_unpad( org->races );

			if ( org->races[0] == '\0' )
			{
				PURGE_DATA( org->races );
				org->races = str_dup( "None" );
			}
			send_to_char( "Race removed.\n\r", ch );
		}
		else
		{
			buf[0] = '\0';
			if ( strstr( org->races, "None" ) != NULL )
			{
				org->races = string_replace( org->races, "None", "\0" );
				org->races = string_unpad( org->races );
			}

			if ( strstr( org->races, "All" ) != NULL )
			{
				org->races = string_replace( org->races, "All", "\0" );
				org->races = string_unpad( org->races );
			}

			if (!IS_NULLSTR(org->races) )
			{
				strncat( buf, org->races, sizeof(buf) - strlen(buf) - 1 );
				strncat( buf, " ", sizeof(buf) - strlen(buf) - 1 );
			}
			strncat( buf, name, sizeof(buf) - strlen(buf) - 1 );
			PURGE_DATA( org->races );
			org->races = string_proper( str_dup( buf ) );

			send_to_char( "Race added.\n\r", ch );
			send_to_char( org->races,ch);
			send_to_char( "\n\r",ch);
		}

	}

	else if(!str_prefix(arg, "clans"))
	{
		char name[MSL]={'\0'};
		char buf[MSL]={'\0'};
		int race;

		if(IS_NULLSTR(argument))
		{
			send_to_char( "Syntax:  org clans [$clan] -toggles clan\n\r", ch );
			send_to_char( "Syntax:  org races All     -allows everyone\n\r", ch );
			send_to_char( "Currently acceptible races and clans:\n\r", ch);
			send_to_char(org->races, ch);
			return;
		}

		one_argument( argument, name );

		if((race = clan_lookup(argument)) == -1 && str_cmp(argument, "All"))
		{
			send_to_char("That is not a valid clan.\n\r", ch);
			return;
		}

		if(race > -1)
		{
			send_to_char(Format("%s", capitalize(clan_table[race].name)), ch);
		}
		else
		{
			send_to_char("All", ch);
		}

		if (!str_cmp(name, "All"))
		{
			PURGE_DATA(org->races);
			org->races = str_dup( "All" );
			send_to_char("All races and clans are now acceptible.\n\r", ch);
		}
		else if ( strstr( org->races, name ) != NULL )
		{
			org->races = string_replace( org->races, name, "\0" );
			org->races = string_unpad( org->races );

			if ( org->races[0] == '\0' )
			{
				PURGE_DATA( org->races );
				org->races = str_dup( "None" );
			}
			send_to_char( "Clan removed.\n\r", ch );
		}
		else
		{
			buf[0] = '\0';
			if ( strstr( org->races, "None" ) != NULL )
			{
				org->races = string_replace( org->races, "None", "\0" );
				org->races = string_unpad( org->races );
			}

			if ( strstr( org->races, "All" ) != NULL )
			{
				org->races = string_replace( org->races, "All", "\0" );
				org->races = string_unpad( org->races );
			}

			if (!IS_NULLSTR(org->races) )
			{
				strncat( buf, org->races, sizeof(buf) - strlen(buf) - 1 );
				strncat( buf, " ", sizeof(buf) - strlen(buf) - 1 );
			}
			strncat( buf, name, sizeof(buf) - strlen(buf) - 1 );
			PURGE_DATA( org->races );
			org->races = string_proper( str_dup( buf ) );

			send_to_char( "Clan added.\n\r", ch );
			send_to_char( org->races,ch);
			send_to_char( "\n\r",ch);
		}

	}

	fwrite_org(org);
	send_to_char("Ok.\n\r", ch);
}

void do_bonus(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *vch;
	int bonus = 0;
	char arg[MIL]={'\0'};

	CheckCH(ch);

	argument = one_argument(argument, arg);

	if((vch = get_char_world(ch, arg)) == NULL)
	{
		send_to_char("They're not here.\n\r", ch);
		return;
	}

	if(!is_number(argument))
	{
		send_to_char("You have to give some number of bonus gift points.\n\r", ch);
		return;
	}

	if((bonus = atoi(argument)) <= 0)
	{
		send_to_char("You have to give a positive amount.\n\r", ch);
		return;
	}

	vch->xpgift += bonus;
	act("You give $N $t bonus gift experience.", ch, Format("%d", bonus), vch, TO_CHAR, 1);
	act("$n gives you $t bonus gift experience.", ch, Format("%d", bonus), vch, TO_VICT, 1);
}

void do_pardon(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *vch;

	CheckCH(ch);

	if((vch = get_char_world(ch, argument)) == NULL)
	{
		send_to_char("They're not here.\n\r", ch);
		return;
	}

	vch->warrants = 0;
	if(vch->in_room->vnum == ROOM_VNUM_JAIL)
	{
		char_from_room(vch);
		char_to_room(vch, get_room_index(ROOM_VNUM_START));
		do_look(vch, "");
	}

	act("You release $N from prison.", ch, NULL, vch, TO_CHAR, 1);
	act("$n releases you from prison.", ch, NULL, vch, TO_VICT, 1);
}

void do_goquest(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *vch;

	CheckCH(ch);

	if(IS_NULLSTR(argument))
	{
		send_to_char("Syntax: offerquest <person>\n\r", ch);
		return;
	}

	if((vch = get_char_world(ch, argument)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	gen_quest(TRUE, vch);
	send_to_char("You arrange for a 'quest' offer.\n\r", ch);
}

void do_ritemoves(CHAR_DATA *ch, char *argument)
{
	int i = 0, j = 0;
	int race = 0;
	bool found = FALSE;

	CheckCH(ch);

	if(IS_NULLSTR(argument))
	{
		race = ch->race;
	}
	else if((race = race_lookup(argument)) < 0)
	{
		send_to_char("That's not a valid race.\n\r", ch);
		return;
	}

	if(race >= PC_RACE)
	{
		send_to_char("Only PC races have the ability to have rituals.\n\r", ch);
		return;
	}

	for(i = 0; ritual_table[i].name != NULL; i++)
	{
		if( !str_cmp(ritual_table[i].races, "all")
				|| race == race_lookup(ritual_table[i].races))
		{
			if(!found)
			{
				send_to_char("Ritual move cheat sheet\n\r", ch);
			}
			found = TRUE;

			send_to_char(Format("%s: ", ritual_table[i].name), ch);
			for(j = 0; j < MAX_RITE_STEPS; j++)
			{
				if(ritual_table[i].actions[j] > -1)
				{
					send_to_char(Format("%s ", rite_actions[ritual_table[i].actions[j]].name), ch);
				}
			}
			send_to_char("\n\r", ch);
		}
	}

	if(!found)
	{
		send_to_char("There are no rituals available to that race.\n\r", ch);
	}
}

void do_timeset(CHAR_DATA *ch, char *argument)
{
	char arg[MIL]={'\0'};
	int i = 0;

	CheckCH(ch);

	if(IS_NULLSTR(argument))
	{
		send_to_char("Syntax: timeset date D/M/YYYY\n\r", ch);
		send_to_char("        timeset time [0-23]\n\r", ch);
		return;
	}

	argument = one_argument(arg, argument);

	if(IS_NULLSTR(argument))
	{
		do_function(ch, &do_timeset, "" );
		return;
	}

	if(!str_prefix(arg, "date"))
	{
		/* Set the day. */
		argument = one_chunk(arg, argument, '/');
		if(!is_number(arg))
		{
			do_function(ch, &do_timeset, "" );
			return;
		}
		i = atoi(arg);
		if(1 > i || i > 31)
		{
			send_to_char("The day must be between 1 and 31.\n\r", ch);
			return;
		}
		time_info.day = i;

		/* Set the month. */
		argument = one_chunk(arg, argument, '/');
		if(!is_number(arg))
		{
			do_function(ch, &do_timeset, "" );
			return;
		}
		i = atoi(arg);
		if(1 > i || i > 12)
		{
			send_to_char("The month must be between 1 and 12.\n\r", ch);
			return;
		}
		time_info.month = i;

		/* Set the year. */
		if(!is_number(argument))
		{
			do_function(ch, &do_timeset, "" );
			return;
		}
		i = atoi(argument);
		time_info.year = i;
	}
	else if(!str_prefix(arg, "time"))
	{
		if(!is_number(argument))
		{
			do_function(ch, &do_timeset, "" );
			return;
		}

		i = atoi(argument);
		if(i < 0 || i > 23)
		{
			send_to_char( "There's 24 hours in a day, which do you want? (0-23)\n\r", ch);
			return;
		}

		time_info.hour = i;
	}
	else
	{
		do_function(ch, &do_timeset, "" );
		return;
	}
}

MUDCMD(do_trackbuffer)
{
	BUFFER *output = new_buf();
	BUFFER *count, *count_next;
	int counter =0;

	for(count = buffer_list; count; count = count_next)
	{
		count_next = count->next;
		if(count != output)
			BufPrintf(output,"Buffer Found:File: %s, Function: %s, Line: %d\n\r",count->file, count->function, count->line);
		else
			BufPrintf(output,"Buffer Found:File: Called by this function! (Ignore!)\n\r");
		counter++;
	}

	BufPrintf(output,"%d buffers were found open.\n\r",counter);
	page_to_char(buf_string(output), ch);
	free_buf(output);
	return;
}

void do_arealinks(CHAR_DATA *ch, char *argument)
{
    BUFFER *buffer;
    AREA_DATA *parea;
    EXIT_DATA *pexit;
    ROOM_INDEX_DATA *to_room;
    ROOM_INDEX_DATA *from_room;
    char buf[MAX_STRING_LENGTH] = {'\0'};
    char arg1[MAX_INPUT_LENGTH] = {'\0'};
    int vnum = 0;
    int iHash, door;
    bool found = FALSE;

    static char * const dir_name[] = {"north","east","south","west","up","down"};

    argument = one_argument(argument, arg1);

    /* First, the 'all' option */
    if (!str_cmp(arg1,"all"))
    {
        buffer = new_buf();

        for (parea = area_first; parea != NULL; parea = parea->next)
        {
            /* First things, add area name  and vnums to the buffer */
            sprintf(buf, "*** %s (%d to %d) ***\n\r",
                    parea->name, parea->min_vnum, parea->max_vnum);

            found = FALSE;
            for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
            {
                for ( from_room = room_index_hash[iHash];
                        from_room != NULL;
                        from_room = from_room->next )
                {
                    if ( from_room->vnum < parea->min_vnum
                            ||   from_room->vnum > parea->max_vnum )
                        continue;

                    for (door = 0; door < 6; door++)
                    {
                        /* Does an exit exist in this direction? */
                        if ( (pexit = from_room->exit[door]) != NULL )
                        {
                            to_room = pexit->u1.to_room;

                            if ( to_room != NULL
                                    &&  (to_room->vnum < parea->min_vnum
                                         ||   to_room->vnum > parea->max_vnum) )
                            {
                                found = TRUE;
                                sprintf(buf, "%s (%d) links %s to %s (%d)\n\r",
                                        parea->name, from_room->vnum, dir_name[door],
                                        to_room->area->name, to_room->vnum);

                                add_buf(buffer, buf);
                            }
                        }
                    }
                }
            }

            if (!found)
                sprintf(buf, "%s No links to other areas found.\n\r", parea->name);

            add_buf(buffer, buf);
        }

        page_to_char(buffer->string, ch);
        free_buf(buffer);

        return;
    }

    if (arg1[0] == '\0')
    {
        parea = ch->in_room ? ch->in_room->area : NULL;

        if (parea == NULL)
        {
            send_to_char("You aren't in an area right now, funky.\n\r",ch);
            return;
        }
    }

    else if (is_number(arg1))
    {
        vnum = atoi(arg1);

        if (vnum <= 0)
        {
            send_to_char("The vnum must be greater than 1.\n\r",ch);
            return;
        }

        for (parea = area_first; parea != NULL; parea = parea->next)
        {
            if (vnum >= parea->min_vnum && vnum <= parea->max_vnum)
                break;
        }

        if (parea == NULL)
        {
            send_to_char("There is no area containing that vnum.\n\r",ch);
            return;
        }
    }

    else
    {
        for (parea = area_first; parea != NULL; parea = parea->next)
        {
            if (!str_prefix(arg1, parea->name))
                break;
        }

        if (parea == NULL)
        {
            send_to_char("There is no such area.\n\r",ch);
            return;
        }
    }

    for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
    {
        for ( from_room = room_index_hash[iHash];
                from_room != NULL;
                from_room = from_room->next )
        {
            if ( from_room->vnum < parea->min_vnum
                    ||   from_room->vnum > parea->max_vnum )
                continue;

            for (door = 0; door < 6; door++)
            {
                if ( (pexit = from_room->exit[door]) != NULL )
                {
                    to_room = pexit->u1.to_room;

                    if ( to_room != NULL
                            &&  (to_room->vnum < parea->min_vnum
                                 ||   to_room->vnum > parea->max_vnum) )
                    {
                        found = TRUE;
                        sprintf(buf, "%s (%d) links %s to %s (%d)\n\r",
                                parea->name, from_room->vnum, dir_name[door],
                                to_room->area->name, to_room->vnum);
                        send_to_char(buf, ch);
                    }
                }
            }
        }
    }

    if (!found)
    {
        send_to_char("No links to other areas found.\n\r",ch);
        return;
    }

}

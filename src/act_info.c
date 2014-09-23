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

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "twilight.h"
#include "magic.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
#include "interp.h"
#include "version.h"
#include "grid.h"

char *flag_string       args ( (const struct flag_type *flag_table,int bits) );

void fwrite_votes();
int find_door( CHAR_DATA *ch, char *arg );
char *power_string(const struct gift_type *power_table, int bits, int level);
void odd_parts_to_char(CHAR_DATA *ch, CHAR_DATA *vch);

/* Note lookup functions for bg checks */
NOTE_DATA * find_bg_keyword args( (char * arg) );
NOTE_DATA * find_knowledge_keyword args( (char * arg) );

/* for keeping track of the player count */
int max_on = 0;


/*
 * Local functions.
 */
char *  format_obj_to_char  args( ( OBJ_DATA *obj, CHAR_DATA *ch, bool fShort ) );
void    show_list_to_char   args( ( OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing ) );
void    show_char_to_char_0 args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void    show_char_to_char_1 args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void    show_char_to_char   args( ( CHAR_DATA *list, CHAR_DATA *ch ) );
bool    check_blind         args( ( CHAR_DATA *ch ) );



char *format_obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch, bool fShort )
{
	static char buf[MSL]={'\0'};

	buf[0] = '\0';

	if IS_NULLSTR(obj->short_descr)
		{
			obj->short_descr = str_dup("This object needs a short description.");
			log_string(LOG_BUG, Format("Object VNUM %d has no short description.", obj->pIndexData->vnum));
		}

	if IS_NULLSTR(obj->description)
	{
		obj->description = str_dup("This object needs a description.");
		log_string(LOG_BUG, Format("Object VNUM %d has no description.", obj->pIndexData->vnum));
	}

	if ( !IS_SET(obj->extra2, OBJ_PACKAGED) &&
			((fShort && (obj->short_descr == NULL || obj->short_descr[0] == '\0'))
					||  (obj->description == NULL || obj->description[0] == '\0')))
		return buf;

	if (fShort && obj->in_obj != NULL && obj->in_obj->item_type == ITEM_FURNITURE)
		strncat( buf, "    " , sizeof(buf));

	if ( IS_OBJ_STAT(obj, ITEM_INVIS)       )   strncat( buf, "(Invis) ", sizeof(buf)     );
	if ( IS_OBJ_STAT(obj, ITEM_GLOW)        )   strncat( buf, "(Glowing) ", sizeof(buf)   );
	if ( IS_OBJ_STAT(obj, ITEM_DREAM)       )   strncat( buf, "(Dream) ", sizeof(buf)     );
	if ( IS_OBJ_STAT(obj, ITEM_UMBRAL)      )   strncat( buf, "(Umbra) ", sizeof(buf)     );
	if ( IS_OBJ_STAT(obj, ITEM_HIDDEN)      )   strncat( buf, "(Hidden) ", sizeof(buf)    );

	if ( fShort )
	{
		if ( obj->short_descr != NULL ) {
			if(IS_SET(obj->extra2, OBJ_PACKAGED))
				strncat( buf, "a package", sizeof(buf) );
			else
				strncat( buf, obj->short_descr, sizeof(buf) );
		}
		if (obj->in_obj != NULL && obj->in_obj->item_type == ITEM_FURNITURE) {
			strncat( buf, " on " , sizeof(buf));
			if(IS_SET(obj->in_obj->extra2, OBJ_PACKAGED))
				strncat(buf, "a package", sizeof(buf));
			else
				strncat( buf, obj->in_obj->short_descr, sizeof(buf) );
		}
	}
	else
	{
		if ( obj->description != NULL) {
			if(IS_SET(obj->extra2, OBJ_PACKAGED))
				strncat( buf, "a package sits here.", sizeof(buf) );
			else
				strncat( buf, obj->description, sizeof(buf) );
		}

		if ( obj->item_type == ITEM_FURNITURE && obj->contains != NULL
				&& !IS_SET(obj->extra2, OBJ_PACKAGED) )
			show_list_to_char(obj->contains, ch, TRUE, FALSE);
	}

	if(strlen(buf) <= 0)
		strncat(buf, "This object has no description.\n\r", sizeof(buf));
		log_string(LOG_BUG, Format("Object VNUM %d has no description.", obj->pIndexData->vnum));

	return buf;
}



/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char( OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing )
{
	BUFFER *output;
	OBJ_DATA *obj;
	char **prgpstrShow;
	char *pstrShow;
	int *prgnShow = 0;
	int nShow = 0;
	int iShow = 0;
	int count = 0;
	bool fCombine;

	if ( ch->desc == NULL )
		return;

	if (!list)
		return;

	/*
	 * Alloc space for output lines.
	 */
	output = new_buf();

	count = 0;
	for ( obj = list; obj != NULL; obj = obj->next_content )
		count++;

	ALLOC_DATA(prgpstrShow, char*, (count *sizeof(char*)));
	ALLOC_DATA(prgnShow, int, count);

	nShow   = 0;

	/*
	 * Format the list of objects.
	 */
	for ( obj = list; obj != NULL; obj = obj->next_content )
	{
		if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ))
		{
			pstrShow = format_obj_to_char( obj, ch, fShort );

			fCombine = FALSE;

			if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
			{
				/*
				 * Look for duplicates, case sensitive.
				 * Matches tend to be near end so run loop backwords.
				 */
				for ( iShow = nShow - 1; iShow >= 0; iShow-- )
				{
					if ( !str_cmp( prgpstrShow[iShow], pstrShow ) )
					{
						prgnShow[iShow]++;
						fCombine = TRUE;
						break;
					}
				}
			}

			/*
			 * Couldn't combine, or didn't want to.
			 */
			if ( !fCombine )
			{
				prgpstrShow [nShow] = str_dup( pstrShow );
				prgnShow    [nShow] = 1;
				nShow++;
			}
		}
	}

	/*
	 * Output the formatted list.
	 */
	for ( iShow = 0; iShow < nShow; iShow++ )
	{
		if (prgpstrShow[iShow][0] == '\0')
		{
			PURGE_DATA(prgpstrShow[iShow]);
			continue;
		}

		if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
		{
			if ( prgnShow[iShow] != 1 )
			{
				add_buf(output, (char *)Format("(%2d) ", prgnShow[iShow]));
			}
			else
			{
				add_buf(output,"     ");
			}
		}
		add_buf(output,prgpstrShow[iShow]);
		add_buf(output,"\n\r");
		PURGE_DATA( prgpstrShow[iShow] );
	}

	if ( fShowNothing && nShow == 0 )
	{
		if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
			send_to_char( "     ", ch );
		send_to_char( "Nothing.\n\r", ch );
	}
	page_to_char(buf_string(output),ch);

	/*
	 * Clean up.
	 */
	free_buf(output);
	PURGE_DATA( prgpstrShow);
	PURGE_DATA( prgnShow);

	return;
}



void show_char_to_char_0( CHAR_DATA *victim, CHAR_DATA *ch )
{
	char buf[MSL]={'\0'};
	char buf2[MSL]={'\0'};

	buf[0] = '\0';

	if ( IS_ADMIN(ch) )
	{
		if(IS_NPC(victim))
		{
			strncat( buf, Format("[%s] ", victim->short_descr), sizeof(buf) );
		}
		else
		{
			strncat( buf, Format("[%s] ", victim->name), sizeof(buf) );
		}


		if(IS_SET(victim->act2, ACT2_STORY))
		{
			strncat(buf, Format("\tR[\tYST\tR]\tn "), sizeof(buf2));
		}

		if(IS_SET(victim->comm, COMM_AFK))
		{
			strncat(buf, Format("[\tYAFK\tn] "), sizeof(buf));
		}

		if ( !IS_SET(ch->act, ACT_DREAMING) && IS_SET(victim->act, ACT_DREAMING) )
			strncat( buf, "(Dreaming) ", sizeof(buf) );
		if ( !IS_SET(ch->act, ACT_UMBRA) && IS_SET(victim->act, ACT_UMBRA) )
			strncat( buf, "(Umbral) ", sizeof(buf) );
		if ( (IS_SET(ch->act, ACT_UMBRA) || IS_SET(ch->act, ACT_DREAMING))
				&& (!IS_SET(victim->act, ACT_UMBRA)
						&& !IS_SET(victim->act, ACT_DREAMING)) )
			strncat( buf, "(Real World) ", sizeof(buf));

		if ( victim->invis_level >= LEVEL_IMMORTAL) strncat( buf, "(Wiz) ", MSL      );
	}

	if(!SAME_PLANE(victim, ch) && !IS_ADMIN(ch))
		return;

	if ( victim->position == victim->start_pos && !IS_NULLSTR(victim->long_descr)
					&& IS_NPC(victim)
					&& (victim->shape == SHAPE_HUMAN || victim->shape == SHAPE_NONE) )
	{
		strncat( buf, victim->long_descr, sizeof(buf) );
		send_to_char( buf, ch );
		return;
	}

	if(ch->form == FORM_BLOOD)
	{
		if(can_see(ch, victim))
			strncat( buf, "a pool of blood", sizeof(buf) );
		else
			strncat( buf, "someone", sizeof(buf) );
	}
	else if(ch->form == FORM_SHADOW)
	{
		if(can_see(ch, victim))
			strncat( buf, "a dark shadow", sizeof(buf) );
		else
			strncat( buf, "someone", sizeof(buf) );
	}
	else if(ch->form == FORM_HORRID)
	{
		if(can_see(ch, victim))
			strncat( buf, "a horrid beast", sizeof(buf) );
		else
			strncat( buf, "someone", sizeof(buf) );
	}
	else if(ch->form == FORM_ASH)
	{
		if(can_see(ch, victim))
			strncat( buf, "a pile of ashes", sizeof(buf) );
		else
			strncat( buf, "someone", sizeof(buf) );
	}
	else if(victim->shape == SHAPE_HUMAN || victim->shape == SHAPE_NONE)
	{
		if(LOOKS_DIFFERENT(victim))
		{
			if(can_see(ch, victim))
			{
				strncat( buf, Format("%s", victim->alt_name), sizeof(buf) );
			}
			else
			{
				strncat( buf, Format("someone"), sizeof(buf) );
			}
		}
		else
			strncat( buf, PERS( victim, ch ), sizeof(buf) );
	}
	else if(victim->shape == SHAPE_WOLF)
	{
		if(can_see(ch, victim)) {
			if(victim->race == race_lookup("werewolf")) {
				strncat( buf, "a large ", sizeof(buf) );
				strncat( buf, clan_table[victim->clan].alt_race, sizeof(buf) );
			} else strncat( buf, "a large wolf", sizeof(buf) );
		} else strncat( buf, "someone", sizeof(buf) );
	}
	else if(victim->shape == SHAPE_CRINOS)
	{
		if(can_see(ch, victim)) {
			strncat( buf, "a snarling ", sizeof(buf) );
			strncat( buf, clan_table[victim->clan].crinos_desc, sizeof(buf) );
		} else
			strncat( buf, "someone", sizeof(buf) );
	}
	else if(victim->shape == SHAPE_BAT)
	{
		if(can_see(ch, victim))
			strncat( buf, "a bat", sizeof(buf) );
		else
			strncat( buf, "someone", sizeof(buf) );
	}

	if(IS_SET(victim->form, FORM_MIST))
	{
		strncat( buf, " drifts a little in the breeze.", sizeof(buf) );
	}
	else if(IS_SET(victim->form, FORM_BLOOD))
	{
		strncat( buf, " ripples a little.", sizeof(buf) );
	}
	else if(IS_SET(victim->form, FORM_SHADOW))
	{
		strncat( buf, " dims the light.", sizeof(buf) );
	}
	else if(IS_SET(victim->form, FORM_ASH))
	{
		strncat( buf, " rests peacefully, stirring slightly in the breeze.", sizeof(buf) );
	}
	else
	{
		switch ( victim->position )
		{
		case P_DEAD:  strncat( buf, " is DEAD!!", sizeof(buf) );              break;
		case P_MORT:  strncat( buf, " is mortally wounded.", sizeof(buf) );   break;
		case P_INCAP: strncat( buf, " is incapacitated.", sizeof(buf) );      break;
		case P_STUN:  strncat( buf, " is lying here stunned.", sizeof(buf) ); break;
		case P_SLEEP:
			if (victim->on != NULL)
			{
				if (IS_SET(victim->on->value[2],SLEEP_AT))
				{
					strncat(buf,Format(" is sleeping at %s.", victim->on->short_descr), sizeof(buf));
				}
				else if (IS_SET(victim->on->value[2],SLEEP_ON))
				{
					strncat(buf,Format(" is sleeping on %s.", victim->on->short_descr), sizeof(buf));
				}
				else
				{
					strncat(buf,Format(" is sleeping in %s.", victim->on->short_descr), sizeof(buf));
				}
			}
			else
				strncat(buf," is sleeping here.", sizeof(buf));
			break;
		case P_TORPOR:
			strncat(buf, " lies here in torpor.", sizeof(buf));
			break;
		case P_REST:
			if (victim->on != NULL)
			{
				if (IS_SET(victim->on->value[2],REST_AT))
				{
					strncat(buf, Format(" is resting at %s.", victim->on->short_descr), sizeof(buf));
				}
				else if (IS_SET(victim->on->value[2],REST_ON))
				{
					strncat(buf,Format(" is resting on %s.", victim->on->short_descr), sizeof(buf));
				}
				else
				{
					strncat(buf, Format(" is resting in %s.", victim->on->short_descr), sizeof(buf));
				}
			}
			else
				strncat( buf, " is resting here.", sizeof(buf) );
			break;
		case P_SIT:
			if (victim->on != NULL)
			{
				if (IS_SET(victim->on->value[2],SIT_AT))
				{
					strncat(buf, Format(" is sitting at %s.", victim->on->short_descr), sizeof(buf));
				}
				else if (IS_SET(victim->on->value[2],SIT_ON))
				{
					strncat(buf, Format(" is sitting on %s.", victim->on->short_descr), sizeof(buf));
				}
				else
				{
					strncat(buf, Format(" is sitting in %s.", victim->on->short_descr), sizeof(buf));
				}
			}
			else
				strncat(buf, " is sitting here.", sizeof(buf));
			break;
		case P_STAND:
			if (victim->on != NULL)
			{
				if (IS_SET(victim->on->value[2],STAND_AT))
				{
					strncat(buf, Format(" is standing at %s", victim->on->short_descr), sizeof(buf));
				}
				else if (IS_SET(victim->on->value[2],STAND_ON))
				{
					strncat(buf, Format(" is standing on %s", victim->on->short_descr), sizeof(buf));
				}
				else
				{
					strncat(buf, Format(" is standing in %s", victim->on->short_descr), sizeof(buf));
				}
			}
			else
				strncat( buf, " is here", sizeof(buf) );
			if ( IS_AFFECTED(victim, AFF_CHARM)       )
				strncat( buf, " with glassy eyes.", sizeof(buf)    );
			else
				strncat( buf, ".", sizeof(buf) );
			break;
		case P_FIGHT:
			strncat( buf, " is here, fighting ", sizeof(buf) );
			if ( victim->fighting == NULL )
				strncat( buf, "thin air??", sizeof(buf) );
			else if ( victim->fighting == ch )
				strncat( buf, "YOU!", sizeof(buf) );
			else if ( victim->in_room == victim->fighting->in_room )
			{
				strncat( buf, PERS( victim->fighting, ch ), sizeof(buf) );
				strncat( buf, ".", sizeof(buf) );
			}
			else
				strncat( buf, "someone who left??", sizeof(buf) );
			break;
		}
	}

	strncat( buf, "\n\r", sizeof(buf) );
	buf[0] = UPPER(buf[0]);
	send_to_char( buf, ch );
	return;
}



void show_char_to_char_1( CHAR_DATA *victim, CHAR_DATA *ch )
{
	OBJ_DATA *obj;
	char buf[MSL]={'\0'};
	char *msg;
	int iWear = 0;
	int diff = 0;
	bool found = FALSE;

	if ( victim->description[0] == '\0' )
	{
		act( "You see nothing special about $M.", ch, NULL, victim, TO_CHAR,1);
	}
	else if (LOOKS_DIFFERENT(victim))
	{
		send_to_char( victim->alt_description, ch );
	}
	else
	{
		send_to_char( victim->description, ch );
	}

	if(IS_SET(victim->form, FORM_MIST))
	{
		act("$N's form is a ghostly mist.", ch, NULL, victim, TO_CHAR, 1);
	}

	if(IS_SET(victim->form, FORM_BLOOD))
	{
		act("$N's form is a pool of blood.", ch, NULL, victim, TO_CHAR, 1);
	}

	if(IS_SET(victim->form, FORM_SHADOW))
	{
		act("$N's form is a dark shadow.", ch, NULL, victim, TO_CHAR, 1);
	}

	if(IS_SET(victim->form, FORM_HORRID))
	{
		act("$N's form is a horrid beast.", ch, NULL, victim, TO_CHAR, 1);
	}

	if(IS_SET(victim->form, FORM_ASH))
	{
		act("$N's form is a pile of ashes.", ch, NULL, victim, TO_CHAR, 1);
	}

	diff = victim->health + victim->agghealth - 7;

	if ( diff  <=   0 ) msg = "$N is incapacitated.";
	else if ( diff  ==   1 ) msg = "$N is having trouble moving.";
	else if ( diff  ==   2 ) msg = "$N is hurt badly.";
	else if ( diff  ==   3 ) msg = "$N is hurt.";
	else if ( diff  ==   4 ) msg = "$N is hurt.";
	else if ( diff  ==   5 ) msg = "$N has been weakened a little.";
	else if ( diff  ==   6 ) msg = "$N has been weakened a little.";
	else                    msg = "$N is at full health.";

	act( msg, ch, NULL, victim, TO_CHAR, 1 );
	odd_parts_to_char(ch, victim);

	for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
	{
		if ( ( obj = get_eq_char( victim, iWear ) ) != NULL
				&&   can_see_obj( ch, obj ) )
		{
			if ( !found )
			{
				send_to_char( "\n\r", ch );
				act( "$N is using:", ch, NULL, victim, TO_CHAR, 1 );
				found = TRUE;
			}
			snprintf(buf, sizeof(buf), "< %-19s > %s\n\r", wear_loc_strings[iWear].name, format_obj_to_char( obj, ch, TRUE ));
			if((obj != get_eq_char( victim, WEAR_UNDERPANTS ) && obj != get_eq_char( victim, WEAR_UNDERTOP ))
					|| (obj == get_eq_char( victim, WEAR_UNDERPANTS ) && get_eq_char(victim, WEAR_LEGS) == NULL)
							|| (obj == get_eq_char( victim, WEAR_UNDERTOP ) && get_eq_char(victim, WEAR_BODY) == NULL)
			)
				send_to_char( buf, ch );
		}
	}

	return;
}


void show_char_to_char_2( CHAR_DATA *victim, CHAR_DATA *ch )
{
	int diff = 0;
	char *msg;

	if ( victim->description[0] == '\0' )
	{
		act( "You see nothing special about $M.", ch, NULL, victim, TO_CHAR,1);
	}
	else if (LOOKS_DIFFERENT(victim))
	{
		send_to_char( victim->alt_description, ch );
	}
	else
	{
		send_to_char( victim->description, ch );
	}

	diff = victim->health + victim->agghealth - 7;

	if ( diff  <=   0 ) msg = "$N is incapacitated.";
	else if ( diff  ==   1 ) msg = "$N is having trouble moving.";
	else if ( diff  ==   2 ) msg = "$N is hurt badly.";
	else if ( diff  ==   3 ) msg = "$N is hurt.";
	else if ( diff  ==   4 ) msg = "$N is hurt.";
	else if ( diff  ==   5 ) msg = "$N has been weakened a little.";
	else if ( diff  ==   6 ) msg = "$N has been weakened a little.";
	else                    msg = "$N is at full health.";

	act( msg, ch, NULL, victim, TO_CHAR, 1 );

	return;
}


void do_glance( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *vch;

	CheckCH(ch);

	if((vch = get_char_room(ch, argument)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	show_char_to_char_2(vch, ch);
}


void do_peek( CHAR_DATA *ch, char *argument )
{
	int diff = 6;
	int dice = 0;
	CHAR_DATA *victim;

	CheckCH(ch);

	if (( victim = get_char_room( ch, argument ) ) == NULL || !SAME_PLANE(victim, ch))
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	if (ch == victim)
	{
		send_to_char("Just look at your inventory.\n\r", ch);
		return;
	}

	diff = get_curr_stat(victim, STAT_PER) + victim->ability[LARCENY].value;
	dice = ch->ability[SUBTERFUGE].value + 5;

	if ( can_see( victim, ch ) && dice_rolls(victim, dice, diff) )
	{
		act( "$n looks sideways at you.", ch, NULL, victim, TO_VICT,    1 );
		act( "$n looks sideways at $N.",  ch, NULL, victim, TO_NOTVICT, 1 );
	}

	diff = get_curr_stat(ch, STAT_PER) + ch->ability[SUBTERFUGE].value;
	dice = victim->ability[LARCENY].value;

	if(IS_ADMIN(ch)) { diff = 0; dice = 10; }

	if ( victim != ch &&   !IS_NPC(ch) &&   dice_rolls(ch, dice, diff) > 2)
	{
		send_to_char( "\n\rYou peek at the inventory:\n\r", ch );
		show_list_to_char( victim->carrying, ch, TRUE, TRUE );
	}
	else
	{
		send_to_char("You fail to see anything of their inventory.\n\r", ch);
	}

	return;
}



void show_char_to_char( CHAR_DATA *list, CHAR_DATA *ch )
{
	CHAR_DATA *rch;

	for ( rch = list; rch != NULL; rch = rch->next_in_room )
	{
		if ( rch == ch && ch->listening == NULL )
			continue;

		if ( get_trust(ch) < rch->invis_level)
			continue;

		if ( can_see( ch, rch ) )
		{
			show_char_to_char_0( rch, ch );
		}
		else if ( room_is_dark( ch->in_room )
				&&        IS_AFFECTED(rch, AFF_INFRARED ) )
		{
			send_to_char( "You see glowing red eyes watching YOU!\n\r", ch );
		}
	}

	return;
} 



bool check_blind( CHAR_DATA *ch )
{

	if (!IS_NPC(ch) && IS_SET(ch->plr_flags,PLR_HOLYLIGHT))
		return TRUE;

	if ( IS_AFFECTED(ch, AFF_BLIND) )
	{
		send_to_char( "You can't see a thing!\n\r", ch );
		return FALSE;
	}

	return TRUE;
}

/* RT Commands to replace news, motd, imotd, etc from ROM */

void do_motd(CHAR_DATA *ch, char *argument)
{
	do_function(ch, &do_help, "motd");
}

void do_imotd(CHAR_DATA *ch, char *argument)
{
	do_function(ch, &do_help, "imotd");
}

void do_rules(CHAR_DATA *ch, char *argument)
{
	do_function(ch, &do_help, "rules");
}

void do_wizlist(CHAR_DATA *ch, char *argument)
{
	do_function(ch, &do_help, "wizlist");
}

/* RT this following section holds all the auto commands from ROM, as well as
   replacements for config */

void do_autolist(CHAR_DATA *ch, char *argument)
{
	CheckCH(ch);

	/* lists most player flags */
	if (IS_NPC(ch))
		return;

	send_to_char("   action     status\n\r",ch);
	send_to_char("---------------------\n\r",ch);

	send_to_char("autoexit       ",ch);
	if (IS_SET(ch->plr_flags,PLR_AUTOEXIT))
		send_to_char("ON\n\r",ch);
	else
		send_to_char("OFF\n\r",ch);

	send_to_char("compact mode   ",ch);
	if (IS_SET(ch->comm,COMM_COMPACT))
		send_to_char("ON\n\r",ch);
	else
		send_to_char("OFF\n\r",ch);

	send_to_char("prompt         ",ch);
	if (IS_SET(ch->comm,COMM_PROMPT))
		send_to_char("ON\n\r",ch);
	else
		send_to_char("OFF\n\r",ch);

	send_to_char("combine items  ",ch);
	if (IS_SET(ch->comm,COMM_COMBINE))
		send_to_char("ON\n\r",ch);
	else
		send_to_char("OFF\n\r",ch);

	if (IS_SET(ch->plr_flags,PLR_NOFOLLOW))
		send_to_char("You do not welcome followers.\n\r",ch);
	else
		send_to_char("You accept followers.\n\r",ch);
}

void do_autoexit(CHAR_DATA *ch, char *argument)
{
	CheckCH(ch);

	if (IS_SET(ch->plr_flags,PLR_AUTOEXIT))
	{
		send_to_char("Exits will no longer be displayed.\n\r",ch);
		REMOVE_BIT(ch->plr_flags,PLR_AUTOEXIT);
	}
	else
	{
		send_to_char("Exits will now be displayed.\n\r",ch);
		SET_BIT(ch->plr_flags,PLR_AUTOEXIT);
	}
}

void do_brief(CHAR_DATA *ch, char *argument)
{
	CheckCH(ch);

	if (IS_SET(ch->comm,COMM_BRIEF))
	{
	  send_to_char("Full descriptions activated.\n\r",ch);
	  REMOVE_BIT(ch->comm,COMM_BRIEF);
	}
	else
	{
	  send_to_char("Short descriptions activated.\n\r",ch);
	  SET_BIT(ch->comm,COMM_BRIEF);
	}
}

void do_compact(CHAR_DATA *ch, char *argument)
{
	CheckCH(ch);

	if (IS_SET(ch->comm,COMM_COMPACT))
	{
	  send_to_char("Compact mode removed.\n\r",ch);
	  REMOVE_BIT(ch->comm,COMM_COMPACT);
	}
	else
	{
	  send_to_char("Compact mode set.\n\r",ch);
	  SET_BIT(ch->comm,COMM_COMPACT);
	}
}

void do_prompt(CHAR_DATA *ch, char *argument)
{
	char buf[MSL]={'\0'};

	CheckCH(ch);

	if ( IS_NULLSTR(argument) )
	{
		if (IS_SET(ch->comm,COMM_PROMPT))
		{
			send_to_char("You will no longer see prompts.\n\r",ch);
			REMOVE_BIT(ch->comm,COMM_PROMPT);
		}
		else
		{
			send_to_char("You will now see prompts.\n\r",ch);
			SET_BIT(ch->comm,COMM_PROMPT);
		}
		return;
	}

	if( !str_cmp( argument, "all" )  || !str_cmp( argument, "default" ) )
		strcpy( buf, "<\tY%h\tn> ");
	else
	{
		if ( strlen(argument) > 50 )
			argument[50] = '\0';
		strcpy( buf, argument );
		smash_tilde( buf );
		if (str_suffix("%c",buf))
			strcat(buf," " );

	}

	PURGE_DATA( ch->prompt );
	ch->prompt = str_dup( buf );
	send_to_char(Format("Prompt set to %s\n\r",ch->prompt),ch);
	return;
}

void do_combine(CHAR_DATA *ch, char *argument)
{
	CheckCH(ch);

	if (IS_SET(ch->comm,COMM_COMBINE))
	{
	  send_to_char("Long inventory selected.\n\r",ch);
	  REMOVE_BIT(ch->comm,COMM_COMBINE);
	}
	else
	{
	  send_to_char("Combined inventory selected.\n\r",ch);
	  SET_BIT(ch->comm,COMM_COMBINE);
	}
}

void do_nofollow(CHAR_DATA *ch, char *argument)
{
	CheckCH(ch);

	if (IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM))
		return;

	if (IS_SET(ch->plr_flags,PLR_NOFOLLOW))
	{
		send_to_char("You now accept followers.\n\r",ch);
		REMOVE_BIT(ch->plr_flags,PLR_NOFOLLOW);
	}
	else
	{
		send_to_char("You no longer accept followers.\n\r",ch);
		SET_BIT(ch->plr_flags,PLR_NOFOLLOW);
		die_follower( ch );
	}
}

void state_obj_cond(OBJ_DATA *obj, CHAR_DATA *ch)
{

	if(obj->condition > 90)
		send_to_char("It is in \tGexcellent condition\tn.\n\r", ch);
	else if(obj->condition > 80)
		send_to_char("It is in \tGgood condition\tn.\n\r", ch);
	else if(obj->condition > 70)
		send_to_char("It has begun to show signs of wear.\n\r", ch);
	else if(obj->condition > 60)
		send_to_char("It is showing signs of wear.\n\r", ch);
	else if(obj->condition > 50)
		send_to_char("It is in fair condition.\n\r", ch);
	else if(obj->condition > 40)
		send_to_char("It is not looking so good.\n\r", ch);
	else if(obj->condition > 30)
		send_to_char("It is in \tOpoor condition\tn.\n\r", ch);
	else if(obj->condition > 20)
		send_to_char("It is in \tRvery bad condition\tn.\n\r", ch);
	else
		send_to_char("It is \tRfalling apart\tn.\n\r", ch);

	send_to_char(Format("It is of %s quality.\n\r", quality_flags[obj->quality].name), ch);

	if(obj->item_type == ITEM_BOMB)
		send_to_char("It is ticking softly.\n\r", ch);
}

int get_door(CHAR_DATA *ch, char *argument)
{
	ROOM_INDEX_DATA *looking;
	int door = 0;

	if(IS_SET(ch->act2, ACT2_ASTRAL))
		looking = ch->listening;
	else
		looking = ch->in_room;

	for(door = 0; door < 6; door++)
		if(looking->exit[door]
		&& (is_name(argument, looking->exit[door]->keyword)
		   || !str_prefix(argument, dir_name[door])))
			return door;

	return -1;
}

void do_look( CHAR_DATA *ch, char *argument )
{
	EXIT_DATA *pexit;
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	ROOM_INDEX_DATA *original;
	ROOM_INDEX_DATA *looking;
	char arg1 [MAX_INPUT_LENGTH]={'\0'};
	char arg2 [MAX_INPUT_LENGTH]={'\0'};
	char arg3 [MAX_INPUT_LENGTH]={'\0'};
	char *pdesc;
	int door = 0;
	int number = 0,count = 0;
	
	CheckCH(ch);

	if ( ch->desc == NULL )
		return;

	if ( ch->position < P_SLEEP )
	{
		send_to_char( "You can't see anything but stars!\n\r", ch );
		return;
	}

	if ( ch->position == P_SLEEP && !IS_SET(ch->act2, ACT2_ASTRAL))
	{
		send_to_char("Your dreams are filled with nightmarish images!\n\r",ch);
		return;
	}

	if (IS_SET(ch->affected_by2, AFF2_EARTHMELD))
	{
		send_to_char("It's dark... really dark...\n\r", ch);
		return;
	}

	if ( !check_blind( ch ) )
		return;

	if(IS_SET(ch->act2, ACT2_ASTRAL))
		looking = ch->listening;
	else
		looking = ch->in_room;

	if ( !IS_NPC(ch)
			&&   !IS_SET(ch->plr_flags, PLR_HOLYLIGHT)
			&&   !IS_AFFECTED(ch, AFF_DARK_VISION)
			&&   room_is_dark( looking ) )
	{
		send_to_char( "It is pitch black ... \n\r", ch );
		show_char_to_char( looking->people, ch );
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	number = number_argument(arg1,arg3);
	count = 0;

	if ( IS_NULLSTR(arg1) || !str_cmp( arg1, "auto" ) )
	{
		/* 'look' or 'look auto' */
		if ((IS_ADMIN(ch)
				&& ((IS_NPC(ch) && ch->desc->original && IS_ADMIN(ch->desc->original))
						|| IS_SET(ch->plr_flags,PLR_HOLYLIGHT)))
						|| ch->pcdata->security > 0)
		{
			send_to_char(Format(" [\tcRoom %d\tn] ",looking->vnum),ch);
		}

		if(IS_SET(ch->act,ACT_UMBRA))
		{
			send_to_char( Format("\tW%s\tn", looking->uname), ch );

			send_to_char( "\n\r", ch );

			if(!IS_NPC(ch) && !IS_SET(ch->comm, COMM_BRIEF)) 
			{
				send_to_char( "\tB", ch);
				send_to_char( "  ",ch);
				send_to_char( looking->udescription, ch );
				send_to_char( "\tn", ch);
			}
		}
		else if(IS_SET(ch->act,ACT_DREAMING))
		{
			send_to_char( Format("\tG%s\tn", looking->dname), ch );

			send_to_char( "\n\r", ch );

			if(!IS_NPC(ch) && !IS_SET(ch->comm, COMM_BRIEF)) {
				send_to_char( "\ty", ch);
				send_to_char( "  ",ch);
				send_to_char( looking->ddescription, ch );
				send_to_char( "\tn", ch);
			}
		}

		else
		{
			send_to_char( Format("\tC%s\tn", looking->name), ch );

			send_to_char( "\n\r", ch );

			// Show the room description. Make the description text white.
			if(!IS_NPC(ch) && !IS_SET(ch->comm, COMM_BRIEF)) 
			{
				send_to_char( "\tW", ch);
				send_to_char( "  ",ch);
				send_to_char( looking->description, ch );
				send_to_char( "\tn", ch);
			}
		}

		// Show the exits. Need to colorize.
		if ( IS_SET(ch->plr_flags, PLR_AUTOEXIT) )
		{
			send_to_char("\n\r",ch);
			do_function(ch, &do_exits, "auto");
		}

		// Show objects in the room.
		send_to_char("\tW---\tYObjects\tW---\tn\n\r", ch);
		show_list_to_char( ch->in_room->contents, ch, FALSE, FALSE );
		send_to_char("\n\r", ch);

		// Show people and mobs in the room.
		send_to_char("\tW---\tYPeople\tW---\tn\n\r", ch);
		show_char_to_char( ch->in_room->people, ch );

		/*  if(IS_SET(ch->act, ACT_UMBRA))
		show_char_to_char( looking->people,   ch ); */
		return;
	}

	if ( !str_cmp( arg1, "i" ) || !str_cmp(arg1, "in")  || !str_cmp(arg1,"on"))
	{
		/* 'look in' */
		if ( IS_NULLSTR(arg2) )
		{
			send_to_char( "Look in what?\n\r", ch );
			return;
		}

		if ( ( obj = get_obj_here( ch, arg2 ) ) == NULL )
		{
			send_to_char( "You do not see that here.\n\r", ch );
			return;
		}

		switch ( obj->item_type )
		{
		default:
			send_to_char( "That is not a container.\n\r", ch );
			break;

		case ITEM_DRINK_CON:
			if ( obj->value[1] <= 0 )
			{
				send_to_char( "It is empty.\n\r", ch );
				break;
			}

			send_to_char( Format("It's %s filled with a %s liquid.\n\r",
					obj->value[1] <     obj->value[0] / 4 ? "less than half-" : obj->value[1] < 3 * obj->value[0] / 4 ? "about half-"     : "more than half-",
					liq_table[obj->value[2]].liq_color), ch );
			break;

		case ITEM_WEAPON:
			if(obj->value[0] == WEAPON_FIREARM)
			{
				send_to_char(Format("%s contains %d rounds.\n\r", obj->short_descr, obj->value[3]), ch);
			}
			break;

		case ITEM_CONTAINER:
		case ITEM_CORPSE_NPC:
		case ITEM_CORPSE_PC:
			if ( IS_SET(obj->value[1], CONT_CLOSED) )
			{
				send_to_char( "It is closed.\n\r", ch );
				break;
			}

			act( "$p holds:", ch, obj, NULL, TO_CHAR, 1 );
			show_list_to_char( obj->contains, ch, TRUE, TRUE );
			break;

		case ITEM_FURNITURE:
			act( "$p holds:", ch, obj, NULL, TO_CHAR, 1 );
			show_list_to_char( obj->contains, ch, TRUE, TRUE );
			break;
		}
		return;
	}

	if ( ( victim = get_char_room( ch, arg1 ) ) != NULL
			&& SAME_PLANE(victim, ch) )
	{
		if(IS_NULLSTR(arg2)) {
			show_char_to_char_1( victim, ch );
		} else if ( can_see( ch, victim ) && SAME_PLANE(victim, ch)
				&& (pdesc = get_extra_descr( arg2, victim->extra_descr )) != NULL)
		{
			if (++count == number)
			{
				send_to_char( pdesc, ch );
				return;
			}
		} else act("You don't see anything special on $N like that.",
				ch, NULL, victim, TO_CHAR, 1);
		if(!IS_SET(ch->act2, ACT2_ASTRAL))
			act("$n looks at you.", ch, NULL, victim, TO_VICT, 1);

		return;
	}

	for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
	{
		if ( can_see_obj( ch, obj ) )
		{  /* player can see object */
			pdesc = get_extra_descr( arg3, obj->extra_descr );
			if ( pdesc != NULL )
			{
				if (++count == number)
				{
					send_to_char( pdesc, ch );
					return;
				}
				else continue;
			}

			pdesc = get_extra_descr( arg3, obj->pIndexData->extra_descr );
			if ( pdesc != NULL )
			{
				if (++count == number)
				{
					send_to_char( pdesc, ch );
					return;
				}
				else continue;
			}

			if (!str_prefix(arg3, "package")
					&& IS_SET(obj->extra2, OBJ_PACKAGED))
				if (++count == number)
				{
					send_to_char( "An unobtrusively wrapped package sits here.", ch );
					send_to_char( "\n\r", ch);
					state_obj_cond(obj,ch);
					return;
				}

			if ( is_name( arg3, obj->name ))
				if (++count == number)
				{
					send_to_char( obj->full_desc, ch );
					send_to_char( "\n\r", ch);
					state_obj_cond(obj,ch);
					return;
				}
		}
	}

	for ( obj = looking->contents; obj != NULL; obj = obj->next_content )
	{
		if ( can_see_obj( ch, obj ) )
		{
			pdesc = get_extra_descr( arg3, obj->extra_descr );
			if ( pdesc != NULL )
				if (++count == number)
				{
					send_to_char( pdesc, ch );
					return;
				}

			pdesc = get_extra_descr( arg3, obj->pIndexData->extra_descr );
			if ( pdesc != NULL )
				if (++count == number)
				{
					send_to_char( pdesc, ch );
					return;
				}

			if (!str_prefix(arg3, "package")
					&& IS_SET(obj->extra2, OBJ_PACKAGED))
				if (++count == number)
				{
					send_to_char( "An unobtrusively wrapped package sits here.", ch );
					send_to_char( "\n\r", ch);
					state_obj_cond(obj,ch);
					return;
				}

			if ( is_name( arg3, obj->name ) )
				if (++count == number)
				{
					send_to_char( obj->full_desc, ch );
					send_to_char( "\n\r", ch);
					state_obj_cond(obj,ch);
					return;
				}
		}
	}

	pdesc = get_extra_descr(arg3,looking->extra_descr);
	if (pdesc != NULL)
	{
		if (++count == number)
		{
			send_to_char(pdesc,ch);
			return;
		}
	}

	if (count > 0 && count != number)
	{
		if (count == 1)
		{
			send_to_char( Format("You only see one %s here.\n\r",arg3), ch);
		}
		else
		{
			send_to_char( Format("You only see %d of those here.\n\r",count), ch);
		}

		return;
	}

	if ( !str_cmp( arg1, "n" ) || !str_cmp( arg1, "north" ) ) door = 0;
	else if ( !str_cmp( arg1, "e" ) || !str_cmp( arg1, "east"  ) ) door = 1;
	else if ( !str_cmp( arg1, "s" ) || !str_cmp( arg1, "south" ) ) door = 2;
	else if ( !str_cmp( arg1, "w" ) || !str_cmp( arg1, "west"  ) ) door = 3;
	else if ( !str_cmp( arg1, "u" ) || !str_cmp( arg1, "up"    ) ) door = 4;
	else if ( !str_cmp( arg1, "d" ) || !str_cmp( arg1, "down"  ) ) door = 5;
	else if ( (door = get_door(ch, arg1)) == -1 )
	{
		send_to_char( "You do not see that here.\n\r", ch );
		return;
	}

	/* 'look direction' */
	if ( ( pexit = looking->exit[door] ) == NULL )
	{
		send_to_char( "Nothing special there.\n\r", ch );
		return;
	}

	if(IS_SET(pexit->exit_info, EX_WINDOW))
	{
		send_to_char(Format("%s\n\r", pexit->description == '\0' ? "Through the window you see...\n\r" : pexit->description), ch);
		original = looking;
		obj = ch->on;
		char_from_room( ch );
		char_to_room( ch, pexit->u1.to_room );
		do_function(ch, &do_look, "");
		char_from_room( ch );
		char_to_room( ch, original );
		ch->on = obj;
	}
	else if ( pexit->description != NULL && !IS_NULLSTR(pexit->description) )
		send_to_char( pexit->description, ch );
	else
		send_to_char( "Nothing special there.\n\r", ch );

	if ( pexit->keyword    != NULL && !IS_NULLSTR(pexit->keyword) && pexit->keyword[0] != ' ' )
	{
		if ( IS_SET(pexit->exit_info, EX_CLOSED) )
		{
			act( "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR, 1 );
		}
		else if ( IS_SET(pexit->exit_info, EX_ISDOOR) )
		{
			act( "The $d is open.",   ch, NULL, pexit->keyword, TO_CHAR, 1 );
		}
	}

	return;
}

/* RT added back for the hell of it */
void do_read (CHAR_DATA *ch, char *argument )
{
	CheckCH(ch);

	if(IS_NULLSTR(argument))
	{
		send_to_char("What do you want to read?\n\r", ch);
		return;
	}

	do_function(ch, &do_look, "argument");
}

void do_examine( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH]={'\0'};
	OBJ_DATA *obj;

	CheckCH(ch);

	one_argument( argument, arg );

	if ( IS_NULLSTR(arg) )
	{
		send_to_char( "Examine what?\n\r", ch );
		return;
	}

	do_function(ch, &do_look, arg);

	if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
	{
		switch ( obj->item_type )
		{
		default:
			break;

		case ITEM_MONEY:
			if (obj->value[0] == 0)
			{
				if (obj->value[1] == 0)
				{
					send_to_char("Odd...there's no coins in the pile.\n\r", ch);
				}
				else if (obj->value[1] == 1)
				{
					send_to_char("Wow. One whole dollar.\n\r", ch);
				}
				else
				{
					send_to_char(Format("There are %d dollars in the pile.\n\r", obj->value[1]), ch);
				}
			}
			else if (obj->value[1] == 0)
			{
				if (obj->value[0] == 1)
				{
					send_to_char( "Wow. One cent.\n\r", ch);
				}
				else
				{
					send_to_char(Format("There are %d cents in the pile.\n\r", obj->value[0]), ch);
				}
			}
			else
				send_to_char(Format("There are %d dollars and %d cents in the pile.\n\r", obj->value[1],obj->value[0]),ch);
			break;

		case ITEM_WEAPON:
		case ITEM_DRINK_CON:
		case ITEM_CONTAINER:
		case ITEM_CORPSE_NPC:
		case ITEM_CORPSE_PC:
			do_function(ch, &do_look, (char *)Format("in %s", argument));
		}
	}

	return;
}



/*
 * Thanks to Zrin for auto-exit part.
 */
void do_exits( CHAR_DATA *ch, char *argument )
{
	ROOM_INDEX_DATA *looking;
	EXIT_DATA *pexit;
	extern char * const dir_name[];
	char buf[MSL]={'\0'}, buf2[MSL]={'\0'};
	int door = 0;
	bool found = FALSE;
	bool fAuto;

	CheckCH(ch);

	fAuto  = !str_cmp( argument, "auto" );

	if ( !check_blind( ch ) )
		return;

	if(IS_SET(ch->act2, ACT2_ASTRAL))
	{
		looking = ch->listening;
	}
	else
	{
		looking = ch->in_room;
	}

	if (fAuto)
	{
		send_to_char("[\tCExits:\tn", ch);
	}
	else if (IS_ADMIN(ch))
	{
		send_to_char(Format("\tCObvious exits from room %d:\tn\n\r",looking->vnum), ch);
	}
	else
	{
		send_to_char("\tCObvious exits:\tn\n\r", ch);
	}

	for ( door = 0; door <= 5; door++ )
	{
		if(!IS_ADMIN(ch))
		{
			if ( ( pexit = looking->exit[door] ) != NULL
					&&   pexit->u1.to_room != NULL
					&&   can_see_room(ch,pexit->u1.to_room)
			&&   !IS_SET(pexit->exit_info, EX_HIDDEN))
			{
				found = TRUE;
				if ( fAuto )
				{
					if( !IS_SET(pexit->exit_info, EX_WINDOW) )
					{
						strncat( buf, " ", sizeof(buf) );
						if( IS_SET(pexit->exit_info, EX_HIDDEN) )
							strncat( buf, "\tR-", sizeof(buf) );
						if( IS_SET(pexit->exit_info, EX_CLOSED) )
							strncat( buf, "(", sizeof(buf) );
						strncat( buf, dir_name[door], sizeof(buf) );
						if( IS_SET(pexit->exit_info, EX_CLOSED) )
							strncat( buf, ")", sizeof(buf) );
						if( IS_SET(pexit->exit_info, EX_HIDDEN) )
							strncat( buf, "-\tn", sizeof(buf) );
					}
					else
					{
						strncat( buf, " <", sizeof(buf) );
						strcat(buf, dir_name[door]);
						strncat( buf, ">", sizeof(buf) );
					}
				}
				else
				{
					if(!IS_SET(pexit->exit_info, EX_HIDDEN))
					{
						snprintf( buf2, MSL, "%-5s - %s", capitalize( dir_name[door] ), room_is_dark( pexit->u1.to_room )
								?  "Too dark to tell" : pexit->u1.to_room->name );
						strcat(buf, buf2);
						if (IS_ADMIN(ch))
						{
							snprintf( buf2, MSL, " (room %d)\n\r", pexit->u1.to_room->vnum);
						}
						else
							{
								snprintf( buf2, MSL, "\n\r");
							}
						strcat(buf, buf2);
					}
				}
			}
		}
		else
		{
			if ( ( pexit = looking->exit[door] ) != NULL
					&&   pexit->u1.to_room != NULL
					&&   can_see_room(ch,pexit->u1.to_room) )
			{
				found = TRUE;
				if ( fAuto )
				{
					strncat( buf, " ", sizeof(buf) );
					if( IS_SET(pexit->exit_info, EX_WINDOW) )
						strncat( buf, "<", sizeof(buf) );
					else if( IS_SET(pexit->exit_info, EX_CLOSED) )
					{
						if( IS_SET(pexit->exit_info, EX_HIDDEN) )
							strncat( buf, "\tR-(", sizeof(buf) );
						else
							strncat( buf, "(", sizeof(buf) );
					}
					strcat(buf, dir_name[door]);
					if( IS_SET(pexit->exit_info, EX_WINDOW) )
						strncat( buf, ">", sizeof(buf) );
					else if( IS_SET(pexit->exit_info, EX_CLOSED) )
					{
						if( IS_SET(pexit->exit_info, EX_HIDDEN) )
							strncat( buf, ")-\tn", sizeof(buf) );
						else
							strncat( buf, ")", sizeof(buf) );
					}
				}
				else
				{
					snprintf( buf2, MSL, "%-5s - %s", capitalize( dir_name[door] ), room_is_dark( pexit->u1.to_room )
							?  "Too dark to tell" : pexit->u1.to_room->name );
					strcat(buf, buf2);
					if (IS_ADMIN(ch))
						{
							snprintf( buf2, MSL, " (room %d)\n\r", pexit->u1.to_room->vnum);
						}
					else
						{
							snprintf( buf2, MSL, "\n\r");
						}
					strcat(buf, buf2);
				}
			}
		}
	}

	if ( !found )
		strncat( buf, fAuto ? " none" : "None.\n\r", sizeof(buf) );

	if ( fAuto )
		strncat( buf, "]\n\r", sizeof(buf) );

	send_to_char( buf, ch );
	send_to_char( "", ch );
	return;
}

void do_score( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *user = ch;
	int num = 0, i = 0;
	int statcount = 0;

	CheckCH(ch);

	if(!IS_NULLSTR(argument) && IS_ADMIN(user))
	{
		if((ch = get_char_world(user, argument)) == NULL)
		{
			send_to_char("They aren't online.\n\r", user);
			return;
		}

		if(ch->trust > user->trust)
		{
			send_to_char("Not available for those with higher authority.\n\r", user);
			return;
		}
	}

	if(ch->race == race_lookup("human") && IS_SET(ch->act2, ACT2_GHOUL))
	{
		send_to_char( Format("\r\n\tW---------------------------------<\tG   Ghoul\tW>-----------------------------------\tn\r\n"), user);
	}
	else
	{
		send_to_char( Format("\n\r\tW---------------------------------<\tG%8s\tW>-----------------------------------\tn\n\r",
				capitalize(race_table[ch->race].name)), user);
	}

	/* snprintf(altname, sizeof(altname), " [%s]", !IS_NULLSTR(ch->alt_name) ? ch->alt_name : "" ); */

	send_to_char( Format("First Name: %-15s  ", IS_NPC(ch) ? ch->short_descr : ch->name ), user);
	if(ch->race == race_lookup("werewolf"))
		send_to_char( Format("Breed: %-15s  ",breed_table[ch->breed].name), user);
	else
		send_to_char( Format("Nature: %-15s  ", capitalize(ch->nature)), user);

	if(ch->race == race_lookup("werewolf"))
		send_to_char( "Pack Name: ", user);
	else if (ch->race == race_lookup("vampire"))
		send_to_char( Format("Clan: %-20s", capitalize(clan_table[ch->clan].name)), user);
	else
		send_to_char( Format("Group: %-19s", capitalize(clan_table[ch->clan].name)), user);
	send_to_char("\r\n", user);

	send_to_char( Format("Last Name: %s%-15s  ", !IS_NULLSTR(ch->surname)? " " : " ", !IS_NULLSTR(ch->surname)? ch->surname : ""), user);

	if(ch->race == race_lookup("werewolf"))
		send_to_char( Format("Auspice: %-13s  ",auspice_table[ch->auspice].name), user);
	else
		send_to_char( Format("Demeanor: %-13s  ", capitalize(ch->demeanor)), user);

	if(ch->race == race_lookup("werewolf"))
		send_to_char( "Pack Totem: ", user);
	else if (ch->race == race_lookup("vampire"))
		send_to_char( Format("Generation: %d", ch->gen), user);
	else
		send_to_char("", user);

	send_to_char("\r\n", user);

	if(IS_ADMIN(user))
	{
		if(ch->trust > MAX_LEVEL || ch->trust < 0)
			ch->trust = 1;
		send_to_char( Format("\tWAdmin Level: %-14s  \tn", staff_status[ch->trust].name), user);
	}
	else
	{
		send_to_char(Format("%29s", " "), user);
	}

	if(ch->race == race_lookup("werewolf"))
		send_to_char( Format("Tribe: %-15s  ",capitalize(clan_table[ch->clan].name)), user);
	else
		send_to_char( Format("Profession: %-12s ", ch->profession), user);

	if(ch->race == race_lookup("werewolf"))
		send_to_char( Format("Profession: %-16s  ", ch->profession), user);
	else if(ch->race == race_lookup("vampire"))
		send_to_char( Format("Sire: %-16s  ", ch->sire), user);
	else
		send_to_char("", user);
   
	send_to_char("\r\n", user);

	send_to_char("\tW--------------------------------<\tGAttributes\tW>----------------------------------\tn\n\r", user);

	send_to_char ("Str: ", user);
	if (get_curr_stat(ch,STAT_STR)<=0)
	{
		send_to_char( "\t[U9675/O]", user);
		statcount=1;
		while (statcount < 10)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
	}
	else
	{
		while (statcount < get_curr_stat(ch,STAT_STR))
		{
			send_to_char( "\t[U9679/*]", user);
			statcount++;
		}
		while (statcount < 10)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
	}

	send_to_char (Format("%14sCHA: ", " "), user);
	statcount = 0;
	if (get_curr_stat(ch,STAT_CHA)<=0)
	{
		send_to_char( "\t[U9675/O]", user);
		statcount=1;
		while (statcount < 10)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
	}
	else
	{
		while (statcount < get_curr_stat(ch,STAT_CHA))
		{
			send_to_char( "\t[U9679/*]", user);
			statcount++;
		}
		while (statcount < 10)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
	}

	send_to_char (Format("%10sPER: ", " "), user);
	statcount = 0;
	if (get_curr_stat(ch,STAT_PER)<=0)
	{
		send_to_char( "\t[U9675/O]", user);
		statcount=1;
		while (statcount < 10)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
   }
	else
	{
		while (statcount < get_curr_stat(ch,STAT_PER))
		{
			send_to_char( "\t[U9679/*]", user);
			statcount++;
		}
		while (statcount < 10)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
	}
	send_to_char("\n\r", user);

	send_to_char ("DEX: ", user);
	statcount = 0;
	if (get_curr_stat(ch,STAT_DEX)<=0)
	{
		send_to_char( "\t[U9675/O]", user);
		statcount=1;
		while (statcount < 10)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
	}
	else
	{
		while (statcount < get_curr_stat(ch,STAT_DEX))
		{
			send_to_char( "\t[U9679/*]", user);
			statcount++;
		}
		while (statcount < 10)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
	}

	send_to_char (Format("%14sMAN: ", " "), user);
	statcount = 0;
	if (get_curr_stat(ch,STAT_MAN)<=0)
	{
		send_to_char( "\t[U9675/O]", user);
		statcount=1;
		while (statcount < 10)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
	}
	else
	{
		while (statcount < get_curr_stat(ch,STAT_MAN))
		{
			send_to_char( "\t[U9679/*]", user);
			statcount++;
		}
		while (statcount < 10)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
	}

	send_to_char (Format("%10sINT: ", " "), user);
	statcount = 0;
	if (get_curr_stat(ch,STAT_INT)<=0)
	{
		send_to_char( "\t[U9675/O]", user);
		statcount=1;
		while (statcount < 10)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
	}
	else
	{
		while (statcount < get_curr_stat(ch,STAT_INT))
		{
			send_to_char( "\t[U9679/*]", user);
			statcount++;
		}
		while (statcount < 10)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
	}
	send_to_char("\n\r", user);

	send_to_char ("STA: ", user);
	statcount = 0;
	if (get_curr_stat(ch,STAT_STA)<=0)
	{
		send_to_char( "\t[U9675/O]", user);
		statcount=1;
		while (statcount < 10)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
	}
	else
	{
		while (statcount < get_curr_stat(ch,STAT_STA))
		{
			send_to_char( "\t[U9679/*]", user);
			statcount++;
		}
		while (statcount < 10)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
	}

	send_to_char (Format("%14sAPP: ", " "), user);
	statcount = 0;
	if (get_curr_stat(ch,STAT_APP)<=0)
	{
		send_to_char( "\t[U9675/O]", user);
		statcount=1;
		while (statcount < 10)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
	}
	else
	{
		while (statcount < get_curr_stat(ch,STAT_APP))
		{
			send_to_char( "\t[U9679/*]", user);
			statcount++;
		}
		while (statcount < 10)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
	}

	send_to_char (Format("%10sWIT: ", " "), user);
	statcount = 0;
	if (get_curr_stat(ch,STAT_WIT)<=0)
	{
		send_to_char( "\t[U9675/O]", user);
		statcount=1;
				while (statcount < 10)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
	}
	else
	{
		while (statcount < get_curr_stat(ch,STAT_WIT))
		{
			send_to_char( "\t[U9679/*]", user);
			statcount++;
		}
		while (statcount < 10)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
	}
	statcount=0;
	send_to_char("\n\r", user);

	send_to_char("\tW--------------------------------<\tGAdvantages\tW>----------------------------------\tn\n\r", user);

/*    send_to_char( "Virtues:\n\r", user );*/
	if(ch->race != race_lookup("werewolf"))
	{
		send_to_char(Format("%s:  %-20d Conscience: %d\n\r",
				race_table[ch->race].pc_race?pc_race_table[ch->race].GHB:"Humanity", ch->GHB, ch->virtues[0]), user);
	}
	else
	{
		send_to_char(Format("%s:  %2d/%-20d Conscience: %d\n\r",
				race_table[ch->race].pc_race?pc_race_table[ch->race].GHB:"Humanity", ch->GHB, ch->max_GHB, ch->virtues[0]), user);
	}


	send_to_char( Format("%s:  %2d/%-20d Self-Control: %d\n\r", race_table[ch->race].pc_race?pc_race_table[ch->race].RBPG:"Faith",
			ch->RBPG, ch->max_RBPG, ch->virtues[1]), user);

	send_to_char( Format("Willpower: %2d/%-17d Courage: %d\n\r", ch->willpower, ch->max_willpower, ch->virtues[2]), user);

	send_to_char( Format("Health: %-22s  ", health_string(ch)), user );
	send_to_char( Format("Carrying: %d/%d lbs.\n\r", get_carry_weight(ch) / 10, can_carry_w(ch) /10), user );

	if((num = ch->cents/100) > 0)
	{
		ch->dollars = ch->dollars + num;
		ch->cents = ch->cents - (100 * num);
	}

	send_to_char("\tW------------------------------------------------------------------------------\tn\n\r", user);

	send_to_char(Format("Experience/OOC Experience: %d / %d\n\r", ch->exp, ch->oocxp), user);
	send_to_char(Format("Experience to Gift: %d\n\r", ch->xpgift), user);
	send_to_char(Format("Cash on Hand: $ %d.%.2d.\n\r", ch->dollars, ch->cents), user);

	send_to_char( "You are: ", user );
	if ( !IS_NPC(ch) && IS_DRUNK(ch) )
		send_to_char( "drunk ",   user );
	if ( !IS_NPC(ch) && IS_HIGH(ch) )
		send_to_char( "high ",   user );
	if ( !IS_NPC(ch) && IS_TRIPPING(ch) )
		send_to_char( "tripping ",   user );
	if ( !IS_NPC(ch) && NEAR_FRENZY(ch) && !IS_SET(ch->act2, ACT2_FRENZY) )
		send_to_char( "near frenzy ", user );
	else if(IS_SET(ch->act2, ACT2_FRENZY))
		send_to_char( "frenzied ", user );
	if ( !IS_NPC(ch) && ch->condition[COND_THIRST] ==  0 )
		send_to_char( "thirsty ", user );
	if ( !IS_NPC(ch) && ch->condition[COND_HUNGER]   ==  0 )
		send_to_char( "hungry ",  user );
	if ( !IS_NPC(ch) && ch->condition[COND_ANGER]   >  10 )
		send_to_char( "angry ",  user );
	if ( !IS_NPC(ch) && ch->condition[COND_PAIN]   >  10 )
		send_to_char( "in pain ",  user );

	switch ( ch->position )
	{
	case P_DEAD:
		send_to_char( "-=<DEAD>=-\n\r",     user );
		break;
	case P_MORT:
		send_to_char( "mortally wounded.\n\r",  user );
		break;
	case P_INCAP:
		send_to_char( "incapacitated.\n\r", user );
		break;
	case P_TORPOR:
		send_to_char( "in torpor.\n\r",     user );
		break;
	case P_STUN:
		send_to_char( "stunned.\n\r",       user );
		break;
	case P_SLEEP:
		send_to_char( "sleeping.\n\r",      user );
		break;
	case P_REST:
		send_to_char( "resting.\n\r",       user );
		break;
	case P_SIT:
		send_to_char( "sitting.\n\r",       user );
		break;
	case P_STAND:
		send_to_char( "standing.\n\r",      user );
		break;
	case P_FIGHT:
		send_to_char( "fighting.\n\r",      user );
		break;
	}

	if(IS_SET(ch->act2, ACT2_GHOUL))
	{
		send_to_char(Format("You are the devoted servant of %s.\n\r", ch->ghouled_by), user);
	}

	send_to_char("\tW--------------------------------<\tGBackgrounds\tW>---------------------------------\tn\n\r", user);

	i = 0;
	for(num=0;background_table[num].name;num++)
	{
		if(background_table[num].settable)
		{
			if(num < MAX_BACKGROUND)
			{
				send_to_char(Format("\t<send href='help %s'>%11s\t</send>:%3d ", background_table[num].name, background_table[num].name,
						ch->backgrounds[num]), user);
				i++;
			}
		}
		if(i == 4)
		{
			send_to_char("\n\r", user);
			i = 0;
		}
	}

	//send_to_char("\n\r\n\r", user);

	send_to_char("\tW---------------------------------<\tGInfluences\tW>---------------------------------\tn\n\r", user);

	for(num=0;influence_table[num].name;num++)
	{
		send_to_char(Format("%11s:%3d ", influence_table[num].name, ch->influences[num]), user);
		if((num+1)%4 == 0)
			send_to_char("\n\r", user);
	}
	send_to_char("\n\r", user);
}

void do_affects(CHAR_DATA *ch, char *argument )
{
	AFFECT_DATA *paf, *paf_last = NULL;

	CheckCH(ch);

	send_to_char( "Affects:\n\r", ch );

	if ( ch->affected != NULL )
	{
		for ( paf = ch->affected; paf != NULL; paf = paf->next )
		{
			send_to_char(Format("%-15s", skill_table[paf->type].name), ch);

			if ( paf->duration == -1 )
			{
				send_to_char( " is a permanent affect", ch );
			}
			else
			{
				send_to_char( Format(" will affect you for %d hours", paf->duration), ch );
			}
		}

		send_to_char( "\n\r", ch );
		paf_last = paf;
	}
	else
		send_to_char("Nothing is affecting you.\n\r",ch);

	return;
}

char *  const   month_name  [] =
{
	"January", "February", "March", "April",
	"May", "June", "July", "August", "September",
	"October", "November", "December"
};

void do_time( CHAR_DATA *ch, char *argument )
{
	extern char str_boot_time[];
	char *suf;
	int day = time_info.day + 1;

	CheckCH(ch);

	if ( day > 4 && day <  20 ) suf = "th";
	else if ( day % 10 ==  1       ) suf = "st";
	else if ( day % 10 ==  2       ) suf = "nd";
	else if ( day % 10 ==  3       ) suf = "rd";
	else                             suf = "th";

	send_to_char(Format("It is %d o'clock %s, %d%s %s.\n\r", (time_info.hour % 12 == 0) ? 12 : time_info.hour %12,
			time_info.hour >= 12 ? "pm" : "am", day, suf, month_name[time_info.month]),ch);
	if(time_info.hour < 7 || time_info.hour > 18)
	{
		switch(time_info.moon_phase)
		{
		case MOON_FULL:
			send_to_char("A full moon hangs in the sky.\n\r", ch);
			break;
		case MOON_GIBBOUS:
			send_to_char("A fat gibbous moon ambles across the sky.\n\r", ch);
			break;
		case MOON_NEW:
			send_to_char("The moon is absent from the night sky.\n\r", ch);
			break;
		case MOON_CRESCENT:
			send_to_char("A mere sliver of moon peers down from the sky.\n\r", ch);
			break;
		case MOON_HALF:
			send_to_char("The moon shines with a balance of light and dark.\n\r", ch);
			break;
		}
	}
	if(timestop)
	{
		send_to_char("Game time has been stopped.\n\r", ch);
	}

	send_to_char( Format("Process started: %sSystem time: %s\n\r", str_boot_time, (char *) ctime( &current_time )), ch );
	return;
}


void do_lastboot(CHAR_DATA *ch, char *argument)
{
	extern char str_boot_time[];

	CheckCH(ch);

	send_to_char( Format("Process started: %sSystem time: %s\n\r", str_boot_time, (char *) ctime( &current_time )), ch );
	return;
}


void do_weather( CHAR_DATA *ch, char *argument )
{
	CheckCH(ch);

	static char * const sky_look[5] =
	{
			"cloudless",
			"cloudy",
			"rainy",
			"lit by flashes of lightning",
			"foggy"
	};

	if ( !IS_OUTSIDE(ch) )
	{
		send_to_char( "You can't see the weather indoors.\n\r", ch );
		return;
	}

	send_to_char( Format("The sky is %s and %s.\n\r",
			sky_look[weather_info.sky], weather_info.change >= 0 ? "a warm southerly breeze blows" : "a cold northern gust blows"), ch );
	return;
}

void do_help( CHAR_DATA *ch, char *argument )
{
	HELP_DATA *pHelp;
	BUFFER *output;
	char argall[MAX_INPUT_LENGTH]={'\0'};
	char argone[MAX_INPUT_LENGTH]={'\0'};
	int level = 0;
	bool found = FALSE;

	CheckCH(ch);

	output = new_buf();

	if ( IS_NULLSTR(argument) )
		argument = "summary";

	/* this parts handles help a b so that it returns help 'a b' */
	argall[0] = '\0';
	while (!IS_NULLSTR(argument) )
	{
		argument = one_argument(argument,argone);
		if (!IS_NULLSTR(argall))
			strncat(argall," ", sizeof(argall) );
		strncat(argall,argone, sizeof(argall) );
	}

	if ( strlen( argall ) == 1 )
	{
		add_buf(output, (char *)Format("Topics beginning with %s:\n\r", argall));
	}

	for ( pHelp = help_list; pHelp != NULL; pHelp = pHelp->next )
	{
		if(pHelp->level == -1) level = -1;
		else level = (pHelp->level < 0) ? -1 * pHelp->level - 1 : pHelp->level;

		if (level >= get_trust( ch ) )
			continue;

		if ( is_name( argall, pHelp->keyword ) )
		{
			/* add seperator if found */
			if (found && strlen( argall ) > 1 )
				add_buf(output, "\n\r============================================================\n\r\n\r");
			if ( pHelp->level >= 0 && str_cmp( argall, "imotd" ) )
			{
				add_buf(output, "\tWKeywords:\tn ");
				add_buf(output,pHelp->keyword);
				add_buf(output,"\n\r");
			}

			if(!IS_NULLSTR(pHelp->topic))
			{
				add_buf(output, "\tWTopic:\tn \tM");
				add_buf(output, pHelp->topic);
				add_buf(output,"\tn\n\r");
			}

			if(!IS_NULLSTR(pHelp->quote))
			{
				add_buf(output, "Quote:\n\r");
				add_buf(output, pHelp->quote);
				add_buf(output,"\n\r");
			}

			if(!IS_NULLSTR(pHelp->syntax))
			{
				add_buf(output, "\tWSyntax:\tn \tC");
				add_buf(output, pHelp->syntax);
				add_buf(output,"\tn\n\r");
			}

			/*
			 * Strip leading '.' to allow initial blanks.
			 */
			if(!IS_NULLSTR(pHelp->description))
			{
				add_buf(output, "\tWDescription:\tn\n\r ");
				if ( pHelp->description[0] == '.' )
					add_buf(output,pHelp->description+1);
				else
					add_buf(output,pHelp->description);
			}

			if(!IS_NULLSTR(pHelp->website))
			{
				add_buf(output, "\tWWebsite:\tn ");
				add_buf(output, pHelp->website);
				add_buf(output,"\n\r");
			}

			if(!IS_NULLSTR(pHelp->see_also))
			{
				add_buf(output, "\tWSee Also:\tn \tC");
				add_buf(output, pHelp->see_also);
				add_buf(output,"\tn\n\r");
			}

			/*
			 * Strip leading '.' to allow initial blanks.
			 */
			if(!IS_NULLSTR(pHelp->unformatted))
			{
				if ( pHelp->unformatted[0] == '.' )
					add_buf(output,pHelp->unformatted+1);
				else
					add_buf(output,pHelp->unformatted);
			}

			found = TRUE;
			/* small hack :) */
			if (ch->desc != NULL && ch->desc->connected != CON_PLAYING)
				break;
		}
	}

	if (!found)
	{
		send_to_char( "\tWNo help on that word. Have you tried investigating or researching that?\tn\n\r", ch );
		log_string(LOG_BUG, (char *)Format("[*****] HELP: No help for: %s - %s", argall, ctime(&current_time)));
	}
	else
		page_to_char(buf_string(output),ch);
	free_buf(output);
}


/* whois command */
void do_whois (CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *wch;
	BUFFER *output;
	char arg[MAX_INPUT_LENGTH]={'\0'};
	char const *clan;
	int app = 0;
	bool online = FALSE;
	bool in_char_list = FALSE;
	bool fRPOK = FALSE;

	CheckCH(ch);

	one_argument(argument,arg);

	send_to_char("\n\r", ch);

	if (IS_NULLSTR(arg))
	{
		send_to_char("You must provide a name.\n\r",ch);
		return;
	}

	output = new_buf();

	if(is_online(arg)) online = TRUE;
	if(pc_in_char_list(arg)) in_char_list = TRUE;

	if((wch = get_player(arg)) == NULL)
	{
		send_to_char("No such player exists.\n\r", ch);
		return;
	}

	/*
	 * Figure out stuff to print.
	 */
	clan = clan_table[wch->clan].who_name;
	app = get_curr_stat(wch, STAT_APP);
	if(IS_SET(wch->plr_flags, PLR_RP_OK)) fRPOK = TRUE;

	/*
	 * Format it up.
	 */
	if(IS_ADMIN(ch))
	{
		send_to_char (Format("\tW######### [ \tC%12s %s\tn \tW] #########\tn\n\r", wch->name, IS_NULLSTR(wch->surname) ? "" : wch->surname), ch);
		send_to_char (Format("\tW%-10s\tn : \tG%s\tn\n\r", "Position", staff_status[wch->trust].name), ch);
		send_to_char (Format("\tW%-10s\tn : \tG%s\tn\n\r", "Gender", gender_string(wch)), ch);
		send_to_char (Format("\tW%-10s\tn : \tG%s\tn\n\r", "Appearance", appearance_string(wch)), ch);
		send_to_char (Format("Famous?    : %s\n\r", fame_table[wch->backgrounds[FAME_STATUS]].name), ch);
		send_to_char (Format("Available for RP : %s\n\r", wch->pcdata->rpok_string), ch);
		send_to_char("\tW######################################\tn\n\r",ch);
		send_to_char (Format("Race          : %s\n\r", wch->race < MAX_PC_RACE ? pc_race_table[wch->race].name:"     "), ch);
		send_to_char (Format("Clan/Tribe    : %s\n\r", capitalize(clan_table[wch->clan].name)), ch);
		send_to_char (Format("AFK: %s\n\r", IS_SET(wch->comm, COMM_AFK) ? "\tRA\tn": ""), ch);
		if (wch->desc && wch->desc->pProtocol && wch->desc->pProtocol->pVariables[eMSDP_UTF_8]->ValueInt == 1)
			send_to_char (Format("UTF-8 Enabled : Yes\n\r"), ch);
		else
			send_to_char (Format("UTF-8 Enabled : No\n\r"), ch);
	}
	else
	{
		send_to_char (Format("\tW##### [ \tC%s%s%s\tn \tW] #####\tn\n\r", wch->name, IS_NULLSTR(wch->surname) ? "" : wch->surname), ch);
		send_to_char (Format("\tW%-10s\tn : \tG%s\tn\n\r", "Position", staff_status[wch->trust].name), ch);
		send_to_char (Format("\tW%-10s\tn : \tG%s\tn\n\r", "Gender", gender_string(wch)), ch);
		send_to_char (Format("\tW%-10s\tn : \tG%s\tn\n\r", "Appearance", appearance_string(wch)), ch);
		send_to_char (Format("Available for RP : %s\n\r", wch->pcdata->rpok_string), ch);
		send_to_char("\tW######################################\tn\n\r",ch);
	}

	if(!IS_NULLSTR(wch->married))
		act("$N is married to $t.", ch, wch->married, wch, TO_CHAR, 1);

	if(!online && !in_char_list) free_char(wch);

	do_profession(ch, arg);

	do_laston(ch, arg);
}

void do_profession(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *wch;
	int fail = 0;
	bool online = FALSE;
	bool in_char_list = FALSE;

	CheckCH(ch);

	if(IS_NULLSTR(argument))
	{
		send_to_char("Whose profession are you interested in?\n\r", ch);
		return;
	}

	if(is_online(argument)) online = TRUE;
	if(pc_in_char_list(argument)) in_char_list = TRUE;

	if((wch = get_player(argument)) == NULL)
	{
		send_to_char("That player does not exist.\n\r", ch);
		return;
	}

	if((fail = dice_rolls(ch, get_curr_stat(ch, STAT_INT) + ch->ability[POLITICS].value,
			UMAX(8 - ch->backgrounds[CONTACTS] - ch->backgrounds[ALLIES], 3))) < 0)
	{
		fail = number_range(1, 5);
		switch(fail)
		{
		case 1:
		case 2:
			act("$N holds the position of Maker.", ch,NULL,wch,TO_CHAR,1);
			break;
		case 3:
			act("$N holds the position of Musician.", ch, NULL, wch, TO_CHAR, 1);
			break;
		case 4:
			act("$N holds the position of Laird Toadstool Hopper.", ch, NULL, wch, TO_CHAR, 1);
			break;
		case 5:
			act("$N might be out to get you... you just never know.", ch, NULL, wch, TO_CHAR, 1);
			break;
		}
		return;
	}
	else if(fail == 0 || fail == 1)
	{
		act("You can't figure out $N's profession.", ch, NULL, wch, TO_CHAR, 1);
	}
	else if(fail >=2)
	{
		if(!IS_NULLSTR(wch->profession) && str_cmp(wch->profession, "None"))
		{
			if(!str_cmp(wch->profession, "Mayor"))
				act("$N holds the position of $t.", ch, wch->sex==2?"Lady Mayoress":"Lord Mayor", wch, TO_CHAR, 1);
			else if(!str_cmp(wch->profession, "Alderman") || !str_cmp(wch->profession, "Judge"))
				act("$N holds the position of $t.\n\r", ch, wch->profession, wch, TO_CHAR, 1);
			else if(!str_cmp(wch->profession, "Chief"))
				act("$N holds the position of Police Chief.", ch, NULL, wch, TO_CHAR, 1);
			else
				act("$N has joined the $t profession.", ch, wch->profession, wch, TO_CHAR, 1);
		}
		else
		{
			act("You can't figure out $N's profession.", ch, NULL, wch, TO_CHAR, 1);
		}
	}

	if(!online && !in_char_list)
		free_char(wch);
}

/*
 * New 'who' command originally by Alander of Rivers of Mud.
 * Extended and customised by One Thousand Monkeys
 */
void do_who( CHAR_DATA *ch, char *argument )
{
	BUFFER *output;
	DESCRIPTOR_DATA *d;
	PACK_DATA *pack = NULL;
	char buf[MSL]={'\0'};
	int iRace = 0;
	int iClan = 0;
	int nNumber = 0;
	int nMatch = 0;
	int app = 0;
	bool rgfClan[MAX_CLAN];
	bool rgfRace[MAX_PC_RACE];
	bool fClanRestrict = FALSE;
	bool fClan = FALSE;
	bool fRaceRestrict = FALSE;
	bool fImmortalOnly = FALSE;
	bool fRPOK = FALSE;
	bool fPack = FALSE;
	
	CheckCH(ch);
 
	/*
	 * Set default arguments.
	 */
	for ( iRace = 0; iRace < MAX_PC_RACE; iRace++ )
		rgfRace[iRace] = FALSE;
	for (iClan = 0; iClan < MAX_CLAN; iClan++)
	rgfClan[iClan] = FALSE;

	send_to_char("\n\r", ch);
 
	send_to_char( "\t[F220]************************************************************\tn\n\r", ch);
	send_to_char( "\t[F220]***\tn  \tW       People currently on Project Twilight\tn         \t[F220]***\tn\n\r", ch );
	send_to_char( "\t[F220]************************************************************\tn\n\r", ch);

	/*
	 * Parse arguments.
	 */
	nNumber = 0;
	for ( ;; )
	{
		char arg[MAX_INPUT_LENGTH]={'\0'};

		argument = one_argument( argument, arg );
		if ( IS_NULLSTR(arg) )
			break;

		/*
		 * Look for clan to turn on.
		 */
		if (!str_prefix(arg,"admin") || !str_prefix(arg,"staff"))
		{
			fImmortalOnly = TRUE;
			send_to_char( "\t[F220]***\tn  \tW                     PT Staff                     \tn  \t[F220]***\tn\n\r", ch );
			send_to_char( "\t[F220]************************************************************\tn\n\r", ch);

		}
		else if (!str_prefix(arg, "available") || !str_prefix(arg, "rpok") || !str_prefix(arg, "rpmod"))
		{
			fRPOK = TRUE;
			send_to_char( "\t[F220]***\tn  \tW                  RP Available                    \tn  \t[F220]***\tn\n\r", ch );
			send_to_char( "\t[F220]************************************************************\tn\n\r", ch);

		}
		else if (!str_prefix(arg, "pack"))
		{
			fPack = TRUE;
			if(!IS_ADMIN(ch) || IS_NULLSTR(argument))
				pack = pack_lookup(ch->pack);
			else
				pack = pack_lookup(argument);

			if(pack == NULL)
			{
				if(!IS_ADMIN(ch) || IS_NULLSTR(argument))
					send_to_char("You aren't part of a pack.\n\r", ch);
				else
					send_to_char("No such pack.\n\r", ch);
			}
		}
		else if(IS_ADMIN(ch))
		{
			iRace = race_lookup(arg);

			if (iRace == 0 || iRace >= MAX_PC_RACE)
			{
				if (!str_prefix(arg,"clan"))
					fClan = TRUE;
				else
				{
					iClan = clan_lookup(arg);
					if (iClan)
					{
						fClanRestrict = TRUE;
						rgfClan[iClan] = TRUE;
					}
					else
					{
						send_to_char("That's not a valid race, kith, tribe or clan.\n\r", ch);
						return;
					}
				}
			}
			else
			{
				fRaceRestrict = TRUE;
				rgfRace[iRace] = TRUE;
			}
		}
	}

	/*
	 * Add a header.
	 */
	if(fPack && pack != NULL)
	{
		send_to_char(Format("The %s Pack\n\r", pack->name), ch);
	}

	/*
	 * Now show matching chars.
	 */
	nMatch = 0;
	buf[0] = '\0';
	output = new_buf();
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
		CHAR_DATA *wch;
		char const *clan;
 
		/*
		 * Check for match against restrictions.
		 */
		if ( d->connected != CON_PLAYING || !can_see_main( ch, d->character ) )
			continue;
 
		wch   = ( d->original != NULL ) ? d->original : d->character;

	if (!can_see_main(ch,wch))
		continue;

		if ( ( fImmortalOnly  && !IS_ADMIN(wch) )
		|| ( fRaceRestrict && !rgfRace[wch->race])
	|| ( fClan && !is_clan(wch))
	|| ( fClanRestrict && !rgfClan[wch->clan])
	|| ( fRPOK && !IS_SET(wch->plr_flags, PLR_RP_OK)))
			continue;

	if ( fPack && pack != pack_lookup(wch->pack) )
			continue;
 
		nMatch++;
 
		/*
		 * Figure out stuff to print.
	 */
	clan = clan_table[wch->clan].who_name;
	app = get_curr_stat(wch, STAT_APP);
	send_to_char("\n\r", ch);

	/*
	 * Format it up.
	 */
	if(IS_SET(ch->comm, COMM_BRIEF))
	{
		if(IS_ADMIN(ch))
				{
				snprintf( buf, sizeof(buf), "[\tY%4s %s\tn][\tY%6s] %s%s%s\tW%s\tn\n\r",
				wch->race < MAX_PC_RACE ? pc_race_table[wch->race].who_name
							: "     ",
				clan,
				gender_string(wch),
				wch->invis_level >= LEVEL_IMMORTAL ? "[\tYWIZI\tn]" : "",
				IS_SET(wch->act2, ACT2_STORY) ? "[\t[F205]ST\tn]": "",
				IS_SET(wch->comm, COMM_AFK) ? "[\tYAFK\tn]": "",
				wch->name);
			}
		else
			{
				snprintf( buf, sizeof(buf), "[%s] %s%s\tW%s\tn\n\r",
				gender_string(wch),
				IS_SET(wch->act2, ACT2_STORY) ? "[\t[F205]ST\tn]": "",
				IS_SET(wch->comm, COMM_AFK) ? "[\tYAFK\tn]": "",
				wch->name );
			}
	}
	else
	{
	if(IS_ADMIN(ch))
	  snprintf( buf, sizeof(buf), "[\tY%4s %s\tn][\tY%s %6s\tn]%s%s%s \tW%s %s\tn %s%s%s\tn\n\r",
		wch->race < MAX_PC_RACE ? pc_race_table[wch->race].who_name 
					: "     ",
		clan,
		appearance_string(wch),
		gender_string(wch),
		wch->invis_level >= LEVEL_IMMORTAL ? "[\tYWIZI\tn]" : "",
		IS_SET(wch->act2, ACT2_STORY) ? "[\t[F205]ST\tn]": "",
		IS_SET(wch->comm, COMM_AFK) ? "[\tYAFK\tn]": "",
		wch->name,
		IS_NULLSTR(wch->surname) ? "" : wch->surname,
		fRPOK ? " [": "",
		IS_NPC(wch) ? "" : !fRPOK ? wch->pcdata->title : wch->pcdata->rpok_string,
		fRPOK ? "]": "");
	else
	  snprintf( buf, sizeof(buf), "[%s]%s%s \tW%s %s\tn\tR%s\tn%s\tR%s\tn\n\r",
		gender_string(wch),
		IS_SET(wch->act2, ACT2_STORY) ? "[\t[F205]ST\tn]": "",
		IS_SET(wch->comm, COMM_AFK) ? "[\tYAFK\tn]": "",
		wch->name,
		IS_NULLSTR(wch->surname) ? "" : wch->surname,
		fRPOK ? " [": "",
		IS_NPC(wch) ? "" : !fRPOK ? wch->pcdata->title : wch->pcdata->rpok_string,
		fRPOK ? "]": "");
	}
	add_buf(output,buf);
	}

	add_buf(output, (char *)Format("\n\rPlayers found: %d\n\r", nMatch));
	page_to_char( buf_string(output), ch );
	free_buf(output);
	return;
}

void do_count ( CHAR_DATA *ch, char *argument )
{
	DESCRIPTOR_DATA *d;
	int count = 0;

	CheckCH(ch);

	for ( d = descriptor_list; d != NULL; d = d->next )
		if ( d->connected == CON_PLAYING && can_see_main( ch, d->character ) )
			count++;

	max_on = UMAX(count,max_on);

	if (max_on == count)
	{
		send_to_char(Format("There are %d characters on, the most so far today.\n\r",count), ch);
	}
	else
	{
		send_to_char(Format("There are %d characters on, the most on today was %d.\n\r",count,max_on), ch);
	}
}

void do_inventory( CHAR_DATA *ch, char *argument )
{
	CheckCH(ch);
	send_to_char( "You are carrying:\n\r", ch );
	show_list_to_char( ch->carrying, ch, TRUE, TRUE );
	return;
}

void do_equipment( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj;
	int iWear = 0;
	bool found = FALSE;

	CheckCH(ch);

	send_to_char( "You are using:\n\r", ch );
	for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
	{
		if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
			continue;

		send_to_char( Format("< %-19s > ", wear_loc_strings[iWear].name), ch );
		if ( iWear == WEAR_SKINFLAP )
		{
			for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
			{
				if ( obj->wear_loc == iWear )
				{
					if(found == TRUE) send_to_char( ", ", ch );
					send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
					found = TRUE;
				}
			}
			send_to_char( "\n\r", ch );
		}
		else if ( can_see_obj( ch, obj ) )
		{
			send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
			send_to_char( "\n\r", ch );
		}
		else
		{
			send_to_char( "something.\n\r", ch );
		}
		found = TRUE;
	}

	if ( !found )
		send_to_char( "Nothing.\n\r", ch );

	return;
}

void do_credits( CHAR_DATA *ch, char *argument )
{
	CheckCH(ch);
	do_function(ch, &do_help, "credits");
	return;
}


void do_where( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH]={'\0'};
	CHAR_DATA *victim;
	AREA_DATA *pArea;
	DESCRIPTOR_DATA *d;
	bool found;

	CheckCH(ch);

	one_argument( argument, arg );

	if ( IS_NULLSTR(arg) )
	{
		pArea = ch->in_room->area;
		send_to_char(Format("You are in: %s\n\r", pArea->name), ch);

		send_to_char( "Players near you:\n\r", ch );
		found = FALSE;
		for ( d = descriptor_list; d; d = d->next )
		{
			if ( d->connected == CON_PLAYING
					&& ( victim = d->character ) != NULL
					&&   !IS_NPC(victim)
					&&   victim->in_room != NULL
					&&   !IS_SET(victim->in_room->room_flags,ROOM_NOWHERE)
					&&   (is_room_owner(ch,victim->in_room)
							||    !room_is_private(victim->in_room))
							&&   victim->in_room->area == ch->in_room->area
							&&   can_see( ch, victim ) )
			{
				found = TRUE;
				send_to_char( Format("%-28s %s\n\r", victim->name, victim->in_room->name), ch );
			}
		}
		if ( !found )
			send_to_char( "None\n\r", ch );
	}
	else
	{
		found = FALSE;
		for ( victim = char_list; victim != NULL; victim = victim->next )
		{
			if ( victim->in_room != NULL
					&&   victim->in_room->area == ch->in_room->area
					&&   !IS_AFFECTED(victim, AFF_HIDE)
					&&   !IS_AFFECTED(victim, AFF_SNEAK)
					&&   can_see( ch, victim )
					&&   is_name( arg, victim->name ) )
			{
				found = TRUE;
				send_to_char( Format("%-28s %s\n\r", PERS(victim, ch), victim->in_room->name), ch );
				break;
			}
		}
		if ( !found )
			act( "You didn't find any $T.", ch, NULL, arg, TO_CHAR, 1 );
	}

	return;
}


void do_consider( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH]={'\0'};
	char *msg;
	int diff = 0;

	CheckCH(ch);

	one_argument( argument, arg );

	if ( IS_NULLSTR(arg) )
	{
		send_to_char( "Consider killing whom?\n\r", ch );
		return;
	}

	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
		send_to_char( "They're not here.\n\r", ch );
		return;
	}

	diff = victim->health;

	if ( diff  ==   0 ) msg = "$N is incapacitated.";
	else if ( diff  ==   1 ) msg = "$N is having trouble moving.";
	else if ( diff  ==   2 ) msg = "$N is hurt badly.";
	else if ( diff  ==   3 ) msg = "$N is hurt.";
	else if ( diff  ==   4 ) msg = "$N is hurt.";
	else if ( diff  ==   5 ) msg = "$N has been weakened a little.";
	else if ( diff  ==   6 ) msg = "$N has been weakened a little.";
	else                    msg = "$N is at full health.";

	act( msg, ch, NULL, victim, TO_CHAR, 1 );

	if(get_curr_stat(victim, STAT_STR) >= get_curr_stat(victim, STAT_DEX)
			&& get_curr_stat(victim, STAT_STR) >= get_curr_stat(victim, STAT_STA))
	{
		if(get_curr_stat(ch, STAT_STR) > get_curr_stat(victim, STAT_STR))
			msg = "$N seems weaker than you.";
		else if(get_curr_stat(ch, STAT_STR) < get_curr_stat(victim, STAT_STR))
			msg = "$N seems stronger than you.";
		else if(get_curr_stat(ch, STAT_STR) == get_curr_stat(victim, STAT_STR))
			msg = "$N seems about as strong as you.";
	}

	act( msg, ch, NULL, victim, TO_CHAR, 1 );

	if(get_curr_stat(victim, STAT_DEX) >= get_curr_stat(victim, STAT_STA)
			&& get_curr_stat(victim, STAT_DEX) >= get_curr_stat(victim, STAT_STR))
	{
		if(get_curr_stat(ch, STAT_DEX) > get_curr_stat(victim, STAT_DEX))
			msg = "$N seems slower than you.";
		else if(get_curr_stat(ch, STAT_DEX) < get_curr_stat(victim, STAT_DEX))
			msg = "$N seems more deft than you.";
		else if(get_curr_stat(ch, STAT_DEX) == get_curr_stat(victim, STAT_DEX))
			msg = "$N seems about as dexterous as you.";
	}

	act( msg, ch, NULL, victim, TO_CHAR, 1 );

	if(get_curr_stat(victim, STAT_STA) >= get_curr_stat(victim, STAT_DEX)
			&& get_curr_stat(victim, STAT_STA) >= get_curr_stat(victim, STAT_DEX))
	{
		if(get_curr_stat(ch, STAT_STA) > get_curr_stat(victim, STAT_STA))
			msg = "$N doesn't seem as tough as you.";
		else if(get_curr_stat(ch, STAT_STA) < get_curr_stat(victim, STAT_STA))
			msg = "$N seems tougher than you.";
		else if(get_curr_stat(ch, STAT_STA) == get_curr_stat(victim, STAT_STA))
			msg = "$N seems about as tough as you.";
	}

	act( msg, ch, NULL, victim, TO_CHAR, 1 );

	if(get_points(ch) > get_points(victim))
		msg = "$N seems less impressive than you all up.";
	else if(get_points(ch) < get_points(victim))
		msg = "$N seems more impressive than you all up.";
	else if(get_points(ch) == get_points(victim))
		msg = "$N seems to have developed to the same extent as you.";

	act( msg, ch, NULL, victim, TO_CHAR, 1 );

	return;
}



void set_title( CHAR_DATA *ch, char *title )
{
	char buf[MSL]={'\0'};

	CheckChNPC(ch);

	if ( title[0] != '.' && title[0] != ',' && title[0] != '!' && title[0] != '?' )
	{
		buf[0] = ' ';
		strncpy( buf+1, title, 45 );
	}
	else
	{
		strncpy( buf, title, 45 );
	}

	PURGE_DATA( ch->pcdata->title );
	ch->pcdata->title = str_dup( buf );
	return;
}



void do_title( CHAR_DATA *ch, char *argument )
{
	CheckCH(ch);
	CheckChNPC(ch);

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Change your title to what?\n\r", ch );
		return;
	}

	if ( strlen(argument) > 45 ) {
//      argument[45] = '\0';
		send_to_char("Title cannot be longer than 45 characters long.\n\r",ch);
		return;
	}

	smash_tilde( argument );
	set_title( ch, argument );
	act( "Title set to: $t\n\r", ch, ch->pcdata->title, NULL, TO_CHAR, 1 );
}



void do_description( CHAR_DATA *ch, char *argument )
{
	CheckCH(ch);

	if ( IS_NULLSTR(argument) && !LOOKS_DIFFERENT(ch) )
	{
		string_append( ch, &ch->description );
		return;
	}
	else if ( IS_NULLSTR(argument) && LOOKS_DIFFERENT(ch) )
	{
		string_append( ch, &ch->alt_description );
		return;
	}

	send_to_char( "Syntax:  desc    - line edit\n\r", ch );
	return;
}


void do_switchdesc(CHAR_DATA *ch, char *argument)
{
	char *temp;

	CheckCH(ch);

	temp = str_dup(ch->description);
	ch->description = str_dup(ch->switch_desc);
	ch->switch_desc = str_dup(temp);

	send_to_char("Descriptions switched.\n\r", ch);
}


void do_password( CHAR_DATA *ch, char *argument )
{
	char arg1[MAX_INPUT_LENGTH]={'\0'};
	char arg2[MAX_INPUT_LENGTH]={'\0'};
	char *pwdnew;
	char *p;

	CheckCH(ch);
	CheckChNPC(ch);

	/*
	 * Can't use one_argument because it smashes case.
	 * Since we're needing this in a few places there's exact_argument.
	 */
	argument = exact_argument(argument, arg1);
	argument = exact_argument(argument, arg2);

	if ( IS_NULLSTR(arg1) || IS_NULLSTR(arg2) )
	{
		send_to_char( "Syntax: password [old] [new].\n\r", ch );
		return;
	}

	if ( str_cmp( crypt( arg1, ch->pcdata->pwd ), ch->pcdata->pwd ) )
	{
		WAIT_STATE( ch, 40 );
		send_to_char( "Wrong password.  Wait 10 seconds.\n\r", ch );
		return;
	}

	if ( strlen(arg2) < 5 )
	{
		send_to_char( "New password must be at least five characters long.\n\r", ch );
		return;
	}

	/*
	 * No tilde allowed because of player file format.
	 */
	pwdnew = crypt( arg2, ch->name );
	for ( p = pwdnew; *p != '\0'; p++ )
	{
		if ( *p == '~' )
		{
			send_to_char( "New password not acceptable, try again.\n\r", ch );
			log_string( LOG_SECURITY, Format("%s tried to change their password unsuccessfully.", ch->name));
			return;
		}
	}

	PURGE_DATA( ch->pcdata->pwd );
	ch->pcdata->pwd = str_dup( pwdnew );
	save_char_obj( ch );
	log_string( LOG_SECURITY, Format("%s changed their password.", ch->name) );
	send_to_char( "Ok.\n\r", ch );
	return;
}

void do_research(CHAR_DATA *ch, char *argument)
{
	NOTE_DATA *pbg;
	BUFFER *output;
	int diff = 0, fail = 0;
	bool found = FALSE;

	CheckCH(ch);

	if(IS_NULLSTR(argument))
	{
		send_to_char("Research what?\n\r", ch);
		return;
	}
	else
		pbg = find_knowledge_keyword(argument);

	if(pbg == NULL)
	{
		send_to_char(Format("Your research finds nothing about %s.", argument), ch);
		log_string(LOG_BUG, (char *)Format("BG MISSING | No BG for: %s - %s", argument, ctime(&current_time)));
		return;
	}

	diff = atoi(pbg->subject);
	fail = dice_rolls(ch, get_curr_stat(ch, STAT_INT) + ch->ability[ACADEMICS].value, diff);

	output = new_buf();
	for(pbg = know_list; pbg != NULL; pbg = pbg->next)
	{
		if(is_name(argument, pbg->to_list) && (fail >= pbg->successes || IS_ADMIN(ch)))
		{
			found = TRUE;
			if(IS_ADMIN(ch))
			{
				add_buf( output, (char *)Format("\tGKeywords:\tn %s\n\r", pbg->to_list) );
			}
			add_buf( output, pbg->text );
		}
	}

	if(!found)
	{
		send_to_char(Format("Your research finds nothing about %s.", argument), ch);
	}
	else
		page_to_char(buf_string(output), ch);
	free_buf(output);
}

void do_investigate(CHAR_DATA *ch, char *argument)
{
	NOTE_DATA *pbg;
	CHAR_DATA *vch = NULL;
	BUFFER *output;
	char buf[MSL]={'\0'};
	int diff = 0, fail = 0;
	bool IsChar = FALSE;
	bool found = FALSE;
	
	CheckCH(ch);

	if(IS_NULLSTR(argument))
	{
		send_to_char("Investigate what?\n\r", ch);
		return;
	}

	if(!str_cmp(argument, "here"))
	{
		snprintf(buf, sizeof(buf), "%d", ch->in_room->vnum);
		pbg = find_bg_keyword(buf);
	}
	else
		pbg = find_bg_keyword(argument);

	if(pbg == NULL && (vch = get_char_world(ch, argument)) == NULL)
	{
		send_to_char(Format("You can find out nothing about %s.", argument), ch);
		log_string(LOG_BUG, (char *)Format("FACT MISSING | No facts for: %s", argument));
		return;
	}

	if(pbg == NULL && vch != NULL)
		IsChar = TRUE;

	if(pbg != NULL)
		diff = UMAX(atoi(pbg->subject) - ch->backgrounds[CONTACTS] - ch->backgrounds[ALLIES], 3);
	else
		diff = UMAX(5 + vch->ability[SUBTERFUGE].value - ch->backgrounds[CONTACTS] - ch->backgrounds[ALLIES], 3);

	fail = dice_rolls(ch, get_curr_stat(ch, STAT_INT) + ch->ability[INVESTIGATION].value, diff);


	if(!IsChar)
	{
		output = new_buf();
		for(pbg = bg_list; pbg != NULL; pbg = pbg->next)
		{
			if(is_name(argument, pbg->to_list)
					&& (fail >= pbg->successes || IS_ADMIN(ch)))
			{
				found = TRUE;
				if(IS_ADMIN(ch))
				{
					add_buf( output, (char *)Format("\tGKeywords:\tn %s\n\r", pbg->to_list) );
				}
				add_buf( output, pbg->text );
			}
		}

		if(!found)
		{
			send_to_char(Format("You find out nothing about %s.", argument), ch);
		}
		else
			page_to_char(buf_string(output), ch);
		free_buf(output);
	}
	else if(IsChar && vch != NULL && fail >= 0)
	{
		send_to_char("You find a photograph, it shows:\n\r", ch);
		send_to_char(vch->description, ch);
		return;
	}
	else
	{
		send_to_char(Format("You can find out nothing about %s.", argument), ch);
	}
}

void do_socials(CHAR_DATA *ch, char *argument)
{
	int iSocial = 0;
	int col = 0;

	CheckCH(ch);

	for (iSocial = 0; !IS_NULLSTR(social_table[iSocial].name); iSocial++)
	{
		send_to_char(Format("%-12s ",social_table[iSocial].name), ch);
		if (++col % 4 == 0)
			send_to_char("\n\r",ch);
	}

	if ( col % 4 != 0)
		send_to_char("\n\r",ch);
	return;
}

void do_abilities(CHAR_DATA *ch, char *argument)
{
	int sn = 0, count = 0;

	CheckCH(ch);

	if (IS_NPC(ch))
		return;

	send_to_char("\n\r\tW---------------------------------<\tGAbilities\tW>----------------------------------\tn\n\r", ch);

	for (sn = 0; sn < MAX_ABIL; sn++)
	{
		if (ability_table[sn].name == NULL )
			break;

		if(IS_ATTRIB_AVAILABLE(ch->race, sn))
		{
			send_to_char(Format("\t<send href='help %s'>%-15s\t</send> %-2d  | ", capitalize(ability_table[sn].name), capitalize(ability_table[sn].name), ch->ability[sn].value), ch);
			count++;if(!(count%3)) send_to_char("\n\r", ch);
		}
	}
	send_to_char("\n\r", ch);
}

void do_powers(CHAR_DATA *ch, char *argument)
{
	int sn = 0;
	char name[MSL]={'\0'};

	CheckCH(ch);

	if (IS_NPC(ch))
	  return;

	send_to_char("\n\rPowers:\n\r", ch);

	if (IS_VAMPIRE(ch))
	{
		if(IS_ADMIN(ch))
		{
		for(sn = 0; disc_table[sn].vname != NULL; sn++)
		{
			if(sn != DISC_OBEAH) 
			{
				snprintf(name, sizeof(name), "%s", disc_table[sn].vname);
				send_to_char(Format("\t<send href='help %s'>%-15s\t</send> %-2d ", capitalize(name),capitalize(name), ch->disc[sn]), ch);
				send_to_char("\n\r", ch);
			}
		}
		send_to_char("\n\r", ch);
		}
		else
		for(sn = 0; disc_table[sn].vname != NULL; sn++)
		{
			if(sn != DISC_OBEAH) 
			{
				if(ch->disc[sn] != 0)
				{
				snprintf(name, sizeof(name), "%s", disc_table[sn].vname);
				send_to_char(Format("\t<send href='help %s'>%-15s\t</send> %-2d ", capitalize(name),capitalize(name), ch->disc[sn]), ch);
				send_to_char("\n\r", ch);
				}	
			}
		}
		send_to_char("\n\r", ch);
	}
	else if (ch->race == race_lookup("werewolf"))
	{
		for(sn = 0; disc_table[sn].wname != NULL; sn++)
		{
			snprintf(name, sizeof(name), disc_table[sn].wname);
			send_to_char(Format("\t<send help='help %s'>%-15s\t</send> %-2d ", capitalize(name), capitalize(name), ch->disc[sn]), ch);
			send_to_char("\n\r", ch);
		}
	}
	else if (ch->race == race_lookup("faerie"))
	{
		for(sn = 0; disc_table[sn].fname != NULL; sn++)
		{
			snprintf(name, sizeof(name), disc_table[sn].fname);
			send_to_char(Format("%-15s %-2d ", capitalize(name), ch->disc[sn]), ch);
		}
		send_to_char("\n\r", ch);
	}
	else if (ch->race == race_lookup("human"))
	{
		sn = clan_table[ch->clan].powers[0];
		if(sn == -1)
		{
			send_to_char("None.\n\r", ch);
			return;
		}
		snprintf(name, sizeof(name), disc_table[sn].hname);
		send_to_char(Format("\t<send help='help %s'>%-15s\t</send> %-2d ", capitalize(name), capitalize(name), ch->disc[sn]), ch);
		send_to_char("\n\r", ch);
	}
}

void do_power_list(CHAR_DATA *ch, char *argument)
{
	int i = 0;

	CheckCH(ch);

	switch(ch->race)
	{
	case RACE_HUMAN:
		send_to_char("No powers are available for humans at the moment.\n\r", ch);
		break;

	case RACE_WEREWOLF:
	{
		for( i = 1; i <= 5/*ch->backgrounds[RACE_STATUS]*/; i++)
		{
			if(i == 1) {
				send_to_char("Your Gifts:\n\r", ch);
			}
			send_to_char(Format("Rank %d Gifts:\n\r", i), ch);
			send_to_char(power_string(gift_table,ch->powers[i-1],i),ch);
			send_to_char("\n\r", ch);
		}
		/*      if(ch->clan == clan_lookup("bone gnawers"))
		{
			if(ch->disc[DISC_TRIBE] > 0)
			{
			send_to_char("Tribe Powers:\n\r", ch);
			if(ch->disc[DISC_TRIBE] >= 1)
			send_to_char("COOKING: Making the inedible edible.\n\r", ch);
			if(ch->disc[DISC_TRIBE] >= 2)
			send_to_char("ODOUR: Defensive scents.\n\r", ch);
			if(ch->disc[DISC_TRIBE] >= 3)
			send_to_char("TERMITE: Eating wooden objects.\n\r", ch);
			if(ch->disc[DISC_TRIBE] >= 4)
			send_to_char("ATTUNE: Sensing those in the area.\n\r", ch);
			if(ch->disc[DISC_TRIBE] >= 5)
			send_to_char("SURVIVOR: Surviving death.\n\r", ch);
			}
		}
		if(ch->clan == clan_lookup("silver fangs"))
		{
			if(ch->disc[DISC_TRIBE] > 0)
			{
			send_to_char("Tribe Powers:\n\r", ch);
			if(ch->disc[DISC_TRIBE] >= 1)
			send_to_char("SENSEWYRM: Detecting Wyrmtaint.\n\r", ch);
			if(ch->disc[DISC_TRIBE] >= 2)
			send_to_char("MOONSHIELD: Armor of Luna.\n\r", ch);
			if(ch->disc[DISC_TRIBE] >= 3)
			send_to_char("SILVERCLAWS: Turn your claws to gleaming silver.\n\r", ch);
			if(ch->disc[DISC_TRIBE] >= 4)
			send_to_char("WRATH: Unleash the wrath of Gaia on the Wyrm's minions.\n\r", ch);
			if(ch->disc[DISC_TRIBE] >= 5)
			send_to_char("LUNA'S AVENGER: Become a giant silver werewolf.\n\r", ch);
			}
		}
		if(ch->clan == clan_lookup("glass walkers"))
		{
			if(ch->disc[DISC_TRIBE] > 0)
			{
			send_to_char("Tribe Powers:\n\r", ch);
			if(ch->disc[DISC_TRIBE] >= 1)
			send_to_char("Cybersenses: Expand your senses to incredile proportions. (HEIGHTEN)\n\r", ch);
			if(ch->disc[DISC_TRIBE] >= 2)
			send_to_char("Control Tech: SIPHON.\n\r", ch);
			if(ch->disc[DISC_TRIBE] >= 3)
			send_to_char("HEATMETAL: Turn up the heat on an opponent's metal equipment.\n\r", ch);
			if(ch->disc[DISC_TRIBE] >= 4)
			send_to_char("DOPPLEGANGER: Take on another face.\n\r", ch);
			if(ch->disc[DISC_TRIBE] >= 5)
			send_to_char("Phone Travel: Travelling from phone to phone. (PTRAVEL)\n\r", ch);
			}
		} */
		/*      if(ch->clan == clan_lookup(""))
		{
			if(ch->disc[DISC_TRIBE] > 0)
			{
			send_to_char("Tribe Powers:\n\r", ch);
			if(ch->disc[DISC_TRIBE] >= 1)
			send_to_char("\n\r", ch);
			if(ch->disc[DISC_TRIBE] >= 2)
			send_to_char("\n\r", ch);
			if(ch->disc[DISC_TRIBE] >= 3)
			send_to_char("\n\r", ch);
			if(ch->disc[DISC_TRIBE] >= 4)
			send_to_char("\n\r", ch);
			if(ch->disc[DISC_TRIBE] >= 5)
			send_to_char("\n\r", ch);
			}
		} */
		/*
		if(ch->breed == breed_lookup("homid"))
		{
			if(ch->disc[DISC_BREED] > 0)
			{
			send_to_char("Breed Powers:\n\r", ch);
			if(ch->disc[DISC_BREED] >= 1)
			send_to_char("MANSMELL: Scare animals with the scent of humanity.\n\r", ch);
			if(ch->disc[DISC_BREED] >= 2)
			send_to_char("PERSUASION: Improve your ability to convince.\n\r", ch);
			if(ch->disc[DISC_BREED] >= 3)
			send_to_char("DISQUIET: Cause target to suffer depression.\n\r", ch);
			if(ch->disc[DISC_BREED] >= 4)
			send_to_char("TONGUES: Not yet implemented.\n\r", ch);
			if(ch->disc[DISC_BREED] >= 5)
			send_to_char("No power yet implemented.\n\r", ch);
			}
		}
		if(ch->breed == breed_lookup("metis"))
		{
			if(ch->disc[DISC_BREED] > 0)
			{
			send_to_char("Breed Powers:\n\r", ch);
			if(ch->disc[DISC_BREED] >= 1)
			send_to_char("SENSEWYRM: Sense the presence of wyrm taint.\n\r", ch);
			if(ch->disc[DISC_BREED] >= 2)
			send_to_char("SHED: Move faster at the expense of your stamina.\n\r", ch);
			if(ch->disc[DISC_BREED] >= 3)
			send_to_char("MINDSPEAK: Telepathic messages.\n\r", ch);
			if(ch->disc[DISC_BREED] >= 4)
			send_to_char("GROVEL: Ingratiating yourself to stop a fight.\n\r", ch);
			if(ch->disc[DISC_BREED] >= 5)
			send_to_char("No power yet implemented.\n\r", ch);
			}
		}
		if(ch->breed == breed_lookup("lupus"))
		{
			if(ch->disc[DISC_BREED] > 0)
			{
			send_to_char("Breed Powers:\n\r", ch);
			if(ch->disc[DISC_BREED] >= 1)
			send_to_char("HEIGHTEN: Expanding your senses to incredible proportions.\n\r", ch);
			if(ch->disc[DISC_BREED] >= 2)
			send_to_char("TRUESCENT: Scent of the True Form.\n\r", ch);
			if(ch->disc[DISC_BREED] >= 3)
			send_to_char("BECKON: Summoning animals.\n\r", ch);
			if(ch->disc[DISC_BREED] >= 4)
			send_to_char("GNAW: Not yet implemented.\n\r", ch);
			if(ch->disc[DISC_BREED] >= 5)
			send_to_char("SUMMON ELEMENTAL: Not yet implemented.\n\r", ch);
			}
		}
		if(ch->auspice == auspice_lookup("ragabash"))
		{
			if(ch->disc[DISC_AUSPICE] > 0)
			{
			send_to_char("Auspice Powers:\n\r", ch);
			if(ch->disc[DISC_AUSPICE] >= 1)
			send_to_char("SCENT OF RUNNING WATER: Makes the Garou untrackable.\n\r", ch);
			if(ch->disc[DISC_AUSPICE] >= 2)
			send_to_char("BLUR: Hide in the shadows.\n\r", ch);
			if(ch->disc[DISC_AUSPICE] >= 3)
			send_to_char("VANISH: Become invisible.\n\r", ch);
			if(ch->disc[DISC_AUSPICE] >= 4)
			send_to_char("FILCH: Taking the Forgotten.\n\r", ch);
			if(ch->disc[DISC_AUSPICE] >= 5)
			send_to_char("LUNA'S BLESSING: No aggravated damage from silver.\n\r", ch);
			}
		}
		if(ch->auspice == auspice_lookup("theurge"))
		{
			if(ch->disc[DISC_AUSPICE] > 0)
			{
			send_to_char("Auspice Powers:\n\r", ch);
			if(ch->disc[DISC_AUSPICE] >= 1)
			send_to_char("SENSEWYRM: Sensing Wyrm taint.\n\r", ch);
			if(ch->disc[DISC_AUSPICE] >= 2)
			send_to_char("MOTHER'S TOUCH: Heal one wound.\n\r", ch);
			if(ch->disc[DISC_AUSPICE] >= 3)
			send_to_char("COMPEL: Command spirits.\n\r", ch);
			if(ch->disc[DISC_AUSPICE] >= 4)
			send_to_char("EXORCISM: Drive away spirits.\n\r", ch);
			if(ch->disc[DISC_AUSPICE] >= 5)
			send_to_char("SPIRIT DRAIN: Gain Gnosis by injuring a spirit.\n\r", ch);
			}
		}
		if(ch->auspice == auspice_lookup("philodox"))
		{
			if(ch->disc[DISC_AUSPICE] > 0)
			{
			send_to_char("Auspice Powers:\n\r", ch);
			if(ch->disc[DISC_AUSPICE] >= 1)
			send_to_char("RESIST PAIN: Pain will not slow you down.\n\r", ch);
			if(ch->disc[DISC_AUSPICE] >= 2)
			send_to_char("BECKON: Summon animals.\n\r", ch);
			if(ch->disc[DISC_AUSPICE] >= 3)
			send_to_char("WEAKARM: Weaken an opponent.\n\r", ch);
			if(ch->disc[DISC_AUSPICE] >= 4)
			send_to_char("SNIFFUMBRA: Scry a location in the umbra.\n\r", ch);
			if(ch->disc[DISC_AUSPICE] >= 5)
			send_to_char("TRUEFORM: Force a shapeshifter into their true form.\n\r", ch);
			}
		}
		if(ch->auspice == auspice_lookup("galliard"))
		{
			if(ch->disc[DISC_AUSPICE] > 0)
			{
			send_to_char("Auspice Powers:\n\r", ch);
			if(ch->disc[DISC_AUSPICE] >= 1)
			send_to_char("BEAST SPEECH: Understand animal speech.\n\r", ch);
			if(ch->disc[DISC_AUSPICE] >= 2)
			send_to_char("WYLDCALL: Howl to all Garou.\n\r", ch);
			if(ch->disc[DISC_AUSPICE] >= 3)
			send_to_char("DREAMSPEAK: Speak into your target's dreams.\n\r", ch);
			if(ch->disc[DISC_AUSPICE] >= 4)
			send_to_char("COBRAEYES: Rendering a target docile.\n\r", ch);
			if(ch->disc[DISC_AUSPICE] >= 5)
			send_to_char("BRIDGE WALKER: Step from place to place.\n\r", ch);
			}
		}
		if(ch->auspice == auspice_lookup("ahroun"))
		{
			if(ch->disc[DISC_AUSPICE] > 0)
			{
			send_to_char("Auspice Powers:\n\r", ch);
			if(ch->disc[DISC_AUSPICE] >= 1)
			send_to_char("FALLTOUCH: Knock an opponent down with a touch.\n\r", ch);
			if(ch->disc[DISC_AUSPICE] >= 2)
			send_to_char("RAZOR CLAWS: Increased damage.\n\r", ch);
			if(ch->disc[DISC_AUSPICE] >= 3)
			send_to_char("SENSESILVER: Detecting the presence of silver nearby.\n\r", ch);
			if(ch->disc[DISC_AUSPICE] >= 4)
			send_to_char("SILVERCLAWS: Baring claws of shining silver.\n\r", ch);
			if(ch->disc[DISC_AUSPICE] >= 5)
			send_to_char("STRENGTH OF WILL: Share some of your willpower with your allies. (SHAREWILL)\n\r", ch);
			}
		} */
	}
	break;

	case RACE_VAMPIRE:
	{
		send_to_char("Check out your disciplines and their respective helps.", ch);
	}
	break;
	}
}

void do_gifts(CHAR_DATA *ch, char *argument)
{
	int i = 0;
	int level = 0;
	int count = 0;

	CheckCH(ch);

	if(IS_NULLSTR(argument))
	{
		do_power_list(ch, "");
		return;
	}

	if(!str_prefix(argument, "list"))
	{
		send_to_char("Garou Gifts:\n\r", ch);
		for(level = 1; level < 6; level++) {
			count = 0;
			send_to_char(Format("\n\rLevel %d Gifts:\n\r", level), ch);
			for(i = 0; gift_table[i].name; i++)
			{
				if(gift_table[i].level == level)
				{
					send_to_char(Format("%25s", gift_table[i].name), ch);
					count++;
				}
				if(count == 3)
				{
					send_to_char("\n\r", ch);
					count = 0;
				}
			}
		}
	}
}

void do_diceroll(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *vch, *to;
	char buf[MSL]={'\0'};
	char result[MSL]={'\0'};
	char arg1[MAX_INPUT_LENGTH]={'\0'};
	char arg2[MAX_INPUT_LENGTH]={'\0'};
	char arg3[MAX_INPUT_LENGTH]={'\0'};
	int dice = 0;
	int successcheck = FALSE, difficulty = -1, successes = 0;
	int a = -1, b = -1, c = -1, d = -1, e = -1, f = -1, g = -1, h = -1;
	int health = ch->health + ch->agghealth - 7, whom;

	CheckCH(ch);

	char *extra_names[7] = { "banality", "glamour", "gnosis", "humanity", "rage", "willpower", "faith" };

	argument = one_argument(argument, arg1);
	if(!str_cmp(arg1, "success")) {
		argument = one_argument(argument, arg1);
		if(is_number(arg1))
		{
			difficulty = atoi(arg1);
			argument = one_argument(argument, arg1);
		}
		argument = one_argument(argument, arg3);
		successcheck = TRUE;
	}
	argument = one_argument(argument, arg2);

	if(IS_ADMIN(ch) && (IS_NULLSTR(argument) || str_prefix(argument, "show")))
	{
		if(successcheck)
		{
			if((vch = get_char_world(ch, arg2)) == NULL)
			{
				send_to_char("They aren't here.\n\r", ch);
				return;
			}
		}
		else {
			if((vch = get_char_world(ch, arg2)) == NULL)
			{
				send_to_char("They aren't here.\n\r", ch);
				return;
			}
		}

		to = vch;
		whom = TO_CHAR;

		if(is_number(arg1)) h = atoi(arg1);
		else if((a = stat_lookup(arg1, vch)) == -1)
			if((b = abil_lookup(arg1, vch)) == -1)
				if((c = disc_lookup(vch, arg1)) == -1)
					if((d = virtue_lookup(arg1)) == -1)
						if((e = influence_lookup(arg1)) == -1)
							if((f = background_lookup(arg1)) == -1);
		if(successcheck) {
			if(is_number(arg1)) h = atoi(arg1);
			else if((a = stat_lookup(arg3, vch)) == -1)
				if((b = abil_lookup(arg3, vch)) == -1)
					if((c = disc_lookup(vch, arg3)) == -1)
						if((d = virtue_lookup(arg3)) == -1)
							if((e = influence_lookup(arg3)) == -1)
								if((f = background_lookup(arg3)) == -1);
		}
	}
	else
	{
		vch = ch;
		if(is_number(arg1)) h = atoi(arg1);
		else if((a = stat_lookup(arg1, ch)) == -1)
			if((b = abil_lookup(arg1, ch)) == -1)
				if((c = disc_lookup(ch, arg1)) == -1)
					if((d = virtue_lookup(arg1)) == -1)
						if((e = influence_lookup(arg1)) == -1)
							if((f = background_lookup(arg1)) == -1);
		if(successcheck) {
			if(is_number(arg1)) h = atoi(arg1);
			else if((a = stat_lookup(arg3, ch)) == -1)
				if((b = abil_lookup(arg3, ch)) == -1)
					if((c = disc_lookup(ch, arg3)) == -1)
						if((d = virtue_lookup(arg3)) == -1)
							if((e = influence_lookup(arg3)) == -1)
								if((f = background_lookup(arg3)) == -1);
		}

		to = ch;
		whom = TO_ROOM;
	}

	if(a >= 0)
		dice += get_curr_stat(vch, a);
	else if(b >= 0)
		dice += vch->ability[b].value;
	else if(c >= 0)
		dice += vch->disc[c];
	else if(d >= 0)
		dice += vch->virtues[d];
	else if(e >= 0) {
		dice += vch->influences[e];
		if(number_range(0, 100) >= 98 && vch->influences[e] > 0)
			vch->influences[e]--;
	}
	else if(f >= 0)
		dice += vch->backgrounds[f];
	else if(h >= 0)
		dice += UMIN(h,30);
	else
	{
		switch(LOWER(arg2[0]))
		{
		case 'b':
			if(!str_prefix(arg2, "banality"))
			{
				dice += vch->max_GHB;
				g = 0;
				break;
			}
		case 'f':
			if(!str_prefix(arg2, "faith"))
			{
				/*              if(vch->race == race_lookup("vampire"))
				{
				send_to_char( "Vampires cannot call on faith.\n\r", ch );
				return;
				}*/

				dice += vch->max_RBPG;
				g = 6;
				break;
			}
		case 'g':
			if(!str_prefix(arg2, "glamour"))
			{
				dice += vch->max_RBPG;
				g = 1;
				break;
			}
			if(!str_prefix(arg2, "gnosis"))
			{
				dice += vch->max_GHB;
				g = 2;
				break;
			}
		case 'h':
			if(!str_prefix(arg2, "humanity"))
			{
				dice += vch->max_GHB;
				g = 3;
				break;
			}
		case 'r':
			if(!str_prefix(arg2, "rage"))
			{
				dice += vch->max_RBPG;
				g = 4;
				break;
			}
		case 'w':
			if(!str_prefix(arg2, "willpower"))
			{
				dice += vch->max_willpower;
				g = 5;
				break;
			}
		default:
			send_to_char("Syntax: droll [stat/ability/power/virtue/influence/background] [person]\n\r", ch);
			return;
			break;
		}
	}

	if(vch->dice_mod)
	{
		act(Format("(Dice modification: %d dice)\n\r", vch->dice_mod), ch, NULL, to, whom, 1);
		if(whom!=TO_CHAR) send_to_char(buf, ch);
	}

	if(vch->diff_mod)
	{
		act(Format("(Difficulty modification: %d difficulty)\n\r", vch->diff_mod), ch, NULL, to, whom, 1);
		if(whom!=TO_CHAR) send_to_char(buf, ch);
	}

	if(vch->race == race_lookup("vampire") && !IS_NIGHT())
	{
		act(Format("(Vampire in daylight: -3 dice)\n\r"), ch, NULL, to, whom, 1);
		dice = UMAX(dice - 3, 1);
		if(whom!=TO_CHAR) send_to_char(buf, ch);
	}

	

	if(!IS_AFFECTED(ch, AFF_RESIST_PAIN))
	{
		if(health == 6 || health == 5)
		{
			act(Format("(Health modifier: -1 dice)\n\r"), ch, NULL, to, whom, 1);
			if(whom!=TO_CHAR) send_to_char(buf, ch);
		}
		else if(health == 4 || health == 3)
		{
			act(Format("(Health modifier: -2 dice)\n\r"), ch, NULL, to, whom, 1);
			if(whom!=TO_CHAR) send_to_char(buf, ch);
		}
		else if(health == 2 || health == 1)
		{
			act(Format("(Health modifier: -5 dice)\n\r"), ch, NULL, to, whom, 1);
			if(whom!=TO_CHAR) send_to_char(buf, ch);
		}
		else if(health <= 0)
		{
			act(Format("(Health modifier: No dice at all.)\n\r"), ch, NULL, to, whom, 1);
			if(whom!=TO_CHAR) send_to_char(buf, ch);
		}
	}

	if(ch->fighting != NULL)
		if(IS_AFFECTED(ch->fighting, AFF_ODOUR)) {
			act(Format("(Increase difficulty: Fighting skunk gift +1)\n\r"), ch, NULL, to, whom, 1);
			if(whom!=TO_CHAR) send_to_char(buf, ch);
		}

	snprintf(result, sizeof(result), "%s rolls: ", IS_NPC(vch) ? vch->short_descr : vch->name);
	if(a >= 0)
	{
		strncat(result, Format("%s ", stat_table[a].name), sizeof(result) );
	}
	if(b >= 0)
	{
		strncat(result, Format("%s ", ability_table[b].name), sizeof(result) );
	}
	if(c >= 0)
	{
		if(ch->race == race_lookup("human"))
		{
			strncat(result, Format("%s ", disc_table[c].hname), sizeof(result) );
		}
		else if(IS_VAMPIRE(ch))
		{
			strncat(result, Format("%s ", disc_table[c].vname), sizeof(result) );
		}
		else if(ch->race == race_lookup("werewolf"))
		{
			strncat(result, Format("%s ", disc_table[c].wname), sizeof(result) );
		}
		else if(ch->race == race_lookup("faerie"))
		{
			strncat(result, Format("%s ", disc_table[c].fname), sizeof(result) );
		}
	}
	if(d >= 0)
	{
		strncat(result, Format("%s ", virtue_table[d].name), sizeof(result));
	}
	if(e >= 0)
	{
		strncat(result, Format("%s ", influence_table[e].name), sizeof(result));
	}
	if(f >= 0)
	{
		strncat(result, Format("%s ", background_table[f].name), sizeof(result));
	}
	if(g >= 0)
	{
		strncat(result, Format("%s ", extra_names[g]), sizeof(result));
	}
	if(h >= 0)
	{
		strncat(result, Format("%d dice ", h), sizeof(result));
	}
	strncat(result, "\n\r", sizeof(result));

	if(dice <= 0)
	{
		act(Format("No dice to roll.\n\r"), ch, NULL, to, whom, 1);
		if(whom!=TO_CHAR) send_to_char(buf, ch);
	}
	else
	{
		for(; dice > 0; dice--)
		{
			a = number_range(1, 10);
			if(IS_SET(ch->off_flags, LOADED_DICE_POS))
				a = UMAX(number_fuzzy(a), a);
			if(IS_SET(ch->off_flags, LOADED_DICE_NEG))
				a = UMIN(number_fuzzy(a), a);
			if(IS_SET(ch->off_flags, LOADED_DICE_BPOS))
				a = UMIN(UMAX(a+2, a), 10);
			if(IS_SET(ch->off_flags, LOADED_DICE_BNEG))
				a = UMAX(UMIN(a-2, a), 1);
			strncat(result, Format("%d ", a), sizeof(result) );
			if(difficulty > 1)
			{
				if(a >= difficulty) successes++;
				if(a == 1) successes--;
			}
			else
				successes++;
		}
		strncat(result, "\n\r", sizeof(result) );
		if(successcheck)
		{
			if(successes >= 0)
				strncat(result, Format("Successes: %d\n\r", successes), sizeof(result) );
			else
				strncat(result, Format("Successes: Botch!\n\r"), sizeof(result) );
		}
	}
	act(result, ch, NULL, to, whom, 1);
	if(whom!=TO_CHAR) send_to_char(result, ch);
}


NOMINEE_DATA vote_tally[MAX_OFFICES][MAX_NOMINEES];
char *incumbent_mayor;
char *incumbent_alder[MAX_ALDERMEN]={'\0'};
char *incumbent_judge[MAX_JUDGES]={'\0'};
char *incumbent_pchief;

time_t next_election_result;
time_t last_election_result;
int election_result[MAX_OFFICES];

bool is_nominee(CHAR_DATA *ch, int office)
{
	int i = 0;

	for(i = 0; i < MAX_NOMINEES; i++)
	{
		if(vote_tally[office][i].name != NULL
				&& !str_cmp(ch->name, vote_tally[office][i].name))
			return TRUE;
	}

	return FALSE;
}

void do_nominate(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *nominee;
	char arg[MAX_INPUT_LENGTH]={'\0'};
	bool online = FALSE;
	bool in_char_list = FALSE;
	int i = 0;

	CheckCH(ch);

	argument = one_argument(argument, arg);
	if(IS_NULLSTR(argument) || IS_NULLSTR(arg))
	{
		send_to_char("Nominate whom for which position?\n\r", ch);
		return;
	}

	if(ch->pcdata->last_vote > current_time - 24*60*60 && !IS_ADMIN(ch))
	{
		send_to_char("Only one vote per 24 hours.\n\r", ch);
		return;
	}

	if(is_online(arg)) online = TRUE;
	if(pc_in_char_list(arg)) in_char_list = TRUE;

	if((nominee = get_player(arg)) == NULL)
	{
		send_to_char("No such person.\n\r", ch);
		return;
	}

	if(nominee->race != race_lookup("human"))
	{
		send_to_char("Only mortals should be in the public eye.\n\r", ch);
		return;
	}

	if(is_nominee(nominee, 0))
	{
		send_to_char("They're already nominated to be mayor!\n\r", ch);
		return;
	}
	else if(is_nominee(nominee, 1))
	{
		send_to_char("They're already nominated to be an alderman!\n\r", ch);
		return;
	}
	else if(is_nominee(nominee, 2))
	{
		send_to_char("They're already nominated to be a judge!\n\r", ch);
		return;
	}

	if(!str_prefix(argument, "mayor"))
	{
		for(i = 0; i < 6; i++)
		{
			if(vote_tally[0][i].name == NULL
					|| !str_cmp(vote_tally[0][i].name, "None"))
			{
				vote_tally[0][i].name = str_dup(nominee->name);
				vote_tally[0][i].votes = 1;
				break;
			}
		}
	}
	else if(!str_prefix(argument, "alderman"))
	{
		for(i = 0; i < 6; i++)
		{
			if(vote_tally[1][i].name == NULL
					|| !str_cmp(vote_tally[0][i].name, "None"))
			{
				vote_tally[1][i].name = str_dup(nominee->name);
				vote_tally[1][i].votes = 1;
				break;
			}
		}
	}
	else if(!str_prefix(argument, "judge"))
	{
		for(i = 0; i < 6; i++)
		{
			if(vote_tally[2][i].name == NULL
					|| !str_cmp(vote_tally[0][i].name, "None"))
			{
				vote_tally[2][i].name = str_dup(nominee->name);
				vote_tally[2][i].votes = 1;
				break;
			}
		}
	}
	else
	{
		send_to_char("Positions available are: judge, alderman, mayor.\n\r", ch);
		return;
	}

	ch->pcdata->last_vote = current_time;
	fwrite_votes();
	send_to_char("Nomination submitted.\n\r", ch);

	if(!online && !in_char_list)
	{
		free_char(nominee);
	}
}

void do_vote(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *nominee;
	char arg[MAX_INPUT_LENGTH]={'\0'};
	int i = 0;
	bool online = FALSE;
	bool in_char_list = FALSE;

	CheckCH(ch);

	argument = one_argument(argument, arg);

	if(IS_NULLSTR(argument) || IS_NULLSTR(arg))
	{
		send_to_char("Vote whom for which position?\n\r", ch);
		return;
	}

	if(ch->pcdata->last_vote > current_time - 24*60*60)
	{
		send_to_char("Only one vote per 24 hours.\n\r", ch);
		return;
	}

	if(is_online(arg)) online = TRUE;
	if(pc_in_char_list(arg)) in_char_list = TRUE;

	if((nominee = get_player(arg)) == NULL)
	{
		send_to_char("No such person.\n\r", ch);
		return;
	}

	if(nominee->race != race_lookup("human"))
	{
		send_to_char("Only the mortal should be in the public eye.\n\r", ch);
		return;
	}

	if(!str_prefix(argument, "mayor"))
	{
		if(!is_nominee(nominee, 0))
		{
			send_to_char("No such nominee.\n\r", ch);
			return;
		}

		for(i = 0; i < 6; i++)
		{
			if(!str_cmp(vote_tally[0][i].name, nominee->name))
			{
				vote_tally[0][i].votes++;
				break;
			}
		}
	}
	else if(!str_prefix(argument, "alderman"))
	{
		if(!is_nominee(nominee, 1))
		{
			send_to_char("No such nominee.\n\r", ch);
			return;
		}

		for(i = 0; i < 6; i++)
		{
			if(!str_cmp(vote_tally[1][i].name, nominee->name))
			{
				vote_tally[1][i].votes++;
				break;
			}
		}
	}
	else if(!str_prefix(argument, "judge"))
	{
		if(!is_nominee(nominee, 2))
		{
			send_to_char("No such nominee.\n\r", ch);
			return;
		}

		for(i = 0; i < 6; i++)
		{
			if(!str_cmp(vote_tally[2][i].name, nominee->name))
			{
				vote_tally[2][i].votes++;
				break;
			}
		}
	}
	else
	{
		send_to_char("Positions available are: judge, alderman, mayor.\n\r", ch);
		return;
	}

	ch->pcdata->last_vote = current_time;
	fwrite_votes();
	send_to_char("Vote lodged.\n\r", ch);

	if(!online && !in_char_list)
		free_char(nominee);
}

void announce_election()
{
	CHAR_DATA *ch;
	int i = 0, j = 0;
	bool online = FALSE;
	bool in_char_list = FALSE;

	for(i=0;i<MAX_OFFICES;i++)
	{
		online = FALSE;
		if(vote_tally[i][election_result[i]].name == NULL
				|| !str_cmp(vote_tally[i][election_result[i]].name, "None"))
			continue;

		if(is_online(vote_tally[i][election_result[i]].name))
			online = TRUE;

		if(pc_in_char_list(vote_tally[i][election_result[i]].name))
			in_char_list = TRUE;

		if((ch = get_player(vote_tally[i][election_result[i]].name)) != NULL) {
			do_echo( NULL, (char *)Format("%s has been elected as %s.\n", ch->name, office_table[i].name) );
			PURGE_DATA(ch->profession);
			ch->profession = str_dup(office_table[i].name);
			ch->pospts = 100;
			ch->backgrounds[FAME_STATUS] += office_table[i].fame;
			save_char_obj(ch);
			if(!online && !in_char_list)
				free_char(ch);
		}

		online = FALSE;
		in_char_list = FALSE;
		switch(i)
		{
		case 0:
			if(is_online(incumbent_mayor))
				online = TRUE;
			if(pc_in_char_list(incumbent_mayor))
				in_char_list = TRUE;
			if((ch = get_player(incumbent_mayor)) != NULL) {
				PURGE_DATA(ch->profession);
				ch->profession = str_dup("None");
				ch->backgrounds[FAME_STATUS] -= office_table[i].fame / 2;
				save_char_obj(ch);
				if(!online && !in_char_list)
					free_char(ch);
			}
			break;

		case 1:
			for(j = 0; j < MAX_ALDERMEN; j++)
			{
				if(is_online(incumbent_alder[j]))
					online = TRUE;
				if(pc_in_char_list(incumbent_alder[j]))
					in_char_list = TRUE;
				if((ch = get_player(incumbent_alder[j])) != NULL) {
					PURGE_DATA(ch->profession);
					ch->profession = str_dup("None");
					ch->backgrounds[FAME_STATUS] -= office_table[i].fame / 2;
					save_char_obj(ch);
					if(!online && !in_char_list)
						free_char(ch);
				}}
			break;

		case 2:
			for(j = 0; j < MAX_JUDGES; j++)
			{
				if(is_online(incumbent_judge[j]))
					online = TRUE;
				if(pc_in_char_list(incumbent_judge[j]))
					in_char_list = TRUE;
				if((ch = get_player(incumbent_judge[j])) != NULL) {
					PURGE_DATA(ch->profession);
					ch->profession = str_dup("None");
					ch->backgrounds[FAME_STATUS] -= office_table[i].fame / 2;
					save_char_obj(ch);
					if(!online && !in_char_list)
						free_char(ch);
				}}
			break;
		}

	}
}

void run_election()
{
	int i = 0,j = 0,k = 0;
	bool is_tie;

	for(i = 0; i < MAX_OFFICES; i++)
		election_result[i] = 0;

	for(i = 0; i < MAX_OFFICES; i++)
	{
		is_tie = FALSE;
		k = 0;
		for(j = 0; j < MAX_NOMINEES; j++)
		{
			if(k < vote_tally[i][j].votes)
				continue;
			else if(k == vote_tally[i][j].votes)
				is_tie = TRUE;
			else if(k > vote_tally[i][j].votes)
			{
				is_tie = FALSE;
				k = vote_tally[i][j].votes;
			}
		}
		if(is_tie)
		{
			for(j = 0; j < MAX_NOMINEES; j++)
			{
				if(k == vote_tally[i][j].votes)
				{
					vote_tally[i][j].votes += number_range(0,5);
				}
			}
			i--;
		}
		else
		{
			for(j = 0; j < MAX_NOMINEES; j++)
			{
				if(k == vote_tally[i][j].votes)
					election_result[i] = j;
			}
		}
	}

	last_election_result = next_election_result;
	next_election_result = current_time + 14*24*60*60;
	announce_election();
	for(i=0;i<MAX_OFFICES;i++)
		for(j=0;j<MAX_NOMINEES;j++)
		{
			PURGE_DATA(vote_tally[i][j].name);
			vote_tally[i][j].name = str_dup("None");
			vote_tally[i][j].votes = 0;
		}
	fwrite_votes();
}

void do_nominees (CHAR_DATA *ch, char *argument)
{
	int i = 0,j = 0;

	CheckCH(ch);

	send_to_char("The nominees are:\n\r", ch);
	for(i=0;i<MAX_OFFICES;i++)
	{
		send_to_char(Format("%s: ", office_table[i].name), ch);
		for(j=0;j<MAX_NOMINEES;j++)
		{
			if(IS_NULLSTR(vote_tally[i][j].name))
				continue;

			send_to_char(Format("%s%s", !str_cmp(vote_tally[i][j].name, "None") ? "" : vote_tally[i][j].name, !str_cmp(vote_tally[i][j].name, "None") ? "" : " "), ch);
		}
		send_to_char("\n\r", ch);
	}
}

void do_nature (CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH]={'\0'};

	CheckCH(ch);
	CheckChNPC(ch);

	if ( IS_NULLSTR(argument) )
	{
		send_to_char("What do you want your character's nature to be?\n\r", ch);
		return;
	}

	if (str_cmp(ch->nature, "None"))
	{
		send_to_char("You have already set your nature.\n\r", ch);
		return;
	}

	one_argument(argument, arg);

	if (flag_lookup(arg, personality_archetypes) < 0)
	{
		send_to_char("That personality archetype is not recognised.\n\r", ch);
		return;
	}

	smash_tilde( arg );
	PURGE_DATA(ch->nature);
	ch->nature = str_dup(arg);
	send_to_char( "Ok.\n\r", ch );
}

void do_demeanor (CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH]={'\0'};

	CheckCH(ch);
	CheckChNPC(ch);

	if ( IS_NULLSTR(argument) )
	{
		send_to_char("What do you want your character's demeanor to be?\n\r", ch);
		return;
	}

	one_argument(argument, arg);

	if (flag_lookup(arg, personality_archetypes) < 0)
	{
		send_to_char("That personality archetype is not recognised.\n\r", ch);
		return;
	}

	smash_tilde( arg );
	PURGE_DATA(ch->demeanor);
	ch->demeanor = str_dup(arg);
	send_to_char( "Ok.\n\r", ch );
}

void do_archetypes(CHAR_DATA *ch, char *argument)
{
	int i = 0;

	CheckCH(ch);

	if(ch->race != race_lookup("vampire") && ch->race != race_lookup("human"))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	send_to_char("The Personality Archetypes are:\n\r", ch);

	for(i = 0; personality_archetypes[i].name; i++)
	{
		send_to_char(Format("%24s", personality_archetypes[i].name), ch);
		if(!((i+1)%3))
		{
			send_to_char("\n\r", ch);
		}
	}

	send_to_char("\n\r", ch);
}

void do_trainingcost(CHAR_DATA *ch, char *argument)
{
	char result[MSL]={'\0'};
	char arg1[MAX_INPUT_LENGTH]={'\0'};
	char arg2[MAX_INPUT_LENGTH]={'\0'};
	int dice = 0;
	int a = -1, b = -1, c = -1, d = -1;

	CheckCH(ch);

	if(IS_NULLSTR(argument))
	{
		send_to_char("What do you want to calculate the cost for?\n\r", ch);
		return;
	}

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if((a = stat_lookup(arg1, ch)) == -1)
	{
		if((b = abil_lookup(arg1, ch)) == -1)
		{
			if((c = disc_lookup(ch, arg1)) == -1)
			{
				if((d = virtue_lookup(arg1)) == -1)
				{
					send_to_char("Syntax: costcalc [stat/ability/power/virtue]\n\r", ch);
					return;
				}
			}
		}
	}

	if(a >= 0)
		dice = ch->perm_stat[a];
	else if(b >= 0)
		dice = ch->ability[b].value;
	else if(c >= 0)
		dice = ch->disc[c];
	else if(d >= 0)
		dice = ch->virtues[d];

	if((a == stat_lookup("appearance", ch) && ch->clan == clan_lookup("nosferatu")) || dice == 5)
	{
		send_to_char("You can't increase that.\n\r", ch);
		return;
	}

	send_to_char("The cost is: ", ch);
	if(a >= 0)
	{
		strncat(result, Format("%d.", xp_cost_mod(ch, dice ? dice * 4: 10, a)), sizeof(result));
	}
	else if(b >= 0)
	{
		strncat(result, Format("%d.", xp_cost_mod(ch, dice ? dice * 2: 3, b)), sizeof(result) );
	}
	else if(c >= 0)
	{
		if (ch->disc[c] == 0)
			dice = 10;
		else if(c == clan_table[ch->clan].powers[0] || c == clan_table[ch->clan].powers[1] || c == clan_table[ch->clan].powers[2])
			dice = 5 * ch->disc[c];
		else if(ch->clan == clan_lookup("none"))
			dice = 7 * ch->disc[c];
		else
			dice = 10 * ch->disc[c];
		strncat(result, Format("%d.", xp_cost_mod(ch, dice, c)), sizeof(result) );
	}
	else if(d >= 0)
	{
		strncat(result, Format("%d. (approx only)", xp_cost_mod(ch, dice ? dice * 3: 10, d)), sizeof(result) );
	}
	strncat(result, "\n\r", sizeof(result) );

	send_to_char(result, ch);
}

void do_setage(CHAR_DATA *ch, char *argument)
{
	int age = 0;

	CheckCH(ch);

	if(!is_number(argument))
	{
		send_to_char("What is your character's apparent age?\n\r", ch);
		return;
	}

	if((age = atoi(argument)) <= 10)
	{
		send_to_char("You must set a reasonable apparent age.\n\r", ch);
		return;
	}

	ch->char_age = age;
	send_to_char("Age set.\n\r", ch);
}

void do_experience(CHAR_DATA *ch, char *argument)
{
	CheckCH(ch);
	send_to_char(Format("You have %d experience points to spend and %d to give away.\n\r", ch->exp, ch->xpgift), ch);
	send_to_char("To use your experience, type advance or train.\n\r", ch);
	send_to_char("To give away experience, type gift.\n\r", ch);
	send_to_char("For more information, read HELP ADVANCE, HELP TRAIN and HELP GIFT.\n\r", ch);
}

void do_generation(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;

	CheckCH(ch);

	if(!IS_ADMIN(ch))
	{
		send_to_char(Format("Your generation is: %d.\n\r", ch->gen), ch);
		return;
	}

	if(IS_NULLSTR(argument))
	{
		send_to_char("Whose generation do you wish to check?\n\r", ch);
		return;
	}

	if((victim = get_char_world(ch, argument)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	if(victim->race != race_lookup("vampire"))
	{
		send_to_char( "Not being a vampire, generation has no meaning for them.\n\r", ch);
		return;
	}

	send_to_char(Format("%s resides in generation %d.\n\r", victim->name, victim->gen), ch);
}

void do_stats(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *user = ch;

	CheckCH(ch);

	send_to_char("\tW--------------------------------<Attributes>----------------------------------\tn\n\r", user);

	send_to_char( Format("Str: %5d  Cha: %5d  Per: %5d\n\r",
	get_curr_stat(ch,STAT_STR),
	get_curr_stat(ch,STAT_CHA),
	get_curr_stat(ch,STAT_PER)), user );

	send_to_char( Format("Dex: %5d  Man: %5d  Int: %5d\n\r",
	get_curr_stat(ch,STAT_DEX),
	get_curr_stat(ch,STAT_MAN),
	get_curr_stat(ch,STAT_INT)), user );

	send_to_char( Format("Sta: %5d  App: %5d  Wit: %5d\n\r",
	get_curr_stat(ch,STAT_STA),
	get_curr_stat(ch,STAT_APP),
	get_curr_stat(ch,STAT_WIT)), user );

	send_to_char("--------------------------------<          >----------------------------------\n\r", user);
}

void do_talents(CHAR_DATA *ch, char *argument)
{
	int sn = 0, count = 0;

	CheckCH(ch);

	if (IS_NPC(ch))
		return;

	send_to_char("\n\r\tW--------------------------------<Talents>---------------------------------\tn\n\r", ch);

	for (sn = 0; sn < 13; sn++)
	{
		if (ability_table[sn].name == NULL )
			break;

		if(IS_ATTRIB_AVAILABLE(ch->race, sn))
		{
			send_to_char(Format("%-15s %-2d      ", capitalize(ability_table[sn].name), ch->ability[sn].value), ch);
			count++;if(!(count%3)) send_to_char("\n\r", ch);
		}
	}
	send_to_char("\n\r", ch);
}

void do_skills(CHAR_DATA *ch, char *argument)
{
	int sn = 0, count = 0;

	CheckCH(ch);

	if (IS_NPC(ch))
		return;

	send_to_char("\n\r\tW---------------------------------<Skills>----------------------------------\tn\n\r", ch);

	for (sn = 13; sn < 24; sn++)
	{
		if (ability_table[sn].name == NULL )
			break;

		if(IS_ATTRIB_AVAILABLE(ch->race, sn))
		{
			send_to_char(Format("%-15s %-2d      ", capitalize(ability_table[sn].name), ch->ability[sn].value), ch);
			count++;
			if(!(count%3))
				send_to_char("\n\r", ch);
		}
	}
	send_to_char("\n\r", ch);
}

void do_knowstats(CHAR_DATA *ch, char *argument)
{
	int sn = 0, count = 0;

	CheckCH(ch);

	if (IS_NPC(ch))
		return;

	send_to_char("\n\r\tW--------------------------------<Knowledges>---------------------------------\tn\n\r", ch);

	for (sn = 24; sn < MAX_ABIL; sn++)
	{
		if (ability_table[sn].name == NULL )
			break;

		if(IS_ATTRIB_AVAILABLE(ch->race, sn))
		{
			send_to_char(Format("%-15s %-2d      ", capitalize(ability_table[sn].name), ch->ability[sn].value), ch);
			count++;if(!(count%3)) send_to_char("\n\r", ch);
		}
	}
	send_to_char("\n\r", ch);
}

void do_premise(CHAR_DATA *ch, char *argument)
{
	do_help(ch, "premise");
}

void do_whorp(CHAR_DATA *ch, char *argument)
{
	do_who(ch, "rp");
}

void do_sentence(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH]={'\0'};
	CHAR_DATA *vch;

	CheckCH(ch);

	if(ch->in_room->vnum != ROOM_VNUM_JAIL && !IS_ADMIN(ch))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if(!IS_ADMIN(ch) || IS_NULLSTR(argument))
	{
		send_to_char(Format("You have a sentence of %d ticks.\n\r", ch->warrants/4), ch);
		return;
	}

	argument = one_argument(argument, arg);

	if((vch = get_char_world(ch, arg)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	if(!IS_NULLSTR(argument) && !is_number(argument))
	{
		send_to_char("To set warrants on a target, use: sentence <target> <amount>\n\r", ch);
		return;
	}

	if(!IS_NULLSTR(argument) && is_number(argument))
	{
		vch->warrants = atoi(argument) * 4;
		act(Format("$N now has a sentence of %d ticks.\n\r", vch->warrants/4), ch, NULL, vch, TO_CHAR, 1);
		return;
	}

	if(vch->in_room->vnum == ROOM_VNUM_JAIL)
	{
		act(Format("$N has a sentence of %d ticks.", vch->warrants/4), ch, NULL, vch, TO_CHAR, 1);
	}
	else if(vch->warrants >= 50)
	{
		act(Format("$N will have a sentence of %d ticks if caught.", vch->warrants/4), ch, NULL, vch, TO_CHAR, 1);
	}
	else
	{
		act(Format("$N hasn't got enough warrants out for $M arrest."), ch, NULL, vch, TO_CHAR, 1);
	}

}

void do_hunted(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *vch;

	CheckCH(ch);

	send_to_char(Format("\tW%15s%21s%23s\tn\n\r", "Name", "Warrant Level","Hunter Activity"), ch);

	for(vch = char_list; vch != NULL; vch = vch->next)
	{
		if(IS_NPC(vch)) continue;

		send_to_char(Format("\tY%15s\tn", vch->name), ch);

		if(vch->in_room->vnum == ROOM_VNUM_JAIL)
		{
			send_to_char(Format("\tRIn jail (%3d ticks) \tn", vch->warrants/4), ch);
		}
		else if(vch->warrants >= 50)
		{
			send_to_char(Format("\tOArrestable (%3d)    \tn", vch->warrants/4), ch);
		}
		else if(vch->warrants >= 44)
		{
			send_to_char(Format("\tYBorderline (%3d)    \tn", vch->warrants/4), ch);
		}
		else
		{
			send_to_char(Format("%21s", "\tGNone\tn"), ch);
		}

		if(vch->hunter_vis >= 50)
		{
			send_to_char(Format("\tRActively hunted (%3d) \tn", vch->hunter_vis), ch);
		}
		else if(vch->hunter_vis >= 44)
		{
			send_to_char(Format("\tONear alerting (%3d)   \tn", vch->hunter_vis), ch);
		}
		else if(vch->hunter_vis > 0)
		{
			send_to_char(Format("\tYNot alerting (%3d)    \tn",vch->hunter_vis), ch);
		}
		else
		{
			send_to_char(Format("%23s", "\tGNone\tn"), ch);
		}

		send_to_char("\n\r", ch);
	}
}

void do_celebs(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *vch;

	CheckCH(ch);

	send_to_char("\tWCelebrities currently online are:\tn\n\r", ch);

	for(vch = char_list; vch != NULL; vch = vch->next)
	{
		if(IS_NPC(vch) || vch->backgrounds[FAME_STATUS] == 0) continue;

		send_to_char(Format("\tY%15s - %s\tn\n\r", vch->name, fame_table[vch->backgrounds[FAME_STATUS]].name), ch);
	}
}

void do_nocommand(CHAR_DATA *ch, char *argument)
{
	CheckCH(ch);
	send_to_char("Huh?\n\r", ch);
}

void do_knock(CHAR_DATA *ch, char *argument)
{
	EXIT_DATA *pEx;

	CheckCH(ch);

	if((pEx = ch->in_room->exit[find_door(ch, argument)]) == NULL)
	{
		return;
	}

	if(!IS_SET(pEx->exit_info, EX_CLOSED))
	{
		send_to_char("Why would you want to knock on an open portal?\n\r", ch);
		return;
	}

	send_to_char(pEx->keyword, ch);

	act("$n knocks on the $t$T.", ch, pEx->keyword[0]!='\0'?pEx->keyword:dir_name[pEx->orig_door],
					pEx->keyword[0]!='\0'?"":" door", TO_ROOM, 0);
	act("You knock on the $t$T.", ch, pEx->keyword[0]!='\0'?pEx->keyword:dir_name[pEx->orig_door],
					pEx->keyword[0]!='\0'?"":" door", TO_CHAR, 0);
	act("You hear a knock from the $t.", pEx->u1.to_room->people, dir_name[rev_dir[pEx->orig_door]], NULL, TO_CHAR, 0);
	act("You hear a knock from the $t.", pEx->u1.to_room->people, dir_name[rev_dir[pEx->orig_door]], NULL, TO_ROOM, 0);
}

void do_surname(CHAR_DATA *ch, char *argument)
{
	CheckCH(ch);

	if(IS_NULLSTR(argument))
	{
		send_to_char("Lastname cleared.\n\r", ch);
		PURGE_DATA(ch->surname);
		ch->surname = NULL;
		return;
	}

	if( strlen(argument) > 15 )
	{
		send_to_char("Last names cannot be longer than 15 characters.", ch);
		return;
	}

	send_to_char("Your lastname has been set.\n\r", ch);
	PURGE_DATA(ch->surname);
	ch->surname = str_dup(argument);
}

void do_laston(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *vch = NULL;
	bool in_char_list = FALSE;

	CheckCH(ch);

	if(IS_NULLSTR(argument))
	{
		send_to_char("Whose last logon are you trying to check?\n\r", ch);
		return;
	}

	if(is_online(argument))
	{
		send_to_char("\tGThey are currently online.\tn\n\r", ch);
		return;
	}

	if(pc_in_char_list(argument))
		in_char_list = TRUE;

	if((vch = get_player(argument)) == NULL)
	{
		send_to_char("That player does not exist.\n\r", ch);
		return;
	}

	if(IS_NULLSTR(vch->laston))
		act("$N has not saved since the implementation of the laston command.", ch, NULL, vch, TO_CHAR, 1);
	else
		act("\tR$N is not online.  $N was last online: $t\tn.", ch, vch->laston, vch, TO_CHAR, 1);

	if(!in_char_list) free_char(vch);
}

void do_setting(CHAR_DATA *ch, char *argument)
{
	do_help(ch, "setting");
}

void do_theme(CHAR_DATA *ch, char *argument)
{
	do_help(ch, "theme");
}

void do_disclaim(CHAR_DATA *ch, char *argument)
{
	do_help(ch, "disclaimer");
}

void do_www(CHAR_DATA *ch, char *argument)
{
	do_help(ch, "website");
}

void do_stocks(CHAR_DATA *ch, char *argument)
{
	STOCKS *stock, *chst;
	int count = 0;

	CheckCH(ch);

	if(stock_list == NULL)
	{
		send_to_char("\tRWARNING: The market has not been configured yet.\tn\n\r", ch);
		return;
	}

	send_to_char("\tWTicker | Name                      | Price          | Owned\tn\n\r", ch);
	send_to_char("\tW-----------------------------------------------------------\tn\n\r", ch);
	for(stock = stock_list; stock; stock = stock->next)
	{
		for(chst = ch->stocks; chst != NULL; chst = chst->next)
		{
			if(!str_cmp(chst->ticker, stock->ticker)) break;
		}

		count++;
		send_to_char(Format("\tY[%4s] \tW|\tY %-25s \tW|\tY ($%5d.%.2d)    \tW|\tY %d\tn",
				stock->ticker, stock->name, stock->cost/100, stock->cost%100,
				chst==NULL?0:chst->cost), ch);
		send_to_char("\n\r", ch);
	}
}

void do_use(CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *obj;
	CHAR_DATA *vch;
	char arg[MAX_INPUT_LENGTH]={'\0'};
	char buf[MSL]={'\0'};
	int sn = 0, cmd = 0;

	CheckCH(ch);

	argument = one_argument(argument, arg);

	if(!str_cmp(arg, "phone") || !str_cmp(arg, "telephone"))
	{
		if(IS_NULLSTR(argument))
		{
			send_to_char("Use the call command to make a call.\n\r", ch);
			return;
		}
		snprintf(buf, sizeof(buf), "call %s", argument);
		interpret(ch, buf);
		return;
	}
	if(!str_cmp(arg, "payphone"))
	{
		if(IS_NULLSTR(argument))
		{
			send_to_char( "Payphones are not in service, find a mobile phone.\n\r", ch);
			return;
		}
		return;
	}
	if(!str_cmp(arg, "computer"))
	{
		send_to_char( "At the moment there's really no way to use computers.\n\r", ch);
		send_to_char( "If you can think of a way you'd like to use one, let us know.\n\r", ch);
		return;
	}
	if(!str_cmp(arg, "blood") || !str_cmp(arg, "vitae"))
	{
		do_help(ch, "bloodpoints");
	}
	if ( ( obj = get_obj_wear( ch, arg ) ) != NULL ) {
	if(obj->item_type == ITEM_PILL)
		send_to_char("You have to EAT a pill.\n\r", ch);
	else if(obj->item_type == ITEM_TELEPHONE)
		send_to_char("Use the call command to make a call.\n\r", ch);
	else if(obj->to_user_none[0] == '\0' && obj->to_room_none[0] == '\0'
		 && obj->to_user_self[0] == '\0' && obj->to_room_self[0] == '\0'
		 && obj->to_user_other[0] == '\0' && obj->to_room_other[0] == '\0'
		 && obj->to_vict_other[0] == '\0') {
		send_to_char(Format("I'm not sure what you can use a %s for.\n\r",arg), ch);
	}
	else {
		if(obj->uses > 0 || obj->uses == -1)
		{
			if(IS_NULLSTR(argument))
			{
				if(!IS_NULLSTR(obj->to_user_none))
						act( obj->to_user_none, ch, NULL, NULL, TO_CHAR, 0 );
				if(!IS_NULLSTR(obj->to_room_none))
					act( obj->to_room_none, ch, NULL, NULL, TO_ROOM, 0 );
			}
			else if((vch = get_char_room(ch, argument)) == NULL)
				send_to_char("They aren't here.\n\r", ch);
			else if(vch == ch)
			{
				if(!IS_NULLSTR(obj->to_user_self) )
					act( obj->to_user_self, ch, NULL, vch, TO_CHAR, 0 );
				if(!IS_NULLSTR(obj->to_room_self) )
					act( obj->to_room_self, ch, NULL, vch, TO_ROOM, 0 );
			}
			else
			{
				if(!IS_NULLSTR(obj->to_user_other) )
					act( obj->to_user_other, ch, NULL, vch, TO_CHAR, 0 );
				if(!IS_NULLSTR(obj->to_room_other) )
					act( obj->to_room_other, ch, NULL, vch, TO_NOTVICT, 0 );
				if(!IS_NULLSTR(obj->to_vict_other) )
					act( obj->to_vict_other, ch, NULL, vch, TO_VICT, 0 );
			}
			if(obj->uses > 0) obj->uses--;
		}
		else {
			if(!IS_NULLSTR(obj->to_user_used) )
				act( obj->to_user_used, ch, NULL, NULL, TO_CHAR, 0 );
			if(!IS_NULLSTR(obj->to_room_self) )
				act( obj->to_room_used, ch, NULL, NULL, TO_ROOM, 0 );
		}
	}
	return;
	}
	if((obj = get_obj_carry(ch, arg, ch)) != NULL) {
	if(obj->item_type == ITEM_PILL)
		send_to_char("You have to EAT a pill.\n\r", ch);
	else if(obj->item_type == ITEM_WEAPON)
	send_to_char("Try wielding it. (Then use the combat commands.)\n\r",ch);
	else if(obj->item_type == ITEM_TELEPHONE)
		send_to_char("Use the call command to make a call.\n\r", ch);
	else if(obj->to_user_none[0] == '\0' && obj->to_room_none[0] == '\0'
		 && obj->to_user_self[0] == '\0' && obj->to_room_self[0] == '\0'
		 && obj->to_user_other[0] == '\0' && obj->to_room_other[0] == '\0'
		 && obj->to_vict_other[0] == '\0') {
		send_to_char(Format("I'm not sure what you can use a %s for.\n\r",arg), ch);
	}
	else {
		if(obj->uses > 0 || obj->uses == -1) {
		if(IS_NULLSTR(argument)) {
			if(!IS_NULLSTR(obj->to_user_none) )
			act( obj->to_user_none, ch, NULL, NULL, TO_CHAR, 0 );
			if(!IS_NULLSTR(obj->to_room_none))
			act( obj->to_room_none, ch, NULL, NULL, TO_ROOM, 0 );
		} else if((vch = get_char_room(ch, argument)) == NULL)
			send_to_char("They aren't here.\n\r", ch);
		else if(vch == ch) {
			if(!IS_NULLSTR(obj->to_user_self) )
			act( obj->to_user_self, ch, NULL, vch, TO_CHAR, 0 );
			if(!IS_NULLSTR(obj->to_room_self) )
			act( obj->to_room_self, ch, NULL, vch, TO_ROOM, 0 );
		}
		else {
			if(!IS_NULLSTR(obj->to_user_other) )
			act( obj->to_user_other, ch, NULL, vch, TO_CHAR, 0 );
			if(!IS_NULLSTR(obj->to_room_other) )
			act( obj->to_room_other, ch, NULL, vch, TO_NOTVICT, 0 );
			if(!IS_NULLSTR(obj->to_vict_other) )
			act( obj->to_vict_other, ch, NULL, vch, TO_VICT, 0 );
		}
		if(obj->uses > 0) obj->uses--;
		}
		else {
		if(!IS_NULLSTR(obj->to_user_used) )
			act( obj->to_user_used, ch, NULL, NULL, TO_CHAR, 0 );
		if(!IS_NULLSTR(obj->to_room_self) )
			act( obj->to_room_used, ch, NULL, NULL, TO_ROOM, 0 );
		}
	}
	return;
	}
	if((obj = get_obj_list(ch, arg, ch->in_room->contents)) != NULL) {
		if(obj->item_type == ITEM_PILL)
			send_to_char("You have to EAT a pill.\n\r", ch);
		else if(obj->item_type == ITEM_WEAPON)
			send_to_char("Try wielding it. (Then use the combat commands.)\n\r",ch);
		else if(obj->item_type == ITEM_TELEPHONE)
			send_to_char("Use the call command to make a call.\n\r", ch);
		else if(obj->to_user_none[0] == '\0' && obj->to_room_none[0] == '\0'
				&& obj->to_user_self[0] == '\0' && obj->to_room_self[0] == '\0'
						&& obj->to_user_other[0] == '\0' && obj->to_room_other[0] == '\0'
								&& obj->to_vict_other[0] == '\0') {
			send_to_char(Format("I'm not sure what you can use a %s for.\n\r",arg), ch);
		}
	else {
		if(obj->uses > 0 || obj->uses == -1) {
		if(IS_NULLSTR(argument))
		{
			if(!IS_NULLSTR(obj->to_user_none) )
			act( obj->to_user_none, ch, NULL, NULL, TO_CHAR, 0 );
			if(!IS_NULLSTR(obj->to_room_none))
			act( obj->to_room_none, ch, NULL, NULL, TO_ROOM, 0 );
		} else if((vch = get_char_room(ch, argument)) == NULL)
			send_to_char("They aren't here.\n\r", ch);
		else if(vch == ch) {
			if(!IS_NULLSTR(obj->to_user_self) )
			act( obj->to_user_self, ch, NULL, vch, TO_CHAR, 0 );
			if(!IS_NULLSTR(obj->to_room_self) )
			act( obj->to_room_self, ch, NULL, vch, TO_ROOM, 0 );
		}
		else {
			if(!IS_NULLSTR(obj->to_user_other) )
			act( obj->to_user_other, ch, NULL, vch, TO_CHAR, 0 );
			if(!IS_NULLSTR(obj->to_room_other) )
			act( obj->to_room_other, ch, NULL, vch, TO_NOTVICT, 0 );
			if(!IS_NULLSTR(obj->to_vict_other) )
			act( obj->to_vict_other, ch, NULL, vch, TO_VICT, 0 );
		}
		if(obj->uses > 0) obj->uses--;
		}
		else {
		if(!IS_NULLSTR(obj->to_user_used) )
			act( obj->to_user_used, ch, NULL, NULL, TO_CHAR, 0 );
		if(!IS_NULLSTR(obj->to_room_self) )
			act( obj->to_room_used, ch, NULL, NULL, TO_ROOM, 0 );
		}
	}
	return;
	}
	if((sn = stat_lookup(arg, ch)) >= 0)
	{
		send_to_char(Format("You can DROLL your %s, but there is no way to directly use it.\n\r", stat_table[sn].name), ch);
		send_to_char("The stats are generally used in other commands to provide the number of dice to roll.\n\r", ch);
		return;
	}
	if((sn = abil_lookup(arg, ch)) >= 0)
	{
		send_to_char(Format("You can DROLL your %s, but there may be no way to directly use it.\n\r", ability_table[sn].name), ch);
		send_to_char( "Checking for commands...\n\r", ch);
		send_to_char("Hassle Rayal to fix the command search.\n\r", ch);
		return;
	}
	if((sn = disc_lookup(ch, arg)) >= 0)
	{
		send_to_char( "Some powers have not yet been implemented, but generally if the help\n\r", ch);
		send_to_char( "exists, it will contain the commands related to the discipline or\n\r", ch);
		send_to_char("power group.\n\r", ch);
		return;
	}
	if(!str_prefix(arg, "disciplines"))
	{
		send_to_char("Type disciplines for your list of discipline dots.\n\r", ch);
		return;
	}
	if(!str_prefix(arg, "gifts"))
	{
		send_to_char("Type powers for a list of your basic gifts.\n\r", ch);
		return;
	}
	if(!str_prefix(arg, "skills"))
	{
		send_to_char("Your skills are in use each time you use a command.\n\r", ch);
		return;
	}
	if(command_lookup(arg) >= 0)
	{
		if((cmd = command_available(ch, arg)) >= 0 && CAN_USE_CMD(ch, cmd))
		{
			act("\tYActivating command: \tG$t\tn", ch, Format("%s %s", arg, argument), NULL, TO_CHAR, 0);
			interpret(ch, (char *)Format("%s %s", arg, argument));
			send_to_char( "\n\r\tYYou don't need to type use before this command.\tn\n\r", ch);
			return;
		}
		send_to_char("The command you are seeking to use does exist,\n\r", ch);
		send_to_char("but is not available for your use.", ch);
		return;
	}
	send_to_char("What are you trying to use?\n\r", ch);
	if(!IS_NULLSTR(arg))
	{
		log_string(LOG_BUG, (char *)Format("Request to use: %s (Argument: %s) by %s", arg, IS_NULLSTR(argument)?"None":argument, IS_NPC(ch)?"Mob":ch->name));
	}
}

void do_level(CHAR_DATA *ch, char *argument)
{
	CheckCH(ch);
	send_to_char("Levels have no meaning here.\n\r", ch);
}

void do_fight(CHAR_DATA *ch, char *argument)
{
	CheckCH(ch);
	send_to_char("You need to use the combat commands! (See HELP COMBAT)\n\r", ch);
}

void do_rent(CHAR_DATA *ch, char *argument)
{
	CheckCH(ch);
	send_to_char("Currently there is no need to rent anything.\n\r", ch);
	send_to_char("You can however buy a home...\n\r", ch);
}

void do_compare(CHAR_DATA *ch, char *argument)
{
	CheckCH(ch);
	send_to_char("You don't need to compare objects.\n\r", ch);
	send_to_char("Objects in PT are largely standardized.\n\r", ch);
}

void do_notgw(CHAR_DATA *ch, char *argument)
{
	CheckCH(ch);
	send_to_char("Repeat after me: Project Twilight is not based on GodWars.\n\r", ch);
}

void do_blood(CHAR_DATA *ch, char *argument)
{
	CheckCH(ch);
	send_to_char( Format("%s:  %d/%-20d\n\r", pc_race_table[ch->race].RBPG, ch->RBPG, ch->max_RBPG), ch );
}

void do_cash(CHAR_DATA *ch, char *argument)
{
	int num = 0;

	CheckCH(ch);

	if((num = ch->cents/100) > 0)
	{
		ch->dollars = ch->dollars + num;
		ch->cents = ch->cents - (100 * num);
	}

	send_to_char( Format("You have $ %d.%.2d.\n\r", ch->dollars, ch->cents), ch );
}

void do_frenzy(CHAR_DATA *ch, char *argument)
{
	CheckCH(ch);
	send_to_char("A frenzy is not really something you want to bring on.\n\r", ch);
}

void do_brief2(CHAR_DATA *ch, char *argument)
{
	do_help(ch, "brief");
}

void do_map(CHAR_DATA *ch, char *argument)
{
	do_help(ch, "map");
}

void do_totems(CHAR_DATA *ch, char *argument)
{
	PACK_DATA *pack;
	int i = 0, count = 0;

	CheckCH(ch);

	if(!str_prefix("list", argument))
	{
		for(i = 1; spirit_table[i].name != NULL; i++)
		{
			if (spirit_table[i].settable) {
				count++;
				send_to_char(Format("%10s", spirit_table[i].name), ch);
				if(count%4 == 0)
					send_to_char("\n\r", ch);
			}
		}
	}

	if(ch->clan > 1)
		send_to_char(Format("Your tribal totem is %s who feels %s towards you.\n\r", totem_table[clan_table[ch->clan].totem].name,
				totem_attitudes[ch->totem_attitudes[0]].name), ch);
	else
		send_to_char(Format("Not belonging to a tribe, you do not have a tribal totem.\n\r"), ch);

	if((pack = pack_lookup(ch->pack)) != NULL)
	{
		send_to_char(Format("Your pack's totem is %s and feels %s towards you.\n\r", flag_string(spirit_table, pack->totem),
				totem_attitudes[ch->totem_attitudes[1]].name), ch);
	}
}

void do_pack(CHAR_DATA *ch, char *argument)
{
	do_who(ch, (char *)Format("pack %s", argument));
}

void do_reinc(CHAR_DATA *ch, char *argument)
{
	CheckCH(ch);
	send_to_char("One must be dead before one can reincarnate.\n\r", ch);
}

void do_incumbent(CHAR_DATA *ch, char *argument)
{
	int j = 0;

	CheckCH(ch);

	send_to_char("\tW------------------------------------------------------------------------------\tn\n\r", ch );
	send_to_char("The current city officials are:\n\r", ch);
	act("\tGMayor:\tn $t", ch, incumbent_mayor, NULL, TO_CHAR, 1);

	send_to_char("\tGAldermen:\tn\n\r", ch);
	for(j = 0; j < MAX_ALDERMEN; j++)
		act("\t[U10148/*] $t\n\r", ch, incumbent_alder[j], NULL, TO_CHAR, 1);

	send_to_char("\tGJudges:\tn\n\r", ch);
	for(j = 0; j < MAX_JUDGES; j++)
		act("\t[U10148/*] $t\n\r", ch, incumbent_judge[j], NULL, TO_CHAR, 1);
	send_to_char("\tW------------------------------------------------------------------------------\tn\n\r", ch );
}

void do_suicide(CHAR_DATA *ch, char *argument)
{
	CheckCH(ch);
	send_to_char("Perhaps you're trying to DELETE?\n\r", ch);
	return;
}

/*
 * Sets time to house filename's file timestamp.
 */
int file_time( const char *filename, time_t *time )
{
	struct stat statbuf;

	if( stat( filename, &statbuf ) < 0 )
		return -1;

	*time = statbuf.st_mtime;

	return 0;
}

/*
 * Get time of last build.
 */
void do_updatetime( CHAR_DATA *ch, char *argument)
{
	bool found = FALSE;
	time_t time;

	CheckCH(ch);

	send_to_char("\tW------------------------------------------------------------------------------\tn\n\r", ch );
	send_to_char("\tYLast code update:\tn ", ch);

	if( file_time( "../src/project", &time ) < 0 ) {
		if( file_time( "../src/project.exe", &time ) < 0 )
			found = FALSE;
	}
	else
		found = TRUE;

	Assert(found = TRUE, "Project.exe file was not located.");

	if(found)
		send_to_char(ctime(&time), ch);
	else
		send_to_char(Format("\tRWARNING:\tn Unable to determine update time.\n\r"), ch);

	send_to_char("\tGEngine: ", ch);
	send_to_char(PT_ENGINE, ch);
	send_to_char("\tn\n\r", ch);
	send_to_char("\tYVersion: ", ch);
	send_to_char(mudVersion, ch);
	send_to_char("\tn\n\r", ch);
	send_to_char("\tW------------------------------------------------------------------------------\tn\n\r", ch);
}

void do_lines(CHAR_DATA *ch, char *argument)
{

	CheckCH(ch);

	if(IS_NULLSTR(argument))
	{
		send_to_char(Format("Your scroll is set to %d lines.\n\r", ch->lines), ch);
		return;
	}

	if(!is_number(argument))
	{
		send_to_char("Syntax: lines [number]\n\r", ch);
		send_to_char("(-1 for unlimited)\n\r", ch);
		return;
	}

	ch->lines = atoi(argument);
	send_to_char(Format("Your scroll is now set to %d lines.\n\r", ch->lines), ch);
}


void tip_to_char(CHAR_DATA *ch)
{
	HELP_DATA *tip;
	bool found = FALSE;
	BUFFER *output;

	output = new_buf();

	for(tip = tip_list; tip != NULL; tip = tip->next)
	{
		if(atoi(tip->keyword) == ch->current_tip)
		{
			add_buf(output, "\n\rTIP:\n\r\n\r");
			/*
			 * Strip leading '.' to allow initial blanks.
			 */
			if ( tip->unformatted[0] == '.' )
				add_buf(output,tip->unformatted+1);
			else
				add_buf(output,tip->unformatted);
			found = TRUE;
		}
	}

	if(!found)
	{
		ch->current_tip = 0;
	}
	else
	{
		page_to_char(buf_string(output),ch);
		ch->current_tip++;
	}
	free_buf(output);
}

void odd_parts_to_char(CHAR_DATA *ch, CHAR_DATA *vch)
{
	long oddflags = vch->parts;
	int i = 0;

	REMOVE_BIT(oddflags, race_table[vch->race].parts);

	if(ch->shape == SHAPE_WOLF)
		REMOVE_BIT(oddflags, race_table[race_lookup("wolf")].parts);
	if(ch->shape == SHAPE_BAT)
		REMOVE_BIT(oddflags, race_table[race_lookup("bat")].parts);

	act("\tYSomething's strange about $N. For starters $E has:\tn", ch, NULL, vch, TO_CHAR, 1);

	if(is_affected(vch, skill_lookup("eyes of the serpent")) && !IS_AFFECTED2(ch, AFF2_IMMOBILIZED))
	{
		send_to_char("\t[U10148/*] \tRgolden eyes with vertical slits\tn\n\r", ch);
	}

	if(IS_AFFECTED(vch, AFF_DARK_VISION))
	{
		send_to_char("\t[U10148/*] \tRglowing red eyes\tn\n\r", ch);
	}

	if(oddflags == 0)
			return;

	for(i=0;part_flags[i].name != NULL; i++)
	{
		if(IS_SET(oddflags, part_flags[i].bit))
		{
			send_to_char(Format("\t[U10148/*] \tR%s\tn\n\r", part_flags[i].name), ch);
		}
	}

	send_to_char("\n\r", ch);
}

void do_addy (CHAR_DATA *ch, char *argument)
{
	CheckCH(ch);

	send_to_char("Web Site:\n\r", ch);
	send_to_char(MYURL, ch);
	send_to_char("\n\r\n\rTelnet Location:\n\r", ch);
	send_to_char(MYSERV, ch);
	send_to_char("\n\r", ch);
}

void do_score_revised( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *user = ch;
	char buf[MSL]={'\0'};
	int num = 0, i = 0;
	int statcount = 0;

	CheckCH(ch);

	if(!IS_NULLSTR(argument) && IS_ADMIN(user))
	{
		if((ch = get_char_world(user, argument)) == NULL)
		{
			send_to_char("They aren't online.\r\n", user);
			return;
		}

		if(ch->trust > user->trust)
		{
			send_to_char("Not available for those with higher authority.\r\n", user);
			return;
		}
	}

	buf[0] = '\0';

	if(ch->race == race_lookup("human") && IS_SET(ch->act2, ACT2_GHOUL))
	{
		send_to_char( Format("\r\n\tW---------------------------------<\tG   Ghoul\tW>-----------------------------------\tn\r\n"), user);
	}
	else
	{
		send_to_char( Format("\n\r\tW---------------------------------<\tG%8s\tW>-----------------------------------\tn\n\r",
				capitalize(race_table[ch->race].name)), user);
	}

	/* snprintf(altname, sizeof(altname), " [%s]", !IS_NULLSTR(ch->alt_name) ? ch->alt_name : "" ); */

	send_to_char( Format("First Name: %-15s  ", IS_NPC(ch) ? ch->short_descr : ch->name ), user);
	if(ch->race == race_lookup("werewolf"))
		send_to_char( Format("Breed: %-15s  ",breed_table[ch->breed].name), user);
	else
		send_to_char( Format("Nature: %-15s  ", capitalize(ch->nature)), user);

	if(ch->race == race_lookup("werewolf"))
		send_to_char( "Pack Name: ", user);
	else
		send_to_char( Format("Clan: %-20s", capitalize(clan_table[ch->clan].name)), user);

	send_to_char("\r\n", user);

	send_to_char( Format("Last Name: %s%-15s  ", !IS_NULLSTR(ch->surname)? " " : " ", !IS_NULLSTR(ch->surname)? ch->surname : ""), user);

	if(ch->race == race_lookup("werewolf"))
		send_to_char( Format("Auspice: %-13s  ",auspice_table[ch->auspice].name), user);
	else
		send_to_char( Format("Demeanor: %-13s  ", capitalize(ch->demeanor)), user);

	if(ch->race == race_lookup("werewolf"))
		send_to_char( "Pack Totem: ", user);
	else
		send_to_char( Format("Generation: %d", ch->gen), user);

	send_to_char("\r\n", user);

	if(IS_ADMIN(user))
	{
		if(ch->trust > MAX_LEVEL || ch->trust < 0)
			ch->trust = 1;
		send_to_char( Format("\tWAdmin Level: %-14s  \tn", staff_status[ch->trust].name), user);
	}
	else
	{
		send_to_char(Format("%29s", " "), user);
	}

	if(ch->race == race_lookup("werewolf"))
		send_to_char( Format("Tribe: %-15s  ",capitalize(clan_table[ch->clan].name)), user);
	else
		send_to_char( Format("Profession: %-12s ", ch->profession), user);

	if(ch->race == race_lookup("werewolf"))
		send_to_char( Format("Profession: %-16s  ", ch->profession), user);
	else
		send_to_char( Format("Sire: %-16s  ", ch->sire), user);
   
	send_to_char("\r\n", user);

	send_to_char("\tW--------------------------------<\tGAttributes\tW>----------------------------------\tn\n\r", user);

	send_to_char ("Str: ", user);
	if (get_curr_stat(ch,STAT_STR)<=0)
	{
		send_to_char( "\t[U9675/O]", user);
		statcount=1;
		while (statcount < 10)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
	}
	else
	{
		while (statcount < get_curr_stat(ch,STAT_STR))
		{
			send_to_char( "\t[U9679/*]", user);
			statcount++;
		}
		while (statcount < 10)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
	}

	send_to_char (Format("%14sCHA: ", " "), user);
	statcount = 0;
	if (get_curr_stat(ch,STAT_CHA)<=0)
	{
		send_to_char( "\t[U9675/O]", user);
		statcount=1;
		while (statcount < 10)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
	}
	else
	{
		while (statcount < get_curr_stat(ch,STAT_CHA))
		{
			send_to_char( "\t[U9679/*]", user);
			statcount++;
		}
		while (statcount < 10)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
	}

	send_to_char (Format("%10sPER: ", " "), user);
	statcount = 0;
	if (get_curr_stat(ch,STAT_PER)<=0)
	{
		send_to_char( "\t[U9675/O]", user);
		statcount=1;
		while (statcount < 10)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
   }
	else
	{
		while (statcount < get_curr_stat(ch,STAT_PER))
		{
			send_to_char( "\t[U9679/*]", user);
			statcount++;
		}
		while (statcount < 10)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
	}
	send_to_char("\n\r", user);

	send_to_char ("DEX: ", user);
	statcount = 0;
	if (get_curr_stat(ch,STAT_DEX)<=0)
	{
		send_to_char( "\t[U9675/O]", user);
		statcount=1;
		while (statcount < 10)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
	}
	else
	{
		while (statcount < get_curr_stat(ch,STAT_DEX))
		{
			send_to_char( "\t[U9679/*]", user);
			statcount++;
		}
		while (statcount < 10)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
	}

	send_to_char (Format("%14sMAN: ", " "), user);
	statcount = 0;
	if (get_curr_stat(ch,STAT_MAN)<=0)
	{
		send_to_char( "\t[U9675/O]", user);
		statcount=1;
		while (statcount < 10)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
	}
	else
	{
		while (statcount < get_curr_stat(ch,STAT_MAN))
		{
			send_to_char( "\t[U9679/*]", user);
			statcount++;
		}
		while (statcount < 10)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
	}

	send_to_char (Format("%10sINT: ", " "), user);
	statcount = 0;
	if (get_curr_stat(ch,STAT_INT)<=0)
	{
		send_to_char( "\t[U9675/O]", user);
		statcount=1;
		while (statcount < 10)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
	}
	else
	{
		while (statcount < get_curr_stat(ch,STAT_INT))
		{
			send_to_char( "\t[U9679/*]", user);
			statcount++;
		}
		while (statcount < 10)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
	}
	send_to_char("\n\r", user);

	send_to_char ("STA: ", user);
	statcount = 0;
	if (get_curr_stat(ch,STAT_STA)<=0)
	{
		send_to_char( "\t[U9675/O]", user);
		statcount=1;
		while (statcount < 10)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
	}
	else
	{
		while (statcount < get_curr_stat(ch,STAT_STA))
		{
			send_to_char( "\t[U9679/*]", user);
			statcount++;
		}
		while (statcount < 10)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
	}

	send_to_char (Format("%14sAPP: ", " "), user);
	statcount = 0;
	if (get_curr_stat(ch,STAT_APP)<=0)
	{
		send_to_char( "\t[U9675/O]", user);
		statcount=1;
		while (statcount < 10)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
	}
	else
	{
		while (statcount < get_curr_stat(ch,STAT_APP))
		{
			send_to_char( "\t[U9679/*]", user);
			statcount++;
		}
		while (statcount < 10)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
	}

	send_to_char (Format("%10sWIT: ", " "), user);
	statcount = 0;
	if (get_curr_stat(ch,STAT_WIT)<=0)
	{
		send_to_char( "\t[U9675/O]", user);
		statcount=1;
				while (statcount < 10)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
	}
	else
	{
		while (statcount < get_curr_stat(ch,STAT_WIT))
		{
			send_to_char( "\t[U9679/*]", user);
			statcount++;
		}
		while (statcount < 10)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
	}
	statcount=0;
	send_to_char("\n\r", user);

	send_to_char("\tW--------------------------------<Dice Pools>----------------------------------\tn\n\r", user);

	send_to_char( Format("%s:  %2d/%-20d\n\r", race_table[ch->race].pc_race?pc_race_table[ch->race].RBPG:"Faith",
			ch->RBPG, ch->max_RBPG), user);

	send_to_char(Format("Experience/OOC Experience: %d / %d\n\r", ch->exp, ch->oocxp), user);
	send_to_char(Format("Experience to Gift: %d\n\r", ch->xpgift), user);

	if(IS_SET(ch->act2, ACT2_GHOUL))
	{
		send_to_char(Format("You are the devoted servant of %s.\r\n", ch->ghouled_by), user);
	}

	send_to_char("\tW------------------------------------------------------------------------------\tn\n\r", user);

	send_to_char( Format("%s\n\r", race_table[ch->race].pc_race?pc_race_table[ch->race].RBPG:"Faith"), ch);
	// Available RBPG pool
	if (ch->willpower<=0)	
	{
		while (statcount < ch->max_RBPG)
		{
			send_to_char( "\t[U9746/O]", user);
			statcount++;
		}
	}
	else
	{
		while (statcount < ch->RBPG)
		{
			send_to_char( "\t[U9744/*]", user);
			statcount++;
		}
		while (statcount < ch->max_RBPG)
		{
			send_to_char( "\t[U9746/O]", user);
			statcount++;
		}
	}
	statcount=0;
	send_to_char("\n\r", ch);



	send_to_char("\tW------------------------------------------------------------------------------\tn\n\r", user);

	send_to_char( Format("%8s (%2d)", race_table[ch->race].pc_race?pc_race_table[ch->race].GHB:"Humanity", ch->GHB), ch);
	send_to_char( Format("%15sConscience   ", " ", ch->virtues[0]), ch);
	
	if (ch->virtues[0]<=0)
	{
		send_to_char( "\t[U9675/O]", user);
		statcount=1;
				while (statcount < 5)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
	}
	else
	{
		while (statcount < ch->virtues[0])
		{
			send_to_char( "\t[U9679/*]", user);
			statcount++;
		}
		while (statcount < 5)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
	}
	statcount=0;
	send_to_char("\n\r", ch);

	if (ch->GHB<=0)
	{
		send_to_char( "\t[U9675/O]", user);
		statcount=1;
				while (statcount < 10)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
	}
	else
	{
		while (statcount < ch->GHB)
		{
			send_to_char( "\t[U9679/*]", user);
			statcount++;
		}
		while (statcount < 10)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
	}
	statcount=0;

	// Spacing for proper formatting
	send_to_char( Format("%18s", " "), ch);

	send_to_char( "Self-Control ", ch);
	if (ch->virtues[1]<=0)
	{
		send_to_char( "\t[U9675/O]", user);
		statcount=1;
				while (statcount < 5)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
	}
	else
	{
		while (statcount < ch->virtues[1])
		{
			send_to_char( "\t[U9679/*]", user);
			statcount++;
		}
		while (statcount < 5)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
	}
	statcount=0;
	send_to_char("\n\r", ch);

	send_to_char("Willpower ", ch);
	send_to_char( "(Resist ", ch);
	if (IS_SET(ch->act2, ACT2_RESIST))
	{
		send_to_char( Format("%3s)", "on"), ch);
	}
	else
	{
		send_to_char( Format("\t<send href='resist'>%3s\t</send>)", "off"), ch);
	}

	send_to_char( Format("%6s", " "), ch);
	send_to_char( "Courage      ", ch);
	if (ch->virtues[2]<=0)
	{
		statcount=1;
				while (statcount < 5)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
	}
	else
	{
		while (statcount < ch->virtues[2])
		{
			send_to_char( "\t[U9679/*]", user);
			statcount++;
		}
		while (statcount < 5)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
	}
	statcount=0;
	send_to_char("\n\r", ch);

	// Max willpower first
	if (ch->max_willpower<=0)	
	{
		send_to_char( "\t[U9675/O]", user);
		statcount=1;
				while (statcount < 10)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
	}
	else
	{
		while (statcount < ch->max_willpower)
		{
			send_to_char( "\t[U9679/*]", user);
			statcount++;
		}
		while (statcount < 10)
		{
			send_to_char( "\t[U9675/O]", user);
			statcount++;
		}
	}
	statcount=0;
	send_to_char("\n\r", ch);

	// Available willpower
	if (ch->willpower<=0)	
	{
		send_to_char( "\t[U9746/O]", user);
		statcount=1;
		while (statcount < ch->max_willpower)
		{
			send_to_char( "\t[U9746/O]", user);
			statcount++;
		}
	}
	else
	{
		while (statcount < ch->willpower)
		{
			send_to_char( "\t[U9744/*]", user);
			statcount++;
		}
		while (statcount < ch->max_willpower)
		{
			send_to_char( "\t[U9746/O]", user);
			statcount++;
		}
	}
	statcount=0;

	
	send_to_char("\n\r", ch);
	send_to_char("\tW--------------------------------<\tGBackgrounds\tW>---------------------------------\tn\n\r", user);

	i = 0;
	for(num=0; background_table[num].name; num++)
	{
		if(background_table[num].settable)
		{
			if(num < MAX_BG)
			{
				send_to_char(Format("\t<send href='help %s'>%-11s\t</send>:", background_table[num].name, background_table[num].name), user);
				if(ch->backgrounds[num]<=0)
				{
					send_to_char("\t[U9675/O]", user);
					statcount = 1;
					while (statcount < 5)
					{
						send_to_char("\t[U9675/O]", user);
						statcount++;
					}
				}
				else
				{
					while(statcount < ch->backgrounds[num])
					{
						send_to_char("\t[U9679/*]", user);
						statcount++;
					}
					while (statcount < 5)
					{
						send_to_char("\t[U9675/O]", user);
						statcount++;
					}

				}
				send_to_char("\r\n", user);
				statcount = 0;
				i++;
			}
		}
	}

	send_to_char("\tW---------------------------------<\tGInfluences\tW>---------------------------------\tn\n\r", user);

	for(num=0;influence_table[num].name;num++)
	{
		send_to_char(Format("%11s:%4d ", influence_table[num].name, ch->influences[num]), user);
		if((num+1)%4 == 0)
			send_to_char("\n\r", user);
	}
	send_to_char("\n\r", user);
}

void do_testceleb(CHAR_DATA *ch, char *argument)
{
	CheckCH(ch);

	send_to_char("\tWCelebrities currently online are:\tn\n\r", ch);
	send_to_char(Format("\tYAllies->%15s - %s\tn\n\r", ch->name, fame_table[ch->backgrounds[ALLIES]].name), ch);
	send_to_char(Format("\tYPurebreed->%15s - %s\tn\n\r", ch->name, fame_table[ch->backgrounds[PUREBREED]].name), ch);
	send_to_char(Format("\tYContacts->%15s - %s\tn\n\r", ch->name, fame_table[ch->backgrounds[CONTACTS]].name), ch);
	send_to_char(Format("\tYFetishes->%15s - %s\tn\n\r", ch->name, fame_table[ch->backgrounds[FETISHES]].name), ch);
	send_to_char(Format("\tYGeneration->%15s - %s\tn\n\r", ch->name, fame_table[ch->backgrounds[GENERATION]].name), ch);
	send_to_char(Format("\tYHerd->%15s - %s\tn\n\r", ch->name, fame_table[ch->backgrounds[HERD]].name), ch);
	send_to_char(Format("\tYMentor->%15s - %s\tn\n\r", ch->name, fame_table[ch->backgrounds[MENTOR]].name), ch);
	send_to_char(Format("\tYPast life->%15s - %s\tn\n\r", ch->name, fame_table[ch->backgrounds[PASTLIFE]].name), ch);
	send_to_char(Format("\tYResources->%15s - %s\tn\n\r", ch->name, fame_table[ch->backgrounds[RESOURCES]].name), ch);
	send_to_char(Format("\tYRetainers->%15s - %s\tn\n\r", ch->name, fame_table[ch->backgrounds[RETAINERS]].name), ch);
	send_to_char(Format("\tYClan Status->%15s - %s\tn\n\r", ch->name, fame_table[ch->backgrounds[CLAN_STATUS]].name), ch);
	send_to_char(Format("\tYFame->%15s - %s\tn\n\r", ch->name, fame_table[ch->backgrounds[FAME_STATUS]].name), ch);
	send_to_char(Format("\tYRace Status->%15s - %s\tn\n\r", ch->name, fame_table[ch->backgrounds[RACE_STATUS]].name), ch);

	// for(vch = char_list; vch != NULL; vch = vch->next)
	// {
	//     if(IS_NPC(vch) || vch->backgrounds[FAME_STATUS] == 0) continue;

	//     send_to_char(Format("\tY%15s - %s\tn\n\r", vch->name, fame_table[vch->backgrounds[FAME_STATUS]].name), ch);
	// }
}

const char *styleBackgrounds(CHAR_DATA *ch, int num) 
{

    int statcount = 0;
    static char            ostr  [ MSL ]={'\0'};

    strncpy(ostr, "", MSL);

    if(ch->desc && ch->desc->pProtocol && ch->desc->pProtocol->pVariables[eMSDP_UTF_8]->ValueInt == 1 ) 
    {
        if(ch->backgrounds[num]<=0)
        {
            snprintf(ostr, MSL, "\t[U9675/O]");
            statcount = 1;
            while (statcount < 5)
            {
                strncat(ostr, "\t[U9675/O]", MSL);
                statcount++;
            }
        }
        else
        {
            while(statcount < ch->backgrounds[num])
            {
                strncat(ostr, "\t[U9679/*]", MSL);
                statcount++;
            }
            while (statcount < 5)
            {
                strncat(ostr, "\t[U9675/O]", MSL);
                statcount++;
            }
        }
    } 
    else 
    {
        if(ch->backgrounds[num]<=0)
        {
            snprintf(ostr, MSL, "O");
            statcount = 1;
            while (statcount < 5)
            {
                strncat(ostr, "O", MSL);
                statcount++;
            }
        }
        else
        {
            while(statcount < ch->backgrounds[num])
            {
                strncat(ostr, "*", MSL);
                statcount++;
            }
            while (statcount < 5)
            {
                strncat(ostr, "O", MSL);
                statcount++;
            }
        }
    }
    statcount = 0;
    return ostr;
}

void do_charsheet( CHAR_DATA *ch, char *argument )
{
	GRID_DATA *grid;
	GRID_ROW *row;
	GRID_CELL *cell;
	char name[MSL]={'\0'};
	int num = 0;
	int sn = 0;
	int i = 0;
	int statcount = 0;
	CheckCH(ch);

	if(IS_ADMIN(ch))
	{
		if(ch->trust > MAX_LEVEL || ch->trust < 0)
			ch->trust = 1;
	}

	grid = create_grid(75);
	row = create_row(grid);
	cell = row_append_cell(row, 27, "%-10s: %-15s\n%-10s: %s%-15s\n%-10s %-14s ", "First Name",IS_NPC(ch) ? ch->short_descr : ch->name, "Last Name",!IS_NULLSTR(ch->surname)? "" : "", !IS_NULLSTR(ch->surname)? ch->surname : "",
					   IS_ADMIN(ch) ? "Admin Level:" : "", IS_ADMIN(ch) ? staff_status[ch->trust].name : "");

	cell = row_append_cell(row, 27, "%-8s: ", IS_WEREWOLF(ch) ? "Breed" : "Nature");
	if (ch->race == race_lookup("werewolf"))
		cell_append_contents(cell, "%-14s\n", (breed_table[ch->breed].name));
	else
		cell_append_contents(cell, "%-14s\n", (capitalize(ch->nature)));
	cell_append_contents(cell, "%-8s: ", IS_WEREWOLF(ch) ? "Auspice" : "Demeanor");
	if(ch->race == race_lookup("werewolf"))
		cell_append_contents(cell, "%-14s",auspice_table[ch->auspice].name);
	else
		cell_append_contents(cell, "%-14s", capitalize(ch->demeanor));
	cell_append_contents(cell, "Profession/Sire: %s", "test");

	cell = row_append_cell(row, 21, "%-5s: %s\n", IS_WEREWOLF(ch) ? "Tribe" : "Clan", capitalize(clan_table[ch->clan].name));
	cell_append_contents(cell, "%-10s: ", IS_WEREWOLF(ch) ? "Totem" : "Generation");
	 if(ch->race == race_lookup("werewolf"))
        cell_append_contents(cell, "%s\n", "N/A");
    else
        cell_append_contents(cell, "%d\n", ch->gen);
	
	cell_append_contents(cell, "%-10s: %s", "Profession", ch->profession);

	grid_to_char(grid, ch, TRUE);

	for(sn = 0; disc_table[sn].vname != NULL; sn++)
	{
		if(sn != DISC_OBEAH) 
		{
			if(ch->disc[sn] != 0)
			{
				snprintf(name, sizeof(name), "%s", disc_table[sn].vname);
				send_to_char(Format("\t<send href='help %s'>%-15s\t</send> %-2d     ", capitalize(name),capitalize(name), ch->disc[sn]), ch);

				i = 0;
				for(num=0; background_table[num].name; num++)
				{
					if(background_table[num].settable)
					{
						if(ch->backgrounds[num] != 0)
						{
							send_to_char(Format("\t<send href='help %s'>%-11s\t</send>:", background_table[num].name, background_table[num].name), ch);
							if(ch->backgrounds[num]<=0)
							{
								send_to_char("\t[U9675/O]", ch);
								statcount = 1;
								while (statcount < 5)
								{
									send_to_char("\t[U9675/O]", ch);
									statcount++;
								}
							}
							else
							{
								while(statcount < ch->backgrounds[num])
								{
									send_to_char("\t[U9679/*]", ch);
									statcount++;
								}
								while (statcount < 5)
								{
									send_to_char("\t[U9675/O]", ch);
									statcount++;
								}

							}
							send_to_char("\r\n", ch);
							statcount = 0;
							i++;
						}
					}
				}
			}

		}
	}
}

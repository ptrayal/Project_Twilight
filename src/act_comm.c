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

/* Last Edited by Rayal */

#if defined(Macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#include "twilight.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
#include "interp.h"



/* RT code to delete yourself */
void do_delete( CHAR_DATA *ch, char *argument)
{

	CheckCH(ch);
	CheckChNPC(ch);

	if(ch->desc->editor)
	{
		send_to_char("You can't delete yourself while in an editor.\r\n", ch);
		return;
	}

	if (ch->pcdata->confirm_delete)
	{
		if (!IS_NULLSTR(argument))
		{
			send_to_char("Delete status removed.\r\n",ch);
			ch->pcdata->confirm_delete = FALSE;
			return;
		}
		else
		{
			wiznet("\tY[WIZNET]\tn $N turns $Mself into line noise.",ch,NULL,0,0,0);
			stop_fighting(ch,TRUE);
			clear_orgs(ch);
			do_function(ch, &do_quit, "");
			unlink((char *)Format("%s%s", PLAYER_DIR, capitalize( ch->name )));
			return;
		}
	}

	if (!IS_NULLSTR(argument))
	{
		send_to_char("Just type delete. No argument.\r\n",ch);
		return;
	}

	send_to_char("Type delete again to confirm this command.\r\n",ch);
	send_to_char("\tYWARNING: this command is irreversible.\tn\r\n",ch);
	send_to_char("Typing delete with an argument will undo delete status.\r\n", ch);
	ch->pcdata->confirm_delete = TRUE;
	wiznet("\tY[WIZNET]\tn $N is contemplating deletion.",ch,NULL,0,0,get_trust(ch));
}
		
/*
 * Translate statement into other "language".
 */
void translate( CHAR_DATA *ch, char * oldwords, char * newwords )
{
	char buf  [4*MSL]={'\0'};
	char *pName;
	int iSyl = 0;
	int length = 0;

	struct syl_type
	{
		char *  old;
		char *  bnew;
		char * wolf;
		char *  bat;
	};

	static const struct syl_type syl_table[] =
	{
	{ " ",		" ",		" ",		" "		},
	{ "ar",		"abra",		"ow",		"eek"		},
	{ "au",		"kada",		"ow",		"ea"		},
	{ "bless",	"fido",		"arooo",	"skeel"		},
	{ "blind",	"nose",		"arf",		"shree"		},
	{ "bur",	"mosa",		"grow",		"skeeeah"	},
	{ "cu",		"judi",		"-cough-",	"ee"		},
	{ "de",		"oculo",	"rl",		"kea"		},
	{ "en",		"unso",		"snrl",		"k"		},
	{ "light",	"dies",		"sn",		"kreeek"	},
	{ "lo",		"hi",		"arl",		"krae"		},
	{ "mor",	"zak",		"arr",		"squee"		},
	{ "move",	"sido",		"rrrrrr",	"skeeah"	},
	{ "ness",	"lacri",	"owwwwl",	"keep"		},
	{ "ning",	"illa",		"owl",		"eeeyee"	},
	{ "per",	"duda",		"arrr",		"pek"		},
	{ "ra",		"gru",		"grr",		"ea"		},
	{ "fresh",	"ima",		"howwwwwwl",	"squeak"	},
	{ "re",		"candus",	"arooo",	"quee"		},
	{ "son",	"sabru",	"owl",		"shreek"	},
	{ "tect",	"infra",	"owl",		"kreep"		},
	{ "the",	"trak",		"grr",		"skree"		},
	{ "tri",	"cula",		"grrr",		"reeic"		},
	{ "ven",	"nofo",		"grar",		"sque"		},
	{ "a", "a", "a", "a" }, { "b", "b", "gr", "e" },
	{ "c", "q", "graw", "quea" }, { "d", "e", "hooo", "eak" },
	{ "e", "z", "ow", "ee" }, { "f", "y", "owl", "k" },
	{ "g", "o", "gr", "squ" }, { "h", "p", "how", "k" },
	{ "i", "u", "ow", "ee" }, { "j", "y", "owoo", "eak" },
	{ "k", "t", "grrr", "eek" }, { "l", "r", "owl", "squea" },
	{ "m", "w", "ooow", "squee" }, { "n", "i", "grr", "ee" },
	{ "o", "a", "oooo", "eep" }, { "p", "s", "owl", "squ" },
	{ "q", "d", "ow", "sque" }, { "r", "f", "rrr", "ee" },
	{ "s", "g", "sna", "squ" }, { "t", "h", "grr", "k" },
	{ "u", "j", "ow", "ea" }, { "v", "z", "sna", "eep" },
	{ "w", "x", "owl", "eak" }, { "x", "n", "h", "skree" },
	{ "y", "l", "owoo", "ee" }, { "z", "k", "sna", "k" },
	{ "", "", "", "" }
	};

	buf[0]      = '\0';
	for ( pName = oldwords; *pName != '\0'; pName += length )
	{
	for ( iSyl = 0; (length = strlen(syl_table[iSyl].old)) != 0; iSyl++ )
	{
		if ( !str_prefix( syl_table[iSyl].old, pName ) )
		{
		if(ch->shape == SHAPE_WOLF)
			strncat( buf, syl_table[iSyl].wolf, sizeof(buf) );
		else if(ch->shape == SHAPE_BAT)
			strncat( buf, syl_table[iSyl].bat, sizeof(buf) );
/*	        strncat( buf, syl_table[iSyl].bnew, sizeof(buf) ); */
		else strncat( buf, syl_table[iSyl].old, sizeof(buf) );
			break;
		}
	}

	if ( length == 0 )
		length = 1;
	}

	snprintf( newwords, sizeof(newwords), "%s", buf );

	/* Removed to try to fix name changes.
	return str_dup(buf2);
	*/
	return;
}

/*
 * Stuff to discreetify a string with messages in "'s inside.
 * (handles multiple messages)
 * For use in emotes mostly.
 */
void discreetify( char *argument, char *discreet )
{
	char *letter, *endbit;
	char temp[MSL]={'\0'};

	if ((letter = strstr(argument,"\"")) == NULL)
	{
		strncpy(discreet, argument, sizeof(*discreet));
		return;
	}
	if ((endbit = strstr(letter + 1,"\"")) == NULL)
	{
		strncpy(discreet, argument, sizeof(*discreet));
		return;
	}

	endbit++;

	strncpy(temp,argument, sizeof(temp));
	temp[strlen(argument) - strlen(letter)] = '\0';
	strncat(temp, "\"something you can't make out\"", sizeof(temp));

	if(strstr(endbit, "\""))
		discreetify(endbit, endbit);
	strncat(temp, endbit, sizeof(temp));

	/* Removed to try to fix name changes.
	argument = str_dup(temp);

	return argument;
	 */
	strncpy(discreet, temp, sizeof(*discreet));

	return;
}


/*
 * Stuff to add say colouring to a string with messages in "'s inside.
 * (handles multiple messages)
 * For use in emotes.
 */
void emote_say_colouring( char *argument, CHAR_DATA *ch )
{
	char *letter, *endbit;
	char temp[MSL]={'\0'};
	char middle[MSL]={'\0'};

	if ((letter = strstr(argument,"\"")) == NULL)
		return;
	if ((endbit = strstr(letter + 1,"\"")) == NULL)
		return;

	endbit++;

	strncpy(temp,argument, sizeof(temp));
	temp[strlen(argument) - strlen(letter)] = '\0';

	strncpy(middle,letter, sizeof(middle));
	middle[strlen(letter) - strlen(endbit)] = '\0';

	strncat(temp, "\tW", sizeof(temp));

	strncat(temp, middle, sizeof(temp));

	if(strstr(endbit, "\""))
		emote_say_colouring(endbit, ch);
	strncat(temp, endbit, sizeof(temp));

	/* Removed to try to fix name changes.
	argument = str_dup(temp);

	return argument;
	 */
	strncpy(argument, temp, sizeof(*argument));

	return;
}

/* RT code to display channel status */

void do_channels( CHAR_DATA *ch, char *argument)
{

	CheckCH(ch);

	/* lists all channels and their status */
	send_to_char("\tW|   \tYchannel\tW     | \tYstatus\tW|\tn\n\r",ch);
	send_to_char("\tW|---------------|-------|\tn\n\r",ch);

	if (IS_ADMIN(ch))
	{
		send_to_char("\tW|\tnImmtalk channel", ch);
		if(!IS_SET(ch->comm,COMM_NOWIZ))
			send_to_char("\tW|\tn \tGON\tn    \tW|\tn\n\r",ch);
		else
			send_to_char("\tW|\tn \tROFF\tn   \tW|\tn\n\r",ch);

		send_to_char("\tW|\tnThink channel  ",ch);
		if(!IS_SET(ch->comm,COMM_THINK_ON))
			send_to_char("\tW|\tn \tGON\tn    \tW|\tn\n\r",ch);
		else
			send_to_char("\tW|\tn \tROFF\tn   \tW|\tn\n\r",ch);
	}

	send_to_char("\tW|\tnOOC Channel    ",ch);
	if (!IS_SET(ch->comm,COMM_OOC_OFF))
		send_to_char("\tW|\tn \tGON\tn    \tW|\tn\n\r",ch);
	else
		send_to_char("\tW|\tn \tROFF\tn   \tW|\tn\n\r",ch);

	send_to_char("\tW|\tnDiscreet mode  ",ch);
	if (IS_SET(ch->comm,COMM_DISCREET))
		send_to_char("\tW|\tn \tGON\tn    \tW|\tn\n\r",ch);
	else
		send_to_char("\tW|\tn \tROFF\tn   \tW|\tn\n\r",ch);

	send_to_char("\tW|\tnQuiet mode     ",ch);
	if (IS_SET(ch->comm,COMM_QUIET))
		send_to_char("\tW|\tn \tGON\tn    \tW|\tn\n\r",ch);
	else
		send_to_char("\tW|\tn \tROFF\tn   \tW|\tn\n\r",ch);

	send_to_char("\tW|\tnTelephone/Deaf ",ch);
	if (!IS_SET(ch->comm,COMM_NOPHONE))
		send_to_char("\tW|\tn \tGON\tn    \tW|\tn\n\r",ch);
	else
		send_to_char("\tW|\tn \tROFF\tn   \tW|\tn\n\r",ch);

	send_to_char("\tW|\tnYelling        ",ch);
	if (!IS_SET(ch->comm,COMM_NOGOSSIP))
		send_to_char("\tW|\tn \tGON\tn    \tW|\tn\n\r",ch);
	else
		send_to_char("\tW|\tn \tROFF\tn   \tW|\tn\n\r",ch);

	send_to_char("\tW|\tnTips           ",ch);
	if (IS_SET(ch->comm,COMM_TIPS))
		send_to_char("\tW|\tn \tGON\tn    \tW|\tn\n\r",ch);
	else
		send_to_char("\tW|\tn \tROFF\tn   \tW|\tn\n\r",ch);

	if(ch->played < 60*60*10 || IS_ADMIN(ch))
	{
		send_to_char("\tW|\tnHelpline       \tW|\tn \tGON    \tW|\tn\n\r", ch);
	}

	if (!IS_NPC(ch) && ch->pcdata->security > 0)
	{
		send_to_char("\tW|\tnBuilder channel\tW|\tn \tGON    \tW|\tn\n\r", ch);
	}

	send_to_char("\n\r", ch);

	if (ch->lines != PAGELEN)
	{
		if (ch->lines)
		{
			send_to_char(Format("You display %d lines of scroll.\n\r",ch->lines+2),ch);
		}
		else
			send_to_char("Scroll buffering is off.\n\r",ch);
	}

	if (ch->prompt != NULL)
	{
		send_to_char(Format("Your current prompt is: %s\n\r",ch->prompt),ch);
	}

	if (IS_SET(ch->comm,COMM_NOSHOUT))
		send_to_char("You cannot shout.\n\r",ch);

	if (IS_SET(ch->comm,COMM_NOTELL))
		send_to_char("You cannot use a phone.\n\r",ch);

	if (IS_SET(ch->comm,COMM_NOCHANNELS))
		send_to_char("You cannot use channels.\n\r",ch);

	if (IS_SET(ch->comm,COMM_NOEMOTE))
		send_to_char("You cannot show emotions.\n\r",ch);

	if (IS_SET(ch->comm,COMM_AFK))
		send_to_char("You are AFK.\n\r",ch);
}

/* Toggle comm flags */
void do_toggle( CHAR_DATA *ch, char *argument )
{
	int flag = 0;

	CheckCH(ch);

	if(IS_NULLSTR(argument))
	{
		do_channels(ch, argument);
		return;
	}

	if((flag = flag_lookup2(argument, comm_flags)) == NO_FLAG)
	{
		if(!IS_ADMIN(ch)
				|| (flag = flag_lookup2(argument, admin_comm_flags)) == NO_FLAG)
		{
			send_to_char("No such flag.\n\r", ch);
			return;
		}
	}

	if(IS_SET(ch->comm, flag))
	{
		REMOVE_BIT(ch->comm, flag);
		send_to_char(Format("You turn off %s.\n\r", flag_string(comm_flags, flag)), ch);
	}
	else
	{
		SET_BIT(ch->comm, flag);
		send_to_char(Format("You turn on %s.\n\r", flag_string(comm_flags, flag)), ch);
	}
}

/* RT deaf blocks out all shouts */

void do_phone( CHAR_DATA *ch, char *argument)
{

	CheckCH(ch);

   if (IS_SET(ch->comm,COMM_NOPHONE))
   {
	 send_to_char("You turn on your phone.\n\r",ch);
	 REMOVE_BIT(ch->comm,COMM_NOPHONE);
   }
   else 
   {
	 send_to_char("You turn off your phone.\n\r",ch);
	 SET_BIT(ch->comm,COMM_NOPHONE);
   }
}

/* RT quiet blocks out all communication */

void do_quiet ( CHAR_DATA *ch, char * argument)
{
	CheckCH(ch);

	if (IS_SET(ch->comm,COMM_QUIET))
	{
	  send_to_char("Quiet mode removed.\n\r",ch);
	  REMOVE_BIT(ch->comm,COMM_QUIET);
	}
   else
   {
	 send_to_char("From now on, you will only hear says and emotes.\n\r",ch);
	 SET_BIT(ch->comm,COMM_QUIET);
   }
}

/* RT tips turns the tip feed on or off */

void do_tips( CHAR_DATA *ch, char *argument)
{
	CheckCH(ch);

	send_to_char("\n\r", ch);
	send_to_char("\tWSo, you need a tip?  Ok, why don't you try these\tn.\n\r\n\r", ch);

	if(ch->bg_count < 5)
		send_to_char( Format("\t[U10148/*] \tOYou should spend some of your free backgrounds.  You have %d you could spend\tn.\n\r", (5 - ch->bg_count)), ch );

	if(ch->exp < 5)
		{
			send_to_char( Format ("\t[U10148/*] You should try \t<send href='help %s'>%-11s\t</send>.  Investigating uses your skills and thus may not always be successful.  I would start by \t(investigate %s\t).  Then try investigating other topics.\n\r", "investigate", "investigating", clan_table[ch->clan].name), ch);
		}

	if(ch->exp < 5)
	{
		send_to_char( "\t[U10148/*] Lost?  Try \t(step newbie school\t) to get back to the newbie school.\n\r", ch);
	}

	// Brandon - This is the old tips command.
	// if (IS_SET(ch->comm,COMM_TIPS))
	// {
	// 	send_to_char("You turn off the tip feed.\n\r",ch);
	// 	REMOVE_BIT(ch->comm,COMM_TIPS);
	// }
	// else 
	// {
	// 	send_to_char("You turn on the tip feed.\n\r",ch);
	// 	SET_BIT(ch->comm,COMM_TIPS);
	// }
}

/* No gossiping in PT, just yelling. */
void do_yell( CHAR_DATA *ch, char *argument )
{
	DESCRIPTOR_DATA *d;

	CheckCH(ch);

	if (IS_NULLSTR(argument) )
	{
		if (IS_SET(ch->comm,COMM_NOGOSSIP))
		{
			send_to_char("You can hear the yelling of others.\n\r",ch);
			REMOVE_BIT(ch->comm,COMM_NOGOSSIP);
		}
		else
		{
			send_to_char("You block your ears to stop the yelling.\n\r",ch);
			SET_BIT(ch->comm,COMM_NOGOSSIP);
		}
	}
	else  /* gossip message sent, turn gossip on if it isn't already */
	{
		if (IS_SET(ch->comm,COMM_QUIET))
		{
			send_to_char("You must turn off quiet mode first.\n\r",ch);
			return;
		}

		if (IS_SET(ch->comm,COMM_NOCHANNELS))
		{
			send_to_char("You have been muted.\n\r",ch);
			return;

		}

		if (strlen(argument) >= (MSL/2))
		{
			send_to_char("Your yell is too long.  Try a shorter yell.\n\r", ch);
			return;
		}


		REMOVE_BIT(ch->comm,COMM_NOGOSSIP);

		send_to_char( Format("You yell '%s'\n\r", argument), ch );
		for ( d = descriptor_list; d != NULL; d = d->next )
		{
			CHAR_DATA *victim;
			char buf2[4*MSL]={'\0'};


			victim = d->original ? d->original : d->character;

			if ( victim == NULL || IS_SET(victim->comm, COMM_DEAF) )
				continue;

			if ( d->connected == CON_PLAYING &&
					d->character != ch &&
					!IS_SET(victim->comm,COMM_NOGOSSIP)	&&
					!IS_SET(victim->comm,COMM_QUIET)		&&
					victim->in_room->area == ch->in_room->area &&
					!IS_SET(victim->comm,COMM_OLC)		&&
					can_comprehend(victim, ch) )
			{
				act_new( "$n yells '$t'", ch,argument, d->character, TO_VICT,P_SLEEP, 0 );
				if(IS_AFFECTED(victim, AFF_HSENSES)
						&& ch->in_room == victim->in_room
						&& victim->shape <= SHAPE_HUMAN
						&& !IS_ADMIN(victim)
						&& SAME_PLANE(ch, victim))
				{
					send_to_char("Your ears bleed as you go deaf.\n\r", d->character);
					SET_BIT(victim->comm, COMM_DEAF);
				}
			}
			else if ( d->connected == CON_PLAYING &&
					d->character != ch &&
					!IS_SET(victim->comm,COMM_NOGOSSIP)        &&
					!IS_SET(victim->comm,COMM_QUIET)           &&
					victim->in_room->area == ch->in_room->area &&
					!IS_SET(victim->comm,COMM_OLC) )
			{
				translate(ch, argument, buf2);
				act_new( "$n yells '$t'", ch, buf2, d->character, TO_VICT, P_SLEEP, 0 );
				if(IS_AFFECTED(ch, AFF_HSENSES)
						&& ch->in_room == victim->in_room
						&& victim->shape <= SHAPE_HUMAN
						&& !IS_ADMIN(victim)
						&& SAME_PLANE(ch, victim))
				{
					send_to_char("Your ears bleed as you go deaf.\n\r", d->character);
					SET_BIT(victim->comm, COMM_DEAF);
				}
			}
		}

		if(IS_AFFECTED(ch, AFF_HIDE) || IS_AFFECTED(ch, AFF_INVISIBLE) || IS_AFFECTED(ch, AFF_SNEAK))
			do_visible(ch,"");

	}
}

/* Call of the Wyld. Galliard WW use only, understood by all WWs */
void do_callwyld( CHAR_DATA *ch, char *argument )
{
	DESCRIPTOR_DATA *d;

	CheckCH(ch);
 
	  REMOVE_BIT(ch->comm,COMM_NOGOSSIP);

	  if(ch->auspice != auspice_lookup("galliard") || ch->disc[DISC_AUSPICE] < 2)
	  {
	send_to_char("Huh?\n\r", ch);
	return;
	  }

	  if(ch->power_timer > 0)
	  {
		send_to_char("Your powers have not rejuvenated yet.\n\r", ch);
		return;
	  }
 
	  send_to_char( Format("You howl with the strength of the Wyld '%s'\n\r", argument), ch );
	  for ( d = descriptor_list; d != NULL; d = d->next )
	  {
		CHAR_DATA *victim;

		victim = d->original ? d->original : d->character;

		if ( d->connected == CON_PLAYING &&
			 d->character != ch &&
			 !IS_SET(victim->comm,COMM_NOGOSSIP)	&&
			 !IS_SET(victim->comm,COMM_QUIET)		&&
		 !IS_SET(victim->comm,COMM_OLC)		&&
		 (victim->race == race_lookup("werewolf")   ||
		 IS_ADMIN(victim)) )
		{
		  act_new( "$n howls with the strength of the Wyld '$t'", ch,argument, d->character, TO_VICT, P_SLEEP, 0 );
		}
		else if ( d->connected == CON_PLAYING &&
			 d->character != ch &&
			 !IS_SET(victim->comm,COMM_NOGOSSIP)	&&
			 !IS_SET(victim->comm,COMM_QUIET)		&&
		 !IS_SET(victim->comm,COMM_OLC)		&&
		 victim->in_room == ch->in_room		&&
		 victim->race != race_lookup("werewolf") )
		{
		  act_new("$n howls loudly!",ch,NULL,d->character,TO_VICT,P_SLEEP,0);
		}
	else
		{
		  act_new("Something howls in the distance!", ch,NULL,d->character,TO_VICT,P_SLEEP,0);
		}
	  }

	ch->power_timer = 1;
}


/* clan channels */
void do_clantalk( CHAR_DATA *ch, char *argument )
{
	DESCRIPTOR_DATA *d;

	CheckCH(ch);

	if (!is_clan(ch) || clan_table[ch->clan].independent)
	{
		send_to_char("You aren't in a clan.\n\r",ch);
		return;
	}
	if ( IS_NULLSTR(argument) )
	{
		if (IS_SET(ch->comm,COMM_NOCLAN))
		{
			send_to_char("Clan channel is now ON\n\r",ch);
			REMOVE_BIT(ch->comm,COMM_NOCLAN);
		}
		else
		{
			send_to_char("Clan channel is now OFF\n\r",ch);
			SET_BIT(ch->comm,COMM_NOCLAN);
		}
		return;
	}

	if (IS_SET(ch->comm,COMM_NOCHANNELS))
	{
		send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
		return;
	}

	REMOVE_BIT(ch->comm,COMM_NOCLAN);

	send_to_char( Format("You clan '%s'\n\r", argument), ch );
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
		if ( d->connected == CON_PLAYING &&
				d->character != ch &&
				is_same_clan(ch,d->character) &&
				!IS_SET(d->character->comm,COMM_NOCLAN) &&
				!IS_SET(d->character->comm,COMM_QUIET) )
		{
			act_new("$n clans '$t'",ch,argument,d->character,TO_VICT,P_DEAD,1);
		}
	}

	return;
}

void do_buildertalk( CHAR_DATA *ch, char *argument )
{
	DESCRIPTOR_DATA *d;

	CheckCH(ch);

	if((!ch->pcdata || ch->pcdata->security <= 0) && !IS_ADMIN(ch))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	act_new("\tY[Builder]\tn $n: $t",ch,argument,NULL,TO_CHAR,P_DEAD,1);

	for ( d = descriptor_list; d != NULL; d = d->next )
	{
		if ( d->connected == CON_PLAYING &&
				(IS_ADMIN(d->character) ||
						d->character->pcdata->security > 0))
		{
			act_new("\tY[Builder]\tn $n: $t",ch,argument,d->character,TO_VICT, P_DEAD,1);
		}
	}

	return;
}

void do_helpline( CHAR_DATA *ch, char *argument )
{
	DESCRIPTOR_DATA *d;

	CheckCH(ch);

	act_new(Format("\tR[Help Line] %s: $t\tn", IS_NPC(ch)?ch->short_descr:ch->name),ch,argument,NULL,TO_CHAR,P_DEAD,1);

	for ( d = descriptor_list; d != NULL; d = d->next )
	{
		if ( d->connected == CON_PLAYING &&
				!IS_SET(d->character->comm,COMM_NOWIZ) )
		{
			act_new(Format("\tR[Help Line] %s: $t\tn", IS_NPC(ch)?ch->short_descr:ch->name),ch,argument,d->character,TO_VICT,P_DEAD,1);
		}
	}

	return;
}

void do_oocchan( CHAR_DATA *ch, char *argument )
{
	DESCRIPTOR_DATA *d;

	CheckCH(ch);

	if(IS_NULLSTR(argument))
	{
		send_to_char("What do you want to say on the OOC channel?\n\r", ch);
		return;
	}

	if(IS_SET(ch->comm, COMM_OOC_OFF))
	{
		send_to_char("Ooc channel toggled on.\n\r", ch);
		REMOVE_BIT(ch->comm, COMM_OOC_OFF);
	}

	act_new(Format("\tY[OOC] %s: \ty$t\tn", IS_NPC(ch)?ch->short_descr:ch->name),ch,argument,NULL,TO_CHAR,P_DEAD,1);

	for ( d = descriptor_list; d != NULL; d = d->next )
	{
		if ( d->connected == CON_PLAYING &&
				!IS_SET(d->character->comm,COMM_NOWIZ)
				&& !IS_SET(d->character->comm, COMM_OOC_OFF))
		{
			act_new(Format("\tY[OOC] %s: \ty$t\tn", IS_NPC(ch)?ch->short_descr:ch->name),ch,argument,d->character,TO_VICT,P_DEAD,1);
		}
	}

	return;
}

void do_immtalk( CHAR_DATA *ch, char *argument )
{
	DESCRIPTOR_DATA *d;

	CheckCH(ch);

	if ( IS_NULLSTR(argument) )
	{
		if (IS_SET(ch->comm,COMM_NOWIZ))
		{
			send_to_char("Immortal channel is now ON\n\r",ch);
			REMOVE_BIT(ch->comm,COMM_NOWIZ);
		}
		else
		{
			send_to_char("Immortal channel is now OFF\n\r",ch);
			SET_BIT(ch->comm,COMM_NOWIZ);
		}
		return;
	}

	act_new(Format("\tY[Staff]\tn \tR%s: \tr$t\tn", IS_NPC(ch)?ch->short_descr:ch->name),ch,argument,NULL,TO_CHAR,P_DEAD,1);

	// Test Code
	// act_new(Format("\t<FRAME Name='Tells' Left='-20c' Top='0' Width='80c' Height='20c'>\tY[Staff]\tn \tR%s: \tr$t\tn\t</FRAME>", IS_NPC(ch)?ch->short_descr:ch->name),ch,argument,NULL,TO_CHAR,P_DEAD,1);

	for ( d = descriptor_list; d != NULL; d = d->next )
	{
		if ( d->connected == CON_PLAYING &&
				IS_ADMIN(d->character) &&
				!IS_SET(d->character->comm,COMM_NOWIZ) )
		{
			act_new(Format("\tY[Staff]\tn \tR%s: \tr$t\tn", IS_NPC(ch)?ch->short_descr:ch->name),ch,argument,d->character,TO_VICT,P_DEAD,1);
		}
	}

	return;
}

void do_oocsay( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *rch;

	CheckCH(ch);

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Say what?\n\r", ch );
		return;
	}

	if(ch->in_room != NULL)
	{
		for ( rch = ch->in_room->people; rch; rch = rch->next_in_room )
		{
			if ( rch != ch )
			{
				if(!IS_SET(ch->act2, ACT2_STORY))
				{
					if((ch->on && rch->on == ch->on && IS_SET(ch->comm, COMM_DISCREET))
							|| ch->on == NULL || !IS_SET(ch->comm, COMM_DISCREET))
					{
						act(Format("\tG%s says, in an ooc manner, \tY'$t'\tn", IS_NPC(ch)?ch->short_descr:ch->name), ch, argument, rch, TO_VICT, 1 );
					}
					else
						act("$n mutters something to those close enough to hear.", ch, NULL, rch, TO_VICT, 1 );
				}
				else
				{
					act(Format("\tY%s speaks as storyteller, \tR'$t'\tn", IS_NPC(ch)?ch->short_descr:ch->name), ch, argument, rch, TO_VICT, 1 );
				}
			}
		}
	}

	if(IS_SET(ch->act2, ACT2_STORY))
	{
		act(Format("\tYYou speak as storyteller, '\tR$T\tY'\tn"), ch, NULL, argument, TO_CHAR, 1 );
	}
	else
	{
		act(Format("\tGYou say, in an ooc manner, '\tY$T\tG'\tn"), ch, NULL, argument, TO_CHAR, 1 );
	}

	return;
}

void do_think( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *rch;
	int fail = 0;
	ROOM_INDEX_DATA *to_room;

	CheckCH(ch);

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Think what?\n\r", ch );
		return;
	}

	if(IS_SET(ch->act2, ACT2_ASTRAL))
		to_room = ch->listening;
	else to_room = ch->in_room;

	for ( rch = to_room->people; rch; rch = rch->next_in_room )
	{
		if ( rch != ch )
		{
			if(IS_SET(rch->affected_by, AFF_AUSPEX4))
			{
				fail = dice_rolls(rch, get_curr_stat(rch,
						STAT_INT) + rch->ability[SUBTERFUGE].value, rch->max_willpower);
			}

			if(((IS_ADMIN(rch) && IS_SET(ch->comm, COMM_THINK_ON))
					|| (IS_SET(rch->affected_by, AFF_AUSPEX4) && fail > 0))
					&& !IS_SET(rch->act2, ACT2_ASTRAL))
			{
				act(Format("\tM%s thinks, '\tY$t\tM'\tn", IS_NPC(ch)?ch->short_descr:ch->name), ch, argument, rch, TO_VICT, 1 );
			}
		}
	}

	for ( rch = to_room->listeners; rch; rch = rch->next_in_room )
	{
		if ( rch != ch )
		{
			if(IS_SET(rch->affected_by, AFF_AUSPEX4))
			{
				fail = dice_rolls(rch, get_curr_stat(rch,
						STAT_INT) + rch->ability[SUBTERFUGE].value, rch->max_willpower);
			}

			if(IS_SET(ch->comm, COMM_THINK_ON)
					|| (IS_SET(rch->affected_by, AFF_AUSPEX4) && fail > 0))
			{
				act(Format("\tM%s thinks, '\tY$t\tM'\tn", IS_NPC(ch)?ch->short_descr:ch->name), ch, argument, rch, TO_VICT, 1 );
			}
		}
	}

	act( Format("\tMYou think '\tY$T\tM'\tn"), ch, NULL, argument, TO_CHAR, 1 );

	return;
}

	void do_say( CHAR_DATA *ch, char *argument )
	{
		char buf[4*MSL]={'\0'};
		CHAR_DATA *rch, *rch_next;
		CheckCH(ch);
		if ( IS_NULLSTR(argument) )
		{
			send_to_char( "Say what?\n\r", ch );
			return;
		}
		translate(ch, argument, buf);
		MOBtrigger = FALSE;
		act( Format("\tCYou say '\tY$T\tC'\tn"), ch, NULL, argument, TO_CHAR, 0 );
		for ( rch = ch->in_room->people; rch; rch = rch_next )
		{
			rch_next = rch->next_in_room;
			if ( rch != ch )
			{
				if(!IS_SET(ch->comm, COMM_DEAF))
				{
					if((ch->on && rch->on == ch->on && IS_SET(ch->comm, COMM_DISCREET))
							|| ch->on == NULL
							|| !IS_SET(ch->comm, COMM_DISCREET)
							|| IS_AFFECTED(rch, AFF_HSENSES)
							|| IS_SET(rch->affected_by, AFF_HSENSES))
					{
						act( Format("\tC$n says, \tY'$t\tY'\tn"), ch, ((!IS_NPC(rch) && ch->clan==rch->clan) || can_comprehend(rch, ch)) ? argument : buf, rch, TO_VICT, 0 );
					}
					else
					{
						MOBtrigger = TRUE;
						act("$n mutters something to those close enough to hear.", ch, NULL, rch, TO_VICT, 0 );
						MOBtrigger = FALSE;
					}
				}
				else
				{
					MOBtrigger = TRUE;
					act("$n says something, but you don't hear a word.", ch, NULL, rch, TO_VICT, 0 );
					MOBtrigger = FALSE;
				}
				if ( IS_NPC(rch) && HAS_TRIGGER( rch, TRIG_SPEECH )
						&&  rch->position == rch->pIndexData->default_pos )
					mp_act_trigger( ((!IS_NPC(rch) && ch->clan==rch->clan) || can_comprehend(rch, ch)) ? argument : buf, rch, ch, NULL, NULL, TRIG_SPEECH );
			}
		}
		MOBtrigger = TRUE;
		if(IS_AFFECTED(ch, AFF_HIDE) || IS_AFFECTED(ch, AFF_INVISIBLE) || IS_AFFECTED(ch, AFF_SNEAK))
			do_visible(ch,"");
		return;
	}

void do_dsay( CHAR_DATA *ch, char *argument )
{
	char buf[4*MSL]={'\0'};
	CHAR_DATA *rch, *rch_next;
	CHAR_DATA *victim;
	char arg[MIL]={'\0'};
	char string[MSL]={'\0'};
	char *name;

	CheckCH(ch);

	argument = one_argument(argument, arg);

	if ( IS_NULLSTR(arg) )
	{
		send_to_char( "To whom are you speaking?\n\r", ch );
		return;
	}

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Say what?\n\r", ch );
		return;
	}

	if((victim = get_char_room(ch, arg)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	translate(ch, argument, buf);

	MOBtrigger = FALSE;
	for ( rch = ch->in_room->people; rch; rch = rch_next )
	{
		rch_next = rch->next_in_room;
		if ( rch != ch )
		{
			if(!IS_SET(ch->comm, COMM_DEAF))
			{
				if((ch->on && rch->on == ch->on && IS_SET(ch->comm, COMM_DISCREET))
						|| ch->on == NULL
						|| !IS_SET(ch->comm, COMM_DISCREET)
						|| IS_AFFECTED(rch, AFF_HSENSES)
						|| IS_SET(rch->affected_by, AFF_HSENSES))
				{
					if(rch == victim)
					{
						snprintf(string, sizeof(string), "\tC$n says to you '\tY%s$t\tC'\tn", ch->colours[0][0]=='\0'?"{c":ch->colours[0]);
					}
					else
					{
						if(IS_NPC(victim)) name = victim->short_descr;
						else if(LOOKS_DIFFERENT(victim)) name = victim->alt_name;
						else name = victim->name;
						snprintf(string, sizeof(string), "\tC$n says to %s '\tY$t\tC'\tn", name);
					}
					act(string,	ch,	((!IS_NPC(rch) && ch->clan==rch->clan) || can_comprehend(rch, ch)) ? argument : buf, rch, TO_VICT, 0 );
				}
				else
				{
					MOBtrigger = TRUE;
					act("$n mutters something to those close enough to hear.", ch, NULL, rch, TO_VICT, 0 );
					MOBtrigger = FALSE;
				}
			}
			else
			{
				MOBtrigger = TRUE;
				act("$n says something, but you don't hear a word.", ch, NULL, rch, TO_VICT, 0 );
				MOBtrigger = FALSE;
			}

			if ( IS_NPC(rch) && HAS_TRIGGER( rch, TRIG_SPEECH ) &&   rch->position == rch->pIndexData->default_pos )
				mp_act_trigger( ((!IS_NPC(rch) && ch->clan==rch->clan) || can_comprehend(rch, ch)) ? argument : buf,
						rch, ch, NULL, NULL, TRIG_SPEECH );
		}
	}

	act((char *)Format("\tCYou say to $N '\tY$t\tC'\tn"), ch, argument, victim, TO_CHAR, 0 );
	MOBtrigger = TRUE;

	if(IS_AFFECTED(ch, AFF_HIDE) || IS_AFFECTED(ch, AFF_INVISIBLE) || IS_AFFECTED(ch, AFF_SNEAK))
		do_visible(ch,"");

	return;
}

void do_whisper( CHAR_DATA *ch, char *argument )
{
	char buf[4*MSL]={'\0'};
	CHAR_DATA *rch;
	CHAR_DATA *victim;
	char arg[MIL]={'\0'};
	char string[MSL]={'\0'};
	char *name;

	CheckCH(ch);

	argument = one_argument(argument, arg);

	if ( IS_NULLSTR(arg) )
	{
		send_to_char( "To whom are you speaking?\n\r", ch );
		return;
	}

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Whisper what?\n\r", ch );
		return;
	}

	if((victim = get_char_room(ch, arg)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	translate(ch, argument, buf);

	for ( rch = ch->in_room->people; rch; rch = rch->next_in_room )
	{
		if ( rch != ch )
		{
			if(rch == victim)
			{
				snprintf(string, sizeof(string), "\tC$n whispers to you '\tY$t\tC'\tn");
			}
			else
			{
				if(IS_NPC(victim))
					name = victim->short_descr;
				else if(LOOKS_DIFFERENT(victim))
					name = victim->alt_name;
				else name = victim->name;
				if((IS_AFFECTED(rch, AFF_HSENSES) || IS_SET(rch->affected_by, AFF_HSENSES))
						&& !IS_SET(rch->comm, COMM_DEAF)) {
					snprintf(string, sizeof(string), "\tC$n whispers to %s '\tY%s$t\tC'\tn", name, ch->colours[0][0]=='\0'?"{c":ch->colours[0]);
					MOBtrigger = FALSE;
				}
				else
				{
					snprintf(string, sizeof(string), "\tC%s$n whispers something to %s.\tn", ch->colours[0][0]=='\0'?"{c":ch->colours[0], name);
				}
			}
			act(string, ch, ((!IS_NPC(rch) && ch->clan==rch->clan) || can_comprehend(rch, ch)) ? argument : buf, rch, TO_VICT, 0 );
			if ( IS_NPC(rch) && HAS_TRIGGER( rch, TRIG_SPEECH )
					&&   rch->position == rch->pIndexData->default_pos )
				mp_act_trigger( ((!IS_NPC(rch) && ch->clan==rch->clan) || can_comprehend(rch, ch)) ? argument : buf,
						rch, ch, NULL, NULL, TRIG_SPEECH );
			MOBtrigger = TRUE;
		}
	}

	act(Format("\tCYou whisper to $N '\tY%t\tC'\tn"), ch, argument, victim, TO_CHAR, 0 );

	if((IS_AFFECTED(ch, AFF_HIDE) || IS_AFFECTED(ch, AFF_INVISIBLE)
			|| IS_AFFECTED(ch, AFF_SNEAK)) && number_range(0,100) > 75)
		do_visible(ch,"");

	return;
}

bool HAS_PHONE ( CHAR_DATA * ch )
{
	OBJ_DATA *obj;
	bool found = FALSE;
	int iWear = 0;

	found = FALSE;
	for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
	{
		if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
			continue;

		if ( obj->item_type == ITEM_TELEPHONE )
			found = TRUE;
	}

	return found;
}

bool OWNS_PHONE (CHAR_DATA *ch)
{
	OBJ_DATA *obj;
	bool found = FALSE;

	found = FALSE;
	for ( obj = ch->carrying; obj; obj = obj->next_content )
	{
		if ( obj->item_type == ITEM_TELEPHONE )
			found = TRUE;
	}

	return found;
}

void do_tell( CHAR_DATA *ch, char *argument )
{
	char arg[MIL]={'\0'};
	CHAR_DATA *victim = NULL;

	CheckCH(ch);

	if ( IS_SET(ch->comm, COMM_NOTELL) || IS_SET(ch->comm,COMM_DEAF) )
	{
		send_to_char( "You cannot use tells at the moment.\n\r", ch );
		return;
	}

	if ( IS_SET(ch->comm, COMM_QUIET) )
	{
		send_to_char( "Turn off quiet to use tells.\n\r", ch);
		return;
	}

	argument = one_argument( argument, arg );

	if ( IS_NULLSTR(arg) || IS_NULLSTR(argument) )
	{
		send_to_char( "Tell whom what?\n\r", ch );
		return;
	}

	/*
	 * Can tell to PC's anywhere, but NPC's only in same room.
	 * -- Furey
	 */
	if ( ( victim = get_char_world( ch, arg ) ) == NULL
			|| ( IS_NPC(victim) && victim->in_room != ch->in_room ) )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	if ( victim->desc == NULL && !IS_NPC(victim))
	{
		act("$N is having link problems... try again later.", ch,NULL,victim,TO_CHAR, 0);
		add_buf(victim->pcdata->buffer, (char *)Format("%s's tells you from a recording '%s'\n\r",PERS(ch,victim), CapitalSentence(argument) ) );
		return;
	}

	if (((IS_SET(victim->comm,COMM_QUIET) || IS_SET(victim->comm,COMM_DEAF))
			&& !IS_ADMIN(ch)) || IS_SET(victim->comm,COMM_OLC))
	{
		act( "$E can't hear you.", ch, 0, victim, TO_CHAR, 1 );
		return;
	}

	if (!IS_NPC(victim) && IS_REJECTED(ch, victim))
	{
		act( "$E has blacklisted you.", ch, 0, victim, TO_CHAR, 1 );
		return;
	}

	if (IS_SET(victim->comm,COMM_AFK))
	{
		if (IS_NPC(victim))
		{
			act("$E is AFK, and not receiving tells.",ch,NULL,victim,TO_CHAR,1);
			return;
		}

		act("$E is AFK, but your tell will go through when $E returns.", ch,NULL,victim,TO_CHAR,1);
		add_buf(victim->pcdata->buffer, (char *)Format("%s tells you '%s'\n\r",PERS(ch,victim), CapitalSentence(argument) ) );
		return;
	}

	act( "\tMYou tell $N '\tY$t\tM'\tn", ch, argument, victim, TO_CHAR, 1 );
	act_new("\tM$n tells you '\tY$t\tM'\tn",ch,argument,victim,TO_VICT,P_DEAD,1);
	victim->reply	= ch;
	victim->on_phone	= TRUE;
	ch->reply		= victim;
	ch->on_phone	= TRUE;

	if ( !IS_NPC(ch) && IS_NPC(victim) && HAS_TRIGGER(victim,TRIG_SPEECH) )
		mp_act_trigger( argument, victim, ch, NULL, NULL, TRIG_SPEECH );

	return;
}

void do_call( CHAR_DATA *ch, char *argument )
{
	char arg[MIL]={'\0'};
	CHAR_DATA *victim = NULL;

	CheckCH(ch);

	if ( !HAS_PHONE(ch) )
	{
		send_to_char( "You can't make a call without a phone in your hand!\n\r", ch );
		return;
	}

	if ( IS_SET(ch->comm, COMM_NOTELL) || IS_SET(ch->comm,COMM_NOPHONE) )
	{
		send_to_char( "I'm sorry, your number is currently disconnected.\n\r", ch );
		return;
	}

	if ( IS_SET(ch->comm, COMM_QUIET) )
	{
		send_to_char( "You must turn on your phone first.\n\r", ch);
		return;
	}

	argument = one_argument( argument, arg );

	if ( (!str_cmp(arg, "end") || !str_cmp(arg, "End"))
			&& (ch->on_phone) )
	{
		if( ch != ch->calling && ch->calling != NULL )
			ch->calling->on_phone = FALSE;
		ch->on_phone = FALSE;
		send_to_char("You hang up.\n\r",ch);
		if( ch->calling != NULL )
			send_to_char("The phone goes dead.\n\r", ch->calling);
		return;
	}

	if ( IS_SET(ch->in_room->room_flags, ROOM_NOPHONE) )
	{
		send_to_char( "You can't seem to get any reception.\n\r", ch);
		return;
	}

	if ( IS_NULLSTR(arg) || IS_NULLSTR(argument) )
	{
		send_to_char( "Phone whom to say what?\n\r", ch );
		return;
	}
	if(!ch->on_phone)
		send_to_char( "The phone rings.\n\r", ch );

	/*
	 * Can tell to PC's anywhere, but NPC's only in same room.
	 * -- Furey
	 */
	if ( ( victim = get_char_world( ch, arg ) ) == NULL
			|| ( IS_NPC(victim) && victim->in_room != ch->in_room ) )
	{
		send_to_char( "That number is not connected.\n\rPlease check the number, and try again.\n\r", ch );
		return;
	}

	if ( IS_SET(victim->in_room->room_flags, ROOM_NOPHONE) )
	{
		send_to_char( "The phone seems to ring, but no-one picks up.\n\r", ch);
		return;
	}

	if ( (ch->on_phone) && ((ch->reply != victim) && (ch->reply != NULL)) )
	{
		send_to_char("You're already in a call!\n\r",ch);
		return;
	}

	if(OWNS_PHONE(victim) && !HAS_PHONE(victim))
	{
		send_to_char("The phone rings, but no-one picks it up.\n\r", ch);
		act("$n's phone rings.", ch, NULL, NULL, TO_ROOM, 0);
		if((IS_AFFECTED(ch, AFF_INVISIBLE)
				|| IS_AFFECTED(ch, AFF_HIDE)
				|| IS_AFFECTED(ch, AFF_SNEAK))
				&& SAME_PLANE(ch, victim))
			do_visible(ch, "");
		if((IS_AFFECTED(victim, AFF_INVISIBLE)
				|| IS_AFFECTED(victim, AFF_HIDE)
				|| IS_AFFECTED(victim, AFF_SNEAK))
				&& SAME_PLANE(ch, victim))
			do_visible(victim, "");
		act("Your phone rings.", ch, NULL, victim, TO_VICT, 0);
		return;
	}

	if ( !HAS_PHONE(victim) )
	{
		send_to_char( "That number is not connected.\n\rPlease check the number, and try again.\n\r", ch );
		return;
	}

	if ( victim->desc == NULL && !IS_NPC(victim))
	{
		act("$N seems to be out of order...try again later.", ch,NULL,victim,TO_CHAR, 0);
		add_buf(victim->pcdata->buffer, (char *)Format("%s's voice crackles '%s'\n\r",PERS(ch,victim), CapitalSentence(argument) ) );
		return;
	}

	if ( !(IS_ADMIN(ch) && ch->trust < LEVEL_IMMORTAL) && !IS_AWAKE(victim) )
	{
		act("$E seems not to be answering the phone.",ch,0,victim,TO_CHAR, 0);
		return;
	}

	if (((IS_SET(victim->comm,COMM_QUIET) || IS_SET(victim->comm,COMM_NOPHONE))
			&& !IS_ADMIN(ch)) || IS_SET(victim->comm,COMM_OLC))
	{
		act("$E must have switched off their phone.",ch,0,victim,TO_CHAR,0);
		return;
	}

	if(IS_SET(victim->comm,COMM_AFK))
	{
		act("$N is currently unavailable.", ch, NULL, victim, TO_CHAR, 0);
		return;
	}

	if (SAME_PLANE(ch,victim))
	{
		if ( !victim->on_phone)
		{
			act_new("You pick up a call from $n",ch,NULL,victim,TO_VICT,P_DEAD,0);
			if(IS_AFFECTED(victim, AFF_INVISIBLE) || IS_AFFECTED(victim, AFF_HIDE)
					|| IS_AFFECTED(victim, AFF_SNEAK))
				do_visible(victim, "");
			act("$n's phone rings.", victim, NULL, NULL, TO_ROOM,0);
		}
		else
		{
			if(victim->calling != ch)
			{
				send_to_char( "I'm sorry, your party is on another call.\n\r", ch );
				send_to_char( "A small blip sounds.\n\r", victim );
				return;
			}
		}
	}
	else
	{
		send_to_char("The phone rings, but nobody answers.\n\r", ch);
		return;
	}

	act( "\tRYou tell $N '\tY$t\tR'\tn", ch, argument, victim, TO_CHAR, 0 );
	if(!IS_SET(ch->comm, COMM_DISCREET))
		act( "\tR$n says into $s phone '\tY$t\tR'\tn", ch, argument, NULL, TO_ROOM, 0 );
	else
		act( "$n says something into $s phone.", ch, argument, NULL, TO_ROOM, 0 );
	act_new("\tR$n tells you '\tY$t\tR'\tn",ch,argument,victim,TO_VICT,P_DEAD,0);
	victim->calling	= ch;
	victim->on_phone	= TRUE;
	ch->calling		= victim;
	ch->on_phone	= TRUE;

	if ( !IS_NPC(ch) && IS_NPC(victim) && HAS_TRIGGER(victim,TRIG_SPEECH) )
		mp_act_trigger( argument, victim, ch, NULL, NULL, TRIG_SPEECH );

	return;
}

/* Speaking to sleeping characters anywhere in the mud. */
void do_dreamspeak( CHAR_DATA *ch, char *argument )
{
	char arg[MIL]={'\0'};
	CHAR_DATA *victim = NULL;

	CheckCH(ch);

	if(ch->auspice != auspice_lookup("galliard") || ch->disc[DISC_AUSPICE] < 3)
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if(ch->power_timer > 0)
	{
		send_to_char("Your powers have not rejuvenated yet.\n\r", ch);
		return;
	}

	if ( IS_SET(ch->comm, COMM_NOTELL) || IS_SET(ch->comm,COMM_QUIET) )
	{
		send_to_char( "They cannot hear you.\n\r", ch );
		return;
	}

	argument = one_argument( argument, arg );

	if ( IS_NULLSTR(arg) || IS_NULLSTR(argument) )
	{
		send_to_char( "Whose dreams do you wish to interrupt, and what will you say?\n\r", ch );
		return;
	}

	/*
	 * No telling to mobs.
	 */
	if ( ( victim = get_char_world( ch, arg ) ) == NULL
			|| victim->position != P_SLEEP )
	{
		send_to_char( "They cannot hear you.\n\r", ch );
		return;
	}

	if ( victim->desc == NULL )
	{
		act("$N cannot hear through the fog of sleep.",
				ch,NULL,victim,TO_CHAR,1);
		return;
	}

	if ((IS_SET(victim->comm,COMM_QUIET)
			&& !IS_ADMIN(ch)) || IS_SET(victim->comm,COMM_OLC))
	{
		act( "$E is unable to hear your message.", ch, 0, victim, TO_CHAR, 1 );
		return;
	}

	act("You speak into $N's dream, saying '$t'",ch,argument,victim,TO_CHAR,1);
	act_new("$n's voice comes to you in your dream, saying '$t'",ch,argument,victim,TO_VICT,P_DEAD,1);

	ch->power_timer = 2;

	if ( !IS_NPC(ch) && IS_NPC(victim) && HAS_TRIGGER(victim,TRIG_SPEECH) )
		mp_act_trigger( argument, victim, ch, NULL, NULL, TRIG_SPEECH );

	return;
}


/* Speaking to characters anywhere in the mud. */
void do_mentalspeech( CHAR_DATA *ch, char *argument )
{
	char arg[MIL]={'\0'};
	CHAR_DATA *victim = NULL;

	CheckCH(ch);

	if(ch->breed != breed_lookup("metis") || ch->disc[DISC_BREED] < 3)
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if(ch->power_timer > 0)
	{
		send_to_char("Your powers have not rejuvenated yet.\n\r", ch);
		return;
	}

	if ( IS_SET(ch->comm, COMM_NOTELL) || IS_SET(ch->comm,COMM_QUIET) )
	{
		send_to_char( "They cannot hear you.\n\r", ch );
		return;
	}

	argument = one_argument( argument, arg );

	if ( IS_NULLSTR(arg) || IS_NULLSTR(argument) )
	{
		send_to_char( "Whose thoughts do you wish to interrupt, and what will you say?\n\r", ch );
		return;
	}

	/*
	 * No telling to mobs.
	 */
	if ( ( victim = get_char_world( ch, arg ) ) == NULL)
	{
		send_to_char( "They cannot hear you.\n\r", ch );
		return;
	}

	if ( victim->desc == NULL )
	{
		act("$N cannot hear through the psychic static.",
				ch,NULL,victim,TO_CHAR,1);
		return;
	}

	if ((IS_SET(victim->comm,COMM_QUIET)
			&& !IS_ADMIN(ch)) || IS_SET(victim->comm,COMM_OLC))
	{
		act( "$E is unable to hear your message.", ch, 0, victim, TO_CHAR, 1 );
		return;
	}

	if ( !SAME_PLANE(victim, ch) )
	{
		act("$N seems beyond your reach.",ch,NULL,victim,TO_CHAR,1);
		return;
	}

	act( "You speak to $N's thoughts, saying '$t'", ch, argument, victim, TO_CHAR, 0 );
	act_new("$n's voice speaks in your head, saying '$t'", ch,argument,victim,TO_VICT,P_DEAD,0);

	ch->power_timer = 2;

	if ( !IS_NPC(ch) && IS_NPC(victim) && HAS_TRIGGER(victim,TRIG_SPEECH) )
		mp_act_trigger( argument, victim, ch, NULL, NULL, TRIG_SPEECH );

	return;
}


void do_reply( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;

	CheckCH(ch);

	if ( IS_SET(ch->comm, COMM_NOTELL) )
	{
		send_to_char( "Your message didn't get through.\n\r", ch );
		return;
	}

	if ( ( victim = ch->reply ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	if ( victim->desc == NULL && !IS_NPC(victim))
	{
		act("$N seems to have misplaced $S link...try again later.", ch,NULL,victim,TO_CHAR, 0);
		add_buf(victim->pcdata->buffer, (char *)Format("\tR%s tells you '\tY%s\tR'\tn\n\r",PERS(ch,victim), CapitalSentence(argument) ) );
		return;
	}

	if ( !IS_ADMIN(ch) && !IS_AWAKE(victim) )
	{
		act( "$E can't hear you.", ch, 0, victim, TO_CHAR, 1 );
		return;
	}

	if (((IS_SET(victim->comm,COMM_QUIET) || IS_SET(victim->comm,COMM_DEAF))
			&&  !IS_ADMIN(ch) && !IS_ADMIN(victim))
			|| IS_SET(victim->comm,COMM_OLC))
	{
		act_new( "$E is not receiving tells.", ch, 0, victim, TO_CHAR,P_DEAD,1);
		return;
	}

	if (!IS_ADMIN(victim) && !IS_AWAKE(ch))
	{
		send_to_char( "In your dreams, or what?\n\r", ch );
		return;
	}

	if (IS_SET(victim->comm,COMM_AFK))
	{
		if (IS_NPC(victim))
		{
			act("$E is AFK, and not receiving tells.",ch,NULL,victim,TO_CHAR,1);
			return;
		}

		act("$E is AFK, but your tell will go through when $E returns.", ch,NULL,victim,TO_CHAR,1);
		add_buf(victim->pcdata->buffer, (char *)Format("%s tells you '%s'\n\r",PERS(ch,victim), CapitalSentence(argument)) );
		return;
	}

	act_new("\tRYou tell $N '\tY$t\tR'\tn",ch,argument,victim,TO_CHAR,P_DEAD,1);
	act_new("\tR$n tells you '\tY$t\tR'\tn",ch,argument,victim,TO_VICT,P_DEAD,1);
	victim->reply	= ch;

	if ( !IS_NPC(ch) && IS_NPC(victim) && HAS_TRIGGER(victim,TRIG_SPEECH) )
		mp_act_trigger( argument, victim, ch, NULL, NULL, TRIG_SPEECH );

	return;
}


void do_dmote( CHAR_DATA *ch, char *argument )
{

	CheckCH(ch);

	if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) )
	{
		send_to_char( "You can't show your emotions.\n\r", ch );
		return;
	}

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Dmote what?\n\r", ch );
		return;
	}

	emote_say_colouring(argument, ch);

	act( "$t$T \tR[\tC$n\tR]\tn", ch, NULL, argument, TO_ROOM, 0 );
	act( "$t$T \tR[\tC$n\tR]\tn", ch, NULL, argument, TO_CHAR, 0 );

	if(IS_AFFECTED(ch, AFF_HIDE) || IS_AFFECTED(ch, AFF_INVISIBLE) || IS_AFFECTED(ch, AFF_SNEAK))
	do_visible(ch,"");

	return;
}


void do_dpmote( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *vch;
	char *letter,*name;
	char last[MIL]={'\0'};
	char temp[MSL]={'\0'};
	int matches = 0;

	CheckCH(ch);

	if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) )
	{
		send_to_char( "You can't show your emotions.\n\r", ch );
		return;
	}

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Emote what?\n\r", ch );
		return;
	}

	emote_say_colouring(argument, ch);
	act( "\tc$t [\tC$n\tc]\tn", ch, argument, NULL, TO_CHAR, 0 );

	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
	{
		if (vch->desc == NULL || vch == ch)
			continue;

		if ((letter = strstr(argument,vch->name)) == NULL)
		{
			act(Format("%s$t [\tC$N\tn]"), vch,argument,ch,TO_CHAR,0);
			continue;
		}

		strncpy(temp,argument, sizeof(temp));
		temp[strlen(argument) - strlen(letter)] = '\0';
		name = vch->name;

		for (; *letter != '\0'; letter++)
		{
			if (*letter == '\'' && matches == strlen(vch->name))
			{
				strncat(temp,"r", sizeof(temp));
				continue;
			}

			if (*letter == 's' && matches == strlen(vch->name))
			{
				matches = 0;
				continue;
			}

			if (matches == strlen(vch->name))
			{
				matches = 0;
			}

			if (*letter == *name)
			{
				matches++;
				name++;
				if (matches == strlen(vch->name))
				{
					strncat(temp,"you", sizeof(temp));
					last[0] = '\0';
					name = vch->name;
					continue;
				}
				strncat(last,letter,1);
				continue;
			}

			matches = 0;
			strncat(temp,last, sizeof(temp));
			strncat(temp,letter,1);
			last[0] = '\0';
			name = vch->name;
		}

		act(Format("%s$t [\tC$N\tn]"), vch,temp,ch,TO_CHAR,0);
	}

	if(IS_AFFECTED(ch, AFF_HIDE) || IS_AFFECTED(ch, AFF_INVISIBLE) || IS_AFFECTED(ch, AFF_SNEAK))
		do_visible(ch,"");

	return;
}


void do_emote( CHAR_DATA *ch, char *argument )
{
	char discreet[MSL]={'\0'};
	char release[MSL]={'\0'};
	CHAR_DATA *rch;

	CheckCH(ch);

	if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) )
	{
		send_to_char( "You can't show your emotions.\n\r", ch );
		return;
	}

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Emote what?\n\r", ch );
		return;
	}

	discreetify(argument, discreet);

	strncpy(release, argument, sizeof(release));
	emote_say_colouring(discreet, ch);
	emote_say_colouring(release, ch);

	for ( rch = ch->in_room->people; rch; rch = rch->next_in_room )
	{
		if(ch!=rch)
		{
			if(((ch->on && rch->on == ch->on && IS_SET(ch->comm, COMM_DISCREET))
					|| ch->on == NULL
					|| !IS_SET(ch->comm, COMM_DISCREET)
					|| IS_AFFECTED(rch, AFF_HSENSES)
					|| IS_SET(rch->affected_by, AFF_HSENSES))
					&& !IS_SET(ch->comm, COMM_DEAF))
			{
				if(!str_prefix("'s ", release))
				{
					act( Format("$n$t\tn"), ch, release, rch, TO_VICT, 0 );
				}
				else
				{
					act( Format("$n $t\tn"), ch, release, rch, TO_VICT, 0 );
				}
			} else {
				if(!str_prefix("'s ", release))
				{
					act( Format("$n$t\tn"), ch, discreet, rch, TO_VICT, 0 );
				}
				else
				{
					act( Format("$n $t\tn"), ch, discreet, rch, TO_VICT, 0 );
				}
			}
		}
	}

	if(!str_prefix("'s ", release))
	{
		act( Format("$n$t\tn"), ch, release, NULL, TO_CHAR, 0 );
	}
	else
	{
		act( Format("$n $t\tn"), ch, release, NULL, TO_CHAR, 0 );
	}

	if(IS_AFFECTED(ch, AFF_HIDE) || IS_AFFECTED(ch, AFF_INVISIBLE) || IS_AFFECTED(ch, AFF_SNEAK))
		do_visible(ch,"");

	return;
}


void do_pmote( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *vch;
	char *name,*letter;
	char last[MIL]={'\0'};
	char temp[MSL]={'\0'};
	char discreet[4*MIL]={'\0'};
	char release[4*MSL]={'\0'};
	char string[MSL]={'\0'};
	int matches = 0;

	CheckCH(ch);

	if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) )
	{
		send_to_char( "You can't show your emotions.\n\r", ch );
		return;
	}

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Emote what?\n\r", ch );
		return;
	}

	strncpy(release, argument, sizeof(release));
	discreetify(argument, discreet);

	emote_say_colouring(discreet, ch);
	emote_say_colouring(release, ch);

	if(!str_prefix("'s ", release))
	{
		act( "$t$n$T{x", ch, ch->colours[1][0]=='\0'?"{c":ch->colours[1], release, TO_CHAR, 0 );
	}
	else
	{
		act( "$t$n $T{x", ch, ch->colours[1][0]=='\0'?"{c":ch->colours[1], release, TO_CHAR, 0 );
	}

	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
	{
		if (vch->desc == NULL || vch == ch)
			continue;

		if ((letter = strstr(release,vch->name)) == NULL)
		{
			if(ch!=vch)
			{
				if(((ch->on && vch->on == ch->on && IS_SET(ch->comm, COMM_DISCREET))
						|| ch->on == NULL
						|| !IS_SET(ch->comm, COMM_DISCREET)
						|| IS_AFFECTED(vch, AFF_HSENSES)
						|| IS_SET(vch->affected_by, AFF_HSENSES))
						&& !IS_SET(ch->comm, COMM_DEAF))
				{
					if(!str_prefix("'s ", release))
					{
						snprintf(string, sizeof(string), "%s$n$t{x", ch->colours[1][0]=='\0'?"{c":ch->colours[1]);
						act( string, ch, release, vch, TO_VICT, 0 );
					}
					else
					{
						snprintf(string, sizeof(string), "%s$n $t{x", ch->colours[1][0]=='\0'?"{c":ch->colours[1]);
						act( string, ch, release, vch, TO_VICT, 0 );
					}
				}
				else
				{
					if(!str_prefix("'s ", release))
					{
						snprintf(string, sizeof(string), "%s$n$t{x", ch->colours[1][0]=='\0'?"{c":ch->colours[1]);
						act( string, ch, discreet, vch, TO_VICT, 0 );
					}
					else
					{
						snprintf(string, sizeof(string), "%s$n $t{x", ch->colours[1][0]=='\0'?"{c":ch->colours[1]);
						act( string, ch, discreet, vch, TO_VICT, 0 );
					}
				}
			}
			else
			{
				act( string, ch, release, NULL, TO_CHAR, 0 );
			}

			continue;
		}

		strncpy(temp,release, sizeof(temp));
		temp[strlen(release) - strlen(letter)] = '\0';
		last[0] = '\0';
		name = vch->name;

		for (; *letter != '\0'; letter++)
		{
			if (*letter == '\'' && matches == strlen(vch->name))
			{
				strncat(temp,"r", sizeof(temp));
				continue;
			}

			if (*letter == 's' && matches == strlen(vch->name))
			{
				matches = 0;
				continue;
			}

			if (matches == strlen(vch->name))
			{
				matches = 0;
			}

			if (*letter == *name)
			{
				matches++;
				name++;
				if (matches == strlen(vch->name))
				{
					strncat(temp,"you", sizeof(temp));
					last[0] = '\0';
					name = vch->name;
					continue;
				}
				strncat(last,letter,1);
				continue;
			}

			matches = 0;
			strncat(temp,last, sizeof(temp));
			strncat(temp,letter,1);
			last[0] = '\0';
			name = vch->name;
		}

		if(ch!=vch)
		{
			discreetify(temp, discreet);
			emote_say_colouring(discreet, ch);

			if(((ch->on && vch->on == ch->on && IS_SET(ch->comm, COMM_DISCREET))
					|| ch->on == NULL
					|| !IS_SET(ch->comm, COMM_DISCREET)
					|| IS_AFFECTED(vch, AFF_HSENSES)
					|| IS_SET(vch->affected_by, AFF_HSENSES))
					&& !IS_SET(ch->comm, COMM_DEAF))
			{
				if(!str_prefix("'s ", release))
				{
					act( Format("%s$n$t{x", ch->colours[1][0]=='\0'?"{c":ch->colours[1]), ch, temp, vch, TO_VICT, 0 );
				}
				else
				{
					act( Format("%s$n $t{x",ch->colours[1][0]=='\0'?"{c":ch->colours[1]), ch, temp, vch, TO_VICT, 0 );
				}
			}
			else
			{
				if(!str_prefix("'s ", release))
				{
					act( Format("%s$n$t{x", ch->colours[1][0]=='\0'?"{c":ch->colours[1]), ch, discreet, vch, TO_VICT, 0 );
				}
				else
				{
					act( Format("%s$n $t{x", ch->colours[1][0]=='\0'?"{c":ch->colours[1]), ch, discreet, vch, TO_VICT, 0 );
				}
			}
		}
		else { act( Format("%s$n $t{x", ch->colours[1][0]=='\0'?"{c":ch->colours[1]), ch, release, NULL, TO_CHAR, 0 ); }
	}

	if(IS_AFFECTED(ch, AFF_HIDE) || IS_AFFECTED(ch, AFF_INVISIBLE)
			|| IS_AFFECTED(ch, AFF_SNEAK))
		do_visible(ch,"");

	return;
}


void do_bug( CHAR_DATA *ch, char *argument )
{
	append_file( ch, BUG_FILE, argument );
	if(ch!=NULL)
		send_to_char( "Bug logged.\n\r", ch );
	return;
}


void do_quit( CHAR_DATA *ch, char *argument )
{
	DESCRIPTOR_DATA *d,*d_next;
	int id = 0;

	CheckCH(ch);
	CheckChNPC(ch);

	if ( ch->position == P_FIGHT )
	{
		send_to_char( "No way! You are fighting.\n\r", ch );
		return;
	}

	if ( ch->position  < P_STUN  )
	{
		send_to_char( "You're not DEAD yet.\n\r", ch );
		return;
	}

	send_to_char("Parting is such sweet sorrow.\n\r",ch);
	act( "$n has left the game.", ch, NULL, NULL, TO_ROOM, 0 );
	log_string( LOG_COMMAND, Format("%s has quit the game.", ch->name));
	wiznet("\tY[WIZNET]\tn $N rejoins the real world.",ch,NULL,WIZ_LOGINS,0,get_trust(ch));

	/*
	 * Ditch buildermode mobs and objects.
	 */
	if(IS_SET(ch->comm, COMM_BUILDER))
	{
		cleanse_builder_stuff(ch);
		REMOVE_BIT(ch->comm, COMM_BUILDER);
	}

	/* Turn off AFK if they are not logged in. */
	if (IS_SET(ch->comm,COMM_AFK))
	{
		REMOVE_BIT(ch->comm,COMM_AFK);
	}

	/*
	 * After extract_char the ch is no longer valid!
	 */
	save_char_obj( ch );
	id = ch->id;
	d = ch->desc;
	extract_char( ch, TRUE );
	if ( d != NULL )
		close_socket( d );

	/* toast evil cheating bastards */
	for (d = descriptor_list; d != NULL; d = d_next)
	{
		CHAR_DATA *tch;

		d_next = d->next;
		tch = d->original ? d->original : d->character;
		if (tch && tch->id == id)
		{
			extract_char(tch,TRUE);
			close_socket(d);
		}
	}

	return;
}


void do_save( CHAR_DATA *ch, char *argument )
{
	CheckCH(ch);
	CheckChNPC(ch);

	save_char_obj( ch );
	if(str_prefix(argument, "no_messages"))
		send_to_char("Saving...\n\r", ch);
	return;
}


void do_shadow( CHAR_DATA *ch, char *argument )
{
	char arg[MIL]={'\0'};
	CHAR_DATA *victim;
	int var_successes = 0, k = 0;
	int var_difficulty = 0;

	CheckCH(ch);

	one_argument( argument, arg );

	if ( IS_NULLSTR(arg) )
	{
		send_to_char( "Shadow whom?\n\r", ch );
		return;
	}

	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL )
	{
		act( "But you couldn't leave $N!", ch, NULL, ch->master, TO_CHAR, 0 );
		return;
	}

	if ( victim == ch )
	{
		if ( ch->master == NULL )
		{
			send_to_char( "Trying to be your own shadow hey?\n\r", ch );
			send_to_char( "These nice men in the white coats will take good care of you.\n\r", ch );
			return;
		}
		stop_follower(ch);
		return;
	}

	if(IS_AFFECTED(ch, AFF_HIDE))
		k++;
	if(IS_AFFECTED(ch, AFF_HIDE))
		k += 2;


	var_difficulty = dice_rolls( victim, get_curr_stat(victim, STAT_PER) + victim->ability[ALERTNESS].value, 6);
	var_successes = dice_rolls(ch, get_curr_stat(ch, STAT_DEX) + ch->ability[STEALTH].value + k, 6);
	if(var_successes > var_difficulty)
	{
		if ( ch->master != NULL )
			stop_follower( ch );

		add_follower( ch, victim );
	}
	else if(var_successes <= var_difficulty)
	{
		act("$n tries to tail you!", ch, NULL, victim, TO_VICT, 0);
		act("$N spots you!", ch, NULL, victim, TO_CHAR, 0);
		if(IS_NPC(victim) && SAME_PLANE(ch, victim))
		{
			strike(victim, ch);
		}
	}
	else
	{
		send_to_char("You worry too much that you'll be spotted.\n\r", ch);
	}
	return;
}


void do_follow( CHAR_DATA *ch, char *argument )
{
	char arg[MIL]={'\0'};
	CHAR_DATA *victim;

	CheckCH(ch);

	one_argument( argument, arg );

	if ( IS_NULLSTR(arg) )
	{
		send_to_char( "Follow whom?\n\r", ch );
		return;
	}

	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL )
	{
		act( "But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR, 0 );
		return;
	}

	if ( victim == ch )
	{
		if ( ch->master == NULL )
		{
			send_to_char( "You already follow yourself.\n\r", ch );
			return;
		}
		stop_follower(ch);
		return;
	}

	if (!IS_NPC(victim) && IS_SET(victim->plr_flags,PLR_NOFOLLOW)
			&& !IS_ADMIN(ch))
	{
		act("$N doesn't seem to want any followers.\n\r",
				ch,NULL,victim, TO_CHAR, 0);
		return;
	}

	REMOVE_BIT(ch->plr_flags,PLR_NOFOLLOW);

	if ( ch->master != NULL )
		stop_follower( ch );

	add_follower( ch, victim );
	return;
}


void add_follower( CHAR_DATA *ch, CHAR_DATA *master )
{
	if ( ch->master != NULL )
	{
		log_string(LOG_BUG, "Add_follower: non-null master.");
		return;
	}

	ch->master        = master;
	ch->leader        = NULL;

	if ( can_see( master, ch ) )
		act( "$n now follows you.", ch, NULL, master, TO_VICT, 0 );

	act( "You now follow $N.",  ch, NULL, master, TO_CHAR, 0 );

	return;
}



void stop_follower( CHAR_DATA *ch )
{
	if ( ch->master == NULL )
	{
		log_string(LOG_BUG, " Stop_follower: null master.");
		return;
	}

	if ( IS_AFFECTED(ch, AFF_CHARM) )
	{
		REMOVE_BIT( ch->affected_by, AFF_CHARM );
		affect_strip( ch, gsn_charm_person );
	}

	if ( can_see( ch->master, ch ) && ch->in_room != NULL)
	{
		act( "$n stops following you.",     ch, NULL, ch->master, TO_VICT, 0 );
		act( "You stop following $N.",      ch, NULL, ch->master, TO_CHAR, 0 );
	}
	if (ch->master->pet == ch)
		ch->master->pet = NULL;

	ch->master = NULL;
	ch->leader = NULL;
	return;
}

void die_follower( CHAR_DATA *ch )
{
	CHAR_DATA *fch;

	if ( ch->master != NULL )
	{
		if (ch->master->pet == ch)
			ch->master->pet = NULL;
		stop_follower( ch );
	}

	ch->leader = NULL;

	for ( fch = char_list; fch != NULL; fch = fch->next )
	{
		if ( fch->master == ch )
			stop_follower( fch );
		if ( fch->leader == ch )
			fch->leader = fch;
	}

	return;
}



void do_order( CHAR_DATA *ch, char *argument )
{
	char buf[MSL]={'\0'};
	char arg[MIL]={'\0'};
	char arg2[MIL]={'\0'};
	CHAR_DATA *victim;
	CHAR_DATA *och;
	CHAR_DATA *och_next;
	bool found = FALSE;
	bool fAll;

	CheckCH(ch);

	argument = one_argument( argument, arg );
	one_argument(argument,arg2);

	if (!str_cmp(buf,"delete") || !str_cmp(buf, "quit") || !str_cmp(buf, "concede")
			|| !str_cmp(arg2, "mob"))
	{
		send_to_char("That will NOT be done.\n\r",ch);
		return;
	}

	if ( IS_NULLSTR(arg) || IS_NULLSTR(argument) )
	{
		send_to_char( "Order whom to do what?\n\r", ch );
		return;
	}

	if ( IS_AFFECTED( ch, AFF_CHARM ) )
	{
		send_to_char( "You feel like taking, not giving, orders.\n\r", ch );
		return;
	}

	if ( !str_cmp( arg, "all" ) )
	{
		fAll   = TRUE;
		victim = NULL;
	}
	else
	{
		fAll   = FALSE;
		if ( ( victim = get_char_room( ch, arg ) ) == NULL )
		{
			send_to_char( "They aren't here.\n\r", ch );
			return;
		}

		if ( victim == ch )
		{
			send_to_char("Are you okay there ordering yourself around?\n\r",ch);
			return;
		}

		if (!IS_AFFECTED(victim, AFF_CHARM) || victim->master != ch)
		{
			send_to_char( "Do it yourself!\n\r", ch );
			return;
		}
	}

	found = FALSE;
	for ( och = ch->in_room->people; och != NULL; och = och_next )
	{
		och_next = och->next_in_room;

		if ( IS_AFFECTED(och, AFF_CHARM)
				&&   och->master == ch
				&& ( fAll || och == victim ) )
		{
			found = TRUE;
			act( Format("$n orders you to '%s'.", argument), ch, NULL, och, TO_VICT, 0 );
			if(och->wait <= 0) {
				interpret( och, argument );
			} else {
				send_to_char("You can't comply with that order right now.\n\r", och);
				act( "$N can't comply with that order yet.", ch, NULL, och, TO_CHAR, 0 );
			}
		}
	}

	if ( found )
	{
		WAIT_STATE(ch,PULSE_VIOLENCE);
		send_to_char( "Ok.\n\r", ch );
	}
	else
		send_to_char( "You have no followers here.\n\r", ch );
	return;
}



void do_group( CHAR_DATA *ch, char *argument )
{
	char arg[MIL]={'\0'};
	CHAR_DATA *victim;

	CheckCH(ch);

	one_argument( argument, arg );

	if ( IS_NULLSTR(arg) )
	{
		CHAR_DATA *gch;
		CHAR_DATA *leader;

		leader = (ch->leader != NULL) ? ch->leader : ch;
		send_to_char( Format("%s's group:\n\r", PERS(leader, ch)), ch );

		for ( gch = char_list; gch != NULL; gch = gch->next )
		{
			if ( is_same_group( gch, ch ) )
			{
				send_to_char( Format("[ %s ] %-16s %s %5d xp\n\r", IS_NPC(gch) ? "Mob" : clan_table[gch->clan].who_name, capitalize( PERS(gch, ch) ), health_string(gch), gch->exp), ch );
			}
		}
		return;
	}

	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	if ( ch->master != NULL || ( ch->leader != NULL && ch->leader != ch ) )
	{
		send_to_char( "But you are following someone else!\n\r", ch );
		return;
	}

	if ( victim->master != ch && ch != victim )
	{
		act_new("$N isn't following you.",ch,NULL,victim,TO_CHAR,P_SLEEP,1);
		return;
	}

	if (IS_AFFECTED(victim,AFF_CHARM))
	{
		send_to_char("You can't remove charmed mobs from your group.\n\r",ch);
		return;
	}

	if (IS_AFFECTED(ch,AFF_CHARM))
	{
		act_new("You like your master too much to leave $m!",
				ch,NULL,victim,TO_VICT,P_SLEEP,1);
		return;
	}

	if ( is_same_group( victim, ch ) && ch != victim )
	{
		victim->leader = NULL;
		act_new("$n removes $N from $s group.",
				ch,NULL,victim,TO_NOTVICT,P_REST,1);
		act_new("$n removes you from $s group.",
				ch,NULL,victim,TO_VICT,P_SLEEP,1);
		act_new("You remove $N from your group.",
				ch,NULL,victim,TO_CHAR,P_SLEEP,1);
		return;
	}

	victim->leader = ch;
	act_new("$N joins $n's group.",ch,NULL,victim,TO_NOTVICT,P_REST,1);
	act_new("You join $n's group.",ch,NULL,victim,TO_VICT,P_SLEEP,1);
	act_new("$N joins your group.",ch,NULL,victim,TO_CHAR,P_SLEEP,1);
	return;
}


void do_gtell( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *gch;

	CheckCH(ch);

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Tell your group what?\n\r", ch );
		return;
	}

	if ( IS_SET( ch->comm, COMM_NOTELL ) )
	{
		send_to_char( "Your message didn't get through!\n\r", ch );
		return;
	}

	for ( gch = char_list; gch != NULL; gch = gch->next )
	{
		if ( is_same_group( gch, ch ) )
			act_new("$n tells the group '$t'",
					ch,argument,gch,TO_VICT,P_SLEEP,1);
	}

	act_new("You tell the group '$t'",
			ch,argument,gch,TO_CHAR,P_SLEEP,1);

	return;
}



/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group( CHAR_DATA *ach, CHAR_DATA *bch )
{
	if ( ach == NULL || bch == NULL)
		return FALSE;

	if ( ach->leader != NULL ) ach = ach->leader;
	if ( bch->leader != NULL ) bch = bch->leader;
	return ach == bch;
}

void do_discreet(CHAR_DATA *ch, char *argument)
{
	CheckCH(ch);

	if(!IS_SET(ch->comm, COMM_DISCREET))
	{
		SET_BIT(ch->comm, COMM_DISCREET);
		send_to_char("You will now lower your voice when talking on or at furniture.\n\r", ch);
		return;
	}
	else
	{
		REMOVE_BIT(ch->comm, COMM_DISCREET);
		send_to_char("You no longer lower your voice when talking on or at furniture.\n\r", ch);
		return;
	}
}

void do_chat(CHAR_DATA *ch, char *argument)
{
	CheckCH(ch);
	send_to_char("Global channels are limited to oocchat in Project Twilight.\n\r",	ch);
}

void do_rp_ok(CHAR_DATA *ch, char *argument)
{
	CheckCH(ch);

	if(IS_NPC(ch))
	{
		return;
	}

	if(IS_SET(ch->plr_flags, PLR_RP_OK) && IS_NULLSTR(argument))
	{
		send_to_char("People will no longer be able to join you for RP.\n\r", ch);
		REMOVE_BIT(ch->plr_flags, PLR_RP_OK);
		PURGE_DATA(ch->pcdata->rpok_string);
		ch->pcdata->rpok_string = str_dup("Not Available");
		return;
	}
	else
	{
		send_to_char("You can now be joined by others for RP.\n\r", ch);
		SET_BIT(ch->plr_flags, PLR_RP_OK);
		if(!IS_NULLSTR(argument))
		{
			send_to_char(Format("Your RP title has been set to: %s\n\r", argument), ch);
			PURGE_DATA(ch->pcdata->rpok_string);
			ch->pcdata->rpok_string = str_dup(argument);
		}
	}
}

void do_rpmode(CHAR_DATA *ch, char *argument)
{
	CheckCH(ch);

	if(IS_SET(ch->act2, ACT2_RP_ING))
	{
		send_to_char("RP mode de-activated.\n\r", ch);
		REMOVE_BIT(ch->act2, ACT2_RP_ING);
		return;
	}

	send_to_char("RP mode activated.\n\r", ch);
	SET_BIT(ch->act2, ACT2_RP_ING);
}

/* afk command */
void do_afk ( CHAR_DATA *ch, char * argument)
{
	CheckCH(ch);

	if (IS_SET(ch->comm,COMM_AFK))
	{
		send_to_char("AFK mode removed. Type 'replay' to see tells.\n\r",ch);
		REMOVE_BIT(ch->comm,COMM_AFK);
		act("$n returns to the keyboard.", ch, NULL, NULL, TO_ROOM, 0);
	}
	else
	{
		send_to_char("You are now in AFK mode.\n\r",ch);
		SET_BIT(ch->comm,COMM_AFK);
		act("$n goes afk.", ch, NULL, NULL, TO_ROOM, 0);
	}
}

void do_replay (CHAR_DATA *ch, char *argument)
{
	CheckCH(ch);

	if (IS_NPC(ch))
	{
		send_to_char("You can't replay.\n\r",ch);
		return;
	}

	if ( !ch->pcdata ->buffer || IS_NULLSTR(ch->pcdata->buffer->string) )
	{
		send_to_char("You have no tells to replay.\n\r",ch);
		return;
	}

	page_to_char(buf_string(ch->pcdata->buffer),ch);
	clear_buf(ch->pcdata->buffer);
}

void do_hangup(CHAR_DATA *ch, char *argument)
{
	CheckCH(ch);
	do_call(ch, "end");
}

void send_email_confirmation( CHAR_DATA *ch )
{
	char buf[MSL]={'\0'};
	snprintf(buf, sizeof(buf), "../scripts/emailconfirm.pl %s %s %ld", ch->email_addr, ch->name, ch->email_lock);
}

void set_email( CHAR_DATA *ch, char *email )
{
	char buf[MSL]={'\0'};

	CheckChNPC(ch);

	strncpy( buf, email, sizeof(buf));
	PURGE_DATA( ch->email_addr );
	ch->email_addr = str_dup( buf );
	send_email_confirmation( ch );
	return;
}

void do_email(CHAR_DATA *ch, char *argument)
{
	char arg[MIL]={'\0'};

	CheckCH(ch);

	if(!IS_NULLSTR(argument))
	{
		if(ch->email_lock > 0)
		{
			send_to_char( "You need to enter the lock code from the email sent.\n\r", ch);
			send_to_char("To change the submitted email address use: \tCemail [address]\tn\n\r", ch);
			return;
		}
		else
		{
			send_to_char("Syntax: \tCemail [address]\tn\n\r", ch);
			return;
		}
	}

	one_argument(argument, arg);

	if(is_number(arg))
	{
		long lock;
		if(ch->email_lock == 0)
		{
			send_to_char( "You need to submit an email address before entering a lock code.\n\r", ch);
			send_to_char("Syntax: \tCemail [address]\tn\n\r", ch);
			return;
		}

		if((lock = atol(arg)) == ch->email_lock)
		{
			send_to_char("Email unlocked. You may now send emails from the note system.\n\r", ch);
			SET_BIT(ch->plr_flags, PLR_EMAIL);
			do_save(ch,"");
		}
		else
		{
			send_to_char("That is not the correct email lock code.\n\r", ch);
		}

		return;
	}

	if(!strchr(arg, '@') && !strchr(strchr(arg, '@'), '.'))
	{
		send_to_char("That doesn't even look like a valid email address.\n\r", ch);
		return;
	}

	smash_tilde( arg );
	set_email( ch, arg );
	act( "Email address set to: $t\n\r", ch, ch->email_addr, NULL, TO_CHAR, 1 );
	if(IS_SET(ch->plr_flags, PLR_EMAIL)) REMOVE_BIT(ch->plr_flags, PLR_EMAIL);
	do_save(ch,"");
}

void do_ignore(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *vch;
	int online = 0, in_char_list = 0;

	CheckCH(ch);

	if(IS_NULLSTR(argument))
	{
		send_to_char("Syntax: ignore [name]\n\r", ch);
		return;
	}

	online = is_online(argument);
	in_char_list = pc_in_char_list(argument);
	if((vch = get_char_world(ch, argument)) == NULL)
	{
		send_to_char("That player does not exist.\n\r", ch);
		return;
	}

	if ( strstr( ch->ignore, vch->name ) != '\0' )
	{
		ch->ignore = string_replace( ch->ignore, vch->name, "" );
		ch->ignore = string_unpad( ch->ignore );

		if ( IS_NULLSTR(ch->ignore) )
		{
			PURGE_DATA( ch->ignore );
			ch->ignore = str_dup( "None" );
		}
		send_to_char( "Ignore status removed.\n\r", ch );
	}
	else
	{
		char buf[MSL]={'\0'};
		
		buf[0] = '\0';
		if ( strstr(ch->ignore, "None") != '\0' )
		{
			ch->ignore = string_replace( ch->ignore, "None", "\0" );
			ch->ignore = string_unpad( ch->ignore );
		}

		if (!IS_NULLSTR(ch->ignore))
		{
			strncat( buf, ch->ignore, sizeof(buf) );
			strncat( buf, " ", sizeof(buf) );
		}
		strncat( buf, vch->name, sizeof(buf) );
		PURGE_DATA( ch->ignore );
		ch->ignore = string_proper( str_dup( buf ) );
		send_to_char( "Ignore block activated.\n\r", ch );
	}

	if(!online && !in_char_list) free_char(vch);
}

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

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <time.h>
#include "twilight.h"

/* does aliasing and other fun stuff */
void substitute_alias(DESCRIPTOR_DATA *d, char *argument)
{
	CHAR_DATA *ch;
    char buf[MSL]={'\0'};
	char prefix[MIL]={'\0'};
	char name[MIL]={'\0'};
	char *point;
	int alias = 0;

	ch = d->original ? d->original : d->character;

	/* check for prefix */
	if (!IS_NULLSTR(ch->prefix) && str_prefix("prefix",argument))
	{
		if (strlen(ch->prefix) + strlen(argument) > MAX_INPUT_LENGTH - 2)
			send_to_char("Line to long, prefix not processed.\r\n",ch);
		else
		{
			snprintf(prefix, sizeof(prefix), "%s %s",ch->prefix,argument);
			argument = prefix;
		}
	}

	if (IS_NPC(ch) || IS_NULLSTR(ch->pcdata->alias[0])
			||	!str_prefix("alias",argument) || !str_prefix("una",argument)
			||  !str_prefix("prefix",argument))
	{
		interpret(d->character,argument);
		return;
	}

	strncpy(buf,argument, sizeof(buf));

	for (alias = 0; alias < MAX_ALIAS; alias++)	 /* go through the aliases */
	{
		if (ch->pcdata->alias[alias] == NULL)
			break;

		if (!str_prefix(ch->pcdata->alias[alias],argument))
		{
			point = one_argument(argument,name);
			if (!str_cmp(ch->pcdata->alias[alias],name))
			{
				buf[0] = '\0';
				strncat(buf,ch->pcdata->alias_sub[alias], sizeof(buf));
				strncat(buf," ", sizeof(buf));
				strncat(buf,point, sizeof(buf));

				if (strlen(buf) > MAX_INPUT_LENGTH - 1)
				{
					send_to_char( "Alias substitution too long. Truncated.\r\n",ch);
					buf[MAX_INPUT_LENGTH -1] = '\0';
				}
				break;
			}
		}
	}
	interpret(d->character,buf);
}

void do_alia(CHAR_DATA *ch, char *argument)
{
	send_to_char("I'm sorry, alias must be entered in full.\n\r",ch);
	return;
}

void do_alias(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *rch;
	char arg[MIL]={'\0'};
	int pos = 0;

	smash_tilde(argument);

	if (ch->desc == NULL)
		rch = ch;
	else
		rch = ch->desc->original ? ch->desc->original : ch;

	if (IS_NPC(rch))
		return;

	argument = one_argument(argument,arg);


	if (IS_NULLSTR(arg))
	{

		if (IS_NULLSTR(rch->pcdata->alias[0]))
		{
			send_to_char("You have no aliases defined.\n\r",ch);
			return;
		}
		send_to_char("Your current aliases are:\n\r",ch);

		for (pos = 0; pos < MAX_ALIAS; pos++)
		{
			if (rch->pcdata->alias[pos] == NULL
					||	rch->pcdata->alias_sub[pos] == NULL)
				break;

			send_to_char(Format("    %s:  %s\n\r",rch->pcdata->alias[pos], rch->pcdata->alias_sub[pos]),ch);
		}
		return;
	}

	if (!str_prefix("una",arg) || !str_cmp("alias",arg))
	{
		send_to_char("Sorry, that word is reserved.\n\r",ch);
		return;
	}

	if (strchr(arg,' ')||strchr(arg,'"')||strchr(arg,'\'')) {
		send_to_char("The word to be aliased should not contain a space, a tick or a double-quote.\n\r",ch);
		return;
	}

	if (IS_NULLSTR(argument))
	{
		for (pos = 0; pos < MAX_ALIAS; pos++)
		{
			if (rch->pcdata->alias[pos] == NULL
					||	rch->pcdata->alias_sub[pos] == NULL)
				break;

			if (!str_cmp(arg,rch->pcdata->alias[pos]))
			{
				send_to_char(Format("%s aliases to '%s'.\n\r",rch->pcdata->alias[pos], rch->pcdata->alias_sub[pos]),ch);
				return;
			}
		}

		send_to_char("That alias is not defined.\n\r",ch);
		return;
	}

	if (!str_prefix(argument,"delete") || !str_prefix(argument,"prefix"))
	{
		send_to_char("That shall not be done!\n\r",ch);
		return;
	}

	for (pos = 0; pos < MAX_ALIAS; pos++)
	{
		if (rch->pcdata->alias[pos] == NULL)
			break;

		if (!str_cmp(arg,rch->pcdata->alias[pos])) /* redefine an alias */
		{
			PURGE_DATA(rch->pcdata->alias_sub[pos]);
			rch->pcdata->alias_sub[pos] = str_dup(argument);
			send_to_char(Format("%s is now realiased to '%s'.\n\r",arg,argument),ch);
			return;
		}
	}

	if (pos >= MAX_ALIAS)
	{
		send_to_char("Sorry, you have reached the alias limit.\n\r",ch);
		return;
	}

	/* make a new alias */
	PURGE_DATA( rch->pcdata->alias[pos] );
	PURGE_DATA( rch->pcdata->alias_sub[pos] );
	rch->pcdata->alias[pos]		= str_dup(arg);
	rch->pcdata->alias_sub[pos]	= str_dup(argument);
	send_to_char(Format("%s is now aliased to '%s'.\n\r",arg,argument),ch);
}


void do_unalias(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *rch;
	char arg[MIL]={'\0'};
	int pos = 0;
	bool found = FALSE;

	if (ch->desc == NULL)
		rch = ch;
	else
		rch = ch->desc->original ? ch->desc->original : ch;

	if (IS_NPC(rch))
		return;

	argument = one_argument(argument,arg);

	if (IS_NULLSTR(arg))
	{
		send_to_char("Unalias what?\n\r",ch);
		return;
	}

	for (pos = 0; pos < MAX_ALIAS; pos++)
	{
		if (rch->pcdata->alias[pos] == NULL)
			break;

		if (found)
		{
			rch->pcdata->alias[pos-1]		= rch->pcdata->alias[pos];
			rch->pcdata->alias_sub[pos-1]	= rch->pcdata->alias_sub[pos];
			rch->pcdata->alias[pos]			= NULL;
			rch->pcdata->alias_sub[pos]		= NULL;
			continue;
		}

		if(!str_cmp(arg,rch->pcdata->alias[pos]))
		{
			send_to_char("Alias removed.\n\r",ch);
			PURGE_DATA(rch->pcdata->alias[pos]);
			PURGE_DATA(rch->pcdata->alias_sub[pos]);
			rch->pcdata->alias[pos] = NULL;
			rch->pcdata->alias_sub[pos] = NULL;
			found = TRUE;
		}
	}

	if (!found)
		send_to_char("No alias of that name to remove.\n\r",ch);
}

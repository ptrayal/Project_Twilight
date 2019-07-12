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
 *                                                                         *
 *  ROM 2.4 is copyright 1993-1998 Russ Taylor                             *
 *  ROM has been brought to you by the ROM consortium                      *
 *      Russ Taylor (rtaylor@hypercube.org)                                *
 *      Gabrielle Taylor (gtaylor@hypercube.org)                           *
 *      Brian Moore (zump@rom.org)                                         *
 *  By using this code, you have agreed to follow the terms of the         *
 *  ROM license, in the file Rom24/doc/rom.license                         *
 *                                                                         *
 * The following code is based on ILAB OLC by Jason Dinkel.                *
 * Mobprogram code by Lordrom for Nevermore.                               *
 *                                                                         *
 * Much of the code is the original work of Peter Fitzgerald who turned    *
 * it over to Brandon Morrison who has adopted and improved the code.      *
 * Copyright (C) 2012 - 2019                                               *
 **************************************************************************/

#if defined(Macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "twilight.h"
#include "tables.h"
#include "olc.h"
#include "recycle.h"

#define MPEDIT( fun )           bool fun(CHAR_DATA *ch, char*argument)


const struct olc_cmd_type mpedit_table[] =
{
/*	{	command		function	}, */

	{	"commands",	show_commands	},
	{	"create",	mpedit_create	},
	{	"code",		mpedit_code	},
	{	"show",		mpedit_show	},
	{	"list",		mpedit_list	},
	{	"?",		show_help	},

	{	NULL,		0		}
};

void mpedit( CHAR_DATA *ch, char *argument)
{
    MPROG_CODE *pMcode;
    char arg[MAX_INPUT_LENGTH]={'\0'};
    char command[MAX_INPUT_LENGTH]={'\0'};
    int cmd = 0;
    AREA_DATA *ad;

    smash_tilde(argument);
    strncpy(arg, argument, sizeof(arg));
    argument = one_argument( argument, command);

    EDIT_MPCODE(ch, pMcode);

    if (pMcode)
    {
	ad = get_vnum_area( pMcode->vnum );

	if ( ad == NULL ) /* ??? */
	{
		edit_done(ch);
		return;
	}

	if ( !IS_BUILDER(ch, ad) )
	{
		send_to_char("MPEdit: Insufficient security to modify code.\n\r", ch);
		edit_done(ch);
		return;
	}
    }

    if (IS_NULLSTR(command))
    {
        mpedit_show(ch, argument);
        return;
    }

    if (!str_cmp(command, "done") )
    {
        edit_done(ch);
        return;
    }

    for (cmd = 0; mpedit_table[cmd].name != NULL; cmd++)
    {
	if (!str_prefix(command, mpedit_table[cmd].name) )
	{
		if ((*mpedit_table[cmd].olc_fun) (ch, argument) && pMcode)
			if ((ad = get_vnum_area(pMcode->vnum)) != NULL)
				SET_BIT(ad->area_flags, AREA_CHANGED);
		return;
	}
    }

    interpret(ch, arg);

    return;
}

void do_mpedit(CHAR_DATA *ch, char *argument)
{
    MPROG_CODE *pMcode;
    char command[MAX_INPUT_LENGTH]={'\0'};

    argument = one_argument(argument, command);

    if( is_number(command) )
    {
	int vnum = atoi(command);
	AREA_DATA *ad;

	if ( (pMcode = get_mprog_index(vnum)) == NULL )
	{
		send_to_char("MPEdit : That vnum does not exist.\n\r",ch);
		return;
	}

	ad = get_vnum_area(vnum);

	if ( ad == NULL )
	{
		send_to_char( "MPEdit : That vnum has not been assigned to an area.\n\r", ch );
		return;
	}

	if ( !IS_BUILDER(ch, ad) )
	{
		send_to_char("MPEdit : You do not have access to that area.\n\r", ch );
		return;
	}

	ch->desc->pEdit		= (void *)pMcode;
	ch->desc->editor	= ED_MPCODE;

	return;
    }

    if ( !str_cmp(command, "create") )
    {
	if (IS_NULLSTR(argument))
	{
		send_to_char( "Syntax : mpedit create [vnum]\n\r", ch );
		return;
	}

	mpedit_create(ch, argument);
	return;
    }

    send_to_char( "Syntax : mpedit [vnum]\n\r", ch );
    send_to_char( "         mpedit create [vnum]\n\r", ch );

    return;
}

MPEDIT (mpedit_create)
{
    MPROG_CODE *pMcode;
    int value = atoi(argument);
    AREA_DATA *ad;

    if (IS_NULLSTR(argument) || value < 1)
    {
	send_to_char( "Syntax : mpedit create [vnum]\n\r", ch );
	return FALSE;
    }

    ad = get_vnum_area(value);

    if ( ad == NULL )
    {
    	send_to_char( "MPEdit : VNUM not assigned to any area.\n\r", ch );
    	return FALSE;
    }
    
    if ( !IS_BUILDER(ch, ad) )
    {
        send_to_char("MPEdit : Insufficient security to create MobProgs.\n\r", ch);
        return FALSE;
    }

    if ( get_mprog_index(value) )
    {
	send_to_char("MPEdit: Code vnum already exists.\n\r",ch);
	return FALSE;
    }

    pMcode			= new_mpcode();
    pMcode->vnum		= value;
    LINK_SINGLE(pMcode, next, mprog_list);

    ch->desc->pEdit		= (void *)pMcode;
    ch->desc->editor		= ED_MPCODE;

    send_to_char("MobProgram Code Created.\n\r",ch);

    return TRUE;
}

MPEDIT(mpedit_show)
{
    MPROG_CODE *pMcode;

    EDIT_MPCODE(ch,pMcode);

    send_to_char(Format("Vnum:       [%d]\n\rCode:\n\r%s\n\r", pMcode->vnum, pMcode->code), ch);

    return FALSE;
}

MPEDIT(mpedit_code)
{
    MPROG_CODE *pMcode;
    EDIT_MPCODE(ch, pMcode);

    if (IS_NULLSTR(argument) )
    {
       string_append(ch, &pMcode->code);
       return TRUE;
    }

    send_to_char("Syntax: code\n\r",ch);
    return FALSE;
}

MPEDIT( mpedit_list )
{
    MPROG_CODE *mprg;
    BUFFER *buffer;
    AREA_DATA *ad;
    char blah;
    int count = 1;
    bool fAll = !str_cmp(argument, "all");

    buffer = new_buf();

    for (mprg = mprog_list; mprg !=NULL; mprg = mprg->next)
	if ( fAll || ENTRE(ch->in_room->area->min_vnum, mprg->vnum, ch->in_room->area->max_vnum) )
	{
		ad = get_vnum_area(mprg->vnum);

		if ( ad == NULL )
			blah = '?';
		else
		if ( IS_BUILDER(ch, ad) )
			blah = '*';
		else
			blah = ' ';

		add_buf(buffer, (char *)Format("[%3d] (%c) %5d\n\r", count, blah, mprg->vnum));

		count++;
	}

    if ( count == 1 )
    {
    	if ( fAll )
    		add_buf( buffer, "There are no MobPrograms.\n\r" );
    	else
    		add_buf( buffer, "There are no MobPrograms in this area.\n\r" );
    }

    page_to_char(buf_string(buffer), ch);
    free_buf(buffer);

    return FALSE;
}

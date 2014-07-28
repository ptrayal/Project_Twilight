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
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include "twilight.h"
#include "tables.h"
#include "recycle.h"


BUFFER *buffer_list;
int top_buffer;
long    last_pc_id;
long    last_mob_id;
extern          int                     top_reset;
extern          int                     top_area;
extern          int                     top_exit;
extern          int                     top_ed;
extern          int                     top_room;


/* Other Funcions
 * - This is where I'm putting all functions and such that are not specifically
 *   new or free.  It'll hopefully help bring some organization to the file.
 *   -Rayal
 */

long get_pc_id(void)
{
    int val = 0;

    val = (current_time <= last_pc_id) ? last_pc_id + 1 : current_time;
    last_pc_id = val;
    return val;
}

long get_mob_id(void)
{
    last_mob_id++;
    return last_mob_id;
}

const int buf_size[MAX_BUF_LIST] =
{
		16,32,64,128,256,1024,2048,4096,8192,16384
};

int get_size (int val)
{
	int i = 0;

	for (i = 0; i < MAX_BUF_LIST; i++)
		if (buf_size[i] >= val)
		{
			return buf_size[i];
		}

	return -1;
}

void free_extra_descr( EXTRA_DESCR_DATA *pExtra );
void free_affect( AFFECT_DATA *af );


/* Stuff for Recycling  - Consolidation, ain't it grand.*/
BAN_DATA		*ban_list;

/* Start all the fun stuff */
ACTOR_DATA *new_actor()
{
	ACTOR_DATA *actor;

	ALLOC_DATA(actor, ACTOR_DATA, 1);

	actor->mob = 0;

	return actor;
}

void free_actor(ACTOR_DATA *actor)
{
	Escape(actor);

	actor->next = NULL;
	actor->mob = 0;
	PURGE_DATA(actor);
}


HELP_DATA *new_help()
{
	HELP_DATA *help;

	ALLOC_DATA(help, HELP_DATA, 1);

	help->level = 0;
	help->keyword = NULL;
	help->races = NULL;
	help->clans = NULL;
	help->topic = NULL;
	help->syntax = NULL;
	help->description = NULL;
	help->unformatted = NULL;
	help->see_also = NULL;
	help->quote = NULL;
	help->website = NULL;

	return help;
}

void free_help(HELP_DATA *help)
{
	Escape(help);

	PURGE_DATA( help->clans );
	PURGE_DATA( help->description );
	PURGE_DATA( help->keyword );
	PURGE_DATA( help->quote	);
	PURGE_DATA( help->races	);
	PURGE_DATA( help->see_also );
	PURGE_DATA( help->syntax );
	PURGE_DATA( help->topic	);
	PURGE_DATA( help->unformatted );
	PURGE_DATA( help->website );
	help->level = 0;
	PURGE_DATA( help );
}

    
NEWSPAPER *new_newspaper()
{
	NEWSPAPER *paper;
	int i = 0;

	ALLOC_DATA(paper, NEWSPAPER, 1);

	paper->name = NULL;
	paper->cost = 0;
	paper->on_stands = 0;
	for(i=0;i<MAX_ARTICLES;i++)
		paper->articles[i] = -1;

	return paper;
}

void free_newspaper(NEWSPAPER *paper)
{
	Escape(paper);

	PURGE_DATA( paper->name );
	paper->cost = 0;
	paper->on_stands = 0;


	PURGE_DATA(paper);
}

    
NOTE_DATA *new_note()
{
	NOTE_DATA *note;

	ALLOC_DATA(note, NOTE_DATA, 1);

	note->race = 0;
	note->successes = 1;
	note->type = 0;
	note->date = NULL;
	note->sender = NULL;
	note->to_list = NULL;
	note->subject = NULL;
	note->text = NULL;

	return note;
}

void free_note(NOTE_DATA *note)
{
	Escape(note);

	PURGE_DATA( note->date    );
	PURGE_DATA( note->sender  );
	PURGE_DATA( note->subject );
	PURGE_DATA( note->text    );
	PURGE_DATA( note->to_list );

	PURGE_DATA( note );
}

    
BAN_DATA *new_ban(void)
{
	BAN_DATA *ban;

	ALLOC_DATA(ban, BAN_DATA, 1);

	ban->name = NULL;
	ban->ban_flags = 0;
	ban->level = 0;

	return ban;
}

void free_ban(BAN_DATA *ban)
{
	Escape(ban);

	PURGE_DATA(ban->name);

	PURGE_DATA(ban);
}


DESCRIPTOR_DATA *new_descriptor(void)
{
	DESCRIPTOR_DATA *d;

	ALLOC_DATA(d, DESCRIPTOR_DATA, 1);

	d->connected		= CON_GET_NAME;
	d->host				= NULL;
	d->pEdit			= NULL;
	d->pString			= NULL;
	d->showstr_head		= NULL;
	d->showstr_point	= NULL;
	d->outsize			= 2000;
	d->editor			= 0;
	d->descriptor		= 0;
	d->fcommand			= FALSE;
	d->repeat			= 0;
	d->outtop			= 0;
	d->pProtocol			= NULL;
	ALLOC_DATA(d->outbuf, char, d->outsize);

	return d;
}

void free_descriptor(DESCRIPTOR_DATA *d)
{
	Escape(d);

	PURGE_DATA(d->host );
	PURGE_DATA(d->pEdit);
	PURGE_DATA(d->pString);
	PURGE_DATA(d->outbuf );
	PURGE_DATA(d->showstr_head );
	PURGE_DATA(d->showstr_point);
	PURGE_DATA(d);
}


GEN_DATA *new_gen_data(void)
{
	GEN_DATA *gen;

	ALLOC_DATA(gen, GEN_DATA, 1);

	gen->freebies	= 0;
	gen->points_chosen	= 0;
	gen->virtue_dots	= 0;
	return gen;
}

void free_gen_data(GEN_DATA *gen)
{
	Escape(gen);
	PURGE_DATA(gen);
}


QUEST_DATA *new_quest()
{
	QUEST_DATA *quest;

	ALLOC_DATA(quest, QUEST_DATA, 1);

	quest->quest_flags  = 0;
	quest->time_limit	= 0;
	quest->quest_type	= 0;
	quest->state	= 0;

	return quest;
}

void free_quest(QUEST_DATA *quest)
{
	Escape(quest);

	quest->quest_flags  = 0;
	quest->time_limit	= 0;
	quest->quest_type	= 0;
	quest->state	= 0;
	quest->obj		= NULL;
	PURGE_DATA(quest);
}


REACT_DATA *new_react_data()
{
	REACT_DATA *script;

	ALLOC_DATA(script, REACT_DATA, 1);

	script->priority	= 0;
	script->reaction	= NULL;

	return script;
}

void free_react_data(REACT_DATA *script)
{
	Escape(script);

	PURGE_DATA(script->reaction);
	PURGE_DATA(script);
}


REACT *new_react()
{
	REACT *react;
	ALLOC_DATA(react, REACT, 1);

	react->trig		= NULL;
	react->attitude	= 0;

	return react;
}

void free_react(REACT *react)
{
	REACT_DATA *reactd;
	REACT_DATA *next;
	REACT_DATA *pe;

	Escape(react);

	react->attitude = -1;
	PURGE_DATA(react->trig);

	for(reactd = react->react; reactd; reactd = next)
	{
		next = reactd->next;
		if(reactd == react_data_first)
		{
			react_data_first = react_data_first->next;
		}
		else
		{
			for(pe = react_data_first; pe->next != NULL; pe = pe->next)
			{
				if(pe->next == reactd)
				{
					pe->next = reactd->next;
					break;
				}
			}
		}
		free_react_data(reactd);
	}
	PURGE_DATA(react);
}


PERSONA *new_persona()
{
	PERSONA *persona;

	ALLOC_DATA(persona, PERSONA, 1);

	persona->name	= NULL;
	persona->vnum	= 0;

	return persona;
}

void free_persona(PERSONA *persona)
{
	REACT *react;
	REACT *next;
	REACT *pe;
	int i = 0;

	Escape(persona);

	persona->vnum = -1;
	PURGE_DATA(persona->name);
	for(i = 0; i <= MAX_ATTITUDE; i++)
		for(react = persona->matrix[i]; react; react = next)
		{
			next = react->next_in_matrix_loc;
			if(react == react_first)
			{
				react_first = react_first->next;
			}
			else
			{
				for(pe = react_first; pe->next != NULL; pe = pe->next)
				{
					if(pe->next == react)
					{
						pe->next = react->next;
						break;
					}
				}
			}
			free_react(react);
		}

	PURGE_DATA(persona);
}

SCRIPT_DATA *new_script()
{
	SCRIPT_DATA *script;

	ALLOC_DATA(script, SCRIPT_DATA, 1);

	script->author		= NULL;
	script->trigger		= NULL;
	script->reaction	= NULL;
	script->active		= 0;
	script->actor		= 0;
	script->delay		= 0;
	script->first_script	= 0;
	script->vnum		= 0;


	return script;
}

void free_script(SCRIPT_DATA *script)
{

	Escape(script);

	script->vnum = -1;
	script->delay = 0;
	script->active = 0;
	script->actor = 0;
	script->event = NULL;
	PURGE_DATA(script->author);
	PURGE_DATA(script->trigger);
	PURGE_DATA(script->reaction);
	PURGE_DATA(script);
}

EVENT_DATA *new_event()
{
	EVENT_DATA *event;

	ALLOC_DATA(event, EVENT_DATA, 1);

	event->author	= NULL;
	event->title	= NULL;
	event->loc		= 0;
	event->races	= 0;
	event->stop		= 0;
	event->vnum		= 0;

	return event;
}

void free_event(EVENT_DATA *event)
{
	SCRIPT_DATA *script;
	SCRIPT_DATA *next;
	SCRIPT_DATA *ps;
	ACTOR_DATA *actor, *actor_next;

	Escape(event);

	event->vnum = -1;
	event->plot = NULL;
	PURGE_DATA(event->author);
	PURGE_DATA(event->title);
	event->races = 0;
	event->next_in_plot = NULL;

	for(script = event->script_list; script; script = next)
	{
		next = script->next_in_event;
		if(script == script_first)
		{
			script_first = script_first->next;
		}
		else
		{
			for(ps = script_first; ps->next != NULL; ps = ps->next)
			{
				if(ps->next == script)
				{
					ps->next = script->next;
					break;
				}
			}
		}
		free_script(script);
	}

	for(actor = event->actors; actor; actor = actor_next)
	{
		actor_next = actor->next;
		free_actor(actor);
	}
	event->actors = NULL;
	PURGE_DATA(event);
}

PLOT_DATA *new_plot()
{
	PLOT_DATA *plot;

	ALLOC_DATA(plot, PLOT_DATA, 1);

	plot->author	= NULL;
	plot->title		= NULL;
	plot->races		= 0;
	plot->vnum		= 0;

	return plot;
}

void free_plot(PLOT_DATA *plot)
{
	EVENT_DATA *event;
	EVENT_DATA *next;
	EVENT_DATA *pe;

	Escape(plot);

	plot->vnum = -1;
	PURGE_DATA(plot->author);
	PURGE_DATA(plot->title);
	for(event = plot->event_list; event; event = next)
	{
		next = event->next_in_plot;
		if(event == event_list)
		{
			event_list = event_list->next;
		}
		else
		{
			for(pe = event_list; pe->next != NULL; pe = pe->next)
			{
				if(pe->next == event)
				{
					pe->next = event->next;
					break;
				}
			}
		}
		free_event(event);
	}
	plot->races = 0;
	PURGE_DATA(plot);
}


EXTRA_DESCR_DATA *new_extra_descr(void)
{
	EXTRA_DESCR_DATA *ed;

	ALLOC_DATA(ed, EXTRA_DESCR_DATA, 1);

	ed->keyword = NULL;
	ed->description = NULL;
	ed->timer = 0;
	ed->ability = 0;
	ed->attribute = 0;
	ed->difficulty = 0;
	ed->successes = 0;
	return ed;
}

void free_extra_descr(EXTRA_DESCR_DATA *ed)
{

	Escape(ed);

	PURGE_DATA(ed->keyword);
	PURGE_DATA(ed->description);
	PURGE_DATA(ed);
}


COMBAT_DATA *new_combat_move(void)
{
	COMBAT_DATA *move;

	ALLOC_DATA(move, COMBAT_DATA, 1);

	return move;
}

void free_combat_move(COMBAT_DATA *move)
{

	Escape(move);
	PURGE_DATA(move);
}


AFFECT_DATA *new_affect(void)
{
	AFFECT_DATA *af;

	ALLOC_DATA(af, AFFECT_DATA, 1);

	af->bitvector	= 0;
	af->duration	= 0;
	af->level		= 0;
	af->location	= 0;
	af->modifier	= 0;
	af->type		= 0;
	af->where		= 0;

	return af;
}

void free_affect(AFFECT_DATA *af)
{
	Escape(af);
	PURGE_DATA(af);
}


OBJ_DATA *new_obj(void)
{
	OBJ_DATA *obj;

	ALLOC_DATA(obj, OBJ_DATA, 1);

	obj->BUILDER_MODE	= 0;
	obj->condition		= 100;
	obj->cost			= 0;
	obj->description	= NULL;
	obj->enchanted		= FALSE;
	obj->extra2			= 0;
	obj->extra_flags	= 0;
	obj->fetish_flags	= 0;
	obj->fetish_level	= 0;
	obj->fetish_target	= 0;
	obj->full_desc		= NULL;
	obj->item_type		= 0;
	obj->material		= NULL;
	obj->name			= NULL;
	obj->owner			= NULL;
	obj->quality		= 2;
	obj->reset_loc		= 0;
	obj->short_descr	= NULL;
	obj->timer			= -1;
	obj->to_room_none	= NULL;
	obj->to_room_other	= NULL;
	obj->to_room_self	= NULL;
	obj->to_room_used	= NULL;
	obj->to_user_none	= NULL;
	obj->to_user_other	= NULL;
	obj->to_user_self	= NULL;
	obj->to_user_used	= NULL;
	obj->to_vict_other	= NULL;
	obj->uses			= 0;
	obj->wear_flags		= 0;
	obj->wear_loc		= 0;
	obj->weight			= 0;

	return obj;
}

void free_obj(OBJ_DATA *obj)
{
	AFFECT_DATA *paf, *paf_next;
	EXTRA_DESCR_DATA *ed, *ed_next;

	Escape(obj);

	for (paf = obj->affected; paf != NULL; paf = paf_next)
	{
		paf_next = paf->next;
		free_affect(paf);
	}
	obj->affected = NULL;

	for (ed = obj->extra_descr; ed != NULL; ed = ed_next )
	{
		ed_next = ed->next;
		free_extra_descr(ed);
	}
	obj->extra_descr = NULL;

	PURGE_DATA( obj->description );
	PURGE_DATA( obj->full_desc   );
	PURGE_DATA( obj->material);
	PURGE_DATA( obj->name        );
	obj->owner = NULL;
	PURGE_DATA( obj->short_descr );

	PURGE_DATA( obj->to_room_none );
	PURGE_DATA( obj->to_room_other );
	PURGE_DATA( obj->to_room_self );
	PURGE_DATA( obj->to_room_used );
	PURGE_DATA( obj->to_user_none );
	PURGE_DATA( obj->to_user_other );
	PURGE_DATA( obj->to_user_self );
	PURGE_DATA( obj->to_user_used );
	PURGE_DATA( obj->to_vict_other );
	obj->to_user_none          =   NULL;
	obj->to_user_self          =   NULL;
	obj->to_user_other         =   NULL;
	obj->to_room_none          =   NULL;
	obj->to_room_self          =   NULL;
	obj->to_room_other         =   NULL;
	obj->to_vict_other         =   NULL;
	obj->to_room_used          =   NULL;
	obj->to_user_used		=   NULL;
	obj->uses				=   -2;

	PURGE_DATA(obj);
}


CHAR_DATA *new_char (void)
{
	CHAR_DATA *ch;
	int i = 0;

	ALLOC_DATA(ch, CHAR_DATA, 1);

	ch->hunted			= FALSE;

	ch->aifile			= NULL;
	ch->alt_description	= NULL;
	ch->alt_long_descr	= NULL;
	ch->alt_name		= NULL;
	ch->demeanor		= NULL;
	ch->description		= NULL;
	ch->email_addr		= NULL;
	ch->ghouled_by		= NULL;
	ch->ignore			= NULL;
	ch->laston			= NULL;
	ch->long_descr		= NULL;
	ch->married			= NULL;
	ch->material		= str_dup("flesh");
	ch->name			= NULL;
	ch->nature			= NULL;
	ch->next_in_room	= NULL;
	ch->oldname			= NULL;
	ch->pack			= NULL;
	ch->pet             = NULL;
	ch->prefix			= NULL;
	ch->profession		= str_dup("None");
	ch->prompt			= NULL;
	ch->reply 			= NULL;
	ch->short_descr		= NULL;
	ch->sire			= NULL;
	ch->surname			= NULL;
	ch->switch_desc		= NULL;
	ch->to_learn		= NULL;

	ch->BUILDER_MODE	= 0;
	ch->act				= 0;
	ch->act2			= 0;
	ch->act_points		= 0;
	ch->affected_by		= 0;
	ch->affected_by2	= 0;
	ch->agghealth		= 7;
	ch->attitude		= 0;
	ch->balance			= 0;
	ch->bank_account	= 0;
	ch->bg_count		= 0;
	ch->cents			= 0;
	ch->char_age		= 0;
	ch->clan			= 0;
	ch->combo_success	= 0;
	ch->comm			= 0;
	ch->concede			= 0;
	ch->current_tip		= 0;
	ch->dollars			= 0;
	ch->email_lock		= 0;
	ch->employer		= 0;
	ch->exp				= 0;
	ch->falldam			= 0;
	ch->form			= 0;
	ch->gen				= 0;
	ch->health			= 7;
	ch->herd_timer		= 0;
	ch->home			= 0;
	ch->hunter_vis		= 0;
	ch->id				= 0;
	ch->imm_flags		= 0;
	ch->jump_timer		= 0;
	ch->lines			= PAGELEN;
	ch->logon			= current_time;
	ch->markup			= 0;
	ch->max_traits[0]	= 5;
	ch->max_traits[1]	= 5;
	ch->off_flags		= 0;
	ch->on_phone		= 0;
	ch->ooc_xp_count	= 0;
	ch->ooc_xp_time		= 0;
	ch->oocxp			= 0;
	ch->parts			= 0;
	ch->played			= 0;
	ch->plr_flags		= 0;
	ch->position		= P_STAND;
	ch->pospts			= 0;
	ch->res_flags		= 0;
	ch->ritepoint		= 0;
	ch->stock_ticker	= 0;
	ch->version			= 0;
	ch->vuln_flags		= 0;
	ch->wait			= 0;
	ch->warrants		= 0;
	ch->wiznet			= 0;
	ch->xp_job_acts		= 0;
	ch->xpgift			= 0;
	ch->GHB				= 0;
	ch->RBPG			= 0;
	ch->auspice			= 0;
	ch->bg_timer		= 0;
	ch->blood_timer		= 0;
	ch->breed			= 0;
	ch->carry_number	= 0;
	ch->carry_weight	= 0;
	ch->combat_flag		= 0;
	ch->dam_type		= 0;
	ch->daze			= 0;
	ch->default_pos		= P_STAND;
	ch->dice_mod		= 0;
	ch->diff_mod		= 0;
	ch->group			= 0;
	ch->incog_level		= 0;
	ch->infl_timer		= 0;
	ch->invis_level		= 0;
	ch->jump_dir		= 0;
	ch->max_GHB			= 0;
	ch->max_RBPG		= 0;
	ch->max_willpower	= 0;
	ch->mprog_delay		= 0;
	ch->power_timer		= 0;
	ch->race			= 0;
	ch->saving_throw	= 0;
	ch->sex				= 0;
	ch->shape			= 0;
	ch->size			= 0;
	ch->start_pos		= P_STAND;
	ch->timer			= 0;
	ch->torpor_timer	= 0;
	ch->train_success	= 0;
	ch->willpower		= 0;
	ch->trust			= 0;

	for (i = 0; i < 3; i++)
		ch->armor[i] = 100;
	for (i = 0; i < 3; i++)
		ch->clan_powers[i] = -1;
	for (i = 0; i < 3; i++)
		ch->colours[i] = NULL;
	for (i = 0; i < MAX_STATS; i ++)
	{
		ch->perm_stat[i] = 1;
		ch->mod_stat[i] = 0;
	}
	clear_rite(ch);

	return ch;
}


void free_char (CHAR_DATA *ch)
{
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;

	Escape(ch);

	if (IS_NPC(ch))
		mobile_count--;

	for (obj = ch->carrying; obj != NULL; obj = obj_next)
	{
		obj_next = obj->next_content;
		extract_obj(obj);
	}

	for (paf = ch->affected; paf != NULL; paf = paf_next)
	{
		paf_next = paf->next;
		affect_remove(ch,paf);
	}

	if(ch->script != NULL)
		free_script(ch->script);

	PURGE_DATA(ch->aifile);
	PURGE_DATA(ch->alt_description);
	PURGE_DATA(ch->alt_long_descr);
	PURGE_DATA(ch->alt_name);
	PURGE_DATA(ch->colours[0]);
	PURGE_DATA(ch->colours[1]);
	PURGE_DATA(ch->demeanor);
	PURGE_DATA(ch->description);
	PURGE_DATA(ch->email_addr);
	PURGE_DATA(ch->ignore);
	PURGE_DATA(ch->laston);
	PURGE_DATA(ch->long_descr);
	PURGE_DATA(ch->material);
	PURGE_DATA(ch->name);
	PURGE_DATA(ch->nature);
	PURGE_DATA(ch->oldname);
	PURGE_DATA(ch->pack);
	PURGE_DATA(ch->prefix);
	PURGE_DATA(ch->profession);
	PURGE_DATA(ch->prompt);
	PURGE_DATA(ch->short_descr);
	PURGE_DATA(ch->surname);
	PURGE_DATA(ch->switch_desc);
	PURGE_DATA(ch->to_learn);
	PURGE_DATA(ch->pnote);
	PURGE_DATA(ch->pcdata);
	ch->ghouled_by = NULL;
	ch->married = NULL;
    ch->next_in_room = NULL;
	ch->reply = NULL;
	ch->sire = NULL;

	PURGE_DATA(ch);
	return;
}


PC_DATA *new_pcdata(void)
{
	int alias = 0;

	PC_DATA *pcdata;

	ALLOC_DATA(pcdata, PC_DATA, 1);

	pcdata->confirm_delete		= FALSE;
	pcdata->full_reset			= FALSE;

	pcdata->acct_login		= NULL;
	pcdata->bamfin			= NULL;
	pcdata->bamfout			= NULL;
	pcdata->block_join		= NULL;
	pcdata->ignore_reject	= NULL;
	pcdata->pwd				= NULL;
	pcdata->rpok_string		= NULL;
	pcdata->title			= NULL;

	pcdata->points			= 0;
	pcdata->security		= 0;
	pcdata->true_sex		= 0;

	for (alias = 0; alias < MAX_ALIAS; alias++)
	{
		pcdata->alias[alias] = NULL;
		pcdata->alias_sub[alias] = NULL;
	}

	pcdata->buffer = new_buf();

	return pcdata;
}
	

void free_pcdata(PC_DATA *pcdata)
{
    int alias = 0;

    Escape(pcdata);

    PURGE_DATA(pcdata->acct_login);
    PURGE_DATA(pcdata->bamfin);
    PURGE_DATA(pcdata->bamfout);
    PURGE_DATA(pcdata->block_join);
    PURGE_DATA(pcdata->ignore_reject);
    PURGE_DATA(pcdata->pwd);
    PURGE_DATA(pcdata->rpok_string);
    PURGE_DATA(pcdata->title);
    free_buf(pcdata->buffer);
    
    for (alias = 0; alias < MAX_ALIAS; alias++)
    {
    	PURGE_DATA(pcdata->alias[alias]);
    	PURGE_DATA(pcdata->alias_sub[alias]);
    }


    PURGE_DATA(pcdata);
    return;
}


MEMORY *new_memory(void)
{
	MEMORY *memory;

	ALLOC_DATA(memory, MEMORY, 1);

	memory->attitude	= 0;
	memory->name		= NULL;
	memory->reaction	= 0;

	memory->next = NULL;
	return memory;
}

void free_new_memory(MEMORY *memory)
{
	Escape(memory);

	PURGE_DATA(memory->name);
	PURGE_DATA(memory);
	return;

}

BUFFER *__new_buf(const char *file, const char *function, int line)
{
	BUFFER *buffer;

	ALLOC_DATA(buffer, BUFFER, 1);

	buffer->next	= NULL;
	buffer->state	= BUFFER_SAFE;
	buffer->size	= get_size(BASE_BUF);

	/*For debugging purposes*/
	buffer->file = str_dup(file);
	buffer->function= str_dup(function);
	buffer->line = line;

	ALLOC_DATA(buffer->string, char, buffer->size);

	buffer->string[0]	= '\0';
	top_buffer++;
	LINK_SINGLE(buffer, next, buffer_list);

	VALIDATE(buffer);

	return buffer;
}

BUFFER *new_buf_size(int size)
{
    BUFFER *buffer;

    ALLOC_DATA(buffer, BUFFER, 1);

    buffer->next        = NULL;
    buffer->state       = BUFFER_SAFE;
    buffer->size        = get_size(size);
    buffer->file		= NULL;
    buffer->function    = NULL;
    if (buffer->size == -1)
    {
        log_string(LOG_BUG, Format("new_buf: buffer size %d too large.",size));
        exit(1);
    }
    ALLOC_DATA(buffer->string, char, buffer->size);
    buffer->string[0]   = '\0';
    VALIDATE(buffer);

    return buffer;
}


void free_buf(BUFFER *buffer)
{

	Escape(buffer);

	free(buffer->file);
	buffer->file = NULL;
	free(buffer->function);
	buffer->function = NULL;

	top_buffer--;
	UNLINK_SINGLE(buffer, next, BUFFER, buffer_list);

	PURGE_DATA(buffer->file);
	PURGE_DATA(buffer->function);
	PURGE_DATA(buffer->string);
	buffer->string = NULL;
	buffer->size   = 0;
	buffer->state  = BUFFER_FREED;
	INVALIDATE(buffer);
	PURGE_DATA(buffer);
}


bool add_buf(BUFFER *buffer, char *string)
{
	int len = 0;
	char *oldstr;
	int oldsize = buffer->size;

	oldstr = buffer->string;

	if (buffer->state == BUFFER_OVERFLOW) /* don't waste time on bad strings! */
		return FALSE;

	len = strlen(buffer->string) + strlen(string) + 1;

	while (len >= buffer->size) /* increase the buffer size */
	{
		buffer->size 	= get_size(buffer->size + 1);
		{
			if (buffer->size == -1) /* overflow */
			{
				buffer->size = oldsize;
				buffer->state = BUFFER_OVERFLOW;
				log_string(LOG_BUG, Format("buffer overflow past size %d",buffer->size));
				return FALSE;
			}
		}
	}

	if (buffer->size != oldsize)
	{
		ALLOC_DATA(buffer->string, char, buffer->size);

		strncpy(buffer->string,oldstr, buffer->size);
		PURGE_DATA(oldstr);
	}

	strncat(buffer->string,string, buffer->size);
	return TRUE;
}

void BufPrintf ( BUFFER * buffer, char * fmt, ... )
{
    char buf[MSL]={'\0'};
	va_list args;

	va_start ( args, fmt );
	vsnprintf ( buf, sizeof(buf), fmt, args );
	va_end ( args );

	add_buf ( buffer, buf );
	return;
}

void clear_buf(BUFFER *buffer)
{
    buffer->string[0] = '\0';
    buffer->state     = BUFFER_SAFE;
}


char *buf_string(BUFFER *buffer)
{
    return buffer->string;
}


MPROG_LIST *new_mprog(void)
{
   MPROG_LIST *mp;

   ALLOC_DATA(mp, MPROG_LIST, 1);

   mp->vnum             = 0;
   mp->trig_type        = 0;
   mp->code             = NULL;
   mp->trig_phrase		= NULL;
   return mp;
}

void free_mprog(MPROG_LIST *mp)
{
	Escape(mp);

   PURGE_DATA(mp->code);
   PURGE_DATA(mp->trig_phrase);
   PURGE_DATA(mp);
}


RESET_DATA *new_reset_data( void )
{
    RESET_DATA *pReset;

    ALLOC_DATA(pReset, RESET_DATA, 1);

    pReset->next        =   NULL;
    pReset->command     =   'X';
    pReset->arg1        =   0;
    pReset->arg2        =   0;
    pReset->arg3        =   0;

    return pReset;
}


void free_reset_data( RESET_DATA *pReset )
{
	Escape(pReset);
	PURGE_DATA(pReset);
   	return;
}


AREA_DATA *new_area( void )
{
    AREA_DATA *pArea;

    ALLOC_DATA(pArea, AREA_DATA, 1);

    pArea->name = str_dup( "New Area");
    pArea->builders = str_dup("None");
    pArea->credits = str_dup("None");
    pArea->security		=   1;
    pArea->pricemod		=   1;
    pArea->min_vnum		=   0;
    pArea->max_vnum		=   0;
    pArea->age			=   0;
    pArea->nplayer		=   0;
    pArea->empty		=   TRUE;              /* ROM patch */
    pArea->file_name	=   str_dup( (char *)Format("area%d.are", pArea->vnum) );
    pArea->next			=   NULL;
    pArea->area_flags	=   AREA_ADDED;
    pArea->vnum			=   top_area-1;
    pArea->low_range	= 0;
    pArea->high_range	= 0;

    return pArea;
}


void free_area( AREA_DATA *pArea )
{
	Escape(pArea);

	PURGE_DATA( pArea->builders );
	PURGE_DATA( pArea->credits);
	PURGE_DATA( pArea->file_name );
	PURGE_DATA( pArea->name );
	PURGE_DATA(pArea);
    return;
}


EXIT_DATA *new_exit( void )
{
    EXIT_DATA *pExit;

    ALLOC_DATA(pExit, EXIT_DATA, 1);

    pExit->u1.to_room   =   NULL;                  /* ROM OLC */
    pExit->jumpto.to_room   =   NULL;              /* PT OLC */
    pExit->next         =   NULL;
    pExit->exit_info    =   0;
    pExit->key          =   0;
    pExit->keyword      =   NULL;
    pExit->description  =   NULL;
    pExit->rs_flags     =   0;

    return pExit;
}


void free_exit( EXIT_DATA *pExit )
{
	Escape(pExit);

	PURGE_DATA( pExit->keyword );
	PURGE_DATA( pExit->description );
	PURGE_DATA( pExit );
    return;
}


ROOM_INDEX_DATA *new_room_index( void )
{
    ROOM_INDEX_DATA *pRoom;
    int door = 0;

    ALLOC_DATA(pRoom, ROOM_INDEX_DATA, 1);

    pRoom->next             =   NULL;
    pRoom->people           =   NULL;
    pRoom->contents         =   NULL;
    pRoom->extra_descr      =   NULL;
    pRoom->area             =   NULL;

    for ( door=0; door < MAX_DIR; door++ )
        pRoom->exit[door]   =   NULL;

    pRoom->name             =   str_dup( "Unnamed Room");
    pRoom->description      =   str_dup( "None");
    pRoom->uname            =   str_dup( "Unnamed Room");
    pRoom->udescription	    =   str_dup( "None");
    pRoom->dname            =   str_dup( "Unnamed Room");
    pRoom->ddescription	    =   str_dup( "None");
    pRoom->owner	    =   NULL;
    pRoom->vnum             =   0;
    pRoom->room_flags       =   0;
    pRoom->light            =   0;
    pRoom->sector_type      =   0;
    pRoom->clan		    =	0;
    pRoom->heal_rate	    =   100;
    pRoom->mana_rate	    =   100;
    pRoom->at_stop	    =   -1;
    pRoom->going_to	    =   -1;

    for( door = 0; door < MAX_CAR_STOPS; door++)
	pRoom->stops[door] = -1;
    pRoom->car = -1;

    return pRoom;
}


void free_room_index( ROOM_INDEX_DATA *pRoom )
{
	int door = 0;
	ROOM_INDEX_DATA *orig_room;

	EXTRA_DESCR_DATA *pExtra, *pExNext;
	RESET_DATA *pReset, *pResNext;
	CHAR_DATA *pCh, *pChNext = NULL;

	Escape(pRoom);

	orig_room = pRoom;

	PURGE_DATA( pRoom->ddescription );
	PURGE_DATA( pRoom->description );
	PURGE_DATA( pRoom->dname );
	PURGE_DATA( pRoom->name );
	PURGE_DATA( pRoom->owner );
	PURGE_DATA( pRoom->udescription );
	PURGE_DATA( pRoom->uname );

	for ( door = 0; door < MAX_DIR; door++ )
	{
		if ( pRoom->exit[door] )
		{
			free_exit(pRoom->exit[door]);
/*
			ROOM_INDEX_DATA *pToRoom;
			sh_int rev;
			 rev = rev_dir[door];
			 pToRoom = pRoom->exit[door]->u1.to_room;

			 if (!pToRoom) continue;

			 if ( pToRoom->exit[rev] && pToRoom->exit[rev]->u1.to_room == pRoom)
			 {
				 free_exit( pToRoom->exit[rev] );
				 pToRoom->exit[rev] = NULL;
			 }

			  * Remove this exit.
			 free_exit( pRoom->exit[door] );
			 pRoom->exit[door] = NULL;
			*/
		}
	}

	for ( pExtra = pRoom->extra_descr; pExtra; pExtra = pExNext )
	{
		pExNext = pExtra->next;
		free_extra_descr( pExtra );
	}

	for ( pReset = pRoom->reset_first; pReset; pReset = pResNext )
	{
		pResNext = pReset->next;
		free_reset_data( pReset );
	}

	for ( pCh = pRoom->people; pCh; pCh = pChNext )
	{
		char_from_room(pCh);
		char_to_room(pCh, get_room_index(ROOM_VNUM_START));
		send_to_char("The room disintegrates... You feel your position shift.\n\r", pCh);
	}

	for ( pCh = pRoom->listeners; pCh; pCh = pChNext )
	{
		pChNext = pCh->next_listener;
		char_from_listeners(pCh);
		send_to_char("You are thrust back into your body.\n\r", pCh);
	}

/*	corrupts the list -- this needs to be moved to the correct purging spot for olc_delete_room
	to protect against this corruption.
	iHash = pRoom->vnum % MAX_KEY_HASH;
	if(pRoom == room_index_hash[iHash])
	{
		room_index_hash[iHash] = pRoom->next;
	}
	else
	{
		for(Next = room_index_hash[iHash]; Next; Next = Next->next)
		{
			if(Next->next == pRoom)
			{
				Next->next = pRoom->next;
				break;
			}
		}
	}
*/

	assert(orig_room == pRoom);	// assert fail if we are not the same! (did we corrupt)

	PURGE_DATA(orig_room);
	return;
}


SHOP_DATA *new_shop( void )
{
    SHOP_DATA *pShop;
    int buy = 0;

    ALLOC_DATA(pShop, SHOP_DATA, 1);

    pShop->next         =   NULL;

    for ( buy=0; buy<MAX_TRADE; buy++ )
        pShop->buy_type[buy]    =   0;

	pShop->keeper       =   0;
	pShop->profit_buy   =   100;
	pShop->profit_sell  =   100;
	pShop->open_hour    =   0;
	pShop->close_hour   =   23;
	pShop->raw_materials	= 0;

    return pShop;
}


void free_shop( SHOP_DATA *pShop )
{
	Escape(pShop);
	UNLINK_SINGLE(pShop, next, SHOP_DATA, shop_list);

	PURGE_DATA(pShop);
    return;
}


OBJ_INDEX_DATA *new_obj_index( void )
{
    OBJ_INDEX_DATA *pObj;
    int value = 0;

    ALLOC_DATA(pObj, OBJ_INDEX_DATA, 1);

    pObj->material = str_dup("unknown");
    pObj->name = str_dup("no name");
    pObj->short_descr = str_dup("(no short description)");
    pObj->description = str_dup("(no description)");
    pObj->full_desc = str_dup( "(no full description)");
	pObj->next          =   NULL;
    pObj->extra_descr   =   NULL;
    pObj->affected      =   NULL;
    pObj->area          =   NULL;
    pObj->company		=   NULL;
    pObj->vnum          =   0;
    pObj->item_type     =   ITEM_TRASH;
    pObj->extra_flags   =   0;
    pObj->wear_flags    =   0;
    pObj->count         =   0;
    pObj->weight        =   0;
    pObj->cost          =   0;
    pObj->condition     =   100;                        /* ROM */
    for ( value = 0; value < 5; value++ )               /* 5 - ROM */
        pObj->value[value]  =   0;
    pObj->quality	=   2;

    pObj->fetish_level = 0;
    pObj->fetish_flags = 0;
    pObj->fetish_target = 0;

    pObj->to_user_none          = NULL;
    pObj->to_user_self          = NULL;
    pObj->to_user_other         = NULL;
    pObj->to_room_none          = NULL;
    pObj->to_room_self          = NULL;
    pObj->to_room_other         = NULL;
    pObj->to_vict_other         = NULL;
    pObj->to_room_used          = NULL;
    pObj->to_user_used			= NULL;
    pObj->uses					=   -2;

    return pObj;
}


void free_obj_index( OBJ_INDEX_DATA *pObj )
{
    EXTRA_DESCR_DATA *pExtra, *pExtra_next;
    AFFECT_DATA *pAf, *pAf_next;

    Escape(pObj);

    PURGE_DATA( pObj->company );
    PURGE_DATA( pObj->description );
    PURGE_DATA( pObj->full_desc );
    PURGE_DATA( pObj->material );
    PURGE_DATA( pObj->name );
    PURGE_DATA( pObj->short_descr );
    PURGE_DATA( pObj->to_room_none );
    PURGE_DATA( pObj->to_room_other );
    PURGE_DATA( pObj->to_room_self );
    PURGE_DATA( pObj->to_room_used );
    PURGE_DATA( pObj->to_user_none );
    PURGE_DATA( pObj->to_user_other );
    PURGE_DATA( pObj->to_user_self );
    PURGE_DATA( pObj->to_user_used );
    PURGE_DATA( pObj->to_vict_other);
    for ( pAf = pObj->affected; pAf; pAf = pAf_next )
    {
	pAf_next = pAf->next;
        free_affect( pAf );
    }

    for ( pExtra = pObj->extra_descr; pExtra; pExtra = pExtra_next )
    {
	pExtra_next = pExtra->next;
        free_extra_descr( pExtra );
    }


    PURGE_DATA( pObj );
    return;
}


MOB_INDEX_DATA *new_mob_index( void )
{
    MOB_INDEX_DATA *pMob;
    int i = 0;

    ALLOC_DATA(pMob, MOB_INDEX_DATA, 1);

    pMob->next          =   NULL;
    pMob->spec_fun      =   NULL;
    pMob->pShop         =   NULL;
    pMob->area          =   NULL;
    pMob->player_name = str_dup("No Name");
    pMob->short_descr = str_dup("(no short description)");
    pMob->long_descr = str_dup("(no long description)");
    pMob->material = str_dup("unknown");
    pMob->description = str_dup("(no description)");

    pMob->GHB				= 0;
    pMob->RBPG				= 0;
    pMob->act				= ACT_IS_NPC;
    pMob->act2				= 0;
    pMob->affected_by		= 0;
    pMob->affected_by2		= 0;
    pMob->count				= 0;
    pMob->dam_type			= 0;
    pMob->default_pos		= P_STAND; /*  -- Hugin */
    pMob->form				= 0;           /* ROM patch -- Hugin */
    pMob->group				= 0;
    pMob->health			= 7;
    pMob->hit[DICE_BONUS]	= 0;   /* ROM patch -- Hugin */
    pMob->hit[DICE_NUMBER]	= 0;   /* ROM patch -- Hugin */
    pMob->hit[DICE_TYPE]	= 0;   /* ROM patch -- Hugin */
    pMob->imm_flags     = 0;           /* ROM patch -- Hugin */
    pMob->killed        = 0;
    pMob->level         = 0;
    pMob->mprog_flags	= 0;
    pMob->off_flags     = 0;           /* ROM patch -- Hugin */
    pMob->parts         = 0;           /* ROM patch -- Hugin */
    pMob->race          = race_lookup( "human" ); /* - Hugin */
    pMob->res_flags     = 0;           /* ROM patch -- Hugin */
    pMob->sex           = 0;
    pMob->size          = SIZE_MEDIUM; /* ROM patch -- Hugin */
    pMob->start_pos		= P_STAND; /*  -- Hugin */
    pMob->vnum          = 0;
    pMob->vuln_flags    = 0;           /* ROM patch -- Hugin */
    pMob->wealth		= 0;
    pMob->willpower		= 0;

    for (i = 0; i < MAX_STATS; i ++)
        pMob->stat[i] = 0;

    for (i = 0; i < MAX_ABIL; i ++)
        pMob->ability[i].value = 0;

    for (i = 0; i < MAX_DISC; i ++)
        pMob->disc[i] = 0;

    for (i = 0; i < MAX_VIRTUES; i ++)
        pMob->virtues[i] = 0;

    for (i = 0; i < MAX_INFL; i ++)
        pMob->influences[i] = 0;

    for (i = 0; i < MAX_BG; i ++)
        pMob->backgrounds[i] = 0;


    return pMob;
}


void free_mob_index( MOB_INDEX_DATA *pMob )
{
	Escape(pMob);
	SCRIPT_DATA *pEvent, *pEvent_next;

	PURGE_DATA( pMob->description );
	PURGE_DATA( pMob->long_descr );
	PURGE_DATA( pMob->material);
	PURGE_DATA( pMob->player_name );
	PURGE_DATA( pMob->short_descr );

	MPROG_LIST *mp, *mpn;
	for(mp = pMob->mprogs; mp; mp = mpn)
	{
		mpn = mp->next;
		free_mprog(mp);
	}

	for(pEvent = pMob->triggers; pEvent; pEvent = pEvent_next) {
		pEvent_next = pEvent->next_in_event;

		free_script(pEvent);
	}

    free_shop( pMob->pShop );

    PURGE_DATA(pMob);
    return;
}

/* BREAK POINT */

MPROG_CODE *new_mpcode(void)
{
     MPROG_CODE *NewCode;

     ALLOC_DATA(NewCode, MPROG_CODE, 1);

     NewCode->vnum    = 0;
     NewCode->code    = NULL;
     NewCode->next    = NULL;

     return NewCode;
}

void free_mpcode(MPROG_CODE *pMcode)
{
	Escape(pMcode);

	PURGE_DATA(pMcode->code);
	PURGE_DATA(pMcode);
	return;
}


void save_bans(void)
{
	BAN_DATA *pban;
	FILE *fp;
	bool found = FALSE;

	closeReserve();
	if ( ( fp = fopen( BAN_FILE, "w" ) ) == NULL )
	{
		perror( BAN_FILE );
	}

	for (pban = ban_list; pban != NULL; pban = pban->next)
	{
		if (IS_SET(pban->ban_flags,BAN_PERMANENT))
		{
			found = TRUE;
			fprintf(fp,"%-20s %-2d %s\n",pban->name,pban->level,
					print_flags(pban->ban_flags));
		}
	}

	fclose(fp);
	openReserve();
	if (!found)
		unlink(BAN_FILE);
}

void load_bans(void)
{
	FILE *fp;
	BAN_DATA *ban_last;

	if ( ( fp = fopen( BAN_FILE, "r" ) ) == NULL )
		return;

	ban_last = NULL;
	for ( ; ; )
	{
		BAN_DATA *pban;
		if ( feof(fp) )
		{
			fclose( fp );
			return;
		}

		pban = new_ban();

		PURGE_DATA( pban->name );
		pban->name = str_dup(fread_word(fp));
		pban->level = fread_number(fp);
		pban->ban_flags = fread_flag(fp);
		fread_to_eol(fp);


		LINK_SINGLE(pban, next, ban_list);
		ban_last = pban;
	}
}

bool check_ban(char *site,int type)
{
	BAN_DATA *pban;
	char host[MSL]={'\0'};

	strncpy(host,capitalize(site), sizeof(host));
	host[0] = LOWER(host[0]);

	for ( pban = ban_list; pban != NULL; pban = pban->next )
	{
		if(!IS_SET(pban->ban_flags,type))
			continue;

		if (IS_SET(pban->ban_flags,BAN_PREFIX)
				&&  IS_SET(pban->ban_flags,BAN_SUFFIX)
				&&  strstr(pban->name,host) != NULL)
			return TRUE;

		if (IS_SET(pban->ban_flags,BAN_PREFIX)
				&&  !str_suffix(pban->name,host))
			return TRUE;

		if (IS_SET(pban->ban_flags,BAN_SUFFIX)
				&&  !str_prefix(pban->name,host))
			return TRUE;
	}

	return FALSE;
}


void ban_site(CHAR_DATA *ch, char *argument, bool fPerm)
{
	char buf2[MSL]={'\0'};
	char arg1[MAX_INPUT_LENGTH]={'\0'};
	char arg2[MAX_INPUT_LENGTH]={'\0'};
	char *name;
	BUFFER *buffer;
	BAN_DATA *pban, *prev;
	bool prefix = FALSE,suffix = FALSE;
	int type = 0;

	argument = one_argument(argument,arg1);
	argument = one_argument(argument,arg2);

	if ( IS_NULLSTR(arg1) )
	{
		if (ban_list == NULL)
		{
			send_to_char("No sites banned at this time.\n\r",ch);
			return;
		}
		buffer = new_buf();

		add_buf(buffer,"Banned sites  level  type     status\n\r");
		for (pban = ban_list;pban != NULL;pban = pban->next)
		{
			snprintf(buf2, sizeof(buf2), "%s%s%s",
					IS_SET(pban->ban_flags,BAN_PREFIX) ? "*" : "", pban->name, IS_SET(pban->ban_flags,BAN_SUFFIX) ? "*" : "");
			add_buf(buffer, (char *)Format("%-12s    %-3d  %-7s  %s\n\r",
					buf2, pban->level, IS_SET(pban->ban_flags,BAN_NEWBIES) ? "newbies" :
							IS_SET(pban->ban_flags,BAN_PERMIT)  ? "permit"  :
									IS_SET(pban->ban_flags,BAN_ALL)     ? "all"	: "",
											IS_SET(pban->ban_flags,BAN_PERMANENT) ? "perm" : "temp"));
		}

		page_to_char( buf_string(buffer), ch );
		free_buf(buffer);
		return;
	}

	/* find out what type of ban */
	if (IS_NULLSTR(arg2) || !str_prefix(arg2,"all"))
		type = BAN_ALL;
	else if (!str_prefix(arg2,"newbies"))
		type = BAN_NEWBIES;
	else if (!str_prefix(arg2,"permit"))
		type = BAN_PERMIT;
	else
	{
		send_to_char("Acceptable ban types are all, newbies, and permit.\n\r", ch);
		return;
	}

	name = arg1;

	if (name[0] == '*')
	{
		prefix = TRUE;
		name++;
	}

	if (name[strlen(name) - 1] == '*')
	{
		suffix = TRUE;
		name[strlen(name) - 1] = '\0';
	}

	if (strlen(name) == 0)
	{
		send_to_char("You have to ban SOMETHING.\n\r",ch);
		return;
	}

	prev = NULL;
	for ( pban = ban_list; pban != NULL; prev = pban, pban = pban->next )
	{
		if (!str_cmp(name,pban->name))
		{
			if (pban->level > get_trust(ch))
			{
				send_to_char( "That ban was set by a higher power.\n\r", ch );
				return;
			}
			else
			{
				if (prev == NULL)
					ban_list = pban->next;
				else
					prev->next = pban->next;
				free_ban(pban);
			}
		}
	}

	pban = new_ban();
	PURGE_DATA( pban->name );
	pban->name = str_dup(name);
	pban->level = get_trust(ch);

	/* set ban type */
	pban->ban_flags = type;

	if (prefix)
		SET_BIT(pban->ban_flags,BAN_PREFIX);
	if (suffix)
		SET_BIT(pban->ban_flags,BAN_SUFFIX);
	if (fPerm)
		SET_BIT(pban->ban_flags,BAN_PERMANENT);

	LINK_SINGLE(pban, next, ban_list);

	save_bans();
	send_to_char( Format("%s has been banned.\n\r",pban->name), ch );
	return;
}

void do_ban(CHAR_DATA *ch, char *argument)
{
    ban_site(ch,argument,FALSE);
}

void do_permban(CHAR_DATA *ch, char *argument)
{
    ban_site(ch,argument,TRUE);
}

void do_allow( CHAR_DATA *ch, char *argument )                        
{
	char arg[MAX_INPUT_LENGTH]={'\0'};
	BAN_DATA *prev;
	BAN_DATA *curr;

	one_argument( argument, arg );

	if ( IS_NULLSTR(arg) )
	{
		send_to_char( "Remove which site from the ban list?\n\r", ch );
		return;
	}

	prev = NULL;
	for ( curr = ban_list; curr != NULL; prev = curr, curr = curr->next )
	{
		if ( !str_cmp( arg, curr->name ) )
		{
			if (curr->level > get_trust(ch))
			{
				send_to_char( "You are not powerful enough to lift that ban.\n\r",ch);
				return;
			}
			if ( prev == NULL )
				ban_list   = ban_list->next;
			else
				prev->next = curr->next;

			free_ban(curr);
			send_to_char( Format("Ban on %s lifted.\n\r",arg), ch );
			save_bans();
			return;
		}
	}

	send_to_char( "Site is not banned.\n\r", ch );
	return;
}


STOCKS *new_stock()
{
	STOCKS *stock;

	ALLOC_DATA(stock, STOCKS, 1);

	stock->name = NULL;
	stock->ticker = NULL;
	stock->last_change = current_time;
	stock->cost = 0;
	stock->phase = 0;
	stock->upordown = 0;

	return stock;
}

void free_stock(STOCKS *stock)
{
	Escape(stock);

    PURGE_DATA( stock->name   );
    PURGE_DATA( stock->ticker );
    PURGE_DATA(stock);
}


PACK_DATA *new_pack()
{
	PACK_DATA *pack;

	ALLOC_DATA(pack, PACK_DATA, 1);

	pack->name	= NULL;
	pack->totem	= 0;

	return pack;
}

void free_pack(PACK_DATA *pack)
{
	Escape(pack);

    PURGE_DATA( pack->name );
    pack->totem = -1;
    PURGE_DATA(pack);
}


ORG_DATA *new_org()
{
	ORG_DATA *org;
	int i = 0;

	ALLOC_DATA(org, ORG_DATA, 1);

	org->applicants = str_dup("None");
	org->races = str_dup("None");
	org->name		= NULL;
	org->who_name	= NULL;
	org->file_name	= NULL;
	org->leader		= NULL;
	for(i=0; i<5; i++)
		org->commands[i] = NULL;
	for(i=0; i<6; i++)
		org->title[i] = NULL;

	org->step_point		= 0;
	org->funds			= 0;
	org->default_auths	= 0;
	org->type			= 0;

	return org;
}

void free_org(ORG_DATA *org)
{
	ORGMEM_DATA *nextmem;
	ORGMEM_DATA *point;
	int i = 0;

	Escape(org);

	org->applicants = NULL;
	PURGE_DATA( org->file_name);
	org->leader = NULL;
	PURGE_DATA( org->name     );
	PURGE_DATA( org->races );
	PURGE_DATA( org->who_name );
	for(i=0; i<5; i++)
		PURGE_DATA(org->commands[i]);
	for(i=0; i<6; i++)
		PURGE_DATA(org->title[i]);

	org->step_point		= 0;
	org->type			= 0;
	org->funds			= 0;
	org->default_auths	= 0;
	if(org->members)
	{
		nextmem = org->members;
		for(point = org->members; nextmem; point = nextmem)
		{
			nextmem = point->next;
			free_orgmem(point);
		}
		org->members = NULL;
	}
	PURGE_DATA(org);
}


ORGMEM_DATA *new_orgmem()
{
	ORGMEM_DATA *om;

	ALLOC_DATA(om, ORGMEM_DATA, 1);

	om->name		= NULL;
	om->status		= 0;
	om->auth_flags	= 0;
	om->status_votes	= 0;
	return om;
}

void free_orgmem(ORGMEM_DATA *om)
{
	Escape(om);

	PURGE_DATA( om->name );
	PURGE_DATA(om);
}


TRAIT_DATA *new_trait()
{
	TRAIT_DATA *trait;

	ALLOC_DATA(trait, TRAIT_DATA, 1);

	trait->qualifier = NULL;
	trait->detail = NULL;
	trait->type = 0;
	trait->value = 0;
	trait->next = NULL;

	return trait;
}

void free_trait(TRAIT_DATA *trait)
{
	Escape(trait);

    PURGE_DATA( trait->detail );
    PURGE_DATA( trait->qualifier );
    PURGE_DATA( trait );
}


LIQUID_DATA *new_liqdata()
{
	LIQUID_DATA *liquid;

	ALLOC_DATA(liquid, LIQUID_DATA, 1);

	liquid->liquid = -1;
	liquid->quantity = 0;

	return liquid;
}


void free_liquid(LIQUID_DATA *liquid)
{
	Escape(liquid);
	PURGE_DATA(liquid);
}


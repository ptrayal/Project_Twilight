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
 
/***************************************************************************
* ROM 2.4 is copyright 1993-1998 Russ Taylor                               *
* ROM has been brought to you by the ROM consortium                        *
*     Russ Taylor (rtaylor@hypercube.org)                                  *
*     Gabrielle Taylor (gtaylor@efn.org)                                   *
*     Brian Moore (zump@rom.org)                                           *
* By using this code, you have agreed to follow the terms of the           *
* ROM license, in the file Rom24/doc/rom.license                           *
***************************************************************************/

#if defined(Macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "merc.h"
#include "recycle.h"


BUFFER *buffer_list;
int top_buffer;



NOTE_DATA *new_note()
{
	NOTE_DATA *note;

	ALLOC_DATA(note, NOTE_DATA, 1);

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

	gen->points_chosen	= 0;
	return gen;
}

void free_gen_data(GEN_DATA *gen)
{
	Escape(gen);
	PURGE_DATA(gen);
}

EXTRA_DESCR_DATA *new_extra_descr(void)
{
	EXTRA_DESCR_DATA *ed;

	ALLOC_DATA(ed, EXTRA_DESCR_DATA, 1);

	ed->keyword = NULL;
	ed->description = NULL;

	return ed;
}

void free_extra_descr(EXTRA_DESCR_DATA *ed)
{
	Escape(ed);

	PURGE_DATA(ed->keyword);
	PURGE_DATA(ed->description);
	PURGE_DATA(ed);
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

	obj->name 			= NULL;
	obj->description	= NULL;
	obj->short_descr	= NULL;

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
	
	PURGE_DATA( obj->name        );
	PURGE_DATA( obj->description );
	PURGE_DATA( obj->short_descr );
	obj->owner = NULL;
	
	PURGE_DATA(obj);
}


CHAR_DATA *new_char (void)
{
	CHAR_DATA *ch;
	int i = 0;

	ALLOC_DATA(ch, CHAR_DATA, 1);

	ch->name                    = NULL;
	ch->short_descr             = NULL;
	ch->long_descr              = NULL;
	ch->description             = NULL;
	ch->prompt                  = NULL;
	ch->prefix					= NULL;
	ch->logon                   = current_time;
	ch->lines                   = PAGELEN;
	for (i = 0; i < 4; i++)
		ch->armor[i]            = 100;
	ch->position                = POS_STANDING;
	ch->hit                     = 20;
	ch->max_hit                 = 20;
	ch->mana                    = 100;
	ch->max_mana                = 100;
	ch->move                    = 100;
	ch->max_move                = 100;
	for (i = 0; i < MAX_STATS; i ++)
	{
		ch->perm_stat[i] = 13;
		ch->mod_stat[i] = 0;
	}

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

	PURGE_DATA(ch->name);
	PURGE_DATA(ch->short_descr);
	PURGE_DATA(ch->long_descr);
	PURGE_DATA(ch->description);
	PURGE_DATA(ch->prompt);
	PURGE_DATA(ch->prefix);
	free_note  (ch->pnote);
	free_pcdata(ch->pcdata);

	PURGE_DATA(ch);
	return;
}


PC_DATA *new_pcdata(void)
{
	int alias = 0;

	PC_DATA *pcdata;

	ALLOC_DATA(pcdata, PC_DATA, 1);

	for (alias = 0; alias < MAX_ALIAS; alias++)
	{
	pcdata->alias[alias] = NULL;
	pcdata->alias_sub[alias] = NULL;
	}

	pcdata->bamfin			= NULL;
	pcdata->bamfout			= NULL;
	pcdata->pwd				= NULL;
	pcdata->title			= NULL;
	pcdata->security		= 0;
	pcdata->true_sex		= 0;

	pcdata->buffer = new_buf();
	
	return pcdata;
}
	

void free_pcdata(PC_DATA *pcdata)
{
	int alias = 0;

    Escape(pcdata);

	PURGE_DATA(pcdata->pwd);
	PURGE_DATA(pcdata->bamfin);
	PURGE_DATA(pcdata->bamfout);
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

	


/* stuff for setting ids */
long	last_pc_id;
long	last_mob_id;

long get_pc_id(void)
{
	int val = (current_time <= last_pc_id) ? last_pc_id + 1 : current_time;

	last_pc_id = val;
	return val;
}

long get_mob_id(void)
{
	last_mob_id++;
	return last_mob_id;
}

MEM_DATA *mem_data_free;

/* procedures and constants needed for buffering */

BUFFER *buf_free;

MEM_DATA *new_mem_data(void)
{
	MEM_DATA *memory;
  
	ALLOC_DATA(memory, MEM_DATA, 1);

	memory->next = NULL;
	memory->id = 0;
	memory->reaction = 0;
	memory->when = 0;
	
	return memory;
}

void free_mem_data(MEM_DATA *memory)
{
	Escape(memory);

	PURGE_DATA(memory);
}

/* buffer sizes */
const int buf_size[MAX_BUF_LIST] =
{
	16,32,64,128,256,1024,2048,4096,8192,16384
};

/* local procedure for finding the next acceptable size */
/* -1 indicates out-of-boundary error */
int get_size (int val)
{
	int i;

	for (i = 0; i < MAX_BUF_LIST; i++)
	if (buf_size[i] >= val)
	{
		return buf_size[i];
	}
	
	return -1;
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
        bug("new_buf: buffer size %d too large.",size);
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


bool add_buf(BUFFER *buffer, const char *string)
{
	int len = 0;
	char *oldstr;
	int oldsize = buffer->size;

	if(IS_NULLSTR(string))
		return false;

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
				bug("buffer overflow past size %d",buffer->size);
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

void clear_buffer(void) {
	BUFFER *buf, *buf_next;

	log_string("Cleaning: buffer_list");
	for(buf = buffer_list; buf; buf = buf_next) {
		buf_next = buf->next;
		free_buf(buf); // free_buf has UNLINK_SINGLE in it so we don't need to remove it from the list here
	}
	buffer_list = NULL;
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
   return mp;
}

void free_mprog(MPROG_LIST *mp)
{
   Escape(mp);

   PURGE_DATA(mp->code);
   PURGE_DATA(mp->trig_phrase);
   PURGE_DATA(mp);
}

HELP_AREA * new_had ( void )
{
	HELP_AREA * had;
	
	ALLOC_DATA(had, HELP_AREA, 1);

	return had;
}

HELP_DATA * new_help ( void )
{
	HELP_DATA *help;

	ALLOC_DATA(help, HELP_DATA, 1);

	help->level = 0;
	help->keyword = NULL;
	help->text = NULL;
	
	return help;
}

void free_help(HELP_DATA *help)
{
	Escape(help);

	PURGE_DATA(help->keyword);
	PURGE_DATA(help->text);
	
	PURGE_DATA( help );
}

// New stuff to recycle

WIZ_DATA *new_wiz(void)
{
	static WIZ_DATA wiz_zero;
	WIZ_DATA *wiz;

	ALLOC_DATA(wiz, WIZ_DATA, 1);

	*wiz = wiz_zero;
	wiz->name = NULL;
	return wiz;
}

void free_wiz(WIZ_DATA *wiz)
{
	Escape(wiz);

	PURGE_DATA(wiz->name);
	PURGE_DATA(wiz);	
}

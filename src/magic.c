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

#if defined(Macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdlib.h>
#include <time.h>
#include "twilight.h"
#include "magic.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
#include "interp.h"

bool check_social(CHAR_DATA *ch, char *command, char *argument);
void be_frenzied(CHAR_DATA *ch);
void stanza(char *argument);

/*
 * Local functions.
 */
void	say_spell			args( ( CHAR_DATA *ch, int sn ) );
int		do_transform 		args( ( CHAR_DATA *ch, char *argument ) );
int 	has_enough_power	args( ( CHAR_DATA *ch) );

/*
 * Imported functions
 */
bool    remove_obj      args( ( CHAR_DATA *ch, int iWear, bool fReplace ) );
ROOM_INDEX_DATA *     find_location   args( ( CHAR_DATA *ch, char *arg ) );
char *flag_string       args ( (const struct flag_type *flag_table,int bits) );


/*
 * Function: has_enough_power
 * Purpose: Check to see if a user has a power_timer on them and if so, they cannot use their powers.
 */
int has_enough_power(CHAR_DATA *ch)
{
	if (ch->power_timer > 0)
	{
		send_to_char("Your powers have not rejuvenated yet.\n\r", ch);
		return 0;
	}
        return 1;
}


/*
 * Lookup a skill by name.
 */
int skill_lookup( const char *name )
{
	int sn = 0;

	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
		if ( skill_table[sn].name == NULL )
			break;
		if ( LOWER(name[0]) == LOWER(skill_table[sn].name[0])
				&&   !str_prefix( name, skill_table[sn].name ) )
			return sn;
	}

	return -1;
}

int find_spell( CHAR_DATA *ch, const char *name )
{
	/* finds a spell the character can cast if possible */
	int sn = 0, found = -1;

	/*
    if (IS_NPC(ch))
	return skill_lookup(name);
	 */

	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
		if (skill_table[sn].name == NULL)
			break;
		if (LOWER(name[0]) == LOWER(skill_table[sn].name[0])
				&&  !str_prefix(name,skill_table[sn].name))
		{
			if ( found == -1)
				found = sn;
			if (ch->learned[sn] > 0 && skill_table[sn].spell_fun != spell_null)
				return sn;
		}
	}
	return found;
}


/*
 * Lookup a skill by slot number.
 * Used for object loading.
 */
int slot_lookup( int slot )
{
	extern bool fBootDb;
	int sn = 0;

	if ( slot <= 0 )
		return -1;

	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
		if ( slot == skill_table[sn].slot )
			return sn;
	}

	if ( fBootDb )
	{
		log_string(LOG_BUG, Format("Slot_lookup: bad slot %d.", slot ));
		abort( );
	}

	return -1;
}


/*
 * Utter mystical words for an sn.
 */
void say_spell( CHAR_DATA *ch, int sn )
{
    CHAR_DATA *rch;
    char buf  [MSL]={'\0'};
    char buf2 [MSL]={'\0'};
    char *pName;
    int iSyl;
    int length;

    struct syl_type
    {
	char *	old;
	char *	bnew;
    };

    static const struct syl_type syl_table[] =
    {
	{ " ",		" "		},
	{ "ar",		"abra"		},
	{ "au",		"kada"		},
	{ "bless",	"fido"		},
	{ "blind",	"nose"		},
	{ "bur",	"mosa"		},
	{ "cu",		"judi"		},
	{ "de",		"oculo"		},
	{ "en",		"unso"		},
	{ "light",	"dies"		},
	{ "lo",		"hi"		},
	{ "mor",	"zak"		},
	{ "move",	"sido"		},
	{ "ness",	"lacri"		},
	{ "ning",	"illa"		},
	{ "per",	"duda"		},
	{ "ra",		"gru"		},
	{ "fresh",	"ima"		},
	{ "re",		"candus"	},
	{ "son",	"sabru"		},
	{ "tect",	"infra"		},
	{ "tri",	"cula"		},
	{ "ven",	"nofo"		},
	{ "a", "a" }, { "b", "b" }, { "c", "q" }, { "d", "e" },
	{ "e", "z" }, { "f", "y" }, { "g", "o" }, { "h", "p" },
	{ "i", "u" }, { "j", "y" }, { "k", "t" }, { "l", "r" },
	{ "m", "w" }, { "n", "i" }, { "o", "a" }, { "p", "s" },
	{ "q", "d" }, { "r", "f" }, { "s", "g" }, { "t", "h" },
	{ "u", "j" }, { "v", "z" }, { "w", "x" }, { "x", "n" },
	{ "y", "l" }, { "z", "k" },
	{ "", "" }
    };

    for ( pName = skill_table[sn].name; *pName != '\0'; pName += length )
    {
    	for ( iSyl = 0; (length = strlen(syl_table[iSyl].old)) != 0; iSyl++ )
    	{
    		if ( !str_prefix( syl_table[iSyl].old, pName ) )
    		{
    			strncat( buf, syl_table[iSyl].bnew, sizeof(buf) );
    			break;
    		}
    	}

    	if ( length == 0 )
    		length = 1;
    }

    snprintf( buf2, sizeof(buf2), "$n utters the words, '%s'.", buf );
    snprintf( buf,  sizeof(buf), "$n utters the words, '%s'.", skill_table[sn].name );

    for ( rch = ch->in_room->people; rch; rch = rch->next_in_room )
    {
    	if ( rch != ch )
    		act((!IS_NPC(rch) && ch->clan==rch->clan) ? buf : buf2, ch, NULL, rch, TO_VICT, 0 );
    }

    return;
}



/*
 * Compute a saving throw.
 * Negative apply's make saving throw better.
 */
bool saves_spell( int level, CHAR_DATA *victim, int dam_type )
{
	int save = 0;
	int agg = 3;

	if(IS_NATURAL(victim)) 
		save = 6;
	else
		save = 4;

	switch(check_immune(victim,dam_type))
	{
	case IS_IMMUNE:		agg = 0;	break;
	case IS_RESISTANT:	save += 2;	break;
	case IS_VULNERABLE:	save -= 2;	break;
	}

	save = do_soak(victim, save, agg);

	return save > 0;
}

/* RT save for dispels */

bool saves_dispel( int dis_level, int spell_level, int duration)
{
    int save = 0;
    
    if (duration == -1)
      spell_level += 5;  
      /* very hard to dispel permanent effects */

    save = 50 + (spell_level - dis_level) * 5;
    save = URANGE( 5, save, 95 );
    return number_percent( ) < save;
}

/* co-routine for dispel magic and cancellation */

bool check_dispel( int dis_level, CHAR_DATA *victim, int sn)
{
    AFFECT_DATA *af;

    if (is_affected(victim, sn))
    {
        for ( af = victim->affected; af != NULL; af = af->next )
        {
            if ( af->type == sn )
            {
                if (!saves_dispel(dis_level,af->level,af->duration))
                {
                    affect_strip(victim,sn);
        	    if ( skill_table[sn].msg_off )
        	    {
            		send_to_char( skill_table[sn].msg_off, victim );
            		send_to_char( "\n\r", victim );
        	    }
		    return TRUE;
		}
		else
		    af->level--;
            }
        }
    }
    return FALSE;
}


/*
 * The kludgy global is for spells who want more stuff from command line.
 */
char *target_name;

void do_cast( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    char arg1[MAX_INPUT_LENGTH]={'\0'};
    char arg2[MAX_INPUT_LENGTH]={'\0'};
    void *vo;
    int sn = -1;
    int target = TARGET_NONE;

    /*
     * Switched NPC's can cast spells, but others can't.
     */
    if ( IS_NPC(ch) && ch->desc == NULL)
	return;

    target_name = one_argument( argument, arg1 );
    one_argument( target_name, arg2 );

    if ( str_cmp( arg1, "ventriloquate" ) ) {
    /*@@@@@*/
    act("You prance about like Harry friggin' Potter.", ch, NULL, NULL, TO_CHAR, 0);
    act("$n prances about like Harry friggin' Potter.", ch, NULL, NULL, TO_ROOM, 0);
    return;

    if ( IS_NULLSTR(arg1) )
    {
	send_to_char( "Cast which what where?\n\r", ch );
	return;
    }

    if ((sn = find_spell(ch,arg1)) < 1
    ||  skill_table[sn].spell_fun == spell_null
    ||  ch->learned[sn] == 0
    ||  (!IS_NPC(ch) && skill_table[sn].available[ch->race]))
    {
	send_to_char( "You don't know any rituals of that name.\n\r", ch );
	return;
    }

    if ( ch->position < skill_table[sn].minimum_position )
    {
	send_to_char( "You can't concentrate enough.\n\r", ch );
	return;
    }

    } /* End Casting lockdown. */

    /* This is probably temporary. */
    if ((sn = find_spell(ch,arg1)) < 1
    ||  skill_table[sn].spell_fun == spell_null)
    {
	send_to_char( "Something went wrong.\n\r", ch );
	return;
    }

    /*
     * Locate targets.
     */
    victim	= NULL;
    obj		= NULL;
    vo		= NULL;
    target	= TARGET_NONE;
      
    switch ( skill_table[sn].target )
    {
    default:
	log_string(LOG_BUG, Format("Do_cast: bad target for sn %d.", sn ));
	return;

    case TAR_IGNORE:
	break;

    case TAR_CHAR_OFFENSIVE:
	if ( IS_NULLSTR(arg2) )
	{
	    if ( ( victim = ch->fighting ) == NULL )
	    {
		send_to_char( "Cast the spell on whom?\n\r", ch );
		return;
	    }
	}
	else
	{
	    if ( ( victim = get_char_room( ch, target_name ) ) == NULL )
	    {
		send_to_char( "They aren't here.\n\r", ch );
		return;
	    }
	}

        if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
	{
	    send_to_char( "You can't do that on your own follower.\n\r",
		ch );
	    return;
	}

	vo = (void *) victim;
	target = TARGET_CHAR;
	break;

    case TAR_CHAR_DEFENSIVE:
	if ( IS_NULLSTR(arg2) )
	{
	    victim = ch;
	}
	else
	{
	    if ( ( victim = get_char_room( ch, target_name ) ) == NULL )
	    {
		send_to_char( "They aren't here.\n\r", ch );
		return;
	    }
	}

	vo = (void *) victim;
	target = TARGET_CHAR;
	break;

    case TAR_CHAR_SELF:
	if ( !IS_NULLSTR(arg2) && !is_name( target_name, ch->name ) )
	{
	    send_to_char( "You cannot cast this spell on another.\n\r", ch );
	    return;
	}

	vo = (void *) ch;
	target = TARGET_CHAR;
	break;

    case TAR_OBJ_INV:
	if ( IS_NULLSTR(arg2) )
	{
	    send_to_char( "What should the spell be cast upon?\n\r", ch );
	    return;
	}

	if ( ( obj = get_obj_carry( ch, target_name, ch ) ) == NULL )
	{
	    send_to_char( "You are not carrying that.\n\r", ch );
	    return;
	}

	vo = (void *) obj;
	target = TARGET_OBJ;
	break;

    case TAR_OBJ_CHAR_OFF:
	if (IS_NULLSTR(arg2))
	{
	    if ((victim = ch->fighting) == NULL)
	    {
		send_to_char("Cast the spell on whom or what?\n\r",ch);
		return;
	    }
	
	    target = TARGET_CHAR;
	}
	else if ((victim = get_char_room(ch,target_name)) != NULL)
	{
	    target = TARGET_CHAR;
	}

	if (target == TARGET_CHAR) /* check the sanity of the attack */
	{

            if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
            {
                send_to_char( "You can't do that on your own follower.\n\r", ch );
                return;
            }

	    vo = (void *) victim;
 	}
	else if ((obj = get_obj_here(ch,target_name)) != NULL)
	{
	    vo = (void *) obj;
	    target = TARGET_OBJ;
	}
	else
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
	break; 

    case TAR_OBJ_CHAR_DEF:
        if (IS_NULLSTR(arg2))
        {
            vo = (void *) ch;
            target = TARGET_CHAR;
        }
        else if ((victim = get_char_room(ch,target_name)) != NULL)
        {
            vo = (void *) victim;
            target = TARGET_CHAR;
	}
	else if ((obj = get_obj_carry(ch,target_name,ch)) != NULL)
	{
	    vo = (void *) obj;
	    target = TARGET_OBJ;
	}
	else
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
	break;
    }

    if ( str_cmp( skill_table[sn].name, "ventriloquate" ) )
	say_spell( ch, sn );
    
    /* FIX - Put in tests into spells (abil calls/skill calls) */
  
    WAIT_STATE( ch, skill_table[sn].beats );

    (*skill_table[sn].spell_fun) ( sn, ch->trust, ch, vo,target);

    if ((skill_table[sn].target == TAR_CHAR_OFFENSIVE
    ||   (skill_table[sn].target == TAR_OBJ_CHAR_OFF && target == TARGET_CHAR))
    &&   victim != ch
    &&   victim->master != ch)
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	for ( vch = ch->in_room->people; vch; vch = vch_next )
	{
	    vch_next = vch->next_in_room;
	    if ( victim == vch && victim->fighting == NULL )
	    {
		multi_hit( victim, ch, TYPE_UNDEFINED );
		break;
	    }
	}
    }

    return;
}


/*
 * Cast spells at targets using a magical object.
 */
void obj_cast_spell( int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj )
{
    void *vo;
    int target = TARGET_NONE;

    if ( sn <= 0 )
	return;

    if ( sn >= MAX_SKILL || skill_table[sn].spell_fun == spell_null )
    {
	log_string(LOG_BUG, Format("Obj_cast_spell: bad sn %d.", sn ));
	return;
    }

    switch ( skill_table[sn].target )
    {
    default:
	log_string(LOG_BUG, Format("Obj_cast_spell: bad target for sn %d.", sn ));
	return;

    case TAR_IGNORE:
	vo = NULL;
	break;

    case TAR_CHAR_OFFENSIVE:
	if ( victim == NULL )
	    victim = ch->fighting;
	if ( victim == NULL )
	{
	    send_to_char( "You can't do that.\n\r", ch );
	    return;
	}
	vo = (void *) victim;
	target = TARGET_CHAR;
	break;

    case TAR_CHAR_DEFENSIVE:
    case TAR_CHAR_SELF:
	if ( victim == NULL )
	    victim = ch;
	vo = (void *) victim;
	target = TARGET_CHAR;
	break;

    case TAR_OBJ_INV:
	if ( obj == NULL )
	{
	    send_to_char( "You can't do that.\n\r", ch );
	    return;
	}
	vo = (void *) obj;
	target = TARGET_OBJ;
	break;

    case TAR_OBJ_CHAR_OFF:
        if ( victim == NULL && obj == NULL)
	{
	    if (ch->fighting != NULL)
		victim = ch->fighting;
	    else
	    {
		send_to_char("You can't do that.\n\r",ch);
		return;
	    }
	}

	    if (victim != NULL)
	    {
		vo = (void *) victim;
		target = TARGET_CHAR;
	    }
	    else
	    {
	    	vo = (void *) obj;
	    	target = TARGET_OBJ;
	    }
        break;


    case TAR_OBJ_CHAR_DEF:
	if (victim == NULL && obj == NULL)
	{
	    vo = (void *) ch;
	    target = TARGET_CHAR;
	}
	else if (victim != NULL)
	{
	    vo = (void *) victim;
	    target = TARGET_CHAR;
	}
	else
	{
	    vo = (void *) obj;
	    target = TARGET_OBJ;
	}
	
	break;
    }

    target_name = "";
    (*skill_table[sn].spell_fun) ( sn, level, ch, vo,target);

    

    if ( (skill_table[sn].target == TAR_CHAR_OFFENSIVE
    ||   (skill_table[sn].target == TAR_OBJ_CHAR_OFF && target == TARGET_CHAR))
    &&   victim != ch
    &&   victim->master != ch )
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	for ( vch = ch->in_room->people; vch; vch = vch_next )
	{
	    vch_next = vch->next_in_room;
	    if ( victim == vch && victim->fighting == NULL )
	    {
		multi_hit( victim, ch, TYPE_UNDEFINED );
		break;
	    }
	}
    }

    return;
}


/*
 * Spell functions.
 */
void spell_blindness( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_BLIND) || saves_spell(level,victim,DAM_OTHER))
	return;

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.location  = APPLY_DEX;
    af.modifier  = -4;
    af.duration  = 1+level;
    af.bitvector = AFF_BLIND;
    affect_to_char( victim, &af );
    send_to_char( "You are blinded!\n\r", victim );
    act("$n appears to be blinded.",victim,NULL,NULL,TO_ROOM,0);
    return;
}

void spell_charm_person( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	int duration = 0;

	if ( victim == ch )
	{
		send_to_char( "You like yourself even better!\n\r", ch );
		return;
	}

	if(target == 0)
	{
		duration = level;

		if ( IS_AFFECTED(victim, AFF_CHARM)
				||   IS_AFFECTED(ch, AFF_CHARM)
				||   IS_SET(victim->imm_flags,IMM_CHARM)
				||   (dice_rolls( ch, get_curr_stat(ch, STAT_MAN) + ch->ability[LEADERSHIP].value, victim->willpower) <= 0) )
			return;
	}
	else
	{
		if ( IS_AFFECTED(victim, AFF_CHARM)
				||   IS_AFFECTED(ch, AFF_CHARM)
				||   IS_SET(victim->imm_flags,IMM_CHARM)
				||   ((duration = dice_rolls( ch, get_curr_stat(ch, STAT_CHA) + ch->ability[LEADERSHIP].value, victim->willpower)) <= 0) )
			return;
	}

	if ( victim->master )
		stop_follower( victim );
	add_follower( victim, ch );
	victim->leader = ch;
	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level	 = level;
	af.duration  = number_fuzzy( duration * 2 );
	af.location  = 0;
	af.modifier  = 0;
	af.bitvector = AFF_CHARM;
	affect_to_char( victim, &af );
	act( "Isn't $n just so nice?", ch, NULL, victim, TO_VICT, 1 );
	if ( ch != victim )
		act("$N looks at you with adoring eyes.",ch,NULL,victim,TO_CHAR,1);
	return;
}

void spell_create_food( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *mushroom;

    mushroom = create_object( get_obj_index( OBJ_VNUM_GRUEL ) );
    mushroom->value[0] = level;
    mushroom->value[1] = level;
    obj_to_room( mushroom, ch->in_room );
    act( "$p suddenly appears.", ch, mushroom, NULL, TO_ROOM, 0 );
    act( "$p suddenly appears.", ch, mushroom, NULL, TO_CHAR, 0 );
    return;
}

void spell_cure_blindness(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( !is_affected( victim, gsn_blindness ) )
    {
        if (victim == ch)
          send_to_char("You aren't blind.\n\r",ch);
        else
          act("$N doesn't appear to be blinded.",ch,NULL,victim,TO_CHAR,1);
        return;
    }
 
    if (check_dispel(level,victim,gsn_blindness))
    {
        send_to_char( "Your vision returns!\n\r", victim );
        act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM,0);
    }
    else
        send_to_char("Spell failed.\n\r",ch);
}


void spell_cure_disease( int sn, int level, CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( !is_affected( victim, gsn_plague ) )
    {
        if (victim == ch)
          send_to_char("You aren't ill.\n\r",ch);
        else
          act("$N doesn't appear to be diseased.",ch,NULL,victim,TO_CHAR,1);
        return;
    }
    
    if (check_dispel(level,victim,gsn_plague))
    {
	send_to_char("Your sores vanish.\n\r",victim);
	act("$n looks relieved as $s sores vanish.",victim,NULL,NULL,TO_ROOM,0);
    }
    else
	send_to_char("Spell failed.\n\r",ch);
}


void spell_cure_poison( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
 
    if ( !is_affected( victim, gsn_poison ) )
    {
        if (victim == ch)
          send_to_char("You aren't poisoned.\n\r",ch);
        else
          act("$N doesn't appear to be poisoned.",ch,NULL,victim,TO_CHAR,1);
        return;
    }
 
    if (check_dispel(level,victim,gsn_poison))
    {
        send_to_char("A warm feeling runs through your body.\n\r",victim);
        act("$n looks much better.",victim,NULL,NULL,TO_ROOM,0);
    }
    else
        send_to_char("Spell failed.\n\r",ch);
}

void spell_curse( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;

    /* deal with the object case first */
    if (target == TARGET_OBJ)
    {
        obj = (OBJ_DATA *) vo;
        if (IS_OBJ_STAT(obj,ITEM_EVIL))
        {
            act("$p is already filled with evil.",ch,obj,NULL,TO_CHAR,1);
            return;
        }

        if (IS_OBJ_STAT(obj,ITEM_BLESS))
        {
            AFFECT_DATA *paf;

            paf = affect_find(obj->affected,skill_lookup("bless"));
        }

        af.where        = TO_OBJECT;
        af.type         = sn;
        af.level        = level;
        af.duration     = 2 * level;
        af.location     = APPLY_STA;
        af.modifier     = +1;
        af.bitvector    = ITEM_EVIL;
        affect_to_obj(obj,&af);

        act("$p glows with a malevolent aura.",ch,obj,NULL,TO_ALL,0);

	if (obj->wear_loc != WEAR_NONE)
	    ch->saving_throw += 1;
        return;
    }

    /* character curses */
    victim = (CHAR_DATA *) vo;

    if (IS_AFFECTED(victim,AFF_CURSE) || saves_spell(level,victim,DAM_NEGATIVE))
	return;
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 2*level;
    af.location  = APPLY_DEX;
    af.modifier  = -1 * (level / 8);
    af.bitvector = AFF_CURSE;
    affect_to_char( victim, &af );

    af.location  = APPLY_STA;
    af.modifier  = level / 8;
    affect_to_char( victim, &af );

    send_to_char( "You feel unclean.\n\r", victim );
    if ( ch != victim )
	act("$N looks very uncomfortable.",ch,NULL,victim,TO_CHAR,1);
    return;
}

void spell_detect_hidden(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DETECT_HIDDEN) )
    {
        if (victim == ch)
          send_to_char("You are already as alert as you can be. \n\r",ch);
        else
          act("$N can already sense hidden lifeforms.", ch,NULL,victim,TO_CHAR,1);
        return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_DETECT_HIDDEN;
    affect_to_char( victim, &af );
    send_to_char( "Your awareness improves.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}


void spell_detect_invis( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DETECT_INVIS) )
    {
        if (victim == ch)
          send_to_char("You can already see invisible.\n\r",ch);
        else
          act("$N can already see invisible things.",ch,NULL,victim,TO_CHAR,1);
        return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_INVIS;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes tingle.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}


void spell_detect_poison( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;

    if ( obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOOD )
    {
	if ( obj->value[3] != 0 )
	    send_to_char( "You smell poisonous fumes.\n\r", ch );
	else
	    send_to_char( "It looks delicious.\n\r", ch );
    }
    else
    {
	send_to_char( "It doesn't look poisoned.\n\r", ch );
    }

    return;
}


void spell_dispel_evil( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam = 0;
  
    if ( !IS_NPC(ch) && !IS_NATURAL(ch) )
	victim = ch;

    dam = UMAX(victim->agghealth, dice(level,4));
    if ( saves_spell( level, victim,DAM_HOLY) )
	dam /= 2;

    if (IS_NATURAL(victim))
	dam = 0;

    damage( ch, victim, dam, sn, DAM_HOLY ,TRUE, 1, -1);
    return;
}


void spell_fly( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_FLYING) )
    {
	if (victim == ch)
	  send_to_char("You are already airborne.\n\r",ch);
	else
	  act("$N doesn't need your help to fly.",ch,NULL,victim,TO_CHAR,1);
	return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level + 3;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_FLYING;
    affect_to_char( victim, &af );
    send_to_char( "Your feet rise off the ground.\n\r", victim );
    act( "$n's feet rise off the ground.", victim, NULL, NULL, TO_ROOM, 0 );
    return;
}

void spell_identify( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	AFFECT_DATA *paf;

	send_to_char( Format("Object '%s' is type %s, extra flags %s.\n\rWeight is %d, value is %d.\n\r", obj->name,
			item_name(obj->item_type), flag_string( extra_flags, obj->extra_flags ), obj->weight / 10, obj->cost), ch );

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

		if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
		{
			send_to_char( " '", ch );
			send_to_char( skill_table[obj->value[3]].name, ch );
			send_to_char( "'", ch );
		}

		send_to_char( ".\n\r", ch );
		break;

	case ITEM_DRINK_CON:
		send_to_char(Format("It holds %s-colored %s.\n\r", liq_table[obj->value[2]].liq_color, liq_table[obj->value[2]].liq_name),ch);
		break;

	case ITEM_CONTAINER:
		send_to_char(Format("Capacity: %d#  Maximum weight: %d#  flags: %s\n\r", obj->value[0], obj->value[3],
				flag_string(container_flags, obj->value[1])),ch);
		if (obj->value[4] != 100)
		{
			send_to_char(Format("Weight multiplier: %d%%\n\r", obj->value[4]),ch);
		}
		break;

	case ITEM_WEAPON:
		send_to_char("Weapon type is ",ch);
		switch (obj->value[0])
		{
		case(WEAPON_EXOTIC) : send_to_char("exotic.\n\r",ch);	break;
		case(WEAPON_FIREARM): send_to_char("firearm.\n\r",ch);	break;
		case(WEAPON_BLADE)  : send_to_char("blade.\n\r",ch);	break;
		case(WEAPON_BLUNT)	: send_to_char("blunt.\n\r",ch);	break;
		case(WEAPON_GRAPPLE): send_to_char("grapple.\n\r",ch);	break;
		case(WEAPON_WHIP)	: send_to_char("whip.\n\r",ch);		break;
		default		: send_to_char("unknown.\n\r",ch);	break;
		}
		send_to_char( Format("Difficulty is %d, Damage is at %d.\n\r", obj->value[1], obj->value[2]), ch );
		if (obj->value[4])  /* weapon flags */
		{
			send_to_char(Format("Weapons flags: %s\n\r", flag_string(weapon_flags, obj->value[4])),ch);
		}
		break;

		case ITEM_ARMOR:
			send_to_char( Format("Armor class is %d pierce, %d bash, %d slash, and %d vs. magic.\n\r", obj->value[0], obj->value[1], obj->value[2], obj->value[3]), ch );
			break;
	}

	if (!obj->enchanted)
		for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
		{
			if ( paf->location != APPLY_NONE && paf->modifier != 0 )
			{
				send_to_char(Format("Affects %s by %d.\n\r", flag_string( apply_flags, paf->location ), paf->modifier),ch);
				if (paf->bitvector)
				{
					switch(paf->where)
					{
					case TO_AFFECTS:
						send_to_char(Format("Adds %s affect.\n", flag_string(affect_flags, paf->bitvector)), ch);
						break;
					case TO_OBJECT:
						send_to_char(Format("Adds %s object flag.\n", flag_string( extra_flags, paf->bitvector)), ch);
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
		}

	for ( paf = obj->affected; paf != NULL; paf = paf->next )
	{
		if ( paf->location != APPLY_NONE && paf->modifier != 0 )
		{
			send_to_char( Format("Affects %s by %d", flag_string( apply_flags, paf->location ), paf->modifier), ch );
			if ( paf->duration > -1)
			{
				send_to_char(Format(", %d hours.\n\r",paf->duration), ch);
			}
			else
			{
				send_to_char(".\n\r", ch);
			}

			if (paf->bitvector)
			{
				switch(paf->where)
				{
				case TO_AFFECTS:
					send_to_char(Format("Adds %s affect.\n", flag_string(affect_flags, paf->bitvector)), ch);
					break;
				case TO_OBJECT:
					send_to_char(Format("Adds %s object flag.\n", flag_string( extra_flags, paf->bitvector)), ch);
					break;
				case TO_WEAPON:
					send_to_char(Format("Adds %s weapon flags.\n", flag_string(weapon_flags, paf->bitvector)), ch);
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
	}

	return;
}


void spell_invis( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	AFFECT_DATA af;

	/* object invisibility */
	if (target == TARGET_OBJ)
	{
		obj = (OBJ_DATA *) vo;

		if (IS_OBJ_STAT(obj,ITEM_INVIS))
		{
			act("$p is already invisible.",ch,obj,NULL,TO_CHAR,1);
			return;
		}

		af.where	= TO_OBJECT;
		af.type		= sn;
		af.level	= level;
		af.duration	= level + 12;
		af.location	= APPLY_NONE;
		af.modifier	= 0;
		af.bitvector	= ITEM_INVIS;
		affect_to_obj(obj,&af);

		act("$p fades out of sight.",ch,obj,NULL,TO_ALL,0);
		return;
	}

	/* character invisibility */
	victim = (CHAR_DATA *) vo;

	if ( IS_AFFECTED(victim, AFF_INVISIBLE) )
		return;

	act( "$n fades out of existence.", victim, NULL, NULL, TO_ROOM, 0 );

	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level + 12;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_INVISIBLE;
	affect_to_char( victim, &af );
	send_to_char( "You fade out of existence.\n\r", victim );
	return;
}

void spell_locate_object( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
	BUFFER *buffer;
	OBJ_DATA *obj;
	OBJ_DATA *in_obj;
	char buf[MSL]={'\0'};
	int number = 0;
	int max_found = IS_ADMIN(ch) ? 200 : 2 * level;
	bool found = FALSE;
	
	buffer = new_buf();

	for ( obj = object_list; obj != NULL; obj = obj->next )
	{
		if ( !can_see_obj( ch, obj ) || !is_name( target_name, obj->name )
				||   IS_OBJ_STAT(obj,ITEM_NOLOCATE) )
			continue;

		found = TRUE;
		number++;

		for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
			;

		if ( in_obj->carried_by != NULL && can_see(ch,in_obj->carried_by))
		{
			snprintf( buf, sizeof(buf), "one is carried by %s\n\r", PERS(in_obj->carried_by, ch) );
		}
		else
		{
			if (IS_ADMIN(ch) && in_obj->in_room != NULL)
				snprintf( buf, sizeof(buf), "one is in %s [Room %d]\n\r", in_obj->in_room->name, in_obj->in_room->vnum);
			else
				snprintf( buf, sizeof(buf), "one is in %s\n\r", in_obj->in_room == NULL ? "somewhere" : in_obj->in_room->name );
		}

		buf[0] = UPPER(buf[0]);
		add_buf(buffer,buf);

		if (number >= max_found)
			break;
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

	return;
}

void spell_null( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    send_to_char( "That's not a spell!\n\r", ch );
    return;
}


/* RT plague spell, very nasty */
void spell_plague( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (saves_spell(level,victim,DAM_DISEASE) )
	{
		if (ch == victim)
			send_to_char("You feel momentarily ill, but it passes.\n\r",ch);
		else
			act("$N seems to be unaffected.",ch,NULL,victim,TO_CHAR,1);
		return;
	}

	af.where     = TO_AFFECTS;
	af.type 	  = sn;
	af.level	  = level * 3/4;
	af.duration  = level;
	af.location  = APPLY_STR;
	af.modifier  = -5;
	af.bitvector = AFF_PLAGUE;
	affect_join(victim,&af);

	send_to_char("You scream in agony as plague sores erupt from your skin.\n\r",victim);
	act("$n screams in agony as plague sores erupt from $s skin.", victim,NULL,NULL,TO_ROOM,0);
}

void spell_poison( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;

    if (target == TARGET_OBJ)
    {
	obj = (OBJ_DATA *) vo;

	if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON)
	{
	    if (IS_OBJ_STAT(obj,ITEM_BLESS) || IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
	    {
		act("Your spell fails to corrupt $p.",ch,obj,NULL,TO_CHAR,1);
		return;
	    }
	    obj->value[3] = 1;
	    act("$p is infused with poisonous vapors.",ch,obj,NULL,TO_ALL,0);
	    return;
	}

	if (obj->item_type == ITEM_WEAPON)
	{
	    if (IS_WEAPON_STAT(obj,WEAPON_FLAMING)
	    ||  IS_WEAPON_STAT(obj,WEAPON_FROST)
	    ||  IS_WEAPON_STAT(obj,WEAPON_VAMPIRIC)
	    ||  IS_WEAPON_STAT(obj,WEAPON_SHOCKING)
	    ||  IS_OBJ_STAT(obj,ITEM_BLESS) || IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
	    {
		act("You can't seem to envenom $p.",ch,obj,NULL,TO_CHAR,1);
		return;
	    }

	    if (IS_WEAPON_STAT(obj,WEAPON_POISON))
	    {
		act("$p is already envenomed.",ch,obj,NULL,TO_CHAR,1);
		return;
	    }

	    af.where	 = TO_WEAPON;
	    af.type	 = sn;
	    af.level	 = level / 2;
	    af.duration	 = level/8;
 	    af.location	 = 0;
	    af.modifier	 = 0;
	    af.bitvector = WEAPON_POISON;
	    affect_to_obj(obj,&af);

	    act("$p is coated with deadly venom.",ch,obj,NULL,TO_ALL,0);
	    return;
	}

	act("You can't poison $p.",ch,obj,NULL,TO_CHAR,1);
	return;
    }

    victim = (CHAR_DATA *) vo;

    if ( saves_spell( level, victim,DAM_POISON) )
    {
	act("$n turns slightly green, but it passes.", victim,NULL,NULL,TO_ROOM,0);
	send_to_char("You feel momentarily ill, but it passes.\n\r",victim);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_STR;
    af.modifier  = -2;
    af.bitvector = AFF_POISON;
    affect_join( victim, &af );
    send_to_char( "You feel very sick.\n\r", victim );
    act("$n looks very ill.",victim,NULL,NULL,TO_ROOM,0);
    return;
}


void spell_protection_evil(int sn,int level,CHAR_DATA *ch,void *vo, int target)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if ( IS_AFFECTED(victim, AFF_PROTECT_EVIL) )
		/* FIX */

	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = 24;
	af.location  = APPLY_STA;
	af.modifier  = -1;
	af.bitvector = AFF_PROTECT_EVIL;
	affect_to_char( victim, &af );
	send_to_char( "You feel holy and pure.\n\r", victim );
	if ( ch != victim )
		act("$N is protected from evil.",ch,NULL,victim,TO_CHAR,1);
	return;
}
 
void spell_remove_curse( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    bool found = FALSE;

    /* do object cases first */
    if (target == TARGET_OBJ)
    {
	obj = (OBJ_DATA *) vo;

	if (IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_NOREMOVE))
	{
	    if (!IS_OBJ_STAT(obj,ITEM_NOUNCURSE)
	    &&  !saves_dispel(0,0,0)) /* FIX */
	    {
		REMOVE_BIT(obj->extra_flags,ITEM_NODROP);
		REMOVE_BIT(obj->extra_flags,ITEM_NOREMOVE);
		act("$p glows blue.",ch,obj,NULL,TO_ALL,0);
		return;
	    }

	    act("The curse on $p is beyond your power.",ch,obj,NULL,TO_CHAR,1);
	    return;
	}
	act("There doesn't seem to be a curse on $p.",ch,obj,NULL,TO_CHAR,1);
	return;
    }

    /* characters */
    victim = (CHAR_DATA *) vo;

    if (check_dispel(level,victim,gsn_curse))
    {
	send_to_char("You feel better.\n\r",victim);
	act("$n looks more relaxed.",victim,NULL,NULL,TO_ROOM,0);
    }

   for (obj = victim->carrying; (obj != NULL && !found); obj = obj->next_content)
   {
        if ((IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_NOREMOVE))
	&&  !IS_OBJ_STAT(obj,ITEM_NOUNCURSE))
        {   /* attempt to remove curse */
            if (!saves_dispel(0,0,0)) /* FIX */
            {
                found = TRUE;
                REMOVE_BIT(obj->extra_flags,ITEM_NODROP);
                REMOVE_BIT(obj->extra_flags,ITEM_NOREMOVE);
                act("Your $p glows blue.",victim,obj,NULL,TO_CHAR,1);
                act("$n's $p glows blue.",victim,obj,NULL,TO_ROOM,0);
            }
         }
    }
}

void spell_sleep( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
  
    if ( IS_AFFECTED(victim, AFF_SLEEP) )
	return;

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 4 + level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_SLEEP;
    affect_join( victim, &af );

    if ( IS_AWAKE(victim) )
    {
	send_to_char( "You feel very sleepy ..... zzzzzz.\n\r", victim );
	act( "$n goes to sleep.", victim, NULL, NULL, TO_ROOM, 0 );
	victim->position = P_SLEEP;
    }
    return;
}

void spell_slow( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
    if ( is_affected( victim, sn ) || IS_AFFECTED(victim,AFF_SLOW))
    {
        if (victim == ch)
          send_to_char("You can't move any slower!\n\r",ch);
        else
          act("$N can't get any slower than that.", ch,NULL,victim,TO_CHAR,1);
        return;
    }
 
    if (saves_spell(level,victim,DAM_OTHER) 
    ||  IS_SET(victim->imm_flags,IMM_MAGIC))
    {
	if (victim != ch)
            send_to_char("Nothing seemed to happen.\n\r",ch);
        send_to_char("You feel momentarily lethargic.\n\r",victim);
        return;
    }
 
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/2;
    af.location  = APPLY_DEX;
    af.modifier  = -1 - (level >= 18) - (level >= 25) - (level >= 32);
    af.bitvector = AFF_SLOW;
    affect_to_char( victim, &af );
    send_to_char( "You feel yourself slowing d o w n...\n\r", victim );
    act("$n starts to move in slow motion.",victim,NULL,NULL,TO_ROOM,0);
    return;
}


void spell_ventriloquate( int sn, int level, CHAR_DATA *ch,void *vo,int target)
{
	char buf1[MSL]={'\0'};
	char buf2[MSL]={'\0'};
	char speaker[MAX_INPUT_LENGTH]={'\0'};
	CHAR_DATA *vch;

	target_name = one_argument( target_name, speaker );

	snprintf( buf1, sizeof(buf1), "%s says '%s'.\n\r",              speaker, target_name );
	snprintf( buf2, sizeof(buf2), "Someone makes %s say '%s'.\n\r", speaker, target_name );
	buf1[0] = UPPER(buf1[0]);

	for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
	{
		if (!is_exact_name( speaker, vch->name) && IS_AWAKE(vch))
			send_to_char( saves_spell(level,vch,DAM_OTHER) ? buf2 : buf1, vch );
	}

	return;
}

extern char *target_name;

/*
 * Disciplines, Bunks/Cantrips, Spirituality stuff, powers
 */
void do_resist(CHAR_DATA *ch, char *argument)
{
	if(!IS_SET(ch->act2, ACT2_RESIST))
	{
		SET_BIT(ch->act2, ACT2_RESIST);
		send_to_char("You will now use willpower to attempt to resist outside influences.\n\r", ch);
	}
	else
	{
		REMOVE_BIT(ch->act2, ACT2_RESIST);
		send_to_char("You will no longer use willpower to resist outside influences.\n\r", ch);
	}
}


void do_melpominee1 (CHAR_DATA *ch, char *argument)
{

	if(ch->race != race_lookup("vampire")
			|| ch->disc[DISC_MELPOMINEE] < 1)
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	ch->power_timer = 1;
	do_cast(ch, (char *)Format("ventriloquate %s", argument));
}


void do_melpominee3 (CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *vch, *vch_next;
	char arg[MSL]={'\0'};
	int fail = 0, door = 0;

	if(ch->race != race_lookup("vampire")
			|| ch->disc[DISC_MELPOMINEE] < 3)
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	argument = one_argument(arg, argument);

	if(IS_NULLSTR(arg))
	{
		send_to_char("What emotion do you wish to affect?\n\r", ch);
		send_to_char("Syntax: madrigal <emotion> <song lyrics>\n\r", ch);
		send_to_char("(Only fear and anger are currently coded, other emotions\n\r", ch);
		send_to_char("must be roleplayed.)\n\r", ch);
		return;
	}

	if(IS_NULLSTR(argument))
	{
		send_to_char("You have to sing something to evoke the emotion.\n\r",ch);
		send_to_char("Syntax: madrigal <emotion> <song lyrics>\n\r", ch);
		return;
	}

	fail = dice_rolls(ch, get_curr_stat(ch, STAT_CHA)
			+ ch->ability[PERFORMANCE].value, 7);

	ch->power_timer = 2;

	/* Sing the song */
	if(IS_AFFECTED(ch, AFF_HIDE) || IS_AFFECTED(ch, AFF_INVISIBLE)
			|| IS_AFFECTED(ch, AFF_SNEAK))
		do_visible(ch, "");

	stanza(argument);
	act("$n $t", ch, argument, NULL, TO_CHAR, 1);
	act("$n $t", ch, argument, NULL, TO_ROOM, 0);

	if(IS_SET(ch->in_room->room_flags, ROOM_JAMMIN)
			&& (IS_SET(ch->comm, COMM_JAMMIN)
					|| (ch->leader && IS_SET(ch->leader->comm, COMM_JAM_LEAD))))
		for(door = 0; door < 6; door++)
		{
			if(ch->in_room->exit[door])
				act("$n $t", ch, argument, ch->in_room->exit[door]->u1.to_room, TO_OROOM, 0);
		}

	/* Apply the effect. */
	if(fail > 0)
	{
		for(vch = ch->in_room->people; vch != NULL; vch = vch_next)
		{
			vch_next = vch->next_in_room;
			if(number_range(1, 5) <= fail)
			{
				if(!str_cmp("fear", arg))
					gain_condition(vch, COND_FEAR, 5*fail);
				else if(!str_cmp("anger", arg))
					gain_condition(vch, COND_ANGER, 5*fail);
				act("You feel an overwhelming rush of $t.", vch, arg, NULL, TO_CHAR, 0);
			}
		}
		if(IS_SET(ch->in_room->room_flags, ROOM_JAMMIN)
				&& (IS_SET(ch->comm, COMM_JAMMIN)
						|| (ch->leader && IS_SET(ch->leader->comm, COMM_JAM_LEAD))))
			for(door = 0; door < 6; door++)
			{
				if(ch->in_room->exit[door])
					for(vch = ch->in_room->exit[door]->u1.to_room->people;
							vch != NULL; vch = vch_next)
					{
						vch_next = vch->next_in_room;
						if(number_range(1, 5) <= fail)
						{
							if(!str_cmp("fear", arg))
								gain_condition(vch, COND_FEAR, 5*fail);
							else if(!str_cmp("anger", arg))
								gain_condition(vch, COND_ANGER, 5*fail);
							act("You feel an overwhelming rush of $t.", vch, arg, NULL, TO_CHAR, 0);
						}
					}
			}
	}
}


void do_melpominee4 (CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *vch;
	AFFECT_DATA af;
	AFFECT_DATA *paf;
	TRAIT_DATA *trait;
	char arg[MSL]={'\0'};
	int fail = 0, door = 0;

	if(ch->race != race_lookup("vampire")
			|| ch->disc[DISC_MELPOMINEE] < 4)
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if(ch->power_timer > 0
			&& !is_affected(ch, skill_lookup("virtuosa")))
	{
		send_to_char("Your powers have not rejuvenated yet.\n\r", ch);
		return;
	}

	argument = one_argument(arg, argument);

	if(IS_NULLSTR(arg))
	{
		send_to_char("Whom are you trying to affect?\n\r", ch);
		send_to_char("Syntax: sirenbeckon <character> <song lyrics>\n\r", ch);
		return;
	}

	if(IS_NULLSTR(argument))
	{
		send_to_char("You have to sing something to have an effect.\n\r",ch);
		send_to_char("Syntax: sirenbeckon <character> <song lyrics>\n\r", ch);
		return;
	}

	if((vch = get_char_room(ch, arg)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	fail = dice_rolls(ch, get_curr_stat(ch, STAT_MAN)
			+ ch->ability[PERFORMANCE].value, vch->willpower);

	ch->power_timer = 1;

	/* Sing the song */
	if(IS_AFFECTED(ch, AFF_HIDE) || IS_AFFECTED(ch, AFF_INVISIBLE)
			|| IS_AFFECTED(ch, AFF_SNEAK))
		do_visible(ch, "");

	stanza(argument);
	act("$n $t", ch, argument, NULL, TO_CHAR, 1);
	act("$n $t", ch, argument, NULL, TO_ROOM, 0);

    if(IS_SET(ch->in_room->room_flags, ROOM_JAMMIN)
         && (IS_SET(ch->comm, COMM_JAMMIN)
         || (ch->leader && IS_SET(ch->leader->comm, COMM_JAM_LEAD))))
        for(door = 0; door < 6; door++)
        {
          if(ch->in_room->exit[door])
            act("$n $t", ch, argument, ch->in_room->exit[door]->u1.to_room, TO_OROOM, 0);
        }

    /* Apply the effect. */
    if(fail > 0)
    {
	af.where     = TO_AFFECTS;
	af.type      = skill_lookup("siren's beckoning");
	af.level     = ch->disc[DISC_MELPOMINEE];
	af.duration  = fail * 5;
	if((paf = affect_find(vch->affected, skill_lookup("siren's beckoning")))
			== NULL)
	{
	    af.location  = APPLY_DERANGEMENT;
	    af.bitvector = number_range(0, 21);
	    /* Pick a sub-flag (if necessary) */
	    if(af.bitvector <= 2)
		af.modifier  = number_range(0, 4);
	    else if(af.bitvector <= 5)
		af.modifier  = number_range(0, 30);
	    else if(af.bitvector == 6)
		af.modifier  = number_range(0, 9);
	    else af.modifier = -1;
	}
	else
	{
	    af.bitvector = paf->bitvector;
	    af.location  = paf->location;
	    af.modifier  = paf->modifier;
	}
	affect_to_char( vch, &af );

	if((paf = affect_find(vch->affected, skill_lookup("siren's beckoning")))
			!= NULL)
	{
	    if(paf->duration / 5 >= 20)
	    {
		/* Make Derangement Permanent */
		trait = new_trait();
		trait->type = 3;
		trait->value = 0;
		PURGE_DATA(trait->qualifier);
		trait->qualifier =
				str_dup(derangement_table[paf->bitvector].name);
		if(paf->bitvector<(int)trait_lookup("obsessive/compulsive",
				derangement_table))
		{
			PURGE_DATA(trait->detail);
		    if(paf->bitvector < 3)
			trait->detail = mania_table[paf->modifier].name;
		    if(paf->bitvector < 6)
			trait->detail = phobia_table[paf->modifier].name;
		    if(paf->bitvector == 6)
			trait->detail = compulsion_table[paf->modifier].name;
		}
		trait->next = ch->traits;
		ch->traits = trait;
		affect_strip(vch, skill_lookup("siren's beckoning"));
	    }
	}
    }
    else if(fail < 0 && is_affected(vch, skill_lookup("siren's beckoning")))
    {
	affect_strip(vch, skill_lookup("siren's beckoning"));
	send_to_char("You seem to have un-done the effect.\n\r", ch);
    }
}


void do_melpominee5 (CHAR_DATA *ch, char *argument)
{
	AFFECT_DATA af;
	char arg[MSL]={'\0'};
	char arg2[MSL]={'\0'};
	char target[MSL]={'\0'};
	char *targetlist;
	int i = 0, targetcount = 0;

	if(ch->race != race_lookup("vampire")
			|| ch->disc[DISC_MELPOMINEE] < 5)
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	argument = one_argument(arg, argument);
	argument = one_argument(arg2, argument);

	if(IS_NULLSTR(arg) || str_prefix(arg, "sirenbeckon"))
	{
		send_to_char("You have to chose a command to use (@@@@@ or sirenbeckon)\n\r", ch);
		send_to_char("Syntax: virtuosa <command> <targets> <message>\n\r", ch);
		send_to_char("(Targets must be separated by a colon ':')\n\r", ch);
		return;
	}

	if(IS_NULLSTR(arg2))
	{
		send_to_char("You have to chose at least one target.\n\r", ch);
		send_to_char("Syntax: virtuosa <command> <targets> <message>\n\r", ch);
		send_to_char("(Targets must be separated by a colon ':')\n\r", ch);
		return;
	}

	if(IS_NULLSTR(argument))
	{
		send_to_char("You have to sing something to have an effect.\n\r",ch);
		send_to_char("Syntax: virtuosa <command> <targets> <message>\n\r", ch);
		send_to_char("(Targets must be separated by a colon ':')\n\r", ch);
		return;
	}

	targetcount = UMIN(count_multi_args(arg2, (char *)':'),
			get_curr_stat(ch, STAT_STA) + ch->ability[PERFORMANCE].value);

	if(ch->RBPG < (targetcount - 1)/5 + (targetcount%5 > 0 ? 1 : 0))
	{
		send_to_char("You don't have enough blood for that.\n\r", ch);
		return;
	}

	ch->power_timer = targetcount;
	ch->RBPG -= (targetcount - 1)/5 + (targetcount%5 > 0 ? 1 : 0);

	af.where     = TO_AFFECTS;
	af.type      = skill_lookup("virtuosa");
	af.level     = ch->disc[DISC_MELPOMINEE];
	af.duration  = -1;
	af.location  = APPLY_DERANGEMENT;
	af.bitvector = -1;
	af.modifier  = -1;
	affect_to_char( ch, &af );

	targetlist = arg2;
	for(i = 0; i < targetcount; i++)
	{
		if(targetlist[0] == '\0') break;
		targetlist = one_chunk(target, targetlist, ':');
		interpret(ch, (char *)Format("%s %s %s", arg, target, argument));
	}

	affect_strip(ch, skill_lookup("virtuosa"));
}


void do_thanatosis1 (CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *obj;

	if(ch->race != race_lookup("vampire") || ch->disc[DISC_THANATOSIS] < 1)
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if((obj = get_obj_carry(ch, argument, ch)) == NULL)
	{
		send_to_char("You don't have that item.\n\r", ch);
		return;
	}

	if(obj->weight > 2)
	{
		send_to_char("That's too big to hide in a wrinkle.\n\r", ch);
		return;
	}

	SET_BIT(obj->extra_flags, ITEM_HIDDEN);
	do_function(ch, &do_wear, (char *)Format("'%s' hagwrinkle", obj->name));
	ch->power_timer = 1;
}


void do_thanatosis2 (CHAR_DATA *ch, char *argument)
{
	AFFECT_DATA af;
	CHAR_DATA *vch;
	int fail = 0, person = 0;

	if(ch->race != race_lookup("vampire") || ch->disc[DISC_THANATOSIS] < 2)
	{
		send_to_char("You have not mastered this power of Thanatosis.\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if((vch = get_char_room(ch, argument)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	ch->power_timer = 3;
	fail = dice_rolls(ch, get_curr_stat(ch, STAT_DEX)
			+ ch->ability[MEDICINE].value,
			get_curr_stat(vch, STAT_STA) + ch->disc[DISC_FORTITUDE]);

	if(fail > 0)
	{
		vch->agghealth--;
		if(IS_SET(vch->form, FORM_PLANT))
		{
			raw_kill(vch, TRUE);
		}
		else if(IS_NATURAL(vch))
		{
			vch->perm_stat[STAT_APP]--;
			person = TRUE;
		}
		else
		{
			af.where     = TO_AFFECTS;
			af.type      = skill_lookup("putrefaction");
			af.level     = ch->disc[DISC_THANATOSIS];
			af.duration  = 10 * ch->disc[DISC_THANATOSIS];
			af.location  = APPLY_APP;
			af.bitvector = AFF_NONE;
			af.modifier  = -1;
			affect_to_char( vch, &af );
			person = TRUE;
		}

		if(person)
		{
			act("You touch $N, putrefying $S arm.", ch, NULL, vch, TO_CHAR, 1);
			act("$n touches you, causing your arm to shrivel.",	ch, NULL, vch, TO_VICT, 1);
			act("$n touches $N, causing $N's arm to shrivel.", ch, NULL, vch, TO_NOTVICT, 1);
		}
		else
		{
			act("You touch $N, killing it.", ch, NULL, vch, TO_CHAR, 1);
			act("$n touches you, killing you because you're a plant.", ch, NULL, vch, TO_VICT, 1);
			act("$n touches $N, causing $N's to shrivel and die.", ch, NULL, vch, TO_NOTVICT, 1);
		}
	}
	else
	{
		act("You touch $N, but nothing happens.", ch, NULL, vch, TO_CHAR, 1);
		act("$n touches you, but nothing happens.", ch, NULL, vch, TO_VICT, 1);
		act("$n touches $N.", ch, NULL, vch, TO_NOTVICT, 1);
	}
}


void do_thanatosis3 (CHAR_DATA *ch, char * string)
{
	OBJ_DATA *obj, *obj_next;

	if((ch->race != race_lookup("vampire"))||(ch->disc[DISC_THANATOSIS] < 3))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if(IS_SET(ch->form,FORM_ASH))
	{
		send_to_char("You re-form from a pile of ash.\n\r", ch);
		act("A pile of ash billows up and solidifies into $n.",
				ch, NULL, NULL, TO_ROOM, 0);
		REMOVE_BIT(ch->form,FORM_ASH);
		REMOVE_BIT(ch->imm_flags,IMM_PIERCE|IMM_SLASH|IMM_FIRE|IMM_COLD|IMM_LIGHTNING|IMM_POISON|IMM_NEGATIVE|IMM_ENERGY|IMM_DISEASE|IMM_LIGHT);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if(ch->RBPG <= 2)
	{
		send_to_char("You don't have enough blood for that.\n\r", ch);
		return;
	}

	ch->power_timer = 3;
	ch->RBPG -= 2;
	WAIT_STATE(ch, 2);
	send_to_char("Your body turns to a pile of ashes.\n\r", ch);
	act("$n collapses into a pile of ashes!", ch, NULL, NULL, TO_ROOM, 0);
	SET_BIT(ch->form, FORM_ASH);
	SET_BIT(ch->imm_flags,IMM_PIERCE|IMM_SLASH|IMM_FIRE|IMM_COLD|IMM_LIGHTNING|IMM_POISON|IMM_NEGATIVE|IMM_ENERGY|IMM_DISEASE|IMM_LIGHT);
	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
		obj_next = obj->next_content;

		obj_from_char( obj );
		obj_to_room( obj, ch->in_room );
		if (IS_OBJ_STAT(obj,ITEM_MELT_DROP))
		{
			act("$p dissolves into ashes.",ch,obj,NULL,TO_ROOM,1);
			act("$p dissolves into ashes.",ch,obj,NULL,TO_CHAR,1);
			extract_obj(obj);
		}
	}
}


bool player_switch( CHAR_DATA *ch, char *argument, bool Animals )
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH]={'\0'};
	int fail = 0;

	one_argument( argument, arg );

	if ( IS_NULLSTR(arg) )
	{
		send_to_char( "Who are you trying to take over?\n\r", ch );
		return FALSE;
	}

	if ( ch->desc == NULL )
		return FALSE;

	if ( ch->desc->original != NULL )
	{
		send_to_char( "You are already outside your own body.\n\r", ch );
		return FALSE;
	}

	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return FALSE;
	}

	if ( victim == ch )
	{
		send_to_char( "What's the point to that?\n\r", ch );
		return FALSE;
	}

	if (!IS_NPC(victim))
	{
		send_to_char("You can't possess players.\n\r",ch);
		return FALSE;
	}

	if(IS_ANIMAL(victim) != Animals)
	{
		if(Animals)
			send_to_char("You cannot affect animals with this power.\n\r", ch);
		else
			send_to_char("You can only affect animals with this power.\n\r", ch);
		return FALSE;
	}

	if (!is_room_owner(ch,victim->in_room) && ch->in_room != victim->in_room
			&&  room_is_private(victim->in_room) && !IS_TRUSTED(ch,MASTER))
	{
		send_to_char("That character is in a private room.\n\r",ch);
		return FALSE;
	}

	if ( victim->desc != NULL && IS_ADMIN(victim->desc->original) )
	{
		send_to_char( "Character in use.\n\r", ch );
		return FALSE;
	}

	wiznet((char*)Format("\tY[WIZNET]\tn $N switches into %s",victim->short_descr), ch,NULL,WIZ_SWITCHES,WIZ_SECURE,get_trust(ch));

	if( victim->desc != NULL && victim->desc->original )
	{
		fail = dice_rolls(ch, ch->max_willpower, victim->desc->original->max_willpower);
		if(fail > 0)
			fail -= dice_rolls(victim->desc->original, victim->desc->original->max_willpower, ch->max_willpower);

		if(fail > 0)
		{
			send_to_char("Your control is usurped!\n\r", victim);
			do_function(victim, &do_return, "");
		} else {
			send_to_char("You fail to take control!\n\r", ch);
			return TRUE;
		}
	} else {
		fail = dice_rolls(ch, ch->max_willpower, victim->max_willpower);
		if(fail > 0)
			fail -= dice_rolls(victim, ch->max_willpower, ch->max_willpower);

		if(fail <= 0)
		{
			send_to_char("You fail to take control!\n\r", ch);
			return TRUE;
		}
	}

	/* REMEMBER TO ADD SLEEP/WAKE STUFF */
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
	do_function(ch, &do_sleep, "");
	victim->comm = ch->comm;
	victim->lines = ch->lines;
	do_function(victim, &do_look, "");
	send_to_char( "You suppress the essence of your victim.\n\r", victim );
	return TRUE;
}


void init_clan_powers(CHAR_DATA *ch, int clan)
{
	int i = 0,k = 0;

	switch(ch->race)
	{
	case RACE_HUMAN:
		break;
	case RACE_WEREWOLF:
		k = clan_table[clan].powers[0];
		ch->disc[k] = 1;
		break;
	case RACE_VAMPIRE:
		for(i = 0; i < 3; i++)
		{
			k = clan_table[clan].powers[i];
			ch->disc[k] = 1;
		}
		break;
	case RACE_CHANGELING:
		for(i = 0; i < 3; i++)
		{
			k = clan_table[clan].powers[i];
			ch->disc[k] = 1;
		}
		break;
	}
	return;
}

const   sh_int  gauntlet   [SECT_MAX]      =
{
    10, 9, 8, 6, 7, 6, 6, 8, 10, 6, 10, 9
};

void do_umbral_walk(CHAR_DATA *ch, char *argument)
{
	int fail = 0;
	OBJ_DATA *obj;

	if (ch->race != race_lookup("werewolf"))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}
	else
	{
		if(time_info.hour < 6 || time_info.hour > 19)
			fail = dice_rolls(ch, get_curr_stat(ch, STAT_PER) + ch->GHB,gauntlet[ch->in_room->sector_type]);
		else
			fail = dice_rolls(ch, ch->virtues[0],gauntlet[ch->in_room->sector_type]);
		if(fail)
		{
			ch->power_timer = 3;
			if(IS_SET(ch->act, ACT_UMBRA))
			{
				REMOVE_BIT(ch->act, ACT_UMBRA);
				send_to_char("You slip from the Umbra.\n\r",ch);
				for(obj=ch->carrying;obj!=NULL;obj=obj->next_content)
					obj_to_plane(obj, 0);
			}
			else
			{
				SET_BIT(ch->act, ACT_UMBRA);
				send_to_char("You slip into the Umbra.\n\r",ch);
				for(obj=ch->carrying;obj!=NULL;obj=obj->next_content)
					obj_to_plane(obj, 1);
			}
		}
		else
		{
			send_to_char("Your efforts prove futile.\n\r",ch);
		}
	}
}

const   sh_int  boundary   [SECT_MAX]      =
{
    10, 9, 8, 6, 7, 6, 6, 8, 10, 6, 10, 9
};

void do_dream_walk(CHAR_DATA *ch, char * argument)
{
	OBJ_DATA *obj;

	if (/* ch->race != race_lookup("faerie") */  !(IS_ADMIN(ch)) )
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}
	else
	{
		if(dice_rolls(ch, get_curr_stat(ch, STAT_PER) + ch->GHB,8))
		{
			ch->power_timer = 3;
			if(IS_SET(ch->act, ACT_DREAMING))
			{
				REMOVE_BIT(ch->act, ACT_DREAMING);
				send_to_char("You wake from the dream.\n\r",ch);
				for(obj=ch->carrying;obj!=NULL;obj=obj->next_content)
					obj_to_plane(obj, 0);
			}
			else
			{
				SET_BIT(ch->act, ACT_DREAMING);
				send_to_char("The dream surrounds you.\n\r",ch);
				for(obj=ch->carrying;obj!=NULL;obj=obj->next_content)
					obj_to_plane(obj, 2);
			}
		}
		else
		{
			send_to_char("Your efforts prove futile.\n\r",ch);
		}
	}
}


void do_scry(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *vch;
	ROOM_INDEX_DATA *location;
	ROOM_INDEX_DATA *original;
	OBJ_DATA *on;
	char arg[MSL]={'\0'};
	int fail = 0;

	if (ch->race != race_lookup("faerie") || ch->disc[DISC_ACTOR] >= 3
			|| ch->disc[DISC_REALM] >= 2)
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if(IS_NULLSTR(argument) )
	{
		send_to_char("Sneak a peek at whom?\n\r", ch);
		return;
	}

	one_argument(argument, arg);

	if((vch = get_char_world(ch, arg)) == NULL)
	{
		send_to_char("They aren't around.\n\r", ch);
		return;
	}

	if(ch->trust < vch->trust)
	{
		send_to_char( "You cannot view that location.\n\r", ch );
		return;
	}

	if(ch->race == race_lookup("faerie"))
		fail = dice_rolls(ch, get_curr_stat(ch, STAT_PER) + ch->ability[KENNING].value, (vch->willpower + 3 > 10) ? 10 : vch->willpower + 3);
	else fail = 0;

	ch->power_timer = 4;
	if(fail > 0)
	{
		if ( ( location = find_location( ch, arg ) ) == NULL )
		{
			send_to_char( "No such location.\n\r", ch );
			return;
		}

		if (!is_room_owner(ch,location) && room_is_private( location )
				&&  get_trust(ch) < MAX_LEVEL)
		{
			send_to_char( "You cannot view that location.\n\r", ch );
			return;
		}

		original = ch->in_room;
		on = ch->on;
		char_from_room( ch );
		char_to_room( ch, location );
		do_function(ch, &do_look, "");
		char_from_room( ch );
		char_to_room( ch, original );
		ch->on = on;
	}
	else
	{
		send_to_char("You fail to see anything.\n\r", ch);
	}
}


int do_transform (CHAR_DATA *ch, char *argument)
{
	char arg[MSL]={'\0'};
	int race = 0, success = FALSE;

	one_argument(argument, arg);
	race = race_lookup(arg);

	switch (arg[0])
	{
	case 'b':
		if(!str_prefix(arg, "bat") || !str_cmp(arg, "bat"))
		{
			if(ch->shape == SHAPE_BAT)
			{
				send_to_char("But that's the shape you are!\n\r", ch);
				return FALSE;
			}

			switch (ch->shape)
			{
			case SHAPE_WOLF:
			case SHAPE_BAT:
			case SHAPE_SNAKE:
				do_transform(ch, "human");
				break;
			case SHAPE_CRINOS:
				if(ch->race != race_lookup("werewolf"))
					do_transform(ch, "human");
				break;
			default : break;
			}

			send_to_char("Your body contorts and shrinks.\n\r", ch);
			act("$n transforms into a bat!", ch, NULL, NULL, TO_ROOM, 0);
			do_function(ch, &do_remove, "all");
			ch->shape = SHAPE_BAT;
			/* alter all the other race crap */
			ch->affected_by = ch->affected_by|race_table[race].aff;
			ch->imm_flags = ch->imm_flags|race_table[race].imm;
			ch->res_flags = ch->res_flags|race_table[race].res;
			ch->vuln_flags = ch->vuln_flags|race_table[race].vuln;
			ch->form = race_table[race].form;
			ch->parts = race_table[race].parts;
			ch->size = SIZE_TINY;
			success = TRUE;
			break;
		}
		break;
	case 's':
		if(!str_prefix(arg, "snake") || !str_cmp(arg, "snake"))
		{
			if(ch->shape == SHAPE_SNAKE)
			{
				send_to_char("But that's the shape you are!\n\r", ch);
				return FALSE;
			}

			switch (ch->shape)
			{
			case SHAPE_BAT:
			case SHAPE_WOLF:
				do_transform(ch, "human");
				break;
			case SHAPE_CRINOS:
				if(ch->race != race_lookup("werewolf"))
					do_transform(ch, "human");
				break;
			default : break;
			}

			send_to_char("Your body contorts and reshapes itself.\n\r", ch);
			act("$n transforms into a large snake!", ch, NULL, NULL, TO_ROOM, 0);
			do_function(ch, &do_remove, "all");
			ch->shape = SHAPE_SNAKE;
			/* alter all the other race crap */
			ch->affected_by = ch->affected_by|race_table[race].aff;
			ch->imm_flags = ch->imm_flags|race_table[race].imm;
			ch->res_flags = ch->res_flags|race_table[race].res;
			ch->vuln_flags = ch->vuln_flags|race_table[race].vuln;
			ch->form = race_table[race].form;
			ch->parts = race_table[race].parts;
			ch->size = SIZE_MEDIUM;
			success = TRUE;
			break;
		}
		break;
	case 'w':
		if(!str_prefix(arg, "wolf") || !str_cmp(arg, "wolf"))
		{
			if(ch->shape == SHAPE_WOLF)
			{
				send_to_char("But that's the shape you are!\n\r", ch);
				return FALSE;
			}

			switch (ch->shape)
			{
			case SHAPE_BAT:
			case SHAPE_SNAKE:
				do_transform(ch, "human");
				break;
			case SHAPE_CRINOS:
				if(ch->race != race_lookup("werewolf"))
					do_transform(ch, "human");
				break;
			default : break;
			}

			send_to_char("Your body contorts and reshapes itself.\n\r", ch);
			act("$n transforms into a wolf!", ch, NULL, NULL, TO_ROOM, 0);
			do_function(ch, &do_remove, "all");
			ch->shape = SHAPE_WOLF;
			/* alter all the other race crap */
			ch->affected_by = ch->affected_by|race_table[race].aff;
			ch->imm_flags = ch->imm_flags|race_table[race].imm;
			ch->res_flags = ch->res_flags|race_table[race].res;
			ch->vuln_flags = ch->vuln_flags|race_table[race].vuln;
			ch->form = race_table[race].form;
			ch->parts = race_table[race].parts;
			ch->size = SIZE_LARGE;
			success = TRUE;
			break;
		}
		break;
	case 'c':
		if(!str_prefix(arg, "crinos") || !str_cmp(arg, "crinos"))
		{
			if(ch->shape == SHAPE_CRINOS)
			{
				send_to_char("But that's the shape you are!\n\r", ch);
				return FALSE;
			}

			switch (ch->shape)
			{
			case SHAPE_WOLF:
			case SHAPE_BAT:
			case SHAPE_SNAKE:
				do_transform(ch, "human");
				break;
			default : break;
			}

			send_to_char("Your body contorts and reshapes itself.\n\r", ch);
			act("$n transforms into a snarling $t!", ch, clan_table[ch->clan].crinos_desc, NULL, TO_ROOM, 0);
			race_lookup("werewolf");
			ch->shape = SHAPE_CRINOS;
			/* alter all the other race crap */
			ch->affected_by = ch->affected_by|race_table[race].aff;
			ch->imm_flags = ch->imm_flags|race_table[race].imm;
			ch->res_flags = ch->res_flags|race_table[race].res;
			ch->vuln_flags = ch->vuln_flags|race_table[race].vuln;
			ch->form = race_table[race].form;
			ch->parts = race_table[race].parts;
			ch->size = SIZE_HUGE;
			success = TRUE;
			break;
		}
		break;
	case 'h':
		if(!str_prefix(arg, "human") || !str_cmp(arg, "human"))
		{
			if(ch->shape == SHAPE_HUMAN)
			{
				send_to_char("But that's the shape you are!\n\r", ch);
				return FALSE;
			}

			if(ch->shape == SHAPE_NONE)
			{
				ch->shape = SHAPE_HUMAN;
				send_to_char("But that's the shape you are!\n\r", ch);
				return FALSE;
			}

			send_to_char("Your body contorts as your features become human.\n\r", ch);
			act("$n changes shape, returning to human form!", ch, NULL, NULL, TO_ROOM, 0);
			ch->shape = SHAPE_HUMAN;
			/* alter all the other race crap */
			ch->affected_by = ch->affected_by|race_table[race].aff;
			ch->imm_flags = ch->imm_flags|race_table[race].imm;
			ch->res_flags = ch->res_flags|race_table[race].res;
			ch->vuln_flags = ch->vuln_flags|race_table[race].vuln;
			ch->form = race_table[race].form;
			ch->parts = race_table[race].parts;
			ch->size = SIZE_MEDIUM;
			success = TRUE;
			break;
		}
		break;
	default:
		send_to_char("No such form!\n\r", ch);
		break;
	}

	return success;
}

void do_ww_transform (CHAR_DATA *ch, char *argument)
{
	char arg[MSL]={'\0'};
	bool fRage = FALSE;

	argument = one_argument(argument, arg);

	if(!IS_NULLSTR(argument) && !str_prefix(argument, "rage"))
	{
		fRage = TRUE;
		ch->RBPG--;
	}

	if(!IS_NULLSTR(argument) && str_prefix(argument, "rage"))
	{
		send_to_char("Syntax: transform <shape> [rage]\n\r", ch);
		return;
	}

	if((!str_prefix(arg, "wolf") && ch->breed == breed_lookup("lupus"))
			|| (!str_prefix(arg, clan_table[ch->clan].crinos_desc)
					&& ch->breed == breed_lookup("lupus"))
					|| (!str_prefix(arg, "human") && ch->breed == breed_lookup("homid"))
					|| (!str_prefix(arg, "crinos") && ch->breed == breed_lookup("metis")))
		fRage = TRUE;


	if(ch->race == race_lookup("werewolf"))
	{
		if( str_prefix(arg, clan_table[ch->clan].crinos_desc)
				|| str_prefix(arg, "wolf") || str_prefix(arg, "human") )
		{
			if(ch->power_timer <= 0 || fRage)
			{
				if(do_transform(ch, arg)) {
					/* Removed as a balancing choice.
		if(!IS_AFFECTED(ch, AFF_HSENSES)
		  && !IS_SET(ch->affected_by, AFF_HSENSES)
		  && ch->shape > SHAPE_HUMAN)
		    do_function(ch, &do_auspex1, "all");
		else if((IS_AFFECTED(ch, AFF_HSENSES)
		  || IS_SET(ch->affected_by, AFF_HSENSES))
		  && ch->shape <= SHAPE_HUMAN)
		    do_function(ch, &do_auspex1, "all");
					 */
					if(!fRage)
						ch->power_timer = 4;
				}
			}
			else
			{
				send_to_char("You are still too weak to transform.\n\r", ch);
			}
		}
		else
			send_to_char("No such form!\n\r", ch);
	}
	else
	{
		send_to_char("Huh?\n\r", ch);
	}
}

void do_protean4 (CHAR_DATA *ch, char *argument)
{
	char arg[MSL]={'\0'};

	one_argument(argument, arg);

	if(ch->race == race_lookup("vampire") && ch->disc[DISC_PROTEAN] >= 4)
	{
		if(IS_NULLSTR(arg))
		{
			send_to_char("Change into what?\n\r", ch);
			return;
		}

		if (!has_enough_power(ch))
			return;

		if(ch->RBPG < 2)
		{
			send_to_char("You don't have the blood to spare.\n\r", ch);
			return;
		}

		if( str_prefix(arg, "bat") || str_prefix(arg, "wolf")
				|| str_prefix(arg, "human") ) {
			if(do_transform(ch, arg)) {
				ch->power_timer = 4;
				ch->RBPG--;
			}
		}
		else
			send_to_char("No such form!\n\r", ch);
	}
	else
	{
		send_to_char("Huh?\n\r", ch);
	}
}

void do_enchant (CHAR_DATA *ch, char *argument)
{
	AFFECT_DATA af;
	CHAR_DATA *victim;
	char arg1[MSL]={'\0'};
	char arg2[MSL]={'\0'};
	int num = 0;

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if(ch->race != race_lookup("faerie") || (ch->disc[DISC_ACTOR] == 1 && ch->disc[DISC_REALM] == 2 && ch->disc[DISC_ART] == 2))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if((victim = get_char_room(ch, arg1)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	if(IS_AFFECTED(victim, AFF_ENCHANTED) || victim->race == race_lookup("faerie"))
	{
		send_to_char(Format("The wonders of the imagination are already open to %s.", victim->name), ch);
		return;
	}

	if(!is_number(arg2))
	{
		send_to_char("You must specify the number of glamour points you wish to expend.\n\r", ch);
		return;
	}

	num = atoi(arg2);
	if(num > ch->RBPG)
	{
		send_to_char("You don't have that much glamour.\n\r", ch);
		return;
	}

	ch->power_timer = 2;
	if( dice_rolls(ch, get_curr_stat(ch, STAT_WIT) + ch->ability[MYTHLORE].value, victim->willpower) )
	{
		victim->condition[COND_FRENZY] = 0;
		victim->condition[COND_ANGER] = 0;
		ch->RBPG -= num;
		af.where = TO_AFFECTS;
		af.type = skill_lookup("calm");
		af.level = ch->max_RBPG;
		af.duration = num * 10;
		af.location = APPLY_NONE;
		af.modifier = 0;
		af.bitvector = AFF_ENCHANTED;
		affect_to_char(victim, &af);
		act_new("Twinkling flecks of light appear in $N's eyes.", ch, NULL, victim, TO_CHAR, P_STAND, 0);
		act_new("Your vision shimmers slightly.", ch, NULL, victim, TO_VICT, P_REST, 1);
		return;
	}
	else
	{
		send_to_char("There seems to be no effect.\n\r", ch);
		return;
	}

}

void do_charm (CHAR_DATA *ch, char* argument)
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH]={'\0'};

	one_argument(argument, arg);

	if(ch->race != race_lookup("faerie") || ch->disc[DISC_ACTOR] < 2 || ch->disc[DISC_ART] < 2)
	{
		send_to_char("Huh!\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if(IS_NULLSTR(arg))
	{
		send_to_char("Who do you wish to charm?\n\r", ch);
		return;
	}

	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	ch->power_timer = 2;
	spell_charm_person(skill_lookup("charm"), ch->disc[DISC_ART], ch, (void *)victim, 0);
}

void do_ghoul (CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	char arg[MSL]={'\0'};

	if(ch->race != race_lookup("vampire"))
	{
		send_to_char("Huh?\n\r", ch);
	}
	else
	{
		if(IS_NULLSTR(argument))
		{
			send_to_char("Who do you wish to ghoul?\n\r", ch);
			return;
		}

		one_argument(argument, arg);

		if((victim = get_char_room(ch, arg)) == NULL)
		{
			send_to_char("They aren't here.\n\r", ch);
			return;
		}

		if(IS_ADMIN(victim) || !IS_NATURAL(victim) || IS_NPC(victim))
		{
			send_to_char("They aren't within reach of your powers.\n\r", ch);
			return;
		}

		if(!IS_SET(victim->concede, CEED_GHOUL))
		{
			send_to_char("They must concede to your advances.\n\r", ch);
			return;
		}

		if(IS_SET(victim->act2, ACT2_GHOUL))
		{
			send_to_char("You can't ghoul a ghoul.\n\r", ch);
			return;
		}

		SET_BIT(victim->act2, ACT2_GHOUL);

		act_new("$n seizes upon you, feeding a small amount of $s blood to you.", ch, NULL, victim, TO_VICT, P_REST, 1);

		act_new("$n seizes upon $N, feeding a small amount of $s blood to you.", ch, NULL, victim, TO_NOTVICT, P_REST, 0);

		act_new("You seize upon $N, feeding a small amount of $s blood to you.", ch, NULL, victim, TO_CHAR, P_REST, 1);
	}
}

void do_embrace (CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	char arg[MSL]={'\0'};
	int i = 0;

	if(ch->race == race_lookup("human"))
	{
		send_to_char("Huh?\n\r", ch);
	}
	else
	{
		if(IS_NULLSTR(argument))
		{
			send_to_char("Whose transformation do you wish to assist?\n\r", ch);
			return;
		}

	one_argument(argument, arg);

	if((victim = get_char_room(ch, arg)) == NULL)
	{
	    send_to_char("They aren't here.\n\r", ch);
	    return;
	}

        if(!IS_NATURAL(victim) || IS_NPC(victim))
	{
	    send_to_char("They aren't within reach of your powers.\n\r", ch);
	    return;
	}

	if(!IS_SET(victim->concede, CEED_EMBRACE))
	{
	    send_to_char("They must concede to your advances.\n\r", ch);
	    return;
	}

	victim->race = ch->race;
	victim->clan = ch->clan;

	victim->affected_by = victim->affected_by|race_table[victim->race].aff;
	victim->imm_flags = victim->imm_flags|race_table[victim->race].imm;
	victim->res_flags = victim->res_flags|race_table[victim->race].res;
	victim->vuln_flags = victim->vuln_flags|race_table[victim->race].vuln;
	victim->form = race_table[victim->race].form;
	victim->parts = race_table[victim->race].parts;
	victim->size = pc_race_table[victim->race].size;

	for(i=0;i<MAX_DISC;i++)
	    victim->disc[i] = 0;
	init_clan_powers(victim, victim->clan);
	victim->health  = MAX_HEALTH;
	victim->willpower = ch->virtues[2];

	if(ch->race == race_lookup("vampire"))
	{
	    act_new(
		"You seize upon $N and drain $S blood, replacing it with a small amount of your own.",
		ch, NULL, victim, TO_CHAR, P_REST, 1);

	    act_new(
		"$n seizes upon you and drains your blood, replacing it with a small amount of $s own.",
		ch, NULL, victim, TO_VICT, P_REST, 1);

	    act_new(
		"$n seizes upon $N and drains $S blood, replacing it with a small amount of $s own.",
		ch, NULL, victim, TO_NOTVICT, P_REST, 0);
	    if (clan_lookup("nosferatu") == victim->clan)
	        victim->perm_stat[STAT_APP] = 0;
	    victim->GHB = victim->virtues[0] + victim->virtues[1];
	    victim->max_GHB = victim->GHB;
	    victim->gen = ch->gen + 1;
	    ch->max_RBPG = 23 - ch->gen;
	    if(ch->max_RBPG < 10) ch->max_RBPG = 10;
	}
	else if(ch->race == race_lookup("werewolf"))
	{
	    act_new(
		"You seize upon $N and awaken the hunter within.",
		ch, NULL, victim, TO_CHAR, P_REST, 1);

	    act_new(
		"$n seizes upon you and you feel your soul cry out for the wilds.",
		ch, NULL, victim, TO_VICT, P_REST, 1);

	    act_new(
		"$n seizes upon $N and stares intently into $S eyes.",
		ch, NULL, victim, TO_ROOM, P_REST, 0);
	    victim->GHB = victim->virtues[0] + victim->virtues[1];
	    victim->max_GHB = victim->GHB;
	    victim->RBPG = victim->ability[SURVIVAL].value + victim->ability[PRIMAL_URGE].value;
	    victim->max_RBPG = victim->RBPG;
	}
	else if(ch->race == race_lookup("faerie"))
	{
	    act_new(
		"You seize upon $N and lead $S faerie spirit from its banal shackles.",
		ch, NULL, victim, TO_CHAR, P_REST, 1);

	    act_new(
		"$n seizes upon you and your eyes open wide to the wonders around you.",
		ch, NULL, victim, TO_VICT, P_REST, 1);

	    act_new(
		"$n seizes upon $N and $S seems to shimmer slightly for a moment.",
		ch, NULL, victim, TO_ROOM, P_REST, 0);
	    victim->GHB = 10 - (victim->virtues[0] + victim->virtues[1]);
	    victim->max_GHB = victim->GHB;
	    victim->RBPG = victim->ability[KENNING].value + victim->ability[EMPATHY].value;
	    victim->max_RBPG = victim->RBPG;
	}

    }
}

void do_ffog (CHAR_DATA *ch, char *string)
{
	CHAR_DATA *pch;

    if(ch->race != race_lookup("faerie")
	|| (ch->race == race_lookup("faerie")
	&& (ch->disc[DISC_ACTOR] < 2 || ch->disc[DISC_REALM] < 1 || ch->disc[DISC_ART] < 1)) )
    {
	send_to_char("Huh\n\r", ch);
	return;
    }

	if (!has_enough_power(ch))
		return;

    act("$n conjures a cloud of purple smoke.", ch, NULL, NULL, TO_ROOM, 0);
    send_to_char("You conjure a cloud of purple smoke.\n\r", ch);

    for(pch = ch->in_room->people; pch != NULL; pch = pch->next_in_room)
    {
	if(pch->invis_level > 0)
	    continue;

	if(pch == ch || saves_spell( ch->disc[DISC_REALM], pch, DAM_OTHER))
	    continue;

	affect_strip(pch, gsn_invis);
	affect_strip(pch, gsn_sneak);
	REMOVE_BIT(pch->affected_by, AFF_HIDE);
	REMOVE_BIT(pch->affected_by, AFF_INVISIBLE);
	REMOVE_BIT(pch->affected_by, AFF_SNEAK);
	act( "$n appears from the purple cloud coughing and spluttering.", pch, NULL, NULL, TO_ROOM, 0 );
	send_to_char( "You start coughing, giving away your presence.\n\r", pch);
    }

    ch->power_timer = 3;

    return;
}

void heat_metal(CHAR_DATA *ch, int fail)
{
	OBJ_DATA *obj;
	AFFECT_DATA af;

	af.where     = TO_AFFECTS;
	af.type      = skill_lookup("heat metal");
	af.level     = fail;
	af.duration  = fail;
	af.location  = APPLY_NONE;
	af.modifier  = -1;
	af.bitvector = AFF_NONE;

	for(obj = ch->carrying; obj; obj = obj->next_content)
	{
		if(material_table[material_lookup(obj->material)].is_metal)
		{
			act("$n's $p looks like it's heating up.",ch,obj,NULL,TO_ROOM, 0);
			act("Your $p begins to heat up.",ch,obj,NULL,TO_CHAR, 1);
			affect_to_obj( obj, &af );
		}
	}
}

/* do_wwgift3_3: Heat metal - Formerly cockroach3 */
void do_wwgift3_3(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	char arg[MSL]={'\0'};
	int fail = 0;

	/* Affect objects as AFF_HEAT_METAL
   Alert character to each item heating
   Add to aff updater pain/damage/resistance references
	 */
	if (ch->race != race_lookup("werewolf") || !IS_SET(ch->powers[2], C))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	one_argument(argument, arg);

	if((victim = get_char_room(ch, arg)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	fail = dice_rolls(ch, get_curr_stat(ch, STAT_INT) + ch->ability[OCCULT].value, victim->willpower);
	ch->power_timer = 2;
	if(fail > 1)
	{
		heat_metal(victim, fail);
		return;
	}
	else if(fail >= 0)
	{
		send_to_char("The metals remain cool.\n\r", ch);
		return;
	}
	else
	{
		heat_metal(ch, fail * -1);
		return;
	}
}

/* do_wwgift2_3: Siphon - Formerly cockroach2 */
void do_wwgift2_3(CHAR_DATA *ch, char *string)
{
	CHAR_DATA *victim;
	char arg[MSL]={'\0'};
	int siphon = 0;
	int diff = 0;
	int fail = 0;
	int d = 0, c = 0;

	if(ch->race != race_lookup("werewolf") || !IS_SET(ch->powers[0], C))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	one_argument(string, arg);

	if((victim = get_char_world(ch, arg)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	siphon = get_curr_stat(ch, STAT_INT) + ch->ability[COMPUTER].value;
	diff = get_curr_stat(victim, STAT_INT) + ch->ability[FINANCE].value;

	ch->power_timer = 1;

	if((fail = dice_rolls(ch, siphon, diff)) > 0)
	{
		if(!IS_SET(victim->act2, ACT2_NOWARRANT))
			add_warrant(ch, 3, FALSE);
		if (victim->dollars == 0 && victim->cents == 0)
		{
			fail = 0;
		}
		else
		{
			d = UMAX(victim->bank_account/100, number_fuzzy(fail));
			c = UMAX(victim->bank_account%100, number_fuzzy(fail));
			ch->dollars += d; ch->cents += c;
			victim->bank_account -= d*100; victim->bank_account -= c;
		}
		send_to_char(Format("You siphon off $%d . %c from %s.\n\r", d, c, victim->name), ch);
	}

	if(fail == 0)
	{
		if(!IS_SET(victim->act2, ACT2_NOWARRANT))
			add_warrant(ch, 3, TRUE);
		send_to_char("You can't seem to siphon any funds.\n\r", ch);
		return;
	}
	else if(fail < 0)
	{
		if(!IS_SET(victim->act2, ACT2_NOWARRANT))
			add_warrant(ch, 3, TRUE);
		send_to_char("You can't seem to siphon any funds.\n\r", ch);
		act("$n tried to steal funds from you electronically.", ch, NULL, victim, TO_VICT, 1);
		return;
	}

}

/* do_wwgift5_3: Phone Travel (ptravel) - Formerly cockroach5 */
void do_wwgift5_3(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim = NULL;
	char arg[MAX_INPUT_LENGTH]={'\0'};

	if((ch->clan != clan_lookup("glass walker"))
			|| (ch->clan == clan_lookup("glass walker") && ch->disc[DISC_TRIBE] < 5))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if ( !HAS_PHONE(ch) )
	{
		send_to_char( "You can't make a call without a phone in your hand!\n\r", ch );
		return;
	}

	if ( IS_SET(ch->comm, COMM_NOTELL) || IS_SET(ch->comm,COMM_DEAF) )
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
		if( (ch != ch->reply) && (ch->reply != NULL) )
			ch->reply->on_phone = FALSE;
		ch->on_phone = FALSE;
		send_to_char("You hang up.\n\r",ch);
		send_to_char("The phone goes dead.\n\r", ch->reply);
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

	if ( (ch->on_phone) && ((ch->reply != victim) && (ch->reply != NULL)) )
	{
		send_to_char("You're already in a call!\n\r",ch);
		return;
	}

	if(OWNS_PHONE(victim))
	{
		send_to_char("The phone rings, but no-one picks it up.\n\r", ch);
		act("$n's phone rings.", ch, NULL, NULL, TO_ROOM, 0);
		if(IS_AFFECTED(ch, AFF_INVISIBLE)
				|| IS_AFFECTED(ch, AFF_HIDE)
				|| IS_AFFECTED(ch, AFF_SNEAK))
			do_function(ch, &do_visible, "");
		return;
	}

	if ( !HAS_PHONE(victim) )
	{
		send_to_char( "That number is not connected.\n\rPlease check the number, and try again.\n\r", ch );
		return;
	}

	if ( victim->desc == NULL && !IS_NPC(victim))
	{
		act("$N seems to be out of order...try again later.", ch,NULL,victim,TO_CHAR,1);
		add_buf(victim->pcdata->buffer, (char *)Format("%s's voice crackles '%s'\n\r",PERS(ch,victim),CapitalSentence(argument)) );
		return;
	}

	if ( !(IS_ADMIN(ch) && ch->trust < LEVEL_IMMORTAL) && !IS_AWAKE(victim) )
	{
		act( "$E seems not to be answering the phone.", ch,0,victim,TO_CHAR,1);
		return;
	}

	if (((IS_SET(victim->comm,COMM_QUIET) || IS_SET(victim->comm,COMM_DEAF))
			&& !IS_ADMIN(ch)) || IS_SET(victim->comm,COMM_OLC))
	{
		act( "$E must have switched off their phone.", ch, 0, victim,TO_CHAR,1);
		return;
	}

	if ( !victim->on_phone )
	{
		act("$n's phone rings.", ch, NULL, NULL, TO_ROOM, 0);
		act_new("You pick up a call from $n",ch,NULL,victim,TO_VICT,P_DEAD,1);
		if(IS_AFFECTED(victim, AFF_INVISIBLE)
				|| IS_AFFECTED(victim, AFF_HIDE)
				|| IS_AFFECTED(victim, AFF_SNEAK))
			do_function(victim, &do_visible, "");
	}
	else
	{
		if(victim->reply != ch)
		{
			send_to_char( "I'm sorry, your party is on another call.\n\r", ch );
			send_to_char( "A small blip sounds.\n\r", victim );
			return;
		}
	}

	act( "You leap from $N's phone.", ch, argument, victim, TO_CHAR, 0 );
	act_new("$n springs out of $N's phone.",ch,NULL,victim,TO_NOTVICT,P_REST,0);
	act_new("$n springs out of your phone.",ch,NULL,victim,TO_VICT,P_REST,0);
	char_from_room(ch);
	char_to_room(ch, victim->in_room);
	ch->power_timer = 4;

	return;
}


void acid_effect(void *vo, int level, int dam, int target)
{
    if (target == TARGET_ROOM) /* nail objects on the floor */
     {
	ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
	OBJ_DATA *obj, *obj_next;

	for (obj = room->contents; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;
	    acid_effect(obj,level,dam,TARGET_OBJ);
	}
	return;
    }

    if (target == TARGET_CHAR)  /* do the effect on a victim */
    {
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	OBJ_DATA *obj, *obj_next;

	/* let's toast some gear */
	for (obj = victim->carrying; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;
	    acid_effect(obj,level,dam,TARGET_OBJ);
	}
	return;
    }

    if (target == TARGET_OBJ) /* toast an object */
    {
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	OBJ_DATA *t_obj,*n_obj;
	int chance, mat;
	char *msg;

	if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF)
	||  IS_OBJ_STAT(obj,ITEM_NOPURGE)
	||  number_range(0,4) == 0)
	    return;

	mat = material_lookup(obj->material);

	chance = level + dam;

	if (IS_OBJ_STAT(obj,ITEM_BLESS))
	    chance -= 4;

	if(material_table[mat].is_flammable) chance += 10;
	if(material_table[mat].is_fabric) chance += 20;
	if(material_table[mat].is_metal) chance += 15;
	if(material_table[mat].is_explosive) chance += 10;
	if(material_table[mat].is_edible) chance += 10;

	chance *= 2;

	if(chance < obj->condition)
	{
	    obj->quality *= chance/obj->condition;
	    obj->condition -= chance;
	}
	else
	{
	    obj->quality = 0;
	    obj->condition = 0;
	}

	switch (obj->item_type)
	{
	    default:
		msg = "$p fumes and dissolves.";
		break;
	    case ITEM_CONTAINER:
	    case ITEM_CORPSE_PC:
	    case ITEM_CORPSE_NPC:
		msg = "$p fumes and dissolves.";
		break;
	    case ITEM_ARMOR:
		msg = "$p is pitted and etched.";
		break;
	    case ITEM_CLOTHING:
		msg = "$p is corroded.";
	 	break;
	    case ITEM_SCROLL:
		chance += 10;
		msg = "$p is burned into waste.";
		break;
	}

	if (obj->carried_by != NULL)
	    act(msg,obj->carried_by,obj,NULL,TO_ALL,0);
	else if (obj->in_room != NULL && obj->in_room->people != NULL)
	    act(msg,obj->in_room->people,obj,NULL,TO_ALL,0);

	if (obj->item_type == ITEM_ARMOR)  /* etch it */
	{
	    AFFECT_DATA *paf;
	    /*bool af_found = FALSE;*/
	    int i;

 	    affect_enchant(obj);

	    for ( paf = obj->affected; paf != NULL; paf = paf->next)
            {
            }

            if (obj->carried_by != NULL && obj->wear_loc != WEAR_NONE)
                for (i = 0; i < 4; i++)
                    obj->carried_by->armor[i] += 1;
            return;
	}

	if(obj->condition > 0) return;

	/* get rid of the object */
	if (obj->contains)  /* dump contents */
	{
	    for (t_obj = obj->contains; t_obj != NULL; t_obj = n_obj)
	    {
		n_obj = t_obj->next_content;
		obj_from_obj(t_obj);
		if (obj->in_room != NULL)
		    obj_to_room(t_obj,obj->in_room);
		else if (obj->carried_by != NULL)
		    obj_to_room(t_obj,obj->carried_by->in_room);
		else
		{
		    extract_obj(t_obj);
		    continue;
		}

		acid_effect(t_obj,level/2,dam/2,TARGET_OBJ);
	    }
 	}

	extract_obj(obj);
	return;
    }
}


void cold_effect(void *vo, int level, int dam, int target)
{
    if (target == TARGET_ROOM) /* nail objects on the floor */
    {
        ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
        OBJ_DATA *obj, *obj_next;

        for (obj = room->contents; obj != NULL; obj = obj_next)
        {
            obj_next = obj->next_content;
            cold_effect(obj,level,dam,TARGET_OBJ);
        }
        return;
    }

    if (target == TARGET_CHAR) /* whack a character */
    {
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	OBJ_DATA *obj, *obj_next;

	/* chill touch effect */
	if (!saves_spell(level/4 + dam / 20, victim, DAM_COLD))
	{
	    AFFECT_DATA af;

            act("$n turns blue and shivers.",victim,NULL,NULL,TO_ROOM,0);
	    act("A chill sinks deep into your bones.", victim,NULL,NULL,TO_CHAR,1);
            af.where     = TO_AFFECTS;
            af.type      = skill_lookup("chill touch");
            af.level     = level;
            af.duration  = 6;
            af.location  = APPLY_STR;
            af.modifier  = -1;
            af.bitvector = AFF_NONE;
            affect_join( victim, &af );
	}

	/* hunger! (warmth sucked out */
	if (!IS_NPC(victim))
	    gain_condition(victim,COND_HUNGER,dam/20);

	/* let's toast some gear */
	for (obj = victim->carrying; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;
	    cold_effect(obj,level,dam,TARGET_OBJ);
	}
	return;
   }

   if (target == TARGET_OBJ) /* toast an object */
   {
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	int chance, mat;
	char *msg;

	if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF)
	||  IS_OBJ_STAT(obj,ITEM_NOPURGE)
	||  number_range(0,4) == 0)
	    return;

	mat = material_lookup(obj->material);

	chance = level + dam;

	if (IS_OBJ_STAT(obj,ITEM_BLESS))
	    chance -= 4;

	if(material_table[mat].is_fabric) chance += 5;
	if(material_table[mat].is_explosive) chance += 5;
	if(material_table[mat].is_edible) chance += 10;
	if(material_table[mat].is_liquid) chance += 20;
	if(material_table[mat].is_metal) chance -= 40;

	chance *= 2;

	if(chance < obj->condition)
	{
	    obj->quality *= chance/obj->condition;
	    obj->condition -= chance;
	}
	else
	{
	    obj->quality = 0;
	    obj->condition = 0;
	}

	switch(obj->item_type)
	{
	    default:
		return;
	    case ITEM_POTION:
		msg = "$p freezes and shatters!";
		chance += 25;
		break;
	    case ITEM_DRINK_CON:
		msg = "$p freezes and shatters!";
		chance += 5;
		break;
	}

	if (obj->carried_by != NULL)
	    act(msg,obj->carried_by,obj,NULL,TO_ALL,0);
	else if (obj->in_room != NULL && obj->in_room->people != NULL)
	    act(msg,obj->in_room->people,obj,NULL,TO_ALL,0);

	if(obj->condition > 0) return;

	extract_obj(obj);
	return;
    }
}


void fire_effect(void *vo, int level, int dam, int target)
{
    if (target == TARGET_ROOM)  /* nail objects on the floor */
    {
	ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
	OBJ_DATA *obj, *obj_next;

	for (obj = room->contents; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;
	    fire_effect(obj,level,dam,TARGET_OBJ);
	}
	return;
    }

    if (target == TARGET_CHAR)   /* do the effect on a victim */
    {
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	OBJ_DATA *obj, *obj_next;

	/* chance of blindness */
	if (!IS_AFFECTED(victim,AFF_BLIND)
	&&  !saves_spell(level / 4 + dam / 20, victim,DAM_FIRE))
	{
            AFFECT_DATA af;
            act("$n is blinded by smoke!",victim,NULL,NULL,TO_ROOM,0);
            act("Your eyes tear up from smoke...you can't see a thing!", victim,NULL,NULL,TO_CHAR,1);

            af.where        = TO_AFFECTS;
            af.type         = skill_lookup("fire breath");
            af.level        = level;
            af.duration     = number_range(0,level/10);
            af.location     = APPLY_DEX;
            af.modifier     = -4;
            af.bitvector    = AFF_BLIND;

            affect_to_char(victim,&af);
	}

	/* getting thirsty */
	if (!IS_NPC(victim))
	    gain_condition(victim,COND_THIRST,dam/20);

	/* let's toast some gear! */
	for (obj = victim->carrying; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;

	    fire_effect(obj,level,dam,TARGET_OBJ);
        }
	return;
    }

    if (target == TARGET_OBJ)  /* toast an object */
    {
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	OBJ_DATA *t_obj,*n_obj;
	int chance, mat;
	char *msg;

    	if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF)
        ||  IS_OBJ_STAT(obj,ITEM_NOPURGE)
	||  number_range(0,4) == 0
	||  !material_table[material_lookup(obj->material)].is_flammable)
            return;

	mat = material_lookup(obj->material);

	chance = level + dam;

	if (IS_OBJ_STAT(obj,ITEM_BLESS))
	    chance -= 4;

	if(material_table[mat].is_fabric) chance += 10;
	if(material_table[mat].is_flammable) chance += 20;
	if(material_table[mat].is_edible) chance += 10;
	if(material_table[mat].is_liquid) chance += 20;
	if(material_table[mat].is_metal) chance -= 40;

	chance *= 2;

	if(chance < obj->condition)
	{
	    obj->quality *= chance/obj->condition;
	    obj->condition -= chance;
	}
	else
	{
	    obj->quality = 0;
	    obj->condition = 0;
	}

        switch ( obj->item_type )
        {
        default:
            msg = "$p ignites and burns!";
            break;
        case ITEM_CONTAINER:
            msg = "$p ignites and burns!";
            break;
        case ITEM_POTION:
            chance += 25;
            msg = "$p bubbles and boils!";
            break;
        case ITEM_SCROLL:
            chance += 50;
            msg = "$p crackles and burns!";
            break;
        case ITEM_FOOD:
            msg = "$p blackens and crisps!";
            break;
        case ITEM_PILL:
            msg = "$p melts and drips!";
            break;
        }

	if (obj->carried_by != NULL)
            act( msg, obj->carried_by, obj, NULL, TO_ALL, 0 );
	else if (obj->in_room != NULL && obj->in_room->people != NULL)
	    act(msg,obj->in_room->people,obj,NULL,TO_ALL,0);

	if(obj->condition > 0
	&& !material_table[mat].is_flammable
	&& !material_table[mat].is_explosive) return;

        if (obj->contains)
        {
            /* dump the contents */

            for (t_obj = obj->contains; t_obj != NULL; t_obj = n_obj)
            {
                n_obj = t_obj->next_content;
                obj_from_obj(t_obj);
		if (obj->in_room != NULL)
                    obj_to_room(t_obj,obj->in_room);
		else if (obj->carried_by != NULL)
		    obj_to_room(t_obj,obj->carried_by->in_room);
		else
		{
		    extract_obj(t_obj);
		    continue;
		}
		fire_effect(t_obj,level/2,dam/2,TARGET_OBJ);
            }
        }

	if(material_table[mat].is_flammable
	&& !IS_SET(obj->extra2, OBJ_BURNING))
	    SET_BIT(obj->extra2, OBJ_BURNING);

	if(material_table[mat].is_explosive)
	{
	    obj->item_type = ITEM_BOMB;
	    obj->value[1] = 0;
	}
	else
	{
        extract_obj( obj );
	}
	return;
    }
}


void poison_effect(void *vo,int level, int dam, int target)
{
    if (target == TARGET_ROOM)  /* nail objects on the floor */
    {
        ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
        OBJ_DATA *obj, *obj_next;

        for (obj = room->contents; obj != NULL; obj = obj_next)
        {
            obj_next = obj->next_content;
            poison_effect(obj,level,dam,TARGET_OBJ);
        }
        return;
    }

    if (target == TARGET_CHAR)   /* do the effect on a victim */
    {
        CHAR_DATA *victim = (CHAR_DATA *) vo;
        OBJ_DATA *obj, *obj_next;

	/* chance of poisoning */
        if (!saves_spell(level / 4 + dam / 20,victim,DAM_POISON))
        {
	    AFFECT_DATA af;

            send_to_char("You feel poison coursing through your veins.\n\r",
                victim);
            act("$n looks very ill.",victim,NULL,NULL,TO_ROOM,0);

            af.where     = TO_AFFECTS;
            af.type      = gsn_poison;
            af.level     = level;
            af.duration  = level / 2;
            af.location  = APPLY_STR;
            af.modifier  = -1;
            af.bitvector = AFF_POISON;
            affect_join( victim, &af );
        }

	/* equipment */
	for (obj = victim->carrying; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;
	    poison_effect(obj,level,dam,TARGET_OBJ);
	}
	return;
    }

    if (target == TARGET_OBJ)  /* do some poisoning */
    {
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	int chance;


	if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF)
  	||  IS_OBJ_STAT(obj,ITEM_BLESS)
	||  number_range(0,4) == 0)
	    return;

	chance = level / 4 + dam / 10;
	if (chance > 25)
	    chance = (chance - 25) / 2 + 25;
	if (chance > 50)
	    chance = (chance - 50) / 2 + 50;

	switch (obj->item_type)
	{
	    default:
		return;
	    case ITEM_FOOD:
		break;
	    case ITEM_DRINK_CON:
		if (obj->value[0] == obj->value[1])
		    return;
		break;
	}

	chance = URANGE(5,chance,95);

	if (number_percent() > chance)
	    return;

	obj->value[3] = 1;
	return;
    }
}


void shock_effect(void *vo,int level, int dam, int target)
{
    if (target == TARGET_ROOM)
    {
	ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
	OBJ_DATA *obj, *obj_next;

	for (obj = room->contents; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;
	    shock_effect(obj,level,dam,TARGET_OBJ);
	}
	return;
    }

    if (target == TARGET_CHAR)
    {
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	OBJ_DATA *obj, *obj_next;

	/* daze and confused? */
	if (!saves_spell(level/4 + dam/20,victim,DAM_LIGHTNING))
	{
	    send_to_char("Your muscles stop responding.\n\r",victim);
	    DAZE_STATE(victim,UMAX(12,level/4 + dam/20));
	}

	/* toast some gear */
	for (obj = victim->carrying; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;
	    shock_effect(obj,level,dam,TARGET_OBJ);
	}
	return;
    }

    if (target == TARGET_OBJ)
    {
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	int chance, mat;
	char *msg;

	if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF)
	||  IS_OBJ_STAT(obj,ITEM_NOPURGE)
	||  number_range(0,4) == 0)
	    return;

	mat = material_lookup(obj->material);

	chance = level + dam;

	if (IS_OBJ_STAT(obj,ITEM_BLESS))
	    chance -= 4;

	if(material_table[mat].is_fabric) chance += 1;
	if(material_table[mat].is_flammable) chance += 2;
	if(material_table[mat].is_edible) chance += 1;
	if(material_table[mat].is_liquid) chance += 2;
	if(material_table[mat].is_metal) chance -= 4;

	chance *= 2;

	if(chance < obj->condition)
	{
	    obj->quality *= chance/obj->condition;
	    obj->condition -= chance;
	}
	else
	{
	    obj->quality = 0;
	    obj->condition = 0;
	}


	switch(obj->item_type)
	{
	    default:
		msg = "$p scorches!";
		break;
	}

	if (obj->carried_by != NULL)
	    act(msg,obj->carried_by,obj,NULL,TO_ALL,0);
	else if (obj->in_room != NULL && obj->in_room->people != NULL)
	    act(msg,obj->in_room->people,obj,NULL,TO_ALL,0);

	if(obj->condition > 0
	&& !material_table[mat].is_flammable
	&& !material_table[mat].is_explosive) return;

	if(material_table[mat].is_flammable
	&& !IS_SET(obj->extra2, OBJ_BURNING)
	&& chance > 40)
	    SET_BIT(obj->extra2, OBJ_BURNING);

	if(material_table[mat].is_explosive)
	{
	    obj->item_type = ITEM_BOMB;
	    obj->value[1] = 0;
	}
	else
	{
        extract_obj( obj );
	}
	return;
    }
}

/* do_wwgift2_11: Scare - Formerly do_scare @@@@@
void do_wwgift2_11(CHAR_DATA *ch, char *string) */
void do_scare(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	char arg1[MSL]={'\0'};
	int fail = 0;

	argument = one_argument(argument, arg1);

	if((ch->race!=race_lookup("faerie") && ch->race!=race_lookup("werewolf"))
			|| (ch->race == race_lookup("faerie")
					&& (ch->disc[DISC_ACTOR] < 3 || ch->disc[DISC_ART] < 2))
					|| (ch->race == race_lookup("werewolf")
							&& !IS_SET(ch->powers[1], K)))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if((victim = get_char_room(ch, arg1)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	if(ch->race == race_lookup("faerie")) {
		fail = dice_rolls(ch, get_curr_stat(ch, STAT_MAN)
				+ ch->ability[INTIMIDATION].value, victim->willpower);
	} else if(ch->race == race_lookup("werewolf")) {
		fail = dice_rolls(ch, get_curr_stat(ch, STAT_CHA)
				+ ch->ability[INTIMIDATION].value,
				victim->race==race_lookup("werewolf")?5+victim->backgrounds[RACE_STATUS]:victim->willpower);
	}

	ch->power_timer = 2;
	if(fail > 0)
	{
		gain_condition(victim, COND_FEAR, fail * 15);
		if(ch->race == race_lookup("faerie")) {
			send_to_char("You play the strings of the fears.\n\r", ch);
			send_to_char("You get a very creepy feeling that your worst fears are to be realised.\n\r", victim);
		} else if(ch->race == race_lookup("werewolf")) {
			act("You stare down $N.", ch, NULL, victim, TO_CHAR,1);
			act("The menace in $n's eyes frightens you.", ch,NULL,victim,TO_VICT,1);
		}
		act_new("$N shivers and turns pale.",
				ch,NULL,victim,TO_NOTVICT,P_REST,0);
		if(victim->condition[COND_FEAR] > 70)
			do_function(victim, &do_flee, "");
	}
	else
	{
		send_to_char("There seems to be no effect.\n\r", ch);
	}
}


/* do_wwgift1_1: Cooking - Previously rat1 */
void do_wwgift1_1 (CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *obj1, *obj2;
	char arg1[MAX_INPUT_LENGTH]={'\0'};
	char arg2[MAX_INPUT_LENGTH]={'\0'};

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if(ch->race != race_lookup("werewolf") || !IS_SET(ch->powers[0], A))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if (IS_NULLSTR(arg1) || IS_NULLSTR(arg2))
	{
		send_to_char("Cook what with what?\n\r", ch);
		return;
	}

	obj1 = get_obj_list(ch, arg1, ch->carrying);
	obj2 = get_obj_list(ch, arg2, ch->carrying);

	if ( obj1 == NULL || obj2 == NULL || obj1 == obj2 )
	{
		send_to_char("You don't have that!\n\r", ch);
		return;
	}

	ch->power_timer = 1;
	if (ch->disc[DISC_TRIBE] && ch->race == race_lookup("werewolf"))
	{
		spell_create_food( -1, (ch->disc[DISC_TRIBE] * 5), ch, NULL, 1);
		extract_obj(obj1);
		extract_obj(obj2);
	}
}

/* do_wwgift2_1: Gift of the Skunk - Formerly rat2 */
void do_wwgift2_1(CHAR_DATA *ch, char *argument)
{
	if(ch->race != race_lookup("werewolf") || !IS_SET(ch->powers[1], A))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if(IS_AFFECTED(ch, AFF_ODOUR))
	{
		send_to_char("You stop smelling so badly.\n\r", ch);
		REMOVE_BIT(ch->affected_by, AFF_ODOUR);
		return;
	}

	if (!has_enough_power(ch))
		return;

	send_to_char("The skunk spirit lends you some of his odour.\n\r", ch);
	SET_BIT(ch->affected_by, AFF_ODOUR);
	ch->power_timer = 2;
}

/* do_wwgift3_1: Gift of the Termite - Formerly rat3 */
void do_wwgift3_1(CHAR_DATA *ch, char *argument)
{
	/* Get object, destroy object, full + hunger stuff */
	OBJ_DATA *Obj;

	if(ch->race != race_lookup("werewolf") || !IS_SET(ch->powers[2], A))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if(IS_NULLSTR(argument))
	{
		send_to_char("Eat which object?\n\r", ch);
		return;
	}

	if((Obj = get_obj_list(ch, argument, ch->carrying)) == NULL)
	{
		send_to_char("You don't have that object.\n\r", ch);
		return;
	}

	act( "$n eats $p.",  ch, Obj, NULL, TO_ROOM, 0 );
	act( "You eat $p.", ch, Obj, NULL, TO_CHAR, 1 );

	gain_condition(ch, COND_HUNGER, Obj->condition);
	gain_condition(ch, COND_FULL, Obj->condition);
	extract_obj(Obj);
	ch->power_timer = 1;
}

/* do_wwgift4_1: Attune - Formerly rat4 */
void do_wwgift4_1(CHAR_DATA *ch, char *argument)
{
	DESCRIPTOR_DATA *d;
	BUFFER *buffer;
	CHAR_DATA *victim;
	int count = 0;

	if(ch->race != race_lookup("werewolf") || !IS_SET(ch->powers[3], A))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	/* show characters logged */

	buffer = new_buf();
	for (d = descriptor_list; d != NULL; d = d->next)
	{
		if (d->character != NULL && d->connected == CON_PLAYING
				&&  d->character->in_room != NULL && can_see(ch,d->character)
		&&  can_see_room(ch,d->character->in_room)
		&&  d->character->in_room->area == ch->in_room->area)
		{
			victim = d->character;
			count++;
			if (d->original != NULL)
			{
				if(IS_ADMIN(ch))
				{
					add_buf(buffer, (char *)Format("%3d) %s (in the body of %s) is in %s\n\r", count, d->original->name,victim->short_descr, victim->in_room->name));
				}
			}
			else
			{
				add_buf(buffer, (char *)Format("%3d) %s is in %s\n\r", count, victim->name,victim->in_room->name));
			}
		}
	}

	page_to_char(buf_string(buffer),ch);
	free_buf(buffer);
	ch->power_timer = 3;
	return;
}

/* do_wwgift1_2: Sense Wyrm - Formerly falcon1 */
void do_wwgift1_2(CHAR_DATA *ch, char *argument)
{
	/*OBJ_DATA *pObj;*/
	CHAR_DATA *vch;
	int fail = 0;
	bool found = FALSE;

	if(ch->race != race_lookup("werewolf") || !IS_SET(ch->powers[0], B))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	fail = dice_rolls(ch, get_curr_stat(ch, STAT_PER) + ch->virtues[0], 8);

	if(fail <= 0)
	{
		send_to_char("You cannot sense the wyrm's presence.\n\r", ch);
		return;
	}
	else
	{
		for(vch = ch->in_room->people; vch; vch = vch->next_in_room)
		{
			if(can_see(ch, vch))
			{
				if(IS_SET(vch->act2, ACT2_WYRM))
				{
					act_new("$N appears to have the taint of the wyrm.",
							ch,NULL,vch,TO_CHAR,P_DEAD,1);
					found = TRUE;
				}
			}
		}

		if(IS_SET(ch->in_room->room_flags, ROOM_CORRUPTED))
		{
			found = TRUE;
			send_to_char("The corruption seethes in this area.\n\r", ch);
		}
		else if(IS_SET(ch->in_room->room_flags, ROOM_POLLUTED))
		{
			found = TRUE;
			send_to_char("Pollution is evident in this area.\n\r", ch);
		}

		if(!found)
		{
			send_to_char("You cannot sense the wyrm's presence.\n\r", ch);
		}

		ch->power_timer = 1;
		return;
	}
}

/* do_wwgift2_2: Moon Shield - Formerly falcon2 */
void do_wwgift2_2(CHAR_DATA *ch, char *argument)
{
	int boost = 0;

	if(ch->race != race_lookup("werewolf") || !IS_SET(ch->powers[1], B))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if(IS_NULLSTR(argument) || !is_number(argument))
	{
		send_to_char("How much armor do you wish to draw upon?\n\r", ch);
		return;
	}

	if((boost = atoi(argument)) > ch->GHB)
	{
		send_to_char("You don't have that much gnosis to spend.\n\r", ch);
		return;
	}

	ch->GHB -= boost;
	ch->health += boost;
	ch->agghealth += boost;
	ch->power_timer = boost;
	send_to_char("You feel the cool light of Luna's armor tingling in your flesh.\n\r", ch);
}

/* do_wwgift3_2: Silver Claws - Formerly falcon3
void do_wwgift2_3(CHAR_DATA *ch, char *argument) */
void do_falcon3(CHAR_DATA *ch, char *argument)
{
	if(ch->race != race_lookup("werewolf") || (!IS_SET(ch->powers[2], B) && !IS_SET(ch->powers[3], K)))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if(IS_SET(ch->affected_by, AFF_SCLAWS))
	{
		send_to_char("Your claws lose their silvery gleam.\n\r", ch);
		REMOVE_BIT(ch->affected_by, AFF_SCLAWS);
		return;
	}

	if(ch->shape > SHAPE_HUMAN)
	{
		/*SET_BIT(ch->affected_by, AFF_SCLAWS); */
		return;
	}
}

/* do_wwgift4_2: Wrath of Gaia - Formerly falcon4 */
void do_wwgift4_2(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *vch;
	int fail = 0;

	if(ch->race != race_lookup("werewolf") || !IS_SET(ch->powers[3], B))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	send_to_char("You call down the wrath of Gaia.\n\r", ch);
	ch->power_timer = 2;

	for(vch = ch->in_room->people; vch; vch = vch->next_in_room)
	{
		if(IS_SET(vch->act2, ACT2_WYRM))
		{
			fail = dice_rolls(ch, get_curr_stat(ch, STAT_MAN) + ch->ability[INTIMIDATION].value, vch->willpower);
			if(fail > 0)
			{
				gain_condition(vch, COND_FEAR, fail * 15);
				send_to_char("The Wyrm speaks to you, warning you to flee!\n\r", vch);
				act_new("$N shivers and turns pale.", ch, NULL, vch, TO_NOTVICT, P_REST, 0);
				do_function(vch, &do_flee, "");
			}
		}
	}
}

/* do_wwgift1_ahr: Falling Touch - Formerly ahroun1 */
void do_wwgift1_ahr(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *vch;
	int fail = 0;

	if(ch->auspice != auspice_lookup("ahroun") || !ch->disc[DISC_AUSPICE])
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if(IS_NULLSTR(argument))
	{
		send_to_char("Who do you want to knock down?\n\r", ch);
		return;
	}

	if((vch = get_char_room(ch, argument)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	fail = dice_rolls(ch, get_curr_stat(ch, STAT_STR) + ch->ability[BRAWL].value, get_curr_stat(vch, STAT_STA));
	ch->power_timer = 1;

	if(fail > 0)
	{
		/*stun target*/
		if(vch->position > P_STUN)
		{
			act_new("You knock $N over!", ch, NULL, vch, TO_CHAR, P_STUN, 1);
			act_new("$n knocks you over!", ch, NULL, vch, TO_VICT, P_STUN, 1);
			act_new("$n knocks $N over!", ch, NULL, vch, TO_NOTVICT, P_STUN, 0);
			vch->position = P_STUN;
		}
		else
		{
			act_new("$N is already stunned!", ch,NULL,vch,TO_CHAR,P_DEAD,1);
		}
	}
	else
	{
		/*fail messages to target and user*/
		/*mobs attack*/
		act_new("You fail to knock $N down.", ch, NULL, vch, TO_CHAR, P_STUN,1);
		act_new("$n tried to knock you down!", ch, NULL, vch, TO_VICT,P_STUN,1);
		if(IS_NPC(vch))
		{
			set_fighting(vch, ch);
			multi_hit(vch, ch, TYPE_UNDEFINED);
		}
	}
}

/* do_wwgift3_the: Command Spirit - Formerly theurge3 */
void do_wwgift3_the(CHAR_DATA *ch, char *string)
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH]={'\0'};
	char buf[MSL]={'\0'};
	int a;

	string = one_argument(string,arg);

	if(ch->auspice != auspice_lookup("theurge") && ch->disc[DISC_AUSPICE] < 3)
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if (IS_NULLSTR(arg))
	{
		send_to_char("Command whom?\n\r", ch);
		return;
	}

	if (IS_NULLSTR(string))
	{
		send_to_char("What do you want done?", ch);
		return;
	}

	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	if ( victim == ch )
	{
		send_to_char( "Massochist! Just type the command.\n\r", ch );
		return;
	}

	if(!IS_SET(victim->act, ACT_UMBRA) && !IS_SET(victim->act, ACT_CHIMERAE)
			&& !IS_SET(victim->act2, ACT2_SPIRIT) && victim->race != race_lookup("wraith"))
	{
		send_to_char( "This can only affect spirits.\n\r", ch );
		return;
	}

	send_to_char(Format("You will %s\n\r", string), ch);
	a = dice_rolls( ch, (get_curr_stat(ch,STAT_MAN) + ch->ability[LEADERSHIP].value), victim->willpower );
	ch->power_timer = 2;

	if ((ch->race = race_lookup("vampire")) && (ch->disc[DISC_DOMINATE] >= 1)) {
		if ((ch->trust < victim->trust
				|| ch->gen > victim->gen)
				&& IS_NATURAL(victim))
		{
			send_to_char(Format("%s is above your influence.\n\r", victim->name), ch);
			return;
		}
		if (a > 0)
		{
			do_function(ch, &do_say, buf);
			interpret( victim, string );
			return;
		}
		else
		{
			do_function(ch, &do_say, buf);
			return;
		}
	}
	else
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}
}

/* do_wwgift3_9: Sense Silver - Formerly do_sensesilver */
void do_wwgift3_9(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *vch;
	OBJ_DATA *obj;
	bool found = FALSE;

	if(ch->race != race_lookup("werewolf") || !IS_SET(ch->powers[2], I))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	for(vch = ch->in_room->people;vch;vch = vch->next_in_room)
	{
		for(obj = vch->carrying; obj; obj = obj->next_content)
		{
			if(!str_cmp(obj->material, "silver"))
			{
				act("$N's $o sparkles with a silver glint.", ch, obj, vch, TO_CHAR, 1);

				found = TRUE;
			}
		}
	}

	for(obj = ch->in_room->contents; obj; obj = obj->next_content)
	{
		if(!str_cmp(obj->material, "silver"))
		{
			act("$o sparkles with a silver glint.", ch, obj, NULL, TO_CHAR, 1);
			found = TRUE;
		}
	}

	if(!found)
	{
		send_to_char("You sense no silver present.\n\r", ch);
	}

	ch->power_timer = 2;
}

/* do_wwgift4_ahr: Share Will - Formerly do_sharewill */
void do_wwgift4_ahr(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *gch;

	if(ch->auspice != auspice_lookup("ahroun") || ch->disc[DISC_AUSPICE] < 4)
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if(ch->willpower <= 0)
	{
		send_to_char("You have no willpower to share!\n\r", ch);
		return;
	}

	for ( gch = char_list; gch != NULL; gch = gch->next )
	{
		if ( is_same_group( gch, ch ) && gch != ch )
		{
			if(gch->willpower < gch->max_willpower)
				gch->willpower++;
		}
	}

	ch->willpower--;

	ch->power_timer = 4;
}

/* do_wwgift4_phi: Scent of the Beyond - Formerly do_scentbeyond */
void do_wwgift4_phi(CHAR_DATA *ch, char *argument)
{
	char arg[MSL]={'\0'};
	CHAR_DATA *vch;
	int fail = 0;
	ROOM_INDEX_DATA *location;
	ROOM_INDEX_DATA *original;
	OBJ_DATA *on;
	int b_umb = TRUE;
	int b_drm = FALSE;

	if(ch->auspice != auspice_lookup("philodox") || ch->disc[DISC_AUSPICE] < 4)
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if(IS_NULLSTR(argument))
	{
		send_to_char("Sneak a peek at whom?\n\r", ch);
		return;
	}

	one_argument(argument, arg);

	if((vch = get_char_world(ch, arg)) == NULL ||
			IS_SET(vch->act, ACT_UMBRA))
	{
		send_to_char("They aren't around.\n\r", ch);
		return;
	}

	if(ch->trust < vch->trust)
	{
		send_to_char( "You cannot view that location.\n\r", ch );
		return;
	}

	fail = dice_rolls(ch, get_curr_stat(ch, STAT_PER) + ch->ability[OCCULT].value, (vch->willpower + 3 > 10) ? 10 : vch->willpower + 3);

	ch->power_timer = 4;
	if(fail > 0)
	{
		if ( ( location = find_location( ch, arg ) ) == NULL )
		{
			send_to_char( "No such location.\n\r", ch );
			return;
		}

		if (!is_room_owner(ch,location) && room_is_private( location )
				&&  get_trust(ch) < MAX_LEVEL)
		{
			send_to_char( "You cannot view that location.\n\r", ch );
			return;
		}

		original = ch->in_room;
		on = ch->on;
		char_from_room( ch );
		char_to_room( ch, location );
		if(!IS_SET(ch->act, ACT_UMBRA))
		{
			SET_BIT(ch->act, ACT_UMBRA);
			b_umb = FALSE;
		}
		if(IS_SET(ch->act, ACT_DREAMING))
		{
			REMOVE_BIT(ch->act, ACT_DREAMING);
			b_drm = FALSE;
		}
		do_function(ch, &do_look, "");
		if(!b_umb)
		{
			REMOVE_BIT(ch->act, ACT_UMBRA);
		}
		if(!b_drm)
		{
			SET_BIT(ch->act, ACT_DREAMING);
		}
		char_from_room( ch );
		char_to_room( ch, original );
		ch->on = on;
	}
	else
	{
		send_to_char("You fail to see anything.\n\r", ch);
	}
}

/* do_wwgift4_the: Exorcism - Formerly do_exorcism */
void do_wwgift4_the(CHAR_DATA *ch, char *argument)
{
	int fail = 0;
	CHAR_DATA *vch;

	if(ch->auspice != auspice_lookup("theurge") || ch->disc[DISC_AUSPICE] < 4)
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	send_to_char("You exorcise the room, driving out spirits.\n\r", ch);
	act("$n commands all spirits from the room!", ch, NULL, NULL, TO_ROOM, 0);
	ch->power_timer = 2;

	for(vch = ch->in_room->people; vch; vch = vch->next_in_room)
	{
		if(IS_SET(vch->act2, ACT2_SPIRIT)
				|| vch->race == race_lookup("wraith")
				|| IS_SET(vch->act, ACT_CHIMERAE))
		{
			fail = dice_rolls(ch, get_curr_stat(ch, STAT_MAN) + ch->ability[INTIMIDATION].value, vch->willpower);
			if(fail > 0)
			{
				gain_condition(vch, COND_FEAR, fail * 15);
				act("$n's exorcism forces you to flee!\n\r", ch, NULL, vch, TO_VICT, 1);
				act_new("$N shivers and turns pale.", ch, NULL, vch, TO_NOTVICT, P_REST,0);
				do_function(vch, &do_flee, "");
			}
		}
	}
}

/* do_wwgift1_6: Resist Pain - Formerly do_painresist */
void do_wwgift1_6(CHAR_DATA *ch, char *argument)
{
	AFFECT_DATA af;

	if(ch->race != race_lookup("werewolf") || !IS_SET(ch->powers[0], F))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if(IS_AFFECTED(ch, AFF_RESIST_PAIN))
	{
		send_to_char("You are already resisting pain.\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	ch->power_timer = 2;

	af.where     = TO_AFFECTS;
	af.type      = skill_lookup("resist pain");
	af.level     = ch->disc[DISC_AUSPICE];
	af.location  = APPLY_PAIN;
	af.modifier  = -100;
	af.duration  = 1+ch->disc[DISC_AUSPICE];
	af.bitvector = AFF_RESIST_PAIN;
	affect_to_char( ch, &af );
	send_to_char("You steel yourself against pain.\n\r", ch);
}

/* do_wwgift4b_phi: True Form - Formerly do_trueform */
void do_wwgift4b_phi(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	int fail = 0;

	if(ch->auspice != auspice_lookup("philodox") || ch->disc[DISC_AUSPICE] < 4)
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if((victim = get_char_room(ch, argument)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	ch->power_timer = 3;
	fail = dice_rolls(ch,
			get_curr_stat(ch, STAT_MAN) + ch->ability[RITUALS].value,
			victim->max_willpower);

	act("$n says in a commanding tone, 'Take your true form $N!'.", ch, NULL, victim, TO_ROOM, 0);
	act("You say in a commanding tone, 'Take your true form $N!'.", ch, NULL, victim, TO_CHAR, 1);

	if(fail > 1)
	{
		if(ch->race == race_lookup("werewolf"))
		{
			if(ch->breed == breed_lookup("metis") && ch->shape != SHAPE_CRINOS)
				do_transform(ch, "crinos");
			else if(ch->breed == breed_lookup("lupus")
					&& ch->shape != SHAPE_WOLF)
				do_transform(ch, "wolf");
			else if(ch->breed == breed_lookup("homid")
					&& ch->shape != SHAPE_HUMAN && ch->shape != SHAPE_NONE )
				do_transform(ch, "human");
		}
		else if(ch->race == race_lookup("vampire") && ch->shape != SHAPE_HUMAN
				&& ch->shape != SHAPE_NONE)
			do_transform(ch, "human");
		else
			send_to_char("Nothing happens.\n\r", ch);
	}
	else
	{
		send_to_char("Nothing happens.\n\r", ch);
	}
}

/* do_wwgift5_gal: Bridge Walk - Formerly do_bridgewalk */
void do_wwgift5_gal(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	int fail = 0;

	if(ch->auspice != auspice_lookup("galliard") || ch->disc[DISC_AUSPICE] < 5)
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if((victim = get_char_room(ch, argument)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	ch->power_timer = 3;
	fail = dice_rolls(ch, get_curr_stat(ch, STAT_INT) + ch->ability[RITUALS].value, 7);

	act("$n holds $m hands above $m head and howls.", ch,NULL,NULL,TO_ROOM,0);
	send_to_char("You hold your hands above your head and howl the bridge chant.\n\r", ch);

	if(fail > 2)
	{
		act("$n vanishes in a flash of light.", ch, NULL, NULL, TO_ROOM, 0);
		send_to_char("The silver bridge opens up.\n\r", ch);
		char_from_room(ch);
		char_to_room(ch, victim->in_room);
		act("$n appears in a flash of light.", ch, NULL, NULL, TO_ROOM, 0);
		do_look(ch, "");
		do_function(ch, &do_look, "");
		send_to_char("You blink, adjusting to your new surroundings.\n\r", ch);
	}
	else
	{
		send_to_char("Nothing happens.\n\r", ch);
	}
}

/* do_wwgift4_5: Grovel - Formerly do_grovel */
void do_wwgift4_5(CHAR_DATA *ch, char *argument)
{
	int fail = 0;

	if(ch->race != race_lookup("werewolf") || !IS_SET(ch->powers[3], E))
	{
		if( !check_social( ch, "grovel", argument ) )
			send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	ch->power_timer = 2;
	fail = dice_rolls(ch, get_curr_stat(ch, STAT_MAN) + ch->ability[LEADERSHIP].value, 8);
	if(fail > 2)
	{
		do_function(ch, &do_peace, "");
		act("$n grovels in the dirt.", ch, NULL, NULL, TO_ROOM, 0);
		send_to_char("You grovel in the dirt.\n\r", ch);
	}
	else
	{
		act("$n grovels in the dirt.", ch, NULL, NULL, TO_ROOM, 0);
		send_to_char("You grovel in the dirt.\n\r", ch);
	}
}

/* do_wwgift1_4: Smell of Man - Formerly do_smellofman */
void do_wwgift1_4(CHAR_DATA *ch, char *argument)
{
	int fail = 0;
	CHAR_DATA *vch;

	if(ch->race != race_lookup("werewolf") || !IS_SET(ch->powers[0], D))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	send_to_char("You exude a human smell.\n\r", ch);
	ch->power_timer = 1;

	for(vch = ch->in_room->people; vch; vch = vch->next_in_room)
	{
		if(IS_ANIMAL(vch))
		{
			fail = dice_rolls(ch, get_curr_stat(ch, STAT_APP) + ch->ability[INTIMIDATION].value, vch->willpower);
			if(fail > 0)
			{
				gain_condition(vch, COND_FEAR, fail * 15);
				act("$n smells like a man! You flee in panic!", ch, NULL, vch, TO_VICT, 1);
				act_new("$N shivers and turns pale.", ch, NULL, vch, TO_NOTVICT, P_REST, 0);
				do_function(ch, &do_flee, "");
			}
		}
	}
}

/* do_wwgift1_9: Persuasion - Formerly do_persuasion */
void do_wwgift1_9(CHAR_DATA *ch, char *argument)
{
	int fail = 0;
	AFFECT_DATA af;

	if(ch->race != race_lookup("werewolf") || !IS_SET(ch->powers[0], I))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if(IS_NULLSTR(argument))
	{
		send_to_char("Which social attribute to you wish to increase?\n\r", ch);
		return;
	}

	if(stat_lookup("charisma", ch) != stat_lookup(argument, ch)
			&& stat_lookup("manipulation", ch) != stat_lookup(argument, ch)
			&& stat_lookup("appearance", ch) != stat_lookup(argument, ch))
	{
		send_to_char("You can only improve on social attributes.\n\r", ch);
		return;
	}

	ch->power_timer = 1;
	send_to_char("You attempt to enhance your persuasiveness.\n\r", ch);

	if((fail = dice_rolls(ch, ch->willpower, 8)) < 2)
		return; /* Failure state */

	af.where     = TO_AFFECTS;
	af.type      = skill_lookup("persuasion");
	af.level     = ch->disc[DISC_BREED];
	af.modifier  = 1;
	af.duration  = 2*ch->disc[DISC_BREED];
	af.bitvector = AFF_NONE;

	if(stat_lookup("charisma", ch) == stat_lookup(argument, ch))
	{
		af.location  = APPLY_CHA;
	}
	else if(stat_lookup("manipulation", ch) == stat_lookup(argument, ch))
	{
		af.location  = APPLY_MAN;
	}
	else if(stat_lookup("appearance", ch) == stat_lookup(argument, ch))
	{
		af.location  = APPLY_APP;
	}

	affect_to_char( ch, &af );
}

/* do_wwgift2_4: Shed - Formerly do_shed */
void do_wwgift2_4(CHAR_DATA *ch, char *argument)
{
	int fail = 0;
	AFFECT_DATA af;

	if(ch->race != race_lookup("werewolf") || !IS_SET(ch->powers[1], D))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	ch->power_timer = 2;
	send_to_char("You attempt to shed your skin.\n\r", ch);

	if((fail = dice_rolls(ch, ch->willpower, 8)) < 2)
		return; /* Failure state */

	af.where     = TO_AFFECTS;
	af.type      = skill_lookup("shed");
	af.level     = ch->disc[DISC_BREED];
	af.duration  = 2*ch->disc[DISC_BREED];
	af.bitvector = AFF_NONE;

	af.location  = APPLY_DEX;
	af.modifier  = 1;
	affect_to_char( ch, &af );

	af.location  = APPLY_STA;
	af.modifier  = -1;
	affect_to_char( ch, &af );

	/* Creating a shed skin might be worthwhile. */
}

/* do_wwgift3_7: Weak Arm - Formerly do_weakarm */
void do_wwgift3_7(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	int fail = 0;
	AFFECT_DATA af;

	if(ch->race != race_lookup("werewolf") || !IS_SET(ch->powers[2], G))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if(IS_NULLSTR(argument))
	{
		send_to_char("Who do you want to weaken?\n\r", ch);
		return;
	}

	if( (victim = get_char_room(ch, argument)) == NULL)
	{
		send_to_char("They aren't here!\n\r", ch);
		return;
	}

	fail = dice_rolls(ch, get_curr_stat(ch, STAT_DEX) + ch->ability[RITUALS].value, victim->max_willpower);
	ch->power_timer = 1;

	act("You attempt to weaken $N with your touch.", ch,NULL,victim,TO_CHAR,1);
	if(fail <= 0)
	{
		send_to_char("You fail to connect.\n\r", ch);
		return;
	}
	else if(fail > 0)
	{
		af.level     = ch->disc[DISC_AUSPICE];

		af.where     = TO_AFFECTS;
		af.type      = skill_lookup("weak arm");
		af.duration  = fail * 2;
		af.bitvector = AFF_NONE;
		af.location  = APPLY_DIFFICULTY;
		af.modifier  = 2;
		affect_to_char( victim, &af );
		af.location  = APPLY_DICE;
		af.modifier  = -1;
		affect_to_char( victim, &af );
		act("You feel weaker as $n touches you.", ch, NULL, victim, TO_VICT, 1);
		act("$n touches $N.", ch, NULL, victim, TO_NOTVICT, 0);
	}
}

/* do_wwgift3_4: Disquiet - Formerly do_disquiet */
void do_wwgift3_4(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	int fail = 0;
	AFFECT_DATA af;

	if (ch->race != race_lookup("werewolf") || !IS_SET(ch->powers[2], D))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if(IS_NULLSTR(argument))
	{
		send_to_char("Who do you want to distress?\n\r", ch);
		return;
	}

	if( (victim = get_char_room(ch, argument)) == NULL)
	{
		send_to_char("They aren't here!\n\r", ch);
		return;
	}

	fail = dice_rolls(ch, get_curr_stat(ch, STAT_MAN) + ch->ability[EMPATHY].value, victim->max_willpower);
	ch->power_timer = 1;

	act("You attempt to disquiet $N.", ch, NULL, victim, TO_CHAR, 1);
	if(fail <= 0)
	{
		return;
	}
	else if(fail > 0)
	{
		af.level     = ch->disc[DISC_BREED];
		af.where     = TO_AFFECTS;
		af.type      = skill_lookup("disquiet");
		af.duration  = fail * 2;
		af.location  = APPLY_DIFFICULTY;
		af.modifier  = 2;
		af.bitvector = AFF_NONE;
		affect_to_char( victim, &af );
		act("You feel a deep sense of disquiet.", ch, NULL, victim, TO_VICT, 1);
	}
}

/* do_wwgift5_the: Spirit Drain - Formerly do_spiritdrain */
void do_wwgift5_the(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	int fail = 0;

	if(ch->auspice != auspice_lookup("theurge") || ch->disc[DISC_AUSPICE] < 5)
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if(IS_NULLSTR(argument))
	{
		send_to_char("Which spirit do you wish to strike?\n\r", ch);
		return;
	}

	if( (victim = get_char_room(ch, argument)) == NULL)
	{
		send_to_char("They aren't here!\n\r", ch);
		return;
	}

	ch->power_timer = 2;

	fail = dice_rolls(ch,
			get_curr_stat(ch, STAT_STR) + ch->ability[RITUALS].value, 8);
	fail -= dice_rolls(victim, get_curr_stat(victim, STAT_STA) + 2, 8);

	if(fail < 2)
	{
		act("You cannot draw on $N's life force.", ch, NULL, victim,TO_CHAR,1);
		act("$n tries to draw on your life force!", ch, NULL, victim,TO_VICT,1);
		return;
	}

	ch->GHB += 1;
	if(ch->GHB > ch->max_GHB) ch->GHB = ch->max_GHB;

	act("You draw from $N's life force.", ch, NULL, victim, TO_CHAR, 1);
	act("$n draws on your life force!", ch, NULL, victim, TO_VICT, 1);

	damage(ch, victim, 1, skill_lookup("spirit drain"), DAM_NEGATIVE,TRUE,1,-1);
}

/* do_wwgift5_2: Luna's Avenger - Formerly do_lunasavenger */
void do_wwgift5_2(CHAR_DATA *ch, char *argument)
{
	int fail = 0;
	AFFECT_DATA af;

	if(ch->race != race_lookup("werewolf") || !IS_SET(ch->powers[4], B))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	ch->power_timer = 5;

	fail = dice_rolls(ch, get_curr_stat(ch, STAT_INT) + ch->ability[RITUALS].value, 9);

	if(fail < 2)
	{
		send_to_char("Nothing happens.\n\r", ch);
		return;
	}

	if(ch->shape != SHAPE_CRINOS)
		do_function(ch, &do_ww_transform, "crinos");
	act("Your claws and fur turn to gleaming silver.", ch, NULL,NULL,TO_CHAR,1);
	act("$n's claws and fur turn to gleaming silver.", ch, NULL,NULL,TO_ROOM,0);

	af.level     = ch->disc[DISC_BREED];
	af.where     = TO_AFFECTS;
	af.type      = skill_lookup("luna's avenger");
	af.duration  = fail * 2;
	af.bitvector = AFF_SCLAWS;
	af.location  = APPLY_HEALTH;
	af.modifier  = 1;
	affect_to_char( ch, &af );
	af.location  = APPLY_STR;
	af.modifier  = 2;
	affect_to_char( ch, &af );
	af.location  = APPLY_STA;
	af.modifier  = 2;
	affect_to_char( ch, &af );
}

/* do_wwgift5_5: Luna's Blessing - Formerly do_lunasbless */
void do_wwgift5_5(CHAR_DATA *ch, char *argument)
{
	int fail = 0;
	AFFECT_DATA af;

	if(ch->race != race_lookup("werewolf") || !IS_SET(ch->powers[4], E))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	ch->power_timer = 2;

	fail = dice_rolls(ch, get_curr_stat(ch, STAT_INT) + ch->ability[RITUALS].value, 9);

	if(fail < 2)
	{
		send_to_char("Nothing happens.\n\r", ch);
		return;
	}

	act("You no longer fear silver.", ch, NULL, NULL, TO_CHAR, 1);

	af.level     = ch->disc[DISC_BREED];
	af.where     = TO_AFFECTS;
	af.type      = skill_lookup("luna's blessing");
	af.duration  = fail * 2;
	af.bitvector = AFF_LUNA;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	affect_to_char( ch, &af );
}


/* do_wwgift1_10: Shroud - Formerly do_shroud */
void do_wwgift1_10(CHAR_DATA *ch, char *argument)
{
	int fail = 0;
	AFFECT_DATA af;

	if(ch->race != race_lookup("werewolf") || !IS_SET(ch->powers[0], J))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	ch->power_timer = 1;

	fail = dice_rolls(ch,
			get_curr_stat(ch, STAT_INT) + ch->ability[OCCULT].value,
			gauntlet[ch->in_room->sector_type]);

	if(fail < 1)
	{
		send_to_char("Nothing happens.\n\r", ch);
		act("$n looks surprised as nothing happens.", ch,NULL,NULL,TO_ROOM,0);
		return;
	}

	act("You weave a dark shroud.", ch, NULL, NULL, TO_CHAR, 1);
	act("$n causes a dark shroud descend, blocking out all light.", ch, NULL, NULL, TO_ROOM, 0);

	af.level     = ch->disc[DISC_TRIBE];
	af.where     = TO_AFFECTS;
	af.type      = skill_lookup("shroud");
	af.duration  = fail * 2;
	af.bitvector = ROOM_DARK;
	af.location  = APPLY_ROOM_LIGHT;
	af.modifier  = -10000;
	affect_to_room( ch->in_room, &af );
}


void do_vicissitude1 (CHAR_DATA *ch, char *string)
{
	AFFECT_DATA af;
	char arg[MAX_INPUT_LENGTH]={'\0'};
	int app_mod = 0;

	CheckCH(ch);

	if( ch->race != race_lookup("vampire") || ch->disc[DISC_VICISSITUDE] < 1)
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if(IS_AFFECTED2(ch, AFF2_VICISSITUDE1))
	{
		send_to_char("Your features return to normal.\n\r", ch);
		affect_strip(ch, AFF2_VICISSITUDE1);
		affect_strip(ch, skill_lookup("mask"));
		REMOVE_BIT(ch->affected_by2, AFF2_VICISSITUDE1);
		PURGE_DATA(ch->alt_name);
		ch->alt_name = str_dup(ch->name);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if(IS_NULLSTR(string))
	{
		send_to_char("That doesn't change your face.\n\r", ch);
		return;
	}

	one_argument(string, arg);

	if(is_number(arg))
	{
		string = one_argument(string, arg);
		app_mod = atoi(arg);
	}

	af.where     = TO_AFFECTS;
	af.type      = skill_lookup("mask");
	af.level     = ch->disc[DISC_VICISSITUDE];
	af.duration  = -1;
	af.location  = APPLY_APP;
	af.modifier  = app_mod;
	af.bitvector = AFF2_VICISSITUDE1;
	affect_to_char( ch, &af );
	act("$n's features shift!", ch, NULL, NULL, TO_ROOM, 0);
	send_to_char( "Your features shift.\n\r", ch );
	PURGE_DATA(ch->alt_name);
	ch->alt_name = str_dup(string);
	ch->power_timer = 3;
}


void do_vicissitude4 (CHAR_DATA *ch, char * string)
{
	AFFECT_DATA af;

	if((ch->race != race_lookup("vampire")) || (ch->disc[DISC_VICISSITUDE] < 4))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if(IS_SET(ch->form,FORM_HORRID))
	{
		send_to_char("You revert to your regular shape.\n\r", ch);
		act("$n reverts from their horrid form.", ch, NULL, NULL, TO_ROOM, 0);
		REMOVE_BIT(ch->form,FORM_HORRID);
		affect_strip(ch, skill_lookup("horrid form"));
		return;
	}

	if (!has_enough_power(ch))
		return;

	if(ch->RBPG <= 2)
	{
		send_to_char("You don't have enough blood to do that.\n\r", ch);
		return;
	}

	ch->power_timer = 3;
	WAIT_STATE(ch, 2);
	ch->RBPG -= 2;
	send_to_char("Your features shift as your body contorts.\n\r", ch);
	act("$n's features contort into a horrid form!\n\r", ch, NULL, NULL, TO_ROOM, 0);
	SET_BIT(ch->form, FORM_HORRID);

	af.where     = TO_AFFECTS;
	af.type      = skill_lookup("horrid form");
	af.level     = ch->disc[DISC_VICISSITUDE];
	af.duration  = -1;
	af.bitvector = AFF_NONE;
	af.modifier  = 3;
	af.location  = APPLY_STR;
	affect_to_char( ch, &af );
	af.location  = APPLY_DEX;
	affect_to_char( ch, &af );
	af.location  = APPLY_STA;
	affect_to_char( ch, &af );
	af.location  = APPLY_CHA;
	af.modifier  = -1 * UMIN(5, get_curr_stat(ch, STAT_CHA));
	affect_to_char( ch, &af );
	af.location  = APPLY_MAN;
	af.modifier  = -1 * UMIN(5, get_curr_stat(ch, STAT_MAN));
	affect_to_char( ch, &af );
	af.location  = APPLY_APP;
	af.modifier  = -1 * UMIN(5, get_curr_stat(ch, STAT_APP));
	affect_to_char( ch, &af );
}


void do_vicissitude5 (CHAR_DATA *ch, char * string)
{
	OBJ_DATA *obj, *obj_next;

	CheckCH(ch);

	if((ch->race != race_lookup("vampire")) || (ch->disc[DISC_VICISSITUDE] < 5))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if(IS_SET(ch->form,FORM_BLOOD))
	{
		send_to_char("You condense into your solid form.\n\r", ch);
		act("A puddle of blood solidifies into $n.", ch, NULL, NULL, TO_ROOM, 0);
		REMOVE_BIT(ch->form,FORM_BLOOD);
		return;
	}

	if (!has_enough_power(ch))
		return;

	ch->power_timer = 3;
	WAIT_STATE(ch, 2);
	send_to_char("Your body turns to blood.\n\r", ch);
	act("$n transforms into blood.!\n\r", ch, NULL, NULL, TO_ROOM, 0);
	SET_BIT(ch->form, FORM_BLOOD);
	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
		obj_next = obj->next_content;

		obj_from_char( obj );
		obj_to_room( obj, ch->in_room );
		if (IS_OBJ_STAT(obj,ITEM_MELT_DROP))
		{
			act("$p dissolves into blood.",ch,obj,NULL,TO_ROOM,0);
			act("$p dissolves into blood.",ch,obj,NULL,TO_CHAR,1);
			extract_obj(obj);
		}
	}
}


void do_thaumaturgy1 (CHAR_DATA *ch, char *string)
{
	send_to_char ("This discipline is not implemented at this time.", ch);

}


int amp_using_blood(CHAR_DATA *ch, char *argument, CHAR_DATA *forcer)
{
	CHAR_DATA *to = forcer ? forcer : ch;
	int stat = -1, current;

	if(IS_NULLSTR(argument))
	{
		send_to_char("You need to provide a physical attribute to raise.\n\r", to);
		return FALSE;
	}

	if(str_prefix(argument, "strength")
			|| str_prefix(argument, "dexterity")
			|| str_prefix(argument, "stamina"))
	{
		send_to_char("That isn't a physical attribute.\n\r", to);
		return FALSE;
	}

	if(!str_prefix(argument, "strength")) stat = STAT_STR;
	else if(!str_prefix(argument, "dexterity")) stat = STAT_DEX;
	else if(!str_prefix(argument, "stamina")) stat = STAT_STA;
	else
	{
		send_to_char("That isn't a physical attribute.\n\r", to);
		return FALSE;
	}

	if(stat == -1)
	{
		send_to_char("You should never see this message.\n\rPlease let Rayal know that you did.\n\r", to);
		return FALSE;
	}

	current = get_curr_stat(ch, stat);

	ch->RBPG--;

	if( current < 2*ch->perm_stat[stat]
	                              || (current > 2*ch->perm_stat[stat]
	                                                            && dice_rolls(ch, ch->perm_stat[STAT_STA], get_curr_stat(ch,stat)) > 0))
	{
		AFFECT_DATA af;
		af.where     = TO_AFFECTS;
		af.type      = skill_lookup("blood amp");
		af.level     = 1;
		af.location  = stat + 1;
		af.modifier  = 1;
		af.duration  = 10;
		af.bitvector = AFF_NONE;
		affect_to_char( ch, &af );
		send_to_char( "The blood surges in your veins!\n\r", ch );
	}

	ch->power_timer++;

	return TRUE;
}


void do_thaumaturgy2 (CHAR_DATA *ch, char *string)
{
	char arg[MSL]={'\0'};
	int diff = 5;
	int fail = 0;
	CHAR_DATA *vch;

	CheckCH(ch);

	if( ch->race != race_lookup("vampire") || ch->disc[DISC_THAUMATURGY] < 2)
	{
		send_to_char("You do not know Blood Rage.\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if(ch->RBPG <= 1)
	{
		send_to_char("You don't have the blood to spend on that!\n\r", ch);
		return;
	}

	string = one_argument(string, arg);

	if(IS_NULLSTR(string) || IS_NULLSTR(arg))
	{
		send_to_char("\tCbloodrage [target] [physical attribute]\tn\n\r", ch);
		return;
	}

	if((vch = get_char_room(ch, arg)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	if(vch->race != race_lookup("vampire"))
	{
		act("$N can't burn blood like that.", ch, NULL, vch, TO_CHAR, 1);
		return;
	}

	fail = dice_rolls(ch, ch->willpower, diff);

	if(fail > 0)
	{
		gain_condition(vch, COND_FRENZY, 10);
		if(!amp_using_blood(vch, string, ch)) return;
		act("You touch $N causing $M to use up blood.", ch,NULL,vch,TO_CHAR,1);
	}
	else if (fail == 0)
	{
		act("You touch $N.", ch, NULL, vch, TO_CHAR, 1);
	}
	else
	{
		act("You fumble, and collide with $N.", ch, NULL, vch, TO_CHAR, 1);
		amp_using_blood(ch, string, NULL);
		gain_condition(ch, COND_FRENZY, 10);
		act("$n stumbles into you.", ch, NULL, vch, TO_VICT, 0);
		act("$n stumbles into $N.", ch, NULL, vch, TO_NOTVICT, 0);
	}

	ch->RBPG--;
	ch->power_timer = 2;
}


void do_thaumaturgy3(CHAR_DATA *ch, char *argument)
{
	AFFECT_DATA af;
	int fail = dice_rolls(ch, ch->willpower, 6);

	CheckCH(ch);

	if( ch->race != race_lookup("vampire") || ch->disc[DISC_THAUMATURGY] < 3)
	{
		send_to_char("You do not know Potency of Blood.\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if(ch->RBPG <= 1)
	{
		send_to_char("You don't have the blood to spend on that!\n\r", ch);
		return;
	}

	if(fail == 0)
	{
		send_to_char("Nothing happens.\n\r", ch);
		return;
	}

	if(fail < 0)
	{
		fail = UMIN(fail, 13 - ch->gen);
		af.where     = TO_AFFECTS;
		af.type      = skill_lookup("blood of potency");
		af.level     = fail;
		af.location  = APPLY_GENERATION;
		af.modifier  = fail;
		af.duration  = fail;
		af.bitvector = AFF_NONE;
		affect_to_char( ch, &af );
		send_to_char( "The blood surges in your veins!\n\r", ch );
		return;
	}

	af.where     = TO_AFFECTS;
	af.type      = skill_lookup("blood of potency");
	af.level     = fail;
	af.location  = APPLY_GENERATION;
	af.modifier  = UMIN(-fail/2, ch->gen - 3);
	af.duration  = fail;
	af.bitvector = AFF_NONE;
	affect_to_char( ch, &af );
	send_to_char( "The blood surges in your veins!\n\r", ch );

	ch->RBPG--;
	ch->power_timer = 2;

}


void do_thaumaturgy4(CHAR_DATA *ch, char *argument)
{
	/*
	 * AFFECT_DATA af;
	 * int fail = dice_rolls(ch, ch->willpower, 7);
	 */

	 CheckCH(ch);

	if( ch->race != race_lookup("vampire") || ch->disc[DISC_THAUMATURGY] < 4)
	{
		send_to_char("You do not know Theft of Vitae\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if(ch->RBPG <= 1)
	{
		send_to_char("You don't have the blood to spend on that!\n\r", ch);
		return;
	}

	ch->RBPG--;
	ch->power_timer = 2;
}


void do_thaumaturgy5(CHAR_DATA *ch, char *argument)
{
	int fail = 0;
	int diff, blood;
	char arg[MAX_INPUT_LENGTH]={'\0'};
	CHAR_DATA *vch;

	CheckCH(ch);

	/* Cauldron of Blood (diff = UMIN(4 + # of targeted blood points, 10), 1 dam/blood point/mortal death - NPC kill outright, PC must have conceded) */

	if( ch->race != race_lookup("vampire") || ch->disc[DISC_THAUMATURGY] < 5)
	{
		send_to_char("You do not know Cauldron of Blood.\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if(ch->RBPG <= 1)
	{
		send_to_char("You don't have the blood to spend on that!\n\r", ch);
		return;
	}

	argument = one_argument(argument, arg);

	if((vch = get_char_room(ch, argument)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	if(!is_number(arg))
	{
		send_to_char("You must declare how many blood points you want to boil.\n\r", ch);
		return;
	}

	ch->RBPG--;
	ch->power_timer = 2;

	blood = atoi(arg);
	diff = UMIN(4+blood, 10);
	fail = dice_rolls(ch, ch->willpower, diff);

	/* Check for mortality */

	/* If mortal and (an NPC or a PC conceding to be killed), kill the target */

	/* Otherwise, damage the target if not in RPmode */
	/* If in RPmode, announce the damage to target and aggressor
    		AND an ST if one is present AND (one or both are in a session run by them OR if ST is staff) */
}


void clear_rite(CHAR_DATA *ch)
{
	int i;

	for(i = 0; i < MAX_RITE_STEPS; i++)
	{
		ch->riteacts[i] = -1;
	}
	ch->ritepoint = 0;
}


int rite_available(int sn, CHAR_DATA *ch)
{
	ORG_DATA *org;
	ORGMEM_DATA *mem;

	if(strstr(race_table[ch->race].name, ritual_table[sn].races))
	{
		if(ritual_table[sn].level == 0) return TRUE;

		if(ritual_table[sn].disc_test < 0
				&& ch->race == race_lookup("werewolf")
				&& ritual_table[sn].level <= ch->backgrounds[RACE_STATUS])
			return TRUE;

		if(ritual_table[sn].disc_test >= 0
				&& ch->disc[ritual_table[sn].disc_test] >= ritual_table[sn].level)
			return TRUE;
	}

	for(org = org_list; org != NULL; org = org->next)
	{
		if((mem = mem_lookup(org, ch->name)) != NULL
				&& strstr(org->name, ritual_table[sn].races)
		&& mem->status >= ritual_table[sn].level)
			return TRUE;
	}

	return FALSE;
}


void do_rituals( CHAR_DATA *ch, char *argument )
{
	char arg1[MAX_INPUT_LENGTH]={'\0'};
	char arg2[MAX_INPUT_LENGTH]={'\0'};
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	void *vo;
	int sn, rn;
	int target;

	CheckCH(ch);

	target_name = one_argument( argument, arg1 );
	one_argument( target_name, arg2 );

	if(ch->power_timer > 0)
	{
		send_to_char("Your powers have not rejuvenated yet.\n\r", ch);
		return;
	}

	if ( IS_NULLSTR(arg1) )
	{
		send_to_char( "Which action are you trying to perform?\n\r", ch );
		do_function(ch, &do_rituals, "list actions");
		return;
	}

	if(!str_prefix("cancel", arg1))
	{
		send_to_char("You stop performing the ritual.\n\r", ch);
		clear_rite(ch);
		return;
	}

	if(!str_prefix(arg1, "list"))
	{
		if(IS_NULLSTR(arg2))
		{
			send_to_char("Are you trying to list the actions or rites?\n\r",ch);
			return;
		}

		if(!str_prefix(arg2, "actions"))
		{
			send_to_char("Available ritual actions are:\n\r", ch);
			for(sn = 0; rite_actions[sn].name != NULL; sn++)
			{
				if(sn%4==0) send_to_char("\n\r", ch);
				send_to_char(Format("%14s", rite_actions[sn].name), ch);
			}
			send_to_char("\n\r", ch);
			return;
		}

		if(!str_prefix(arg2, "rites"))
		{
			rn = 1;
			send_to_char("Available rituals are:\n\r", ch);
			for(sn = 0; ritual_table[sn].name != NULL; sn++)
			{
				if(rite_available(sn, ch))
				{
					send_to_char(Format("%14s", rite_actions[sn].name), ch);
					rn++;
				}
				if(rn%4==0) send_to_char("\n\r", ch);
			}
			send_to_char("\n\r", ch);
			send_to_char("In order to use any of the above rituals,", ch);
			send_to_char(" you need to know the proper actions.\n\r", ch);
			send_to_char("There is no automated method of learning rites.)\n\r", ch);
			return;
		}

		send_to_char("Are you trying to list the actions or rites?\n\r",ch);
		return;
	}

	if((rn = riteaction_lookup(arg1)) >= 0 && rn < MAX_RITE_ACTIONS)
	{
		if(ch->ritepoint >= MAX_RITE_STEPS)
		{
			send_to_char("You can't add any more steps to this ritual!\n\r",ch);
			send_to_char("(Use 'ritual intone' to complete it.)\n\r",ch);
			return;
		}

		act(rite_actions[rn].to_char, ch, NULL, NULL, TO_CHAR, 1);
		act(rite_actions[rn].to_room, ch, NULL, NULL, TO_ROOM, 0);
		ch->riteacts[ch->ritepoint] = rn;
		ch->ritepoint++;
		WAIT_STATE( ch, rite_actions[rn].beats );
		return;
	}

	if(str_prefix(arg1, "intone"))
	{
		send_to_char("That is not a valid action.\n\r", ch);
		return;
	}


	log_string(LOG_GAME, "rite: 1");
	if((rn = rite_lookup(ch)) < 0)
	{
		send_to_char("Nothing happens.\n\r", ch);
		clear_rite(ch);
		return;
	}

	log_string(LOG_GAME, "rite: 2");
	if ((sn = find_spell(ch,ritual_table[rn].name)) < 1)
	{
		send_to_char( "Your actions seem to mean nothing... yet...\n\r", ch );
		return;
	}

	if (skill_table[sn].spell_fun == spell_null)
	{
		send_to_char( "Your actions seem to mean nothing...\n\r", ch );
		return;
	}

	if ((!IS_NPC(ch) && !skill_table[sn].available[ch->race]))
	{
		send_to_char( "Your actions seem to mean nothing to you...\n\r", ch );
		return;
	}

	log_string(LOG_GAME, "rite: 3");
	/*
	 * Locate targets.
	 */
	victim	= NULL;
	obj		= NULL;
	vo		= NULL;
	target	= TARGET_NONE;

	switch ( skill_table[sn].target )
	{
	default:
		log_string(LOG_BUG, Format("Do_ritual: bad target for sn %d.", sn ));
		return;

	case TAR_IGNORE:
		break;

	case TAR_CHAR_DEFENSIVE:
		if ( IS_NULLSTR(arg2) )
		{
			victim = ch;
		}
		else
		{
			if ( ( victim = get_char_room( ch, target_name ) ) == NULL )
			{
				send_to_char( "They aren't here.\n\r", ch );
				return;
			}
		}

		vo = (void *) victim;
		target = TARGET_CHAR;
		break;

	case TAR_CHAR_SELF:
		if ( !IS_NULLSTR(arg2) && !is_name( target_name, ch->name ) )
		{
			send_to_char("You cannot perform this ritual for another.\n\r", ch);
			return;
		}

		vo = (void *) ch;
		target = TARGET_CHAR;
		break;

	case TAR_OBJ_INV:
		if ( IS_NULLSTR(arg2) )
		{
			send_to_char( "What should the ritual be performed upon?\n\r", ch );
			return;
		}

		if ( ( obj = get_obj_carry( ch, target_name, ch ) ) == NULL )
		{
			send_to_char( "You are not carrying that.\n\r", ch );
			return;
		}

		vo = (void *) obj;
		target = TARGET_OBJ;
		break;

	case TAR_OBJ_CHAR_OFF:
		if (IS_NULLSTR(arg2))
		{
			if ((victim = ch->fighting) == NULL)
			{
				send_to_char("Perform the ritual on whom or what?\n\r",ch);
				return;
			}

			target = TARGET_CHAR;
		}
		else if ((victim = get_char_room(ch,target_name)) != NULL)
		{
			target = TARGET_CHAR;
		}

		if (target == TARGET_CHAR) /* check the sanity of the attack */
		{

			if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
			{
				send_to_char( "You can't do that on your master.\n\r",
						ch );
				return;
			}

			vo = (void *) victim;
		}
		else if ((obj = get_obj_here(ch,target_name)) != NULL)
		{
			vo = (void *) obj;
			target = TARGET_OBJ;
		}
		else
		{
			send_to_char("You don't see that here.\n\r",ch);
			return;
		}
		break;

	case TAR_OBJ_CHAR_DEF:
		if (IS_NULLSTR(arg2))
		{
			vo = (void *) ch;
			target = TARGET_CHAR;
		}
		else if ((victim = get_char_room(ch,target_name)) != NULL)
		{
			vo = (void *) victim;
			target = TARGET_CHAR;
		}
		else if ((obj = get_obj_carry(ch,target_name,ch)) != NULL)
		{
			vo = (void *) obj;
			target = TARGET_OBJ;
		}
		else
		{
			send_to_char("You don't see that here.\n\r",ch);
			return;
		}
		break;
	}

	/* FIX - Put in tests into spells (abil calls/skill calls) */

	log_string(LOG_GAME, "rite: 4");
	WAIT_STATE( ch, ritual_table[rn].beats );	// use rn here, not sn (sn points to skill table) rn points to the ritual_table
							// ritual table is what we need to find the beats for, not the skill_table
							// they are not hte same location : thus causes lockup.
	clear_rite(ch);

	log_string(LOG_GAME, "rite: 5");
	(*skill_table[sn].spell_fun) ( sn, ch->trust, ch, vo,target);

	log_string(LOG_GAME, "rite: 6");
	if ((skill_table[sn].target == TAR_CHAR_OFFENSIVE
			||   (skill_table[sn].target == TAR_OBJ_CHAR_OFF && target == TARGET_CHAR))
			&&   victim != ch)
	{
		CHAR_DATA *vch;
		CHAR_DATA *vch_next;

		for ( vch = ch->in_room->people; vch; vch = vch_next )
		{
			vch_next = vch->next_in_room;
			if ( victim == vch && victim->fighting == NULL )
			{
				strike( victim, ch );
				break;
			}
		}
	}

	log_string(LOG_GAME, "rite: 6");
	return;
}


/* Ritual Functions - Same format as spells */
void rite_pack( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
	PACK_DATA *pack;
	CHAR_DATA *vch = (CHAR_DATA *) vo;
	CHAR_DATA *wch;
	char arg[MAX_INPUT_LENGTH]={'\0'};
	int totem;

	CheckCH(ch);

	if(vch == NULL)
	{
		send_to_char("You haven't selected a leader for the new pack.\n\r", ch);
		return;
	}

	if(!is_in_group(vch) || !vch->leader)
	{
		send_to_char("They aren't the leader of a group.\n\r", ch);
		return;
	}

	if(ch->race != vch->race)
	{
		send_to_char("The pack leader must be the same type of creature as yourself.\n\r", ch);
		return;
	}

	if(target_name[0] == '\0')
	{
		send_to_char("What totem are you trying to select for the pack?\n\r", ch);
		return;
	}

	target_name = one_argument(target_name, arg);
	if((totem = flag_lookup(arg, spirit_table)) == -1)
	{
		send_to_char("That totem does not exist.\n\r", ch);
		return;
	}

	if(target_name[0] == '\0')
	{
		send_to_char("What is the name of the new pack?\n\r", ch);
		return;
	}

	pack = new_pack();
	pack->totem = totem;
	pack->name = str_dup(target_name);
	for(wch = char_list; wch; wch = wch->next)
	{
		if(is_same_group(wch, vch))
		{
			wch->pack = str_dup(pack->name);
			act("$n transforms your group into a cohesive pack!\n\r", wch, NULL, ch, TO_CHAR, 1);
		}
	}

	act("You call on $t to form a pack around $N", ch, spirit_table[totem].name, vch, TO_CHAR, 1);
}

void rite_attune( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	char arg[MAX_INPUT_LENGTH]={'\0'};
	CHAR_DATA *vch;

	target_name = one_argument(target_name, arg);

	if(!IS_NULLSTR(obj->owner) && IS_SET(obj->extra2, OBJ_ATTUNED))
	{
		send_to_char("Someone else already owns this item.\n\r", ch);
		return;
	}

	if(target_name[0] == '\0')
	{
		vch = ch;
	}
	else if((vch = get_char_room(ch, target_name)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	PURGE_DATA(obj->owner);
	obj->owner = str_dup(vch->name);
	SET_BIT(obj->extra2, OBJ_ATTUNED);
	act("$n performs a rite over $p, attuning it to $N.", ch, obj, vch, TO_ROOM, 0);
	act("You perform the rite over $p, attuning it to $N.", ch, obj, vch, TO_CHAR, 1);
	ch->power_timer = 5;
}

void rite_recognition( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
	CHAR_DATA *vch = (CHAR_DATA *) vo;

	if(ch == vch)
	{
		send_to_char("You cannot perform this rite on yourself.\n\r", ch);
		return;
	}

	ch->power_timer = 5;
	if(ch->backgrounds[RACE_STATUS] <= vch->backgrounds[RACE_STATUS] + 1)
	{
		act("$n performs a rite over $N, but has no effect on $N's rank.", ch, NULL, vch, TO_ROOM, 0);
		act("You perform the rite over $N, but has no effect on $M rank.", ch, NULL, vch, TO_CHAR, 1);
	}

	act("$n performs a rite over $N, improving $N's rank.", ch, NULL, vch, TO_ROOM, 0);
	act("You perform the rite over $N, improving $M rank.", ch, NULL, vch, TO_CHAR, 1);
	vch->backgrounds[RACE_STATUS]++;
}

void rite_hero( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{

	if(ch->exp < 5*ch->backgrounds[RACE_STATUS]/2)
	{
		send_to_char("You do not have enough experience to be successful in this rite.\n\r", ch);
		return;
	}

	act("$n performs a rite, howling your deeds into the sky.", ch, NULL, NULL, TO_ROOM, 0);
	act("You perform the rite, howling your deeds into the ether.", ch, NULL, NULL, TO_CHAR, 1);
	ch->power_timer = 20;

	if(UMAX(
			dice_rolls(ch,ch->ability[OCCULT].value+get_curr_stat(ch,STAT_WIT),7),
			dice_rolls(ch,ch->ability[RITUALS].value+get_curr_stat(ch,STAT_CHA),7))
			> ch->backgrounds[RACE_STATUS])
	{
		send_to_char("You sense that Gaia has smiled on you.\n\r", ch);
		ch->exp -= 5*ch->backgrounds[RACE_STATUS]/2;
		ch->backgrounds[RACE_STATUS]++;
		return;
	}

	send_to_char("Gaia has not seen fit to smile on your deeds.\n\r", ch);
}

void rite_introduction( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
	CHAR_DATA *vch, *vch_next;

	CheckCH(ch);

	if( count_words(target_name) < 1 || count_words(target_name) > 20)
	{
		send_to_char("You must send a message of 20 words or less.\n\r", ch);
		return;
	}


	// fixed a part of this from a potential crash (OI)
	for ( vch = char_list; vch != NULL; vch = vch_next )
	{
		// vch_next = important.  without this we can cause an infinate loop if
		// something gets smuckered.
		vch_next = vch->next;

		if(ch->clan == vch->clan)
			act("$N projects an introduction, '$t'", vch, target_name, ch, TO_CHAR, 1);
	}

	ch->power_timer = 2;
}


/* Spirit Invoking */
void generic_spirit(CHAR_DATA *ch, int level, int target, char *spirit)
{
	AFFECT_DATA af;

	af.where     = TO_AFFECTS;

	if(spirit[0] == '\0')
		af.type      = skill_lookup("war spirit");
	else
		af.type      = skill_lookup(spirit);

	af.level     = level;
	af.location  = target;
	af.modifier  = 1+level/2;
	af.duration  = level*2;
	af.bitvector = AFF_NONE;

	if(target == APPLY_DIFFICULTY) af.modifier = af.modifier * -1;

	affect_to_char( ch, &af );
}

void do_ancestor(CHAR_DATA *ch, char *argument)
{
	CheckCH(ch);
	WAIT_STATE( ch, 2 );

	if(ch->backgrounds[PASTLIFE] <= 0)
	{
		send_to_char("Sadly you have no connection to your ancestors.\n\r",ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if(dice_rolls(ch, ch->backgrounds[PASTLIFE], 8) <= 0)
	{
		send_to_char("Your ancestors ignore your call.\n\r", ch);
		ch->power_timer = 2;
		return;
	}

	generic_spirit(ch, 1, number_range(1,22), "ancestor spirit");
	send_to_char("You feel your ancestors join you.\n\r", ch);
	ch->power_timer = 4;
	return;
}

int spirit_war(CHAR_DATA *ch, char *argument, bool n_r, OBJ_DATA *obj)
{
	if(n_r)
	{
		ch->GHB--;
	}
	else if(dice_rolls(ch, ch->max_GHB, UMIN(8,obj->fetish_level+5)) <= 0)
	{
		send_to_char("The war spirit refuses to budge.\n\r", ch);
		return FALSE;
	}

	generic_spirit(ch, obj->fetish_level, obj->fetish_target, "");
	send_to_char("You feel invigorated by the war spirit.\n\r", ch);
	return TRUE;
}

int spirit_pain(CHAR_DATA *ch, char *argument, bool n_r, OBJ_DATA *obj)
{
	if(n_r)
	{
		ch->GHB--;
	}
	else if(dice_rolls(ch, ch->max_GHB, UMIN(8,obj->fetish_level+5)) <= 0)
	{
		send_to_char("The pain spirit refuses to budge.\n\r", ch);
		return FALSE;
	}

	generic_spirit(ch, obj->fetish_level, obj->fetish_target, "");
	send_to_char("You feel invigorated by the pain spirit.\n\r", ch);
	return TRUE;
}

int spirit_lune(CHAR_DATA *ch, char *argument, bool n_r, OBJ_DATA *obj)
{
	if(n_r)
	{
		ch->GHB--;
	}
	else if(dice_rolls(ch, ch->max_GHB, UMIN(8,obj->fetish_level+5)) <= 0)
	{
		send_to_char("The lune refuses to budge.\n\r", ch);
		return FALSE;
	}

	generic_spirit(ch, obj->fetish_level, obj->fetish_target, "");
	send_to_char("You feel invigorated by the lune.\n\r", ch);
	return TRUE;
}

int spirit_falcon(CHAR_DATA *ch, char *argument, bool n_r, OBJ_DATA *obj)
{
	if(n_r)
	{
		ch->GHB--;
	}
	else if(dice_rolls(ch, ch->max_GHB, UMIN(8,obj->fetish_level+5)) <= 0)
	{
		send_to_char("The falcon spirit refuses to budge.\n\r", ch);
		return FALSE;
	}

	generic_spirit(ch, obj->fetish_level, obj->fetish_target, "");
	send_to_char("You feel invigorated by the falcon spirit.\n\r", ch);
	return TRUE;
}

int spirit_thunder(CHAR_DATA *ch, char *argument, bool n_r, OBJ_DATA *obj)
{
	if(n_r)
	{
		ch->GHB--;
	}
	else if(dice_rolls(ch, ch->max_GHB, UMIN(8,obj->fetish_level+5)) <= 0)
	{
		send_to_char("The thunder spirit refuses to budge.\n\r", ch);
		return FALSE;
	}

	generic_spirit(ch, obj->fetish_level, obj->fetish_target, "");
	send_to_char("You feel invigorated by the thunder spirit.\n\r", ch);
	return TRUE;
}


/* @@@@@
 * Prototype for new power commands
 */
int power_cmd (POWER_TYPE *power, CHAR_DATA *ch, char *argument)
{
	return FALSE;
}

void do_scent_trueform ( CHAR_DATA * ch, char * string )
{
	CHAR_DATA *victim;
	char buf[MSL]={'\0'};
	char arg[MSL]={'\0'};
	int fail = 0;

	CheckCH(ch);

	if(ch->race != race_lookup("werewolf") && !IS_SET(ch->powers[1], E))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	one_argument(string, arg);

	if(IS_NULLSTR(arg))
	{
		send_to_char("Who do you wish to smell?\n\r", ch);
		return;
	}

	if( (victim = get_char_room(ch, string)) != NULL )
	{
		ch->power_timer = 2;
		fail = dice_rolls(ch, get_curr_stat(ch, STAT_PER)
				+ ch->ability[PRIMAL_URGE].value, 7);

		if (fail <= 0)
		{
			act("You cannot smell $N's aura.", ch, NULL, victim, TO_CHAR, 1);
			return;
		}

		send_to_char(Format("%s smells like", IS_NPC( victim ) ? victim->short_descr : victim->name), ch);
		if(fail >= 2)
		{
			switch(victim->race)
			{
			case RACE_WEREWOLF:
				strncat(buf,	" one of Gaia's\n\r", sizeof(buf));
				break;
			case RACE_VAMPIRE:
				strncat(buf, " the wyrm\n\r", sizeof(buf));
				break;
			case RACE_HUMAN:
				strncat(buf, " the weaver\n\r", sizeof(buf));
				break;
			}
		}

		if(fail >= 2)
		{
			if(victim->condition[COND_ANGER] > 50)
				strncat(buf, " touched by the Beast of War", sizeof(buf));
			if(victim->condition[COND_PAIN] > 50)
				strncat(buf, " touched by the Defiler Wyrm", sizeof(buf));
			if(victim->condition[COND_FEAR] > 50)
				strncat(buf, " touched by the Eater-of-Souls", sizeof(buf));
			if(victim->condition[COND_FRENZY] > 50)
				strncat(buf, " that is rapidly rippling", sizeof(buf));
			if(fail >= 2)
			{
				strncat(buf, ".\n\r", sizeof(buf));
			}
		}

		send_to_char(buf, ch);
		return;
	}
	else
	{
		send_to_char("They're not here.\n\r", ch);
		return;
	}
}

/*
 * Auspex powers
 */

void do_auspex1 ( CHAR_DATA * ch, char * string )
{
	AFFECT_DATA af;

	CheckCH(ch);

	if( IS_AFFECTED(ch, AFF_HSENSES) )
	{
		affect_strip ( ch, gsn_hsenses );
		REMOVE_BIT   ( ch->affected_by, AFF_HSENSES );
		ch->power_timer = 1;
		return;
	}

	if (!has_enough_power(ch))
		return;

	if( ( ch->race == race_lookup("vampire") && ch->disc[DISC_AUSPEX] >= 1 )
			|| ( ch->race == race_lookup("werewolf") && (ch->shape > SHAPE_HUMAN
					|| IS_SET(ch->powers[0], C)) ) )
	{
		af.where     = TO_AFFECTS;
		af.type      = skill_lookup("heighten");
		if(ch->race == race_lookup("vampire"))
		{
			af.level     = ch->disc[DISC_AUSPEX];
			af.duration  = ch->disc[DISC_AUSPEX] * 5;
			af.modifier  = ch->disc[DISC_AUSPEX];
		}
		else if(ch->race == race_lookup("werewolf"))
		{
			af.level     = ch->max_GHB;
			af.duration  = 5 * ch->max_GHB;
			af.modifier  = UMAX(1, number_fuzzy(ch->max_GHB));
		}
		af.location  = APPLY_PER;
		af.bitvector = AFF_HSENSES;
		affect_to_char( ch, &af );
		send_to_char("Your senses expand, drawing in greater detail.\n\r", ch);
		ch->power_timer = 1;
	}
	else
	{
		send_to_char("Huh?\n\r", ch);
	}
}

void do_auspex2 ( CHAR_DATA * ch, char * string )
{
	CHAR_DATA *victim;
	char buf[MSL]={'\0'};
	char arg[MAX_INPUT_LENGTH]={'\0'};
	int success = 0;
	int power_stat = 0;
	int power_ability = 0;
	int difficulty = 0;

	CheckCH(ch);

	power_stat = get_curr_stat(ch, STAT_PER);
	power_ability = ch->ability[EMPATHY].value;
	difficulty = 8;

	if((ch->race != race_lookup("vampire") 	&& ch->race != race_lookup("werewolf"))
			|| (IS_VAMPIRE(ch) && ch->disc[DISC_AUSPEX] < 2)
			|| (ch->race == race_lookup("werewolf") && !IS_SET(ch->powers[1], E)) )
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	one_argument(string, arg);

	if(IS_NULLSTR(arg))
	{
		send_to_char("Whose aura do you wish to read?\n\r", ch);
		return;
	}

	if( (victim = get_char_room(ch, string)) != NULL )
	{
		ch->power_timer = 2;
		success = dice_rolls(ch, power_stat + power_ability, difficulty);

		if (success <= 0)
		{
			if(ch->race == race_lookup("werewolf"))
				act("\tYFailure\tn: You cannot smell $N's aura.", ch, NULL, victim, TO_CHAR, 1);
			else
				act("\tYFailure\tn: You cannot sense $N's aura.", ch, NULL, victim, TO_CHAR, 1);
			return;
		}

		send_to_char(Format("\tGSuccess\tn: %s is surrounded by", IS_NPC( victim ) ? victim->short_descr : victim->name), ch);
		if(success >= 1)
		{
			switch(victim->race)
			{
			case RACE_WEREWOLF:
				strncat(buf,	" an intense and vibrant aura which moves like flames", sizeof(buf));
				break;
			case RACE_CHANGELING:
				strncat(buf, " a bright iridescent aura", sizeof(buf));
				break;
			case RACE_VAMPIRE:
				strncat(buf, " a pale aura", sizeof(buf));
				break;
			case RACE_HUMAN:
				strncat(buf, " a rosy aura", sizeof(buf));
				break;
			}
			if(success == 1)
			{
				strncat(buf, ".\n\r", sizeof(buf));
			}

		}
		if(success >= 2)
		{
			if(victim->condition[COND_ANGER] > 50)
				strncat(buf, " with red bursts", sizeof(buf));
			if(victim->condition[COND_PAIN] > 50)
				strncat(buf, " with patches of red and purples", sizeof(buf));
			if(victim->condition[COND_FEAR] > 50)
				strncat(buf, " with orange skittering lines", sizeof(buf));
			if(victim->condition[COND_FRENZY] > 50)
				strncat(buf, " that is rapidly rippling", sizeof(buf));
			if(victim->condition[COND_ANGER] < 50
					&& victim->condition[COND_FEAR] < 50
					&& victim->condition[COND_FRENZY] < 50
					&& victim->condition[COND_PAIN] < 50)
				strncat(buf, " that is mild blue color", sizeof(buf));
			if(success >= 2)
			{
				strncat(buf, ".\n\r", sizeof(buf));
			}
		}
		if(success >= 3)
		{
			strncat(buf, "You also see color patterns", sizeof(buf));

			if(victim->clan == clan_lookup("brujah"))
				strncat(buf, " in purple and violet", sizeof(buf));

			else if(victim->clan == clan_lookup("gangrel"))
				strncat(buf, " of green and brown", sizeof(buf));

			else if(victim->clan == clan_lookup("malkavian"))
				strncat(buf, " of yellow", sizeof(buf));

			else if(victim->clan == clan_lookup("nosferatu"))
				strncat(buf, " in a sickly green", sizeof(buf));

			else if(victim->clan == clan_lookup("toreador"))
				strncat(buf, " of green", sizeof(buf));

			else if(victim->clan == clan_lookup("tremere"))
				strncat(buf, " in brown and light green", sizeof(buf));

			else if(victim->clan == clan_lookup("ventrue"))
				strncat(buf, " in lavender", sizeof(buf));

			else if(victim->clan == clan_lookup("lasombra"))
				strncat(buf, " of pulsating and writhing black and grey", sizeof(buf));

			else if(victim->clan == clan_lookup("tzimisce"))
				strncat(buf, " in a myriad of sparkling green", sizeof(buf));

			else if(victim->clan == clan_lookup("assamite"))
				strncat(buf, " of blue with black arteries", sizeof(buf));

			else if(victim->clan == clan_lookup("settite"))
				strncat(buf, " in a deep red and gold", sizeof(buf));

			else if(victim->clan == clan_lookup("giovanni"))
				strncat(buf, " in a deep red and green", sizeof(buf));

			else if(victim->clan == clan_lookup("ravnos"))
				strncat(buf, " in sharp, flickering black and violet", sizeof(buf));

			else if(victim->clan == clan_lookup("child of cacophony"))
				strncat(buf, " in a mottled silver and yellow", sizeof(buf));

			else if(victim->clan == clan_lookup("bone"))
				strncat(buf, " of gold and light green", sizeof(buf));

			else if(victim->clan == clan_lookup("glass"))
				strncat(buf, " of gold and deep red", sizeof(buf));

			else if(victim->clan == clan_lookup("malk"))
				strncat(buf, " with hypnotic swirling colors.\n\r", sizeof(buf));

			else if(victim->clan == clan_lookup("tremere"))
				strncat(buf, " with a myriad of sparkles.\n\r", sizeof(buf));

			else if(victim->clan == clan_lookup("soc. of Leopold"))
				strncat(buf, " with bright gold.\n\r", sizeof(buf));

			else if(victim->clan == clan_lookup("silver"))
				strncat(buf, " of brown and gold", sizeof(buf));
			/*
			else if(victim->clan == clan_lookup("silent"))
				strncat(buf, " of gold and silver", sizeof(buf));
			 */
			else
				strncat(buf, " of white", sizeof(buf));

			if(success == 3)
			{
				strncat(buf, ".\n\r", sizeof(buf));
			}
		}

		if(success >= 4)
		{
			if(victim->GHB > 7)
				strncat(buf, " with golden flecks.\n\r", sizeof(buf));
			else if(victim->GHB < 3)
				strncat(buf, " with white flecks.\n\r", sizeof(buf));
		}

		send_to_char(buf, ch);
		return;
	}
	else
	{
		send_to_char("They're not here.\n\r", ch);
		return;
	}
}

void do_auspex3 ( CHAR_DATA * ch, char * argument )
{
	OBJ_DATA *obj;
	int success = 0;
	int power_stat = 0;
	int power_ability = 0;
	int difficulty = 0;

	CheckCH(ch);

	difficulty = 8;
	power_stat = get_curr_stat(ch, STAT_PER);
	power_ability = ch->ability[EMPATHY].value;

	success = dice_rolls(ch, power_stat + power_ability, difficulty);

	if(ch->race != race_lookup("vampire") || ch->disc[DISC_AUSPEX] < 3)
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if( (obj = get_obj_carry(ch, argument, ch)) == NULL )
	{
		send_to_char("You don't have that object.\n\r", ch);
		log_string(LOG_GAME, (char *)Format("Tried to spirittouch %s", argument));
		return;
	}

	if(IS_NULLSTR(argument))
	{
		send_to_char("What are you trying to learn about?\n\r", ch);
		return;
	}

	if(success<0)
	{
		send_to_char("You are overwhelmed by the psychic impressions on this object and it gives you a massive headache!", ch);
		ch->power_timer = 3;
	}

	if(success==0)
	{
		send_to_char("You get nothing off of this object.", ch);
		ch->power_timer = 3;
	}

	if(success>0)
	{
		if(IS_NULLSTR(obj->owner) || IS_BUILDERMODE(obj))
			send_to_char("No-one has touched this object.\n\r", ch);
		else
		{
			send_to_char( Format("You receive an image of %s holding %s.\n\r", obj->owner, obj->short_descr), ch );
		}
		ch->power_timer = 3;
	}
}

void do_auspex4(CHAR_DATA *ch, char *argument)
{
	CheckCH(ch);

	if(ch->race != race_lookup("vampire") || ch->disc[DISC_AUSPEX] < 4)
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if(IS_AFFECTED(ch,AFF_AUSPEX4))
	{
		REMOVE_BIT(ch->affected_by, AFF_AUSPEX4);
		send_to_char("You stop listening in on the thoughts of others.\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	send_to_char( "You focus to listen to thoughts.\n\r", ch );
	SET_BIT(ch->affected_by, AFF_AUSPEX4);
}

void do_auspex5(CHAR_DATA *ch, char *argument)
{
	int success = 0;
	int power_stat = 0;
	int power_ability = 0;
	int difficulty = 0;

	CheckCH(ch);

	difficulty = 8;
	power_stat = get_curr_stat(ch, STAT_PER);
	power_ability = ch->ability[OCCULT].value;

	success = dice_rolls(ch, power_stat + power_ability, difficulty);

	if(ch->race != race_lookup("vampire") || ch->disc[DISC_AUSPEX] < 5)
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if(ch->power_timer > 0 && !IS_SET(ch->act2, ACT2_ASTRAL))
	{
		send_to_char("Your powers have not rejuvenated yet.\n\r", ch);
		return;
	}

	ch->power_timer = 2;
	if(success > 0 || IS_SET(ch->parts, PART_THREAD))
	{
		if(!IS_SET(ch->act2, ACT2_ASTRAL))
		{
			do_function(ch, &do_sleep, "");
			if(ch->position != P_SLEEP)
			{
				send_to_char( "You can't get comfortable enough to concentrate.\n\r", ch);
				return;
			}

			SET_BIT(ch->act2, ACT2_ASTRAL);
			char_to_listeners(ch, ch->in_room);
			SET_BIT(ch->parts, PART_THREAD);
			send_to_char("You leave your body behind.\n\r", ch);
		}
		else
		{
			if(IS_SET(ch->parts, PART_THREAD))
			{
				REMOVE_BIT(ch->act2, ACT2_ASTRAL);
				REMOVE_BIT(ch->parts, PART_THREAD);
				char_from_listeners(ch);
				send_to_char("You return to your body.\n\r", ch);
				do_function(ch, &do_wake, "");
			}
			else
			{
				if(ch->listening == ch->in_room)
				{
					REMOVE_BIT(ch->act2, ACT2_ASTRAL);
					char_from_listeners(ch);
					send_to_char("You return to your body.\n\r", ch);
					do_function(ch, &do_wake, "");
				}
				else
				{
					send_to_char( "You can't find your way back to your body!\n\r", ch);
				}
			}
			return;
		}

	}
	else if(success <= 0)
	{
		send_to_char("\tYFailure\tn: Nothing happens.\n\r", ch);
	}
}

/*
 * Animalism powers
 */
/**
 * [do_animalism1  This is the Feral Whispers ability.]
 * @param ch     [description]
 * @param string [description]
 */
void do_animalism1 (CHAR_DATA *ch, char *string)
{
	char arg[MAX_INPUT_LENGTH]={'\0'};
	char buf[MSL]={'\0'};
	CHAR_DATA *victim;
	int success = 0;
	int power_stat = 0;
	int power_ability = 0;
	int difficulty = 0;

	CheckCH(ch);

	difficulty = 7;
	power_stat = get_curr_stat(ch,STAT_MAN);
	power_ability = ch->ability[ANIMAL_KEN].value;

	success = dice_rolls( ch, power_stat + power_ability, difficulty );

	string = one_argument(string,arg);
	string = one_argument(string,buf);

	if (ch->race != race_lookup("vampire") || !ch->disc[DISC_ANIMALISM]) 
	{
		send_to_char("\tRWarning\tn: You do not know the discipline Animalism.\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if (IS_NULLSTR(arg))
	{
		send_to_char("Command whom?\n\r", ch);
		return;
	}

	if (IS_NULLSTR(buf))
	{
		send_to_char("What do you want done?\n\r",ch);
		return;
	}

	if (IS_NULLSTR(string))
	{
		send_to_char("You have to say something to command someone!", ch);
		return;
	}

	if (!str_prefix(buf,"delete") || !str_prefix(buf, "quit")
			|| !str_prefix(buf, "concede"))
	{
		send_to_char("That will NOT be done.\n\r",ch);
		return;
	}

	if (!str_prefix(buf, "influences"))
	{
		send_to_char("\tRWarning\tn: You don't have enough control for that.\n\r",ch);
		return;
	}

	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	if ( !IS_ANIMAL(victim) )
	{
		send_to_char( "\tRWarning\tn: That power only works on animals.\n\r", ch);
		return;
	}

	if ( victim == ch )
	{
		send_to_char( "Massochist! Just type the command.\n\r", ch );
		return;
	}

	ch->power_timer = 1;

	if (ch->trust < victim->trust || (ch->gen > victim->gen && !IS_NATURAL(victim)))
	{
		send_to_char(Format("%s is above your influence.\n\r", victim->name), ch);
		return;
	}
	if (success > 0)
	{
		do_function(ch, &do_say, string);
		interpret( victim, buf );
		return;
	}
	else
	{
		do_function(ch, &do_say, string);
		return;
	}
}

/**
 * [do_animalism2  This is the Beckoning power.]
 * @param ch       [description]
 * @param argument [description]
 */
void do_animalism2 (CHAR_DATA *ch, char* argument)
{
	CHAR_DATA *victim;
	char arg[MSL]={'\0'};
	int success = 0;
	int difficulty = 0;
	int power_stat = 0;
	int power_ability = 0;

	CheckCH(ch);

	if((ch->race != race_lookup("vampire")
			&& ch->race != race_lookup("werewolf"))
			|| (ch->race == race_lookup("vampire") && ch->disc[DISC_ANIMALISM] < 4)
			|| (ch->race == race_lookup("werewolf") && !IS_SET(ch->powers[1], H)) )
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if(IS_NULLSTR(argument))
	{
		send_to_char("Beckon who?\n\r", ch);
		return;
	}

	one_argument(argument, arg);

	if((victim = get_char_area(ch, arg)) == NULL)
	{
		send_to_char("You can't seem to reach them.\n\r", ch);
		return;
	}

	if(!IS_ANIMAL(victim))
	{
		send_to_char("Your victim is not an animal.\n\r", ch);
		return;
	}

	ch->power_timer = 4;

	difficulty = victim->willpower;
	power_stat = get_curr_stat(ch, STAT_CHA);
	power_ability = ch->ability[SURVIVAL].value;

	success = dice_rolls(ch, power_stat + power_ability, difficulty);

	if (success > 3)
	{
		SET_BIT(victim->act, ACT_SUMMON);
		victim->summonner = ch;
		send_to_char("Your will reaches out, drawing in a supplicant.\n\r", ch);
		send_to_char("You feel yourself being called, and feel that you must obey.\n\r", victim);
		return;
	}
	else
	{
		send_to_char("You failed.\n\r", ch);
		return;
	}
}

/**
 * [do_animalism3  This is the Quell the Beast power.]
 * @param ch       [description]
 * @param argument [description]
 */
void do_animalism3 (CHAR_DATA *ch, char* argument)
{
	CHAR_DATA *victim;
	char arg[MSL]={'\0'};
	AFFECT_DATA af;
	int success = 0;
	int difficulty = 0;
	int power_stat = 0;
	int power_ability = 0;

	CheckCH(ch);

	if((ch->race != race_lookup("vampire")
			&& ch->race != race_lookup("werewolf"))
			|| (ch->disc[DISC_ANIMALISM] < 3
					&& ch->race == race_lookup("vampire"))
					|| (!IS_SET(ch->powers[4], J)
							&& ch->auspice == auspice_lookup("galliard")))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if(IS_NULLSTR(argument))
	{
		send_to_char("Who do you want to calm down?\n\r", ch);
		return;
	}

	one_argument(argument, arg);

	if((victim = get_char_room(ch, arg)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	if(!IS_NATURAL(victim) && ch->race != race_lookup("werewolf"))
	{
		send_to_char("You can't use this on supernatural creatures.\n\r", ch);
		return;
	}

	difficulty = victim->willpower;
	power_stat = get_curr_stat(ch, STAT_INT);
	power_ability = ch->ability[EMPATHY].value;

	success = dice_rolls(ch, power_stat + power_ability, difficulty);

	ch->power_timer = 2;

	if(success > (victim->willpower / 2))
	{
		af.where = TO_AFFECTS;
		af.type = skill_lookup("calm");
		af.level = ch->disc[DISC_ANIMALISM];
		af.duration = success * 5;
		af.location = APPLY_DEX;
		af.modifier = -2;
		af.bitvector = AFF_CALM;
		affect_to_char(victim, &af);

		af.location = APPLY_DEX;
		affect_to_char(victim, &af);

		if(victim->fighting || victim->position == P_FIGHT)
			stop_fighting(victim, FALSE);

		send_to_char(Format("Calm fills %s.\n\r", victim->name), ch);
		send_to_char("You suddenly feel calm and relaxed.\n\r", victim);
		victim->condition[COND_FRENZY] = 0;
		victim->condition[COND_ANGER] = 0;
		victim->condition[COND_FEAR] = 0;
	}
	else if (success < 0)
	{
		send_to_char("This looks bad.\n\r", ch);
		send_to_char(Format("%s tries to force you to be calm.\n\rYou don't appreciate that.\n\r", ch->name), victim);
		gain_condition(victim, COND_ANGER, 20);
		one_hit(victim, ch, TYPE_UNDEFINED);
		return;
	}
	else
	{
		send_to_char("There seems to be no effect.\n\r", ch);
		return;
	}
}

void do_animalism4(CHAR_DATA *ch, char *argument)
{
	CheckCH(ch);

	if (ch->race != race_lookup("vampire") || ch->disc[DISC_ANIMALISM] < 4)
	{
		if(!ch->desc->original)
		{
			send_to_char("Huh?\n\r", ch);
			return;
		}
	}

	if(ch->desc->original)
	{
		do_function(ch->desc->original, &do_wake, "");
		do_function(ch, &do_return, "");
		return;
	}

	if (!has_enough_power(ch))
		return;

	if(player_switch(ch, argument, TRUE))
		ch->power_timer = 6;
}

void do_animalism5(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *vch;
	int success = 0;
	int difficulty = 0;
	int power_stat = 0;
	int power_ability = 0;

	CheckCH(ch);

	difficulty = 8;
	power_stat = get_curr_stat(ch, STAT_MAN);
	power_ability = ch->ability[ANIMAL_KEN].value;

	if (ch->race != race_lookup("vampire") || ch->disc[DISC_ANIMALISM] < 5)
	{
		if(!ch->desc->original)
		{
			send_to_char("Huh?\n\r", ch);
			return;
		}
	}

	if (!has_enough_power(ch))
		return;

	if((vch = get_char_room(ch, argument)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	success = dice_rolls(ch, power_stat + power_ability, difficulty);

	if(success == 0)
	{
		send_to_char("Nothing happens.\n\r", ch);
	}
	else if(success > 0)
	{
		send_to_char("The rage ebbs from your soul!\n\r", ch);
		ch->condition[COND_FRENZY] = 0;
		send_to_char("The rage builds in your soul!\n\r", vch);
		gain_condition(vch, COND_FRENZY, 50);
		be_frenzied(vch);
	}
	else
	{
		send_to_char("The rage builds in your soul!\n\r", ch);
		gain_condition(ch, COND_FRENZY, 50);
		be_frenzied(ch);
	}
	ch->power_timer = 6;
}

/*
 * Celerity
 */
void do_celerity (CHAR_DATA *ch, char *string)
{
	char commands[40][MSL];
	int i,j,k=0;

	CheckCH(ch);

	for (i=0;i<40;i++)
	{
		if(IS_NULLSTR(string))
			break;
		else
		{
			for(j=0;j<MSL;j++)
			{
				if(IS_NULLSTR(string))
					break;

				if(string[0] == ';')
				{
					(string)++;
					break;
				}
				else
				{
					commands[i][j] = string[0];
					(string)++;
				}
			}
		}
		k++;
	}

	SET_BIT(ch->act, ACT_FAST);
	for(i=0;i<=k;i++)
	{
		if((ch->race == race_lookup("vampire") && ch->disc[DISC_CELERITY] >= k-1)
				|| (ch->race == race_lookup("werewolf") && ch->RBPG >= k-1))
		{
			if(str_cmp(commands[i], "celerity")
					&& str_cmp(commands[i],"ragemove")) interpret(ch, commands[i]);
		}
	}
	REMOVE_BIT(ch->act, ACT_FAST);
}

/*
 * Dominate Powers
 */
void do_dominate1 (CHAR_DATA *ch, char *string)
{
	int fail = 0;
	char arg[MAX_INPUT_LENGTH]={'\0'};
	char buf[MSL]={'\0'};
	CHAR_DATA *victim;

	CheckCH(ch);

	string = one_argument(string,arg);
	string = one_argument(string,buf);

	if (ch->race != race_lookup("vampire") || !ch->disc[DISC_DOMINATE]) {
		send_to_char("You do not know the discpline Dominate.\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if (IS_NULLSTR(arg))
	{
		send_to_char("Command whom?\n\r", ch);
		return;
	}

	if (IS_NULLSTR(buf))
	{
		send_to_char("What do you want done?\n\r",ch);
		return;
	}

	if (IS_NULLSTR(string))
	{
		send_to_char("You have to say something to command someone!", ch);
		return;
	}

	if (!str_prefix(buf,"delete") || !str_prefix(buf, "quit")
			|| !str_prefix(buf, "concede"))
	{
		send_to_char("That will NOT be done.\n\r",ch);
		return;
	}

	if (!str_prefix(buf, "influences"))
	{
		send_to_char("You don't have enough control for that.\n\r",ch);
		return;
	}

	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	if ( victim == ch )
	{
		send_to_char( "Massochist! Just type the command.\n\r", ch );
		return;
	}

	fail = dice_rolls( ch, (get_curr_stat(ch,STAT_MAN) + ch->ability[INTIMIDATION].value), victim->willpower );
	ch->power_timer = 1;

	if(IS_SET(victim->act2,ACT2_RESIST))
	{
		send_to_char("They seem to have a very strong willpower.\n\r", ch);
		fail--;
		victim->willpower--;
	}

	if (ch->trust < victim->trust || (ch->gen > victim->gen	&& !IS_NATURAL(victim)))
	{
		send_to_char(Format("%s is above your influence.\n\r", victim->name), ch);
		return;
	}

	do_function( ch, &do_say, string );
	if( fail > 0 )
	   interpret( victim, buf );
}

void do_dominate2 (CHAR_DATA *ch, char *string)
{
	int fail = 0;
	char arg[MAX_INPUT_LENGTH]={'\0'};
	char buf[MSL]={'\0'};
	CHAR_DATA *victim;

	CheckCH(ch);

	string = one_argument(string,arg);

	if( (ch->race == race_lookup("vampire") && ch->disc[DISC_DOMINATE] < 2)
	/*|| (ch->clan == clan_lookup("silver fang") && ch->disc[DISC_TRIBE] < 3)*/)
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if (IS_NULLSTR(arg))
	{
		send_to_char("Command whom?\n\r", ch);
		return;
	}

	if (IS_NULLSTR(string))
	{
		send_to_char("What do you want done?", ch);
		return;
	}

	if (!str_cmp(buf,"delete") || !str_cmp(buf, "quit") || !str_cmp(buf, "concede"))
	{
		send_to_char("That will NOT be done.\n\r",ch);
		return;
	}

	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	if ( victim == ch )
	{
		send_to_char( "Massochist! Just type the command.\n\r", ch );
		return;
	}

	send_to_char(Format("You will %s\n\r", string), ch);
	fail = dice_rolls( ch, (get_curr_stat(ch,STAT_MAN) + ch->ability[LEADERSHIP].value), victim->willpower );
	ch->power_timer = 2;

	if(IS_SET(victim->act2,ACT2_RESIST))
	{
		send_to_char("\tRThey seem to have a very strong willpower.\tn\n\r", ch);
		fail--;
		victim->willpower--;
	}


	if ((ch->race = race_lookup("vampire")) && (ch->disc[DISC_DOMINATE] >= 1)) {
		if (ch->trust < victim->trust
				|| (ch->gen > victim->gen
						&& !IS_NATURAL(victim)))
		{
			send_to_char(Format("%s is above your influence.\n\r", victim->name), ch);
			return;
		}
		if (fail > 0)
		{
			do_function(ch, &do_say, buf);
			interpret( victim, string );
			return;
		}
		else
		{
			do_function(ch, &do_say, buf);
			return;
		}
	}
	else
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}
}

void do_dominate4 (CHAR_DATA *ch, char* argument)
{

	char arg[MAX_INPUT_LENGTH]={'\0'};
	CHAR_DATA *victim;

	CheckCH(ch);

	one_argument(argument, arg);

	if(ch->race != race_lookup("vampire") || ch->disc[DISC_DOMINATE] < 4)
	{
		send_to_char("Huh!\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if(IS_NULLSTR(arg))
	{
		send_to_char("Who do you wish to control?\n\r", ch);
		return;
	}

	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	ch->power_timer = 4;
	if(ch->disc[DISC_DOMINATE] >= 4)
		spell_charm_person(skill_lookup("charm"), ch->disc[DISC_DOMINATE], ch, (void *)victim, 0);
	else
	{
		send_to_char("Bugger... Looks like you screwed up!\n\r", ch);
		send_to_char(Format("%s stares at you very intently till their eyes cross.\n\r", ch->name), victim);
	}
}

void do_dominate5(CHAR_DATA *ch, char *argument)
{
	CheckCH(ch);

	if (ch->race != race_lookup("vampire") || ch->disc[DISC_DOMINATE] < 5) {
		if(!ch->desc->original)
		{
			send_to_char("Huh?\n\r", ch);
			return;
		}
	}

	if(ch->desc->original) {
		do_function(ch->desc->original, &do_wake, "");
		do_function(ch, &do_return, "");
		return;
	}

	if (!has_enough_power(ch))
		return;

	if(player_switch(ch, argument, FALSE))
		/*ch->power_timer = 7*/;
}

/*
 * Obfuscate Powers
 */
void do_obfuscate1 (CHAR_DATA *ch, char *argument)
{
	CheckCH(ch);

	if((ch->race != race_lookup("vampire")
			&& ch->race != race_lookup("werewolf"))
			|| (ch->race == race_lookup("vampire")
					&& ch->disc[DISC_OBFUSCATE] < 1)
					|| (ch->race == race_lookup("werewolf") && !IS_SET(ch->powers[1], F)) )
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if (IS_ALONE(ch) && room_is_dark(ch->in_room))
	{
		do_function(ch, &do_hide,"");
		ch->power_timer = 1;
	}
	else
	{
		send_to_char("You can't hide where there are no shadows, or with someone watching.\n\r", ch);
		return;
	}
}

void do_obfuscate2 (CHAR_DATA *ch, char *argument)
{
	AFFECT_DATA af;

	CheckCH(ch);

	if((ch->race != race_lookup("vampire") && ch->race != race_lookup("werewolf"))
			|| (ch->race == race_lookup("vampire") && ch->disc[DISC_OBFUSCATE] < 2)
			|| (ch->race == race_lookup("werewolf") && !IS_SET(ch->powers[2], F)) )
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if(IS_AFFECTED(ch, AFF_INVISIBLE))
	{
		send_to_char("You step out of the shadows.\n\r", ch);
		affect_strip(ch, AFF_INVISIBLE);
		affect_strip(ch, skill_lookup("invis"));
		REMOVE_BIT(ch->affected_by, AFF_INVISIBLE);
		PURGE_DATA(ch->alt_name);
		ch->alt_name = str_dup(ch->name);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if (IS_ALONE(ch))
	{
		af.where     = TO_AFFECTS;
		af.type      = skill_lookup("invis");
		if(ch->race == race_lookup("vampire"))
			af.level     = ch->disc[DISC_OBFUSCATE];
		af.duration  = -1;
		af.location  = APPLY_NONE;
		af.modifier  = 0;
		af.bitvector = AFF_INVISIBLE;
		affect_to_char( ch, &af );
		send_to_char( "You fade out of existence.\n\r", ch );
		ch->power_timer = 2;
	}
}

void do_obfuscate3 (CHAR_DATA *ch, char *string)
{
	AFFECT_DATA af;
	char arg[MAX_INPUT_LENGTH]={'\0'};
	int app_mod = 0;

	CheckCH(ch);

	if( (ch->race != race_lookup("vampire")
			&& ch->race != race_lookup("werewolf"))
			|| (ch->race == race_lookup("vampire") && ch->disc[DISC_OBFUSCATE] < 3)
			|| (ch->race == race_lookup("werewolf") && !IS_SET(ch->powers[3], C)) )
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if(IS_AFFECTED(ch, AFF_OBFUSCATE3))
	{
		send_to_char("Your features return to normal.\n\r", ch);
		affect_strip(ch, AFF_OBFUSCATE3);
		affect_strip(ch, skill_lookup("mask"));
		REMOVE_BIT(ch->affected_by, AFF_OBFUSCATE3);
		PURGE_DATA(ch->alt_name);
		ch->alt_name = str_dup(ch->name);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if(IS_NULLSTR(string))
	{
		send_to_char("That doesn't change your face.\n\r", ch);
		return;
	}

	one_argument(string, arg);

	if(is_number(arg))
	{
		string = one_argument(string, arg);
		app_mod = atoi(arg);
	}

	if (IS_ALONE(ch))
	{
		af.where     = TO_AFFECTS;
		af.type      = skill_lookup("mask");
		if(ch->race == race_lookup("vampire"))
			af.level     = ch->disc[DISC_OBFUSCATE];
		if(ch->clan == clan_lookup("glass walker"))
			af.level     = ch->disc[DISC_TRIBE];
		af.duration  = -1;
		af.location  = APPLY_APP;
		af.modifier  = app_mod;
		af.bitvector = AFF_OBFUSCATE3;
		affect_to_char( ch, &af );
		send_to_char( "Your features shift.\n\r", ch );
		PURGE_DATA(ch->alt_name);
		ch->alt_name = str_dup(string);
		ch->power_timer = 3;
	}
	else
	{
		send_to_char("You can't change your looks with others around.\n\r", ch);
	}
}

void do_obfuscate4 (CHAR_DATA *ch, char *argument)
{
	AFFECT_DATA af;
	int success = 0;
	int power_stat = 0;
	int power_ability = 0;
	int difficulty = 0;

	CheckCH(ch);

	power_stat = get_curr_stat(ch, STAT_WIT);
	power_ability = ch->ability[SUBTERFUGE].value;
	difficulty = 7;

	if((ch->race == race_lookup("vampire") && ch->disc[DISC_OBFUSCATE] < 4))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if(IS_AFFECTED(ch, AFF_INVISIBLE))
	{
		send_to_char("You step out of the shadows.\n\r", ch);
		affect_strip(ch, AFF_INVISIBLE);
		affect_strip(ch, skill_lookup("invis"));
		REMOVE_BIT(ch->affected_by, AFF_INVISIBLE);
		PURGE_DATA(ch->alt_name);
		ch->alt_name = str_dup(ch->name);
		return;
	}


	if (!has_enough_power(ch))
		return;


	success = dice_rolls(ch, power_stat + power_ability, difficulty);

	if (success > 0)
	{
		af.where     = TO_AFFECTS;
		af.type      = skill_lookup("invis");
		af.level     = ch->disc[DISC_OBFUSCATE];
		af.duration  = -1;
		af.location  = APPLY_NONE;
		af.modifier  = 0;
		af.bitvector = AFF_INVISIBLE;
		affect_to_char( ch, &af );
		send_to_char( "You fade out of existence.\n\r", ch );
		ch->power_timer = 2;
	}
}

void do_obfuscate5 (CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *wch;
	char arg[MAX_INPUT_LENGTH]={'\0'};
	bool found = FALSE;
	AFFECT_DATA af;
	int fail = 0, app_mod = 0;

	CheckCH(ch);

	if(ch->race != race_lookup("vampire")
			|| (ch->race == race_lookup("vampire") && ch->disc[DISC_OBFUSCATE] < 5))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if(IS_NULLSTR(argument))
	{
		send_to_char("Which power do you wish to activate for your group?\n\r", ch);
		return;
	}

	if(!IS_GROUP_ALONE(ch))
	{
		send_to_char("Your group is not alone here...\n\r", ch);
		send_to_char("Try again in a more secluded locale.\n\r", ch);
		return;
	}

	argument = one_argument(argument, arg);

	switch(arg[0])
	{
	case 'c':
		if(!str_cmp(arg, "cloak"))
		{
			if (room_is_dark(ch->in_room))
			{
				send_to_char("You cloak your group in shadow.\n\r", ch);
				act("$n cloaks the group in shadow.", ch, NULL, NULL, TO_GROUP,1);
				for(wch = ch->in_room->people;wch;wch = wch->next_in_room)
					do_function(wch, &do_hide,"");
			}
			else
			{
				send_to_char("You can't hide where there are no shadows, or with someone watching.\n\r", ch);
				return;
			}
			/* Actually do the cloak thing... */
			found = TRUE;
			ch->power_timer = 5;
		}
		break;
	case 'm':
		if(!str_cmp(arg, "mask"))
		{
			found = TRUE;
			if(IS_NULLSTR(argument))
			{
				send_to_char("You have to mask one group member at a time.\n\r", ch);
				return;
			}

			argument = one_argument(argument, arg);

			if((wch = get_char_room(ch, arg)) == NULL)
			{
				send_to_char("They aren't here.\n\r", ch);
				return;
			}

			if(!is_same_group(ch, wch))
			{
				send_to_char("They aren't in the same group as you.\n\r",
						ch);
				return;
			}

			if(IS_NULLSTR(argument))
			{
				send_to_char("That doesn't change your face.\n\r", ch);
				return;
			}

			/* Actually do the mask thing... */
			one_argument(argument, arg);

			if(is_number(arg))
			{
				argument = one_argument(argument, arg);
				app_mod = atoi(arg);
			}

			af.where     = TO_AFFECTS;
			af.type      = skill_lookup("mask");
			af.level     = ch->disc[DISC_OBFUSCATE];
			af.duration  = ch->disc[DISC_OBFUSCATE] * 12;
			af.location  = APPLY_APP;
			af.modifier  = app_mod;
			af.bitvector = AFF_OBFUSCATE3;
			affect_to_char( wch, &af );
			PURGE_DATA(wch->alt_name);
			wch->alt_name = str_dup(argument);
			act("You change $N's appearance.", ch, NULL, wch, TO_CHAR, 1);
			act("$n changes your appearance.", ch, NULL, wch, TO_VICT, 1);
			act("$n changes $N's appearance.", ch, NULL, wch, TO_GROUP, 1);
			ch->power_timer = 3;
		}
		break;
	case 'u':
		if(!str_cmp(arg, "unseen"))
		{
			found = TRUE;
			/* Actually do the unseen thing... */
			fail = dice_rolls(ch, get_curr_stat(ch, STAT_WIT)
					+ ch->ability[SUBTERFUGE].value, 7);

			if (fail > 0)
			{
				for(wch = ch->in_room->people;wch;wch = wch->next_in_room)
				{
					if(is_same_group(wch, ch))
					{
						af.where     = TO_AFFECTS;
						af.type      = skill_lookup("invis");
						af.level     = ch->disc[DISC_OBFUSCATE];
						af.duration  = ch->disc[DISC_OBFUSCATE] * 12;
						af.location  = APPLY_NONE;
						af.modifier  = 0;
						af.bitvector = AFF_INVISIBLE;
						affect_to_char( wch, &af );
					}
				}
				send_to_char("You render the group invisible.\n\r", ch);
				act("$n renders the group invisible.", ch, NULL, NULL, TO_GROUP, 1);
			}
			else
			{
				send_to_char("You strain, but fail to cause the group to fade from sight.\n\r", ch);
			}
			ch->power_timer = 5;
		}
		break;
	}

	if(!found)
	{
		send_to_char("You need to select a power to use for your group.\n\r", ch);
		send_to_char("Your options are: cloak, unseen and mask.\n\r", ch);
	}
}

/*
 * Presence Powers
 */
void do_presence1 (CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	int fail = dice_rolls(ch, get_curr_stat(ch, STAT_CHA) + ch->ability[INTIMIDATION].value, 7);

	CheckCH(ch);

	if( ch->disc[DISC_PRESENCE] && ch->race == race_lookup("vampire") )
	{
		if (!has_enough_power(ch))
			return;

		if(IS_ALONE(ch))
		{
			send_to_char("There's no-one here to be awestruck!\n\r",ch);
			return;
		}

		ch->power_timer = 1;
		if(fail > 0)
		{
			act("$n draws your eye and holds your gaze.", ch,NULL,NULL,TO_ROOM,0);
			for(victim = ch->in_room->people; victim != NULL; victim = victim->next_in_room)
			{
				if(victim != ch)
					WAIT_STATE(victim, fail*2);
			}
			send_to_char("Everyone turns to stare at you!\n\r", ch);
			return;
		}
		else if(fail == 0)
		{
			send_to_char("Nobody seems to notice.\n\r", ch);
		}
		else if(fail < 0)
		{
			send_to_char("You strut about, making a complete fool of yourself.\n\r", ch);
			act("$n struts about, looking a fool.", ch, NULL, NULL, TO_ROOM, 0);
		}
		return;
	}
	else
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}
}

void do_presence2 (CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	int fail = 0;
	AFFECT_DATA af;

	CheckCH(ch);

	if((ch->disc[DISC_PRESENCE] < 2 && ch->race == race_lookup("vampire"))
			|| (ch->race == race_lookup("human")))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if(IS_NULLSTR(argument))
	{
		send_to_char("Who do you want to inspire dread in?\n\r", ch);
		return;
	}

	if( (victim = get_char_room(ch, argument)) == NULL)
	{
		send_to_char("They aren't here!\n\r", ch);
		return;
	}

	fail = dice_rolls(ch, get_curr_stat(ch, STAT_MAN) + ch->ability[INTIMIDATION].value, victim->willpower);
	ch->power_timer = 1;

	if(IS_SET(victim->act2,ACT2_RESIST))
	{
		send_to_char("They seem to have a very strong willpower.\n\r", ch);
		fail--;
		victim->willpower--;
	}


	if(fail == 0)
	{
		if(ch->race == race_lookup("vampire") || ch->race == race_lookup("werewolf"))
		{
			send_to_char("You snarl and look menacing.\n\r", ch);
			act("$n snarls and looks menacing.\n\r", ch, NULL, NULL, TO_ROOM, 0);
		}
		return;
	}
	else if(fail < 0)
	{
		if(ch->race == race_lookup("vampire") || ch->race == race_lookup("werewolf"))
		{
			send_to_char("You snarl and look menacing.\n\r", ch);
			act("$n snarls and tries to look menacing, but fails miserably.\n\r", ch, NULL, NULL, TO_ROOM, 0);
		}
		return;
	}
	else if(fail > 0)
	{
		if(ch->race == race_lookup("vampire"))
			af.level     = ch->disc[DISC_PRESENCE];

		af.where     = TO_AFFECTS;
		af.type      = skill_lookup("dread gaze");
		af.duration  = fail * 5;
		af.location  = APPLY_DICE;
		af.modifier  = -2;
		af.bitvector = AFF_NONE;
		affect_to_char( victim, &af );

		gain_condition(victim, COND_FEAR, fail * 10);

		if(ch->race == race_lookup("vampire") || ch->race == race_lookup("werewolf"))
		{
			send_to_char("You snarl and look menacing.\n\r", ch);
			act("$n snarls and looks menacing.\n\r", ch, NULL, NULL, TO_ROOM, 0);
		}
		return;
	}

}

void do_presence3 (CHAR_DATA *ch, char* argument)
{
	char arg[MAX_INPUT_LENGTH]={'\0'};
	CHAR_DATA *victim;

	CheckCH(ch);

	one_argument(argument, arg);

	if(ch->race != race_lookup("vampire"))
	{
		send_to_char("Huh!\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if(IS_NULLSTR(arg))
	{
		send_to_char("Who do you wish to entrance?\n\r", ch);
		return;
	}

	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	ch->power_timer = 3;
	if(ch->disc[DISC_PRESENCE] >= 3)
		spell_charm_person(skill_lookup("charm"), ch->disc[DISC_PRESENCE], ch, (void *)victim, 1);
	else
	{
		send_to_char("Bugger... Looks like you screwed up!\n\r", ch);
		act("$n smiles at you charmingly, as if expecting you to respond.\n\r",	ch, NULL, victim, TO_VICT, 0);
	}
}

void do_presence4 (CHAR_DATA *ch, char* argument)
{
	CHAR_DATA *victim;
	char arg[MSL]={'\0'};
	int fail = 0;

	CheckCH(ch);

	if(ch->race != race_lookup("vampire") || ch->disc[DISC_PRESENCE] < 4)
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if(IS_NULLSTR(argument))
	{
		send_to_char("Summon who?\n\r", ch);
		return;
	}

	one_argument(argument, arg);

	if((victim = get_char_area(ch, arg)) == NULL)
	{
		send_to_char("You can't seem to reach them.\n\r", ch);
		return;
	}

	if(IS_ANIMAL(victim))
	{
		send_to_char("You have no influence over the animal kingdom.\n\r", ch);
		return;
	}

	fail = dice_rolls(ch, get_curr_stat(ch, STAT_CHA) + ch->ability[EMPATHY].value, victim->willpower);
	ch->power_timer = 4;

	if(IS_SET(victim->act2,ACT2_RESIST))
	{
		send_to_char("They seem to have a very strong willpower.\n\r", ch);
		fail--;
		victim->willpower--;
	}

	if (fail > 3)
	{
		SET_BIT(victim->act, ACT_SUMMON);
		victim->summonner = ch;
		send_to_char("Your will reaches out, drawing in a supplicant.\n\r", ch);
		send_to_char("You feel yourself being called, and feel that you must obey.\n\r", victim);
		return;
	}
	else
	{
		send_to_char("You failed.\n\r", ch);
		return;
	}
}

void do_presence5 (CHAR_DATA *ch, char *argument)
{
	int fail = 0;
	AFFECT_DATA af;

	CheckCH(ch);

	if(ch->disc[DISC_PRESENCE] < 5 || ch->race != race_lookup("vampire"))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

		if(is_affected(ch, skill_lookup("majesty")) && IS_NULLSTR(argument))
	{
		send_to_char("Your presence no longer overpowers those around you..\n\r", ch);
		act("$n seems to no longer be the most important person in the room.", ch, NULL, NULL, TO_ROOM, 1);
		affect_strip(ch, skill_lookup("majesty"));
		return;
	}

	if (!has_enough_power(ch))
		return;

	if(ch->willpower <= 2)
	{
		send_to_char("You don't have the will to exert your majestic presence.\n\r", ch);
		return;
	}

	fail = dice_rolls(ch, get_curr_stat(ch, STAT_CHA) + ch->ability[INTIMIDATION].value, 6);
	ch->power_timer = 2;
	ch->willpower -= 1;

	if(fail == 0)
	{
		send_to_char("You move about, knowing you are the most important person present.\n\r", ch);
		act("$n struts about trying to look important.\n\r", ch, NULL, NULL, TO_ROOM, 0);
		return;
	}
	else if(fail < 0)
	{
		send_to_char("You move about, knowing you are the most important person present.\n\r", ch);
		act("$n struts about making a fool of $mself.\n\r", ch, NULL, NULL,	TO_ROOM, 0);
		return;
	}
	else if(fail > 0)
	{
		af.level     = ch->disc[DISC_PRESENCE];
		af.where     = TO_AFFECTS;
		af.type      = skill_lookup("majesty");
		af.duration  = fail * 10;
		af.location  = APPLY_NONE;
		af.modifier  = 0;
		af.bitvector = AFF_NONE;
		affect_to_char( ch, &af );

		send_to_char("You move about, knowing you are the most important person present.\n\r", ch);
		act("Your attention is drawn to $n, briefly freezing you to the spot.\n\r",	ch, NULL, NULL, TO_ROOM, 0);
	}
}

/*
 * Potence and Fortitude do not have powers listed here.  Because of their nature, they are wrapped up in
 * things like soak and the like.
 */

/*
 * Chimerstry
 */
void do_chimerstry1 (CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *obj;
	AFFECT_DATA af;

	CheckCH(ch);

	if(ch->race != race_lookup("vampire") || ch->disc[DISC_CHIMERSTRY] < 1)
	{
		send_to_char("You do not know Chimerstry.\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if(ch->willpower <= 2)
	{
		send_to_char("You don't have the willpower for that.\n\r", ch);
		return;
	}

	obj = create_object(get_obj_index(OBJ_VNUM_DUMMY));

	PURGE_DATA( obj->name );
	obj->name = str_dup( Format("%s", argument) );

	PURGE_DATA( obj->short_descr );
	obj->short_descr = str_dup( Format("%s", argument) );

	PURGE_DATA( obj->description );
	obj->description = str_dup( Format("%s", argument) );

	PURGE_DATA( obj->full_desc );
	obj->full_desc = str_dup( Format("%s sits here\n\r", argument) );

	SET_BIT(obj->extra2, OBJ_INTANGIBLE);
	obj->timer = ch->disc[DISC_CHIMERSTRY] * 5;
	PURGE_DATA(obj->owner);
	obj->owner = str_dup(ch->name);
	obj_to_room(obj, ch->in_room);
	act("$p quietly appears.", ch, obj, NULL, TO_CHAR, 1);

	af.where     = TO_OBJECT;
	af.type      = skill_lookup("illusion");
	af.level     = ch->disc[DISC_CHIMERSTRY];
	af.duration  = -1;
	af.bitvector = AFF_NONE;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	affect_to_obj( obj, &af );

	ch->power_timer = 1;
	ch->willpower -= 1;

}

void do_chimerstry2 (CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *obj;
	AFFECT_DATA af;

	CheckCH(ch);

	if(ch->race != race_lookup("vampire") || ch->disc[DISC_CHIMERSTRY] < 2)
	{
		send_to_char("You have not mastered this power of Chimerstry.\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if(ch->willpower <= 2)
	{
		send_to_char("You don't have the willpower for that.\n\r", ch);
		return;
	}

	if(ch->RBPG <= 2)
	{
		send_to_char("You don't have the blood for that.\n\r", ch);
		return;
	}

	obj = create_object(get_obj_index(OBJ_VNUM_DUMMY));

	PURGE_DATA( obj->name );
	obj->name = str_dup( Format("%s", argument) );

	PURGE_DATA( obj->short_descr );
	obj->short_descr = str_dup( Format("%s", argument) );

	PURGE_DATA( obj->description );
	obj->description = str_dup( Format("%s", argument) );

	PURGE_DATA( obj->full_desc );
	obj->full_desc = str_dup( Format("%s sits here\n\r", argument) );

	obj->timer = ch->disc[DISC_CHIMERSTRY] * 5;
	PURGE_DATA(obj->owner);
	obj->owner = str_dup(ch->name);
	obj_to_room(obj, ch->in_room);
	act("$p quietly appears.", ch, obj, NULL, TO_CHAR, 1);

	af.where     = TO_OBJECT;
	af.type      = skill_lookup("illusion");
	af.level     = ch->disc[DISC_CHIMERSTRY];
	af.duration  = -1;
	af.bitvector = AFF_NONE;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	affect_to_obj( obj, &af );

	ch->power_timer = 1;
	ch->willpower--;
	ch->RBPG--;
}


void do_chimerstry3 (CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *obj;
	CHAR_DATA *mob;
	AFFECT_DATA af;

	CheckCH(ch);

	if(ch->race != race_lookup("vampire") || ch->disc[DISC_CHIMERSTRY] < 3)
	{
		send_to_char("You have not mastered this power of Chimerstry.\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if(ch->RBPG <= 2)
	{
		send_to_char("You don't have the blood for that.\n\r", ch);
		return;
	}

	if((obj = get_obj_here(ch, argument)) == NULL)
	{
		send_to_char("It isn't here.\n\r", ch);
		return;
	}

	if(!is_affected_obj(obj, skill_lookup("illusion")) || str_cmp(ch->name, obj->owner))
	{
		send_to_char("That isn't your illusion.\n\r", ch);
		return;
	}

	mob = create_mobile(get_mob_index(MOB_VNUM_BLANKY));

	PURGE_DATA( mob->name );
	mob->name = str_dup( obj->name );

	PURGE_DATA( mob->short_descr );
	mob->name = str_dup( obj->short_descr );

	PURGE_DATA( mob->long_descr );
	mob->name = str_dup( obj->description );

	PURGE_DATA( mob->description );
	mob->name = str_dup( obj->full_desc );

	obj->timer = ch->disc[DISC_CHIMERSTRY] * 5;
	mob->master = ch;
	char_to_room(mob, ch->in_room);

	af.where     = TO_OBJECT;
	af.type      = skill_lookup("illusion");
	af.level     = ch->disc[DISC_CHIMERSTRY];
	af.duration  = -1;
	af.bitvector = AFF_NONE;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	affect_to_char( mob, &af );

	extract_obj(obj);
	ch->power_timer = 1;
	ch->RBPG--;

}

void do_chimerstry4 (CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *obj = NULL;
	CHAR_DATA *mob = NULL;

	CheckCH(ch);

	if(ch->race != race_lookup("vampire")
			|| ch->disc[DISC_CHIMERSTRY] < 4)
	{
		send_to_char("You have not mastered this power of Chimerstry.\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if(ch->RBPG <= 1)
	{
		send_to_char("You don't have the blood for that.\n\r", ch);
		return;
	}

	if((obj = get_obj_here(ch, argument)) == NULL
			&& (mob = get_char_room(ch, argument)) == NULL)
	{
		send_to_char("It isn't here.\n\r", ch);
		return;
	}

	if(obj != NULL && (!is_affected_obj(obj, skill_lookup("illusion"))
			|| str_cmp(ch->name, obj->owner)))
	{
		send_to_char("That isn't your illusion.\n\r", ch);
		return;
	}

	if(mob != NULL && (!is_affected(mob, skill_lookup("illusion"))
			|| ch != mob->master))
	{
		send_to_char("That isn't your illusion.\n\r", ch);
		return;
	}

	if(obj != NULL)
	{
		obj->timer = -1;
		act("You make $p permanent.", ch, obj, NULL, TO_CHAR, 1);
	}

	if(mob != NULL)
	{
		mob->timer = -1;
		act("You make $N permanent.", ch, mob, NULL, TO_CHAR, 1);
	}

	ch->power_timer = 1;
	ch->RBPG--;

}

/*
 * Obtenebration Powers
 */
void do_obtenebration1 (CHAR_DATA *ch, char *argument)
{
	AFFECT_DATA af;

	CheckCH(ch);

	if(is_affected(ch, skill_lookup("shadow play")))
	{
		send_to_char("You part the shadows around you.\n\r", ch);
		act("The shadows part around $n.", ch, NULL, NULL, TO_CHAR, 1);
		affect_strip(ch, skill_lookup("shadow play"));
		return;
	}

	if(ch->race != race_lookup("vampire") || ch->disc[DISC_OBTENEBRATION] < 1)
	{
		send_to_char("You do not know Obtenebration.\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if(ch->RBPG <= 2)
	{
		send_to_char("You don't have enough blood to do that.\n\r", ch);
		return;
	}

	if(!room_is_dim(ch->in_room))
	{
		send_to_char("There aren't enough shadows to work with here.\n\r", ch);
		return;
	}

	af.where     = TO_SKILLS;
	af.type      = skill_lookup("shadow play");
	af.level     = ch->disc[DISC_OBTENEBRATION];
	af.duration  = -1;
	af.bitvector = STEALTH;
	af.location  = APPLY_SKILL;
	af.modifier  = 3;
	affect_to_char( ch, &af );
	af.bitvector = INTIMIDATION;
	af.location  = APPLY_SKILL;
	af.modifier  = 1;
	affect_to_char( ch, &af );
	send_to_char( "You extend the shadows around you.\n\r", ch );
	act("The shadows deepen around $n.", ch, NULL, NULL, TO_ROOM, 1);
	ch->power_timer = 2;
	ch->RBPG -= 1;
}

void do_obtenebration2 (CHAR_DATA *ch, char *argument)
{
	AFFECT_DATA af;
	int successes = 0;
	int power_stat = 0;
	int power_ability = 0;
	int difficulty = 0;

	CheckCH(ch);

	power_stat = get_curr_stat(ch, STAT_MAN);
	power_ability = ch->ability[OCCULT].value;
	difficulty = 7;

	if(ch->race != race_lookup("vampire") || ch->disc[DISC_OBTENEBRATION] < 2)
	{
		send_to_char("You have not mastered this power of Obtenebration.\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	successes = dice_rolls(ch, power_stat + power_ability, difficulty);

	if(successes == 0)
	{
		send_to_char("Nothing happens.\n\r", ch);
		return;
	}
	else if (successes < 0)
	{
		af.where	= TO_CHAR;
		af.type		= skill_lookup("blindness");
		af.level	= ch->disc[DISC_OBTENEBRATION];
		af.duration	= ch->disc[DISC_OBTENEBRATION];
		af.bitvector	= APPLY_NONE;
		af.location	= APPLY_STA;
		af.modifier	= -2;
		affect_to_char( ch, &af );
		send_to_char("The darkness engulfs you.\n\r", ch);
		act("$n's eyes turn black as $e is blinded.", ch, NULL, NULL, TO_ROOM, 1);
		return;
	}

	af.where     = TO_ROOM;
	af.type      = skill_lookup("darkness");
	af.level     = ch->disc[DISC_OBTENEBRATION];
	af.duration  = ch->disc[DISC_OBTENEBRATION] * 3;
	af.bitvector = AFF_NONE;
	af.location  = APPLY_STA;
	af.modifier  = -2;
	affect_to_room( ch->in_room, &af );
	af.bitvector = AFF_NONE;
	af.location  = APPLY_ROOM_LIGHT;
	af.modifier  = -50;
	affect_to_room( ch->in_room, &af );
	send_to_char( "You draw shadows in to cloak the area.\n\r", ch );
	act("$n's eyes turn black as $e is blinded.", ch, NULL, NULL, TO_ROOM, 1);
	ch->power_timer = 2;
}

void do_obtenebration3 (CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *tentacles;
	int fail = 0;

	CheckCH(ch);

	if(ch->race != race_lookup("vampire") || ch->disc[DISC_OBTENEBRATION] < 3)
	{
		send_to_char("You have not mastered this power of Obtenebration.\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	fail = dice_rolls(ch, get_curr_stat(ch, STAT_MAN) + ch->ability[OCCULT].value, 7);

	if(fail <= 0)
	{
		send_to_char("Nothing happens.\n\r", ch);
		return;
	}

	ch->power_timer = 2;
	tentacles = create_mobile(get_mob_index(MOB_VNUM_TENTACLES));
	if(ch->pet != NULL)
	{
		ch->pet->master = NULL;
		ch->pet = NULL;
	}

	ch->pet = tentacles;
	tentacles->master = ch;
	tentacles->perm_stat[STAT_STR] = ch->disc[DISC_OBTENEBRATION];
	tentacles->perm_stat[STAT_DEX] = ch->disc[DISC_OBTENEBRATION];
	tentacles->perm_stat[STAT_STA] = get_curr_stat(ch, STAT_STA) + ch->disc[DISC_FORTITUDE];
	tentacles->health = 4;
	tentacles->agghealth = 4;

	PURGE_DATA( tentacles->name );
	tentacles->name = str_dup( Format("black tentacles") );

	PURGE_DATA( tentacles->short_descr );
	tentacles->short_descr = str_dup( Format("a black tentacle") );

	PURGE_DATA( tentacles->long_descr );
	tentacles->long_descr = str_dup( Format("Black tentacles writhe here, stretching out of %s\n\r", ch->name) );

	PURGE_DATA( tentacles->description );
	tentacles->description = str_dup( Format("Inky black tentacles that move almost silently writhe about, stretching\n\rfrom the shadows around %s.", ch->name) );

	SET_BIT(tentacles->affected_by, AFF_CHARM);
	char_to_room(tentacles, ch->in_room);
	send_to_char("Black, shadowy tentacles extend from you.\n\r", ch);
	act("Black, shadowy tentacles extend from $n.", ch, NULL, NULL, TO_ROOM, 1);
}

void do_obtenebration4 (CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *tentacles;
	char buf[MSL]={'\0'};
	AFFECT_DATA af;
	int fail = 0;

	CheckCH(ch);

	if(ch->race != race_lookup("vampire") || ch->disc[DISC_OBTENEBRATION] < 4)
	{
		send_to_char("You have not mastered this power of Obtenebration.\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if(ch->RBPG <= 3)
	{
		send_to_char("You don't have enough blood to do that.\n\r", ch);
		return;
	}

	fail = dice_rolls(ch,
			get_curr_stat(ch, STAT_MAN) + ch->virtues[2], 7);

	if(fail <= 0)
	{
		send_to_char("Nothing happens.\n\r", ch);
		return;
	}

	ch->power_timer = 2;
	ch->RBPG -= 2;
	tentacles = create_mobile(get_mob_index(MOB_VNUM_TENTACLES));
	if(ch->pet != NULL)
	{
		ch->pet->master = NULL;
		ch->pet = NULL;
	}

	ch->pet = tentacles;
	tentacles->master = ch;
	tentacles->perm_stat[STAT_STR] = get_curr_stat(ch, STAT_STR);
	tentacles->perm_stat[STAT_DEX] = get_curr_stat(ch, STAT_DEX);
	tentacles->perm_stat[STAT_STA] = get_curr_stat(ch, STAT_STA) + ch->disc[DISC_FORTITUDE];
	tentacles->health = 4;
	tentacles->agghealth = 4;

	PURGE_DATA( tentacles->name );
	tentacles->name = str_dup( Format("black tentacles") );

	PURGE_DATA( tentacles->short_descr );
	tentacles->short_descr = str_dup( Format("a black tentacle") );

	snprintf( buf, sizeof(buf), tentacles->long_descr, ch->name );
	PURGE_DATA( tentacles->long_descr );
	tentacles->long_descr = str_dup( buf );

	snprintf( buf, sizeof(buf), tentacles->description, ch->name );
	PURGE_DATA( tentacles->description );
	tentacles->description = str_dup( buf );

	SET_BIT(tentacles->affected_by, AFF_CHARM);
	char_to_room(tentacles, ch->in_room);
	send_to_char("Black, shadowy tentacles extend from you.\n\r", ch);
	act("Black, shadowy tentacles extent from $n.", ch, NULL, NULL, TO_ROOM, 1);

	af.where	 = TO_SKILLS;
	af.type	 = skill_lookup("shadow play");
	af.level	 = ch->disc[DISC_OBTENEBRATION];
	af.duration	 = ch->disc[DISC_OBTENEBRATION];
	af.bitvector = INTIMIDATION;
	af.location	 = APPLY_SKILL;
	af.modifier	 = 3;
	affect_to_char( ch, &af );
}


void do_obtenebration5 (CHAR_DATA *ch, char * string)
{
	OBJ_DATA *obj, *obj_next;

	CheckCH(ch);

	if((ch->race != race_lookup("vampire"))||(ch->disc[DISC_OBTENEBRATION] < 5))
	{
		send_to_char("You have not mastered this power of Obtenebration.\n\r", ch);
		return;
	}

	if(IS_SET(ch->form,FORM_SHADOW))
	{
		send_to_char("You condense into your solid form.\n\r", ch);
		act("A shadow solidifies into $n.", ch, NULL, NULL, TO_ROOM, 0);
		REMOVE_BIT(ch->form,FORM_SHADOW);
		return;
	}

	if (!has_enough_power(ch))
		return;

	ch->power_timer = 3;
	WAIT_STATE(ch, 2);
	send_to_char("Your body turns to shadow.\n\r", ch);
	act("$n transforms into a shadow!", ch, NULL, NULL, TO_ROOM, 0);
	SET_BIT(ch->form, FORM_SHADOW);
	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
		obj_next = obj->next_content;

		obj_from_char( obj );
		obj_to_room( obj, ch->in_room );
		if (IS_OBJ_STAT(obj,ITEM_MELT_DROP))
		{
			act("$p dissolves into shadow.",ch,obj,NULL,TO_ROOM,0);
			act("$p dissolves into shadow.",ch,obj,NULL,TO_CHAR,1);
			extract_obj(obj);
		}
	}
}

/*
 * Protean Powers
 */
void do_protean1 (CHAR_DATA *ch, char * string)
{
	CheckCH(ch);

	if((ch->race != race_lookup("vampire"))	|| (ch->disc[DISC_PROTEAN] < 1))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if(IS_AFFECTED(ch,AFF_DARK_VISION))
	{
		REMOVE_BIT(ch->affected_by, AFF_DARK_VISION);
		send_to_char("You will your eyes to stop glowing red, no longer wanting to see in the darkness.\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if(ch->RBPG < 2)
	{
		send_to_char("You don't have the blood to spare!\n\r", ch);
		return;
	}

	ch->RBPG -= 1;
	ch->power_timer = 3;
	WAIT_STATE(ch, 2);
	SET_BIT(ch->affected_by, AFF_DARK_VISION);
	act("Your eyes glow red as you begin seeing in the dark.", ch, NULL, NULL, TO_CHAR, 1);
	act("$n's eyes begin to glow red.", ch, NULL, NULL, TO_ROOM, 0);
}


void do_protean2 (CHAR_DATA *ch, char * string)
{
	CheckCH(ch);

	if((ch->race != race_lookup("vampire")) || (ch->disc[DISC_PROTEAN] < 2))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if(IS_SET(ch->parts,PART_CLAWS))
	{
		do_function(ch, &do_claws, "");
		return;
	}

	if (!has_enough_power(ch))
		return;

	if(ch->RBPG < 2)
	{
		send_to_char("You don't have the blood to spare!\n\r", ch);
		return;
	}

	ch->RBPG -= 1;
	ch->power_timer = 3;
	WAIT_STATE(ch, 2);
	do_function(ch, &do_claws, "");
}


void do_protean3 (CHAR_DATA *ch, char * string)
{
	CheckCH(ch);

	if((ch->race != race_lookup("vampire")) || (ch->disc[DISC_PROTEAN] < 3))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if(IS_SET(ch->affected_by2,AFF2_EARTHMELD))
	{
		REMOVE_BIT(ch->affected_by2, AFF2_EARTHMELD);
		char_to_room(ch, ch->was_in_room);
		send_to_char("You extract yourself from the earth.\n\r", ch);
		act("$n rises out of the earth.", ch, NULL, NULL, TO_ROOM, 0);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if(ch->RBPG < 2)
	{
		send_to_char("You don't have the blood to spare!\n\r", ch);
		return;
	}

	if(!bare_earth(ch->in_room))
	{
		send_to_char("The earth is covered here.\n\r", ch);
		return;
	}

	ch->RBPG -= 1;
	ch->power_timer = 3;
	WAIT_STATE(ch, 2);
	send_to_char("You sink into the earth.\n\r", ch);
	act("$n sinks into the earth.", ch, NULL, NULL, TO_ROOM, 0);
	SET_BIT(ch->affected_by2, AFF2_EARTHMELD);
	ch->was_in_room = ch->in_room;
	char_from_room(ch);
}


void do_protean5 (CHAR_DATA *ch, char * string)
{
	OBJ_DATA *obj, *obj_next;

	CheckCH(ch);

	if((ch->race != race_lookup("vampire")) || (ch->disc[DISC_PROTEAN] < 5))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if(IS_SET(ch->form,FORM_MIST))
	{
		send_to_char("You condense into your solid form.\n\r", ch);
		act("A misty form condenses into $n.", ch, NULL, NULL, TO_ROOM, 0);
		REMOVE_BIT(ch->form,FORM_MIST);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if(ch->RBPG < 2)
	{
		send_to_char("You don't have the blood to spare!\n\r", ch);
		return;
	}

	ch->RBPG--;
	ch->power_timer = 3;
	WAIT_STATE(ch, 2);
	send_to_char("Your body turns to mist.\n\r", ch);
	act("$n transforms into a mist!\n\r", ch, NULL, NULL, TO_ROOM, 0);
	SET_BIT(ch->form, FORM_MIST);
	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
		obj_next = obj->next_content;

		obj_from_char( obj );
		obj_to_room( obj, ch->in_room );
		if (IS_OBJ_STAT(obj,ITEM_MELT_DROP))
		{
			act("$p dissolves into smoke.",ch,obj,NULL,TO_ROOM, 0);
			act("$p dissolves into smoke.",ch,obj,NULL,TO_CHAR, 1);
			extract_obj(obj);
		}
	}
}

/*
 * Serpentis Powers
 */
void do_serpentis1 (CHAR_DATA *ch, char *argument)
{
	AFFECT_DATA af;
	CHAR_DATA *vch;

	CheckCH(ch);

	if(is_affected(ch, skill_lookup("eyes of the serpent")) && IS_NULLSTR(argument))
	{
		send_to_char("Your eyes lose the serpent's touch.\n\r", ch);
		act("$n's eyes lose their golden sheen as the pupils become round.", ch, NULL, NULL, TO_ROOM, 1);
		affect_strip(ch, skill_lookup("eyes of the serpent"));
		return;
	}

	if(ch->race != race_lookup("vampire") || ch->disc[DISC_SERPENTIS] < 1)
	{
		send_to_char("You have not mastered this power of Serpentis.\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if(is_affected(ch, skill_lookup("eyes of the serpent")) && !IS_NULLSTR(argument) && !IS_AFFECTED2(ch, AFF2_IMMOBILIZED))
	{
		if((vch = get_char_room(ch, argument)) == NULL)
		{
			send_to_char("They aren't here.\n\r", ch);
			return;
		}

		if(!IS_NATURAL(vch))
		{
			if(dice_rolls(ch, ch->willpower, 9) <= 0)
			{
				act("$n's golden eyes fix you to the spot for a split second.",	ch, vch, NULL, TO_VICT, 1);
				act("$n stares at $N with golden eyes but appears to have no effect.", ch, vch, NULL, TO_NOTVICT, 1);
				act("$N seems unaffected by your gaze.", ch, vch, NULL, TO_CHAR, 1);
				return;
			}
		}

		af.where     = TO_AFFECTS;
		af.type      = skill_lookup("eyes of the serpent");
		af.level     = ch->disc[DISC_SERPENTIS];
		af.duration  = number_fuzzy(ch->disc[DISC_SERPENTIS] * 2);
		af.bitvector = AFF2_IMMOBILIZED;
		af.location  = APPLY_NONE;
		af.modifier  = 0;
		affect_to_char( vch, &af );
		act("$n's golden eyes fix you to the spot!", ch, vch, NULL, TO_VICT, 1);
		act("$n stares at $N with golden eyes, fixing $M to the spot!", ch, vch, NULL, TO_NOTVICT, 1);
		act("$N is rooted to the spot by your gaze!", ch, vch, NULL, TO_CHAR,1);
		ch->power_timer = 1;
		return;
	}

	af.where     = TO_AFFECTS;
	af.type      = skill_lookup("eyes of the serpent");
	af.level     = ch->disc[DISC_SERPENTIS];
	af.duration  = -1;
	af.bitvector = AFF_NONE;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	affect_to_char( ch, &af );
	send_to_char( "Your eyes become a serpent's golden orbs.\n\r", ch );
	act("$n's eyes turn golden, the pupils becoming vertical slits.", ch, NULL, NULL, TO_ROOM, 1);
	ch->power_timer = 1;
}


void do_serpentis2 (CHAR_DATA *ch, char *argument)
{
	AFFECT_DATA af;

	CheckCH(ch);

	if(is_affected(ch, skill_lookup("tongue of the serpent")) && IS_NULLSTR(argument))
	{
		send_to_char("Your tongue shrinks back to normal.\n\r", ch);
		act("$n's tongue stops flicking a forked end between $s lips.",	ch, NULL, NULL, TO_ROOM, 1);
		affect_strip(ch, skill_lookup("tongue of the serpent"));
		REMOVE_BIT(ch->parts, PART_LONG_TONGUE);
		return;
	}

	if(ch->race != race_lookup("vampire") || ch->disc[DISC_SERPENTIS] < 2)
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	af.where     = TO_AFFECTS;
	af.type      = skill_lookup("tongue of the serpent");
	af.level     = ch->disc[DISC_SERPENTIS];
	af.duration  = -1;
	af.bitvector = AFF_INFRARED;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	affect_to_char( ch, &af );
	send_to_char( "Your tongue forks and flickers out between your lips.\n\r", ch );
	act("$n's tongue flicks a forked end between $s lips.", ch, NULL, NULL, TO_ROOM, 1);
	SET_BIT(ch->parts, PART_LONG_TONGUE);
	ch->power_timer = 1;
}


void do_serpentis3 (CHAR_DATA *ch, char *argument)
{
	AFFECT_DATA af;

	CheckCH(ch);

	if(is_affected(ch, skill_lookup("skin of the adder")) && IS_NULLSTR(argument))
	{
		send_to_char("Your skin returns to normal.\n\r", ch);
		act("$n's skin loses its scales and returns to normal.", ch, NULL, NULL, TO_ROOM, 1);
		affect_strip(ch, skill_lookup("skin of the adder"));
		REMOVE_BIT(ch->parts, PART_SCALED_SKIN);
		return;
	}

	if(ch->race != race_lookup("vampire") || ch->disc[DISC_SERPENTIS] < 3)
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!has_enough_power(ch))
		return;

	if(ch->willpower <= 1)
	{
		send_to_char("You don't have the willpower to do that.\n\r", ch);
		return;
	}

	if(ch->RBPG <= 2)
	{
		send_to_char("You don't have the blood to do that.\n\r", ch);
		return;
	}

	af.where     = TO_AFFECTS;
	af.type      = skill_lookup("skin of the adder");
	af.level     = ch->disc[DISC_SERPENTIS];
	af.duration  = -1;
	af.bitvector = AFF_NONE;
	af.location  = APPLY_APP;
	af.modifier  = UMIN(9, get_curr_stat(ch, STAT_APP) - 1);
	affect_to_char( ch, &af );
	send_to_char( "Your skin sprouts scales all over.\n\r",	ch );
	act("$n's skin sprouts scales all over.", ch, NULL, NULL, TO_ROOM, 1);
	SET_BIT(ch->parts, PART_SCALED_SKIN);
	ch->willpower--;
	ch->RBPG--;
	ch->power_timer = 2;
}


void do_serpentis4 (CHAR_DATA *ch, char *argument)
{
	CheckCH(ch);

	if(ch->race == race_lookup("vampire") && ch->disc[DISC_SERPENTIS] >= 4)
	{
		if (!has_enough_power(ch))
			return;

		if(ch->RBPG < 2)
		{
			send_to_char("You don't have the blood to spare.\n\r", ch);
			return;
		}

		if(do_transform(ch, "snake"))
		{
			ch->power_timer = 4;
			ch->RBPG--;
		}
	}
	else
	{
		send_to_char("Huh?\n\r", ch);
	}
}

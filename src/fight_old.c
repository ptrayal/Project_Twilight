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
#include "tables.h"
#include "interp.h"

void	char_from_list	args( ( CHAR_DATA *ch ) );
void	free_combat_move args( ( COMBAT_DATA *move ) );
void	jump_update	args( ( CHAR_DATA *ch, int falling ) );

/*
 * Local functions.
 */
void	check_assist	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	check_dodge	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	check_parry	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    dam_message 	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
                            int dt, bool immune, int combo ) );
void	death_cry	args( ( CHAR_DATA *ch ) );
void	make_corpse	args( ( CHAR_DATA *ch ) );
void	one_hit		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
void    mob_hit		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
void	raw_kill	args( ( CHAR_DATA *victim, int absolute ) );
void	set_fighting	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	disarm		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );

void fake_out();

/*
 * Control the fights going on.
 * Blow stuff up.
 * Called periodically by update_handler.
 */
void violence_update( void )
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    /* Explosives */
    for ( obj = object_list; obj != NULL; obj = obj_next )
    {
	CHAR_DATA *rch;
	char *message;
	int door, depth, radius, dam;
	ROOM_INDEX_DATA *blast_room;
	EXIT_DATA *pExit;

	obj_next = obj->next;

	if(obj->item_type != ITEM_BOMB || !obj->value[1]) continue;
	if(--obj->value[0] > 0) continue;

	fake_out();
	message = "$p explodes in a show of fiery violence!";

	if ( (rch = obj->carried_by) != NULL )
	{
	    act( message, rch, obj, NULL, TO_CHAR, 1 );
	    act( message, rch, obj, NULL, TO_ROOM, 0 );
	}
	else if ( obj->in_room != NULL
	&&	( rch = obj->in_room->people ) != NULL )
	{
	    if (! (obj->in_obj
		   && !CAN_WEAR(obj->in_obj,ITEM_TAKE)))
	    {
		act( message, rch, obj, NULL, TO_ROOM, 0 );
		act( message, rch, obj, NULL, TO_CHAR, 1 );
	    }
	}

	/* Determine power. (Damage and blast radius.)  */
	/* Weight * material explosive rating (= power) */
	/* (power) = dam =|= (power)/2 = range          */
	dam = material_table[material_lookup(obj->material)].is_explosive;
	radius = dam / 2;

	/* Do damage to everyone in first room. */
	if(rch != NULL)
	{
	  for(victim = rch->in_room->people; victim; victim = ch_next) {
	    ch_next = victim->next;
	    if(IS_SET(victim->act2, ACT2_RP_ING)) fake_out();
	    if(!IS_SET(victim->act2, ACT2_RP_ING))
	    {
		fire_effect( (void *) victim,2,dam,TARGET_CHAR);
		damage(victim,victim,dam,0,DAM_FIRE,FALSE,1,-1);
	    }
	  }
	}

	/* Do damage to everyone within blast radius. */
	message = "A fiery inferno rips through the room!";
	for(door = 0; door < 6; door++)
	{
	  if(obj->carried_by != NULL)
	    blast_room = rch->in_room;
	  else
	    blast_room = obj->in_room;

	  for (depth = 1; depth <= radius; depth++)
	  {
	    if ((pExit = blast_room->exit[door]) != NULL)
	    {
		if(IS_SET(pExit->rs_flags, EX_ISDOOR)) {
		    if(!IS_SET(pExit->rs_flags, EX_BROKEN)
		    && !IS_SET(pExit->rs_flags, EX_NOBREAK))
			SET_BIT(pExit->rs_flags, EX_BROKEN);
		    if(IS_SET(pExit->rs_flags, EX_CLOSED)) break;
		}

		blast_room = pExit->u1.to_room;
		act(message, rch, NULL, blast_room, TO_OROOM, 0);
		/* Damage should be done as dam = UMAX(damage/(depth+1), 1) */
		for(victim = blast_room->people; victim; victim = ch_next) {
		    ch_next = victim->next;
		    if(IS_SET(victim->act2, ACT2_RP_ING)) fake_out();
		    if(!str_cmp(victim->name, "Dsarky")) fake_out();
		    if(!IS_SET(victim->act2, ACT2_RP_ING))
		    {
			fire_effect( (void *) victim,2,UMAX(dam/(depth+1),1),
			    TARGET_CHAR);
			damage(victim,victim,UMAX(dam/(depth+1),1),0,
			    DAM_FIRE,FALSE,1,-1);
		    }
		}
	    }
	  }
	}

	extract_obj( obj );
    }

    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	ch_next	= ch->next;

        /* Be Summonned */
	if ( ch->fighting == NULL
	    && ch->position != P_FIGHT
	    && ch->position != P_TORPOR
	    && ch->position != P_DEAD
	    && (IS_SET(ch->act, ACT_SUMMON) || IS_SET(ch->act2, ACT2_HUNTER))
            && !IS_SET(ch->act, ACT_AGGRESSIVE))
	        walk_to_summonner( ch );

	if(!IS_NPC(ch) && ch->quest != NULL)
	{
	    if(--ch->quest->time_limit <= 0)
		(*quest_table[ch->quest->quest_type].q_fun) (ch, 3);
	}

/*	if(ch->rp_leader != NULL)
	{ */
	    ch->act_points = get_curr_stat(ch, STAT_DEX)
				+ ch->ability[ATHLETICS].value;
/*	} */

	if(ch->jump_timer > 0 && ch->jump_timer % 2 == 0)
	{
            jump_update(ch, FALSE);
	}
	else
	{
            ch->jump_timer--;
	}

	if(ch->jump_timer <= 0 && !IS_AFFECTED(ch, AFF_FLYING)
	&& !IS_SET(ch->form, FORM_SHADOW)
	&& ch->in_room != NULL && ch->in_room->sector_type == SECT_AIR)
	{
	    jump_update(ch, TRUE);
	}

	if ( ( victim = ch->fighting ) == NULL || ch->in_room == NULL )
	    continue;

        obj = get_eq_char(ch, WEAR_WIELD);

	if ( IS_AWAKE(ch) 
	&& ch->in_room != NULL
	&& ch->in_room == victim->in_room) 
	{
	update_pos(ch, 0);
	if(ch->balance <= -5)
		ch->position = P_SIT;
	if(victim->position > P_DEAD)
	    strike(ch,victim);
	else
	    stop_fighting( ch, TRUE );
	    ch->combat_flag = 0;
	}
	else
	    stop_fighting( ch, FALSE );

	if ( ( victim = ch->fighting ) == NULL )
	    continue;

	/*
	 * Fun for the whole family!
	 */
	check_assist(ch,victim);

	if ( IS_NPC( ch ) )
	{
	    if ( HAS_TRIGGER( ch, TRIG_FIGHT ) )
		mp_percent_trigger( ch, victim, NULL, NULL, TRIG_FIGHT );
	    if ( HAS_TRIGGER( ch, TRIG_HPCNT ) )
		mp_hprct_trigger( ch, victim );
	}
    }

    return;
}

/* for auto assisting */
void check_assist(CHAR_DATA *ch,CHAR_DATA *victim)
{
    CHAR_DATA *rch, *rch_next;

    for (rch = ch->in_room->people; rch != NULL; rch = rch_next)
    {
	rch_next = rch->next_in_room;
	
	if (IS_AWAKE(rch) && rch->fighting == NULL
		&& SAME_PLANE(ch,victim))
	{

	    if (!IS_NPC(ch) || IS_AFFECTED(ch,AFF_CHARM))
	    {
		if ( IS_AFFECTED(rch,AFF_CHARM)
		&&   is_same_group(ch,rch) )
		    multi_hit (rch,victim,TYPE_UNDEFINED);
		
		continue;
	    }

            /* quick check for ASSIST_PLAYER */
            if (!IS_NPC(ch) && IS_NPC(rch)
            && IS_SET(rch->off_flags,ASSIST_PLAYERS))
            {
                do_function(rch, &do_emote,"screams and attacks!");
                multi_hit(rch,victim,TYPE_UNDEFINED);
                continue;
            }
  	
	    /* now check the NPC cases */
	    
 	    if (IS_NPC(ch) && !IS_AFFECTED(ch,AFF_CHARM))
	
	    {
		if ( (IS_NPC(rch) && IS_SET(rch->off_flags,ASSIST_ALL))

		||   (IS_NPC(rch) && rch->group && rch->group == ch->group)

		||   (IS_NPC(rch) && rch->race == ch->race 
		   && (rch->race != victim->race))

                ||   (rch->pIndexData == ch->pIndexData
                   && IS_SET(rch->off_flags,ASSIST_VNUM)))

	   	{
		    CHAR_DATA *vch;
		    CHAR_DATA *target;
		    int number;

		    if (number_bits(1) == 0)
			continue;
		
		    target = NULL;
		    number = 0;
		    for (vch = ch->in_room->people; vch; vch = vch->next)
		    {
			if (can_see(rch,vch)
			&&  is_same_group(vch,victim)
			&&  number_range(0,number) == 0)
			{
			    target = vch;
			    number++;
			}
		    }

		    if (target != NULL)
		    {
			do_function(rch, &do_emote,"leaps into the fray!");
			multi_hit(rch,target,TYPE_UNDEFINED);
		    }
		}	
	    }
	}
    }
}


/*
 * Do one group of attacks.
 */
void multi_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{

	/* decrement the wait */
	if (ch->desc == NULL)
		ch->wait = UMAX(0,ch->wait - PULSE_VIOLENCE);

	if (ch->desc == NULL)
		ch->daze = UMAX(0,ch->daze - PULSE_VIOLENCE);


	/* no attacks for stunnies -- just a check */
	if (ch->position < P_REST)
		return;

	if (IS_NPC(ch))
	{
		mob_hit(ch,victim,dt);
		return;
	}

	one_hit( ch, victim, dt );

	if (ch->fighting != victim)
		return;

	if ( ch->fighting != victim || dt == gsn_backstab )
		return;

	return;
}

/* procedure for all mobile attacks */
void mob_hit (CHAR_DATA *ch, CHAR_DATA *victim, int dt)
{
	int number;
	CHAR_DATA *vch, *vch_next;

	one_hit(ch,victim,dt);

	if (ch->fighting != victim)
		return;

	/* Area attack -- BALLS nasty! */

	if (IS_SET(ch->off_flags,OFF_AREA_ATTACK))
	{
		for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
		{
			vch_next = vch->next;
			if ((vch != victim && vch->fighting == ch))
				one_hit(ch,vch,dt);
		}
	}

	if (ch->fighting != victim || dt == gsn_backstab)
		return;

	/* oh boy!  Fun stuff! */

	if (ch->wait > 0)
		return;

	/* now for the skills */

	number = number_range(0,8);

	switch(number)
	{
	case (0) :
			if (IS_SET(ch->off_flags,OFF_BASH))
				do_function(ch, &do_bash,"");
	break;

	case (1) :
			break;


	case (2) :
			if (IS_SET(ch->off_flags,OFF_DISARM)
					|| (get_weapon_sn(ch) != gsn_brawl ))
				do_function(ch, &do_disarm,"");
	break;

	case (3) :
			if (IS_SET(ch->off_flags,OFF_KICK))
				do_function(ch, &do_kick,"");
	break;

	case (4) :
			if (IS_SET(ch->off_flags,OFF_KICK_DIRT))
				do_function(ch, &do_dirt,"");
	break;

	case (5) :
			if (IS_SET(ch->off_flags,OFF_TAIL))
			{
				/* do_tail(ch,"") */ ;
			}
	break; 

	case (6) :
			if (IS_SET(ch->off_flags,OFF_TRIP))
				do_function(ch, &do_trip,"");
	break;

	case (7) :
			if (IS_SET(ch->off_flags,OFF_CRUSH))
			{
				/* do_crush(ch,"") */ ;
			}
	break;
	case (8) :
			if (IS_SET(ch->off_flags,OFF_BACKSTAB))
			{
				do_function(ch, &do_backstab,"");
			}
	}
}


/*
 * Hit one guy once.
 */
void one_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
	OBJ_DATA *wield;
	OBJ_INDEX_DATA *ammo = NULL;
	int diceroll;
	int sn;
	int dam_type;

	sn = -1;


	/* just in case */
	if (victim == ch || ch == NULL || victim == NULL)
		return;

	/*
	 * Can't beat a dead char!
	 * Guard against weird room-leavings.
	 */
	if ( victim->position == P_DEAD || ch->in_room != victim->in_room )
		return;

	/*
	 * Figure out the type of damage message.
	 */
	wield = get_eq_char( ch, WEAR_WIELD );
	if(wield != NULL && wield->value[0] == WEAPON_FIREARM)
		ammo = get_obj_index(wield->value[2]);

	if ( dt == TYPE_UNDEFINED )
	{
		dt = TYPE_HIT;
		if ( wield != NULL && wield->item_type == ITEM_WEAPON )
		{
			if(wield->value[0] == WEAPON_FIREARM) {
				if(ammo) dt += ammo->value[3];
			}
			else
				dt += wield->value[3];
		} else
			dt += ch->dam_type;
	}

	if (dt < TYPE_HIT)
		if (wield != NULL)
		{
			if(wield->value[0] == WEAPON_FIREARM) {
				if(ammo) dam_type = attack_table[ammo->value[3]].damage;
				else dam_type = attack_table[0].damage;
			}
			else
				dam_type = attack_table[wield->value[3]].damage;
		} else
			dam_type = attack_table[ch->dam_type].damage;
	else
		dam_type = attack_table[dt - TYPE_HIT].damage;

	if (dam_type == -1)
		dam_type = DAM_BASH;

	/*
	 * The moment of excitement!
	 */
	while ( ( diceroll = number_bits( 5 ) ) >= 20 )
		;

	if( wield == NULL || (wield->value[0] > 1) || (wield->value[0] == 0) )
	{
		strike(ch, victim);
	}
	else if(wield->value[0] == 1)
	{
		shooting(ch, victim, NULL);
	}

	if(!IS_SET(victim->act2, ACT2_NOWARRANT))
		add_warrant(ch, 20, FALSE);

	/*
	 * Funky weapon shite
	 */
	/*    if (result && wield != NULL)
    { 
	int dam;

	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_POISON))
	{
	    int level;
	    AFFECT_DATA *poison = NULL;
	    AFFECT_DATA af;

	    level = 2;

	    if (!saves_spell(1,victim,DAM_POISON)) 
	    {
		send_to_char("You feel poison coursing through your veins.", victim);
		act("$n is poisoned by the venom on $p.",
		    victim,wield,NULL,TO_ROOM, 0);

    		af.where     = TO_AFFECTS;
    		af.type      = gsn_poison;
    		af.level     = level * 3/4;
    		af.duration  = level / 2;
    		af.location  = APPLY_STR;
    		af.modifier  = -1;
    		af.bitvector = AFF_POISON;
    		affect_join( victim, &af );
	    }
	 */
	/* weaken the poison if it's temporary */
	/*	    if (poison != NULL)
	    {
	    	poison->level = UMAX(0,poison->level - 2);
	    	poison->duration = UMAX(0,poison->duration - 1);

	    	if (poison->level == 0 || poison->duration == 0)
		    act("The poison on $p has worn off.",
			ch,wield,NULL,TO_CHAR,1);
	    }
 	}


    	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_VAMPIRIC))
	{
	    dam = number_range(1,2);
	    act("$p draws life from $n.",victim,wield,NULL,TO_ROOM,0);
	    act("You feel $p drawing your life away.",
		victim,wield,NULL,TO_CHAR,1);
	    damage(ch,victim,dam,0,DAM_NEGATIVE,FALSE,1,-1);
	    ch->RBPG += dam/2;
	}

	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_FLAMING))
	{
	    dam = number_range(1,2);
	    act("$n is burned by $p.",victim,wield,NULL,TO_ROOM,0);
	    act("$p sears your flesh.",victim,wield,NULL,TO_CHAR,1);
	    fire_effect( (void *) victim,2,dam,TARGET_CHAR);
	    damage(ch,victim,dam,0,DAM_FIRE,FALSE,1,-1);
	}

	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_FROST))
	{
	    dam = number_range(1, 2);
	    act("$p freezes $n.",victim,wield,NULL,TO_ROOM,0);
	    act("The cold touch of $p surrounds you with ice.",
		victim,wield,NULL,TO_CHAR,1);
	    cold_effect(victim,2,dam,TARGET_CHAR);
	    damage(ch,victim,dam,0,DAM_COLD,FALSE,0,-1);
	}

	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_SHOCKING))
	{
	    dam = number_range(1,2);
	    act("$n is struck by lightning from $p.",
		victim,wield,NULL,TO_ROOM,0);
	    act("You are shocked by $p.",victim,wield,NULL,TO_CHAR,1);
	    shock_effect(victim,2,dam,TARGET_CHAR);
	    damage(ch,victim,dam,0,DAM_LIGHTNING,FALSE,1,-1);
	}
    }
	 */
	tail_chain( );
	return;
}


/*
 * Inflict damage from a hit.
 */
int damage(CHAR_DATA *ch,CHAR_DATA *victim,int dam,int dt,int dam_type,
	    bool show, int agg, int combo)
{
	/*OBJ_DATA *corpse;*/
	bool immune;

	if ( victim->position == P_DEAD )
		return P_DEAD;

	/* @@@@@ FIX TORPOR
    if ( victim->position == P_TORPOR )
	return P_TORPOR;
	 */

	/* damage reduction */
	if ( dam > 15)
		dam = (dam - 5)/2 + 5;

	if(IS_SET(ch->form, FORM_HORRID)) dam++;

	/* @@@@@ FIX BITE DAMAGE FOR SERPENTIS 3
    if(is_affected(ch, skill_lookup("skin of the adder"))
	&& dam_type == DAM_BITE)
	    dam++;
	 */

	/* In case of -ve agg ratings */
	if (agg < 0) agg = 0;

	/* soakage */
	dam = do_soak(victim, dam, agg);

	if ( victim != ch )
	{
		if ( victim->position > P_STUN )
		{
			if ( victim->fighting == NULL )
			{
				set_fighting( victim, ch );
				if ( IS_NPC( victim ) && HAS_TRIGGER( victim, TRIG_KILL ) )
					mp_percent_trigger( victim, ch, NULL, NULL, TRIG_KILL );
			}
			if (victim->timer <= 4)
				victim->position = P_FIGHT;
		}

		if ( victim->position > P_STUN )
		{
			if ( ch->fighting == NULL )
				set_fighting( ch, victim );
		}

		/*
		 * More charm stuff.
		 */
		if ( victim->master == ch )
			stop_follower( victim );
	}

	/*
	 * Inviso attacks ... not.
	 */
	if ( IS_AFFECTED(ch, AFF_INVISIBLE) )
	{
		affect_strip( ch, gsn_invis );
		REMOVE_BIT( ch->affected_by, AFF_INVISIBLE );
		act( "$n fades into existence.", ch, NULL, NULL, TO_ROOM, 0 );
	}

	/*
	 * Damage modifiers.
	 */

	if ( dam > 1 && !IS_NPC(victim)
			&&   victim->condition[COND_DRUNK]  > 10 )
		dam = 9 * dam / 10;
	if ( dam > 1 && !IS_NPC(victim)
			&&   victim->condition[COND_HIGH]  > 10 )
		dam = 9 * dam / 10;

	if ( dam > 1 && ((IS_AFFECTED(victim, AFF_PROTECT_EVIL) && !IS_NATURAL(ch) )) )
		dam -= dam / 4;

	immune = FALSE;


	/*
	 * Check for parry, and dodge.

    if ( dt >= TYPE_HIT && ch != victim)
    {
        if ( check_parry( ch, victim ) )
	    return -1;
	if ( check_dodge( ch, victim ) )
	    return -1;
    }
	 */

	switch(check_immune(victim,dam_type))
	{
	case(IS_IMMUNE):
	    		immune = TRUE;
	dam = 0;
	break;
	case(IS_RESISTANT):
	    		dam -= dam/3;
	break;
	case(IS_VULNERABLE):
	    		dam += dam/2;
	break;
	}

	if (show)
		dam_message( ch, victim, dam, dt, immune, combo );

	if(dam > (victim->health + victim->agghealth -7))
	{
		victim->position = P_MORT;
		stop_fighting(ch, TRUE);
	}
	else if(dam == (victim->health + victim->agghealth -7))
	{
		victim->position = P_INCAP;
		stop_fighting(ch, TRUE);
	}

	if (dam == 0)
		return -1;
	else if(IS_SET(ch->off_flags, BANDAGED))
		REMOVE_BIT(ch->off_flags, BANDAGED);

	/*
	 * Hurt the victim.
	 * Inform the victim of his new state.
	 */
	if( (victim->race == race_lookup("vampire")) && (dt == DAM_FIRE) )
	{
		victim->agghealth -= dam;
		update_pos( victim, UMAX(1, agg) );
		if(agg <= 0) agg = 1;
	}
	else if( (victim->race == race_lookup("werewolf")) && (dt == DAM_SILVER) )
	{
		victim->agghealth -= dam;
		update_pos( victim, UMAX(1, agg) );
		if(agg <= 0) agg = 1;
	}
	else if( (victim->race == race_lookup("faerie")) && (dt == DAM_IRON) )
	{
		victim->health -= dam;
		victim->GHB += dam/3;
		update_pos( victim, agg );
	}
	else if(agg)
	{
		victim->agghealth -= dam;
		update_pos( victim, agg );
	}
	else
	{
		victim->health -= dam;
		update_pos( victim, 0 );
	}

	switch( victim->position )
	{
	case P_MORT:
		act( "$n is mortally wounded, and will die soon, if not aided.", victim, NULL, NULL, TO_ROOM, 0 );
		send_to_char("You are mortally wounded, and may die soon, if not aided.\n\r", victim );
		break;

	case P_INCAP:
		act( "$n is incapacitated and will slowly die, if not aided.", victim, NULL, NULL, TO_ROOM, 0 );
		send_to_char("You are incapacitated and will slowly die, if not aided.\n\r", victim );
		break;

	case P_TORPOR:
		act( "$n is mortally wounded, and will slowly die if not aided.", victim, NULL, NULL, TO_ROOM, 0 );
		send_to_char("You enter torpor.\n\r", victim );
		break;

	case P_STUN:
		act( "$n is stunned, but will probably recover.", victim, NULL, NULL, TO_ROOM, 0 );
		send_to_char("You are stunned, but will probably recover.\n\r", victim );
		break;

	case P_DEAD:
		act( "$n is DEAD!!", victim, 0, 0, TO_ROOM, 0 );
		send_to_char( "You have been KILLED!!\n\r\n\r", victim );
		break;

	default:
		if ( dam > MAX_HEALTH / 4 )
			send_to_char( "That really did HURT!\n\r", victim );
		if ( (victim->health + victim->agghealth - 7) < MAX_HEALTH / 4 )
			send_to_char( "You sure are BLEEDING!\n\r", victim );
		break;
	}

	if(dam_type == DAM_FIRE)
		fire_effect((void *) victim, agg+dam/2, dam, TARGET_CHAR);
	if(dam_type == DAM_COLD)
		cold_effect((void *) victim, agg+dam/2, dam, TARGET_CHAR);
	if(dam_type == DAM_LIGHTNING)
		shock_effect((void *) victim, agg+dam/2, dam, TARGET_CHAR);
	if(dam_type == DAM_ACID)
		acid_effect((void *) victim, agg+dam/2, dam, TARGET_CHAR);
	if(dam_type == DAM_POISON)
		poison_effect((void *) victim, agg+dam/2, dam, TARGET_CHAR);

	/*
	 * Sleep spells and extremely wounded folks.
	 */
	if ( !IS_AWAKE(victim) )
		stop_fighting( victim, FALSE );

	/*
	 * Payoff for killing things.
	 */
	if ( (victim->position == P_INCAP && IS_NPC(victim))
			|| victim->position == P_DEAD
			|| victim->position == P_TORPOR )
	{
		if ( !IS_NPC(victim) )
		{
			log_string( Format("%s killed by %s at %d", victim->name, (IS_NPC(ch) ? ch->short_descr : ch->name), ch->in_room->vnum) );
		}

		snprintf( log_buf, 2*MIL, "\tY[WIZNET]\tn %s got toasted by %s at %s [room %d]",
				(IS_NPC(victim) ? victim->short_descr : victim->name),
				(IS_NPC(ch) ? ch->short_descr : ch->name), ch->in_room->name, ch->in_room->vnum);

		if (IS_NPC(victim))
			wiznet(log_buf,NULL,NULL,WIZ_MOBDEATHS,0,0);
		else
			wiznet(log_buf,NULL,NULL,WIZ_DEATHS,0,0);

		/*
		 * Death trigger
		 */
		if ( IS_NPC( victim ) && HAS_TRIGGER( victim, TRIG_DEATH) )
		{
			victim->position = P_STAND;
			mp_percent_trigger( victim, ch, NULL, NULL, TRIG_DEATH );
		}

		if((!str_cmp(ch->description, "") || strlen(ch->description) < 10)
				&& ch->played > 10*60*60)
		{
			send_to_char("No experience without a description.\n\r", ch);
		}
		else
		{
			if(ch->ooc_xp_count < 2) {
				send_to_char("You learn from your encounter.\n\r", ch);
				ch->oocxp += 1;
				ch->ooc_xp_count++;
			} else if(IS_SET(victim->act2, ACT2_HUNTER) && ch->ooc_xp_count < 50) {
				send_to_char("You learn from your encounter.\n\r", ch);
				ch->exp += 1;
				ch->ooc_xp_count++;
			}
		}

		if(ch->quest)
		{
			if(ch->quest->quest_type == Q_HITMAN && ch->quest->victim == victim)
				(*quest_table[ch->quest->quest_type].q_fun) (ch, 2);

			if(victim->quest != NULL && victim->quest->quest_type == Q_HITMAN
					&& victim->quest->victim == victim
					&& victim->quest->questor != ch)
				(*quest_table[victim->quest->quest_type].q_fun)
				(victim->quest->questor, 3);

			if(victim->quest != NULL && (victim->quest->quest_type == Q_BODYGUARD
					|| victim->quest->quest_type == Q_RESCUE)
					&& victim->quest->victim == victim)
				(*quest_table[victim->quest->quest_type].q_fun)
				(victim->quest->questor, 3);
		}

		if(victim->position != P_TORPOR || agg) update_pos( victim, agg );

		return victim->position;
	}

	if ( victim == ch )
		return ch->position;

	/* Link dead salvation. */
	if ( !IS_NPC(victim) && victim->desc == NULL )
	{
		do_function(victim, &do_flee,"");
	}

	tail_chain( );
	return victim->position;
}


/*
 * Check for parry.
 */
bool check_parry( CHAR_DATA *ch, CHAR_DATA *victim )
{
	int chance;

	if ( !IS_AWAKE(victim) )
		return FALSE;

	chance = get_skill(victim,gsn_parry) / 2;

	if ( get_eq_char( victim, WEAR_WIELD ) == NULL )
	{
		if (IS_NPC(victim))
			chance /= 2;
		else
			return FALSE;
	}

	if (!can_see(ch,victim))
		return FALSE;

	if ( dice_rolls(ch, chance,8) >= 1 )
		return FALSE;

	act( "You parry $n's attack.",  ch, NULL, victim, TO_VICT, 1 );
	act( "$N parries your attack.", ch, NULL, victim, TO_CHAR, 1 );
	return TRUE;
}


/*
 * Check for dodge.
 */
bool check_dodge( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim) )
	return FALSE;

    chance = get_skill(victim,gsn_dodge) / 2;

    if (!can_see(victim,ch))
	chance /= 2;

    if ( dice_rolls(ch,chance,6) >= 1 )
        return FALSE;

    act( "You dodge $n's attack.", ch, NULL, victim, TO_VICT, 1 );
    act( "$N dodges your attack.", ch, NULL, victim, TO_CHAR, 1 );
    return TRUE;
}


/*
 * Set position of a victim.
 */
void update_pos( CHAR_DATA *victim, int agg )
{
	if ( (victim->health + victim->agghealth - 7) > 0
			&& (victim->position < P_SLEEP || victim->position > P_STAND) )
	{
		victim->position = P_STAND;
		return;
	}

	if(IS_SET(victim->plr_flags, PLR_ARREST)
			&& (victim->health + victim->agghealth - 7) <= 0)
	{
		victim->health = 6;
		victim->agghealth = 7;
		victim->position = P_STAND;
		if(victim->fighting)
		{
			act("$N loads $n into a police cruiser to be taken to gaol.",
					victim, NULL, victim->fighting, TO_NOTVICT, 0);
			act("$N loads you into a police car and you are hauled away.",
					victim, NULL, victim->fighting, TO_CHAR, 0);
			act("You pack $n into a police car to be taken off to gaol.",
					victim, NULL, victim->fighting, TO_VICT, 0);
		}
		else
		{
			act("The police load $n into a cruiser to be taken to gaol.",
					victim, NULL, NULL, TO_NOTVICT, 0);
			act("A group of police officers haul you away.",
					victim, NULL, NULL, TO_CHAR, 1);
		}
		char_from_room(victim);
		char_to_room(victim, get_room_index(ROOM_VNUM_JAIL));
		return;
	}

	if (((IS_NATURAL(victim))
			&& ((victim->health + victim->agghealth - 7) < 0
					|| (victim->health + victim->agghealth - 7) < 0))
					|| ( (victim->health + victim->agghealth - 7) < 0 && victim->RBPG <= 1 ))
	{
		victim->position = P_DEAD;
		return;
	}

	if ( (victim->health + victim->agghealth - 7) == 0 )
		victim->position = P_INCAP;
	if ( (victim->health + victim->agghealth - 7) < 0 )
	{
		if ( victim->race == race_lookup("human"))
			victim->position = P_MORT;
		else
			if( victim->agghealth > 0 && !agg )
				victim->position = P_TORPOR;
			else
				victim->position = P_MORT;
	}

	return;
}


/*
 * Start fights.
 */
void set_fighting( CHAR_DATA *ch, CHAR_DATA *victim )
{
	if ( ch->fighting != NULL )
	{
		bug( "Set_fighting: already fighting", 0 );
		return;
	}

	if ( IS_AFFECTED(ch, AFF_SLEEP) )
		affect_strip( ch, gsn_sleep );

	do_function(ch, &do_visible,"");

	ch->fighting = victim;
	ch->position = P_FIGHT;

	return;
}


/*
 * Stop fights.
 */
void stop_fighting( CHAR_DATA *ch, bool fBoth )
{
	CHAR_DATA *fch;
	COMBAT_DATA *pcombo;
	COMBAT_DATA *next;

	for ( fch = char_list; fch != NULL; fch = fch->next )
	{
		if ( fch == ch || ( fBoth && fch->fighting == ch ) )
		{
			fch->position	= IS_NPC(fch) ? fch->default_pos : P_STAND;
			update_pos( fch, 0 );
			fch->fighting	= NULL;
			fch->balance = 1;
			for(pcombo = fch->combo; pcombo; pcombo = next)
			{
				next = pcombo->next;
				free_combat_move(pcombo);
			}
			fch->combo = NULL;
		}
	}

	return;
}


/*
 * Make a corpse out of a character.
 */
void make_corpse( CHAR_DATA *ch )
{
	char buf[MSL]={'\0'};
    OBJ_DATA *corpse;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    char *name;

    if ( IS_NPC(ch) )
    {
	name		= ch->short_descr;
	corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_NPC));
	corpse->timer	= number_range( 3, 6 );
	if ( ch->dollars > 0 )
	{
	    obj_to_obj( create_money( ch->dollars, ch->cents ), corpse );
	    ch->cents = 0;
	    ch->dollars = 0;
	}
	corpse->cost = 0;
    }
    else
    {
	name		= ch->name;
	corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_PC));
	corpse->timer	= number_range( 25, 40 );
	    corpse->owner = NULL;
	    if (ch->dollars > 1 || ch->cents > 1)
	    {
		obj_to_obj(create_money(ch->dollars / 2, ch->cents/2), corpse);
	        ch->cents = 0;
	        ch->dollars = 0;
	    }
		
	corpse->cost = 0;
    }

    snprintf( buf, sizeof(buf), corpse->short_descr, name );
    PURGE_DATA( corpse->short_descr );
    corpse->short_descr = str_dup( buf );

    snprintf( buf, sizeof(buf), corpse->description, name );
    PURGE_DATA( corpse->description );
    corpse->description = str_dup( buf );

    PURGE_DATA( corpse->full_desc );
    corpse->full_desc = str_dup( Format("The corpse of someone who in life looked like:\n\r%s", ch->description) );

    if(!str_cmp(ch->material, "flesh") && !str_cmp(ch->material, "none") && (material_lookup(ch->material) != material_lookup("unknown")))
	{
    	PURGE_DATA( corpse->material );
	corpse->material = str_dup(ch->material);
	}

    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
	obj_next = obj->next_content;
	obj_from_char( obj );
	if (obj->item_type == ITEM_POTION)
	    obj->timer = number_range(500,1000);
	if (obj->item_type == ITEM_SCROLL)
	    obj->timer = number_range(1000,2500);
	if (IS_SET(obj->extra_flags,ITEM_ROT_DEATH))
	{
	    obj->timer = number_range(1,3);
	    REMOVE_BIT(obj->extra_flags,ITEM_ROT_DEATH);
	}
	REMOVE_BIT(obj->extra_flags,ITEM_VIS_DEATH);

	if(obj != NULL)
	{
	    if(IS_SET(ch->act, ACT_UMBRA))
		obj_to_plane(obj, 1);
	    if(IS_SET(ch->act, ACT_DREAMING))
		obj_to_plane(obj, 2);
	}
	obj_to_obj( obj, corpse );
    }

    if(IS_SET(ch->act, ACT_UMBRA))
	obj_to_plane(corpse, 1);
    if(IS_SET(ch->act, ACT_DREAMING))
	obj_to_plane(corpse, 2);
    obj_to_room( corpse, ch->in_room );
    return;
}


void death_cry( CHAR_DATA *ch )
{
    ROOM_INDEX_DATA *was_in_room;
    char *msg;
    int door;
    int vnum;

    vnum = 0;
    msg = NULL;

    while(msg == NULL)
    switch ( number_bits(4))
    {
    case  0: msg  = "$n hits the ground ... DEAD.";			break;
    case  1: 
	if (ch->material == 0)
	{
	    msg  = "$n splatters blood on your clothes.";		
	    break;
	}
    case  2: 							
	if (IS_SET(ch->parts,PART_GUTS))
	{
	    msg = "$n spills $s guts all over the floor.";
	    vnum = OBJ_VNUM_GUTS;
	}
	break;
    case  3: 
	if (IS_SET(ch->parts,PART_HEAD))
	{
	    msg  = "$n's severed head plops on the ground.";
	    vnum = OBJ_VNUM_SEVERED_HEAD;				
	}
	break;
    case  4: 
	if (IS_SET(ch->parts,PART_HEART))
	{
	    msg  = "$n's heart is torn from $s chest.";
	    vnum = OBJ_VNUM_TORN_HEART;				
	}
	break;
    case  5: 
	if (IS_SET(ch->parts,PART_ARMS))
	{
	    msg  = "$n's arm is sliced from $s dead body.";
	    vnum = OBJ_VNUM_SLICED_ARM;				
	}
	break;
    case  6: 
	if (IS_SET(ch->parts,PART_LEGS))
	{
	    msg  = "$n's leg is sliced from $s dead body.";
	    vnum = OBJ_VNUM_SLICED_LEG;				
	}
	break;
    case 7:
	if (IS_SET(ch->parts,PART_BRAINS))
	{
	    msg = "$n's head is shattered, and $s brains splash all over you.";
	    vnum = OBJ_VNUM_BRAINS;
	}
    }

    act( msg, ch, NULL, NULL, TO_ROOM, 0 );

    if ( vnum != 0 )
    {
    	char buf[MSL]={'\0'};
	OBJ_DATA *obj;
	char *name;

	name		= IS_NPC(ch) ? ch->short_descr : ch->name;
	obj		= create_object( get_obj_index( vnum ) );
	obj->timer	= number_range( 4, 7 );

	snprintf( buf, sizeof(buf), obj->short_descr, name );
	PURGE_DATA( obj->short_descr );
	obj->short_descr = str_dup( buf );

	snprintf( buf, sizeof(buf), obj->description, name );
	PURGE_DATA( obj->description );
	obj->description = str_dup( buf );

	if (obj->item_type == ITEM_FOOD)
	{
	    if (IS_SET(ch->form,FORM_POISON))
		obj->value[3] = 1;
	    else if (!IS_SET(ch->form,FORM_EDIBLE))
		obj->item_type = ITEM_TRASH;
	}
        if(!str_cmp(ch->material, "flesh") && !str_cmp(ch->material, "none") && (material_lookup(ch->material) != material_lookup("unknown")))
	{
        	PURGE_DATA( obj->material );
	obj->material = str_dup(ch->material);
	}

	if(IS_SET(ch->act, ACT_UMBRA))
	    obj_to_plane(obj, 1);
	if(IS_SET(ch->act, ACT_DREAMING))
	    obj_to_plane(obj, 2);
	obj_to_room( obj, ch->in_room );
    }

    was_in_room = ch->in_room;
    for ( door = 0; door <= 5; door++ )
    {
	EXIT_DATA *pexit;

	if ( ( pexit = was_in_room->exit[door] ) != NULL
	&&   pexit->u1.to_room != NULL
	&&   pexit->u1.to_room != was_in_room )
	{
	    ch->in_room = pexit->u1.to_room;
	    act( msg, ch, NULL, NULL, TO_ROOM, 0 );
	}
    }
    ch->in_room = was_in_room;

    return;
}


void raw_kill( CHAR_DATA *victim, int absolute )
{
    int i = 0;

/*
    if(victim->fighting != NULL)
    {
	if(victim->fighting->ooc_xp_count < 2) {
	    send_to_char("You learn from your encounter.\n\r",
		 victim->fighting);
	    victim->fighting->exp += 1;
	}
	stop_fighting( victim, TRUE );
    }
*/

    if(!absolute && victim->race == race_lookup("werewolf")
	&& ((i = dice_rolls(victim, victim->max_RBPG, 8)) > 0
	    || victim->agghealth > 0))
    {
	bool tag = FALSE;

	if(victim->health < 0)
	    victim->health = 0;
	if(victim->agghealth < 0) {
	    victim->agghealth = 0;
	    tag = TRUE;
	}

	if(absolute) tag = TRUE;

	if(tag) {
	    i -= 7 - victim->health;
	    if(i > 0)
	      victim->agghealth +=
		UMIN(7 - victim->agghealth, i);
	}
	else while((victim->agghealth + victim->health - 7) <= 0)
	{
	    if(victim->health >= 7)
		victim->agghealth++;
	    else victim->health++;
	}
	send_to_char("It hurts, but your body starts to heal.\n\r", victim);
	update_pos(victim, 0);
	if((victim->agghealth + victim->health - 7) > 0)
	    return;
    }

    if(victim->clan == clan_lookup("bone gnawer")
	&& victim->disc[DISC_TRIBE] >= 5
	&& victim->max_willpower > 0)
    {
	victim->max_willpower--;
	if(victim->willpower > victim->max_willpower)
	{
	    victim->willpower = victim->max_willpower;
	}
	if(victim->health <= 0)
	    victim->health = 1;
	if(victim->agghealth <= 0)
	    victim->agghealth = 1;
	while((victim->agghealth + victim->health - 7) <= 0)
	{
	    if(victim->health >= 7)
		victim->agghealth++;
	    else victim->health++;
	}
	send_to_char("It hurts, but you'll live.\n\r", victim);
	return;
    }

    death_cry( victim );
    make_corpse( victim );

    if ( IS_NPC(victim) )
    {
	victim->pIndexData->killed++;
	extract_char( victim, TRUE );
	return;
    }

    clear_orgs(victim);
    if(victim->desc)
    victim->desc->connected = CON_DECEASED;
    SET_BIT(victim->act, ACT_REINCARNATE);

    char_from_room(victim);
    char_from_list(victim);
    send_to_char("You are DEAD!\n\r[Hit Return to Continue]\n\r", victim);
    victim->exp = get_points(victim);
    save_char_obj(victim);
    return;
}


void dam_message( CHAR_DATA *ch, CHAR_DATA *victim,int dam,int dt,bool immune, int combo )
{
    char buf1[256]={'\0'};
    char buf2[256]={'\0'};
    char buf3[256]={'\0'};
    const char *vs;
    const char *vp;
    const char *attack;
    char punct;

    if (ch == NULL || victim == NULL)
	return;

	 if ( dam <=   0 ) { vs = "do nothing to";
			     vp = "does nothing to";		}
    else if ( dam ==   1 ) { vs = "scratch";	vp = "scratches";	}
    else if ( dam ==   2 ) { vs = "hit";	vp = "hits";		}
    else if ( dam ==   3 ) { vs = "injure";	vp = "injures";		}
    else if ( dam ==   4 ) { vs = "wound";	vp = "wounds";		}
    else if ( dam ==   5 ) { vs = "maim";	vp = "maims";		}
    else if ( dam ==   6 ) { vs = "MANGLE";	vp = "MANGLES";		}
    else                   { vs = "do UNSPEAKABLE things to";
			     vp = "does UNSPEAKABLE things to";		}

    punct   = (dam <= 24) ? '.' : '!';

    if ( dt == TYPE_HIT )
    {
    	if (ch  == victim)
    	{
    		snprintf( buf1, sizeof(buf1), "$n %s%s $melf%c",
    				combo==9999?"'s shot ":"",vp,punct);
    		snprintf(buf2, sizeof(buf2), "You%s%s yourself%c",
    				combo==9999?"r shot ":" ",combo==9999?vp:vs,punct);
    	}
    	else
    	{
    		snprintf( buf1, sizeof(buf1), "$n%s%s $N%c",
    				combo==9999?"'s shot ":" ",vp, punct );
    		snprintf( buf2, sizeof(buf2), "You%s%s $N%c",
    				combo==9999?"r shot ":" ",combo==9999?vp:vs, punct );
    		snprintf( buf3, sizeof(buf3), "$n%s%s you%c",
    				combo==9999?"'s shot ":" ",vp, punct );
    	}
    }
    else
    {
    	if ( dt >= 0 && dt < MAX_SKILL )
    		attack	= skill_table[dt].noun_damage;
    	else if ( dt >= TYPE_HIT
    			&& dt < TYPE_HIT + MAX_DAMAGE_MESSAGE)
    		attack	= attack_table[dt - TYPE_HIT - 1].noun;
    	else
    	{
    		bug( "Dam_message: bad dt %d.", dt );
    		dt  = TYPE_HIT;
    		attack  = attack_table[0].noun;
    	}

    	if (immune)
    	{
    		if (ch == victim)
    		{
    			snprintf(buf1, sizeof(buf1), "$n is unaffected by $s own %s.",attack);
    			snprintf(buf2, sizeof(buf2), "Luckily, you are immune to that.");
    		}
    		else
    		{
    			snprintf(buf1, sizeof(buf1), "$N is unaffected by $n's %s!",attack);
    			snprintf(buf2, sizeof(buf2), "$N is unaffected by your %s!",attack);
    			snprintf(buf3, sizeof(buf3), "$n's %s is powerless against you.",attack);
    		}
    	}
    	else
    	{
    		if (ch == victim)
    		{
    			snprintf( buf1, sizeof(buf1), "$n's %s%s %s $m%c",
    					combo==9999?"shot ":"",attack,vp,punct);
    			snprintf( buf2, sizeof(buf2), "Your %s%s %s you%c",
    					combo==9999?"shot ":"",attack,vp,punct);
    		}
    		else
    		{
    			snprintf( buf1, sizeof(buf1), "$n's %s%s %s $N%c",
    					combo==9999?"shot ":"",attack,vp,punct);
    			snprintf( buf2, sizeof(buf2), "Your %s%s %s $N%c",
    					combo==9999?"shot ":"",attack,vp,punct);
    			snprintf( buf3, sizeof(buf3), "$n's %s%s %s you%c",
    					combo==9999?"shot ":"",attack,vp,punct);
    		}
    	}
    }

    if (ch == victim)
    {
    	act(buf1,ch,NULL,NULL,TO_ROOM, 0);
    	act(buf2,ch,NULL,NULL,TO_CHAR, 1);
    }
    else
    {
    	if(combo > 0 && combo != 9999)
    	{
    		act( combo_table[combo].to_room, ch, NULL, victim, TO_NOTVICT, 0 );
    		act( combo_table[combo].to_char, ch, NULL, victim, TO_CHAR, 1 );
    		act( combo_table[combo].to_vict, ch, NULL, victim, TO_VICT, 1 );
    	}

    	act( buf1, ch, NULL, victim, TO_NOTVICT, 0 );
    	act( buf2, ch, NULL, victim, TO_CHAR, 1 );
    	act( buf3, ch, NULL, victim, TO_VICT, 1 );
    }

    return;
}


/*
 * Disarm a creature.
 * Caller must check for successful attack.
 */
void disarm( CHAR_DATA *ch, CHAR_DATA *victim )
{
    OBJ_DATA *obj;

    if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL )
	return;

    if ( IS_OBJ_STAT(obj,ITEM_NOREMOVE))
    {
	act("$S weapon won't budge!",ch,NULL,victim,TO_CHAR, 1);
	act("$n tries to disarm you, but your weapon won't budge!",
	    ch,NULL,victim,TO_VICT, 0);
	act("$n tries to disarm $N, but fails.",ch,NULL,victim,TO_NOTVICT, 0);
	return;
    }

    act( "$n DISARMS you and sends your weapon flying!",
	 ch, NULL, victim, TO_VICT, 1 );
    act( "You disarm $N!",  ch, NULL, victim, TO_CHAR, 1 );
    act( "$n disarms $N!",  ch, NULL, victim, TO_NOTVICT, 0 );

    obj_from_char( obj );
    if ( IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_INVENTORY) )
	obj_to_char( obj, victim );
    else
    {
	obj_to_room( obj, victim->in_room );
	if (IS_NPC(victim) && victim->wait == 0 && can_see_obj(victim,obj))
	    get_obj(victim,obj,NULL);
    }

    return;
}


void do_bash( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH]={'\0'};
    CHAR_DATA *victim;
    int chance = 0;

    one_argument(argument,arg);
 
    if (IS_NULLSTR(arg))
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't fighting anyone!\n\r",ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (victim->position < P_FIGHT)
    {
	act("You'll have to let $M get back up first.",
	    ch,NULL,victim,TO_CHAR,1);
	return;
    }

    if (victim == ch)
    {
	send_to_char("You try to bash your brains out, but fail.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("But $N is your friend!",ch,NULL,victim,TO_CHAR,1);
	return;
    }

    /* size */
    if (ch->size < victim->size)
	chance += (ch->size - victim->size) * 15;
    else
	chance += (ch->size - victim->size) * 10; 

    /* stats */
    chance += get_curr_stat(ch,STAT_STR);
    chance -= get_curr_stat(victim,STAT_DEX);

    if (!IS_NPC(victim) 
	&& chance < get_skill(victim,gsn_dodge) )
    {	
	chance -= 3 * (get_skill(victim,gsn_dodge) - chance);
    }

    /* now the attack */
    if (number_percent() < chance )
    {
    
	act("$n sends you sprawling with a powerful bash!",
		ch,NULL,victim,TO_VICT,1);
	act("You slam into $N, and send $M flying!",ch,NULL,victim,TO_CHAR,0);
	act("$n sends $N sprawling with a powerful bash.",
		ch,NULL,victim,TO_NOTVICT,0);

	DAZE_STATE(victim, 3 * PULSE_VIOLENCE);
	WAIT_STATE(ch,skill_table[gsn_bash].beats);
	victim->position = P_REST;
	damage(ch,victim,number_range(2,2 + 2 * ch->size + chance/20),gsn_bash,
	    DAM_BASH,FALSE, 0, -1);
	
    }
    else
    {
	damage(ch,victim,0,gsn_bash,DAM_BASH,FALSE, 0, -1);
	act("You fall flat on your face!",
	    ch,NULL,victim,TO_CHAR,1);
	act("$n falls flat on $s face.",
	    ch,NULL,victim,TO_NOTVICT,0);
	act("You evade $n's bash, causing $m to fall flat on $s face.",
	    ch,NULL,victim,TO_VICT,1);
	ch->position = P_REST;
	damage(ch,ch,number_range(2,2 + 2 * ch->size + chance/20),gsn_bash,
	    DAM_BASH,FALSE, 0, -1);
	WAIT_STATE(ch,skill_table[gsn_bash].beats * 3/2); 
    }
}

void do_dirt( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH]={'\0'};
    CHAR_DATA *victim;
    int chance;

    one_argument(argument,arg);

    if ( (chance = get_skill(ch,gsn_dirt)) == 0
    ||   (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_KICK_DIRT))
    ||   (!IS_NPC(ch)
    &&    !(ch->ability[BRAWL].value > 0)))
    {
	send_to_char("You get your feet dirty.\n\r",ch);
	return;
    }

    if (IS_NULLSTR(arg))
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't in combat!\n\r",ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(victim,AFF_BLIND))
    {
	act("$E's already been blinded.",ch,NULL,victim,TO_CHAR,1);
	return;
    }

    if (victim == ch)
    {
	send_to_char("Very funny.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("But $N is such a good friend!",ch,NULL,victim,TO_CHAR,1);
	return;
    }

    /* dexterity */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= 2 * get_curr_stat(victim,STAT_DEX);

    /* sloppy hack to prevent false zeroes */
    if (chance % 5 == 0)
	chance += 1;

    /* terrain */

    switch(ch->in_room->sector_type)
    {
	case(SECT_INSIDE):		chance -= 20;	break;
	case(SECT_CITY):		chance -= 10;	break;
	case(SECT_STREET):		chance -= 10;	break;
	case(SECT_FIELD):		chance +=  5;	break;
	case(SECT_FOREST):				break;
	case(SECT_HILLS):				break;
	case(SECT_MOUNTAIN):		chance -= 10;	break;
	case(SECT_WATER_SWIM):		chance  =  0;	break;
	case(SECT_WATER_NOSWIM):	chance  =  0;	break;
	case(SECT_AIR):			chance  =  0;  	break;
	case(SECT_DESERT):		chance += 10;   break;
    }

    if (chance == 0)
    {
	send_to_char("There isn't any dirt to kick.\n\r",ch);
	return;
    }

    /* now the attack */
    if (number_percent() < chance)
    {
	AFFECT_DATA af;
	act("$n is blinded by the dirt in $s eyes!",victim,NULL,NULL,TO_ROOM,0);
	act("$n kicks dirt in your eyes!",ch,NULL,victim,TO_VICT,1);
        damage(ch,victim,number_range(2,5),gsn_dirt,DAM_NONE,FALSE,0,-1);
	send_to_char("You can't see a thing!\n\r",victim);
	WAIT_STATE(ch,skill_table[gsn_dirt].beats);

	af.where	= TO_AFFECTS;
	af.type 	= gsn_dirt;
	af.duration	= 15;
	af.location	= APPLY_PER;
	af.modifier	= -4;
	af.bitvector 	= AFF_BLIND;

	affect_to_char(victim,&af);
    }
    else
    {
	damage(ch,victim,0,gsn_dirt,DAM_NONE,TRUE,0,-1);
	WAIT_STATE(ch,skill_table[gsn_dirt].beats);
    }
}

void do_trip( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH]={'\0'};
    CHAR_DATA *victim;
    int chance;

    one_argument(argument,arg);

    if ( (chance = get_skill(ch,gsn_trip)) == 0
    ||   (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_TRIP))
    ||   (!IS_NPC(ch) 
	  && !(ch->ability[BRAWL].value > 0)))
    {
	send_to_char("Tripping?  What's that?\n\r",ch);
	return;
    }


    if (IS_NULLSTR(arg))
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't fighting anyone!\n\r",ch);
	    return;
 	}
    }

    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }
    
    if (IS_AFFECTED(victim,AFF_FLYING))
    {
	act("$S feet aren't on the ground.",ch,NULL,victim,TO_CHAR,1);
	return;
    }

    if (victim->position < P_FIGHT)
    {
	act("$N is already down.",ch,NULL,victim,TO_CHAR,1);
	return;
    }

    if (victim == ch)
    {
	send_to_char("You fall flat on your face!\n\r",ch);
	WAIT_STATE(ch,2 * skill_table[gsn_trip].beats);
	act("$n trips over $s own feet!",ch,NULL,NULL,TO_ROOM,0);
	return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("$N is so great though!",ch,NULL,victim,TO_CHAR,1);
	return;
    }

    /* size */
    if (ch->size < victim->size)
        chance += (ch->size - victim->size) * 10;  /* bigger = harder to trip */

    /* dex */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= get_curr_stat(victim,STAT_DEX) * 3 / 2;

    /* now the attack */
    if (number_percent() < chance)
    {
	act("$n trips you and you go down!",ch,NULL,victim,TO_VICT,1);
	act("You trip $N and $N goes down!",ch,NULL,victim,TO_CHAR,1);
	act("$n trips $N, sending $M to the ground.",
	    ch,NULL,victim,TO_NOTVICT,0);

	DAZE_STATE(victim,2 * PULSE_VIOLENCE);
        WAIT_STATE(ch,skill_table[gsn_trip].beats);
	victim->position = P_REST;
	damage(ch,victim,number_range(2, 2 +  2 * victim->size),gsn_trip,
	    DAM_BASH,TRUE,0,-1);
    }
    else
    {
	damage(ch,victim,0,gsn_trip,DAM_BASH,TRUE,0,1);
	WAIT_STATE(ch,skill_table[gsn_trip].beats*2/3);
    } 
}

void do_kill( CHAR_DATA *ch, char *argument )
{
send_to_char("Project Twilight uses a combo-based combat system.\n\rPlease read HELP COMBAT\n\r", ch);
}

/*
void do_kill( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH]={'\0'};
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( IS_NULLSTR(arg) )
    {
	send_to_char( "Kill whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "You hit yourself.  Ouch!\n\r", ch );
	multi_hit( ch, ch, TYPE_UNDEFINED );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
	act( "$N is so wonderful though!.", ch, NULL, victim, TO_CHAR, 1 );
	return;
    }

    if ( ch->position == P_FIGHT )
    {
	send_to_char( "You do the best you can!\n\r", ch );
	return;
    }

    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}
*/

void do_backstab( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH]={'\0'};
	CHAR_DATA *victim;
	/*OBJ_DATA *obj;*/

	one_argument( argument, arg );

	if (IS_NULLSTR(arg))
	{
		send_to_char("Backstab whom?\n\r",ch);
		return;
	}

	if (ch->fighting != NULL)
	{
		send_to_char("You're facing the wrong end.\n\r",ch);
		return;
	}

	else if ((victim = get_char_room(ch,arg)) == NULL)
	{
		send_to_char("They aren't here.\n\r",ch);
		return;
	}

	if ( victim == ch )
	{
		send_to_char( "How can you sneak up on yourself?\n\r", ch );
		return;
	}

	WAIT_STATE( ch, skill_table[gsn_backstab].beats );
	if ( number_percent( ) < get_skill(ch,gsn_backstab)
			|| ( get_skill(ch,gsn_backstab) >= 2 && !IS_AWAKE(victim) ) )
	{
		multi_hit( ch, victim, gsn_backstab );
	}
	else
	{
		damage( ch, victim, 0, gsn_backstab,DAM_NONE,TRUE,0,-1);
	}

	return;
}


void do_kick( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if ( !IS_NPC(ch)
    &&   !(ch->ability[BRAWL].value > 0) )
    {
	send_to_char("You better leave the martial arts to fighters.\n\r", ch );
	return;
    }

    if (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_KICK))
	return;

    if ( ( victim = ch->fighting ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_kick].beats );
    if ( get_skill(ch,gsn_kick) > number_percent())
    {
	damage(ch,victim,number_range( 1, get_curr_stat(ch, STAT_STR) + 1 ), gsn_kick,DAM_BASH,TRUE,0,-1);
    }
    else
    {
	damage( ch, victim, 0, gsn_kick,DAM_BASH,TRUE,0,-1);
    }
    return;
}


void do_disarm( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	int chance,hth,ch_weapon,vict_weapon,ch_vict_weapon;

	hth = 0;

	if ((chance = get_skill(ch,gsn_disarm)) == 0)
	{
		send_to_char( "You don't know how to disarm opponents.\n\r", ch );
		return;
	}

	if ( get_eq_char( ch, WEAR_WIELD ) == NULL
			&&   ((hth = get_skill(ch,gsn_brawl)) == 0
					||    (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_DISARM))))
	{
		send_to_char( "You must wield a weapon to disarm.\n\r", ch );
		return;
	}

	if ( ( victim = ch->fighting ) == NULL )
	{
		send_to_char( "You aren't fighting anyone.\n\r", ch );
		return;
	}

	if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL )
	{
		send_to_char( "Your opponent is not wielding a weapon.\n\r", ch );
		return;
	}

	/* find weapon skills */
	ch_weapon = get_weapon_skill(ch,get_weapon_sn(ch));
	vict_weapon = get_weapon_skill(victim,get_weapon_sn(victim));
	ch_vict_weapon = get_weapon_skill(ch,get_weapon_sn(victim));

	/* skill */
	if ( get_eq_char(ch,WEAR_WIELD) == NULL)
		chance = chance * hth/150;
	else
		chance = chance * ch_weapon/100;

	chance += (ch_vict_weapon/2 - vict_weapon) / 2;

	/* dex vs. strength */
	chance += get_curr_stat(ch,STAT_DEX);
	chance -= 2 * get_curr_stat(victim,STAT_STR);

	/* and now the attack */
	if (number_percent() < chance)
	{
		WAIT_STATE( ch, skill_table[gsn_disarm].beats );
		disarm( ch, victim );
	}
	else
	{
		WAIT_STATE(ch,skill_table[gsn_disarm].beats);
		act("You fail to disarm $N.",ch,NULL,victim,TO_CHAR,1);
		act("$n tries to disarm you, but fails.",ch,NULL,victim,TO_VICT,0);
		act("$n tries to disarm $N, but fails.",ch,NULL,victim,TO_NOTVICT,0);
	}
	return;
}


void do_sla( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to SLAY, spell it out.\n\r", ch );
    return;
}


void do_slay( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH]={'\0'};

	one_argument( argument, arg );
	if ( IS_NULLSTR(arg) )
	{
		send_to_char( "Slay whom?\n\r", ch );
		return;
	}

	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	if ( ch == victim )
	{
		send_to_char( "Suicide is a mortal sin.\n\r", ch );
		return;
	}

	if ( !IS_NPC(victim) && victim->trust >= get_trust(ch) )
	{
		send_to_char( "You failed.\n\r", ch );
		return;
	}

	act( "You slay $M in cold blood!",  ch, NULL, victim, TO_CHAR, 1 );
	act( "$n slays you in cold blood!", ch, NULL, victim, TO_VICT, 1 );
	act( "$n slays $N in cold blood!",  victim, NULL, ch, TO_NOTVICT, 0 );
	raw_kill( victim, TRUE );
	return;
}

void do_surrender( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *mob;
	if ( (mob = ch->fighting) == NULL )
	{
		send_to_char( "But you're not fighting!\n\r", ch );
		return;
	}
	act( "You surrender to $N!", ch, NULL, mob, TO_CHAR, 1 );
	act( "$n surrenders to you!", ch, NULL, mob, TO_VICT, 1 );
	act( "$n tries to surrender to $N!", ch, NULL, mob, TO_NOTVICT, 0 );
	stop_fighting( ch, TRUE );

	if ( !IS_NPC( ch ) && IS_NPC( mob )
			&&   ( !HAS_TRIGGER( mob, TRIG_SURR )
					|| !mp_percent_trigger( mob, ch, NULL, NULL, TRIG_SURR ) ) )
	{
		act( "$N seems to ignore your cowardly act!", ch, NULL, mob,TO_CHAR,1);
		multi_hit( mob, ch, TYPE_UNDEFINED );
	}
}

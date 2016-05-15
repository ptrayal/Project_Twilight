#if defined(Macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include "twilight.h"
#include "tables.h"
#include "interp.h"

void run_election();
bool check_social(CHAR_DATA *ch, char *command, char *argument);
void free_script(SCRIPT_DATA *script);
void open_car_doors(ROOM_INDEX_DATA *car);
void close_car_doors(ROOM_INDEX_DATA *car);
void free_obj(OBJ_DATA *obj);
void save_stocks();
void free_extra_descr( EXTRA_DESCR_DATA *pExtra );
OBJ_DATA *new_obj(void);
void tip_to_char(CHAR_DATA *ch);

/*
 * Local functions.
 */
int	hit_gain	args( ( CHAR_DATA *ch ) );
void	mobile_update	args( ( void ) );
void	weather_update	args( ( void ) );
void	char_update	args( ( void ) );
void	obj_update	args( ( void ) );
void	room_update	args( ( void ) );
void	aggr_update	args( ( void ) );
void	stock_update	args( ( void ) );
void	msdp_update	args( ( void ) );

/* used for saving */
int	save_number = 0;

/* used for backing up */
bool backup = FALSE;

/*
 * Regeneration stuff.
 */
int hit_gain( CHAR_DATA *ch )
{
	int gain = 0;

	if (ch->in_room == NULL)
		return 0;

	if ( IS_NPC(ch) )
	{
		gain = 1;
		if (IS_AFFECTED(ch,AFF_REGENERATION))
			ch->health = MAX_HEALTH;

		switch(ch->position)
		{
		default : 		gain /= 2;			break;
		case P_SLEEP: 	gain = 3 * gain/2;	break;
		case P_REST:  						break;
		case P_FIGHT:	gain /= 3;		 	break;
		}

	}
	else
	{
		gain = UMAX(1,get_curr_stat(ch,STAT_STA));

		switch ( ch->position )
		{
		default:	   	gain /= 4;			break;
		case P_SLEEP: 						break;
		case P_REST:  	gain /= 2;			break;
		case P_FIGHT: 	gain /= 6;			break;
		}

		if ( ch->condition[COND_HUNGER]   == 0 )
			gain = 0;

		if ( ch->condition[COND_THIRST] == 0 )
			gain = 0;

	}

	gain = gain * ch->in_room->heal_rate / 100;

	if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
		gain = gain * ch->on->value[3] / 100;

	if ( IS_AFFECTED(ch, AFF_POISON) )
		gain /= 4;

	if (IS_AFFECTED(ch, AFF_PLAGUE))
		gain /= 8;

	if (race_lookup("werewolf") == ch->race && ch->health < MAX_HEALTH)
		gain += 2;

	if (race_lookup("vampire") == ch->race)
		gain = 0;

	return UMIN(gain, MAX_HEALTH - ch->health);
}

struct trip_type
{
    char *trip;
};

const struct trip_type tripping_table [] =
{
    {"Purple hairy spiders crawl over your skin!\n\r"},
    {"A little green man winks at you from around the corner of a building.\n\r"},
    {"Orange swirly colors spin before your eyes for a moment.\n\r"},
    {"An itch starts under your fingernails.\n\r"},
    {"A police officer turns into a snarling werewolf!\n\r"},
    {"The cheshire cat smiles a very wide smile at you.\n\r"},
    {"A white rabbit hops past, disconcertedly looking at a pocket watch.\n\r"},
    {"Bugs Bunny walks in dressed as a girl bunny.\n\r"},
    {"A police officer MANGLES you!\n\r"},
    {"You are DEAD!\n\r"},
    {"Someone is watching you.\n\r"}
};

void trip_out (CHAR_DATA *ch)
{
	int i = number_range(0, 10);

	send_to_char(Format("%s", tripping_table[i].trip), ch);

}

void gain_condition( CHAR_DATA *ch, int iCond, int value )
{
	int condition = 0;

	if ( value == 0 || IS_NPC(ch) || ch->trust >= LEVEL_IMMORTAL
			|| IS_SET(ch->act2, ACT2_RP_ING))
		return;

	condition = ch->condition[iCond];

	if ((iCond == COND_THIRST && condition == -50)
			|| (iCond != COND_THIRST && condition == -1)
			|| (iCond == COND_THIRST && condition == -1
					&& IS_VAMPIRE(ch)))
		return;

	if(iCond == COND_PAIN && IS_AFFECTED(ch, AFF_RESIST_PAIN))
		return;

	if(iCond == COND_PAIN && (ch->health + ch->agghealth - 7) >= 7 && value > 0)
		value *= 5;

	ch->condition[iCond] = URANGE( -48, condition + value, 100 );

	if(IS_VAMPIRE(ch) && iCond == COND_HUNGER)
		ch->condition[iCond] = 50;
	if(IS_VAMPIRE(ch) && iCond == COND_THIRST
			&& ch->condition[iCond] < 10 * ch->RBPG)
		ch->condition[iCond] = 10 * ch->RBPG;

	if ( ch->condition[iCond] <= 0 )
	{
		switch ( iCond )
		{
		case COND_HUNGER:
			send_to_char( "You are hungry.\n\r",  ch );
			break;

		case COND_THIRST:
			if(IS_VAMPIRE(ch))
			{
				send_to_char( "Your hunger gnaws at you.\n\r", ch);
			}
			else
			{
				send_to_char( "You are thirsty.\n\r", ch );
			}
			break;

		case COND_DRUNK:
			if ( condition == 0 )
				send_to_char( "You are sober.\n\r", ch );
			break;
		case COND_PAIN:
			if ( condition == 0 )
				send_to_char( "The pain passes.\n\r", ch );
			break;
		case COND_ANGER:
			if ( condition == 0 )
				send_to_char( "Your anger wanes.\n\r", ch );
			break;
		case COND_FRENZY:
			if ( condition == 0 )
				send_to_char( "Your rage abates.\n\r", ch );
			break;
		}
	}

	if ( ch->condition[iCond] <= 0 )
	{
		switch ( iCond )
		{
		case COND_HIGH:
			if ( condition == 0 )
				send_to_char( "You come down off your high.\n\r", ch );
			break;
		case COND_TRIPPING:
			if ( condition == 0 )
				send_to_char( "The world looks more normal.\n\r", ch );
			if ( condition > 10 )
				trip_out(ch);
			break;
		}
	}

	return;
}


CHAR_DATA *find_shop(int type)
{
	CHAR_DATA *ch = NULL;
	DESCRIPTOR_DATA *d;
	int i = 0, j = 0, k = 0, l = 0;

	for(d = descriptor_list; d; d = d->next) j++;
	i = number_range(1, j);

	l = i;
	for(ch = char_list; ch != NULL; ch = ch->next) j++;

	switch (type) {
	case 0:
		while (i) {
			for(ch = char_list; ch != NULL; ch = ch->next)
			{
				if(i == 1) {
					if(!str_cmp(ch->profession, "Salesman")
							|| !str_cmp(ch->profession, "Maker"))
						return ch;
					else
					{
						i++;
						k++;
					}
				} else i--;
			}
			if(k > j * l) i = 0;
		}
		break;
	case 1:
		while (i) {
			for(ch = char_list; ch != NULL; ch = ch->next)
			{
				if(i == 1) {
					if(!str_cmp(ch->profession, "Salesman") || !str_cmp(ch->profession, "Chef"))
						return ch;
					else
					{
						i++;
						k++;
					}
				} else i--;
			}
			if(k > j * l) i = 0;
		}
		break;
	default:
		break;
	}

	return NULL;
}


/*
 * Mob autonomous action.
 * AI is included... Takes more time to execute now.
 * This function used to take 25% to 35% of ALL Merc cpu time.
 * --
 */
void mobile_update( void )
{
	CHAR_DATA *ch;
	CHAR_DATA *ch_next;
	EXIT_DATA *pexit;
	int door = 0;

	/* Examine all mobs. */
	for ( ch = char_list; ch != NULL; ch = ch_next )
	{
		ch_next = ch->next;

		if ( !IS_NPC(ch) || ch->in_room == NULL )
			continue;

		if ( IS_AFFECTED(ch,AFF_CHARM) && ch->master != NULL )
			continue;

		if (ch->in_room->area->empty && !IS_SET(ch->act,ACT_UPDATE_ALWAYS))
			continue;

		if(!IS_AFFECTED(ch, AFF_CHARM)) {
			/* Police activity. */
			if (IS_SET(ch->act2, ACT2_POLICE))
			{
				arrest_player(ch);
			}

			/* Examine call for special procedure */
			if ( ch->spec_fun != 0 )
			{
				if ( (*ch->spec_fun) ( ch ) )
					continue;
			}
		}

		if (ch->pIndexData->pShop != NULL) /* give him some cash */
			if ((ch->dollars + ch->cents) < ch->pIndexData->wealth)
			{
				ch->dollars += ch->pIndexData->wealth * number_range(1,20)/5;
				ch->cents += ch->pIndexData->wealth * number_range(1,20)/5;
			}

		/*
		 * Check triggers only if mobile still in default position
		 */
		if ( ch->position == ch->pIndexData->default_pos )
		{
			/* Delay */
			if ( HAS_TRIGGER( ch, TRIG_DELAY)
					&&   ch->mprog_delay > 0 )
			{
				if ( --ch->mprog_delay <= 0 )
				{
					mp_percent_trigger( ch, NULL, NULL, NULL, TRIG_DELAY );
					continue;
				}
			}
			if ( HAS_TRIGGER( ch, TRIG_RANDOM) )
			{
				if( mp_percent_trigger( ch, NULL, NULL, NULL, TRIG_RANDOM ) )
					continue;
			}
		}

		/* Kill dead mobs */
		if ( ch->position == P_DEAD )
		{
			raw_kill(ch, FALSE);
			continue;
		}

		/* That's all for sleeping / busy monster, and empty zones */
		if ( ch->position != P_STAND )
			continue;

		if(IS_SET(ch->act2, ACT2_SHOPPER))
		{
			for(door = 0; door < 2; door++)
			{
				ch->drives[door]++;
			}
		}

		if(!IS_SET(ch->act2, ACT2_GO_SHOP))
		{
			if(ch->drives[0] > 50)
			{
				if((ch->summonner = find_shop(0)) != NULL)
				{
					if(ch->summonner->in_room->area == ch->in_room->area)
					{
						SET_BIT(ch->act, ACT_SUMMON);
						SET_BIT(ch->act2, ACT2_GO_SHOP);
					}
					else {
						ch->summonner = NULL;
						ch->drives[0] = 0;
					}
				}
			}
			else if(ch->drives[1] > 50)
			{
				if((ch->summonner = find_shop(1)) != NULL)
				{
					if(ch->summonner->in_room->area == ch->in_room->area)
					{
						SET_BIT(ch->act, ACT_SUMMON);
						SET_BIT(ch->act2, ACT2_GO_SHOP);
					}
					else {
						ch->summonner = NULL;
						ch->drives[1] = 0;
					}
				}
			}
		}

	/* Scavenge */
	if ( IS_SET(ch->act, ACT_SCAVENGER)
	&&   ch->in_room->contents != NULL
	&&   number_bits( 6 ) == 0 )
	{
	    OBJ_DATA *obj;
	    OBJ_DATA *obj_best;
	    int max;

	    max         = 1;
	    obj_best    = 0;
	    for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
	    {
		if ( CAN_WEAR(obj, ITEM_TAKE)
		     && obj->cost > max  && obj->cost > 0
		     && sat_on(obj) == NULL)
		{
		    obj_best    = obj;
		    max         = obj->cost;
		}
	    }

	    if ( obj_best )
	    {
		obj_from_room( obj_best );
		obj_to_char( obj_best, ch );
		act( "$n gets $p.", ch, obj_best, NULL, TO_ROOM, 0 );
	    }
	}

	/* Wander */
	if ( !IS_SET(ch->act, ACT_SENTINEL) 
	&& number_bits(3) == 0
	&& ( door = number_bits( 5 ) ) <= 5
	&& ( pexit = ch->in_room->exit[door] ) != NULL
	&&   pexit->u1.to_room != NULL
	&&   !IS_SET(pexit->exit_info, EX_CLOSED)
	&&   !IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB)
	&& ( !IS_SET(ch->act, ACT_STAY_AREA)
	||   pexit->u1.to_room->area == ch->in_room->area ) 
	&& ( !IS_SET(ch->act, ACT_OUTDOORS)
	||   !IS_SET(pexit->u1.to_room->room_flags,ROOM_INDOORS)) 
	&& ( !IS_SET(ch->act, ACT_INDOORS)
	||   IS_SET(pexit->u1.to_room->room_flags,ROOM_INDOORS))
	&& ch->fighting == NULL
	&& ch->position >= P_STAND)
	{
	    move_char( ch, door, FALSE );
	}

	/* Timed mob obliteration. */
	if ( ch->timer < 0 || --ch->timer > 0 || !IS_NPC(ch) )
	    continue;

	/* Drop Everything! */

    	act("$n disappears suddenly.", ch, NULL, NULL, TO_ROOM, 0 );
	extract_char( ch, TRUE );

    }
    return;
}


/*
 * Update the weather.
 */
void weather_update( void )
{
	DESCRIPTOR_DATA *d;
	char buf[MSL]={'\0'};
	int diff = 0;

	if(++time_info.min >= 30)
	{
		time_info.min = 0;

		switch ( ++time_info.hour )
		{
		case  5:
			weather_info.sunlight = SUN_LIGHT;
			strncat( buf, "The day has begun.\n\r", sizeof(buf)  );
			break;

		case  6:
			weather_info.sunlight = SUN_RISE;
			strncat( buf, "The sun rises in the east.\n\r", sizeof(buf)  );
			break;

		case 19:
			weather_info.sunlight = SUN_SET;
			strncat( buf, "The sun slowly disappears in the west.\n\r", sizeof(buf)  );
			break;

		case 20:
			weather_info.sunlight = SUN_DARK;
			strncat( buf, "The night has begun.\n\r", sizeof(buf)  );
			break;

		case 24:
			time_info.hour = 0;
			time_info.day++;
			break;
		}
	}

	if ( time_info.day   >= 28 )
	{
		time_info.day = 0;
		time_info.month++;
	}

	if ( time_info.month >= 12 )
	{
		time_info.month = 0;
		time_info.year++;
	}

	/*
	 * Moon Phases.
	 */

	if(time_info.day == 13)
	{
		time_info.moon_phase = MOON_NEW;
	}
	else if(time_info.day == 6 || time_info.day == 20)
	{
		time_info.moon_phase = MOON_HALF;
	}
	else if(time_info.day == 27)
	{
		time_info.moon_phase = MOON_FULL;
	}
	else if( ((time_info.day < 27) && (time_info.day > 20))
			|| ((time_info.day >= 0) && (time_info.day < 6)) )
	{
		time_info.moon_phase = MOON_GIBBOUS;
	}
	else if( ((time_info.day < 20) && (time_info.day > 13))
			|| ((time_info.day > 6) && (time_info.day < 13)) )
	{
		time_info.moon_phase = MOON_CRESCENT;
	}

	/*
	 * Weather change.
	 */
	if ( time_info.month >= 6 && time_info.month <= 12 )
		diff = weather_info.mmhg >  985 ? -2 : 2;
	else
		diff = weather_info.mmhg > 1015 ? -2 : 2;

	weather_info.change   += diff * dice(1, 4) + dice(2, 6) - dice(2, 6);
	weather_info.change    = UMAX(weather_info.change, -12);
	weather_info.change    = UMIN(weather_info.change,  12);

	weather_info.mmhg += weather_info.change;
	weather_info.mmhg  = UMAX(weather_info.mmhg,  960);
	weather_info.mmhg  = UMIN(weather_info.mmhg, 1040);

	switch ( weather_info.sky )
	{
	default:
		log_string(LOG_BUG, Format("Weather_update: bad sky %d.", weather_info.sky ));
		weather_info.sky = SKY_CLOUDLESS;
		break;

	case SKY_CLOUDLESS:
		if ( weather_info.mmhg <  990
				|| ( weather_info.mmhg < 1010 && number_bits( 2 ) == 0 ) )
		{
			strncat( buf, "The sky is getting cloudy.\n\r" , sizeof(buf) );
			weather_info.sky = SKY_CLOUDY;
		}
		break;

	case SKY_CLOUDY:
		if ( weather_info.mmhg <  970
				|| ( weather_info.mmhg <  990 && number_bits( 2 ) == 0 ) )
		{
			strncat( buf, "It starts to rain.\n\r", sizeof(buf)  );
			weather_info.sky = SKY_RAINING;
		}
		else
		{
			strncat( buf, "A fog rolls in.\n\r", sizeof(buf)  );
			weather_info.sky = SKY_FOGGY;
		}

		if ( weather_info.mmhg > 1030 && number_bits( 2 ) == 0 )
		{
			strncat( buf, "The clouds disappear.\n\r", sizeof(buf)  );
			weather_info.sky = SKY_CLOUDLESS;
		}
		break;

	case SKY_RAINING:
		if ( weather_info.mmhg <  970 && number_bits( 2 ) == 0 )
		{
			strncat( buf, "Lightning flashes in the sky.\n\r", sizeof(buf)  );
			weather_info.sky = SKY_LIGHTNING;
		}

		if ( weather_info.mmhg > 1030
				|| ( weather_info.mmhg > 1010 && number_bits( 2 ) == 0 ) )
		{
			strncat( buf, "The rain stopped.\n\r", sizeof(buf)  );
			weather_info.sky = SKY_CLOUDY;
		}
		break;

	case SKY_LIGHTNING:
		if ( weather_info.mmhg > 1010
				|| ( weather_info.mmhg >  990 && number_bits( 2 ) == 0 ) )
		{
			strncat( buf, "The lightning has stopped.\n\r", sizeof(buf)  );
			weather_info.sky = SKY_RAINING;
			break;
		}
		break;

	case SKY_FOGGY:
		if ( weather_info.mmhg > 1010
				|| ( weather_info.mmhg >  990 && number_bits( 2 ) == 0 ) )
		{
			strncat( buf, "The fog clears.\n\r" , sizeof(buf) );
			weather_info.sky = SKY_CLOUDLESS;
			break;
		}
		break;
	}

	if ( !IS_NULLSTR(buf) )
	{
		for ( d = descriptor_list; d != NULL; d = d->next )
		{
			if ( d->connected == CON_PLAYING
					&&   IS_OUTSIDE(d->character)
			&&   IS_AWAKE(d->character) )
				send_to_char( buf, d->character );
		}
	}

	return;
}

char *random_train_message()
{
	int i = 0, sum = 0, ping = 0;

	for(i = 0; train_messages[i].name != NULL; i++) sum += train_messages[i].bit;

	ping = number_range(1, sum);

	sum = 0;
	for(i = 0; train_messages[i].name !=NULL && ping < sum; i++) sum += train_messages[i].bit;

	return train_messages[i].name;
}

/*
 * Update all rooms, for room affects only at this stage.
 * Upgraded to handle elevators and trains 15/11/2001
 */
void room_update( void )
{
	ROOM_INDEX_DATA *rm;
	ROOM_INDEX_DATA *rm_next;
	ROOM_INDEX_DATA *at_stop;
	EXTRA_DESCR_DATA *ed;
	EXTRA_DESCR_DATA *back;
	EXTRA_DESCR_DATA *ed_next = NULL;
	int i = 0;
	
	for ( i = 0; i < MAX_KEY_HASH; i++)
		for ( rm = room_index_hash[i]; rm != NULL; rm = rm_next )
		{
			AFFECT_DATA *paf;
			AFFECT_DATA *paf_next;

			rm_next = rm->next;

			assert(rm); // crash if we are NULL!

			if(IS_SET(rm->room_flags, ROOM_ELEVATOR) && rm->going_to >= 0)
			{
				at_stop = get_room_index(rm->stops[rm->at_stop]);

				if(!IS_SET(rm->room_flags, ROOM_OPENDOORS) && rm->going_to == -1)
				{
					rm->at_stop = 0;
					open_car_doors(rm);
				}

				if(rm->at_stop == rm->going_to)
				{
					rm->going_to = -1;
					open_car_doors(rm);
				}
				else if(rm->at_stop < rm->going_to)
				{
					send_to_room("You hear the elevator go past, heading up.",
							at_stop);
					rm->at_stop++;
				}
				else if(rm->at_stop > rm->going_to)
				{
					send_to_room("You hear the elevator go past, heading down.",
							at_stop);
					rm->at_stop--;
				}
			}

			if(IS_SET(rm->room_flags, ROOM_TRAIN))
			{
				if(!IS_SET(rm->room_flags, ROOM_OPENDOORS) && rm->going_to == -1)
				{
					rm->at_stop = 0;
					rm->car_pause = 3;
					open_car_doors(rm);
				}

				if(rm->going_to >= 0)
				{
					if(rm->car_pause == 0)
					{
						rm->at_stop = rm->going_to;
						rm->car_pause = 3;
						open_car_doors(rm);
						rm->going_to = -1;
					}
					else
					{
						send_to_room(Format("%s\n\r", random_train_message()), rm);
					}
				}
				else
				{
					if(rm->car_pause == 0)
					{
						rm->going_to = rm->at_stop + 1;
						if(rm->stops[rm->going_to] == -1)
							rm->going_to = 0;
						close_car_doors(rm);
						rm->car_pause = 3;
					}
				}
			}

			for ( ed = rm->extra_descr; ed; ed = ed_next )
			{
				if(ed == NULL)
					break;

				ed_next = ed->next;

				if ( ed->timer > 0 )
				{
					ed->timer--;
				}
				else if ( ed->timer < 0 ) ;
				else
				{
					if(rm->extra_descr == ed)
					{
						rm->extra_descr = ed->next;
						free_extra_descr(ed);
					}
					else
						for(back = rm->extra_descr; back; back = back->next)
						{
							if(back->next == ed)
							{
								back->next = ed_next;
								free_extra_descr(ed);
							}
						}
				}
			}

			for ( paf = rm->affects; paf != NULL; paf = paf_next )
			{
				paf_next    = paf->next;
				if ( paf->duration > 0 )
				{
					paf->duration--;
					if (number_range(0,4) == 0 && paf->level > 0)
						paf->level--;  /* spell strength fades with time */
				}
				else if ( paf->duration < 0 )
					;
				else
				{
					if ( paf_next == NULL
							||   paf_next->type != paf->type
							||   paf_next->duration > 0 )
					{
						if ( paf->type > 0 && skill_table[paf->type].msg_off )
						{
							act( skill_table[paf->type].msg_off, NULL, NULL, NULL, TO_ROOM, 0 );
						}
					}

					affect_remove_room( rm, paf );
				}
			}
		}
	return;
}

void fake_out() {}

/*
 * Update all chars, including mobs.
 */
void char_update( void )
{   
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *ch_quit;
    int diff = 0;

    ch_quit	= NULL;

    /* update save counter */
    save_number++;

    if (save_number > 29)
	save_number = 0;

    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;

	ch_next = ch->next;

	if(IS_SET(ch->act, ACT_REINCARNATE))
		continue;

	if(ch->position == P_DEAD)
	{
	    raw_kill(ch, FALSE);
	    continue;
	}

	if(ch->pospts < 100)
	  switch(ch->position)
	  {
	    default : 		ch->pospts++;		break;
	    case P_SLEEP: 	ch->pospts += 3;	break;
	    case P_REST:  	ch->pospts += 5;	break;
 	  }

	ch->dollars += ch->backgrounds[RESOURCES];

	if(--ch->ooc_xp_time <= 0)
	{
	    ch->ooc_xp_time = 60;
	    ch->ooc_xp_count = 0;
	}

	if(!IS_NPC(ch) && IS_VAMPIRE(ch))
	{
	    if(ch->GHB < 7 && !IS_SET(ch->act2, ACT2_WYRM))
		SET_BIT(ch->act2, ACT2_WYRM);
	    else if(ch->GHB > 7 && IS_SET(ch->act2, ACT2_WYRM))
		REMOVE_BIT(ch->act2, ACT2_WYRM);
	}

	if(ch->warrants && ch->in_room == get_room_index(ROOM_VNUM_JAIL))
	{
	    ch->warrants -= UMIN(ch->warrants, 4);
	    if(ch->warrants == 0)
	    {
		char_from_room(ch);
		char_to_room(ch, get_room_index(ROOM_VNUM_POLICE_STATION));
		send_to_char("\tGFreed! \tWYou have been released after serving your time.\tn\n\r", ch);
	    }
	}
	else if(ch->warrants)
	    ch->warrants--;

	if(ch->hunter_vis > 40)
	{
	    /* Generate hunter (ACT2_TWILIGHT, ACT2_HUNTER) and set after ch */
	    if(IS_HUNTED(ch))
		gen_hunter(ch);
	    ch->hunter_vis -= 5;
	}
	else if(ch->hunter_vis)
	    ch->hunter_vis--;

	if (ch->infl_timer > 0)
	{
	    ch->infl_timer--;
	    if(ch->infl_timer == 0)
	    	send_to_char("\tGUpdate: \tWYour influence is now available for use.\tn\n\r", ch);
	}

	if (ch->herd_timer > 0)
	{
		ch->herd_timer--;
		if(ch->herd_timer == 0)
			send_to_char("\tGUpdate: \tWYour herd is now available.\tn\n\r", ch);
	}

	if (ch->power_timer > 0)
	{
	    ch->power_timer--;
	    if(ch->power_timer == 0)
		send_to_char("\tGUpdate: \tWYour powers wax stronger.\tn\n\r", ch);
	}

	if (ch->power_timer == 0)
	{
	    if(ch->race == race_lookup("faerie") && ch->RBPG < ch->max_RBPG)
		ch->RBPG += 1;
	    if(ch->race == race_lookup("werewolf") && ch->RBPG < ch->max_RBPG)
		ch->RBPG += 1;
	    if(ch->race == race_lookup("human") && ch->RBPG < ch->max_RBPG)
		ch->RBPG += 1;
	}

	if(time_info.moon_phase == MOON_FULL 
	  && ch->race == race_lookup("werewolf"))
	{
	    if(ch->RBPG < ch->max_RBPG)
		ch->RBPG = ch->max_RBPG;
	    if(ch->GHB < ch->max_GHB)
		ch->GHB = ch->max_GHB;
	}

        if(--ch->torpor_timer > 0)
	{
	    if(ch->RBPG <= 0) ch->RBPG = ch->max_RBPG;
	}
	else
	{
	    update_pos(ch, 0);
	}

	if(IS_VAMPIRE(ch) && ch->blood_timer == 0 && !IS_SET(ch->act2, ACT2_RP_ING))
	{
	    if(ch->RBPG > -10)
	    {
		ch->RBPG -= 1;
		if(ch->RBPG < 0)
		    send_to_char("\tOYou will slip into torpor without some blood really soon!!!\tn\n\r", ch);
	    }
	    else if(!IS_ADMIN(ch))
		ch->position = P_TORPOR;

	    ch->blood_timer = 20;
	}
	else
	{
	    if(!IS_NPC(ch))
	    ch->blood_timer--;
	}

	if(IS_ADMIN(ch))
	{
	    if(ch->RBPG < ch->max_RBPG)
	    ch->RBPG++;
	    if(ch->race != race_lookup("faerie") && ch->GHB < ch->max_GHB)
	    ch->GHB++;
	    ch->condition[COND_FRENZY] = 0;
	    ch->condition[COND_ANGER] = 0;
	    ch->condition[COND_PAIN] = 0;
	}

	if(ch->willpower < ch->max_willpower)
	{
	    ch->willpower++;
	}

        if ( ch->timer > 30 )
            ch_quit = ch;

	if ( ch->position >= P_STUN )
	{

            /* check to see if we need to go home */
            if (IS_NPC(ch) && ch->zone != NULL && ch->zone != ch->in_room->area
            && ch->desc == NULL &&  ch->fighting == NULL 
	    && !IS_AFFECTED(ch,AFF_CHARM) && number_percent() < 5)
            {
            	act("$n wanders on home.",ch,NULL,NULL,TO_ROOM,0);
            	extract_char(ch,TRUE);
            	continue;
            }

	    if ( ch->agghealth  < MAX_HEALTH )
		ch->agghealth  += hit_gain(ch);
	    else
		ch->agghealth = MAX_HEALTH;
	    
	    if ( ch->health  < MAX_HEALTH )
		ch->health  += hit_gain(ch);
	    else
		ch->health = MAX_HEALTH;
	}

	if(!IS_NPC(ch))
	{
	if(--(ch->pcdata->points) <= 0)
	{
	    ch->pcdata->points = HOURS_PER_EXP;
	    diff = 2;
	    if(IS_ADMIN(ch))
		diff = diff*10;
	    ch->xpgift         += diff;
	}
	}

	if ( ch->position == P_STUN )
	    update_pos( ch, 0 );

	gain_condition( ch, COND_HIGH, -4 );
	gain_condition( ch, COND_TRIPPING, -4 );
	if(ch->condition[COND_HIGH] > 60 && number_range(1, 100) > 67)
	{
	    check_social(ch, "giggle", "");
	}
	if(ch->condition[COND_TRIPPING] > 60 && number_range(1, 100) > 90)
	{
	    check_social(ch, "groan", "");
	}

	gain_condition( ch, COND_ANGER, -4 );
	gain_condition( ch, COND_FEAR, -4 );
	gain_condition( ch, COND_PAIN, -4 );
	gain_condition( ch, COND_FRENZY, -10 );

	if( IS_SET(ch->comm, COMM_TIPS) )
		tip_to_char(ch);

	if ( !IS_NPC(ch) && ch->trust < LEVEL_IMMORTAL )
	{
	    OBJ_DATA *obj;

	    /* Decrement room light when light source dies. */
	    if ( ( obj = get_eq_char( ch, WEAR_HOLD ) ) != NULL
	    &&   obj->item_type == ITEM_LIGHT
	    &&   obj->value[2] > 0 )
	    {
		if ( --obj->value[2] == 0 && ch->in_room != NULL )
		{
		    --ch->in_room->light;
		    act( "$p goes out.", ch, obj, NULL, TO_ROOM, 0 );
		    act( "$p flickers and goes out.", ch, obj, NULL, TO_CHAR,1);
		    extract_obj( obj );
		}
	 	else if ( obj->value[2] <= 5 && ch->in_room != NULL)
		    act("$p flickers.",ch,obj,NULL,TO_CHAR,1);
	    }

	    if ( !IS_NPC(ch) && !IS_ADMIN(ch) && ++ch->timer >= 12 )
	    {
		if ( ch->was_in_room == NULL && ch->in_room != NULL )
		{
		    ch->was_in_room = ch->in_room;
		    if ( ch->fighting != NULL )
			stop_fighting( ch, TRUE );
		    act( "$n disappears into the void.", ch, NULL, NULL, TO_ROOM, 0 );
		    send_to_char( "You disappear into the void.\n\r", ch );
		    if (ch->trust > 0)
		        save_char_obj( ch );
		    char_from_room( ch );
		    char_to_room( ch, get_room_index( ROOM_VNUM_LIMBO ) );
		}
	    }

	/* Deal with dead characters */
	    if ( ch->position == P_DEAD )
	    {
	    raw_kill(ch, FALSE);
	    continue;
	    }

	    if(ch->position == P_TORPOR && IS_VAMPIRE(ch)
		&& (ch->condition[COND_THIRST] <= 0 && ch->RBPG == 0) )
		ch->position = P_DEAD;

	    gain_condition( ch, COND_DRUNK,  -1 );
	    if(ch->condition[COND_HIGH] > 10)
	    {
	        gain_condition( ch, COND_FULL, ch->size > SIZE_MEDIUM ? -10 : -6 );
	        gain_condition( ch, COND_HUNGER, ch->size > SIZE_MEDIUM ? -6 : -4);
	    }
	    else
	    {
	        gain_condition( ch, COND_FULL, ch->size > SIZE_MEDIUM ? -4 : -2 );
	        gain_condition( ch, COND_HUNGER, ch->size > SIZE_MEDIUM ? -2 : -1);
	    }
	    gain_condition( ch, COND_THIRST, -1 );
	    gain_condition( ch, COND_HIGH, -1 );
	    gain_condition( ch, COND_TRIPPING, -1 );
	}

	for ( paf = ch->affected; paf != NULL; paf = paf_next )
	{
	    paf_next	= paf->next;
	    if ( paf->duration > 0 )
	    {
		paf->duration--;
		if (number_range(0,4) == 0 && paf->level > 0)
		  paf->level--;  /* spell strength fades with time */
            }
	    else if ( paf->duration < 0 )
		;
	    else
	    {
		if ( paf_next == NULL
		||   paf_next->type != paf->type
		||   paf_next->duration > 0 )
		{
		    if ( paf->type > 0 && skill_table[paf->type].msg_off )
		    {
			send_to_char( skill_table[paf->type].msg_off, ch );
			send_to_char( "\n\r", ch );
		    }
		}
	  
		affect_remove( ch, paf );
	    }
	}

	/*
	 * Careful with the damages here,
	 *   MUST NOT refer to ch after damage taken,
	 *   as it may be lethal damage (on NPC).
	 */

        if (is_affected(ch, gsn_plague) && ch != NULL)
        {
            AFFECT_DATA *af, plague;
            CHAR_DATA *vch;
            int dam;

	    if (ch->in_room == NULL)
		continue;
 
	    act("$n writhes in agony as plague sores erupt from $s skin.", ch,NULL,NULL,TO_ROOM,0);
	    send_to_char("You writhe in agony from the plague.\n\r",ch);
            for ( af = ch->affected; af != NULL; af = af->next )
            {
            	if (af->type == gsn_plague)
                    break;
            }
        
            if (af == NULL)
            {
            	REMOVE_BIT(ch->affected_by,AFF_PLAGUE);
            	continue;
            }
        
            if (af->level == 1)
            	continue;

	    	plague.where		= TO_AFFECTS;
            plague.type 		= gsn_plague;
            plague.level 		= af->level - 1;
            plague.duration 	= number_range(1,2 * plague.level);
            plague.location		= APPLY_STR;
            plague.modifier 	= -5;
            plague.bitvector 	= AFF_PLAGUE;
        
            for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
            {
                if (!saves_spell(1,vch,DAM_DISEASE) 
		&&  !IS_ADMIN(vch)
            	&&  !IS_AFFECTED(vch,AFF_PLAGUE) && number_bits(4) == 0)
            	{
            	    send_to_char("You feel hot and feverish.\n\r",vch);
            	    act("$n shivers and looks very ill.", vch,NULL,NULL,TO_ROOM,0);
            	    affect_join(vch,&plague);
            	}
            }

	    dam = UMIN(ch->trust,2);
	    damage( ch, ch, dam, gsn_plague,DAM_DISEASE,FALSE,0,-1);
        }
	else if ( IS_AFFECTED(ch, AFF_POISON) && ch != NULL
	     &&   !IS_AFFECTED(ch,AFF_SLOW))

	{
	    AFFECT_DATA *poison;

	    poison = affect_find(ch->affected,gsn_poison);

	    if (poison != NULL)
	    {
	        act( "$n shivers and suffers.", ch, NULL, NULL, TO_ROOM, 0 );
	        send_to_char( "You shiver and suffer.\n\r", ch );
	        damage(ch,ch,poison->level/10 + 1,gsn_poison,
		    DAM_POISON,FALSE,0,-1);
	    }
	}

	else if ( ch->position == P_INCAP && number_range(0,1) == 0)
	{
	    damage( ch, ch, 1, TYPE_UNDEFINED, DAM_NONE,FALSE,0,-1);
	}
	else if ( ch->position == P_MORT )
	{
	    damage( ch, ch, 1, TYPE_UNDEFINED, DAM_NONE,FALSE,0,-1);
	}
    }

    /*
     * Autosave and autoquit.
     * Check that these chars still exist.
     */
    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
        ch_next = ch->next;

	if (ch->desc != NULL && ch->desc->descriptor % 30 == save_number)
	    save_char_obj(ch);

        if ( ch == ch_quit )
            do_function(ch, &do_quit, "" );
    }

    return;
}



/*
 * Update all objs.
 * This function is performance sensitive.
 */
void obj_update( void )
{   
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    AFFECT_DATA *paf, *paf_next;

    for ( obj = object_list; obj != NULL; obj = obj_next )
    {
	CHAR_DATA *rch;
	char *message;

	obj_next = obj->next;

	/* go through affects and decrement */
        for ( paf = obj->affected; paf != NULL; paf = paf_next )
        {
            paf_next    = paf->next;
            if ( paf->duration > 0 )
            {
                paf->duration--;
                if (number_range(0,4) == 0 && paf->level > 0)
                  paf->level--;  /* spell strength fades with time */

		if(affect_find(obj->affected, AFF_HEAT_METAL)
		  && !obj->carried_by)
		{
		    affect_remove_obj( obj, paf );
		}
            }
            else if ( paf->duration < 0 )
                ;
            else
            {
                if ( paf_next == NULL
                ||   paf_next->type != paf->type
                ||   paf_next->duration > 0 )
                {
                    if ( paf->type > 0 && skill_table[paf->type].msg_obj )
                    {
			if (obj->carried_by != NULL)
			{
			    rch = obj->carried_by;
			    act(skill_table[paf->type].msg_obj, rch,obj,NULL,TO_CHAR,1);
			}
			if (obj->in_room != NULL 
			&& obj->in_room->people != NULL)
			{
			    rch = obj->in_room->people;
			    act(skill_table[paf->type].msg_obj, rch,obj,NULL,TO_ALL,0);
			}
                    }
                }

                affect_remove_obj( obj, paf );
            }
	    if(paf_next == obj->affected)
		break;
        }

	if(affect_find(obj->affected, AFF_HEAT_METAL))
	{
	    if(obj->carried_by && !IS_NPC(obj->carried_by)
		&& !IS_AFFECTED(obj->carried_by, AFF_RESIST_PAIN))
	    {
		if(IS_SET(obj->carried_by->act2, ACT2_RESIST))
		{
		    if(dice_rolls(obj->carried_by,
		      obj->carried_by->virtues[virtue_lookup("self-control")],
		      8)
		     && obj->carried_by->condition[COND_PAIN] < 40)
		    {
			gain_condition(obj->carried_by, COND_PAIN, 3);
			act("$o sears where it touches you.", obj->carried_by, obj, NULL, TO_CHAR, 1);
		    }
		    else
		    {
			act("$n drops $o like a hot potato.", obj->carried_by, obj, NULL, TO_ROOM, 0);
			act("You drop $o as it gets too hot.", obj->carried_by, obj, NULL, TO_CHAR, 1);
			do_function(obj->carried_by, &do_drop, obj->name );
		    }
		}
		else
		{
		    act("$n drops $o like a hot potato.", obj->carried_by, obj, NULL, TO_ROOM, 0);
		    act("You drop $o as it gets hot.", obj->carried_by, obj, NULL, TO_CHAR, 1);
		    do_function(obj->carried_by, &do_drop, obj->name );
		}
	    }
	}

	/*fake_out();*/
	if(obj->carried_by && !IS_NPC(obj->carried_by)
	  && obj->condition > 0
	  && !IS_SET(obj->extra_flags, ITEM_INVENTORY)
	  && number_range(0,99999999) == 1)
	    obj->condition--;

	if(material_table[material_lookup(obj->material)].is_edible
	  && obj->condition > 0
	  && number_range(0,10000))
	    obj->condition--;

	if(IS_SET(obj->extra2, OBJ_BURNING))
	{
	    if(IS_SET(obj->extra2, OBJ_SLOW_BURN))
	    {
		obj->condition -= 1;
	    }
	    else if(IS_SET(obj->extra2, OBJ_FAST_BURN))
	    {
		obj->condition -= 5;
	    }
	    else
	    {
		obj->condition -= 3;
	    }
	}


	if(obj->condition <= 0 && !IS_SET(obj->extra2, OBJ_NO_DECAY))
	{
	    obj->timer = 5;
	    if(material_table[material_lookup(obj->material)].is_explosive
	    && IS_SET(obj->extra2, OBJ_BURNING))
	    {
		obj->item_type = ITEM_BOMB;
		obj->value[1] = 0;
		obj->value[0] = 2;
	    }
	}


	/*if ( obj->timer < 0 && obj->condition > 0
	    || --obj->timer != 0 )*/
	if ( obj->timer < 0 || --obj->timer > 0 )
	    continue;

	switch ( obj->item_type )
	{
	default:              message = "$p cracks and crumbles.";  break;
	case ITEM_FOUNTAIN:   message = "$p dries up.";         break;
	case ITEM_CORPSE_NPC: message = "$p decays and rots away."; break;
	case ITEM_CORPSE_PC:  message = "$p decays and rots away."; break;
	case ITEM_FOOD:       message = "$p starts to stink and has to be thrown out."; break;
	case ITEM_POTION:     message = "$p has evaporated from disuse.";	
								break;
	case ITEM_PORTAL:     message = "$p fades out of existence."; break;
	case ITEM_CONTAINER: 
	    message = "$p develops holes and becomes useless.";
	    break;
	case ITEM_CLOTHING:   message = "$p becomes too threadbare to be worn.";
	    break;
	}

	if ( obj->carried_by != NULL )
	{
	    if (IS_NPC(obj->carried_by) 
	    &&  obj->carried_by->pIndexData->pShop != NULL)
		obj->carried_by->cents += obj->cost/5;
	    else
	    {
	    	act( message, obj->carried_by, obj, NULL, TO_CHAR, 1 );
	    }
	}
	else if ( obj->in_room != NULL
	&&      ( rch = obj->in_room->people ) != NULL )
	{
	    if (! (obj->in_obj
	           && !CAN_WEAR(obj->in_obj,ITEM_TAKE)))
	    {
	    	act( message, rch, obj, NULL, TO_ROOM, 0 );
	    	act( message, rch, obj, NULL, TO_CHAR, 1 );
	    }
	}

        if ((obj->item_type == ITEM_CONTAINER
	|| obj->item_type == ITEM_FURNITURE
	|| obj->item_type == ITEM_CORPSE_PC
	|| obj->item_type == ITEM_CORPSE_NPC)
	&&  obj->contains)
	{   /* save the contents */
     	    OBJ_DATA *t_obj, *next_obj;

	    for (t_obj = obj->contains; t_obj != NULL; t_obj = next_obj)
	    {
		next_obj = t_obj->next_content;
		obj_from_obj(t_obj);

		if (obj->in_obj) /* in another object */
		    obj_to_obj(t_obj,obj->in_obj);

		else if (obj->carried_by)  /* carried */
		    	obj_to_char(t_obj,obj->carried_by);

		else if (obj->in_room == NULL)  /* destroy it */
		    extract_obj(t_obj);

		else /* to a room */
		    obj_to_room(t_obj,obj->in_room);
	    }
	}

	extract_obj( obj );
    }

    return;
}



/*
 * Aggress.
 *
 * for each mortal PC
 *     for each mob in room
 *         aggress on some random PC
 *
 * This function takes 25% to 35% of ALL Merc cpu time.
 * Unfortunately, checking on each PC move is too tricky,
 *   because we don't the mob to just attack the first PC
 *   who leads the party into the room.
 *
 * -- Furey
 */
void aggr_update( void )
{
	CHAR_DATA *wch, *wch_next;
	CHAR_DATA *ch, *ch_next;
	CHAR_DATA *vch, *vch_next;
	CHAR_DATA *victim, *prev;

	// say WHA? (Zeroed! by Omega)
	wch = wch_next = ch = ch_next = vch = vch_next = victim = prev = NULL;
	for ( wch = char_list; wch != NULL; wch = wch_next )
	{
		wch_next = wch->next;
		if ( IS_NPC(wch)
				||  wch->trust >= LEVEL_IMMORTAL
				||  wch->in_room == NULL
				||  wch->in_room->area->empty
				||  IS_SET(wch->act, ACT_REINCARNATE)) {
			prev = wch;
			continue;
		}
		for ( ch = wch->in_room->people; ch != NULL; ch = ch_next )
		{
			int count = 0;
			ch_next = ch->next_in_room;
			if ( (!IS_NPC(ch)
					&&  !IS_SET(ch->act2, ACT2_FRENZY))
					||  !IS_SET(ch->act, ACT_AGGRESSIVE)
					||  ch->fighting != NULL
					||  IS_AFFECTED(ch, AFF_CHARM)
					||  !IS_AWAKE(ch)
					||  ( IS_SET(ch->act, ACT_WIMPY) && IS_AWAKE(wch) )
					||  !can_see( ch, wch )
					||  number_bits(1) == 0)
				prev = wch;
			continue;
			/*
			 * Ok we have a 'wch' player character and a 'ch' npc aggressor.
			 * Now make the aggressor fight a RANDOM pc victim in the room,
			 *  giving each 'vch' an equal chance of selection.
			 */
			victim = NULL;
			for ( vch = wch->in_room->people; vch != NULL; vch = vch_next )
			{
				vch_next = vch->next_in_room;
				if ( (!IS_NPC(vch)
						|| !IS_NPC(ch))
						&&  vch->trust < LEVEL_IMMORTAL
						&&  ( !IS_SET(ch->act, ACT_WIMPY) || !IS_AWAKE(vch) )
						&&  can_see( ch, vch ) )
				{
					if ( number_range( 0, count ) == 0 )
						victim = vch;
					count++;
				}
			}
			if ( victim == NULL || IS_SET(victim->act2, ACT2_RP_ING)) {
				prev = wch;
				continue;
			}
			if(IS_NPC(ch) || ch->condition[COND_FEAR] < ch->condition[COND_ANGER])
				multi_hit( ch, victim, TYPE_UNDEFINED );
			else
				do_function(ch, &do_flee, "" );
		}
		prev = wch;
	}
	return;
}


/*
 * Script updater... whatever.
 */
void script_update()
{
	CHAR_DATA *ch;

	for(ch = char_list; ch != NULL; ch = ch->next)
	{
		if(ch->script == NULL)
			continue;

		if(ch->script->active)
		{
			if(--ch->script->delay <= 0)
			{
				mob_interpret(ch, ch->script->reaction);
				free_script(ch->script);
				ch->script = NULL;
			}
		}
	}
}

/*
 * Check to see if a running backup has completed.
 */
bool has_backup_finished()
{
	FILE *fp;

	if ( ( fp = fopen( TEMP_FILE, "r" ) ) == NULL )
	{
		return FALSE;
	}

	if(fp != NULL)
		fclose(fp);

	return TRUE;
}

/*
 * Handle all kinds of updates.
 * Called once per pulse from game loop.
 * Random times to defeat tick-timing clients and players.
 */

void update_handler( void )
{
	static  int     pulse_area;
	static  int     pulse_mobile;
	static  int     pulse_violence;
	static  int     pulse_time;
	static  int     pulse_point;
/*	char buf[MSL]={'\0'}; */
	static  int     pulse_msdp;

	if( reboot_countdown > 0 )
	{
		if( --reboot_countdown <= 0 ) {
			do_function(NULL, &do_reboot, "now" );
			return;
		}
		switch(reboot_countdown)
		{
		case 5*60*PULSE_PER_SECOND:
		do_function(NULL, &do_echo, "Reboot in 5 minutes." );
		break;
		case 4*60*PULSE_PER_SECOND:
		do_function(NULL, &do_echo, "Reboot in 4 minutes." );
		break;
		case 3*60*PULSE_PER_SECOND:
		do_function(NULL, &do_echo, "Reboot in 3 minutes." );
		break;
		case 2*60*PULSE_PER_SECOND:
		do_function(NULL, &do_echo, "Reboot in 2 minutes." );
		break;
		case 1*60*PULSE_PER_SECOND:
		do_function(NULL, &do_echo, "Reboot in 1 minute." );
		break;
		}
	}

	if ( --pulse_area <= 0 )
	{
		pulse_area	= PULSE_AREA;
		/* number_range( PULSE_AREA / 2, 3 * PULSE_AREA / 2 ); */
		area_update	( );
	}

	if ( --pulse_mobile <= 0 && !timestop )
	{
		pulse_mobile	= PULSE_MOBILE;
		mobile_update	( );
	}

	if ( --pulse_violence <= 0 )
	{
		pulse_violence	= PULSE_VIOLENCE;
		violence_update	( );
		room_update      ( );
		if(backup && has_backup_finished())
		{
			backup = FALSE;
			wiznet("\tY[WIZNET]\tn Backup complete.", NULL,NULL,WIZ_LOAD,WIZ_SECURE,3);
		}
		/* @@@@@ if(timestop) */
		script_update();

	}

	if ( --pulse_time <= 0 && !timestop )
	{
		pulse_time	= PULSE_TICK;
		weather_update ( );
	}

	if ( --pulse_point <= 0 )
	{
		pulse_point     = PULSE_TICK;
		stock_update	( );
		/*gen_quest	( FALSE, NULL );*/
	}

	if ( pulse_point == PULSE_TICK && !timestop )
	{
		char_update     ( );
		obj_update      ( );
	}

	if ( --pulse_msdp <= 0 )
	{
		pulse_msdp      = PULSE_PER_SECOND;
		msdp_update();
	}

	/* Re-introduce later... maybe.
    if(current_time == next_election_result)
    {
	run_election();
    }
	 */

	aggr_update( );
	tail_chain( );
	return;
}

void stock_update()
{
	STOCKS *stock;
	int igain = 0;
	int idays = 0;
	int iflux = dice(1,7) - 4;
	int istock_flux = dice(1,9);

	for(stock = stock_list; stock; stock = stock->next)
	{
		if(stock->last_change < current_time - 2*24*60*60)
		{
			stock->last_change = current_time;
			if(stock->phase < 2)
				stock->phase++;
			else if(stock->phase == 2)
				if(iflux <= 0)
					stock->phase--;
				else if(iflux <=2)
					stock->phase = 2;
				else
					stock->phase = 0;
			else
				stock->phase = 0;
		}

		idays = (current_time - stock->last_change)/180;

		stock->upordown = number_range(-1, 1);

		switch(stock->phase)
		{
		case 0:
			break;
		case 1:
			igain = (stock->upordown * (stock->cost + istock_flux * stock->cost + idays)) / 1000;
			stock->cost += igain;
			break;
		case 2:
			igain = ((stock->upordown * stock->cost) * ((idays + istock_flux)*32/100)/1050);
			stock->cost += igain;
			break;
		default: break;
		}
	}
	save_stocks();
}

OBJ_DATA *make_newspapers ()
{
	OBJ_DATA *papers = NULL;
	OBJ_DATA *obj;
	NEWSPAPER *pnews;
	EXTRA_DESCR_DATA *ed;
	char buf[MSL]={'\0'};
	char tmp[MSL]={'\0'};
	int i = 0;
	
	for(pnews = paper_list; pnews; pnews = pnews->next)
	{
		obj = create_object(get_obj_index(OBJ_VNUM_NEWSPAPER));

		snprintf(buf, sizeof(buf), obj->name, pnews->name);

		PURGE_DATA(obj->name);
		obj->name = str_dup(buf);

		snprintf(buf, sizeof(buf), obj->short_descr, pnews->name);
		PURGE_DATA(obj->short_descr);
		obj->short_descr = str_dup(buf);
		snprintf(buf, sizeof(buf), obj->description, pnews->name);
		PURGE_DATA(obj->description);
		obj->description = str_dup(buf);

		obj->cost = pnews->cost;
		for(i = MAX_ARTICLES - 1; i >= 0; i--)
		{
			/* Set extra descriptions from articles */
			NOTE_DATA *article;
			int vnum = 0;

			for(article = news_list; article != NULL; article = article->next)
				if( vnum++ == pnews->articles[i] ) break;

			if(article != NULL && article->successes == 0)
			{
				ALLOC_DATA (ed, EXTRA_DESCR_DATA, 1);
				ed->keyword		= str_dup(Format("Page%d: %s", i, article?article->subject:""));
				ed->description	= str_dup(Format("%s", article?article->text:""));
				ed->next		= obj->extra_descr;
				obj->extra_descr	= ed;
			}
		}

		snprintf(tmp, sizeof(tmp), "Contents:\n\r");
		for(ed=obj->extra_descr;ed;ed=ed->next)
		{
			snprintf(tmp, MSL, "%s%s\n", tmp, ed->keyword);
		}

		PURGE_DATA(obj->full_desc);
		obj->full_desc = str_dup(Format("%s\n\n%s", obj->full_desc, tmp));
		obj->wear_loc = -1;

		object_list = obj->next;
		obj->next = papers;
		papers = obj;
	}

	return papers;
}

int update_news_stands ()
{
	CHAR_DATA *seller;
	OBJ_DATA  *published = make_newspapers();
	OBJ_DATA  *obj;
	OBJ_DATA  *new_ob = NULL;
	EXTRA_DESCR_DATA *ed, *objed;

	for(seller = char_list; seller; seller = seller->next)
	{
		if(IS_SET(seller->act2, ACT2_NEWS_STAND)) {
			for(obj = published; obj; obj = obj->next)
			{
				/* clone objects */
				new_ob = create_object(get_obj_index(OBJ_VNUM_NEWSPAPER));

				PURGE_DATA(new_ob->name);
				new_ob->name = str_dup(obj->name);

				PURGE_DATA(new_ob->short_descr);
				new_ob->short_descr = str_dup(obj->short_descr);

				PURGE_DATA(new_ob->description);
				new_ob->description = str_dup(obj->description);

				new_ob->cost = obj->cost;

				for(objed=obj->extra_descr;objed;objed=objed->next)
				{
					/* Set extra descriptions for articles */
					ALLOC_DATA(ed, EXTRA_DESCR_DATA, 1);
					ed->keyword         = str_dup(objed->keyword);
					ed->description     = str_dup(objed->description);
					LINK_SINGLE(ed, next, new_ob->extra_descr);
				}

				SET_BIT( new_ob->extra_flags, ITEM_INVENTORY );
				new_ob->wear_loc = -1;
				PURGE_DATA(new_ob->full_desc);
				new_ob->full_desc = str_dup(obj->full_desc);

				obj_to_char( new_ob, seller );
			}
		}
	}

	/* Destroy newspaper objects */
	new_ob = published;
	for(obj = published; new_ob; obj = new_ob)
	{
		new_ob = obj->next;
		free_obj(obj);
	}

	return TRUE;
}

void jump_update(CHAR_DATA *ch, int falling)
{
	int door = ch->jump_dir;
	ROOM_INDEX_DATA *jump_to = ch->in_room;

	if(ch->in_room->exit[door] != NULL)
		ch->jump_timer = 0;

	if (IS_AFFECTED(ch, AFF_SNEAK)
			&& (dice_rolls(ch, (get_curr_stat(ch,STAT_DEX) + ch->ability[STEALTH].value),9) <= 0))
		REMOVE_BIT(ch->affected_by,AFF_SNEAK);

	/* Fail condition pt. 1 */
	if(ch->jump_timer <= 0 && !falling && !IS_SET(ch->form, FORM_SHADOW))
	{
		ch->jump_dir = DIR_DOWN;
		jump_update(ch, TRUE);
		return;
	}

	if(falling)
	{
		if(ch->in_room->sector_type == SECT_AIR)
			ch->falldam += 2;
	}

	if ( !IS_AFFECTED(ch, AFF_SNEAK)
			&&   ch->invis_level < LEVEL_IMMORTAL)
	{
		if(!IS_AFFECTED(ch, AFF_INVISIBLE) || dice_rolls(ch,
				get_curr_stat(ch, STAT_DEX) + ch->ability[STEALTH].value,
				7) < 1) {
			if(falling)
			{
				if(!LOOKS_DIFFERENT(ch))
					act( "$n falls $T.", ch, NULL, dir_name[door], TO_ROOM, 0 );
				else
				{
					act( Format("%s falls %s.", ch->alt_name, dir_name[door]), ch, NULL, NULL, TO_ROOM, 0 );
				}
			}
			else
			{
				if(!LOOKS_DIFFERENT(ch))
					act( "$n soars $T.", ch, NULL, dir_name[door], TO_ROOM, 0 );
				else
				{
					act( Format("%s soars %s.", ch->alt_name, dir_name[door]), ch, NULL, NULL, TO_ROOM, 0 );
				}
			}
		}
	}

	if(ch->in_room != jump_to)
	{
		char_from_room( ch );
		char_to_room( ch, jump_to );
		ch->jump_timer--;
	}

	if ( !IS_AFFECTED(ch, AFF_SNEAK)
			&&   ch->invis_level < LEVEL_IMMORTAL)
	{
		if(!IS_AFFECTED(ch, AFF_INVISIBLE) || dice_rolls(ch,
				get_curr_stat(ch, STAT_DEX) + ch->ability[STEALTH].value,
				9) < 1) {
			if(falling)
			{
				if(!LOOKS_DIFFERENT(ch))
				{
					act( "$n plummets in from above.", ch, NULL, NULL, TO_ROOM, 0 );
					if(ch->in_room->sector_type != SECT_AIR)
						act("That's GOTTA hurt!", ch, NULL, NULL, TO_ROOM, 0);
				}
				else
				{
					act( Format("%s plummets in from above.", ch->alt_name), ch, NULL, NULL, TO_ROOM, 0 );
					if(ch->in_room->sector_type != SECT_AIR)
						act("That's GOTTA hurt!", ch, NULL, NULL, TO_ROOM, 0);
				}
			}
			else
			{
				if(!LOOKS_DIFFERENT(ch))
					act( "$n leaps in from the $t.", ch, dir_name[rev_dir[door]], NULL, TO_ROOM, 0 );
				else
				{
					act( Format("%s leaps in from the $t.", ch->alt_name), ch, dir_name[rev_dir[door]], NULL, TO_ROOM, 0 );
				}
			}
		}
	}

	if(ch->in_room != jump_to)
		do_function(ch, &do_look, "auto" );
	if(ch->in_room->sector_type != SECT_AIR && falling)
	{
		send_to_char("Gravity is a harsh mistress.\n\r", ch);
		if(ch->in_room->sector_type == SECT_WATER_SWIM)
		{
			send_to_char("You splash down... yep... it still hurts.\n\r", ch);
			damage(ch, ch, ch->falldam/2, gsn_bash, DAM_BASH, TRUE, 0, -1);
		}
		else if(ch->in_room->sector_type == SECT_WATER_NOSWIM)
		{
			send_to_char("You splash down... yep... it still hurts.\n\r", ch);
			damage(ch, ch, ch->falldam/8, gsn_bash, DAM_BASH, TRUE, 0, -1);
		}
		else
		{
			send_to_char("You hear something crunch as you hit the ground.\n\r", ch);
			damage(ch, ch, ch->falldam, gsn_bash, DAM_BASH, TRUE, 0, -1);
		}
	}
}

void msdp_update( void )
{
    DESCRIPTOR_DATA *d;
    int PlayerCount = 0;

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->character && d->connected == CON_PLAYING && !IS_NPC(d->character) )
        {
            char buf[MAX_STRING_LENGTH];
            CHAR_DATA *pOpponent = d->character->fighting;
            ROOM_INDEX_DATA *pRoom = d->character->in_room;
            AFFECT_DATA *paf;

            ++PlayerCount;

            MSDPSetString( d, eMSDP_CHARACTER_NAME, d->character->name );
            MSDPSetNumber( d, eMSDP_EXPERIENCE, d->character->exp );
            MSDPSetNumber( d, eMSDP_BRG, d->character->RBPG );
            MSDPSetNumber( d, eMSDP_BRG_MAX, d->character->max_RBPG );
            MSDPSetNumber( d, eMSDP_WILLPOWER, d->character->willpower );
            MSDPSetNumber( d, eMSDP_WILLPOWER_MAX, d->character->max_willpower );
            MSDPSetNumber( d, eMSDP_DOLLARS, d->character->dollars );
            MSDPSetNumber( d, eMSDP_CENTS, d->character->cents );
            MSDPSetString( d, eMSDP_CLAN, capitalize(clan_table[d->character->clan].name ) );
            MSDPSetNumber( d, eMSDP_STR, get_curr_stat(d->character, STAT_STR) );
            MSDPSetNumber( d, eMSDP_DEX, get_curr_stat(d->character, STAT_DEX) );
            MSDPSetNumber( d, eMSDP_STA, get_curr_stat(d->character, STAT_STA) );
            MSDPSetNumber( d, eMSDP_CHA, get_curr_stat(d->character, STAT_CHA) );
            MSDPSetNumber( d, eMSDP_MAN, get_curr_stat(d->character, STAT_MAN) );
            MSDPSetNumber( d, eMSDP_APP, get_curr_stat(d->character, STAT_APP) );
            MSDPSetNumber( d, eMSDP_INT, get_curr_stat(d->character, STAT_INT) );
            MSDPSetNumber( d, eMSDP_PER, get_curr_stat(d->character, STAT_PER) );
            MSDPSetNumber( d, eMSDP_WIT, get_curr_stat(d->character, STAT_WIT) );
            MSDPSetNumber( d, eMSDP_GEN, d->character->gen );
            MSDPSetNumber( d, eMSDP_GHB, d->character->GHB );
            MSDPSetNumber( d, eMSDP_GHB_MAX, d->character->max_GHB );
            MSDPSetString( d, eMSDP_RACE, capitalize(race_table[d->character->race].name ) );
            MSDPSetString( d, eMSDP_AUSPICE, capitalize(auspice_table[d->character->auspice].name) );
            MSDPSetString( d, eMSDP_BREED, capitalize(breed_table[d->character->breed].name) );
            MSDPSetString( d, eMSDP_HEALTH, capitalize(health_string(d->character)) );

            /*
            MSDPSetNumber( d, eMSDP_HEALTH, d->character->hit );
            MSDPSetNumber( d, eMSDP_HEALTH_MAX, d->character->max_hit );
            */

            /* This would be better moved elsewhere */
            if ( pOpponent != NULL )
            {
                /*
                int hit_points = (pOpponent->hit * 100) / pOpponent->max_hit;
                MSDPSetNumber( d, eMSDP_OPPONENT_HEALTH, hit_points );
                */
                MSDPSetString( d, eMSDP_OPPONENT_NAME, pOpponent->name );
            }
            else /* Clear the values */
            {
                MSDPSetString( d, eMSDP_OPPONENT_NAME, "" );
            }

            /* Only update room stuff if they've changed room */
            if ( pRoom && pRoom->vnum != d->pProtocol->pVariables[eMSDP_ROOM_VNUM]->ValueInt )
            {
                int i; /* Loop counter */
                buf[0] = '\0';

                for ( i = DIR_NORTH; i < MAX_DIR; ++i )
                {
                    if ( pRoom->exit[i] != NULL )
                    {
                        const char MsdpVar[] = { (char)MSDP_VAR, '\0' };
                        const char MsdpVal[] = { (char)MSDP_VAL, '\0' };
                        extern char *const dir_name[];

                        strcat( buf, MsdpVar );
                        strcat( buf, dir_name[i] );
                        strcat( buf, MsdpVal );

                        if ( IS_SET(pRoom->exit[i]->exit_info, EX_CLOSED) )
                            strcat( buf, "C" );
                        else /* The exit is open */
                            strcat( buf, "O" );
                    }
                }

                if ( pRoom->area != NULL )
                    MSDPSetString( d, eMSDP_AREA_NAME, pRoom->area->name );

                MSDPSetString( d, eMSDP_ROOM_NAME, pRoom->name );
                MSDPSetTable( d, eMSDP_ROOM_EXITS, buf );
                MSDPSetNumber( d, eMSDP_ROOM_VNUM, pRoom->vnum );
            }
/*
            MSDPSetNumber( d, eMSDP_WORLD_TIME, d->character-> );
*/

            buf[0] = '\0';
            for ( paf = d->character->affected; paf; paf = paf->next )
            {
                char skill_buf[MAX_STRING_LENGTH];
                sprintf( skill_buf, "%c%s%c%d",
                    (char)MSDP_VAR, skill_table[paf->type].name, 
                    (char)MSDP_VAL, paf->duration );
                strcat( buf, skill_buf );
            }
            MSDPSetTable( d, eMSDP_AFFECTS, buf );

            MSDPUpdate( d );
        }
    }

    /* Ideally this should be called once at startup, and again whenever 
     * someone leaves or joins the mud.  But this works, and it keeps the 
     * snippet simple.  Optimise as you see fit.
     */
    MSSPSetPlayers( PlayerCount );
}

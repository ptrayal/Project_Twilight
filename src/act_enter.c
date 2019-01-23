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
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdlib.h>
#include "twilight.h"
#include "tables.h"
#include "lookup.h"
#include "recycle.h"
#include "interp.h"

void free_obj(OBJ_DATA *obj);
int find_door( CHAR_DATA *ch, char *arg );
int flag_value( const struct flag_type *flag_table, char *argument);
char *flag_string( const struct flag_type *flag_table, int bits );
void fwrite_org(ORG_DATA *org);

/* random room generation procedure */
ROOM_INDEX_DATA  *get_random_room(CHAR_DATA *ch)
{
    ROOM_INDEX_DATA *room;

    for ( ; ; )
    {
        room = get_room_index( number_range( 0, /*65535*/ 10000 ) );
        if ( room != NULL )
        if ( can_see_room(ch,room)
	&&   !room_is_private(room)
	&&   !IS_SET(room->room_flags, ROOM_PRIVATE)
	&&   !IS_SET(room->room_flags, ROOM_SOLITARY)
	&&   (IS_NPC(ch) || IS_SET(ch->act,ACT_AGGRESSIVE) ) )
            break;
    }

    return room;
}

/* RT Enter portals */
void do_enter( CHAR_DATA *ch, char *argument)
{
    ROOM_INDEX_DATA *location;

    CheckCH(ch);

    if ( ch->fighting != NULL )
	return;

    /* nifty portal stuff */
    if (!IS_NULLSTR(argument))
    {
        ROOM_INDEX_DATA *old_room;
	OBJ_DATA *portal;
	CHAR_DATA *fch, *fch_next;

        old_room = ch->in_room;

	portal = get_obj_list( ch, argument,  ch->in_room->contents );

	if (portal == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}

	if (portal->item_type != ITEM_PORTAL
        ||  (IS_SET(portal->value[1],EX_CLOSED)
	&& !IS_TRUSTED(ch,LEVEL_IMMORTAL)))
	{
	    send_to_char("You can't seem to find a way in.\n\r",ch);
	    return;
	}

	if (!IS_SET(portal->value[2],GATE_NOCURSE)
	&&  (IS_AFFECTED(ch,AFF_CURSE)
	||   IS_SET(old_room->room_flags,ROOM_NO_RECALL)))
	{
	    send_to_char("Something prevents you from leaving...\n\r",ch);
	    return;
	}

	if (IS_SET(portal->value[2],GATE_RANDOM) || portal->value[3] == -1)
	{
	    location = get_random_room(ch);
	    portal->value[3] = location->vnum; /* for record keeping :) */
	}
	else if (IS_SET(portal->value[2],GATE_BUGGY) && (number_percent() < 5))
	    location = get_random_room(ch);
	else
	    location = get_room_index(portal->value[3]);

	if (location == NULL
	||  location == old_room
	||  !can_see_room(ch,location)
	||  (room_is_private(location) && !IS_TRUSTED(ch,IMPLEMENTOR)))
	{
	   act("$p doesn't seem to go anywhere.",ch,portal,NULL,TO_CHAR,1);
	   return;
	}

        if (IS_NPC(ch) && IS_SET(ch->act,ACT_AGGRESSIVE))
        {
            send_to_char("Something prevents you from leaving...\n\r",ch);
            return;
        }

	act("$n steps into $p.",ch,portal,NULL,TO_ROOM,0);

	if (IS_SET(portal->value[2],GATE_NORMAL_EXIT))
	    act("You enter $p.",ch,portal,NULL,TO_CHAR,1);
	else
	    act("You walk through $p and find yourself somewhere else...", ch,portal,NULL,TO_CHAR,1);

	char_from_room(ch);
	char_to_room(ch, location);

	if (IS_SET(portal->value[2],GATE_GOWITH)) /* take the gate along */
	{
	    obj_from_room(portal);
	    obj_to_room(portal,location);
	}

	if (IS_SET(portal->value[2],GATE_NORMAL_EXIT))
	    act("$n has arrived.",ch,portal,NULL,TO_ROOM,0);
	else
	    act("$n has arrived through $p.",ch,portal,NULL,TO_ROOM,0);

	do_function(ch, &do_look, "auto");

	/* charges */
	if (portal->value[0] > 0)
	{
	    portal->value[0]--;
	    if (portal->value[0] == 0)
		portal->value[0] = -1;
	}

	/* protect against circular follows */
	if (old_room == location)
	    return;

    	for ( fch = old_room->people; fch != NULL; fch = fch_next )
    	{
            fch_next = fch->next_in_room;

            if (portal == NULL || portal->value[0] == -1)
	    /* no following through dead portals */
                continue;

            if ( fch->master == ch && IS_AFFECTED(fch,AFF_CHARM)
            &&   fch->position < P_STAND)
		do_function(ch, &do_look, "");

            if ( fch->master == ch && fch->position == P_STAND)
            {

                if (IS_NPC(fch) && IS_SET(fch->act,ACT_AGGRESSIVE))
                {
                    act("You can't bring $N into the city.", ch,NULL,fch,TO_CHAR,1);
                    act("You aren't allowed in the city.", fch,NULL,NULL,TO_CHAR,1);
                    continue;
            	}

            	act( "You follow $N.", fch, NULL, ch, TO_CHAR, 1 );
		do_enter(fch,argument);
		do_function(fch, &do_enter, argument);
            }
    	}

 	if (portal != NULL && portal->value[0] == -1
	&& !IS_SET(portal->value[2], GATE_CAR_DOOR))
	{
	    act("$p fades out of existence.",ch,portal,NULL,TO_CHAR,0);
	    if (ch->in_room == old_room)
		act("$p fades out of existence.",ch,portal,NULL,TO_ROOM,0);
	    else if (old_room->people != NULL)
	    {
		act("$p fades out of existence.", old_room->people,portal,NULL,TO_CHAR,0);
		act("$p fades out of existence.", old_room->people,portal,NULL,TO_ROOM,0);
	    }
	    extract_obj(portal);
	}

	/*
	 * If someone is following the char, these triggers get activated
	 * for the followers before the char, but it's safer this way...
	 */
	if ( IS_NPC( ch ) && HAS_TRIGGER( ch, TRIG_ENTRY ) )
	    mp_percent_trigger( ch, NULL, NULL, NULL, TRIG_ENTRY );
	if ( !IS_NPC( ch ) )
	    mp_greet_trigger( ch );

	return;
    }

    send_to_char("Nope, can't do it.\n\r",ch);
    return;
}

/*
 * Stuff for elevators or trains.
 */
void close_car_doors(ROOM_INDEX_DATA *car)
{
	OBJ_DATA *portal;
	ROOM_INDEX_DATA *stop;
	bool found = FALSE;

	for(portal = car->contents; portal != NULL; portal = portal->next_content)
	{
		if(portal->pIndexData->vnum == OBJ_VNUM_CAR_DOORS)
		{
			found = TRUE;
			break;
		}
	}

	if(!found) return;

	stop = get_room_index(portal->value[3]);

	extract_obj(portal);

	for(portal = stop->contents; portal != NULL; portal = portal->next_content)
	{
		if(portal->pIndexData->vnum == OBJ_VNUM_CAR_DOORS)
			break;
	}

	if(portal !=NULL) extract_obj(portal);

	send_to_room("The doors close.\n\r", car);
	send_to_room("The doors close.\n\r", stop);
	REMOVE_BIT(car->room_flags, ROOM_OPENDOORS);

	if(IS_SET(car->room_flags, ROOM_TRAIN))
	{
		send_to_room("The train pulls away from the platform.\n\r", car);
		send_to_room("The train leaves the station.\n\r", stop);
	}
	else if(IS_SET(car->room_flags, ROOM_ELEVATOR))
	{
		send_to_room("The elevator moves.\n\r", car);
		send_to_room("You can hear the elevator start to move.\n\r", stop);
	}
}

void open_car_doors(ROOM_INDEX_DATA *car)
{
	OBJ_DATA *portal;
	ROOM_INDEX_DATA *to_room, *from_room;
	char buf[MSL]={'\0'};
	char buf2[MSL]={'\0'};
	char name[MSL]={'\0'};
	char name2[MSL]={'\0'};
	char tmp[MSL]={'\0'};

	from_room = car;
	to_room = get_room_index(car->stops[car->at_stop]);
	SET_BIT(car->room_flags, ROOM_OPENDOORS);

	if(to_room == NULL) return;

	if(IS_SET(car->room_flags, ROOM_TRAIN))
	{
		snprintf(buf, sizeof(buf), "The train pulls up alongside the platform at %s and the doors open.",	to_room->name);
		snprintf(buf2, sizeof(buf2), "The train pulls into the station and the doors open.");
		snprintf(name, sizeof(name), "train");
		snprintf(name2, sizeof(name2), "station");
	}
	else if(IS_SET(car->room_flags, ROOM_ELEVATOR))
	{
		snprintf(buf, sizeof(buf), "The elevator stops at %s and the doors open.", to_room->name);
		snprintf(buf2, sizeof(buf2), "The elevator doors open.");
		snprintf(name, sizeof(name), "elevator");
		snprintf(name2, sizeof(name2), "floor");
	}

	/* portal one */
	portal = create_object(get_obj_index(OBJ_VNUM_CAR_DOORS));
	portal->timer = -1;
	portal->value[3] = to_room->vnum;
	snprintf(tmp, sizeof(tmp), portal->description, name);
	PURGE_DATA(portal->description);
	portal->description = str_dup(tmp);
	snprintf(tmp, sizeof(tmp), portal->full_desc, name2);
	PURGE_DATA(portal->full_desc);
	portal->full_desc = str_dup(tmp);
	snprintf(tmp, sizeof(tmp), portal->name, name);
	PURGE_DATA(portal->name);
	portal->name = str_dup(tmp);
	SET_BIT(portal->value[2], GATE_CAR_DOOR);

	obj_to_room(portal,from_room);

	if (from_room->people != NULL)
	{
		act("$T.",from_room->people,NULL,buf,TO_ROOM,0);
		act("$T.",from_room->people,NULL,buf,TO_CHAR,0);
	}

	/* portal two */
	portal = create_object(get_obj_index(OBJ_VNUM_CAR_DOORS));
	portal->timer = -1;
	portal->value[3] = from_room->vnum;
	snprintf(tmp, sizeof(tmp), portal->description, name);
	PURGE_DATA(portal->description);
	portal->description = str_dup(tmp);
	snprintf(tmp, sizeof(tmp), portal->full_desc, name);
	PURGE_DATA(portal->full_desc);
	portal->full_desc = str_dup(tmp);
	snprintf(tmp, sizeof(tmp), portal->name, name);
	PURGE_DATA(portal->name);
	portal->name = str_dup(tmp);
	SET_BIT(portal->value[2], GATE_CAR_DOOR);

	obj_to_room(portal,to_room);

	if (to_room->people != NULL)
	{
		act("$T.",to_room->people,NULL,buf2,TO_ROOM,0);
		act("$T.",to_room->people,NULL,buf2,TO_CHAR,0);
	}
}

int get_stop(ROOM_INDEX_DATA *car, ROOM_INDEX_DATA *room)
{
	int i = 0;

	for(i = 0; i < MAX_CAR_STOPS; i++)
	{
		if(car->stops[i] == room->vnum)
		{
			return i;
		}
	}

	return -1;
}

void do_button_press(CHAR_DATA *ch, char *argument)
{
	ROOM_INDEX_DATA *room = ch->in_room;
	ROOM_INDEX_DATA *car = get_room_index(ch->in_room->car);
	int i = -1;

	CheckCH(ch);

	if((!IS_SET(room->room_flags, ROOM_STOP)
			&& !IS_SET(room->room_flags, ROOM_ELEVATOR))
			|| (IS_SET(room->room_flags, ROOM_STOP)
					&& !IS_SET(car->room_flags, ROOM_ELEVATOR)))
	{
		send_to_char("There are no buttons to press here.\n\r", ch);
		return;
	}

	if(IS_SET(room->room_flags, ROOM_STOP))
	{
		if(car == NULL)
		{
			send_to_char("The elevator is out of service.\n\r", ch);
			return;
		}

		if(car->going_to == -1 && car->stops[car->at_stop] == room->vnum)
		{
			send_to_char("The elevator is already here.\n\r", ch);
			return;
		}

		if(car->going_to > -1 && car->stops[car->going_to] != room->vnum)
		{
			send_to_char("The elevator is going somewhere else.\n\r", ch);
			return;
		}

		if(car->going_to > -1 && car->stops[car->going_to] == room->vnum)
		{
			send_to_char("The elevator is already on its way.\n\r", ch);
			return;
		}

		close_car_doors(car);
		if((car->going_to = get_stop(car, room)) < 0)
		{
			send_to_char("The elevator seems to be broken.\n\r", ch);
			return;
		}
		act("You hear the sound of the elevator moving.", ch, NULL, NULL, TO_ROOM,0);
		act("You hear the sound of the elevator moving.", ch, NULL, NULL, TO_CHAR,0);
		if(car->people)
			act("You hear the sound of the elevator moving.", car->people, NULL, NULL, TO_ROOM,0);
		return;
	}
	else if(IS_SET(room->room_flags, ROOM_ELEVATOR))
	{
		if(IS_NULLSTR(argument) || !is_number(argument))
		{
			send_to_char("What button do you want to press?\n\r", ch);
			return;
		}

		if(room->going_to == -1
				&& room->at_stop == (i = atoi(argument)))
		{
			send_to_char("That's where you are already.\n\r", ch);
			return;
		}

		if(room->going_to > -1)
		{
			send_to_char("The elevator is already in motion.\n\r", ch);
			return;
		}

		if(room->stops[i] <= 0)
		{
			send_to_char("There aren't that many buttons!\n\r", ch);
			return;
		}

		close_car_doors(room);
		room->going_to = i;
		act("You hear the sound of the elevator moving.", ch, NULL, NULL, TO_ROOM,0);
		return;
	}
}

void do_smash(CHAR_DATA *ch, char *argument)
{
	int i = 0;
	char *door;
	ROOM_INDEX_DATA *to_room;

	CheckCH(ch);

	if(IS_NULLSTR(argument))
	{
		send_to_char("What are you trying to smash?\n\r", ch);
		return;
	}

	if((i = find_door(ch, argument)) == -1)
	{
		send_to_char("No such door.\n\r", ch);
		return;
	}

	if(!IS_SET(ch->in_room->exit[i]->exit_info, EX_CLOSED))
	{
		send_to_char("That door is open.\n\r", ch);
		return;
	}

	if(IS_SET(ch->in_room->exit[i]->exit_info, EX_BROKEN))
	{
		send_to_char("That door is already broken.\n\r", ch);
		return;
	}

	if(IS_SET(ch->in_room->exit[i]->exit_info, EX_NOBREAK))
	{
		send_to_char("You don't seem to be able to crack it.\n\r", ch);
		return;
	}

	if(!IS_NULLSTR(ch->in_room->exit[i]->description))
		door = ch->in_room->exit[i]->keyword;
	else if(IS_SET(ch->in_room->exit[i]->exit_info, EX_WINDOW))
		door = "the window";
	else door = "the door";

	if(dice_rolls(ch,get_curr_stat(ch,STAT_STR)+ch->ability[ATHLETICS].value,8) > 0)
	{
		SET_BIT(ch->in_room->exit[i]->exit_info, EX_BROKEN);
		REMOVE_BIT(ch->in_room->exit[i]->exit_info, EX_CLOSED);
		act("You slam into $t, smashing it.", ch, door, NULL, TO_CHAR,0);
		act("$n slams into $t to the $T, smashing it.", ch, door, dir_name[i], TO_ROOM,0);
		act("$t to the $T, is smashed by something you can't see.", ch, door, dir_name[i], TO_OPLANE,0);

	/* other side */
		if ( ( to_room = ch->in_room->exit[i]->u1.to_room ) != NULL
			&&   ( to_room->exit[rev_dir[i]] ) != NULL
			&& to_room->exit[rev_dir[i]]->u1.to_room == ch->in_room )
		{
			CHAR_DATA *rch;

			REMOVE_BIT( to_room->exit[rev_dir[i]]->exit_info, EX_CLOSED );
			for ( rch = ch->in_room->exit[i]->u1.to_room->people;
				rch != NULL; rch = rch->next_in_room )
				act( "The $d is smashed from the other side.", rch, NULL, to_room->exit[rev_dir[i]]->keyword, TO_CHAR, 0 );
		}
	}
	else
	{
		act("You slam into $t, hurting yourself.", ch, door, NULL, TO_CHAR,0);
		act("$n slams into $t to the $T, hurting $mself.", ch, door, dir_name[i], TO_ROOM,0);
		gain_condition(ch, COND_PAIN, 1);
	}
}

void do_reject( CHAR_DATA *ch, char *argument )
{
	char name[MSL]={'\0'};
	char buf[MSL]={'\0'};

	CheckCH(ch);

	one_argument( argument, name );

	if ( IS_NULLSTR(name) )
	{
		send_to_char( "Syntax:  reject [$name]  -toggles reject\n\r", ch );
		send_to_char( "Your current reject list is:\n\r", ch);
		send_to_char( ch->pcdata->ignore_reject, ch);
		send_to_char( "\n\r", ch);
		return;
	}

    name[0] = UPPER( name[0] );

    if ( strstr( ch->pcdata->ignore_reject, name ) != NULL )
    {
        ch->pcdata->ignore_reject = string_replace(
	    ch->pcdata->ignore_reject, name, "\0" );
        ch->pcdata->ignore_reject = string_unpad( ch->pcdata->ignore_reject );

        if ( ch->pcdata->ignore_reject[0] == '\0' )
        {
        	PURGE_DATA( ch->pcdata->ignore_reject );
            ch->pcdata->ignore_reject = str_dup( "None" );
        }
        send_to_char( "Reject flagging removed.\n\r", ch );
        return;
    }
    else
    {
        buf[0] = '\0';
        if ( strstr( ch->pcdata->ignore_reject, "None" ) != NULL )
        {
            ch->pcdata->ignore_reject = string_replace(
		ch->pcdata->ignore_reject, "None", "\0" );
            ch->pcdata->ignore_reject = string_unpad(
		ch->pcdata->ignore_reject );
        }

        if (!IS_NULLSTR(ch->pcdata->ignore_reject) )
        {
            strncat( buf, ch->pcdata->ignore_reject, sizeof(buf) - strlen(buf) - 1 );
            strncat( buf, " ", sizeof(buf) - strlen(buf) - 1 );
        }
        strncat( buf, name,sizeof(buf) - strlen(buf) - 1 );
        PURGE_DATA( ch->pcdata->ignore_reject );
        ch->pcdata->ignore_reject = string_proper( str_dup( buf ) );

        send_to_char( "Reject flagging added.\n\r", ch );
        send_to_char( ch->pcdata->ignore_reject,ch);
        return;
    }

    return;
}

void do_block( CHAR_DATA *ch, char *argument )
{
    char name[MSL]={'\0'};
    char buf[MSL]={'\0'};

    CheckCH(ch);

    one_argument( argument, name );

    if ( IS_NULLSTR(name) )
    {
    	send_to_char( "Syntax:  block [$name]  -toggles block\n\r", ch );
    	send_to_char( "Your current rpjoin block list is:\n\r", ch);
    	send_to_char( ch->pcdata->block_join, ch);
    	send_to_char( "\n\r", ch);
    	return;
    }

    name[0] = UPPER( name[0] );

    if ( strstr( ch->pcdata->block_join, name ) != NULL )
    {
        ch->pcdata->block_join = string_replace(
	    ch->pcdata->block_join, name, "\0" );
        ch->pcdata->block_join = string_unpad( ch->pcdata->block_join );

        if ( ch->pcdata->block_join[0] == '\0' )
        {
        	PURGE_DATA( ch->pcdata->block_join );
            ch->pcdata->block_join = str_dup( "None" );
        }
        send_to_char( "Block flagging removed.\n\r", ch );
        return;
    }
    else
    {
        buf[0] = '\0';
        if ( strstr( ch->pcdata->block_join, "None" ) != NULL )
        {
            ch->pcdata->block_join = string_replace(
		ch->pcdata->block_join, "None", "\0" );
            ch->pcdata->block_join = string_unpad(
		ch->pcdata->block_join );
        }

        if (!IS_NULLSTR(ch->pcdata->block_join) )
        {
            strncat( buf, ch->pcdata->block_join, sizeof(buf) - strlen(buf) - 1 );
            strncat( buf, " ", sizeof(buf) - strlen(buf) - 1 );
        }
        strncat( buf, name, sizeof(buf) - strlen(buf) - 1 );
        PURGE_DATA( ch->pcdata->block_join );
        ch->pcdata->block_join = string_proper( str_dup( buf ) );

        send_to_char( "Block flagging added.\n\r", ch );
        send_to_char( ch->pcdata->block_join,ch);
        return;
    }

    return;
}

void do_apply(CHAR_DATA *ch, char *argument)
{
	ORG_DATA *org;
	ORGMEM_DATA *mem;
	char buf[MSL]={'\0'};
	CHAR_DATA *vch;
	DESCRIPTOR_DATA *d;

	CheckCH(ch);

	if(IS_NULLSTR(argument))
	{
		send_to_char("Syntax: apply <organisation>\n\r", ch);
		for(org = org_list; org; org = org->next)
		{
			send_to_char(Format("[%s] %s %s\n\r", org->who_name, org->name, org->type>0?"":"(Self inducting/Open)"), ch);
		}
		return;
	}

	if((org = org_lookup(argument)) == NULL)
	{
		send_to_char("No such organisation.\n\r", ch);
		return;
	}

	if(!strstr(org->races, capitalize(race_table[ch->race].name))
		&& !strstr(org->races, capitalize(clan_table[ch->clan].name))
		&& !strstr(org->races, "All"))
	{
		send_to_char("You aren't eligible for membership.\n\r", ch);
		return;
	}

	if((mem = mem_lookup(org, ch->name)) != NULL)
	{
		send_to_char("You're already a member!\n\r", ch);
		return;
	}

	if(org->type == 0)
	{
		send_to_char("You don't have to apply to that organisation.\n\r", ch);
		send_to_char("Just INDUCT yourself.\n\r", ch);
		return;
	}

	if(is_online(org->leader) && strstr( org->applicants, ch->name ) != NULL)
	{
		if((vch = get_char_world(ch, org->leader)) == NULL)
		{
			for(d = descriptor_list; d; d = d->next)
			{
				if(!str_cmp(org->leader, d->character->name))
				{
					vch = d->character;
				}
			}
		}

		act("$n has applied to join $t.", ch, org->name, vch, TO_VICT,1);
	}

	if ( strstr( org->applicants, ch->name ) != NULL )
	{
		org->applicants = string_replace( org->applicants, ch->name, "" );
		org->applicants = string_unpad( org->applicants );

		if ( org->applicants[0] == '\0' )
		{
			PURGE_DATA( org->applicants );
			org->applicants = str_dup( "None" );
		}
		send_to_char( "Application removed.\n\r", ch );
	}
	else
	{
		buf[0] = '\0';
		if ( strstr(org->applicants, "None") != NULL )
		{
			org->applicants = string_replace( org->applicants, "None", "\0" );
			org->applicants = string_unpad( org->applicants );
		}

		if (!IS_NULLSTR(org->applicants) )
		{
			strncat( buf, org->applicants, sizeof(buf) - strlen(buf) - 1 );
			strncat( buf, " ", sizeof(buf) - strlen(buf) - 1 );
		}
		strncat( buf, ch->name, sizeof(buf) - strlen(buf) - 1 );
		PURGE_DATA( org->applicants );
		org->applicants = string_proper( str_dup( buf ) );
		send_to_char( "Application submitted.\n\r", ch );
	}

	fwrite_org(org);
}

void do_refuse(CHAR_DATA *ch, char *argument)
{
	ORG_DATA *org;
	ORGMEM_DATA *mem;
	char arg[MIL]={'\0'};
	CHAR_DATA *vch;
	DESCRIPTOR_DATA *d;

	CheckCH(ch);

	argument = one_argument(argument, arg);

	if(IS_NULLSTR(arg) || IS_NULLSTR(argument))
	{
		send_to_char("Syntax: refuse [organisation] [applicant]\n\r", ch);
		for(org = org_list; org; org = org->next)
		{
			send_to_char(Format("[%s] %s %s\n\r", org->who_name, org->name,
					org->type>0?"":"(Self inducting/Open)"), ch);
		}
		return;
	}

	if((org = org_lookup(arg)) == NULL)
	{
		send_to_char("No such organisation.\n\r", ch);
		return;
	}

	if((mem = mem_lookup(org, ch->name)) == NULL)
	{
		send_to_char("You aren't even a member of that organisation!\n\r", ch);
		return;
	}

	if(!IS_SET(mem->auth_flags, AUTH_INDUCT))
	{
		send_to_char("You don't have the authority to do that.\n\r", ch);
		return;
	}

	if ( strstr( org->applicants, argument ) != NULL )
	{
		org->applicants = string_replace( org->applicants, argument, "" );
		org->applicants = string_unpad( org->applicants );

		if ( org->applicants[0] == '\0' )
		{
			PURGE_DATA( org->applicants );
			org->applicants = str_dup( "None" );
		}
		send_to_char( "Application refused.\n\r", ch );
		fwrite_org(org);

		if(is_online(argument))
		{
			if((vch = get_char_world(ch, argument)) == NULL)
			{
				for(d = descriptor_list; d; d = d->next)
				{
					if(!str_prefix(arg, d->character->name))
					{
						vch = d->character;
					}
				}
			}

			act("$n has refused your application to join $t.", ch, org->name, vch, TO_VICT,1);
		}

	}
	else
	{
		send_to_char("No such applicant.\n\r", ch);
	}
}

void do_applicants(CHAR_DATA *ch, char *argument)
{
	ORG_DATA *org;
	ORGMEM_DATA *mem;

	CheckCH(ch);

	if(IS_NULLSTR(argument))
	{
		send_to_char("Syntax: applicants [organisation]\n\r", ch);
		for(org = org_list; org; org = org->next)
		{
			send_to_char(Format("[%s] %s\n\r", org->who_name, org->name), ch);
		}
		return;
	}

	if((org = org_lookup(argument)) == NULL)
	{
		send_to_char("No such organisation.\n\r", ch);
		return;
	}

	if((mem = mem_lookup(org, ch->name)) == NULL && !IS_ADMIN(ch))
	{
		send_to_char("You don't even belong to that organisation!\n\r", ch);
		return;
	}

	if(str_cmp(org->leader, ch->name) && !IS_ADMIN(ch) && !IS_SET(mem->auth_flags, AUTH_INDUCT))
	{
		send_to_char("You aren't in a position to see members of that organisation.\n\r", ch);
		return;
	}

	if(!str_cmp(org->applicants, "None"))
	{
		send_to_char("There are currently no outstanding applications.\n\r", ch);
	}
	else
	{
		send_to_char("Currently applying to join the organisation are:\n\r", ch);
		send_to_char(org->applicants, ch);
	}
}

void do_induct(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *vch;
	ORG_DATA *org;
	ORGMEM_DATA *mem;
	bool online;
	char arg[MIL]={'\0'};

	CheckCH(ch);

	if(IS_NULLSTR(argument))
	{
		send_to_char("Syntax: induct [player] [organization]\n\r", ch);
		return;
	}

    argument = one_argument(argument, arg);
    online = is_online(arg);

    if((vch = get_player(arg)) == NULL)
    {
    	send_to_char("No such player.\n\r", ch);
    	return;
    }

    if((org = org_lookup(argument)) == NULL)
    {
    	send_to_char("No such organisation.\n\r", ch);
    	if(!online && vch)
    		free_char(vch);
    	return;
    }

    if(!strstr(org->races, capitalize(race_table[ch->race].name))
	&& !strstr(org->races, capitalize(clan_table[ch->clan].name))
	&& !strstr(org->races, "All"))
    {
    	send_to_char("They aren't eligible for membership.\n\r", ch);
    	if(!online && vch)
    		free_char(vch);
    	return;
    }

    if(org->type != 0 && org->type != 3)
    {
    	if((mem = mem_lookup(org, ch->name)) == NULL && !IS_ADMIN(ch))
    	{
    		send_to_char("You aren't even a member!\n\r", ch);
    		if(!online && vch)
    			free_char(vch);
    		return;
    	}

    	if((mem == NULL || !IS_SET(mem->auth_flags, AUTH_INDUCT))
    			&& !IS_ADMIN(ch))
    	{
    		if(org->type > 0)
    		{
    			send_to_char("You don't have the authority to induct members.\n\r", ch);
    			if(!online && vch)
    				free_char(vch);
    			return;
    		}
    	}

    	if((mem = mem_lookup(org, vch->name)) != NULL)
    	{
    		send_to_char("They're already a member!\n\r", ch);
    		do_refuse(ch, (char *)Format("%s %s", argument, vch->name));
    		if(online) send_to_char("You are already a member.\n\r", vch);
    		if(!online && vch)
    			free_char(vch);
    		return;
    	}

    	if(!is_name(vch->name, org->applicants))
    	{
    		send_to_char("They haven't applied to join that organisation.\n\r", ch);
    		if(!online && vch)
    			free_char(vch);
    		return;
    	}

    	org->applicants = string_replace( org->applicants, vch->name, "" );
    	org->applicants = string_unpad( org->applicants );
    }
    else
    {
    	if((mem = mem_lookup(org, ch->name)) != NULL)
    	{
    		send_to_char("You're already a member!\n\r", ch);
    		return;
    	}
    }

    mem = new_orgmem();
    PURGE_DATA(mem->name);
    mem->name = str_dup(vch->name);
    mem->auth_flags = org->default_auths;
    mem->status = 1;
    mem->next = org->members;
    org->members = mem;

    fwrite_org(org);

    if(ch != vch)
    	act("You induct $N into $t.", ch, org->name, vch, TO_CHAR,1);
    else
    	act("You induct yourself into $t.", ch, org->name, vch, TO_CHAR,1);

    if(!online)
    	free_char(vch);
    else
    	act("You have been inducted into $t by $n.", ch, org->name, vch, TO_VICT, 1);
}

void do_authflag(CHAR_DATA *ch, char *argument)
{
	char arg[MIL]={'\0'};
	ORGMEM_DATA *mem, *chmem;
	ORG_DATA *org;
	int value = 0;

	CheckCH(ch);

	if(IS_NULLSTR(argument))
	{
		send_to_char("Syntax: authorise [org] [member/default] [flags]", ch);
		return;
	}

	argument = one_argument(argument, arg);
	if((org = org_lookup(arg)) == NULL)
	{
		send_to_char("No such organisation.\n\r", ch);
		return;
	}

	if((chmem = mem_lookup(org, ch->name)) == NULL && !IS_ADMIN(ch))
	{
		send_to_char("You aren't even a member!\n\r", ch);
		return;
	}

	if(!IS_SET(chmem->auth_flags, AUTH_MEMBERS) && !IS_ADMIN(ch))
	{
		if(!IS_SET(chmem->auth_flags, AUTH_MEMBERS))
		{
			send_to_char("You don't have the authority.\n\r", ch);
			return;
		}
	}

	argument = one_argument(argument, arg);
	if((mem = mem_lookup(org, arg)) == NULL && !str_cmp(arg, "default"))
	{
		send_to_char("There is no member of that name.\n\r", ch);
		return;
	}

	if(IS_NULLSTR(argument))
	{
		act("$T's authorisations are:\n\r$t.", ch, flag_string(org_auths, org->default_auths), org->name, TO_CHAR, 1);
		return;
	}

	if((value = flag_value(org_auths, argument)) <= 0)
	{
		send_to_char("No such authority flag(s).\n\r", ch);
		return;
	}

	if(!str_prefix(arg, "default") && mem == NULL)
	{
		org->default_auths ^= value;
		send_to_char("Default authorisations for the organisation set.\n\r", ch);
		act("$T's authorisations are now:\n\r$t.", ch, flag_string(org_auths, org->default_auths), org->name, TO_CHAR, 1);
		fwrite_org(org);
		return;
	}

	if(mem && value > 0)
	{
		mem->auth_flags ^= value;
		send_to_char("Authorisations for member set.\n\r", ch);
		act("$T's authorisations are now:\n\r$t.", ch, flag_string(org_auths, mem->auth_flags), mem->name, TO_CHAR, 1);
	}

	fwrite_org(org);
}

void do_status(CHAR_DATA *ch, char *argument)
{
	char arg[MIL]={'\0'};
	ORGMEM_DATA *mem, *chmem;
	ORG_DATA *org;
	int value = 0;
	bool down = FALSE;

	CheckCH(ch);

	if(IS_NULLSTR(argument))
	{
		send_to_char("Syntax: status\n\r", ch);
		send_to_char("        status <org> <member> [<up/down>]\n\r", ch);
		for(value = MAX_BACKGROUND; background_table[value].name; value++)
		{
			send_to_char(Format("%s%s:%4d ", value==0?pc_race_table[ch->race].clan_word:background_table[value].name,
					value==3?"":" Status", ch->backgrounds[value]), ch);
			if(value%4 == 0) send_to_char("\n\r",ch);
		}
		for(org = org_list; org; org = org->next)
		{
			if((mem = mem_lookup(org, ch->name)) != NULL) {
				send_to_char(Format("%s Status:%4d ", org->name, mem->status), ch);
			}
			if(value%4 == 0) send_to_char("\n\r",ch);
		}
		return;
	}

	argument = one_argument(argument, arg);
	if((org = org_lookup(arg)) == NULL)
	{
		send_to_char("No such organisation.\n\r", ch);
		return;
	}

	if((chmem = mem_lookup(org, ch->name)) == NULL)
	{
		send_to_char("You aren't even a member!\n\r", ch);
		return;
	}

	if(IS_NULLSTR(argument))
	{
		send_to_char(Format("%s Status:%4d ", org->name, chmem->status), ch);
		return;
	}

	if(!IS_SET(chmem->auth_flags, AUTH_STATUS))
	{
		send_to_char("You aren't in a position to affect their status.\n\r",ch);
		return;
	}

	argument = one_argument(argument, arg);
	if((mem = mem_lookup(org, arg)) == NULL)
	{
		send_to_char("There is no member of that name.\n\r", ch);
		return;
	}

	if(IS_NULLSTR(argument) || !str_prefix(argument, "up"))
	{
		down = FALSE;
	}
	else if(!str_prefix(argument, "down"))
	{
		down = TRUE;
	}
	else
	{
		send_to_char("What are you trying to do to their status?\n\r", ch);
		return;
	}

	value = chmem->status - mem->status;

	if(value <= 1) value = 1;
	else if(value <= 3) value = 3;
	else if(value <= 5) value = 4;

	if(down == TRUE) value *= -1;

	act("You place your input on the status of $t.", ch, mem->name, NULL, TO_CHAR, 1);

	fwrite_org(org);
}

void do_boot(CHAR_DATA *ch, char *argument)
{
	char arg[MIL]={'\0'};
	ORGMEM_DATA *mem, *chmem;
	ORG_DATA *org;

	CheckCH(ch);

	if(IS_NULLSTR(argument))
	{
		send_to_char("Syntax: boot <org> <member>\n\r", ch);
		return;
	}

    argument = one_argument(argument, arg);
    if((org = org_lookup(arg)) == NULL)
    {
	send_to_char("No such organisation.\n\r", ch);
	return;
    }

    if((chmem = mem_lookup(org, ch->name)) == NULL)
    {
	send_to_char("You aren't even a member!\n\r", ch);
	return;
    }

    if(!IS_SET(chmem->auth_flags, AUTH_BOOT)
	&& org->type != 0 && org->type != 3)
    {
	send_to_char("You aren't in a position to affect memberships.\n\r",ch);
	return;
    }

    argument = one_argument(argument, arg);
    if((mem = mem_lookup(org, arg)) == NULL)
    {
	send_to_char("There is no member of that name.\n\r", ch);
	return;
    }

    if(chmem != mem && (org->type == 0 || org->type == 3)
	&& !IS_SET(chmem->auth_flags, AUTH_BOOT))
    {
	send_to_char( "You're talking about a self-inducting organisation...\n\r", ch);
	send_to_char("And you can't boot someone else!\n\r", ch);
	return;
    }

    if(!str_cmp(mem->name, org->leader))
    {
	send_to_char("You can't boot the leader.\n\r", ch);
	return;
    }

    extract_mem(org, mem);
    act("$t has been booted from $T.", ch, mem->name, org->name, TO_CHAR, 1);
    fwrite_org(org);
}

void do_resign(CHAR_DATA *ch, char *argument)
{
	char arg[MIL]={'\0'};
	ORGMEM_DATA *mem;
	ORG_DATA *org;

	CheckCH(ch);

	if(IS_NULLSTR(argument))
	{
		send_to_char("Syntax: resign [organization]\n\r", ch);
		return;
	}

	argument = one_argument(argument, arg);
	if((org = org_lookup(arg)) == NULL)
	{
		send_to_char("No such organisation.\n\r", ch);
		return;
	}

	if((mem = mem_lookup(org, ch->name)) == NULL)
	{
		send_to_char("You aren't even a member!\n\r", ch);
		return;
	}

	if(!str_cmp(mem->name, org->leader))
	{
		send_to_char("You resign as the leader.\n\r", ch);
		send_to_char("To resign from the organisation, resign again.\n\r", ch);
		PURGE_DATA(org->leader);
		org->leader = NULL;
		return;
	}

	extract_mem(org, mem);
	act("You resign from $T.", ch, NULL, org->name, TO_CHAR, 1);
	fwrite_org(org);
}

void do_members(CHAR_DATA *ch, char *argument)
{
	ORG_DATA *org;
	ORGMEM_DATA *mem;
	int count = 0;

	CheckCH(ch);

	if(IS_NULLSTR(argument))
	{
		send_to_char("Syntax: members [organisation]\n\r", ch);
		for(org = org_list; org; org = org->next)
		{
			send_to_char(Format("[%s] %s\n\r", org->who_name, org->name), ch);
		}
		return;
	}

	if((org = org_lookup(argument)) == NULL)
	{
		send_to_char("No such organisation.\n\r", ch);
		return;
	}

	if((mem = mem_lookup(org, ch->name)) == NULL && !IS_ADMIN(ch))
	{
		send_to_char("You don't even belong to that organisation!\n\r", ch);
		return;
	}

	if(!IS_ADMIN(ch) && !IS_SET(mem->auth_flags, AUTH_MEMBERS))
	{
		send_to_char( "You aren't in a position to see members of that organisation.\n\r", ch);
		return;
	}

	if(org->members == NULL)
	{
		send_to_char("The organisation has no members.\n\r", ch);
	} 
	else 
	{
		count = 0;
		send_to_char(Format("%-14s %-10s", "Name", "Status"), ch);
		send_to_char(Format("%-14s %-10s\n\r", "Name", "Status"), ch);
		send_to_char("------------------------------------------------\n\r",ch);
		for(mem = org->members; mem; mem = mem->next)
		{
			count++;
			send_to_char(Format("%-14s %-10d", mem->name, mem->status), ch);
			if(count % 2 == 0)
				send_to_char("\n\r", ch);
		}
		send_to_char("\n\r", ch);
	}
}

void do_donate(CHAR_DATA *ch, char *argument)
{
	ORG_DATA *org;
	ORGMEM_DATA *mem;
	char arg[MIL]={'\0'};
	int amount = 0;

	CheckCH(ch);

	argument = one_argument(argument, arg);

	if(IS_NULLSTR(argument) || IS_NULLSTR(arg))
	{
		send_to_char("Syntax: donate [organisation] [amount]\n\r", ch);
		for(org = org_list; org; org = org->next)
		{
			send_to_char(Format("[%s] %s\n\r", org->who_name, org->name), ch);
		}
		return;
	}

	if((org = org_lookup(argument)) == NULL)
	{
		send_to_char("No such organisation.\n\r", ch);
		return;
	}

	if((mem = mem_lookup(org, ch->name)) == NULL && !IS_ADMIN(ch))
	{
		send_to_char("You don't even belong to that organisation!\n\r", ch);
		return;
	}

	if(!is_number(argument))
	{
		send_to_char("How much do you want to donate?\n\r", ch);
		return;
	}

	if((amount = atoi(argument)) > ch->dollars)
	{
		send_to_char("You don't have that many dollars to donate.\n\r", ch);
		return;
	}

	ch->dollars -= amount;
	org->funds += amount;
	send_to_char(Format("You make a donation of $%d to %s.\n\r", amount, org->name), ch);
}

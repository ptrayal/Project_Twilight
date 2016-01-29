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

#if defined(Macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <malloc.h>
#include <stdlib.h>
#include <time.h>
#include "twilight.h"
#include "tables.h"
#include "lookup.h"
#include "recycle.h"
#include "interp.h"

/* Needed procedures */
ROOM_INDEX_DATA *find_location( CHAR_DATA *ch, char *arg );
int flag_value                  args ( ( const struct flag_type *flag_table,
                                         char *argument) );
int   get_cost        args( (CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy ) );
EXTRA_DESCR_DATA *new_extra_descr args( (void) );
char *flag_string( const struct flag_type *flag_table, int bits );

void be_frenzied(CHAR_DATA *ch);

char *  const   dir_name    []      =
{
    "north", "east", "south", "west", "up", "down"
};

const   sh_int  rev_dir     []      =
{
    2, 3, 0, 1, 5, 4
};

const   sh_int  movement_loss   [SECT_MAX]  =
{
    2, 6, 4, 6, 8, 12, 8, 2, 12, 16, 12, 4
};



/*
 * Local functions.
 */
int find_door   args( ( CHAR_DATA *ch, char *arg ) );
bool    has_key     args( ( CHAR_DATA *ch, int key ) );



void move_char( CHAR_DATA *ch, int door, bool follow )
{
    CHAR_DATA *fch;
    CHAR_DATA *fch_next;
    ROOM_INDEX_DATA *in_room;
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;

    if(ch->jump_timer > 0 && !IS_AFFECTED(ch,AFF_FLYING))
    {
        send_to_char("You can't until you land!\n\r", ch);
        return;
    }

    if ( door < 0 || door > 5 )
    {
        log_string(LOG_BUG, Format("Do_move: bad door %d.", door));
        return;
    }

    if(IS_SET(ch->act2, ACT2_ASTRAL))
    {
        in_room = ch->listening;
    }
    else
    {
        in_room = ch->in_room;
        /*
         * Exit trigger. If activated, stop movement.
         */
        if( !IS_NPC(ch) && mp_exit_trigger( ch, door ) )
            return;
    }
    if ( ( pexit   = in_room->exit[door] ) == NULL
            ||   ( to_room = pexit->u1.to_room   ) == NULL
            ||   !can_see_room(ch,pexit->u1.to_room))
    {
        send_to_char( "Alas, you cannot go that way.\n\r", ch );
        return;
    }

    if(IS_SET(pexit->exit_info,EX_CLOSED) && !IS_SET(pexit->exit_info,EX_BROKEN)
    && !IS_SET(ch->form, FORM_MIST)
    && !IS_SET(ch->form, FORM_INTANGIBLE)
    && !IS_SET(ch->form, FORM_BLOOD)
    && !IS_SET(ch->form, FORM_SHADOW)
    && (!IS_SET(ch->act2, ACT2_ASTRAL) || (IS_SET(ch->act2, ACT2_ASTRAL)
        && (IS_SET(pexit->exit_info, EX_UMBRA)
           || IS_SET(pexit->exit_info, EX_DREAM)))))
    {
    act( "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR, 0 );
    return;
    }

    if (IS_SET(pexit->exit_info, EX_NARROW) && ch->size > SIZE_MEDIUM
    && !IS_SET(ch->form, FORM_MIST)
    && !IS_SET(ch->form, FORM_INTANGIBLE)
    && !IS_SET(ch->form, FORM_BLOOD)
    && !IS_SET(ch->form, FORM_SHADOW)
    && !is_affected(ch, skill_lookup("skin of the adder")))
    {
    act( "The $d is too small for you to fit through.",
        ch, NULL, pexit->keyword, TO_CHAR, 0 );
    return;
    }

    if (IS_SET(pexit->exit_info, EX_SQUEEZE) && ch->size > SIZE_SMALL
    && !IS_SET(ch->form, FORM_MIST)
    && !IS_SET(ch->form, FORM_INTANGIBLE)
    && !IS_SET(ch->form, FORM_BLOOD)
    && !IS_SET(ch->form, FORM_SHADOW)
    && !is_affected(ch, skill_lookup("skin of the adder")))
    {
    act( "The $d is too small for you to fit through.",
        ch, NULL, pexit->keyword, TO_CHAR, 0 );
    return;
    }

    if (IS_SET(pexit->exit_info, EX_CLOSED)
    && IS_SET(pexit->exit_info, EX_SEALED)
    && !IS_SET(ch->form, FORM_INTANGIBLE))
    {
    act( "The $d is sealed and you can't get through.", ch, NULL, pexit->keyword, TO_CHAR, 0 );
    return;
    }

    if (IS_SET(pexit->exit_info, EX_FALLING))
    {
    act( "The $d leads to a drop, you can go no further.", ch, NULL, pexit->keyword, TO_CHAR, 0 );
    return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM)
    &&   ch->master != NULL
    &&   in_room == ch->master->in_room )
    {
    send_to_char( "What?  And leave your beloved master?\n\r", ch );
    return;
    }

    if ( !is_room_owner(ch,to_room) && room_is_private( to_room ) )
    {
        send_to_char( "That room is private right now.\n\r", ch );
        return;
    }

    if ( !IS_NPC(ch) )
    {
    int iClan, iGuild;
    int move;

    for ( iClan = 0; iClan < MAX_CLAN; iClan++ )
    {
        for ( iGuild = 0; iGuild < MAX_CLAN; iGuild ++)
        {
       /*   if ( iClan != ch->clan
            &&   to_room->vnum == clan_table[iClan].guild[iGuild] )
            {
            send_to_char( "You aren't allowed in there.\n\r", ch );
            return;
        } */
        }
    }

    if ( in_room->sector_type == SECT_AIR )
    {
        if ( !IS_AFFECTED(ch, AFF_FLYING) && !IS_ADMIN(ch)
          && !IS_SET(ch->act2,ACT2_ASTRAL))
        {
        send_to_char( "You can't fly.\n\r", ch );
        return;
        }
    }

    if ( to_room->sector_type == SECT_AIR )
    {
        if ( !IS_AFFECTED(ch, AFF_FLYING) && !IS_ADMIN(ch)
          && !IS_SET(ch->act2,ACT2_ASTRAL))
        {
        send_to_char("You can't fly... You could try jumping.\n\r",ch);
        return;
        }
    }

    if (( in_room->sector_type == SECT_WATER_NOSWIM
    ||    to_room->sector_type == SECT_WATER_NOSWIM )
    &&    !IS_AFFECTED(ch,AFF_FLYING)
    &&    !IS_SET(ch->act2,ACT2_ASTRAL))
    {
        OBJ_DATA *obj;
        bool found;

        /*
         * Look for a boat.
         */
        found = FALSE;

        if (IS_ADMIN(ch))
        found = TRUE;

        for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
        {
        if ( obj->item_type == ITEM_BOAT )
        {
            found = TRUE;
            break;
        }
        }
        if ( !found )
        {
        send_to_char( "You need a boat to go there.\n\r", ch );
        return;
        }
    }

    move = movement_loss[UMIN(SECT_MAX-1, in_room->sector_type)]
                         + movement_loss[UMIN(SECT_MAX-1, to_room->sector_type)]
                                         ;

    move /= 2;  /* i.e. the average */

    if ((IS_SET(pexit->exit_info, EX_NARROW) && ch->size == SIZE_MEDIUM)
            || (IS_SET(pexit->exit_info, EX_SQUEEZE) && ch->size == SIZE_SMALL))
        move += 3;


    /* conditional effects */
    if (IS_AFFECTED(ch,AFF_FLYING))
        move /= 2;

    if (IS_AFFECTED(ch,AFF_SLOW))
        move *= 2;

    move -= dice_rolls(ch, (get_curr_stat(ch,STAT_DEX) + ch->ability[ATHLETICS].value), move);

    if (IS_SET(ch->act,ACT_FAST) && !IS_SET(ch->act2,ACT2_ASTRAL));
    else
    {
        if(move >= 0)
            ch->wait += move;
    }

    }

    if (IS_AFFECTED(ch, AFF_SNEAK)
            && (dice_rolls(ch, (get_curr_stat(ch,STAT_DEX) + ch->ability[STEALTH].value),6) <= 0))
        REMOVE_BIT(ch->affected_by,AFF_SNEAK);

    if ( !IS_AFFECTED(ch, AFF_SNEAK) && !IS_SET(ch->act2, ACT2_ASTRAL)
            &&   ch->invis_level < LEVEL_IMMORTAL)
    {
        if(!IS_AFFECTED(ch, AFF_INVISIBLE) || dice_rolls(ch,
                get_curr_stat(ch, STAT_DEX) + ch->ability[STEALTH].value,
                7) < 1) {
            if(!LOOKS_DIFFERENT(ch))
                act( "$n leaves $T.", ch, NULL, dir_name[door], TO_ROOM, 0 );
            else
            {
                act( Format("%s leaves %s.", ch->alt_name, dir_name[door]), ch, NULL, NULL, TO_ROOM, 0 );
            }
        }
    }

    if(IS_SET(ch->comm, COMM_JAM_LEAD))
    {
        REMOVE_BIT(ch->comm, COMM_JAM_LEAD);
        REMOVE_BIT(in_room->room_flags, ROOM_JAMMIN);
        act("$n's absence ends the jam.", ch, NULL, NULL, TO_ROOM, 0);
        send_to_char("Without you leading the jam, it ends.\n\r", ch);
    }
    else if(IS_SET(ch->comm, COMM_JAMMIN)
            && IS_SET(in_room->room_flags, ROOM_JAMMIN))
    {
        int move = 0;
        REMOVE_BIT(ch->comm, COMM_JAMMIN);
        for(fch = in_room->people; fch != NULL; fch = fch->next_in_room)
        {
            if(!IS_SET(fch->comm, COMM_JAMMIN)
                    && !IS_SET(fch->comm, COMM_JAM_LEAD))
                continue;
            else move = 1;
        }

        if(!move) REMOVE_BIT(in_room->room_flags, ROOM_JAMMIN);
    }

    if(IS_SET(ch->act2, ACT2_ASTRAL))
    {
        char_from_listeners( ch );
        char_to_listeners( ch, to_room );
    }
    else
    {
        char_from_room( ch );
        char_to_room( ch, to_room );
    }
    if ( !IS_AFFECTED(ch, AFF_SNEAK) && !IS_SET(ch->act2, ACT2_ASTRAL)
            &&   ch->invis_level < LEVEL_IMMORTAL)
    {
        if(!IS_AFFECTED(ch, AFF_INVISIBLE) || dice_rolls(ch,
                get_curr_stat(ch, STAT_DEX) + ch->ability[STEALTH].value,
                7) < 1) {
            if(!LOOKS_DIFFERENT(ch))
                act( "$n has arrived.", ch, NULL, NULL, TO_ROOM, 0 );
            else
            {
                act( Format("%s has arrived.", ch->alt_name), ch, NULL, NULL, TO_ROOM, 0 );
            }
        }
    }

    do_function( ch, &do_look, "auto" );

    if (in_room == to_room) /* no circular follows */
        return;

    for ( fch = in_room->people; fch != NULL; fch = fch_next )
    {
        fch_next = fch->next_in_room;

        if( IS_SET(ch->act2, ACT2_ASTRAL)
                && !IS_SET(fch->act2, ACT2_ASTRAL) )
            continue;

        if ( fch->master == ch && IS_AFFECTED(fch,AFF_CHARM)
        &&   fch->position < P_STAND)
            do_function( fch, &do_stand, "" );

        if ( fch->master == ch && fch->position == P_STAND
                &&   can_see_room(fch,to_room))
        {
            act( "You follow $N.", fch, NULL, ch, TO_CHAR, 0 );
            move_char( fch, door, TRUE );
        }
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


void do_north( CHAR_DATA *ch, char *argument )
{
    CheckCH(ch);
    move_char( ch, DIR_NORTH, FALSE );
    return;
}



void do_east( CHAR_DATA *ch, char *argument )
{
    CheckCH(ch);
    move_char( ch, DIR_EAST, FALSE );
    return;
}



void do_south( CHAR_DATA *ch, char *argument )
{
    CheckCH(ch);
    move_char( ch, DIR_SOUTH, FALSE );
    return;
}



void do_west( CHAR_DATA *ch, char *argument )
{
    CheckCH(ch);
    move_char( ch, DIR_WEST, FALSE );
    return;
}



void do_up( CHAR_DATA *ch, char *argument )
{
    CheckCH(ch);
    move_char( ch, DIR_UP, FALSE );
    return;
}



void do_down( CHAR_DATA *ch, char *argument )
{
    CheckCH(ch);
    move_char( ch, DIR_DOWN, FALSE );
    return;
}



int find_door( CHAR_DATA *ch, char *arg )
{
    EXIT_DATA *pexit;
    int door;

    if ( !str_prefix( arg, "north" ) ) door = 0;
    else if ( !str_prefix( arg, "east"  ) ) door = 1;
    else if ( !str_prefix( arg, "south" ) ) door = 2;
    else if ( !str_prefix( arg, "west"  ) ) door = 3;
    else if ( !str_prefix( arg, "up"    ) ) door = 4;
    else if ( !str_prefix( arg, "down"  ) ) door = 5;
    else
    {
        for ( door = 0; door <= 5; door++ )
        {
            if ( ( pexit = ch->in_room->exit[door] ) != NULL
                    &&   (IS_SET(pexit->exit_info, EX_ISDOOR)
                            || IS_SET(pexit->exit_info, EX_WINDOW))
                            &&   pexit->keyword != NULL
                            &&   is_name( arg, pexit->keyword ) )
                return door;
        }
        act( "I see no $T here.", ch, NULL, arg, TO_CHAR, 0 );
        return -1;
    }

    if ( ( pexit = ch->in_room->exit[door] ) == NULL )
    {
        act( "I see no door $T here.", ch, NULL, arg, TO_CHAR, 0 );
        return -1;
    }

    if ( !IS_SET(pexit->exit_info, EX_ISDOOR)
            && !IS_SET(pexit->exit_info, EX_WINDOW) )
    {
        send_to_char( "You can't do that.\n\r", ch );
        return -1;
    }

    return door;
}


int find_dir( CHAR_DATA *ch, char *arg )
{
    EXIT_DATA *pexit;
    int door;

    if ( !str_prefix( arg, "north" ) ) door = 0;
    else if ( !str_prefix( arg, "east"  ) ) door = 1;
    else if ( !str_prefix( arg, "south" ) ) door = 2;
    else if ( !str_prefix( arg, "west"  ) ) door = 3;
    else if ( !str_prefix( arg, "up"    ) ) door = 4;
    else if ( !str_prefix( arg, "down"  ) ) door = 5;
    else
    {
        for ( door = 0; door <= 5; door++ )
        {
            if ( ( pexit = ch->in_room->exit[door] ) != NULL
                    &&   IS_SET(pexit->exit_info, EX_ISDOOR)
            &&   pexit->keyword != NULL
            &&   is_name( arg, pexit->keyword ) )
                return door;
        }
        act( "I see no $T here.", ch, NULL, arg, TO_CHAR, 0 );
        return -1;
    }

    return door;
}


void do_open( CHAR_DATA *ch, char *argument )
{
    char arg[MIL]={'\0'};
    OBJ_DATA *obj;
    int door = 0;

    CheckCH(ch);

    one_argument( argument, arg );

    if ( IS_NULLSTR(arg) )
    {
        send_to_char( "Open what?\n\r", ch );
        return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
        /* open portal */
        if (obj->item_type == ITEM_PORTAL)
        {
            if (!IS_SET(obj->value[1], EX_ISDOOR))
            {
                send_to_char("You can't do that.\n\r",ch);
                return;
            }

            if (!IS_SET(obj->value[1], EX_CLOSED))
            {
                send_to_char("It's already open.\n\r",ch);
                return;
            }

            if (IS_SET(obj->value[1], EX_LOCKED))
            {
                send_to_char("It's locked.\n\r",ch);
                return;
            }

            REMOVE_BIT(obj->value[1], EX_CLOSED);
            act("You open $p.",ch,obj,NULL,TO_CHAR, 0);
            act("$n opens $p.",ch,obj,NULL,TO_ROOM, 0);
            return;
        }

        /* 'open object' */
        if ( obj->item_type != ITEM_CONTAINER)
        { send_to_char( "That's not a container.\n\r", ch ); return; }
        if ( !IS_SET(obj->value[1], CONT_CLOSED) )
        { send_to_char( "It's already open.\n\r",      ch ); return; }
        if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
        { send_to_char( "You can't do that.\n\r",      ch ); return; }
        if ( IS_SET(obj->value[1], CONT_LOCKED) )
        { send_to_char( "It's locked.\n\r",            ch ); return; }

        REMOVE_BIT(obj->value[1], CONT_CLOSED);
        act("You open $p.",ch,obj,NULL,TO_CHAR,0);
        act( "$n opens $p.", ch, obj, NULL, TO_ROOM, 0 );
        return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
        /* 'open door' */
        ROOM_INDEX_DATA *to_room;
        EXIT_DATA *pexit;
        EXIT_DATA *pexit_rev;

        pexit = ch->in_room->exit[door];
        if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
        { send_to_char( "It's already open.\n\r",      ch ); return; }
        if (  IS_SET(pexit->exit_info, EX_LOCKED) )
        { send_to_char( "It's locked.\n\r",            ch ); return; }
        if (  IS_SET(pexit->exit_info, EX_BROKEN) )
        { send_to_char( "The broken seal swings loosely.\n\r",ch);return; }

        REMOVE_BIT(pexit->exit_info, EX_CLOSED);
        act( "$n opens the $d.", ch, NULL, pexit->keyword, TO_ROOM, 0 );
        send_to_char( "Ok.\n\r", ch );

        /* open the other side */
        if ( ( to_room   = pexit->u1.to_room            ) != NULL
                &&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
                &&   pexit_rev->u1.to_room == ch->in_room )
        {
            CHAR_DATA *rch;

            REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
            for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room )
                act( "The $d opens.", rch,NULL,pexit_rev->keyword,TO_CHAR,0);
        }
    }

    return;
}



void do_close( CHAR_DATA *ch, char *argument )
{
    char arg[MIL]={'\0'};
    OBJ_DATA *obj;
    int door = 0;

    CheckCH(ch);

    one_argument( argument, arg );

    if ( IS_NULLSTR(arg) )
    {
        send_to_char( "Close what?\n\r", ch );
        return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
    /* portal stuff */
    if (obj->item_type == ITEM_PORTAL)
    {

        if (!IS_SET(obj->value[1],EX_ISDOOR)
        ||   IS_SET(obj->value[1],EX_NOCLOSE))
        {
        send_to_char("You can't do that.\n\r",ch);
        return;
        }

        if (IS_SET(obj->value[1],EX_CLOSED))
        {
        send_to_char("It's already closed.\n\r",ch);
        return;
        }

        SET_BIT(obj->value[1],EX_CLOSED);
        act("You close $p.",ch,obj,NULL,TO_CHAR,0);
        act("$n closes $p.",ch,obj,NULL,TO_ROOM,0);
        return;
    }

    /* 'close object' */
    if ( obj->item_type != ITEM_CONTAINER )
        { send_to_char( "That's not a container.\n\r", ch ); return; }
    if ( IS_SET(obj->value[1], CONT_CLOSED) )
        { send_to_char( "It's already closed.\n\r",    ch ); return; }
    if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
        { send_to_char( "You can't do that.\n\r",      ch ); return; }

    SET_BIT(obj->value[1], CONT_CLOSED);
    act("You close $p.",ch,obj,NULL,TO_CHAR,0);
    act( "$n closes $p.", ch, obj, NULL, TO_ROOM, 0 );
    return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
    /* 'close door' */
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev;

    pexit   = ch->in_room->exit[door];
    if ( IS_SET(pexit->exit_info, EX_CLOSED) )
        { send_to_char( "It's already closed.\n\r",    ch ); return; }

    if ( IS_SET(pexit->exit_info, EX_NOCLOSE) )
        { send_to_char( "You can't close that.\n\r",    ch ); return; }

    if ( IS_SET(pexit->exit_info, EX_BROKEN) )
        { send_to_char( "You can't close it. It's busted.\n\r",ch);return; }

    SET_BIT(pexit->exit_info, EX_CLOSED);
    act( "$n closes the $d.", ch, NULL, pexit->keyword, TO_ROOM, 0 );
    send_to_char( "Ok.\n\r", ch );

    /* close the other side */
    if ( ( to_room   = pexit->u1.to_room            ) != NULL
    &&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != 0
    &&   pexit_rev->u1.to_room == ch->in_room )
    {
        CHAR_DATA *rch;

        SET_BIT( pexit_rev->exit_info, EX_CLOSED );
        for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room )
        act( "The $d closes.", rch,NULL,pexit_rev->keyword,TO_CHAR,0);
    }
    }

    return;
}


bool has_key( CHAR_DATA *ch, int key )
{
    OBJ_DATA *obj;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
    if ( obj->pIndexData->vnum == key )
        return TRUE;
    }

    return FALSE;
}


void key_chomp( CHAR_DATA *ch, int key )
{
    OBJ_DATA *obj;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
    if ( obj->pIndexData->vnum == key )
        extract_obj(obj);
    }
}


void do_lock( CHAR_DATA *ch, char *argument )
{
    char arg[MIL]={'\0'};
    OBJ_DATA *obj;
    int door = 0;

    CheckCH(ch);

    one_argument( argument, arg );

    if ( IS_NULLSTR(arg) )
    {
    send_to_char( "Lock what?\n\r", ch );
    return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
    /* portal stuff */
    if (obj->item_type == ITEM_PORTAL)
    {
        if (!IS_SET(obj->value[1],EX_ISDOOR)
        ||  IS_SET(obj->value[1],EX_NOCLOSE))
        {
        send_to_char("You can't do that.\n\r",ch);
        return;
        }
        if (!IS_SET(obj->value[1],EX_CLOSED))
        {
        send_to_char("It's not closed.\n\r",ch);
        return;
        }

        if (obj->value[4] < 0 || IS_SET(obj->value[1],EX_NOLOCK))
        {
        send_to_char("It can't be locked.\n\r",ch);
        return;
        }

        if (!has_key(ch,obj->value[4]) && !IS_ADMIN(ch))
        {
        send_to_char("You lack the key.\n\r",ch);
        return;
        }

        if (IS_SET(obj->value[1],EX_LOCKED))
        {
        send_to_char("It's already locked.\n\r",ch);
        return;
        }

        SET_BIT(obj->value[1],EX_LOCKED);
        act("You lock $p.",ch,obj,NULL,TO_CHAR,0);
        act("$n locks $p.",ch,obj,NULL,TO_ROOM,0);
        return;
    }

    /* 'lock object' */
    if ( obj->item_type != ITEM_CONTAINER )
        { send_to_char( "That's not a container.\n\r", ch ); return; }
    if ( !IS_SET(obj->value[1], CONT_CLOSED) )
        { send_to_char( "It's not closed.\n\r",        ch ); return; }
    if ( obj->value[2] < 0 )
        { send_to_char( "It can't be locked.\n\r",     ch ); return; }
    if ( !has_key( ch, obj->value[2] ) && !IS_ADMIN(ch) )
        { send_to_char( "You lack the key.\n\r",       ch ); return; }
    if ( IS_SET(obj->value[1], CONT_LOCKED) )
        { send_to_char( "It's already locked.\n\r",    ch ); return; }

    SET_BIT(obj->value[1], CONT_LOCKED);
    act("You lock $p.",ch,obj,NULL,TO_CHAR,0);
    act( "$n locks $p.", ch, obj, NULL, TO_ROOM, 0 );
    return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
    /* 'lock door' */
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev;

    pexit   = ch->in_room->exit[door];
    if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
        { send_to_char( "It's not closed.\n\r",        ch ); return; }
    if ( pexit->key < 0 )
        { send_to_char( "It can't be locked.\n\r",     ch ); return; }
    if ( !has_key( ch, pexit->key) )
        { send_to_char( "You lack the key.\n\r",       ch ); return; }
    if ( IS_SET(pexit->exit_info, EX_LOCKED) )
        { send_to_char( "It's already locked.\n\r",    ch ); return; }

    SET_BIT(pexit->exit_info, EX_LOCKED);
    send_to_char( "*Click*\n\r", ch );
    act( "$n locks the $d.", ch, NULL, pexit->keyword, TO_ROOM, 0 );

    /* lock the other side */
    if ( ( to_room   = pexit->u1.to_room            ) != NULL
    &&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != 0
    &&   pexit_rev->u1.to_room == ch->in_room )
    {
        SET_BIT( pexit_rev->exit_info, EX_LOCKED );
    }
    }

    return;
}



void do_unlock( CHAR_DATA *ch, char *argument )
{
    char arg[MIL]={'\0'};
    OBJ_DATA *obj;
    int door = 0;

    CheckCH(ch);

    one_argument( argument, arg );

    if ( IS_NULLSTR(arg) )
    {
    send_to_char( "Unlock what?\n\r", ch );
    return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
    /* portal stuff */
    if (obj->item_type == ITEM_PORTAL)
    {
        if (!IS_SET(obj->value[1],EX_ISDOOR))
        {
        send_to_char("You can't do that.\n\r",ch);
        return;
        }

        if (!IS_SET(obj->value[1],EX_CLOSED))
        {
        send_to_char("It's not closed.\n\r",ch);
        return;
        }

        if (obj->value[4] < 0)
        {
        send_to_char("It can't be unlocked.\n\r",ch);
        return;
        }

        if (!has_key(ch,obj->value[4]) && !IS_ADMIN(ch))
        {
        send_to_char("You lack the key.\n\r",ch);
        return;
        }

        if (!IS_SET(obj->value[1],EX_LOCKED))
        {
        send_to_char("It's already unlocked.\n\r",ch);
        return;
        }

        if (IS_SET(obj->value[1],EX_KEYCHOMP)) key_chomp(ch,obj->value[4]);

        REMOVE_BIT(obj->value[1],EX_LOCKED);
        act("You unlock $p.",ch,obj,NULL,TO_CHAR, 0);
        act("$n unlocks $p.",ch,obj,NULL,TO_ROOM, 0);
        return;
    }

    /* 'unlock object' */
    if ( obj->item_type != ITEM_CONTAINER )
        { send_to_char( "That's not a container.\n\r", ch ); return; }
    if ( !IS_SET(obj->value[1], CONT_CLOSED) )
        { send_to_char( "It's not closed.\n\r",        ch ); return; }
    if ( obj->value[2] < 0 )
        { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
    if ( !has_key( ch, obj->value[2] ) )
        { send_to_char( "You lack the key.\n\r",       ch ); return; }
    if ( !IS_SET(obj->value[1], CONT_LOCKED) )
        { send_to_char( "It's already unlocked.\n\r",  ch ); return; }

    REMOVE_BIT(obj->value[1], CONT_LOCKED);
    act("You unlock $p.",ch,obj,NULL,TO_CHAR, 0);
    act( "$n unlocks $p.", ch, obj, NULL, TO_ROOM, 0 );
    return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
    /* 'unlock door' */
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev;

    pexit = ch->in_room->exit[door];
    if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
        { send_to_char( "It's not closed.\n\r",        ch ); return; }
    if ( pexit->key < 0 )
        { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
    if ( !has_key( ch, pexit->key) && !IS_ADMIN(ch) )
        { send_to_char( "You lack the key.\n\r",       ch ); return; }
    if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
        { send_to_char( "It's already unlocked.\n\r",  ch ); return; }

    if (IS_SET(pexit->exit_info,EX_KEYCHOMP)) key_chomp(ch,pexit->key);

    REMOVE_BIT(pexit->exit_info, EX_LOCKED);
    send_to_char( "*Click*\n\r", ch );
    act( "$n unlocks the $d.", ch, NULL, pexit->keyword, TO_ROOM, 0 );

    /* unlock the other side */
    if ( ( to_room   = pexit->u1.to_room            ) != NULL
    &&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
    &&   pexit_rev->u1.to_room == ch->in_room )
    {
        REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
    }
    }

    return;
}



void do_pick( CHAR_DATA *ch, char *argument )
{
    char arg[MIL]={'\0'};
    CHAR_DATA *gch;
    OBJ_DATA *obj;
    int door;
    int diff;

    CheckCH(ch);

    one_argument( argument, arg );

    if ( IS_NULLSTR(arg) )
    {
    send_to_char( "Pick what?\n\r", ch );
    return;
    }

    WAIT_STATE( ch, skill_table[gsn_pick_lock].beats );

    /* look for guards */
    for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
    {
    if ( IS_NPC(gch) && IS_AWAKE(gch) )
    {
        act( "$N is standing too close to the lock.",
        ch, NULL, gch, TO_CHAR, 0 );
        return;
    }
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
    if(IS_SET(obj->value[1],EX_EASY))
    {
        diff = 7;
    }
    else if(IS_SET(obj->value[1],EX_HARD))
    {
        diff = 9;
    }
    else if(IS_SET(obj->value[1],EX_INFURIATING))
    {
        diff = 10;
    }
    else diff = 8;

    if ( !IS_NPC(ch) && (dice_rolls(ch, get_curr_stat(ch, STAT_DEX) + ch->ability[LARCENY].value, diff) <= 3))
    {
        send_to_char( "You failed.\n\r", ch);
        return;
    }

    /* portal stuff */
    if (obj->item_type == ITEM_PORTAL)
    {
        if (!IS_SET(obj->value[1],EX_ISDOOR))
        {
        send_to_char("You can't do that.\n\r",ch);
        return;
        }

        if (!IS_SET(obj->value[1],EX_CLOSED))
        {
        send_to_char("It's not closed.\n\r",ch);
        return;
        }

        if (obj->value[4] < 0)
        {
        send_to_char("It can't be unlocked.\n\r",ch);
        return;
        }

        if (IS_SET(obj->value[1],EX_PICKPROOF))
        {
        send_to_char("You failed.\n\r",ch);
        return;
        }

        REMOVE_BIT(obj->value[1],EX_LOCKED);
        act("You pick the lock on $p.",ch,obj,NULL,TO_CHAR, 0);
        act("$n picks the lock on $p.",ch,obj,NULL,TO_ROOM, 0);
        return;
    }

    /* 'pick object' */
    if ( obj->item_type != ITEM_CONTAINER )
        { send_to_char( "That's not a container.\n\r", ch ); return; }
    if ( !IS_SET(obj->value[1], CONT_CLOSED) )
        { send_to_char( "It's not closed.\n\r",        ch ); return; }
    if ( obj->value[2] < 0 )
        { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
    if ( !IS_SET(obj->value[1], CONT_LOCKED) )
        { send_to_char( "It's already unlocked.\n\r",  ch ); return; }
    if ( IS_SET(obj->value[1], CONT_PICKPROOF) )
        { send_to_char( "You failed.\n\r",             ch ); return; }

    REMOVE_BIT(obj->value[1], CONT_LOCKED);
        act("You pick the lock on $p.",ch,obj,NULL,TO_CHAR, 0);
        act("$n picks the lock on $p.",ch,obj,NULL,TO_ROOM, 0);
    return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
    /* 'pick door' */
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev;

    pexit = ch->in_room->exit[door];

    if(IS_SET(pexit->exit_info,EX_EASY))
    {
        diff = 7;
    }
    else if(IS_SET(pexit->exit_info,EX_HARD))
    {
        diff = 9;
    }
    else if(IS_SET(pexit->exit_info,EX_INFURIATING))
    {
        diff = 10;
    }
    else diff = 8;

    if ( !IS_NPC(ch) && (dice_rolls(ch, get_curr_stat(ch, STAT_DEX) + ch->ability[LARCENY].value, diff) <= 3))
    {
        send_to_char( "You failed.\n\r", ch);
        return;
    }

    if ( !IS_SET(pexit->exit_info, EX_CLOSED) && !IS_ADMIN(ch))
        { send_to_char( "It's not closed.\n\r",        ch ); return; }
    if ( pexit->key < 0 && !IS_ADMIN(ch))
        { send_to_char( "It can't be picked.\n\r",     ch ); return; }
    if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
        { send_to_char( "It's already unlocked.\n\r",  ch ); return; }
    if ( IS_SET(pexit->exit_info, EX_PICKPROOF) && !IS_ADMIN(ch))
        { send_to_char( "You failed.\n\r",             ch ); return; }

    REMOVE_BIT(pexit->exit_info, EX_LOCKED);
    send_to_char( "*Click*\n\r", ch );
    act( "$n picks the $d.", ch, NULL, pexit->keyword, TO_ROOM, 0 );

    /* pick the other side */
    if ( ( to_room   = pexit->u1.to_room            ) != NULL
    &&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
    &&   pexit_rev->u1.to_room == ch->in_room )
    {
        REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
    }
    }

    return;
}


void do_stand( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj = NULL;
    char arg1[MIL]={'\0'};
    char arg2[MIL]={'\0'};

    CheckCH(ch);

    if (!IS_NULLSTR(argument))
    {
    if (ch->position == P_FIGHT)
    {
        send_to_char("Maybe you should finish fighting first?\n\r",ch);
        return;
    }
    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg1);

    if(!str_cmp(arg1, "on") || !str_cmp(arg1, "at") || !str_cmp(arg1, "in"))
        obj = get_obj_list(ch,arg2,ch->in_room->contents);
    else
        obj = get_obj_list(ch,arg1,ch->in_room->contents);
    if (obj == NULL)
    {
        send_to_char("You don't see that here.\n\r",ch);
        return;
    }
    if (obj->item_type != ITEM_FURNITURE
    ||  (!IS_SET(obj->value[2],STAND_AT)
    &&   !IS_SET(obj->value[2],STAND_ON)
    &&   !IS_SET(obj->value[2],STAND_IN)))
    {
        send_to_char("You can't seem to find a place to stand.\n\r",ch);
        return;
    }
    if (ch->on != obj && count_users(obj) >= obj->value[0])
    {
        act_new("There's no room to stand on $p.",
        ch,obj,NULL,TO_CHAR,P_DEAD,0);
        return;
    }
    ch->on = obj;
    }
    
    switch ( ch->position )
    {
    case P_SLEEP:
    if ( IS_AFFECTED(ch, AFF_SLEEP) )
        { send_to_char( "You can't wake up!\n\r", ch ); return; }

    if (obj == NULL)
    {
        send_to_char( "You wake and stand up.\n\r", ch );
        act( "$n wakes and stands up.", ch, NULL, NULL, TO_ROOM, 0 );
        ch->on = NULL;
    }
    else if (IS_SET(obj->value[2],STAND_AT))
    {
       act_new("You wake and stand at $p.",ch,obj,NULL,TO_CHAR,P_DEAD, 0);
       act("$n wakes and stands at $p.",ch,obj,NULL,TO_ROOM, 0);
    }
    else if (IS_SET(obj->value[2],STAND_ON))
    {
        act_new("You wake and stand on $p.",ch,obj,NULL,TO_CHAR,P_DEAD, 0);
        act("$n wakes and stands on $p.",ch,obj,NULL,TO_ROOM, 0);
    }
    else
    {
        act_new("You wake and stand in $p.",ch,obj,NULL,TO_CHAR,P_DEAD, 0);
        act("$n wakes and stands in $p.",ch,obj,NULL,TO_ROOM, 0);
    }
    ch->position = P_STAND;
    do_function( ch, &do_look, "auto" );
    break;

    case P_REST: case P_SIT:
    if (obj == NULL)
    {
        send_to_char( "You stand up.\n\r", ch );
        act( "$n stands up.", ch, NULL, NULL, TO_ROOM, 0 );
        ch->on = NULL;
    }
    else if (IS_SET(obj->value[2],STAND_AT))
    {
        act("You stand at $p.",ch,obj,NULL,TO_CHAR, 0);
        act("$n stands at $p.",ch,obj,NULL,TO_ROOM, 0);
    }
    else if (IS_SET(obj->value[2],STAND_ON))
    {
        act("You stand on $p.",ch,obj,NULL,TO_CHAR, 0);
        act("$n stands on $p.",ch,obj,NULL,TO_ROOM, 0);
    }
    else
    {
        act("You stand in $p.",ch,obj,NULL,TO_CHAR, 0);
        act("$n stands on $p.",ch,obj,NULL,TO_ROOM, 0);
    }
    ch->position = P_STAND;
    break;

    case P_STAND:
    send_to_char( "You are already standing.\n\r", ch );
    break;

    case P_FIGHT:
    send_to_char( "You are already fighting!\n\r", ch );
    break;
    }

    ch->balance = 1;

    return;
}



void do_rest( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj = NULL;

    CheckCH(ch);

    if (ch->position == P_FIGHT)
    {
    send_to_char("You are already fighting!\n\r",ch);
    return;
    }

    /* okay, now that we know we can rest, find an object to rest on */
    if (!IS_NULLSTR(argument))
    {
    obj = get_obj_list(ch,argument,ch->in_room->contents);
    if (obj == NULL)
    {
        send_to_char("You don't see that here.\n\r",ch);
        return;
    }
    }
    else obj = ch->on;

    if (obj != NULL)
    {
        if (obj->item_type != ITEM_FURNITURE
        ||  (!IS_SET(obj->value[2],REST_ON)
        &&   !IS_SET(obj->value[2],REST_IN)
        &&   !IS_SET(obj->value[2],REST_AT)))
        {
        send_to_char("You can't rest on that.\n\r",ch);
        return;
        }

        if (obj != NULL && ch->on != obj && count_users(obj) >= obj->value[0])
        {
        act_new("There's no more room on $p.",ch,obj,NULL,TO_CHAR,P_DEAD,0);
        return;
        }

    ch->on = obj;
    }

    switch ( ch->position )
    {
    case P_SLEEP:
    if (IS_AFFECTED(ch,AFF_SLEEP))
    {
        send_to_char("You can't wake up!\n\r",ch);
        return;
    }

    if (obj == NULL)
    {
        send_to_char( "You wake up and start resting.\n\r", ch );
        act ("$n wakes up and starts resting.",ch,NULL,NULL,TO_ROOM, 0);
    }
    else if (IS_SET(obj->value[2],REST_AT))
    {
        act_new("You wake up and rest at $p.",
            ch,obj,NULL,TO_CHAR,P_SLEEP,0);
        act("$n wakes up and rests at $p.",ch,obj,NULL,TO_ROOM, 0);
    }
        else if (IS_SET(obj->value[2],REST_ON))
        {
            act_new("You wake up and rest on $p.",
                    ch,obj,NULL,TO_CHAR,P_SLEEP, 0);
            act("$n wakes up and rests on $p.",ch,obj,NULL,TO_ROOM, 0);
        }
        else
        {
            act_new("You wake up and rest in $p.",
                    ch,obj,NULL,TO_CHAR,P_SLEEP, 0);
            act("$n wakes up and rests in $p.",ch,obj,NULL,TO_ROOM, 0);
        }
    ch->position = P_REST;
    break;

    case P_REST:
    send_to_char( "You are already resting.\n\r", ch );
    break;

    case P_STAND:
    if (obj == NULL)
    {
        send_to_char( "You rest.\n\r", ch );
        act( "$n sits down and rests.", ch, NULL, NULL, TO_ROOM, 0 );
    }
        else if (IS_SET(obj->value[2],REST_AT))
        {
        act("You sit down at $p and rest.",ch,obj,NULL,TO_CHAR, 0);
        act("$n sits down at $p and rests.",ch,obj,NULL,TO_ROOM, 0);
        }
        else if (IS_SET(obj->value[2],REST_ON))
        {
        act("You sit on $p and rest.",ch,obj,NULL,TO_CHAR, 0);
        act("$n sits on $p and rests.",ch,obj,NULL,TO_ROOM, 0);
        }
        else
        {
        act("You rest in $p.",ch,obj,NULL,TO_CHAR, 0);
        act("$n rests in $p.",ch,obj,NULL,TO_ROOM, 0);
        }
    ch->position = P_REST;
    break;

    case P_SIT:
    if (obj == NULL)
    {
        send_to_char("You rest.\n\r",ch);
        act("$n rests.",ch,NULL,NULL,TO_ROOM, 0);
    }
        else if (IS_SET(obj->value[2],REST_AT))
        {
        act("You rest at $p.",ch,obj,NULL,TO_CHAR, 0);
        act("$n rests at $p.",ch,obj,NULL,TO_ROOM, 0);
        }
        else if (IS_SET(obj->value[2],REST_ON))
        {
        act("You rest on $p.",ch,obj,NULL,TO_CHAR, 0);
        act("$n rests on $p.",ch,obj,NULL,TO_ROOM, 0);
        }
        else
        {
        act("You rest in $p.",ch,obj,NULL,TO_CHAR, 0);
        act("$n rests in $p.",ch,obj,NULL,TO_ROOM, 0);
    }
    ch->position = P_REST;
    break;
    }


    return;
}


void do_sit (CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj = NULL;

    CheckCH(ch);

    if (ch->position == P_FIGHT)
    {
    send_to_char("Maybe you should finish this fight first?\n\r",ch);
    return;
    }

    /* okay, now that we know we can sit, find an object to sit on */
    if (!IS_NULLSTR(argument))
    {
    obj = get_obj_list(ch,argument,ch->in_room->contents);
    if (obj == NULL)
    {
        send_to_char("You don't see that here.\n\r",ch);
        return;
    }
    }
    else obj = ch->on;

    if (obj != NULL)                                                              
    {
    if (obj->item_type != ITEM_FURNITURE
    ||  (!IS_SET(obj->value[2],SIT_ON)
    &&   !IS_SET(obj->value[2],SIT_IN)
    &&   !IS_SET(obj->value[2],SIT_AT)))
    {
        send_to_char("You can't sit on that.\n\r",ch);
        return;
    }

    if (obj != NULL && ch->on != obj && count_users(obj) >= obj->value[0])
    {
        act_new("There's no more room on $p.",ch,obj,NULL,TO_CHAR,P_DEAD,0);
        return;
    }

    ch->on = obj;
    }
    switch (ch->position)
    {
    case P_SLEEP:
        if (IS_AFFECTED(ch,AFF_SLEEP))
        {
        send_to_char("You can't wake up!\n\r",ch);
        return;
        }

            if (obj == NULL)
            {
                send_to_char( "You wake and sit up.\n\r", ch );
                act( "$n wakes and sits up.", ch, NULL, NULL, TO_ROOM, 0 );
            }
            else if (IS_SET(obj->value[2],SIT_AT))
            {
                act_new("You wake and sit at $p.",ch,obj,NULL,TO_CHAR,P_DEAD,0);
                act("$n wakes and sits at $p.",ch,obj,NULL,TO_ROOM, 0);
            }
            else if (IS_SET(obj->value[2],SIT_ON))
            {
                act_new("You wake and sit on $p.",ch,obj,NULL,TO_CHAR,P_DEAD,0);
                act("$n wakes and sits at $p.",ch,obj,NULL,TO_ROOM, 0);
            }
            else
            {
                act_new("You wake and sit in $p.",ch,obj,NULL,TO_CHAR,P_DEAD,0);
                act("$n wakes and sits in $p.",ch,obj,NULL,TO_ROOM, 0);
            }

        ch->position = P_SIT;
        break;
    case P_REST:
        if (obj == NULL)
        send_to_char("You stop resting.\n\r",ch);
        else if (IS_SET(obj->value[2],SIT_AT))
        {
        act("You sit at $p.",ch,obj,NULL,TO_CHAR, 0);
        act("$n sits at $p.",ch,obj,NULL,TO_ROOM, 0);
        }

        else if (IS_SET(obj->value[2],SIT_ON))
        {
        act("You sit on $p.",ch,obj,NULL,TO_CHAR, 0);
        act("$n sits on $p.",ch,obj,NULL,TO_ROOM, 0);
        }
        ch->position = P_SIT;
        break;
    case P_SIT:
        send_to_char("You are already sitting down.\n\r",ch);
        break;
    case P_STAND:
        if (obj == NULL)
            {
        send_to_char("You sit down.\n\r",ch);
                act("$n sits down on the ground.",ch,NULL,NULL,TO_ROOM, 0);
        }
        else if (IS_SET(obj->value[2],SIT_AT))
        {
        act("You sit down at $p.",ch,obj,NULL,TO_CHAR, 0);
        act("$n sits down at $p.",ch,obj,NULL,TO_ROOM, 0);
        }
        else if (IS_SET(obj->value[2],SIT_ON))
        {
        act("You sit on $p.",ch,obj,NULL,TO_CHAR, 0);
        act("$n sits on $p.",ch,obj,NULL,TO_ROOM, 0);
        }
        else
        {
        act("You sit down in $p.",ch,obj,NULL,TO_CHAR, 0);
        act("$n sits down in $p.",ch,obj,NULL,TO_ROOM, 0);
        }
            ch->position = P_SIT;
            break;
    }
    return;
}


void do_sleep( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj = NULL;

    CheckCH(ch);

    switch ( ch->position )
    {
    case P_SLEEP:
        send_to_char( "You are already sleeping.\n\r", ch );
        break;

    case P_REST:
    case P_SIT:
    case P_STAND:
        if (IS_NULLSTR(argument) && ch->on == NULL)
        {
            send_to_char( "You go to sleep.\n\r", ch );
            act( "$n goes to sleep.", ch, NULL, NULL, TO_ROOM, 0 );
            ch->position = P_SLEEP;
        }
        else  /* find an object and sleep on it */
        {
            if (IS_NULLSTR(argument))
                obj = ch->on;
            else
                obj = get_obj_list( ch, argument,  ch->in_room->contents );

            if (obj == NULL)
            {
                send_to_char("You don't see that here.\n\r",ch);
                return;
            }
            if (obj->item_type != ITEM_FURNITURE
                    ||  (!IS_SET(obj->value[2],SLEEP_ON)
                            &&   !IS_SET(obj->value[2],SLEEP_IN)
                            &&   !IS_SET(obj->value[2],SLEEP_AT)))
            {
                send_to_char("You can't sleep on that!\n\r",ch);
                return;
            }

            if (ch->on != obj && count_users(obj) >= obj->value[0])
            {
                act_new("There is no room on $p for you.",
                        ch,obj,NULL,TO_CHAR,P_DEAD,0);
                return;
            }

            ch->on = obj;
            if (IS_SET(obj->value[2],SLEEP_AT))
            {
                act("You go to sleep at $p.",ch,obj,NULL,TO_CHAR, 0);
                act("$n goes to sleep at $p.",ch,obj,NULL,TO_ROOM, 0);
            }
            else if (IS_SET(obj->value[2],SLEEP_ON))
            {
                act("You go to sleep on $p.",ch,obj,NULL,TO_CHAR, 0);
                act("$n goes to sleep on $p.",ch,obj,NULL,TO_ROOM, 0);
            }
            else
            {
                act("You go to sleep in $p.",ch,obj,NULL,TO_CHAR, 0);
                act("$n goes to sleep in $p.",ch,obj,NULL,TO_ROOM, 0);
            }
            ch->position = P_SLEEP;
        }
        break;

    case P_FIGHT:
        send_to_char( "You are already fighting!\n\r", ch );
        break;
    }

    return;
}



void do_wake( CHAR_DATA *ch, char *argument )
{
    char arg[MIL]={'\0'};
    CHAR_DATA *victim;

    CheckCH(ch);

    one_argument( argument, arg );
    if ( IS_NULLSTR(arg) )
    { do_function( ch, &do_stand, argument ); return; }

    if ( !IS_AWAKE(ch) )
    { send_to_char( "You are asleep yourself!\n\r",       ch ); return; }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    { send_to_char( "They aren't here.\n\r",              ch ); return; }

    if ( IS_AWAKE(victim) )
    { act( "$N is already awake.", ch, NULL, victim, TO_CHAR, 0 ); return; }

    if ( IS_AFFECTED(victim, AFF_SLEEP) || IS_SET(victim->act2, ACT2_ASTRAL))
    { act( "You can't wake $M!",   ch, NULL, victim, TO_CHAR, 0 ); return; }

    act_new( "$n wakes you.", ch, NULL, victim, TO_VICT,P_SLEEP, 0 );
    do_function( victim, &do_stand, "" );
    return;
}


void do_sneak( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA af;

    CheckCH(ch);

    send_to_char( "You attempt to move silently.\n\r", ch );
    affect_strip( ch, gsn_sneak );

    if (IS_AFFECTED(ch,AFF_SNEAK))
        return;

    if ( number_percent( ) < get_skill(ch,gsn_sneak))
    {
        af.where     = TO_AFFECTS;
        af.type      = gsn_sneak;
        af.level     = ch->trust;
        af.duration  = ch->trust;
        af.location  = APPLY_NONE;
        af.modifier  = 0;
        af.bitvector = AFF_SNEAK;
        affect_to_char( ch, &af );
    }

    return;
}


void do_hide( CHAR_DATA *ch, char *argument )
{
    CheckCH(ch);

    send_to_char( "You attempt to hide.\n\r", ch );

    if ( IS_AFFECTED(ch, AFF_HIDE) )
        REMOVE_BIT(ch->affected_by, AFF_HIDE);

    if ( dice_rolls(ch, get_curr_stat(ch,STAT_DEX) + ch->ability[STEALTH].value, 6) > 0 )
    {
        SET_BIT(ch->affected_by, AFF_HIDE);
    }

    return;
}



/*
 * Contributed by Alander.
 */
void do_visible( CHAR_DATA *ch, char *argument )
{
    CheckCH(ch);
    affect_strip ( ch, gsn_invis            );
    affect_strip ( ch, gsn_sneak            );
    REMOVE_BIT   ( ch->affected_by, AFF_HIDE        );
    REMOVE_BIT   ( ch->affected_by, AFF_INVISIBLE   );
    REMOVE_BIT   ( ch->affected_by, AFF_SNEAK       );
    send_to_char( "Ok.\n\r", ch );
    return;
}


struct hash_link
{
    int         key;
    struct hash_link    *next;
    void            *data;
};

struct hash_header
{
    int         rec_size;
    int         table_size;
    int         *keylist, klistsize, klistlen; /* this is really lame, AMAZINGLY lame */
    struct hash_link    **buckets;
};

#define WORLD_SIZE  30000
#define HASH_KEY(ht,key)((((unsigned int)(key))*17)%(ht)->table_size)



struct hunting_data
{
    char            *name;
    struct char_data    **victim;
};

struct room_q
{
    int     room_nr;
    struct room_q   *next_q;
};

struct nodes
{
    int visited;
    int ancestor;
};

#define IS_DIR      (get_room_index(q_head->room_nr)->exit[i])
#define GO_OK       (!IS_SET( IS_DIR->exit_info, EX_CLOSED ))
#define GO_OK_SMARTER   1



#if defined( NO_BCOPY )
void bcopy(register char *s1,register char *s2,int len)
{
    while( len-- ) *(s2++) = *(s1++);
}
#endif

#if defined( NO_BZERO )
void bzero(register char *sp,int len)
{
    while( len-- ) *(sp++) = '\0';
}
#endif



void init_hash_table(struct hash_header *ht,int rec_size,int table_size)
{
    ht->rec_size    = rec_size;
    ht->table_size  = table_size;
    ht->buckets     = (struct hash_link**)calloc(sizeof(struct hash_link**),table_size);
    ht->keylist     = (int*)malloc(sizeof(ht->keylist)*(ht->klistsize=128));
    ht->klistlen    = 0;
}

void init_world(ROOM_INDEX_DATA *room_db[])
{
    /* zero out the world */
    bzero((char *)room_db,sizeof(ROOM_INDEX_DATA *)*WORLD_SIZE);
}

void destroy_hash_table(struct hash_header *ht,void (*gman)())
{
    int         i;
    struct hash_link    *scan,*temp;

    for(i=0;i<ht->table_size;i++)
        for(scan=ht->buckets[i];scan;)
        {
            temp = scan->next;
            free(scan);
            scan = temp;
        }
    free(ht->buckets);
    free(ht->keylist);
}

void _hash_enter(struct hash_header *ht,int key,void *data)
{
    /* precondition: there is no entry for <key> yet */
    struct hash_link    *temp;
    int         i;

    temp        = (struct hash_link *)malloc(sizeof(struct hash_link));
    temp->key   = key;
    temp->next  = ht->buckets[HASH_KEY(ht,key)];
    temp->data  = data;
    ht->buckets[HASH_KEY(ht,key)] = temp;
    if(ht->klistlen>=ht->klistsize)
    {
        ht->keylist = (int*)realloc(ht->keylist,sizeof(*ht->keylist)*(ht->klistsize*=2));
    }
    for(i=ht->klistlen;i>=0;i--)
    {
        if(ht->keylist[i-1]<key)
        {
            ht->keylist[i] = key;
            break;
        }
        ht->keylist[i] = ht->keylist[i-1];
    }
    ht->klistlen++;
}

ROOM_INDEX_DATA *room_find(ROOM_INDEX_DATA *room_db[],int key)
{
    return((key<WORLD_SIZE&&key>-1)?room_db[key]:0);
}

void *hash_find(struct hash_header *ht,int key)
{
    struct hash_link *scan;

    scan = ht->buckets[HASH_KEY(ht,key)];

    while(scan && scan->key!=key)
        scan = scan->next;

    return scan ? scan->data : NULL;
}

int room_enter(ROOM_INDEX_DATA *rb[],int key,ROOM_INDEX_DATA *rm)
{
    ROOM_INDEX_DATA *temp;

    temp = room_find(rb,key);
    if(temp) return(0);

    rb[key] = rm;
    return(1);
}

int hash_enter(struct hash_header *ht,int key,void *data)
{
    void *temp;

    temp = hash_find(ht,key);
    if(temp) return 0;

    _hash_enter(ht,key,data);
    return 1;
}

ROOM_INDEX_DATA *room_find_or_create(ROOM_INDEX_DATA *rb[],int key)
{
  ROOM_INDEX_DATA *rv;

  rv = room_find(rb,key);
  if(rv) return rv;

  rv = (ROOM_INDEX_DATA *)malloc(sizeof(ROOM_INDEX_DATA));
  rb[key] = rv;
    
  return rv;
}

void *hash_find_or_create(struct hash_header *ht,int key)
{
    void *rval;

    rval = hash_find(ht, key);
    if(rval) return rval;

    rval = (void*)malloc(ht->rec_size);
    _hash_enter(ht,key,rval);

    return rval;
}

int room_remove(ROOM_INDEX_DATA *rb[],int key)
{
    ROOM_INDEX_DATA *tmp;

    tmp = room_find(rb,key);
    if(tmp)
    {
        rb[key] = 0;
        free(tmp);
    }
    return(0);
}

void *hash_remove(struct hash_header *ht,int key)
{
  struct hash_link **scan;

  scan = ht->buckets+HASH_KEY(ht,key);

  while(*scan && (*scan)->key!=key)
    scan = &(*scan)->next;

  if(*scan)
    {
      int       i;
      struct hash_link  *temp, *aux;

      temp  = (struct hash_link*)(*scan)->data;
      aux   = *scan;
      *scan = aux->next;
      free(aux);

      for(i=0;i<ht->klistlen;i++)
    if(ht->keylist[i]==key)
      break;

      if(i<ht->klistlen)
    {
      bcopy((char *)ht->keylist+i+1,(char *)ht->keylist+i,(ht->klistlen-i)
        *sizeof(*ht->keylist));
      ht->klistlen--;
    }

      return temp;
    }

  return NULL;
}

void room_iterate(ROOM_INDEX_DATA *rb[],void (*func)(),void *cdata)
{
  register int i;

  for(i=0;i<WORLD_SIZE;i++)
    {
      ROOM_INDEX_DATA *temp;
  
      temp = room_find(rb,i);
    }
}

void hash_iterate(struct hash_header *ht,void (*func)(),void *cdata)
{
  int i;

  for(i=0;i<ht->klistlen;i++)
    {
      void      *temp;
      register int  key;

      key = ht->keylist[i];
      temp = hash_find(ht,key);
 
      if(ht->keylist[i]!=key) /* They must have deleted this room */
    i--;              /* Hit this slot again. */

    }
}


int exit_ok( EXIT_DATA *pexit )
{
    ROOM_INDEX_DATA *to_room;

    if ( ( pexit == NULL ) ||   ( to_room = pexit->u1.to_room ) == NULL )
        return 0;

    return 1;
}

void donothing()
{
  return;
}

int find_path( int in_room_vnum, int out_room_vnum, CHAR_DATA *ch, int depth, int in_zone )
{
    struct room_q       *tmp_q, *q_head, *q_tail;
    struct hash_header  x_room;
    int         i, tmp_room, count=0, thru_doors;
    ROOM_INDEX_DATA *herep;
    ROOM_INDEX_DATA *startp;
    EXIT_DATA       *exitp;

    if ( depth <0 )
    {
        thru_doors = TRUE;
        depth = -depth;
    }
    else
    {
        thru_doors = FALSE;
    }

    startp = get_room_index( in_room_vnum );

    init_hash_table( &x_room, sizeof(int), 2048 );
    hash_enter( &x_room, in_room_vnum, (void *) - 1 );

    /* initialize queue */
    q_head = (struct room_q *) malloc(sizeof(struct room_q));
    q_tail = q_head;
    q_tail->room_nr = in_room_vnum;
    q_tail->next_q = 0;

    while(q_head)
    {
        herep = get_room_index( q_head->room_nr );
        /* for each room test all directions */
        if( herep->area == startp->area || !in_zone )
        {
            /* only look in this zone...
         saves cpu time and  makes world safer for players  */
            for( i = 0; i <= 5; i++ )
            {
                exitp = herep->exit[i];
                if( exit_ok(exitp) && ( thru_doors ? GO_OK_SMARTER : GO_OK ) )
                {
                    /* next room */
                    tmp_room = herep->exit[i]->u1.to_room->vnum;
                    if( tmp_room != out_room_vnum )
                    {
                        /* shall we add room to queue ?
             count determines total breadth and depth */
                        if( !hash_find( &x_room, tmp_room )
                                && ( count < depth ) )
                            /* && !IS_SET( RM_FLAGS(tmp_room), DEATH ) ) */
                        {
                            count++;
                            /* mark room as visted and put on queue */

                            tmp_q = (struct room_q *)
                        malloc(sizeof(struct room_q));
                            tmp_q->room_nr = tmp_room;
                            tmp_q->next_q = 0;
                            q_tail->next_q = tmp_q;
                            q_tail = tmp_q;

                            /* ancestor for first layer is the direction */
                            hash_enter( &x_room, tmp_room,
                                    ((int)hash_find(&x_room,q_head->room_nr)
                                            == -1) ? (void*)(i+1)
                                                    : hash_find(&x_room,q_head->room_nr));
                        }
                    }
                    else
                    {
                        /* have reached our goal so free queue */
                        tmp_room = q_head->room_nr;
                        for(;q_head;q_head = tmp_q)
                        {
                            tmp_q = q_head->next_q;
                            free(q_head);
                        }
                        /* return direction if first layer */
                        if ((int)hash_find(&x_room,tmp_room)==-1)
                        {
                            if (x_room.buckets)
                            {
                                /* junk left over from a previous track */
                                destroy_hash_table(&x_room, donothing);
                            }
                            return(i);
                        }
                        else
                        {
                            /* else return the ancestor */
                            int i;

                            i = (int)hash_find(&x_room,tmp_room);
                            if (x_room.buckets)
                            {
                                /* junk left over from a previous track */
                                destroy_hash_table(&x_room, donothing);
                            }
                            return( -1+i);
                        }
                    }
                }
            }
        }

        /* free queue head and point to next entry */
        tmp_q = q_head->next_q;
        free(q_head);
        q_head = tmp_q;
    }

    /* couldn't find path */
    if( x_room.buckets )
    {
        /* junk left over from a previous track */
        destroy_hash_table( &x_room, donothing );
    }
    return -1;
}

void do_hunt( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MSL]={'\0'};
    int direction;
    bool fArea = TRUE;

    CheckCH(ch);

    one_argument( argument, arg );

    if (IS_NPC(ch))
    {
        send_to_char("Huh?\n\r",ch);
        return;
    }

    if( IS_NULLSTR(arg) )
    {
        send_to_char( "Whom are you trying to hunt?\n\r", ch );
        return;
    }


    victim = get_char_world( ch, arg );

    if( victim == NULL )
    {
        send_to_char("No-one around by that name.\n\r", ch );
        return;
    }

    if( ch->in_room == victim->in_room )
    {
        act( "$N is here!", ch, NULL, victim, TO_CHAR, 0 );
        return;
    }

    act( "$n carefully sniffs the air.", ch, NULL, NULL, TO_ROOM, 0 );
    WAIT_STATE( ch, 5 );

    direction = find_path( ch->in_room->vnum, victim->in_room->vnum, ch, -40000, fArea );

    if( direction == -1 )
    {
        act( "You couldn't find a path to $N from here.", ch, NULL, victim, TO_CHAR, 0 );
        return;
    }

    if( direction < 0 || direction > 5 )
    {
        send_to_char( "Hmm... Something seems to be wrong.\n\r", ch );
        return;
    }


    /*
     * Give a random direction if the player misses the die roll.
     */
    if( dice_rolls( ch, (get_curr_stat(ch,STAT_PER) + ch->ability[SURVIVAL].value), 6) > 0
            && !(victim->clan == auspice_lookup("ragabash") && victim->disc[DISC_AUSPICE]) )
    {
        do
        {
            direction = number_door();
        }
        while( ( ch->in_room->exit[direction] == NULL )
                || ( ch->in_room->exit[direction]->u1.to_room == NULL) );
    }

    /*
     * Display the results of the search.
     */
    act( Format("$N is %s from here.", dir_name[direction]), ch, NULL, victim, TO_CHAR, 0 );
    return;
}


void do_drag( CHAR_DATA * ch, char * argument )
{

    char buf[MSL]={'\0'};
    OBJ_DATA  *obj;
    ROOM_INDEX_DATA *rvnum;
    CHAR_DATA *vch;
    int door;

    CheckCH(ch);

   argument = one_argument (argument, buf);

   obj = get_obj_list( ch, buf, ch->in_room->contents );

   if ( !obj )
   {
       send_to_char ( "I do not see anything like that here.\n\r", ch);
       return;
   }

   if ( (!IS_SET( obj->wear_flags, ITEM_TAKE ) &&
       (!IS_SET( obj->wear_flags, ITEM_DRAGGABLE ) ) ) )
   {
       act( "You try to drag $p, but without succes.\n\r", ch, obj, NULL, TO_CHAR, 0);
       act( "$n tries to move $p, but it doesn't budge.\n\r", ch, obj, NULL, TO_ROOM, 0);
       return;
   }

   if ( get_obj_weight(obj) >  (2 * can_carry_w (ch)) )
   {
       act( "You try, but $p is too heavy.\n\r", ch, obj, NULL, TO_CHAR, 0);
       act( "$n tries to move $p, but $? fail.\n\r", ch, obj, NULL, TO_ROOM, 0);
       return;
   }

   if ( IS_NULLSTR(argument) )
   {
       send_to_char ( "Where do you want to drag it ?\n\r", ch);
       return;
   }

   rvnum = ch->in_room;

   door = exit_lookup( ch,argument );
   if ( door == -1 )
   {
       send_to_char("You don't see exit in that direction.\n\r", ch);
       return;
   }

   move_char( ch, door, FALSE );

   if (ch->in_room == rvnum )
       return;

   obj_from_room (obj);
   obj_to_room (obj, ch->in_room);

   act ( "You dragged $p with you.\n\r", ch, obj, NULL, TO_CHAR, 0 );
   act ( "$n dragged $p with $?.\n\r", ch, obj, NULL, TO_ROOM, 0 );

   if ( !(vch = rvnum->people) )
       return;

   act ( "$N dragged $p out.\n\r", vch, obj, ch, TO_CHAR, 0 );
   act ( "$N dragged $p out.\n\r", vch, obj, ch, TO_ROOM, 0 );

   return;
}

void do_train( CHAR_DATA *ch, char *argument )
{
    char arg1[MSL]={'\0'};
    char arg2[MSL]={'\0'};
    char arg3[MSL]={'\0'};
    sh_int *pAbility;
    char *pOutput = NULL;
    int cost;
    int loop;
    int max_stat = 5;
    bool is_ok = FALSE;

    CheckCH(ch);
    CheckChNPC(ch);

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if(IS_SET(ch->plr_flags, PLR_ELDER) && !IS_ADMIN(ch))
    {
        send_to_char("As an elder you are somewhat static.\n\r", ch);
        send_to_char("You can no longer advance in the usual way.\n\r", ch);
        send_to_char("However, you are immune to player decay rules.\n\r", ch);
        return;
    }

    if(IS_ADMIN(ch) || (IS_VAMPIRE(ch) && ch->gen < 8))
        max_stat = 10;

    if (IS_NULLSTR(arg1) || (IS_NULLSTR(arg2) && !(!str_prefix(arg1, "willpower")
            || !str_prefix(arg1, "humanity") || !str_prefix(arg1, "gnosis")
            || !str_prefix(arg1, "banality") || !str_prefix(arg1, "rage")
            || !str_prefix(arg1, "faith") || !str_prefix(arg1, "glamour"))))
    {
        send_to_char( Format("Experience Points: %d.\n\r", ch->exp), ch );
        send_to_char( "SYNTAX: train <section> <attribute>\n\r",ch);
        send_to_char( "\n\r", ch );

        send_to_char( "\tYStat:\tn\n\r", ch );
        cost = 0;
        for(loop = 0; loop < (MAX_STATS - 1); loop++)
        {
            if ( ch->perm_stat[loop] < max_stat )
            {
                cost++;
                send_to_char(Format("\tW%-20s\tn", stat_table[loop].name), ch);
                if(cost % 3 == 0)
                    send_to_char("\n\r", ch);
            }
        }
        send_to_char( "\n\r", ch );

        send_to_char("\tYAbility:\tn\n\r",ch);
        cost = 0;
        for(loop = 0; loop < MAX_ABIL; loop++)
        {
            if ( ( ch->ability[loop].value < max_stat ) && ( IS_ATTRIB_AVAILABLE(ch->race, loop) ) )
            {
                if(ability_table[loop].name != NULL)
                {
                    cost++;
                    send_to_char(Format("\tW%-15s\tn", ability_table[loop].name), ch);
                    if(cost % 4 == 0)
                        send_to_char("\n\r", ch);
                }
            }
        }
        send_to_char( "\n\r", ch );

        send_to_char( "\tYVirtue:\tn\n\r", ch );
        for(loop = 0; loop < 3; loop++)
        {
            if ( ch->virtues[loop] < max_stat )
            {
                if(virtue_table[loop].name != NULL)
                {
                    send_to_char(Format("\tW%-15s\tn", virtue_table[loop].name), ch);
                }
            }
        }
        send_to_char( "\n\r\n\r", ch );

        send_to_char("\tYPowers: (Use advance to improve your range of power.)\tn\n\r",ch);
        cost = 0;
        for(loop = 0; disc_table[loop].vname != NULL; loop++)
        {
            if (ch->race == race_lookup("vampire")) pOutput = disc_table[loop].vname;
            else if (ch->race == race_lookup("werewolf")) pOutput   = disc_table[loop].wname;
            else if (ch->race == race_lookup("faerie")) pOutput = disc_table[loop].fname;
            else if (ch->race == race_lookup("human")) pOutput = disc_table[loop].hname;
            if(pOutput != NULL)
            {
                cost++;
                send_to_char(Format("\tW%-15s\tn", pOutput),ch);
                if(cost % 4 == 0)
                    send_to_char("\n\r", ch);
            }
        }
        send_to_char("\n\r", ch);

        return;
    }

    if (stat_lookup(arg2, ch) || abil_lookup(arg2, ch) || virtue_lookup(arg2)) is_ok = TRUE;

    if(!str_prefix(arg1,"stat"))
    {
        if( ( loop = stat_lookup( arg2, ch ) ) >= 0 )
        {
            pAbility    = &ch->perm_stat[loop];
            pOutput     = stat_table[loop].name;
            cost    = 4 * ch->perm_stat[loop];
        }
        else
        {
            send_to_char( "No such statistic.\n\r", ch);
            return;
        }
        if (ch->perm_stat[loop] >= max_stat)
        {
            act( "Your $T is maxed out.", ch, NULL, pOutput, TO_CHAR, 1 );
            return;
        }
        if (cost < 1) cost = 10;
        cost = xp_cost_mod(ch, cost, ch->perm_stat[loop]);
        if (cost > ch->exp)
        {
            send_to_char( "You don't have enough exp.\n\r", ch );
            return;
        }

        if(stat_lookup(arg2, ch) == stat_lookup("appearance", ch)
                && ch->clan == clan_lookup("nosferatu"))
        {
            send_to_char("The nosferatu may not have a pleasing appearance.\n\r",ch);
            return;
        }

        ch->exp -= cost;
        *pAbility += 1;
        act( Format("Your $T increases to %d!", ch->perm_stat[loop]), ch, NULL, pOutput, TO_CHAR, 1 );
        return;

    }
    else if(!str_prefix(arg1,"ability"))
    {
        if( ( loop = abil_lookup( arg2, ch ) ) >= 0
                && IS_ATTRIB_AVAILABLE(ch->race, loop) )
        {
            pAbility    = &ch->ability[loop].value;
            pOutput     = ability_table[loop].name;
            if(ch->ability[loop].value == 0)
                cost = 3;
            else
                cost = 2 * ch->ability[loop].value;
        }
        else
        {
            send_to_char( "No such ability.\n\r", ch);
            return;
        }
        if (ch->ability[loop].value >= max_stat)
        {
            act( "Your $T is maxed out.", ch, NULL, pOutput, TO_CHAR, 1 );
            return;
        }

        if (cost < 1) cost = 10;
        cost = xp_cost_mod(ch, cost, ch->ability[loop].value);
        if (cost > ch->exp)
        {
            send_to_char( "You don't have enough exp.\n\r", ch );
            return;
        }

        ch->exp -= cost;
        *pAbility += 1;
        act( Format("Your $T increases to %d!", ch->ability[loop].value), ch, NULL, pOutput, TO_CHAR, 1 );
        return;

    }
    else if(!str_prefix(arg1,"virtue"))
    {
        if( ( loop = virtue_lookup( arg2 ) ) >= 0 )
        {
            pAbility    = &ch->virtues[loop];
            pOutput     = virtue_table[loop].name;
            cost    = 5 * ch->virtues[loop];
        }
        else
        {
            send_to_char( "No such virtue.\n\r", ch);
            return;
        }
        if (ch->virtues[loop] >= max_stat)
        {
            act( "Your $T is maxed out.", ch, NULL, pOutput, TO_CHAR, 1 );
            return;
        }
        if (cost < 1) cost = 10;
        cost = xp_cost_mod(ch, cost, ch->virtues[loop]);
        if (cost > ch->exp)
        {
            send_to_char( "You don't have enough exp.\n\r", ch );
            return;
        }

        ch->exp -= cost;
        *pAbility += 1;
        act( Format("Your $T increases to %d!", ch->virtues[loop]), ch, NULL, pOutput, TO_CHAR, 1 );
        return;

    }
    else if(!str_prefix(arg1, "willpower")) {
        pOutput     = "willpower";
        cost    = 3 * ch->max_willpower;
        if (ch->max_willpower >= 10)
        {
            act( "Your $T is maxed out.", ch, NULL, pOutput, TO_CHAR, 1 );
            return;
        }
        cost = xp_cost_mod(ch, cost, ch->max_willpower);
        if (cost > ch->exp)
        {
            send_to_char( "You don't have enough exp.\n\r", ch );
            return;
        }
        ch->exp -= cost;
        ch->max_willpower += 1;
        act( Format("Your $T increases to %d!", ch->max_willpower), ch, NULL, pOutput, TO_CHAR, 1 );
        return;
    }
    else if(!str_prefix(arg1, "humanity") || !str_prefix(arg1, "gnosis")
            || !str_prefix(arg1, "banality")) {
        if(ch->race == race_lookup("human") || ch->race == race_lookup("vampire")) {
            pOutput     = "humanity";
        }
        if(ch->race == race_lookup("werewolf")) {
            pOutput     = "gnosis";
        }
        if(ch->race == race_lookup("faerie")) {
            pOutput     = "banality";
        }
        cost    = 3 * ch->max_GHB;
        if (ch->max_GHB >= 10)
        {
            act( "Your $T is maxed out.", ch, NULL, pOutput, TO_CHAR, 1 );
            return;
        }
        cost = xp_cost_mod(ch, cost, ch->max_GHB);
        if (cost > ch->exp)
        {
            send_to_char( "You don't have enough exp.\n\r", ch );
            return;
        }
        ch->exp -= cost;
        ch->max_GHB += 1;
        ch->GHB += 1;
        act( Format("Your $T increases to %d!", ch->max_GHB), ch, NULL, pOutput, TO_CHAR, 1 );
        return;
    }
    else if(!str_prefix(arg1, "rage") || !str_prefix(arg1, "faith")
            || !str_prefix(arg1, "glamour")) {
        if(ch->race == race_lookup("human")) {
            pOutput     = "faith";
        }
        else if(ch->race == race_lookup("vampire")) {
            send_to_char("SYNTAX:\n\rtrain <section> <attribute>\n\r", ch);
            return;
        }
        else if(ch->race == race_lookup("werewolf")) {
            pOutput     = "rage";
        }
        else if(ch->race == race_lookup("faerie")) {
            pOutput     = "glamour";
        }
        cost    = 3 * ch->max_RBPG;
        if (ch->max_RBPG >= 10)
        {
            act( "Your $T is maxed out.", ch, NULL, pOutput, TO_CHAR, 1 );
            return;
        }
        cost = xp_cost_mod(ch, cost, ch->max_RBPG);
        if (cost > ch->exp)
        {
            send_to_char( "You don't have enough exp.\n\r", ch );
            return;
        }
        ch->exp -= cost;
        ch->max_RBPG += 1;
        act( Format("Your $T increases to %d!", ch->max_RBPG), ch, NULL, pOutput, TO_CHAR, 1 );
        return;
    }
    else
    {
        send_to_char("SYNTAX:\n\rtrain <section> <attribute>\n\r", ch);
    }

}

bool CAN_TRAIN_POWER(CHAR_DATA *ch, int loop, int type)
{
    if(ch->train_success == TRUE)
    return TRUE;

    if(type == 1 && ch->race != race_lookup("werewolf")
    && !IS_SET(ch->powers[gift_table[loop].level-1], gift_table[loop].flag))
    return TRUE;

    if(ch->race == race_lookup("vampire")
    && (loop == ch->clan_powers[0]
    || loop == ch->clan_powers[1]
    || loop == ch->clan_powers[2]))
    return TRUE;

    if(type == 0 && ch->race == race_lookup("werewolf")
    && ch->backgrounds[RACE_STATUS] >= ch->disc[loop])
    return TRUE;

    if(type == 1 && ch->race == race_lookup("werewolf")
    && ch->backgrounds[RACE_STATUS] >= gift_table[loop].level)
    return TRUE;

    if(ch->race != race_lookup("vampire")
    && ch->race != race_lookup("werewolf"))
    return TRUE;

    return FALSE;
}

void do_basediscs( CHAR_DATA *ch, char *argument )
{
    char arg1[MSL]={'\0'};
    int loop;

    CheckCH(ch);

    if(IS_NULLSTR(argument))
    {
        if(ch->clan != clan_lookup("none"))
        {
            for(loop = 0; loop < 3; loop++)
            {
                if(ch->clan_powers[loop] < 0)
                    ch->clan_powers[loop] = clan_table[ch->clan].powers[loop];
            }
            send_to_char("Your clan disciplines are:\n\r", ch);
            for(loop = 0; loop < 3; loop++)
            {
                send_to_char(Format("%s\n\r", disc_table[ch->clan_powers[loop]].vname), ch);
            }
            return;
        }

        if(ch->clan_powers[0] != -1
                && ch->clan_powers[1] != -1
                && ch->clan_powers[2] != -1)
            send_to_char("Your base disciplines have all been selected.\n\r", ch);
        else
            send_to_char("Syntax: basedisc <discipline>\n\r", ch);

        if(ch->clan_powers[0] != -1
                || ch->clan_powers[1] != -1
                || ch->clan_powers[2] != -1)
        {
            for(loop = 0; loop < 3; loop++)
            {
                if(ch->clan_powers[loop] != -1)
                {
                    send_to_char(Format("%s\n\r", disc_table[ch->clan_powers[loop]].vname), ch);
                }
                else send_to_char("< Empty Slot >\n\r", ch);
            }
        }
        return;
    }

    if(ch->clan_powers[0] != -1
            && ch->clan_powers[1] != -1
            && ch->clan_powers[2] != -1)
    {
        send_to_char("You have already selected your core disciplines.\n\r", ch);
        return;
    }

    argument = one_argument( argument, arg1 );

    if( ( loop = disc_lookup( ch, arg1 ) ) < 0 )
    {
        send_to_char("No such discipline.\n\r", ch);
        return;
    }

    if(ch->clan_powers[0] == loop
            || ch->clan_powers[1] == loop
            || ch->clan_powers[2] == loop)
    {
        send_to_char( "You've already selected that discipline as one of your base three choices.\n\r", ch);
        return;
    }

    if(ch->clan_powers[0] == -1) ch->clan_powers[0] = loop;
    else if(ch->clan_powers[1] == -1) ch->clan_powers[1] = loop;
    else if(ch->clan_powers[2] == -1) ch->clan_powers[2] = loop;

    ch->disc[loop]++;
    send_to_char(Format("%s has been added as one of your base disciplines.\n\r", disc_table[loop].vname), ch);
}

void do_train_power( CHAR_DATA *ch, char *argument )
{
    char arg1[MSL]={'\0'};
    char *pOutput = NULL;
    int cost;
    int loop;
    int max_stat = 10;

    CheckCH(ch);
    CheckChNPC(ch);

    argument = one_argument( argument, arg1 );

    if(IS_SET(ch->plr_flags, PLR_ELDER) && !IS_ADMIN(ch))
    {
        send_to_char("As an elder you are somewhat static.\n\r", ch);
        send_to_char("You can no longer advance in the usual way.\n\r", ch);
        send_to_char("However, you are immune to player decay rules.\n\r", ch);
        return;
    }

    if( !IS_NULLSTR(arg1) )
    {
        send_to_char( Format("You have %d experience points.\n\r", ch->exp), ch );
        send_to_char("SYNTAX: advance <power>\n\r",ch);
        send_to_char( "You can train the following:\n\r", ch );

        send_to_char("Powers: \n\r",ch);
        for(loop = 0; disc_table[loop].vname != NULL; loop++)
        {
            if (ch->race == race_lookup("vampire"))
                pOutput = disc_table[loop].vname;
            else if (ch->race == race_lookup("werewolf") && disc_table[loop].wname)
                pOutput = disc_table[loop].wname;
            else if (ch->race == race_lookup("faerie") && disc_table[loop].fname)
                pOutput = disc_table[loop].fname;
            else if (ch->race == race_lookup("human") && disc_table[loop].hname)
                pOutput = disc_table[loop].hname;
            send_to_char(Format("%s ", pOutput),ch);
        }
        send_to_char("\n\r", ch);

        return;
    }

    if( ( loop = disc_lookup( ch, arg1 ) ) >= 0 )
    {
        if (ch->race == race_lookup("vampire")) pOutput = disc_table[loop].vname;
        else if (ch->race == race_lookup("werewolf")) pOutput   = disc_table[loop].wname;
        else if (ch->race == race_lookup("faerie")) pOutput = disc_table[loop].fname;
        else if (ch->race == race_lookup("human")) pOutput = disc_table[loop].hname;

        if(ch->race == race_lookup("vampire"))
            max_stat = UMIN(10, UMAX(5, 13 - ch->gen));

        if (ch->disc[disc_table[loop].index] >= max_stat)
        {
            act( "Your $T is maxed out.", ch, NULL, pOutput, TO_CHAR, 1 );
            return;
        }

        if (ch->race == race_lookup("werewolf")
                && ch->disc[disc_table[loop].index] >= ch->backgrounds[RACE_STATUS])
        {
            act( "You cannot raise your $T until your rank increases.",
                    ch, NULL, pOutput, TO_CHAR, 1 );
            return;
        }

        if (ch->disc[loop] == 0)
            cost = 10;
        else if(ch->race == race_lookup("vampire")
                && ch->clan == clan_lookup("none"))
            cost = 7 * ch->disc[loop];
        else if(loop == clan_table[ch->clan].powers[0]
                                                    || loop == clan_table[ch->clan].powers[1]
                                                                                           || loop == clan_table[ch->clan].powers[2])
            cost = 5 * ch->disc[loop];
        else
            cost = 10 * ch->disc[loop];

        if (cost < 1) cost = 10;
        cost = xp_cost_mod(ch, cost, ch->disc[loop]);
        if (cost > ch->exp)
        {
            send_to_char( "You don't have enough exp.\n\r", ch );
            return;
        }

        if((ch->disc[loop] >= 5 && ch->race == race_lookup("vampire") && ch->gen > 7)
                ||(ch->disc[loop] >= 10 && ch->race == race_lookup("vampire") && ch->gen < 8)
                || (ch->disc[loop] >= 10 && ch->race == race_lookup("werewolf"))
                || (ch->disc[loop] >= 5 && ch->race == race_lookup("faerie"))
                || (ch->disc[loop] >= 5 && ch->race == race_lookup("human")) )
        {
            act( "Your $T is at a maximum.", ch, NULL, pOutput, TO_CHAR, 1 );
            return;
        }

        if(CAN_TRAIN_POWER(ch, loop, 0))
        {
            ch->train_success = FALSE;
            PURGE_DATA(ch->to_learn);
            ch->to_learn = str_dup("None");
            ch->exp -= cost;
            ch->disc[loop] += 1;
            if(ch->race == race_lookup("werewolf"))
                SET_BIT(ch->powers[loop],
                        group_gift_lookup(pOutput, ch->disc[loop]-1));
            act( Format("Your $T increases to %d!", ch->disc[loop]), ch, NULL, pOutput, TO_CHAR, 0 );
        }
        else
        {
            PURGE_DATA(ch->to_learn);
            ch->to_learn = str_dup(pOutput);
            ch->train_success = FALSE;
            act( "You prepare to learn $T.", ch, NULL, pOutput, TO_CHAR, 1 );
        }
        return;

    }
    else
    {
        send_to_char("No such power.\n\r", ch);
        send_to_char("SYNTAX:\n\radvance <power>\n\r", ch);
        return;
    }
}


void do_learn( CHAR_DATA *ch, char *argument )
{
    char arg1[MSL]={'\0'};
    /*    char arg2[MSL]={'\0'};
     *    char arg3[MSL]={'\0'};
     */
     char *pOutput = NULL;
     int cost = 0;
     int loop;
     int count;

     CheckCH(ch);
     CheckChNPC(ch);

     argument = one_argument( argument, arg1 );
     /*    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
      */


     if (!IS_NULLSTR(arg1))
     {
         send_to_char( Format("You have %d experience points.\n\r", ch->exp), ch );
         send_to_char("SYNTAX: learn <gift>\n\r",ch);
         send_to_char( "You can learn the following:\n\r", ch );

         count = 1;
         for(cost = 1; cost < 6; cost++) {
             send_to_char(Format("Rank %d Gifts:\n\r", cost), ch);
             for(loop = 0; gift_table[loop].name; loop++)
             {
                 if(!IS_SET(ch->powers[gift_table[loop].level], gift_table[loop].flag)
                         && gift_table[loop].level == cost)
                 {
                     send_to_char(Format("%26s", gift_table[loop].name), ch);
                     count++;
                     if(count%3 == 0)
                         send_to_char("\n\r", ch);
                 }
             }
             if(count%3 != 0) send_to_char("\n\r", ch);
             send_to_char( "\n\r", ch );
         }

         return;
     }

     if((loop = gift_lookup(arg1)) >= 0)
     {
         if (IS_SET(ch->powers[gift_table[loop].level-1], gift_table[loop].flag))
         {
             send_to_char("You have already learnt that gift.\n\r", ch);
             return;
         }

         pOutput     = gift_table[loop].name;
         cost   = 4 * gift_table[loop].level;

         if (cost < 1) cost = 10;
         if (cost > ch->exp)
         {
             send_to_char( "You don't have enough exp.\n\r", ch );
             return;
         }

         if(CAN_TRAIN_POWER(ch, loop, 1))
         {
             ch->train_success = FALSE;
             PURGE_DATA(ch->to_learn);
             ch->to_learn = str_dup("None");
             ch->exp -= cost;
             SET_BIT(ch->powers[gift_table[loop].level-1],gift_table[loop].flag);
             act( "You learn $T!", ch, NULL, pOutput, TO_CHAR, 1 );
         }
         else
         {
             PURGE_DATA(ch->to_learn);
             ch->to_learn = str_dup(pOutput);
             /* @@@@@ ch->train_success = FALSE; */
             ch->train_success = TRUE;
             act( "You prepare to learn $T.", ch, NULL, pOutput, TO_CHAR, 1 );
         }
     }
     else
     {
         send_to_char("SYNTAX: learn <gift>\n\r", ch);
     }
}


void blood_drink(CHAR_DATA *drinker, CHAR_DATA *victim, int blood, int aggro)
{
    int bite, dodge;

    bite = 0;
    dodge = 0;

    if(aggro)
    {
        bite = get_curr_stat(drinker, STAT_DEX) + drinker->ability[BRAWL].value;
        dodge = get_curr_stat(victim, STAT_DEX) + victim->ability[DODGE].value;
        if(dice_rolls(drinker, bite, 6) < dice_rolls(victim, dodge, 6))
        {
            if(IS_NPC(victim))
            {
                if(IS_AWAKE(victim))
                    multi_hit(victim, drinker, TYPE_UNDEFINED);
                return;
            }
            else
            {
                act_new("$n tries to sink $s fangs into you!", drinker, NULL, victim, TO_VICT, P_REST, 0);
                act_new("You try to sink your fangs into $N but fail!", drinker, NULL, victim, TO_CHAR, P_REST, 0);
                return;
            }
        }
        else
        {
            if(blood == 1)
            {
                if(drinker->RBPG < drinker->max_RBPG)
                {
                    drinker->RBPG++;
                    gain_condition(drinker, COND_THIRST, 10);
                    if(victim->race == race_lookup("vampire") && victim->RBPG)
                        if(victim->RBPG > 0)
                            victim->RBPG -= 1;
                        else
                            victim->agghealth--;
                    else
                        victim->agghealth--;
                    update_pos(victim, 1);
                    act_new("$n sinks $s fangs into you!", drinker, NULL, victim, TO_VICT, P_REST, 0);
                    act_new("You sink your fangs into $N!", drinker, NULL, victim, TO_CHAR, P_REST, 0);
                }
                else
                {
                    act_new("$n sinks $s fangs into you!", drinker, NULL, victim, TO_VICT, P_REST, 0);
                    act_new("You sink your fangs into $N but find yourself too full to drink.", drinker, NULL, victim, TO_CHAR, P_REST, 0);
                }
                return;
            }
            else
            {
                if(drinker->RBPG < drinker->max_RBPG)
                {
                    drinker->RBPG += UMIN(blood, drinker->max_RBPG - drinker->RBPG);
                    gain_condition(drinker, COND_THIRST, blood * 10);
                    if(victim->race == race_lookup("vampire") && victim->RBPG)
                        if(victim->RBPG > 0)
                            victim->RBPG -= blood;
                        else
                            victim->agghealth -= blood/2;
                    else
                        victim->agghealth -= blood/2;
                    update_pos(victim, 1);
                    act_new("$n sinks $s fangs into you!", drinker, NULL, victim, TO_VICT, P_REST, 0);
                    act_new("You sink your fangs into $N!", drinker, NULL, victim, TO_CHAR, P_REST, 0);
                }
                else
                {
                    act_new("$n sinks $s fangs into you!", drinker, NULL, victim, TO_VICT, P_REST, 0);
                    act_new("You sink your fangs into $N but find yourself too full to drink.", drinker, NULL, victim, TO_CHAR, P_REST, 0);
                }
                return;
            }
        }
    }
    else
    {
        if(blood == 1)
        {
            if(victim->RBPG <= blood+1)
            {
                send_to_char("You have to have at least one blood point left.\n\r", victim);
                return;
            }
            if(victim->race == race_lookup("vampire") && victim->RBPG)
                victim->RBPG -= 1;
            else
                victim->agghealth--;
            update_pos(victim, 1);
            if(drinker->RBPG < drinker->max_RBPG)
            {
                drinker->RBPG++;
                gain_condition(drinker, COND_THIRST, blood * 10);
            }
            return;
        }
        else
        {
            if(victim->RBPG <= blood+1)
            {
                send_to_char("You have to have at least one blood point left.\n\r", victim);
                return;
            }
            if(victim->race == race_lookup("vampire") && victim->RBPG)
                victim->RBPG -= blood;
            else
                victim->agghealth -= blood/2;
            update_pos(victim, 1);
            if(drinker->RBPG < drinker->max_RBPG)
            {
                drinker->RBPG += UMIN(blood, drinker->max_RBPG - drinker->RBPG);
                gain_condition(drinker, COND_THIRST, blood * 10);
            }
            return;
        }
    }
}

void do_feed(CHAR_DATA *ch, char *string)
{
    char arg1[MSL]={'\0'};
    char arg2[MSL]={'\0'};
    char arg3[MSL]={'\0'};
    CHAR_DATA *victim;
    int i = 0;

    CheckCH(ch);

    string = one_argument(string, arg1);
    string = one_argument(string, arg2);
    string = one_argument(string, arg3);

    if(IS_NULLSTR(arg1))
    {
        if(ch->race != race_lookup("vampire"))
        {
            send_to_char("Feed whom?\n\r", ch);
        }
        else
        {
            send_to_char("feed [target]\n\r", ch);
            send_to_char("feed from [target]\n\r", ch);
        }
        return;
    }

    if((victim = get_char_room(ch, arg1)) == NULL && str_cmp(arg1, "from"))
    {
        if(ch->race != race_lookup("vampire"))
        {
            send_to_char("Feed whom?\n\r", ch);
        }
        else
        {
            send_to_char("feed [target]\n\r", ch);
            send_to_char("feed from [target]\n\r", ch);
        }
        return;
    }

    if(ch == victim)
    {
        send_to_char("Why would you want to feed from yourself?\n\r", ch);
        return;
    }

    if(ch->position == P_FIGHT)
    {
        send_to_char("Because you're already fighting them it's difficult to feed.", ch);
        return;
    }

    if(!str_cmp(arg1, "from"))
    {
        if(ch->race != race_lookup("vampire"))
        {
            send_to_char("Feed whom?\n\r", ch);
            return;
        }

        if((victim = get_char_room(ch, arg2)) == NULL)
        {
            send_to_char("Feed from whom?\n\r", ch);
            return;
        }

        if(ch == victim)
        {
            send_to_char("Why would you want to feed from yourself?\n\r", ch);
            return;
        }

        if(IS_SET(victim->act2, ACT2_RP_ING))
        {
            send_to_char("You cannot use ooc feeding on an RP-ing player.\n\r", ch);
            return;
        }

        if(!IS_NULLSTR(arg3) && !is_number(arg3))
        {
            send_to_char("How much do you want to bleed?", ch);
            return;
        }
        else if(is_number(arg3))
        {
            i = atoi(arg3);
            if(i > 3)
            {
                i = 3;
                send_to_char("Feeding has been limited to 3 blood points per feed.\n\r", ch);
            }
            blood_drink(ch, victim, i, TRUE);
            WAIT_STATE(ch, 3);
        }
        else
        {
            blood_drink(ch, victim, 1, TRUE);
            WAIT_STATE(ch, 3);
        }

        return;
    }

    if(!is_number(arg2) && !IS_NULLSTR(arg2) )
    {
        send_to_char("How much do you want to bleed?", ch);
        return;
    }

    if( (i = atoi(arg2)) <= 0 )
        blood_drink(victim, ch, 1, FALSE);
    else
        blood_drink(victim, ch, i, FALSE);
    WAIT_STATE(ch, 2);

}

void healing (CHAR_DATA *ch, CHAR_DATA *vch, int num)
{
    int tmp1=0, tmp2=0;
    int healed = UMIN(ch->RBPG - 1, num);

    if(vch->health <= MAX_HEALTH)
    {
        tmp1 = UMIN(healed, (MAX_HEALTH - vch->health));
        vch->health += tmp1;
        act(Format("You heal $N of %d health levels of damage.\n\r", tmp1), ch, NULL, vch, TO_CHAR, 0);
        act(Format("$n heals you of %d health levels of damage.\n\r", tmp1), ch, NULL, vch, TO_VICT, 0);
    }

    if(vch->agghealth <= MAX_HEALTH)
    {
        tmp2 = UMIN(healed/5, (MAX_HEALTH - vch->agghealth));
        vch->agghealth += tmp2;
        act(Format("You heal $N of %d health levels of aggravated damage.\n\r", tmp2), ch, NULL, vch, TO_CHAR, 0);
        act(Format("$n heals you of %d health levels of aggravated damage.\n\r", tmp2), ch, NULL, vch, TO_VICT, 0);
    }

    healed = tmp1 + tmp2 * 5;
    ch->RBPG -= healed;
}

void do_heal (CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *vch;
    char arg1[MSL]={'\0'};
    int num = 0;

    CheckCH(ch);

    argument = one_argument(argument, arg1);

    if(!IS_NULLSTR(arg1))
    {
        send_to_char("Heal who?\n\r", ch);
        return;
    }

    if((vch = get_char_room(ch, arg1)) == NULL)
    {
        send_to_char("Heal who?\n\r", ch);
        return;
    }

    num = 1;

    healing(ch,vch,num);
}

void do_xpgift (CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *vict;
    char arg1[MSL]={'\0'};
    char arg2[MSL]={'\0'};
    int gift = 0;

    CheckCH(ch);

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if(IS_SET(ch->act2, ACT2_NOGIFT))
    {
        send_to_char("Your gift-giving rights have been revoked.\n\r", ch);
        return;
    }

    if(!IS_NULLSTR(arg1))
    {
        send_to_char("Give experience to whom?\n\r", ch);
        return;
    }

    if(ch->xpgift <= 0)
    {
        send_to_char("You don't have any experience available to give.\n\r", ch);
        return;
    }

    if((vict = get_char_world(ch, arg1)) == NULL)
    {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if(IS_NPC(vict))
    {
        send_to_char("Not to mobs.\n\r", ch);
        return;
    }

    if(IS_SET(vict->act2, ACT2_NOGIFT))
    {
        act("$N cannot receive gift xp.", ch, vict, NULL, TO_CHAR, 1);
        return;
    }

    if(ch == vict)
    {
        send_to_char("You cannot reward your own roleplaying.\n\r", ch);
        return;
    }

    if(!is_number(arg2) && !IS_NULLSTR(arg2) )
    {
        send_to_char("How much xp do you want to give?\n\r", ch);
        return;
    }

    if((vict->description[0] == '\0' || strlen(vict->description) < 10) && vict->played > 10*60*60)
    {
        send_to_char("How can they get experience without a description?\n\r", ch);
        return;
    }

    if(arg2 != NULL)
        gift = atoi(arg2);
    else
        gift = 1;

    if(gift <= 0)
    {
        send_to_char("That gift doesn't make sense!\n\r", ch);
        return;
    }

    gift = UMIN(gift, ch->xpgift);
    ch->xpgift -= gift;
    vict->exp   += gift;

    send_to_char(Format("You give %d experience to %s.\n\r", gift, vict->name), ch);
    send_to_char(Format("\tY%s\tn gives you \tG%d experience\tn.\n\r", ch->name, gift), vict);
}

void do_bribe (CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *target;
    CHAR_DATA *victim = NULL;
    char arg1[MSL]={'\0'};
    char arg2[MSL]={'\0'};
    char arg3[MSL]={'\0'};
    int charge = 0, fail = 0;

    CheckCH(ch);

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    argument = one_argument(argument, arg3);

    if((target = get_char_room(ch, arg1)) == NULL)
    {
        send_to_char("Who do you want to bribe?\n\r", ch);
        return;
    }

    if(!is_number(arg2))
    {
        send_to_char("How much to you want to pay?\n\r", ch);
        return;
    }

    charge = atoi(arg2);

    if(!IS_NULLSTR(arg3) )
        victim = get_char_world(ch, arg3);

    if(!IS_SET(target->act2, ACT2_POLICE) && !IS_SET(target->act2, ACT2_COURT))
    {
        send_to_char("It's no use bribing someone with no influence!", ch);
        return;
    }

    if(ch->dollars > (charge * 50))
    {
        ch->dollars -= (charge * 50);

        fail = dice_rolls(ch, get_curr_stat(ch, STAT_CHA) + ch->ability[EMPATHY].value, 9);

        if(fail > 0)
        {
            if(victim == NULL)
            {
                act("$N agrees to organise something.", ch,NULL,victim,TO_CHAR,0);
                ch->warrants -= charge;
            }
            else
            {
                act("$N agrees to organise something.", ch,NULL,victim,TO_CHAR,0);
                victim->warrants = charge;
            }
        }
        else if(fail == 0)
        {
            act("$N ignores your offer.", ch, NULL, victim, TO_CHAR, 0);
            ch->dollars += (charge * 50);
            ch->warrants += 5;
            return;
        }
        else
        {
            act("$N takes your money and stiffs you.", ch, NULL, victim, TO_CHAR, 0);
            return;
        }
    }
    else
    {
        send_to_char("You haven't got the cash to bribe an officer.\n\r", ch);
        return;
    }
}

void arrest_player (CHAR_DATA *cop)
{
    CHAR_DATA *ch;
    CHAR_DATA *next;
    int mod = 1;

    for(ch = cop->in_room->people; ch != NULL; ch = next)
    {
        next = ch->next_in_room;

        if(IS_NPC(ch)) continue;

        if(IS_ADMIN(ch)) continue;

        if(!can_see(cop, ch)) continue;

        if(ch->played < 10*60*60) continue;

        if(!str_cmp(ch->profession, "constable")) mod += 1;
        if(ch->clan == clan_lookup("fbi")) mod += 3;

        if(ch->warrants >= WARRANT_THRESHOLD * mod)
        {
            if(IS_SET(ch->concede, CEED_ARREST))
            {
                act("$N performs a dashing arrest of $n.", ch,NULL,cop,TO_ROOM,0);
                send_to_char("You are arrested and dragged off to gaol!", ch);
                char_from_room(ch);
                char_to_room(ch, get_room_index(ROOM_VNUM_JAIL));
                do_function( ch, &do_look, "" );
                ch->warrants /= 2;
            }
            else
            {
                act("$N attempt to arrest $n, but $e resists!", ch,NULL,cop,TO_ROOM,0);
                act("$N tries to arrest you, but you resist!", ch,NULL,cop,TO_CHAR,0);
                /* Start a fight and call for backup. @@@@@ */
                do_yell(cop, "Request assistance! Suspect resisting arrest!");
                ch->warrants += 20;
                SET_BIT(ch->plr_flags, PLR_ARREST);
                multi_hit(cop, ch, TYPE_UNDEFINED);
            }
        }
    }
}

void do_chase(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *vch;
    int dir, dist;
    bool fArea = TRUE;

    CheckCH(ch);

    if(IS_NULLSTR(argument))
    {
        send_to_char("Chase whom?", ch);
        return;
    }

    if((vch = get_char_world(ch, argument)) == NULL)
    {
        send_to_char("They're not here.\n\r", ch);
        return;
    }

    if(ch->in_room == vch->in_room)
    {
        send_to_char("They're right here!\n\r", ch);
        return;
    }

    if(ch->race == race_lookup("vampire"))
        dist = get_curr_stat(ch, STAT_PER) + ch->ability[STREETWISE].value + ch->disc[DISC_AUSPEX];
    else if(ch->race == race_lookup("werewolf"))
        dist = get_curr_stat(ch, STAT_PER) + ch->ability[PRIMAL_URGE].value + ch->GHB;
    else if(ch->race == race_lookup("faerie"))
        dist = get_curr_stat(ch, STAT_PER) + ch->ability[KENNING].value + ch->disc[DISC_ACTOR];
    else
        dist = get_curr_stat(ch, STAT_PER) + ch->ability[STREETWISE].value;

    if(ch->in_room->area != vch->in_room->area)
        fArea = FALSE;

    for(;dist >= 0; dist--)
    {
        dir = find_path( ch->in_room->vnum, vch->in_room->vnum, ch, (-1 * dist), fArea );
        if(dir == -1)
        {
            send_to_char("You run around but can't find any trace of your quarry.\n\r", ch);
            return;
        }

        move_char(ch, dir, FALSE);
    }
}

void do_run(CHAR_DATA *ch, char *argument)
{
    int dist, dir, max_dist;
    char arg1[MSL]={'\0'};
    char arg2[MSL]={'\0'};

    CheckCH(ch);

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if(!IS_NULLSTR(arg1) || !IS_NULLSTR(arg2))
    {
        send_to_char("Run which direction? How far?\n\r", ch);
        return;
    }

    if(!str_prefix(arg1, "north")) dir = 0;
    else if(!str_prefix(arg1, "east")) dir = 1;
    else if(!str_prefix(arg1, "south")) dir = 2;
    else if(!str_prefix(arg1, "west")) dir = 3;
    else if(!str_prefix(arg1, "up")) dir = 4;
    else if(!str_prefix(arg1, "down")) dir = 5;
    else
    {
        send_to_char("Which direction do you want to run?\n\r", ch);
        return;
    }

    if(!is_number(arg2))
    {
        send_to_char("How far do you want to run?\n\r", ch);
        return;
    }

    dist = atoi(arg2);
    if(ch->race == race_lookup("vampire"))
        max_dist = get_curr_stat(ch, STAT_STA) + ch->ability[ATHLETICS].value + ch->disc[DISC_FORTITUDE];
    else if(ch->race == race_lookup("werewolf"))
        max_dist = get_curr_stat(ch, STAT_STA) + ch->ability[ATHLETICS].value + ch->RBPG;
    else
        max_dist = get_curr_stat(ch, STAT_STA) + ch->ability[ATHLETICS].value;
    if(dist > max_dist)
    {
        dist = max_dist;
    }

    for(;dist >= 0; dist--)
    {
        if(ch->in_room->exit[dir] != NULL)
            move_char(ch, dir, FALSE);
        else
        {
            send_to_char("You come to a point where you can't go any further.\n\r", ch);
            break;
        }
    }
    send_to_char("You stop to recover from the run.\n\r", ch);
}


char *const distance[11]=
{
"right here.", "nearby to the %s.", "not far %s.", "away to the %s",
"off in the distance %s.", "a long way off to the %s",
"further than mortal eyes can see to the %s",
"beyond the range of mortal sight to the %s",
"far beyond the range of mortal sight to the %s",
"incredibly far off to the %s", "to the %s at the very edge of vision"
};

void scan_list           args((ROOM_INDEX_DATA *scan_room, CHAR_DATA *ch,
                               sh_int depth, sh_int door));
void scan_char           args((CHAR_DATA *victim, CHAR_DATA *ch,
                               sh_int depth, sh_int door));
void do_scan(CHAR_DATA *ch, char *argument)
{
    char arg1[MIL]={'\0'};
    ROOM_INDEX_DATA *scan_room;
    EXIT_DATA *pExit;
    sh_int door, depth, tag = 1;

    CheckCH(ch);

    argument = one_argument(argument, arg1);

    if (!IS_NULLSTR(arg1) || is_number(arg1))
    {
        if(!IS_NULLSTR(arg1) )
            tag = UMIN(get_curr_stat(ch, STAT_PER), atoi(arg1));

        if(!IS_SET(ch->act2, ACT2_ASTRAL))
            act("$n looks all around.", ch, NULL, NULL, TO_ROOM, 0);

        send_to_char("Looking around you see:\n\r", ch);
        scan_list(ch->in_room, ch, 0, -1);

        for (door=0;door<6;door++)
        {
            if(IS_SET(ch->act2, ACT2_ASTRAL))
                scan_room = ch->listening;
            else
                scan_room = ch->in_room;

            for (depth = 1; depth <= tag; depth++)
            {
                if ((pExit = scan_room->exit[door]) != NULL
                        && !IS_SET(pExit->exit_info, EX_CLOSED))
                {
                    scan_room = pExit->u1.to_room;
                    scan_list(pExit->u1.to_room, ch, depth, door);
                }
            }
        }
        return;
    }
    else if (!str_cmp(arg1, "n") || !str_cmp(arg1, "north")) door = 0;
    else if (!str_cmp(arg1, "e") || !str_cmp(arg1, "east"))  door = 1;
    else if (!str_cmp(arg1, "s") || !str_cmp(arg1, "south")) door = 2;
    else if (!str_cmp(arg1, "w") || !str_cmp(arg1, "west"))  door = 3;
    else if (!str_cmp(arg1, "u") || !str_cmp(arg1, "up" ))   door = 4;
    else if (!str_cmp(arg1, "d") || !str_cmp(arg1, "down"))  door = 5;
    else { send_to_char("Which way do you want to scan?\n\r", ch); return; }

    if(!IS_SET(ch->act2, ACT2_ASTRAL))
    {
        act("You peer intently $T.", ch, NULL, dir_name[door], TO_CHAR, 0);
        act("$n peers intently $T.", ch, NULL, dir_name[door], TO_ROOM, 0);
    }

    send_to_char(Format("Looking %s you see:\n\r", dir_name[door]), ch);

    if(IS_SET(ch->act2, ACT2_ASTRAL))
        scan_room = ch->listening;
    else
        scan_room = ch->in_room;

    for (depth = 1; depth < get_curr_stat(ch, STAT_PER); depth++)
    {
        if ((pExit = scan_room->exit[door]) != NULL
                && !IS_SET(pExit->exit_info, EX_CLOSED))
        {
            scan_room = pExit->u1.to_room;
            scan_list(pExit->u1.to_room, ch, depth, door);
        }
    }
    return;
}

void scan_list(ROOM_INDEX_DATA *scan_room, CHAR_DATA *ch, sh_int depth, sh_int door)
{
    CHAR_DATA *rch;

    if (scan_room == NULL) return;
    for (rch=scan_room->people; rch != NULL; rch=rch->next_in_room)
    {
        if (rch == ch) continue;
        if (!IS_NPC(rch) && rch->invis_level > get_trust(ch)) continue;
        if (can_see(ch, rch)) scan_char(rch, ch, depth, door);
    }
    return;
}

void scan_char(CHAR_DATA *victim, CHAR_DATA *ch, sh_int depth, sh_int door)
{
    extern char *const distance[];
    char buf[MSL]={'\0'};
    char buf2[MIL]={'\0'};

    buf[0] = '\0';

    strncat(buf, PERS(victim, ch), sizeof(buf));
    strncat(buf, ", ", sizeof(buf));
    snprintf(buf2, sizeof(buf2), distance[depth], dir_name[door]);
    strncat(buf, buf2, sizeof(buf) );
    strncat(buf, "\n\r", sizeof(buf));

    send_to_char(buf, ch);
    return;
}

int on_path(ROOM_INDEX_DATA *r, int path[10])
{
    int i = 0;

    for(i = 0; i < 10; i++)
    {
        if(path[i] == r->vnum)
            return TRUE;
    }

    return FALSE;
}

void do_flee(CHAR_DATA *ch, char *argument)
{
    int path[10];
    int i = 0,k = 0;

    CheckCH(ch);

    if(ch->position < P_STAND)
    {
        send_to_char("You're in no position to flee!\n\r", ch);
        send_to_char( "If you can stand up, you might want to do that first.\n\r", ch);
        return;
    }

    stop_fighting(ch, FALSE);

    for(i=0;i<10;i++) path[i] = 0;

    for(i=0;i<=(get_curr_stat(ch,STAT_STA)+ch->ability[ATHLETICS].value)/2;i++)
    {
        k = number_range(0,5);
        if(ch->in_room->exit[k]
                             && !on_path(ch->in_room->exit[k]->u1.to_room, path))
            move_char(ch, k, FALSE);
        else
            send_to_char("You pause to work out an escape route.\n\r", ch);
    }

    k = 0;
    for(i=0;i<10;i++)
    {
        if(path[i])
            k += movement_loss[get_room_index(path[i])->sector_type];
        else
            k += 4;
    }

    send_to_char("You stop to catch your breath.\n\r", ch);
    WAIT_STATE(ch, k);

    return;
}

void do_calm(CHAR_DATA *ch, char *argument)
{
    int success = 0;
    int diff = 6;

    CheckCH(ch);

    if(ch->clan == clan_lookup("brujah"))
        diff += 2;
    success = dice_rolls(ch, ch->virtues[1], diff);

    if(success > 0)
    {
        if(ch->condition[COND_FRENZY] > 30)
        {
            gain_condition(ch, COND_FRENZY, -60);
            gain_condition(ch, COND_ANGER, -60);
            gain_condition(ch, COND_PAIN, -60);
            send_to_char("You regain some self control.\n\r", ch);
        }
        else
        {
            send_to_char("Your rage is already under control.\n\r", ch);
        }
    }
    else if(success < 0)
    {
        gain_condition(ch, COND_FRENZY, 10);
        be_frenzied(ch);
        send_to_char("The rage leaps to new heights in your breast!\n\r", ch);
    }
}

void do_recall(CHAR_DATA *ch, char *argument)
{
    send_to_char("There is no recall in Project Twilight.\n\r", ch);
}

void do_practice(CHAR_DATA *ch, char *argument)
{
    send_to_char("Try train or advance.\n\r", ch);
}

void do_sacrifice(CHAR_DATA *ch, char *argument)
{
    send_to_char("There is no purpose to sacrificing or donating at the moment.\n\rBetter to leave it for the janitors to clean up.\n\r", ch);
}

OBJ_DATA *select_merchandise(CHAR_DATA *ch, CHAR_DATA *shop, int type)
{
    OBJ_DATA *obj;
    long cost;

    for ( obj = shop->carrying; obj; obj = obj->next_content )
    {
        if ( obj->wear_loc == WEAR_NONE
                &&   can_see_obj( ch, obj )
        &&   ( cost = get_cost( shop, obj, TRUE ) ) > 0 )
        {
            if (IS_OBJ_STAT(obj,ITEM_INVENTORY))
            {
                if(!type
                        && material_table[material_lookup(obj->material)].is_edible
                        && obj->item_type != ITEM_PILL ) {
                    if(ch->dollars*100 < cost) ch->dollars += cost/100 + 1;
                    return obj;
                }
                else if(type
                        &&(!material_table[material_lookup(obj->material)].is_edible
                                || obj->item_type != ITEM_PILL)) {
                    if(ch->dollars*100 < cost) ch->dollars += cost/100 + 1;
                    return obj;
                }
            }
        }
    }

    return NULL;
}

void mob_shopping(CHAR_DATA *ch)
{
    OBJ_DATA *obj;

    if(ch->drives[0] > ch->drives[1])
    {
        if((obj = select_merchandise(ch, ch->summonner, 0)) != NULL) {
            do_buy(ch, (char *)Format("%s %s", obj->name, ch->summonner->name));
        }
        ch->drives[0] = 0;
    }
    else
    {
        if((obj = select_merchandise(ch, ch->summonner, 1)) != NULL) {
            do_buy(ch, (char *)Format("%s %s", obj->name, ch->summonner->name));
        }
        ch->drives[1] = 0;
    }
}

void walk_to_summonner( CHAR_DATA *ch )
{
    int direction = 0;
    bool fArea = TRUE;
    char buf[MSL]={'\0'};

    /*
     * Needs a CHAR_DATA pointer called "hunting" or similar
     * in the char structure. "summonner" in our case.
     */

    if(ch->summonner == NULL)
    {
        send_to_char( "You have lost your way.\n\r", ch );
        REMOVE_BIT(ch->act, ACT_SUMMON);
        if(IS_SET(ch->act2, ACT2_HUNTER))
        {
            REMOVE_BIT(ch->act2, ACT2_HUNTER);
        }
        return;
    }

    if( ch->in_room == ch->summonner->in_room )
    {
        if(!IS_SET(ch->act2, ACT2_HUNTER))
        {
            act( "$N, who summonned you, is here!", ch, NULL, ch->summonner, TO_CHAR, 0 );
            REMOVE_BIT(ch->act, ACT_SUMMON);
            if(IS_SET(ch->act2, ACT2_GO_SHOP))
            {
                REMOVE_BIT(ch->act2, ACT2_GO_SHOP);
                mob_shopping(ch);
            }
        }
        else
        {
            if(!SAME_PLANE(ch, ch->summonner))
            {
                snprintf(buf, sizeof(buf), "I've finally found you %s, %s", ch->summonner->name, insult(ch->summonner));
                do_say(ch, buf);
                multi_hit(ch, ch->summonner, TYPE_UNDEFINED);
            }
        }
        return;
    }

    act( "$n looks about as if listening intently.", ch,NULL,NULL,TO_ROOM,0 );
    WAIT_STATE( ch, 5 );

    direction = find_path( ch->in_room->vnum, ch->summonner->in_room->vnum, ch, -40000, fArea );

    if( direction == -1 )
    {
        act( "You can't run to $N from here.", ch, NULL, ch->summonner, TO_CHAR, 0 );
        REMOVE_BIT(ch->act, ACT_SUMMON);
        ch->summonner = NULL; /* They've lost their target. */
        return;
    }

    if( direction < 0 || direction > 5 )
    {
        act( "You get lost trying to get to $N.", ch, NULL, ch->summonner, TO_CHAR, 0 );
        return;
    }


    /*
     * Give a random direction if the player misses the die roll.
     * leave this in, makes it interesting :)
     */

    if( dice_rolls( ch, (get_curr_stat(ch,STAT_PER) + ch->ability[SURVIVAL].value), 6) > 0 )
        if( dice_rolls( ch, (get_curr_stat(ch,STAT_PER) + ch->ability[SURVIVAL].value), 6) > 0 )
        {
            do
            {
                direction = number_door();
            }
            while( ( ch->in_room->exit[direction] == NULL )
                    || ( ch->in_room->exit[direction]->u1.to_room == NULL) );
        }

    /*
     * Can make them automatically open a door,
     * or can put a message here.
     * Got it set to auto-open at present, If you leave it like this,
     * you might want to add a hack if they are affected by passdoor.
     */

    if( IS_SET( ch->in_room->exit[direction]->exit_info, EX_CLOSED ) ) {
        if( !IS_SET( ch->in_room->exit[direction]->exit_info, EX_LOCKED ) )
        {
            do_open( ch, (char *) dir_name[direction] );
            return;
        }
        else
        {
            if(!has_key(ch, ch->in_room->exit[direction]->key)) {
                act("The door to the $t is locked, frustrating your passage to $N.", ch, dir_name[direction], ch->summonner, TO_CHAR, 0);
                do_yell(ch, "I cannot reach you master! I have failed you!");
                REMOVE_BIT(ch->act, ACT_SUMMON);
                return;
            }
            else
            {
                do_unlock(ch, (char *) dir_name[direction] );
                do_open( ch, (char *) dir_name[direction] );
                return;
            }
        }
    }

    /*
     * Make them move after target. You can do it manually.
     * But easiest to use the existing movement code.
     */
    act( Format("You follow $N's draw, %s from here.", dir_name[direction]), ch, NULL, ch->summonner, TO_CHAR, 0 );
    move_char( ch, direction, FALSE );
    return;
}

void do_fake_add(CHAR_DATA *ch, char *argument)
{
    send_to_char("You're looking for either train or advance.\n\r", ch);
}

void do_rp_join( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *location;
    CHAR_DATA *rch;
    int count = 0;
    int LOC_RP_OK = 0;

    CheckCH(ch);

    if ( !IS_NULLSTR(argument) || is_number(argument))
    {
        send_to_char( "Join whom?\n\r", ch );
        return;
    }

    if ( ( location = find_location( ch, argument ) ) == NULL )
    {
        send_to_char( "No such location.\n\r", ch );
        return;
    }

    for ( rch = location->people; rch != NULL; rch = rch->next_in_room )
    {
    if(IS_SET(rch->plr_flags, PLR_RP_OK))
        LOC_RP_OK = TRUE;
        count++;
    }

    if(!LOC_RP_OK)
    {
    send_to_char( "There doesn't seem to be anyone interested in RP there.\n\r", ch);
    return;
    }

    if (!is_room_owner(ch,location) && room_is_private(location)
    &&  (count > 1 || get_trust(ch) < MAX_LEVEL))
    {
        send_to_char( "That room is private right now.\n\r", ch );
        return;
    }

    if ( ch->fighting != NULL )
        stop_fighting( ch, TRUE );

    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if (get_trust(rch) >= ch->invis_level)
        {
            act("[OOC]: $n races away to roleplay with someone.",ch,NULL, rch,TO_VICT, 0);
        }
    }

    char_from_room( ch );
    char_to_room( ch, location );

    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if (get_trust(rch) >= ch->invis_level)
        {
            act("[OOC]: $n appears, looking to roleplay.",ch,NULL,rch,TO_VICT,0);
        }
    }

    do_function( ch, &do_look, "" );
    return;
}

void do_bail(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;

    CheckCH(ch);

    if(ch->in_room->vnum == ROOM_VNUM_JAIL)
    {
        if(!IS_NULLSTR(argument))
        {
            send_to_char( "You can't bail someone else out while you're gaoled yourself?\n\r", ch);
            return;
        }

        victim = ch;
    } else {

        if((victim = get_char_world(ch, argument)) == NULL)
        {
            send_to_char("They aren't here.\n\r", ch);
            return;
        }

        if(victim->in_room->vnum != ROOM_VNUM_JAIL)
        {
            send_to_char("They aren't jailed!\n\r", ch);
            return;
        }
    }

    if(victim->warrants * 50 > ch->dollars)
    {
        send_to_char("You don't have enough cash to meet the bail.\n\r", ch);
        return;
    }

    ch->dollars -= victim->warrants * 50;
    victim->warrants = 0;
    act("You bail out $N.",ch, NULL, victim, TO_CHAR,0);
    act("$n bails you out. Now all you have to do is wait for the paperwork.",
            ch, NULL, victim, TO_VICT, 0);
}

void do_concede(CHAR_DATA *ch, char *argument)
{
    int i = -1;
    CHAR_DATA *vch;
    char arg[MIL]={'\0'};

    CheckCH(ch);

    argument = one_argument(argument, arg);

    if(!IS_NULLSTR(argument) || !IS_NULLSTR(arg))
    {
        send_to_char("Syntax: concede <victor> <value>\n\rFlags:", ch);
        for(i = 0; concede_flags[i].name; i++)
        {
            send_to_char(concede_flags[i].name, ch);
            send_to_char(" ", ch);
        }
        send_to_char("\n\r", ch);
        return;
    }

    if((vch = get_char_room(ch, arg)) == NULL)
    {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if((i = flag_lookup(argument, concede_flags)) == NO_FLAG)
    {
        send_to_char("You cannot concede to have that done.\n\r", ch);
        return;
    }

    if(IS_SET(ch->concede, i))
    {
        REMOVE_BIT(ch->concede, i);
        send_to_char("You recind your concession.\n\r", ch);
        return;
    }

    SET_BIT(ch->concede, i);
    act("You concede to $t at the hands of $N.", ch, flag_string(concede_flags, i), vch, TO_CHAR, 1);
    act("$n concedes to $t at the hands of $N.", ch, flag_string(concede_flags, i), vch, TO_NOTVICT, 1);
    act("$n concedes to $t at your hands.", ch, flag_string(concede_flags, i), vch, TO_VICT, 1);
}

void do_step(CHAR_DATA *ch, char *argument)
{
    int i = 0;
    int count = 0;
    CHAR_DATA *rch;
    ROOM_INDEX_DATA *location;
    ORG_DATA *org;

    CheckCH(ch);

    /*
     * Added org stepping. Dsarky.
     * Added home stepping. Dsarky.
     */

    /* Added a better way of displaying the steps by adding the count feature
     * so that it would display much like the abilities and powers do.  Also
     * looks a hell of a lot better.  Rayal.
     */
    if(IS_NULLSTR(argument))
    {
        send_to_char("\tGAvailable locations are\tn:\n\r", ch);
        for(i=0;rp_area_table[i].name != NULL;i++)
        {
            if(rp_area_table[i].settable == TRUE) {
                send_to_char(Format("\t<send href='step %s'>%-20s\t</send> |", rp_area_table[i].name, rp_area_table[i].name), ch);
                count++;if(!(count%3)) send_to_char("\n\r", ch);
            }
        }
        for(org = org_list; org; org = org->next)
        {
            if(mem_lookup(org, ch->name))
            {
                send_to_char(Format("\t<send href='step %s'>%-20s\t</send> |", org->name, org->name), ch);
                count++;if(!(count%3)) send_to_char("\n\r", ch);
            }
        }
        if(ch->home > 0)
        {
            send_to_char(Format("\t<send href='step %s'>%-20s\t</send> |", "home", "home"), ch);
            count++;if(!(count%3)) send_to_char("\n\r", ch);
        }
        send_to_char("\n\r", ch);
        return;
    }

    if(ch->in_room->vnum == ROOM_VNUM_JAIL)
    {
        send_to_char("The jail attendant laughs at your request for a taxi.\n\r", ch);
        return;
    }

    if((i = flag_value(rp_area_table, argument)) == NO_FLAG)
    {
        if((org = org_lookup(argument)) == NULL || !get_room_index(i = org->step_point))
        {
            if(!str_prefix(argument, "home")
                    && (ch->home == 0 || !get_room_index(i = ch->rooms[0]->vnum)))
            {
                send_to_char("No such step point available.\n\r", ch);
                return;
            }
        }
    }

    if(i != flag_value(rp_area_table, "newbie school") && ch->dollars + (ch->cents / 100) < 5 && !IS_ADMIN(ch))
    {
        send_to_char("You don't have $5 to pay for the taxi!\n\r", ch);
        return;
    }

    if((location = get_room_index(i)) == NULL)
    {
        send_to_char("\tOThere seems to have been a mistake.\tn\n\r", ch);
        send_to_char("\tOPlease report that target location as being unavailable.\tn\n\r", ch);
        send_to_char(Format("\tOIssue with Room Vnum: %d\tn", i), ch);
        log_string(LOG_BUG, Format("\tOTried to step to Room Vnum: %d\tn", i));
        return;
    }

    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if (get_trust(rch) >= ch->invis_level)
        {
            if (ch->pcdata != NULL && !IS_NULLSTR(ch->pcdata->bamfout) )
                act("$t",ch,ch->pcdata->bamfout,rch,TO_VICT,0);
            else
                act("$n leaves, catching a taxi to another area.",ch, NULL,rch,TO_VICT,0);
        }
    }

    char_from_room( ch );
    char_to_room( ch, location );

    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if (get_trust(rch) >= ch->invis_level)
        {
            if (ch->pcdata != NULL && !IS_NULLSTR(ch->pcdata->bamfin) )
                act("$t",ch,ch->pcdata->bamfin,rch,TO_VICT,0);
            else
                act("$n has arrived.",ch,NULL,rch,TO_VICT,0);
        }
    }

    do_function( ch, &do_look, "auto" );
    if(!IS_ADMIN(ch) && i != flag_value(rp_area_table, "school"))
    {
        if(ch->dollars >= 5)
            ch->dollars -= 5;
        else {
            ch->cents -= (5 - ch->dollars) * 100;
            ch->dollars = 0;
        }
        send_to_char("You pay the taxi driver the $5 you owe him for the trip.\n\r", ch);
    }
}

void do_taxi(CHAR_DATA *ch, char *argument)
{
    int i;
    int count = 0;
    CHAR_DATA *rch;
    ROOM_INDEX_DATA *location;
    ORG_DATA *org;

    CheckCH(ch);

    /*
     * Added org stepping. Dsarky.
     * Added home stepping. Dsarky.
     */

    /* Basically, a modification of the do_step command.  Pointed it to the taxi area table.
     * Rayal
     */

    /* Added a better way of displaying the steps by adding the count feature
     * so that it would display much like the abilities and powers do.  Also
     * looks a hell of a lot better.  Rayal.
     */
    if(IS_NULLSTR(argument))
    {
        send_to_char("\tGA taxi will take you:\tn\n\r", ch);
        for(i=0;taxi_area_table[i].name != NULL;i++)
        {
            if(taxi_area_table[i].settable == TRUE) {
                send_to_char(Format("\t<send href='taxi %s'>%-20s\t</send> |", taxi_area_table[i].name, taxi_area_table[i].name), ch);
                count++;if(!(count%3)) send_to_char("\n\r", ch);
            }
        }
        for(org = org_list; org; org = org->next)
        {
            if(mem_lookup(org, ch->name)) {
                send_to_char(Format("\t<send href='taxi %s'>%-20s\t</send> |", org->name, org->name), ch);
                count++;if(!(count%3)) send_to_char("\n\r", ch);
            }
        }
        if(ch->home > 0)
        {
            send_to_char(Format("\t<send href='taxi %s'>%-20s\t</send> |", "home", "home"), ch);
            count++;if(!(count%3)) send_to_char("\n\r", ch);
        }
        send_to_char("\n\r", ch);
        return;
    }

    if(ch->in_room->vnum == ROOM_VNUM_JAIL)
    {
        send_to_char("The jail attendant laughs at your request for a taxi.\n\r", ch);
        return;
    }

    if ( !IS_OUTSIDE(ch) )
    {
        send_to_char( "You can't call a taxi unless you are outside.\n\r", ch );
        return;
    }

    if((i = flag_value(taxi_area_table, argument)) == NO_FLAG)
    {
        if((org = org_lookup(argument)) == NULL
                || !get_room_index(i = org->step_point))
        {
            if(!str_prefix(argument, "home")
                    && (ch->home == 0 || !get_room_index(i = ch->rooms[0]->vnum)))
            {
                send_to_char("No such step point available.\n\r", ch);
                return;
            }
        }
    }

    if(i != flag_value(taxi_area_table, "school") && ch->dollars + (ch->cents / 100) < 5 && !IS_ADMIN(ch))
    {
        send_to_char("You don't have $5 to pay for the taxi!\n\r", ch);
        return;
    }

    if((location = get_room_index(i)) == NULL)
    {
        send_to_char("There seems to have been a mistake.\n\r", ch);
        send_to_char("Please report that target location as being unavailable.\n\r", ch);
        log_string(LOG_BUG, Format("\tOTried to step to Room Vnum: %d\tn", i));
        return;
    }

    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if (get_trust(rch) >= ch->invis_level)
        {
            if (ch->pcdata != NULL && !IS_NULLSTR(ch->pcdata->bamfout) )
                act("$t",ch,ch->pcdata->bamfout,rch,TO_VICT,0);
            else
                act("$n leaves, catching a taxi to another area.",ch, NULL,rch,TO_VICT,0);
        }
    }

    char_from_room( ch );
    char_to_room( ch, location );

    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if (get_trust(rch) >= ch->invis_level)
        {
            if (ch->pcdata != NULL && !IS_NULLSTR(ch->pcdata->bamfin) )
                act("$t",ch,ch->pcdata->bamfin,rch,TO_VICT,0);
            else
                act("$n has arrived.",ch,NULL,rch,TO_VICT,0);
        }
    }

    do_function( ch, &do_look, "auto" );
    if(!IS_ADMIN(ch) && i != flag_value(taxi_area_table, "school"))
    {
        if(ch->dollars >= 5)
            ch->dollars -= 5;
        else
        {
            ch->cents -= (5 - ch->dollars) * 100;
            ch->dollars = 0;
        }
        send_to_char("You pay the taxi driver the $5 you owe him for the trip.\n\r", ch);
    }
}

void do_newbie (CHAR_DATA *ch, char *argument)
{
    CheckCH(ch);

    do_step(ch, "Newbie School");
    send_to_char("\n\rFrom here you can explore the features of Project Twilight.\n\r", ch);
    send_to_char("Take some time to get acquainted with the commands and\n\r", ch);
    send_to_char("peculiarities of our world.\n\r", ch);
}

void do_pump(CHAR_DATA *ch, char *argument)
{
    int fail = 0;
    int stat = 0;
    AFFECT_DATA af;

    CheckCH(ch);

    if((stat = stat_lookup(argument, ch)) == -1)
    {
        send_to_char("That isn't a valid stat.\n\r", ch);
        return;
    }

    if(stat > 2)
    {
        send_to_char("You can only pump physical stats.\n\r", ch);
        return;
    }

    if(ch->RBPG < 2)
    {
        send_to_char("You don't have enough blood!\n\r", ch);
        return;
    }

    switch(ch->gen)
    {
    case 1: case 2: case 3:
    case 4: case 5:
    case 6: fail = 10;
    break;
    case 9: fail = 7;
    break;
    case 8: fail = 8;
    break;
    case 7: fail = 9;
    break;
    default: fail = 6;
    break;
    }

    if(get_curr_stat(ch, stat) >= 2*ch->perm_stat[stat] || get_curr_stat(ch, stat) >= fail)
    {
        send_to_char("You cannot pump that any further.\n\r", ch);
        return;
    }

    fail = dice_rolls(ch, get_curr_stat(ch, STAT_STA), 8);

    ch->RBPG--;

    if(fail > 0)
    {
        act("Your $t increases.", ch, stat_table[stat].name, NULL, TO_CHAR, 1);
        /* create affect crap */

        switch(stat) {
        case 0: af.location  = APPLY_STR;
        break;
        case 1: af.location  = APPLY_DEX;
        break;
        case 2: af.location  = APPLY_STA;
        break;
        }

        af.where     = TO_AFFECTS;
        af.type      = skill_lookup("blood pump");
        af.level     = fail;
        af.modifier  = 1;
        af.duration  = 5*fail;
        af.bitvector = 0;
        affect_to_char( ch, &af );
    }
    else
    {
        send_to_char("You feel no effects.\n\r", ch);
    }
    WAIT_STATE(ch, 2);
}

void do_bandage(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *vch;

    CheckCH(ch);

    if(IS_NULLSTR(argument) || (vch = get_char_room(ch, argument)) == NULL)
    {
        vch = ch;
    }

    if(IS_SET(vch->off_flags, BANDAGED)
            || (vch->health >= 7 && vch->agghealth >= 7))
    {
        send_to_char("You can't bandage someone with no open wounds.\n\r", ch);
        return;
    }

    if(vch->health < 7)
    {
        vch->health++;
        SET_BIT(vch->off_flags, BANDAGED);
    }
    else if(vch->agghealth < 7)
    {
        vch->agghealth++;
        SET_BIT(vch->off_flags, BANDAGED);
    }

    if(ch!=vch)
    {
        act("You bandage $N's injuries.", ch, NULL, vch, TO_CHAR, 0);
        act("$n bandages your injuries.", ch, NULL, vch, TO_VICT, 0);
        act("$n bandages $N's injuries.", ch, NULL, vch, TO_NOTVICT, 0);
    }
    else
    {
        act("You bandage your injuries.", ch, NULL, NULL, TO_CHAR, 0);
        act("$n bandages $s injuries.", ch, NULL, NULL, TO_ROOM, 0);
    }
}

void do_fangs(CHAR_DATA *ch, char *argument)
{
    CheckCH(ch);

    if(IS_SET(ch->parts, PART_FANGS))
    {
        act("Your fangs shrink back into your gums.", ch, NULL, NULL,TO_CHAR,0);
        act("$n's fangs shrink back into $m gums.", ch, NULL, NULL, TO_ROOM,0);
        REMOVE_BIT(ch->parts, PART_FANGS);
    }
    else
    {
        act("Your fangs extend from your gums.", ch, NULL, NULL, TO_CHAR, 0);
        act("Fangs extend from $n's gums!", ch, NULL, NULL, TO_ROOM, 0);
        SET_BIT(ch->parts, PART_FANGS);
    }
}

void do_mark(CHAR_DATA *ch, char *argument)
{
    char arg[MIL]={'\0'};
    int door;
    EXTRA_DESCR_DATA *ed;

    CheckCH(ch);

    if(IS_NULLSTR(argument))
    {
        send_to_char("mark <arrow/graffiti> [<direction/string>]\n\r", ch);
        return;
    }

    argument = one_argument(argument, arg);

    if(!str_prefix(arg, "arrow")) {
        if(IS_NULLSTR(argument))
        {
            send_to_char("Which direction should the arrow point?\n\r", ch);
            return;
        }

        if((door = exit_lookup(ch, argument)) == -1)
        {
            send_to_char("No such direction.\n\r", ch);
            return;
        }

        ed = new_extra_descr();

        PURGE_DATA(ed->description);
        ed->description = str_dup(Format("An arrow pointing %s has been drawn here.", dir_name[door]));

        PURGE_DATA(ed->keyword);
        ed->keyword = str_dup(Format("arrow %s", dir_name[door]));

        ed->timer = 50;
        ed->next = ch->in_room->extra_descr;
        ch->in_room->extra_descr = ed;
        send_to_char(Format("You draw an arrow pointing %s.\n\r", dir_name[door]), ch);
        act("$n scrawls something.", ch, NULL, NULL, TO_ROOM, 0);
    }
    else if(!str_prefix(arg, "graffiti"))
    {
        if(IS_NULLSTR(argument))
        {
            send_to_char("What do you wish to scrawl?\n\r", ch);
            return;
        }

        ed = new_extra_descr();

        PURGE_DATA(ed->description);
        ed->description = str_dup(Format("Someone has scrawled '%s' here.", argument));

        PURGE_DATA(ed->keyword);
        ed->keyword = str_dup(Format("graffiti scrawled"));

        ed->timer = 50;
        ed->next = ch->in_room->extra_descr;
        ch->in_room->extra_descr = ed;
        send_to_char(Format("You scrawl '%s.'\n\r", argument), ch);
        act("$n scrawls something.", ch, NULL, NULL, TO_ROOM, 0);
    }
    else
    {
        send_to_char( "mark <arrow/graffiti> [<direction/string>]\n\r", ch);
    }
}

void do_claws(CHAR_DATA *ch, char *argument)
{
    CheckCH(ch);

    if(IS_SET(ch->parts, PART_CLAWS))
    {
        act("Your claws shrink away to nails.", ch, NULL, NULL, TO_CHAR, 0);
        act("$n's claws shrink away to nails.", ch, NULL, NULL, TO_ROOM, 0);
        REMOVE_BIT(ch->parts, PART_CLAWS);
        ch->dam_type = 0;
    }
    else
    {
        act("Your claws extend from your fingers.", ch, NULL, NULL, TO_CHAR, 0);
        act("Claws extend from $n's fingers!", ch, NULL, NULL, TO_ROOM, 0);
        SET_BIT(ch->parts, PART_CLAWS);
        ch->dam_type = 5;
    }
}

void do_goschool(CHAR_DATA *ch, char *argument)
{
    CheckCH(ch);

    do_step(ch, "school");
}

void do_auspice(CHAR_DATA *ch, char *argument)
{
    int i;
    int auspice;
    int diff;

    CheckCH(ch);

    if(ch->bg_timer <= 0)
    {
        send_to_char("Once you have set your initial backgrounds you cannot change your auspice.\n\r", ch);
        return;
    }

    if(IS_NULLSTR(argument) || (auspice = auspice_lookup(argument)) == -1)
    {
        send_to_char("What are you trying to change your auspice to?\n\r", ch);
        return;
    }

    /* Remove existing gifts. */
    for(i = 0; i < ch->disc[1]; i++)
    {
        REMOVE_BIT(ch->powers[i], group_gift_lookup(auspice_table[ch->auspice].name, i));
    }
    diff = ch->max_RBPG - auspice_table[ch->auspice].rage;

    /* Set stuff. */
    ch->auspice = auspice;
    for(i = 0; i < ch->disc[1]; i++)
    {
        SET_BIT(ch->powers[i], group_gift_lookup(auspice_table[ch->auspice].name, i));
    }
    ch->max_RBPG = auspice_table[ch->auspice].rage + diff;

    act("Your auspice changes to $t.", ch, auspice_table[ch->auspice].name, NULL, TO_CHAR, 0);
    send_to_char("Your rage and gifts have been reset to match.\n\r", ch);
}

void do_breed(CHAR_DATA *ch, char *argument)
{
    int i;
    int breed;
    int diff = 0;

    CheckCH(ch);

    if(ch->bg_timer <= 0)
    {
        send_to_char("Once you have set your initial backgrounds you cannot change your breed.\n\r", ch);
        return;
    }

    if(IS_NULLSTR(argument) || (breed = breed_lookup(argument)) == -1)
    {
        send_to_char("What are you trying to change your breed to?\n\r", ch);
        return;
    }

    /* Remove existing gifts. */
    for(i = 0; i < ch->disc[2]; i++)
    {
        REMOVE_BIT(ch->powers[i], group_gift_lookup(breed_table[ch->breed].name, i));
    }
    diff = ch->max_GHB - breed_table[ch->breed].gnosis;

    /* Set stuff. */
    ch->breed = breed;
    for(i = 0; i < ch->disc[1]; i++)
    {
        SET_BIT(ch->powers[i], group_gift_lookup(breed_table[ch->breed].name, i));
    }
    ch->max_GHB = breed_table[ch->breed].gnosis + diff;

    act("Your breed changes to $t.", ch, breed_table[ch->breed].name, NULL, TO_CHAR, 0);
    send_to_char("Your gnosis and gifts have been reset to match.\n\r", ch);
}


void jump_char( CHAR_DATA *ch, int door )
{
    CHAR_DATA *fch;
    CHAR_DATA *fch_next;
    ROOM_INDEX_DATA *in_room;
    ROOM_INDEX_DATA *to_room;
    ROOM_INDEX_DATA *jump_to = NULL;
    EXIT_DATA *pexit;
    int successes = 1;
    int diff = 5;
    int bonus = 0;
    int move = 0;

    CheckCH(ch);

    if(ch->race == race_lookup("vampire"))
    bonus = ch->disc[DISC_POTENCE];

    if ( door < 0 || door > 5 )
    {
        log_string(LOG_BUG, Format("Jump_char: bad door %d.", door));
        return;
    }

    in_room = ch->in_room;

    if ( ( pexit   = in_room->exit[door] ) == NULL
    ||   ( to_room = pexit->u1.to_room   ) == NULL
    ||   !can_see_room(ch,pexit->u1.to_room))
    {
        send_to_char( "Alas, you cannot go that way.\n\r", ch );
        return;
    }

    if (IS_SET(pexit->exit_info, EX_FALLING)
    && (jump_to = pexit->jumpto.to_room) == NULL)
    {
    ch->falldam++;
    }

    if(jump_to)
    to_room = jump_to;
    else jump_to = to_room;

    if (IS_SET(pexit->exit_info, EX_CLOSED))
    {
        act( "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR, 0 );
        return;
    }

    while(to_room->sector_type == SECT_AIR
    || to_room->sector_type == SECT_WATER_NOSWIM
    || to_room->sector_type == SECT_WATER_SWIM
    || to_room->sector_type == SECT_POLLUTED)
    {
    successes += 2;
    ch->jump_timer += 2;
    ch->falldam += 2;
    if(to_room->exit[door] != NULL)
        to_room = to_room->exit[door]->u1.to_room;
    else
        break;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM)
    &&   ch->master != NULL
    &&   in_room == ch->master->in_room )
    {
        send_to_char( "What?  And leave your beloved master?\n\r", ch );
        return;
    }

    if ( !is_room_owner(ch,to_room) && room_is_private( to_room ) )
    {
        send_to_char( "That room is private right now.\n\r", ch );
        return;
    }

    if ( !IS_NPC(ch) )
    {
        int iClan, iGuild;
        int move;

        for ( iClan = 0; iClan < MAX_CLAN; iClan++ )
        {
            for ( iGuild = 0; iGuild < MAX_CLAN; iGuild ++)
            {
           /*   if ( iClan != ch->clan
                &&   to_room->vnum == clan_table[iClan].guild[iGuild] )
                {
                    send_to_char( "You aren't allowed in there.\n\r", ch );
                    return;
                } */
            }
        }

        if (in_room->sector_type == SECT_WATER_NOSWIM)
        {
            OBJ_DATA *obj;
            bool found;

            /*
             * Look for a boat.
             */
            found = FALSE;

            if (IS_ADMIN(ch))
                found = TRUE;

            for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
            {
                if ( obj->item_type == ITEM_BOAT )
                {
                    found = TRUE;
            obj_from_char(obj);
            obj_to_room(obj, ch->in_room);
                    break;
                }
            }
            if ( !found )
            {
                send_to_char("Try again when you learn to walk on water.\n\r", ch );
                return;
            }
        else
        {
        send_to_char("You leap from your boat!\n\r", ch);
        diff += 3;
        }
        }

        move = dice_rolls(ch, (get_curr_stat(ch,STAT_DEX) + ch->ability[ATHLETICS].value), diff)
        + dice_rolls(ch, (get_curr_stat(ch,STAT_STR) + ch->ability[ATHLETICS].value + bonus), diff);

        /* conditional effects */
        if (IS_AFFECTED(ch,AFF_FLYING))
        {
            if(move > 0)
                move *= 20;
            else
                move = 100;
        }

        if (IS_AFFECTED(ch,AFF_SLOW))
            move /= 4;

        if(move == 0)
        {
            ch->falldam = 1;
            ch->jump_timer = 0;
        }
        else if(move > successes)
        {
            ch->falldam = 0;
            ch->jump_timer = successes;
        }
        else if(move == successes)
        {
            ch->falldam /= 2;
            ch->jump_timer = move;
        }
        else ch->jump_timer = move;

        ch->jump_dir = door;
    }

    if (IS_AFFECTED(ch, AFF_SNEAK)
    && (dice_rolls(ch, (get_curr_stat(ch,STAT_DEX) + ch->ability[STEALTH].value),9) <= 0))
        REMOVE_BIT(ch->affected_by,AFF_SNEAK);

    if ( !IS_AFFECTED(ch, AFF_SNEAK)
    &&   ch->invis_level < LEVEL_IMMORTAL)
    {
      if(!IS_AFFECTED(ch, AFF_INVISIBLE) || dice_rolls(ch,
                get_curr_stat(ch, STAT_DEX) + ch->ability[STEALTH].value,
                7) < 1) {
        if(!LOOKS_DIFFERENT(ch))
        act( "$n leaps $T.", ch, NULL, dir_name[door], TO_ROOM, 0 );
        else
        {
            act( Format("%s leaps %s.", ch->alt_name, dir_name[door]), ch, NULL, NULL, TO_ROOM, 0 );
        }
      }
    }

    if(IS_SET(ch->comm, COMM_JAM_LEAD))
    {
        REMOVE_BIT(ch->comm, COMM_JAM_LEAD);
        REMOVE_BIT(in_room->room_flags, ROOM_JAMMIN);
        act("$n's absence ends the jam.", ch, NULL, NULL, TO_ROOM, 0);
        send_to_char("Without you leading the jam, it ends.\n\r", ch);
    }
    else if(IS_SET(ch->comm, COMM_JAMMIN)
        && IS_SET(in_room->room_flags, ROOM_JAMMIN))
    {
        int move = 0;
        REMOVE_BIT(ch->comm, COMM_JAMMIN);
        for(fch = in_room->people; fch != NULL; fch = fch->next_in_room)
        {
            if(!IS_SET(fch->comm, COMM_JAMMIN)
            && !IS_SET(fch->comm, COMM_JAM_LEAD))
                continue;
            else move = 1;
        }

        if(!move) REMOVE_BIT(in_room->room_flags, ROOM_JAMMIN);
    }

    char_from_room( ch );

    if(move > 0) {
    char_to_room( ch, jump_to );
    }
    else
    {
    char_to_room( ch, to_room );
    if(to_room->sector_type != SECT_AIR)
    {
      if(to_room->sector_type == SECT_WATER_SWIM)
        damage(ch, ch, ch->falldam/2, gsn_bash, DAM_BASH, TRUE, 0, -1);
      else if(to_room->sector_type == SECT_WATER_NOSWIM)
        damage(ch, ch, ch->falldam/8, gsn_bash, DAM_BASH, TRUE, 0, -1);
      else
        damage(ch, ch, ch->falldam, gsn_bash, DAM_BASH, TRUE, 0, -1);
    }
    }

    if ( !IS_AFFECTED(ch, AFF_SNEAK)
    &&   ch->invis_level < LEVEL_IMMORTAL)
    {
      if(!IS_AFFECTED(ch, AFF_INVISIBLE) || dice_rolls(ch,
                get_curr_stat(ch, STAT_DEX) + ch->ability[STEALTH].value,
                9) < 1) {
       if(move > 0)
       {
        if(!LOOKS_DIFFERENT(ch))
        act( "$n leaps in from the $t.", ch, dir_name[rev_dir[door]], NULL,
        TO_ROOM, 0 );
        else
        {
            act( Format("%s leaps in from the $t.", ch->alt_name), ch, dir_name[rev_dir[door]], NULL, TO_ROOM, 0 );
        }
       }
       else
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
      }
    }

    do_function( ch, &do_look, "auto" );

    if (in_room == to_room) /* no circular follows */
        return;

    for ( fch = in_room->people; fch != NULL; fch = fch_next )
    {
        fch_next = fch->next_in_room;

        if ( fch->master == ch && IS_AFFECTED(fch,AFF_CHARM)
        &&   fch->position < P_STAND)
        do_function( fch, &do_stand, "" );

        if ( fch->master == ch && fch->position == P_STAND
        &&   can_see_room(fch,to_room))
        {
            act( "You go to follow $N, but $E leaps $t.", fch, dir_name[door], ch, TO_CHAR, 0 );
        }
    }

    return;
}

void do_jump(CHAR_DATA *ch, char *argument)
{
    int door;

    CheckCH(ch);

    if(IS_NULLSTR(argument))
    {
        send_to_char("Where are you trying to jump to?\n\r", ch);
        return;
    }

    if(IS_SET(ch->form, FORM_BLOOD))
    {
        send_to_char("People jump puddles, puddles don't jump people.\n\r", ch);
        return;
    }

    if ( ( door = find_door( ch, argument ) ) >= 0 || door <= 5 )
    {
        jump_char( ch, door );
    }
}

void do_traits(CHAR_DATA *ch, char *argument)
{
    char arg[MIL]={'\0'};
    int i, type = -1, sel, subfl;
    TRAIT_DATA *trait;
    AFFECT_DATA *paf;

    CheckCH(ch);

    argument = one_argument(argument, arg);
    if(IS_NULLSTR(argument) || IS_NULLSTR(arg))
    {
        send_to_char("Syntax: trait <merit/flaw/quirk/derangement> <selection> [<sub-flag>]\n\r",ch);
        send_to_char("        trait list [<merit/flaw/quirk/derangement/mania/compulsion/phobia/feedrestrict>] - lists available flags\n\r", ch);

        if(ch->traits != NULL)
        {
            send_to_char("Your current traits are:\n\r", ch);
            for(trait=ch->traits;trait!=NULL;trait=trait->next)
            {
                send_to_char(Format("%-15s", capitalize(trait_type[trait->type].name)), ch);
                send_to_char(Format("%-20s", capitalize(trait->qualifier)), ch);
                send_to_char(Format("%-20s", IS_NULLSTR(trait->detail)?"":capitalize(trait->detail)), ch);
                send_to_char(Format(" %d\n\r", trait->value), ch);
            }
            for(paf=affect_find(ch->affected,skill_lookup("siren's beckoning"));
                    paf != NULL;
                    paf=affect_find(paf->next,skill_lookup("siren's beckoning")))
            {
                send_to_char(Format("%-15s%-20s", "Derangements", capitalize(derangement_table[paf->bitvector].name)), ch);
                if(paf->bitvector <= 2)
                {
                    send_to_char(Format("%-20s (Temporary)\n\r", capitalize(mania_table[paf->modifier].name)), ch);
                }
                else if(paf->bitvector <= 5)
                {
                    send_to_char(Format("%-20s (Temporary)\n\r", capitalize(phobia_table[paf->modifier].name)), ch);
                }
                else if(paf->bitvector == 6)
                {
                    send_to_char(Format("%-20s (Temporary)\n\r", capitalize(compulsion_table[paf->modifier].name)), ch);
                }
                else
                    send_to_char(" (Temporary)\n\r", ch);
            }
        }
        else
            send_to_char("You have no traits set.\n\r", ch);

        return;
    }

    if(!str_prefix(arg, "list"))
    {
        type = flag_lookup(argument, trait_type);

        switch(type)
        {
        case 0:
            send_to_char("Available quirk types are:\n\r", ch);
            for(i = 0; quirk_type[i].name != NULL; i++)
            {
                if(i%4==0) send_to_char("\n\r", ch);
                send_to_char(Format("%20s", quirk_type[i].name), ch);
            }
            send_to_char("\n\r", ch);
            break;

        case 1:
            send_to_char("Available merits are:\n\r", ch);
            for(i = 0; merit_table[i].name != NULL; i++)
            {
                if(i%4==0) send_to_char("\n\r", ch);
                send_to_char(Format("%20s", merit_table[i].name), ch);
            }
            send_to_char("\n\r", ch);
            break;

        case 2:
            send_to_char("Available flaws are:\n\r", ch);
            for(i = 0; flaw_table[i].name != NULL; i++)
            {
                if(i%4==0) send_to_char("\n\r", ch);
                send_to_char(Format("%20s", flaw_table[i].name), ch);
            }
            send_to_char("\n\r", ch);
            break;

        case 3:
            send_to_char("Available derangements are:\n\r", ch);
            for(i = 0; derangement_table[i].name != NULL; i++)
            {
                if(i%4==0) send_to_char("\n\r", ch);
                send_to_char(Format("%20s", derangement_table[i].name), ch);
            }
            send_to_char("\n\r", ch);
            break;

        default:
            if(!str_prefix(argument, "manias"))
            {
                send_to_char("Available manias are:\n\r", ch);
                for(i = 0; mania_table[i].name != NULL; i++)
                {
                    if(i%4==0) send_to_char("\n\r", ch);
                    send_to_char(Format("%20s", mania_table[i].name), ch);
                }
                send_to_char("\n\r", ch);
                return;
            }
            if(!str_prefix(argument, "compulsions")
                    || !str_prefix(argument, "obsessions"))
            {
                send_to_char("Available compulsions are:\n\r", ch);
                for(i = 0; compulsion_table[i].name != NULL; i++)
                {
                    if(i%4==0) send_to_char("\n\r", ch);
                    send_to_char(Format("%20s", compulsion_table[i].name), ch);
                }
                send_to_char("\n\r", ch);
                return;
            }
            if(!str_prefix(argument, "phobias"))
            {
                send_to_char("Available phobias are:\n\r", ch);
                for(i = 0; phobia_table[i].name != NULL; i++)
                {
                    if(i%4==0) send_to_char("\n\r", ch);
                    send_to_char(Format("%20s", phobia_table[i].name), ch);
                }
                send_to_char("\n\r", ch);
                return;
            }
            if(!str_prefix(argument, "feedrestrictions"))
            {
                send_to_char("Available feeding restrictions are:\n\r", ch);
                for(i = 0; feeding_restriction[i].name != NULL; i++)
                {
                    if(i%4==0) send_to_char("\n\r", ch);
                    send_to_char(Format("%20s", feeding_restriction[i].name), ch);
                }
                send_to_char("\n\r", ch);
                return;
            }
            send_to_char("Available trait types are:\n\r", ch);
            for(i = 0; trait_type[i].name != NULL; i++)
            {
                send_to_char(Format("%20s", trait_type[i].name), ch);
            }
            send_to_char("\n\r", ch);
            break;
        }
        return;
    }

    if((type = flag_lookup(arg, trait_type)) == -1)
    {
        send_to_char("No such trait.\n\r", ch);
        do_traits(ch, "");
        return;
    }

    argument = one_argument(argument, arg);

    /* @@@@@ */
    switch(type)
    {
    case 0:
        if((sel = flag_lookup(arg, quirk_type)) < 0)
        {
            send_to_char("That isn't a quirk type.\n\r", ch);
            do_traits(ch, "list quirks");
            return;
        }

        if(IS_NULLSTR(argument))
        {
            send_to_char("You must provide detail of the quirk.\n\r", ch);
            send_to_char( "(e.g. trait quirk like apple - no plurals.\n\r", ch);
            return;
        }

        trait = new_trait();
        trait->type = type;
        PURGE_DATA(trait->qualifier);
        PURGE_DATA(trait->detail);
        trait->qualifier = str_dup(quirk_type[sel].name);
        trait->detail = str_dup(argument);
        break;
    case 1:
        if((sel = trait_lookup(arg, merit_table)) < 0)
        {
            send_to_char("That isn't an available merit.\n\r", ch);
            do_traits(ch, "list merits");
            return;
        }

        trait = new_trait();
        trait->type = type;
        trait->value = merit_table[sel].cost;
        PURGE_DATA(trait->qualifier);
        trait->qualifier = str_dup(merit_table[sel].name);
        break;
    case 2:
        if((sel = trait_lookup(arg, flaw_table)) < 0)
        {
            send_to_char("That isn't an available flaw.\n\r", ch);
            do_traits(ch, "list flaws");
            return;
        }

        if(IS_NULLSTR(argument) && sel == trait_lookup("feedrestriction", flaw_table))
        {
            send_to_char("You must provide detail of the flaw.\n\r", ch);
            return;
        }

        trait = new_trait();
        trait->type = type;
        trait->value = flaw_table[sel].cost;
        if(ch->clan == clan_lookup("ventrue") && sel == trait_lookup("feedrestriction", flaw_table))
            trait->value = 0;
        PURGE_DATA(trait->qualifier);
        trait->qualifier = str_dup(flaw_table[sel].name);
        if(sel == trait_lookup("feedrestriction", flaw_table))
        {
            PURGE_DATA(trait->detail);
            trait->detail = str_dup(argument);
        }
        break;
    case 3:
        if((sel = trait_lookup(arg, derangement_table)) < 0)
        {
            send_to_char("That isn't an acceptable derangement.\n\r", ch);
            do_traits(ch, "list quirks");
            return;
        }

        if(IS_NULLSTR(argument) && sel>(int)trait_lookup("obsessive/compulsive",derangement_table))
        {
            send_to_char("You must provide detail of the derangement.\n\r", ch);
            do_traits(ch, "list derangements");
            return;
        }

        if(sel == trait_lookup("mania1", derangement_table)
                && sel == trait_lookup("mania2", derangement_table)
                && sel == trait_lookup("mania3", derangement_table)
                && (subfl = flag_lookup(argument, mania_table)) == -1)
        {
            send_to_char("I'm freaking out! That's not a mania!\n\r", ch);
            do_traits(ch, "list manias");
            return;
        }

        if(sel == trait_lookup("phobia1", derangement_table)
                && sel == trait_lookup("phobia2", derangement_table)
                && sel == trait_lookup("phobia3", derangement_table)
                && (subfl = flag_lookup(argument, phobia_table)) == -1)
        {
            send_to_char( "Don't scare me like that! That's not a phobia!\n\r", ch);
            do_traits(ch, "list phobias");
            return;
        }

        if(sel == trait_lookup("obsessive/compulsive", derangement_table)
                && (subfl = flag_lookup(argument, compulsion_table)) == -1)
        {
            send_to_char( "Don't scare me like that! That's not a phobia!\n\r", ch);
            do_traits(ch, "list phobias");
            return;
        }

        trait = new_trait();
        trait->type = type;
        trait->value = derangement_table[sel].cost;
        if(ch->clan == clan_lookup("malkavian"))
            trait->value = 0;
        PURGE_DATA(trait->qualifier);
        trait->qualifier = str_dup(derangement_table[sel].name);
        if(sel<(int)trait_lookup("obsessive/compulsive",derangement_table))
        {
            PURGE_DATA(trait->detail);
            trait->detail = str_dup(argument);
        }
        break;
    default:
        send_to_char("No such trait type.\n\r", ch);
        return;
        break;
    }

    if(trait != NULL)
    {
        if(trait->value > 0)
        {
            if(trait->value * 5 > ch->exp)
            {
                send_to_char( "You do not have enough experience to buy that trait.\n\r", ch);
                free_trait(trait);
                return;
            }

            if(trait_count(ch, 1) >= ch->max_traits[1])
            {
                send_to_char( "You've already bought all the positive traits you can.\n\r", ch);
                free_trait(trait);
                return;
            }
        }
        else if(trait->value < 0)
        {
            if(trait_count(ch, -1) >= ch->max_traits[0])
            {
                send_to_char( "You've already bought all the positive traits you can.\n\r", ch);
                free_trait(trait);
                return;
            }
        }

        if(has_trait(ch, trait))
        {
            send_to_char("You already have that trait.\n\r", ch);
            free_trait(trait);
            return;
        }

        ch->exp -= trait->value * 5;
        trait->next = ch->traits;
        ch->traits = trait;
        send_to_char("The trait has been added.\n\r", ch);
    }
}

void do_meditate(CHAR_DATA *ch, char *argument)
{
    int fail = 0;
    AFFECT_DATA af;

    CheckCH(ch);

    if(ch->power_timer > 0)
    {
        send_to_char("You've put strain on your powers, wait a bit...\n\r", ch);
        return;
    }

    act("$n enters a meditative state.", ch, NULL, NULL, TO_ROOM, 0);
    if((fail = dice_rolls(ch, ch->max_GHB, 6)) > 0)
    {
        send_to_char( "You feel more in tune with the influence of Gaia and Luna.\n\r", ch);
        ch->GHB = UMIN(ch->max_GHB, ch->GHB + fail);
        ch->power_timer = 2;
        return;
    }
    else if(fail == 0)
    {
        send_to_char("You don't feel anything change.\n\r", ch);
        ch->power_timer = 1;
        return;
    }
    else
    {
        send_to_char("Luna and Gaia seem to turn their faces from you!\n\r", ch);
        ch->power_timer = fail * -1;
        act("$n collapses in a heap!", ch, NULL, NULL, TO_CHAR, 0);
        send_to_char("You collapse from the strain.\n\r", ch);

        af.where     = TO_AFFECTS;
        af.type      = skill_lookup("sleep");
        af.level     = fail * -1;
        af.duration  = 4 + (-1 * fail);
        af.location  = APPLY_NONE;
        af.modifier  = 0;
        af.bitvector = AFF_SLEEP;
        affect_join( ch, &af );

        ch->position = P_SLEEP;

        return;
    }
}

void do_ooctogift( CHAR_DATA *ch, char *argument)
{
    int i = 0;

    CheckCH(ch);

    if( !is_number(argument) )
    {
        send_to_char("How much gift experience are you trying to buy?\n\r", ch);
        return;
    }

    i = atoi(argument);

    if(i * 3 > ch->oocxp)
    {
        send_to_char("You don't have enough OOC xp to buy that much gift xp.\n\r", ch);
        return;
    }

    ch->xpgift += i;
    ch->oocxp -= i * 3;

    send_to_char("You convert some OOC xp into gift xp.\n\r", ch);
}



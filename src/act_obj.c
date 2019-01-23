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
#include <stdlib.h>
#include "twilight.h"
#include "tables.h"
#include "lookup.h"
#include "interp.h"

bool trade_stocks(CHAR_DATA *ch, STOCKS *st, int num, bool fees, bool buy);

/*
 * Local functions.
 */
#define CD CHAR_DATA
#define OD OBJ_DATA
bool    remove_obj  args( (CHAR_DATA *ch, int iWear, bool fReplace ) );
int get_cost    args( (CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy ) );
void    obj_to_keeper   args( (OBJ_DATA *obj, CHAR_DATA *ch ) );
OD *    get_obj_keeper  args( (CHAR_DATA *ch,CHAR_DATA *keeper,char *argument));

#undef OD
#undef  CD

/* RT part of the corpse looting code */

void get_obj( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container )
{
    /* variables for AUTOSPLIT */
    CHAR_DATA *gch;

    if ( obj->item_type == ITEM_LIQUID || material_table[material_lookup(obj->material)].is_liquid )
    {
        send_to_char( "It drips through your fingers.\n\r", ch);
        /* Add liquid affects */
        return;
    }

    if ( !CAN_WEAR(obj, ITEM_TAKE) )
    {
        send_to_char( "You can't take that.\n\r", ch );
        return;
    }

    if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
        act( "$d: you can't carry that many items.", ch, NULL, obj->name, TO_CHAR, 0 );
        return;
    }

    if ((!obj->in_obj || obj->in_obj->carried_by != ch)
            &&  (get_carry_weight(ch) + get_obj_weight(obj) > can_carry_w(ch)))
    {
        act( "$d: you can't carry that much weight.", ch, NULL, obj->name, TO_CHAR, 0 );
        return;
    }

    if (obj->in_room != NULL && (gch = sat_on(obj)) != NULL)
    {
        act("$N appears to be using $p.", ch,obj,gch,TO_CHAR, 0);
        return;
    }

    if(IS_SET(obj->extra_flags, ITEM_HIDDEN))
    {
        REMOVE_BIT(obj->extra_flags, ITEM_HIDDEN);
    }

    if((ch->race == race_lookup("werewolf")
            && IS_SET(obj->extra_flags, ITEM_ANTI_WEREWOLF))
            || (ch->race == race_lookup("human")
                    && IS_SET(obj->extra_flags, ITEM_ANTI_HUMAN))
                    || (ch->race == race_lookup("vampire")
                            && IS_SET(obj->extra_flags, ITEM_ANTI_VAMPIRE))
                            || (ch->race == race_lookup("faerie")
                                    && IS_SET(obj->extra_flags, ITEM_ANTI_CHANGELING)))
    {
        send_to_char("It burns your hand!\n\r", ch);
        gain_condition(ch, COND_PAIN, 10);
        return;
    }

    if ( container != NULL )
    {
        if (!CAN_WEAR(container, ITEM_TAKE) &&  !IS_OBJ_STAT(obj,ITEM_HAD_TIMER))
            obj->timer = 0;
        act( "You get $p from $P.", ch, obj, container, TO_CHAR, 0 );
        act( "$n gets $p from $P.", ch, obj, container, TO_ROOM, 0 );
        REMOVE_BIT(obj->extra_flags,ITEM_HAD_TIMER);
        obj_from_obj( obj );
    }
    else
    {
        act( "You get $p.", ch, obj, container, TO_CHAR, 0 );
        act( "$n gets $p.", ch, obj, container, TO_ROOM, 0 );
        obj_from_room( obj );
    }

    if ( obj->item_type == ITEM_MONEY)
    {
        ch->cents += obj->value[0];
        ch->dollars += obj->value[1];

        extract_obj( obj );
    }
    else
    {
        obj_to_char( obj, ch );
    }

    return;
}



void do_get( CHAR_DATA *ch, char *argument )
{
    char arg1[MIL]={'\0'};
    char arg2[MIL]={'\0'};
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    OBJ_DATA *container;
    bool found;

    CheckCH(ch);

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (!str_cmp(arg2, "from"))
        argument = one_argument(argument,arg2);

    /* Get type. */
    if ( IS_NULLSTR(arg1) )
    {
        send_to_char( "Get what?\n\r", ch );
        return;
    }

    if ( IS_NULLSTR(arg2) )
    {
    if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
    {
        /* 'get obj' */
        obj = get_obj_list( ch, arg1, ch->in_room->contents );
        if ( obj == NULL )
        {
        act( "I see no $T here.", ch, NULL, arg1, TO_CHAR, 0 );
        return;
        }

        if(IS_SET(obj->extra2, OBJ_FURNISHING)
        || obj->wear_loc == WEAR_HOME)
        {
            act( "You can't take $p, it's part of the furniture.", ch, obj, NULL, TO_CHAR, 0 );
            return;
        }

        if(IS_SET(obj->extra2, OBJ_INTANGIBLE))
        {
            act( "Your hand passes right through $p.", ch, obj, NULL, TO_CHAR, 0 );
            act( "$n's hand passes right through $p as $e tries to pick it up.", ch, obj, NULL, TO_ROOM, 0 );
            return;
        }

        get_obj( ch, obj, NULL );
    }
    else
    {
        /* 'get all' or 'get all.obj' */
        found = FALSE;
        for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
        {
        obj_next = obj->next_content;
        if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
        &&   (can_see_obj( ch, obj ) || IS_SET(obj->extra_flags, ITEM_HIDDEN)) )
        {
            found = TRUE;
            if(!IS_SET(obj->extra2, OBJ_FURNISHING)
            &&   obj->wear_loc != WEAR_HOME
            &&   !IS_SET(obj->extra2, OBJ_INTANGIBLE))
            get_obj( ch, obj, NULL );
        }
        }

        if ( !found ) 
        {
        if ( arg1[3] == '\0' )
            send_to_char( "I see nothing here.\n\r", ch );
        else
            act( "I see no $T here.", ch, NULL, &arg1[4], TO_CHAR, 0 );
        }
    }
    }
    else
    {
    /* 'get ... container' */
    if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
    {
        send_to_char( "You can't do that.\n\r", ch );
        return;
    }

    if ( ( container = get_obj_here( ch, arg2 ) ) == NULL )
    {
        act( "I see no $T here.", ch, NULL, arg2, TO_CHAR, 0 );
        return;
    }

    switch ( container->item_type )
    {
    default:
        send_to_char( "That's not a container.\n\r", ch );
        return;

    case ITEM_CONTAINER:
    case ITEM_CORPSE_NPC:
    case ITEM_CORPSE_PC:
    case ITEM_FURNITURE:
        break;
    }

    if ( IS_SET(container->value[1], CONT_CLOSED) )
    {
        act( "The $d is closed.", ch, NULL, container->name, TO_CHAR, 0 );
        return;
    }

    if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
    {
        /* 'get obj container' */
        obj = get_obj_list( ch, arg1, container->contains );
        if ( obj == NULL )
        {
        act( "I see nothing like that in the $T.", ch, NULL, arg2, TO_CHAR, 0 );
        return;
        }
        get_obj( ch, obj, container );
    }
    else
    {
        /* 'get all container' or 'get all.obj container' */
        found = FALSE;
        for ( obj = container->contains; obj != NULL; obj = obj_next )
        {
        obj_next = obj->next_content;
        if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
        &&   can_see_obj( ch, obj ) )
        {
            found = TRUE;
            get_obj( ch, obj, container );
        }
        }

        if ( !found )
        {
            if ( arg1[3] == '\0' )
                act( "I see nothing in the $T.", ch, NULL, arg2, TO_CHAR, 0 );
            else
                act( "I see nothing like that in the $T.", ch, NULL, arg2, TO_CHAR, 0 );
        }
    }
    }

    return;
}


void do_put( CHAR_DATA *ch, char *argument )
{
    char arg1[MIL]={'\0'};
    char arg2[MIL]={'\0'};
    OBJ_DATA *container;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    CheckCH(ch);

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (!str_cmp(arg2,"in") || !str_cmp(arg2,"on"))
    argument = one_argument(argument,arg2);

    if ( IS_NULLSTR(arg1) || IS_NULLSTR(arg2) )
    {
        send_to_char( "Put what in what?\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
    {
        send_to_char( "You can't do that.\n\r", ch );
        return;
    }

    if ( ( container = get_obj_here( ch, arg2 ) ) == NULL )
    {
        if((is_number(arg1) && (container = get_obj_here(ch, argument)) == NULL) || !is_number(arg1))
        {
            act( "I see no $T here.", ch, NULL, arg2, TO_CHAR, 0 );
            return;
        }
    }

    if ( container->item_type != ITEM_CONTAINER
            && container->item_type != ITEM_FURNITURE )
    {
        send_to_char( "That's not a container.\n\r", ch );
        return;
    }

    if ( IS_SET(container->value[1], CONT_CLOSED)
            && container->item_type != ITEM_FURNITURE )
    {
        act( "The $d is closed.", ch, NULL, container->name, TO_CHAR, 0 );
        return;
    }

    if ( is_number( arg1 ) )
    {
        /* 'put NNNN dollars/cents obj' */
        int amount, dollars = 0, cents = 0;

        amount   = atoi(arg1);
        if ( amount <= 0
                || ( str_cmp( arg2, "coins" ) && str_cmp( arg2, "coin" ) &&
                        str_cmp( arg2, "dollars"  ) && str_cmp( arg2, "cents") ) )
        {
            send_to_char( "Sorry, you can't do that.\n\r", ch );
            return;
        }

    if ( !str_cmp( arg2, "coins") || !str_cmp(arg2,"coin") 
    ||   !str_cmp( arg2, "cents"))
    {
        if (ch->cents < amount)
        {
        send_to_char("You don't have that much change.\n\r",ch);
        return;
        }

        ch->cents -= amount;
        cents = amount;
    }

    else
    {
        if (ch->dollars < amount)
        {
        send_to_char("You don't have that many dollars.\n\r",ch);
        return;
        }

        ch->dollars -= amount;
        dollars = amount;
    }

    for ( obj = container->contains; obj != NULL; obj = obj_next )
    {
        obj_next = obj->next_content;

        switch ( obj->pIndexData->vnum )
        {
        case OBJ_VNUM_CENT_ONE:
        cents += 1;
        extract_obj(obj);
        break;

        case OBJ_VNUM_DOLLAR_ONE:
        dollars += 1;
        cents += 0;
        extract_obj( obj );
        break;

        case OBJ_VNUM_CENTS_SOME:
        cents += obj->value[0];
        extract_obj(obj);
        break;

        case OBJ_VNUM_DOLLARS_SOME:
        dollars += obj->value[1];
        extract_obj( obj );
        break;

        case OBJ_VNUM_COINS:
        cents += obj->value[0];
        dollars += obj->value[1];
        extract_obj(obj);
        break;
        }
    }

    obj_to_obj( create_money( dollars, cents ), container );
    act( "$n puts some cash in $p.", ch, container, NULL, TO_ROOM, 0 );
    send_to_char( "OK.\n\r", ch );
    return;
    }

    if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
    {
    /* 'put obj container' */
    if ( ( obj = get_obj_carry( ch, arg1, ch ) ) == NULL )
    {
        send_to_char( "You do not have that item.\n\r", ch );
        return;
    }

    if ( obj == container )
    {
        send_to_char( "You can't fold it into itself.\n\r", ch );
        return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
        send_to_char( "You can't let go of it.\n\r", ch );
        return;
    }

        if (WEIGHT_MULT(obj) != 100)
        {
           send_to_char("You have a feeling that would be a bad idea.\n\r",ch);
            return;
        }

    if ((((get_obj_weight( obj ) + get_true_weight( container )
         > (container->value[0] * 10))
         && (container->item_type == ITEM_CONTAINER))
             || ((get_obj_weight( obj ) + get_true_weight( container )
         > (container->value[1] * 10))
         && (container->item_type == ITEM_FURNITURE)))
        &&   ((get_obj_weight(obj) > (container->value[3] * 10)
         && container->item_type == ITEM_CONTAINER)
         || (get_obj_weight(obj) > (container->value[4] * 10)
         && container->item_type == ITEM_CONTAINER)))
    {
        send_to_char( "It won't fit.\n\r", ch );
        return;
    }
    
    if (!CAN_WEAR(container,ITEM_TAKE))
    {
        if (obj->timer)
        SET_BIT(obj->extra_flags,ITEM_HAD_TIMER);
        else
            obj->timer = number_range(100,200);
    }

    obj_from_char( obj );
    obj_to_obj( obj, container );

    if (IS_SET(container->value[1],CONT_PUT_ON)
        || container->item_type == ITEM_FURNITURE)
    {
        act("$n puts $p on $P.",ch,obj,container, TO_ROOM, 0);
        act("You put $p on $P.",ch,obj,container, TO_CHAR, 0);
    }
    else
    {
        act( "$n puts $p in $P.", ch, obj, container, TO_ROOM, 0 );
        act( "You put $p in $P.", ch, obj, container, TO_CHAR, 0 );
    }
    }
    else
    {
    /* 'put all container' or 'put all.obj container' */
    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
        obj_next = obj->next_content;

        if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
        &&   can_see_obj( ch, obj )
        &&   WEIGHT_MULT(obj) == 100
        &&   obj->wear_loc == WEAR_NONE
        &&   obj != container
        &&   can_drop_obj( ch, obj )
        &&   (((get_obj_weight( obj ) + get_true_weight( container )
         <= (container->value[0] * 10))
         && (container->item_type == ITEM_CONTAINER))
             || ((get_obj_weight( obj ) + get_true_weight( container )
         <= (container->value[1] * 10))
         && (container->item_type == ITEM_FURNITURE)))
        &&   ((get_obj_weight(obj) < (container->value[3] * 10)
         && container->item_type == ITEM_CONTAINER)
         || (get_obj_weight(obj) < (container->value[4] * 10)
         && container->item_type == ITEM_CONTAINER)))
        {
            if (!CAN_WEAR(obj, ITEM_TAKE) )
            {
            if (obj->timer)
            SET_BIT(obj->extra_flags,ITEM_HAD_TIMER);
                else
                    obj->timer = number_range(100,200);
        }
        obj_from_char( obj );
        obj_to_obj( obj, container );

            if (IS_SET(container->value[1],CONT_PUT_ON)
            || container->item_type == ITEM_FURNITURE)
            {
                    act("$n puts $p on $P.",ch,obj,container, TO_ROOM, 0);
                    act("You put $p on $P.",ch,obj,container, TO_CHAR, 0);
            }
        else
        {
            act( "$n puts $p in $P.", ch, obj, container, TO_ROOM, 0 );
            act( "You put $p in $P.", ch, obj, container, TO_CHAR, 0 );
        }
        }
    }
    }

    return;
}



void do_drop( CHAR_DATA *ch, char *argument )
{
    char arg[MIL]={'\0'};
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    bool found;

    CheckCH(ch);

    argument = one_argument( argument, arg );

    if ( IS_NULLSTR(arg) )
    {
        send_to_char( "Drop what?\n\r", ch );
        return;
    }

    if ( is_number( arg ) )
    {
    /* 'drop NNNN coins' */
    int amount, dollars = 0, cents = 0;

    amount   = atoi(arg);
    argument = one_argument( argument, arg );
    if ( amount <= 0
    || ( str_cmp( arg, "coins" ) && str_cmp( arg, "coin" ) && 
         str_cmp( arg, "dollars"  ) && str_cmp( arg, "cents") ) )
    {
        send_to_char( "Sorry, you can't do that.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "coins") || !str_cmp(arg,"coin") 
    ||   !str_cmp( arg, "cents"))
    {
        if (ch->cents < amount)
        {
        send_to_char("You don't have that much change.\n\r",ch);
        return;
        }

        ch->cents -= amount;
        cents = amount;
    }

    else
    {
        if (ch->dollars < amount)
        {
        send_to_char("You don't have that many dollars.\n\r",ch);
        return;
        }

        ch->dollars -= amount;
        dollars = amount;
    }

    for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
    {
        obj_next = obj->next_content;

        switch ( obj->pIndexData->vnum )
        {
        case OBJ_VNUM_CENT_ONE:
        cents += 1;
        extract_obj(obj);
        break;

        case OBJ_VNUM_DOLLAR_ONE:
        dollars += 1;
        cents += 0;
        extract_obj( obj );
        break;

        case OBJ_VNUM_CENTS_SOME:
        cents += obj->value[0];
        extract_obj(obj);
        break;

        case OBJ_VNUM_DOLLARS_SOME:
        dollars += obj->value[1];
        extract_obj( obj );
        break;

        case OBJ_VNUM_COINS:
        cents += obj->value[0];
        dollars += obj->value[1];
        extract_obj(obj);
        break;
        }
    }

    obj_to_room( create_money( dollars, cents ), ch->in_room );
    act( "$n drops some cash.", ch, NULL, NULL, TO_ROOM, 0 );
    send_to_char( "OK.\n\r", ch );
    return;
    }

    if ( str_cmp( arg, "all" ) && str_prefix( "all.", arg ) )
    {
    /* 'drop obj' */
    if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
    {
        send_to_char( "You do not have that item.\n\r", ch );
        return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
        send_to_char( "You can't let go of it.\n\r", ch );
        return;
    }

    obj_from_char( obj );
    obj_to_room( obj, ch->in_room );
    act( "$n drops $p.", ch, obj, NULL, TO_ROOM, 0 );
    act( "You drop $p.", ch, obj, NULL, TO_CHAR, 0 );
    if (IS_OBJ_STAT(obj,ITEM_MELT_DROP))
    {
        act("$p dissolves into smoke.",ch,obj,NULL,TO_ROOM,0);
        act("$p dissolves into smoke.",ch,obj,NULL,TO_CHAR,0);
        extract_obj(obj);
    }
    }
    else
    {
    /* 'drop all' or 'drop all.obj' */
    found = FALSE;
    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
        obj_next = obj->next_content;

        if ( ( arg[3] == '\0' || is_name( &arg[4], obj->name ) )
        &&   can_see_obj( ch, obj )
        &&   obj->wear_loc == WEAR_NONE
        &&   can_drop_obj( ch, obj ) )
        {
        found = TRUE;
        obj_from_char( obj );
        obj_to_room( obj, ch->in_room );
        act( "$n drops $p.", ch, obj, NULL, TO_ROOM, 0 );
        act( "You drop $p.", ch, obj, NULL, TO_CHAR, 0 );
            if (IS_OBJ_STAT(obj,ITEM_MELT_DROP))
            {
                    act("$p dissolves into smoke.",ch,obj,NULL,TO_ROOM,0);
                    act("$p dissolves into smoke.",ch,obj,NULL,TO_CHAR,0);
                    extract_obj(obj);
            }
        }
    }

    if ( !found )
    {
        if ( arg[3] == '\0' )
        act( "You are not carrying anything.", ch, NULL, arg, TO_CHAR, 0 );
        else
        act( "You are not carrying any $T.", ch, NULL, &arg[4], TO_CHAR, 0 );
    }
    }

    return;
}


void do_give( CHAR_DATA *ch, char *argument )
{
    char arg1 [MIL]={'\0'};
    char arg2 [MIL]={'\0'};
    CHAR_DATA *victim;
    OBJ_DATA  *obj, *t_obj;
    EXTRA_DESCR_DATA *ed, *objed;

    CheckCH(ch);

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( IS_NULLSTR(arg1) || IS_NULLSTR(arg2) )
    {
    send_to_char( "Give what to whom?\n\r", ch );
    return;
    }

    if ( is_number( arg1 ) )
    {
    /* 'give NNNN coins victim' */
    int amount;
    bool cents;

    amount   = atoi(arg1);
    if ( amount <= 0
    || ( str_cmp( arg2, "coins" ) && str_cmp( arg2, "coin" ) && 
         str_cmp( arg2, "dollars"  ) && str_cmp( arg2, "cents")) )
    {
        send_to_char( "Sorry, you can't do that.\n\r", ch );
        return;
    }

    cents = str_cmp(arg2,"dollars");

    argument = one_argument( argument, arg2 );
    if ( IS_NULLSTR(arg2) )
    {
        send_to_char( "Give what to whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( (!cents && ch->dollars < amount) || (cents && ch->cents < amount) )
    {
        send_to_char( "You haven't got that much.\n\r", ch );
        return;
    }

    if (cents)
    {
        ch->cents       -= amount;
        victim->cents   += amount;
    }
    else
    {
        ch->dollars     -= amount;
        victim->dollars += amount;
    }

    act( Format("$n gives you %d %s.",amount, cents ? "cents" : "dollars"), ch, NULL, victim, TO_VICT, 0 );
    act( "$n gives $N some cash.",  ch, NULL, victim, TO_NOTVICT, 0 );
    act( Format("You give $N %d %s.",amount, cents ? "cents" : "dollars"), ch, NULL, victim, TO_CHAR, 0 );

    /*
     * Bribe trigger
     */
    if ( IS_NPC(victim) && HAS_TRIGGER( victim, TRIG_BRIBE ) )
        mp_bribe_trigger( victim, ch, cents ? amount : amount * 100 );

    if (IS_NPC(victim) && IS_SET(victim->act,ACT_IS_CHANGER))
    {
        int change;

        change = (cents ? 95 * amount / 100 / 100
                : 95 * amount);


        if (!cents && change > victim->cents)
            victim->cents += change;

        if (cents && change > victim->dollars)
            victim->dollars += change;

        if (change < 1 && can_see(victim,ch))
        {
            act("$n tells you 'I'm sorry, you did not give me enough to change.'",victim,NULL,ch,TO_VICT,0);
            ch->reply = victim;
            do_function(victim, &do_give, (char *)Format("%d %s %s", amount, cents ? "cents" : "dollars",ch->name));
        }
        else if (can_see(victim,ch))
        {
            do_function(victim, &do_give, (char *)Format("%d %s %s", change, cents ? "dollars" : "cents",ch->name));
            if (cents)
            {
                do_function(victim, &do_give, (char *)Format("%d cents %s", (95 * amount / 100 - change * 100),ch->name));
            }
            act("$n tells you 'Thank you, come again.'", victim,NULL,ch,TO_VICT,0);
            ch->reply = victim;
        }
    }
    return;
    }

    if ( ( obj = get_obj_carry( ch, arg1, ch ) ) == NULL )
    {
        send_to_char( "You do not have that item.\n\r", ch );
        return;
    }

    if ( obj->wear_loc != WEAR_NONE )
    {
        send_to_char( "You must remove it first.\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if(ch->quest && ch->quest->quest_type == Q_COURIER
            && victim == ch->quest->victim && obj == ch->quest->obj)
    {
        act( "$n gives $p to $N.", ch, obj, victim, TO_NOTVICT, 0 );
        act( "$n gives you $p.",   ch, obj, victim, TO_VICT, 0    );
        act( "You give $p to $N.", ch, obj, victim, TO_CHAR, 0    );
        (*quest_table[ch->quest->quest_type].q_fun) (ch, 2);
        return;
    }

    if (IS_NPC(victim) && victim->pIndexData->pShop != NULL)
    {
        act("$N tells you 'Sorry, you'll have to sell that.'", ch,NULL,victim,TO_CHAR, 0);
        ch->reply = victim;
        return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
        send_to_char( "You can't let go of it.\n\r", ch );
        return;
    }

    if ( victim->carry_number + get_obj_number( obj ) > can_carry_n( victim ) )
    {
        act( "$N has $S hands full.", ch, NULL, victim, TO_CHAR, 0 );
        return;
    }

    if (get_carry_weight(victim) + get_obj_weight(obj) > can_carry_w( victim ) )
    {
        act( "$N can't carry that much weight.", ch, NULL, victim, TO_CHAR, 0 );
        return;
    }

    if ( !can_see_obj( victim, obj ) )
    {
        act( "$N can't see it.", ch, NULL, victim, TO_CHAR, 0 );
        return;
    }

    if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) && IS_NPC(victim) )
    {
        t_obj = create_object( obj->pIndexData );
        if(IS_SET(t_obj->extra_flags, ITEM_INVENTORY))
            REMOVE_BIT(t_obj->extra_flags, ITEM_INVENTORY);
        if(IS_SET(victim->act2, ACT2_NEWS_STAND))
        {
            PURGE_DATA(t_obj->name);
            t_obj->name = str_dup(obj->name);
            PURGE_DATA(t_obj->short_descr);
            t_obj->short_descr = str_dup(obj->short_descr);
            PURGE_DATA(t_obj->description);
            t_obj->description = str_dup(obj->description);

            t_obj->cost = obj->cost;

            for(objed=obj->extra_descr;objed;objed=objed->next)
            {
                /* Set extra descriptions for articles */
                ALLOC_DATA (ed, EXTRA_DESCR_DATA, 1);
                ed->keyword         = str_dup(objed->keyword);
                ed->description     = str_dup(objed->description);
                ed->next            = t_obj->extra_descr;
                t_obj->extra_descr = ed;
            }

            t_obj->wear_loc = -1;
            PURGE_DATA(t_obj->full_desc);
            t_obj->full_desc = str_dup(obj->full_desc);
        }

    if (t_obj->timer > 0 && !IS_OBJ_STAT(t_obj,ITEM_HAD_TIMER))
        t_obj->timer = 0;
    REMOVE_BIT(t_obj->extra_flags,ITEM_HAD_TIMER);
    obj_to_char( t_obj, ch );
    obj = t_obj;
    }
    else
    {
        obj_from_char( obj );
        obj_to_char( obj, victim );
    }

    MOBtrigger = FALSE;
    act( "$n gives $p to $N.", ch, obj, victim, TO_NOTVICT, 0 );
    act( "$n gives you $p.",   ch, obj, victim, TO_VICT, 0    );
    act( "You give $p to $N.", ch, obj, victim, TO_CHAR, 0    );
    MOBtrigger = TRUE;

    /*
     * Give trigger
     */
    if ( IS_NPC(victim) && HAS_TRIGGER( victim, TRIG_GIVE ) )
    mp_give_trigger( victim, ch, obj );

    if((victim->race == race_lookup("werewolf")
    && IS_SET(obj->extra_flags, ITEM_ANTI_WEREWOLF))
    || (victim->race == race_lookup("human")
    && IS_SET(obj->extra_flags, ITEM_ANTI_HUMAN))
    || (victim->race == race_lookup("vampire")
    && IS_SET(obj->extra_flags, ITEM_ANTI_VAMPIRE))
    || (victim->race == race_lookup("faerie")
    && IS_SET(obj->extra_flags, ITEM_ANTI_CHANGELING)))
    {
    act("$p burns your hand!", victim, obj, ch, TO_CHAR, 0);
    act("$p sizzles in $n's hand.", victim, obj, ch, TO_ROOM, 0);
    gain_condition(ch, COND_PAIN, 10);
    do_function(victim, &do_drop, obj->name);
    /*
     * Anti-obj trigger
     */
    if ( IS_NPC(victim) && HAS_TRIGGER( victim, TRIG_ANTIOBJ ) )
        mp_antiobj_trigger( victim, ch, obj );
    }

    return;
}


/* for poisoning weapons and food/drink */
void do_envenom(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    AFFECT_DATA af;
    int percent = 0,skill = 0;

    CheckCH(ch);

    /* find out what */
    if (IS_NULLSTR(argument))
    {
    send_to_char("Envenom what item?\n\r",ch);
    return;
    }

    obj =  get_obj_list(ch,argument,ch->carrying);

    if (obj== NULL)
    {
    send_to_char("You don't have that item.\n\r",ch);
    return;
    }

    if ((skill = get_skill(ch,gsn_envenom)) < 1)
    {
    send_to_char("Are you crazy? You'd poison yourself!\n\r",ch);
    return;
    }

    if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON)
    {
    if (IS_OBJ_STAT(obj,ITEM_BLESS) || IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
    {
        act("You fail to poison $p.",ch,obj,NULL,TO_CHAR, 0);
        return;
    }

    if (number_percent() < skill)  /* success! */
    {
        act("$n treats $p with deadly poison.",ch,obj,NULL,TO_ROOM, 0);
        act("You treat $p with deadly poison.",ch,obj,NULL,TO_CHAR, 0);
        if (!obj->value[3])
        {
        obj->value[3] = 1;
        }
        WAIT_STATE(ch,skill_table[gsn_envenom].beats);
        return;
    }

    act("You fail to poison $p.",ch,obj,NULL,TO_CHAR, 0);
    WAIT_STATE(ch,skill_table[gsn_envenom].beats);
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
            act("You can't seem to envenom $p.",ch,obj,NULL,TO_CHAR, 0);
            return;
        }

    if (obj->value[3] < 0 
    ||  attack_table[obj->value[3]].damage == DAM_BASH)
    {
        send_to_char("You can only envenom edged weapons.\n\r",ch);
        return;
    }

        if (IS_WEAPON_STAT(obj,WEAPON_POISON))
        {
            act("$p is already envenomed.",ch,obj,NULL,TO_CHAR, 0);
            return;
        }

    percent = number_percent();
    if (percent < skill)
    {
 
            af.where     = TO_WEAPON;
            af.type      = gsn_poison;
            af.level     = ch->trust * percent / 100;
            af.duration  = ch->trust * percent / 100;
            af.location  = 0;
            af.modifier  = 0;
            af.bitvector = WEAPON_POISON;
            affect_to_obj(obj,&af);
 
            act("$n coats $p with deadly venom.",ch,obj,NULL,TO_ROOM, 0);
        act("You coat $p with venom.",ch,obj,NULL,TO_CHAR, 0);
        WAIT_STATE(ch,skill_table[gsn_envenom].beats);
            return;
        }
    else
    {
        act("You fail to envenom $p.",ch,obj,NULL,TO_CHAR, 0);
        WAIT_STATE(ch,skill_table[gsn_envenom].beats);
        return;
    }
    }

    act("You can't poison $p.",ch,obj,NULL,TO_CHAR, 0);
    return;
}

void do_fill( CHAR_DATA *ch, char *argument )
{
    char arg[MIL]={'\0'};
    OBJ_DATA *obj;
    OBJ_DATA *fountain;
    bool found = FALSE;

    CheckCH(ch);

    one_argument( argument, arg );

    if ( IS_NULLSTR(arg) )
    {
        send_to_char( "Fill what?\n\r", ch );
        return;
    }

    if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
    {
        send_to_char( "You do not have that item.\n\r", ch );
        return;
    }

    for ( fountain = ch->in_room->contents; fountain != NULL;
            fountain = fountain->next_content )
    {
        if ( fountain->item_type == ITEM_FOUNTAIN )
        {
            found = TRUE;
            break;
        }
    }

    if ( !found )
    {
        send_to_char( "There is no fountain here!\n\r", ch );
        return;
    }

    /* replace with ITEM_CONTAINER changes @@@@@ */
    if ( obj->item_type != ITEM_CONTAINER )
    {
        send_to_char( "You can't fill that.\n\r", ch );
        return;
    }

    if ( !IS_SET(obj->extra2, OBJ_WATERPROOF) )
    {
        send_to_char("That's not a waterproof container.\n\r", ch);
        return;
    }

    if (get_true_weight( obj ) > (obj->value[0] * 10))
    {
        send_to_char( "Your container is full.\n\r", ch );
        return;
    }

    act( Format("You fill $p with %s from $P.", liq_table[fountain->value[2]].liq_name), ch, obj,fountain, TO_CHAR, 0 );
    act(Format("$n fills $p with %s from $P.", liq_table[fountain->value[2]].liq_name),ch,obj,fountain,TO_ROOM, 0);
    obj_to_obj(create_puddle_from_liq(liq_table[fountain->value[2]].liq_name, OBJ_VNUM_POOL, obj->value[0] - get_true_weight( obj )),obj);
    for(obj = obj->contains; obj != NULL; obj = obj->next)
    {
        coat_obj_in_liquid_by_name(obj,liq_table[fountain->value[2]].liq_name);
    }
    return;
}

void do_pour (CHAR_DATA *ch, char *argument)
{
    char arg[MSL]={'\0'};
    OBJ_DATA *out, *in, *obj, *obj_next, *cont;
    CHAR_DATA *vch = NULL;
    int amount = 0;

    CheckCH(ch);

    argument = one_argument(argument,arg);

    if (IS_NULLSTR(arg) || IS_NULLSTR(argument))
    {
        send_to_char("Pour what into what?\n\r",ch);
        return;
    }


    if ((out = get_obj_carry(ch,arg, ch)) == NULL)
    {
        send_to_char("You don't have that item.\n\r",ch);
        return;
    }

    if (out->item_type != ITEM_CONTAINER)
    {
        send_to_char("That's not a container.\n\r",ch);
        return;
    }

    if (!str_cmp(argument,"out"))
    {
        if (out->contains == NULL)
        {
            send_to_char("It's already empty.\n\r",ch);
            return;
        }

        for(obj = out->contains; obj != NULL; obj = obj_next)
        {
            obj_next = obj->next;

            obj_from_obj(obj);

            if(obj->item_type == ITEM_LIQUID
                    || material_table[material_lookup(obj->material)].is_liquid
                    == TRUE)
            {

                /* Create puddle. */
                obj = create_puddle_from_obj(obj, OBJ_VNUM_PUDDLE, TRUE);
                if(obj != NULL)
                {
                    obj_to_room(obj, ch->in_room);
                    act(Format("You invert $p, spilling %s all over the ground.", liq_table[out->value[2]].liq_name),ch,out,NULL,TO_CHAR, 0);
                    act(Format("$n inverts $p, spilling %s all over the ground.", liq_table[out->value[2]].liq_name),ch,out,NULL,TO_ROOM, 0);
                }
                else
                {
                    act(Format("You invert $p, but nothing comes out."),ch,out,NULL,TO_CHAR, 0);
                    act(Format("$n inverts $p, but nothing comes out."),ch,out,NULL,TO_ROOM, 0);
                }
            }
        }

        return;
    }

    if ((in = get_obj_here(ch,argument)) == NULL)
    {
        vch = get_char_room(ch,argument);

        if (vch == NULL)
        {
            send_to_char("Pour into what?\n\r",ch);
            return;
        }

        in = get_eq_char(vch,WEAR_HOLD);

        if (in == NULL)
        {
            send_to_char("They aren't holding anything.",ch);
            return;
        }
    }

    if(out->item_type != ITEM_CONTAINER)
    {
        send_to_char("That's not a container!\n\r", ch);
        return;
    }

    if (in == out)
    {
        send_to_char("You cannot change the laws of physics!\n\r",ch);
        return;
    }

    /* Pouring into a non-waterproof container coats the container and
     * everything in it and creates a puddle.
     */
    if (!IS_SET(in->extra2, OBJ_WATERPROOF))
    {
        if (out->contains == NULL)
        {
            send_to_char("It's already empty.\n\r",ch);
            return;
        }

        for(obj = out->contains; obj != NULL; obj = obj_next)
        {
            obj_next = obj->next;

            obj_from_obj(obj);

            if(obj->item_type == ITEM_LIQUID
                    || material_table[material_lookup(obj->material)].is_liquid
                    == TRUE)
            {
                act(Format("You pour $p into $P, which promptly leaks %s all over the ground.", liq_table[out->value[2]].liq_name),ch,out,in,TO_CHAR, 0);

                act(Format("$n pours $p into $P, which promptly leaks %s all over the ground.", liq_table[out->value[2]].liq_name),ch,out,in,TO_ROOM, 0);

                /* Coat contents of target container. */
                for(cont = in->contains; cont != NULL; cont = cont->next)
                {
                    coat_obj_in_liquid(cont, obj);
                }

                /* Create puddle. */
                if(obj->weight < 1) obj->weight = 1;
                obj = create_puddle_from_obj(obj, OBJ_VNUM_PUDDLE, TRUE);
                obj_to_room(obj, ch->in_room);
            }
            else if ((((get_obj_weight( obj ) + get_true_weight( in )
                    > (in->value[0] * 10))
                    && (in->item_type == ITEM_CONTAINER)))
                    &&   ((get_obj_weight(obj) > (in->value[3] * 10)
                            && in->item_type == ITEM_CONTAINER)
                            || (get_obj_weight(obj) > (in->value[4] * 10)
                                    && in->item_type == ITEM_CONTAINER)))
            {
                act( "$P tumbles to the ground as you pour from $p.", ch,out,obj,TO_CHAR, 0);
                act( "$P tumbles to the ground as $n pours from $p.", ch,out,obj,TO_ROOM, 0);
                obj_to_room(obj, ch->in_room);
            }
            else
            {
                act( "$P tumbles into $p.", ch,out,obj,TO_CHAR, 0);
                act( "$n pours $P into $p.", ch,out,obj,TO_ROOM, 0);
                obj_to_obj(obj, in);
            }
        }

        return;
    }

    if (out->contains == NULL)
    {
        act("There's nothing in $p to pour.",ch,out,NULL,TO_CHAR, 0);
        return;
    }

    if (get_true_weight( in ) > (in->value[0] * 10))
    {
        act("$p is already filled to the top.",ch,in,NULL,TO_CHAR, 0);
        return;
    }

    /* Object move/create, change and coat any objs */
    for(obj = out->contains; obj != NULL; obj = obj_next)
    {
        obj_next = obj->next;

        obj_from_obj(obj);

        if(obj->item_type == ITEM_LIQUID
                || material_table[material_lookup(obj->material)].is_liquid == TRUE)
        {
            /* Coat contents of target container. */
            for(cont = in->contains; cont != NULL; cont = cont->next)
            {
                coat_obj_in_liquid(cont, obj);
            }

            /* Create puddle. */
            if(obj->weight < 1) obj->weight = 1;
            amount = UMIN(get_true_weight(obj),
                    in->value[0] - get_true_weight( in ));
            obj->weight -= amount /
                    material_table[material_lookup(obj->material)].weight;
            cont = create_puddle_from_obj(obj, OBJ_VNUM_POOL, FALSE);
            cont->weight = amount;
            obj_to_obj(cont, in);
            if(obj->weight <= 0)
                extract_obj(obj);

            if (vch == NULL)
            {
                act(Format("You pour %s from $p into $P.", liq_table[out->value[2]].liq_name),ch,out,in,TO_CHAR, 0);
                act(Format("$n pours %s from $p into $P.", liq_table[out->value[2]].liq_name),ch,out,in,TO_ROOM, 0);
            }
            else
            {
                act(Format("You pour some %s for $N.", liq_table[out->value[2]].liq_name),ch,NULL,vch,TO_CHAR, 0);
                act(Format("$n pours you some %s.", liq_table[out->value[2]].liq_name),ch,NULL,vch,TO_VICT, 0);
                act(Format("$n pours some %s for $N.", liq_table[out->value[2]].liq_name),ch,NULL,vch,TO_NOTVICT, 0);
            }
        }
        else
        {
            if ((((get_obj_weight( obj ) + get_true_weight( in )
                    > (in->value[0] * 10))
                    && (in->item_type == ITEM_CONTAINER)))
                    &&   ((get_obj_weight(obj) > (in->value[3] * 10)
                            && in->item_type == ITEM_CONTAINER)
                            || (get_obj_weight(obj) > (in->value[4] * 10)
                                    && in->item_type == ITEM_CONTAINER)))
            {
                act( "$P tumbles to the ground as you pour from $p.", ch,out,obj,TO_CHAR, 0);
                act( "$P tumbles to the ground as $n pours from $p.", ch,out,obj,TO_ROOM, 0);
                obj_to_room(obj, ch->in_room);
            }
            else
            {
                act( "$P tumbles into $p.", ch,out,obj,TO_CHAR, 0);
                act( "$n pours $P into $p.", ch,out,obj,TO_ROOM, 0);
                obj_to_obj(obj, in);
            }
        }
    }
}

void do_drink( CHAR_DATA *ch, char *argument )
{
    char arg[MIL]={'\0'};
    OBJ_DATA *obj, *cont = NULL, *cont_next;
    int amount = 0;
    int liquid = 0;

    CheckCH(ch);

    one_argument( argument, arg );

    if ( IS_NULLSTR(arg) )
    {
    for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
    {
        if ( obj->item_type == ITEM_FOUNTAIN )
        break;
    }

    if ( obj == NULL )
    {
        send_to_char( "Drink what?\n\r", ch );
        return;
    }
    }
    else
    {
    if ( ( obj = get_obj_here( ch, arg ) ) == NULL )
    {
        send_to_char( "You can't find it.\n\r", ch );
        return;
    }
    }

    if ( !IS_NPC(ch) && ch->condition[COND_TRIPPING] > 10 )
    {
    send_to_char(
     "You try to take a drink but just make a sucking sound.\n\r", ch );
    return;
    }

    if ( !IS_NPC(ch) && ch->condition[COND_DRUNK] > 10 )
    {
    send_to_char( "You fail to reach your mouth.  *Hic*\n\r", ch );
    return;
    }

    if ( !IS_NPC(ch) && ch->condition[COND_HIGH] > 10 )
    send_to_char( "Mmmmm... feels good... want more!\n\r", ch );

    if (!IS_NPC(ch) && !IS_ADMIN(ch) 
    &&  ch->condition[COND_FULL] > 45)
    {
    send_to_char("You're too full to drink more.\n\r",ch);
    return;
    }

    switch ( obj->item_type )
    {
    default:
    send_to_char( "You can't drink from that.\n\r", ch );
    return;

    case ITEM_FOUNTAIN:
        if ( ( liquid = obj->value[2] )  < 0 )
        {
            log_string(LOG_BUG, Format("Do_drink: bad liquid number %d.", liquid ));
            liquid = obj->value[2] = 0;
        }
    amount = liq_table[liquid].liq_affect[4] * 3;
    break;

/* @@@@@ */
    case ITEM_LIQUID:
        if ( ( liquid = liq_lookup(obj->material) )  < 0 )
        {
            log_string(LOG_BUG, Format("Do_drink: bad liquid number %d.", liquid));
            liquid = 0;
        }
    amount = liq_table[liquid].liq_affect[4];
    amount = UMIN(amount, obj->weight);
    break;

    case ITEM_CONTAINER:
    if ( obj->contains == NULL )
    {
        send_to_char( "There's nothing in it.\n\r", ch );
        return;
    }

    for(cont = obj->contains; cont != NULL; cont = cont_next)
    {
        cont_next = cont->next;
        if(cont->item_type == ITEM_LIQUID)
        {
        if((liquid = liq_lookup(cont->material)) < 0)
        {
            log_string(LOG_BUG, Format("Do_drink: bad liquid number %d.", liquid ));
            liquid = 0;
        }
        amount = liq_table[liquid].liq_affect[4];
        amount = UMIN(amount, cont->weight);
        break;
        }
    }
    break;
     }

    if(ch->race == race_lookup("vampire"))
    {
    if(!str_cmp(liq_table[liquid].liq_name, "blood"))
    {
        gain_condition( ch, COND_THIRST,
        -1 * amount * liq_table[liquid].liq_affect[COND_THIRST] );
    }
    else
    {
        send_to_char("Your stomach revolts against the stuff!\n\r", ch);
        gain_condition( ch, COND_THIRST, -5);
        ch->RBPG -= 1;
    }
    }
    else
    {
    gain_condition( ch, COND_FULL,
        amount * liq_table[liquid].liq_affect[COND_FULL] / 4 );
    gain_condition( ch, COND_THIRST,
        amount * liq_table[liquid].liq_affect[COND_THIRST] / 10 );
    gain_condition(ch, COND_HUNGER,
        amount * liq_table[liquid].liq_affect[COND_HUNGER] / 2 );
    gain_condition( ch, COND_DRUNK,
        amount * liq_table[liquid].liq_affect[COND_DRUNK] / 36 );
    }

    if(ch->race == race_lookup("vampire")
    && !str_cmp(liq_table[liquid].liq_name, "blood"))
    {
    ch->RBPG += UMIN(amount,(ch->max_RBPG - ch->RBPG));
    }

    act( "$n drinks $T from $p.", ch, obj, liq_table[liquid].liq_name, TO_ROOM, 0 );
    act( "You drink $T from $p.", ch, obj, liq_table[liquid].liq_name, TO_CHAR, 0 );

    if ( !IS_NPC(ch) && ch->condition[COND_DRUNK]  > 10 )
        send_to_char( "You feel drunk.\n\r", ch );
    if( ch->race != race_lookup("vampire"))
    {
        if ( !IS_NPC(ch) && ch->condition[COND_FULL]   > 40 )
            send_to_char( "You are full.\n\r", ch );
        if ( !IS_NPC(ch) && ch->condition[COND_THIRST] > 40 )
            send_to_char( "Your thirst is quenched.\n\r", ch );
    }

    if ( obj->value[3] != 0 )
    {
    /* The drink was poisoned ! */
    AFFECT_DATA af;

    act( "$n chokes and gags.", ch, NULL, NULL, TO_ROOM, 0 );
    send_to_char( "You choke and gag.\n\r", ch );
    af.where     = TO_AFFECTS;
    af.type      = gsn_poison;
    af.level     = number_fuzzy(amount); 
    af.duration  = 3 * amount;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_POISON;
    affect_join( ch, &af );
    }

    if(obj->item_type == ITEM_LIQUID)
    if(obj->weight <= 0)
        extract_obj(obj);

    if(cont != NULL)
    if(cont->weight <= 0)
        extract_obj(cont);

    return;
}



void do_eat( CHAR_DATA *ch, char *argument )
{
    char arg[MIL]={'\0'};
    OBJ_DATA *obj;
    int a = 0, b = 0;

    CheckCH(ch);

    if ( ch->race == race_lookup("vampire") && !IS_ADMIN(ch) )
    {
    send_to_char( "Huh?\n\r", ch );
    return;
    }

    one_argument( argument, arg );
    if ( IS_NULLSTR(arg) )
    {
    send_to_char( "Eat what?\n\r", ch );
    return;
    }

    if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
    {
    send_to_char( "You do not have that item.\n\r", ch );
    return;
    }

    if ( !IS_ADMIN(ch) )
    {
    if ( !material_table[material_lookup(obj->material)].is_edible && obj->item_type != ITEM_PILL )
    {
        send_to_char( "That's not edible.\n\r", ch );
        return;
    }

    if ( !IS_NPC(ch) && ch->condition[COND_FULL] > 50 )
    {   
        send_to_char( "You are too full to eat more.\n\r", ch );
        return;
    }

    if (material_table[material_lookup(obj->material)].is_liquid)
    {
        do_function(ch, &do_drink, arg);
        return;
    }
    }

    act( "$n eats $p.",  ch, obj, NULL, TO_ROOM, 0 );
    act( "You eat $p.", ch, obj, NULL, TO_CHAR, 0 );

    switch ( obj->item_type )
    {

    case ITEM_FOOD:
    if ( !IS_NPC(ch) )
    {
        int condition;

        a = obj->value[0] + (obj->weight * material_table[material_lookup(obj->material)].full);
        b = obj->value[1] + (obj->weight * material_table[material_lookup(obj->material)].hunger);
        condition = ch->condition[COND_HUNGER];
        gain_condition( ch, COND_FULL, a );
        gain_condition( ch, COND_HUNGER, b );
        if ( condition == 0 && ch->condition[COND_HUNGER] > 0 )
        send_to_char( "You are no longer hungry.\n\r", ch );
        else if ( ch->condition[COND_FULL] > 50 )
        send_to_char( "You are full.\n\r", ch );
    }

    if (( obj->value[3] != 0
    || material_table[material_lookup(obj->material)].is_poison )
    && !IS_ADMIN(ch))
    {
        /* The food was poisoned! */
        AFFECT_DATA af;

        act( "$n chokes and gags.", ch, 0, 0, TO_ROOM, 0 );
        send_to_char( "You choke and gag.\n\r", ch );

        af.where     = TO_AFFECTS;
        af.type      = gsn_poison;
        af.level     = number_fuzzy(obj->value[0]);
        af.duration  = 2 * obj->value[0];
        af.location  = APPLY_NONE;
        af.modifier  = 0;
        af.bitvector = AFF_POISON;
        affect_join( ch, &af );
    }
    break;

    case ITEM_PILL:
    obj_cast_spell( obj->value[1], obj->value[0], ch, ch, NULL );
    obj_cast_spell( obj->value[2], obj->value[0], ch, ch, NULL );
    obj_cast_spell( obj->value[3], obj->value[0], ch, ch, NULL );
    break;
    default :
    if ( !IS_NPC(ch) && material_table[material_lookup(obj->material)].is_edible)
    {
        int condition;

        a = obj->weight * material_table[material_lookup(obj->material)].full;
        b = obj->weight * material_table[material_lookup(obj->material)].hunger;
        condition = ch->condition[COND_HUNGER];
        gain_condition( ch, COND_FULL, a );
        gain_condition( ch, COND_HUNGER, b );
        if ( condition == 0 && ch->condition[COND_HUNGER] > 0 )
        send_to_char( "You are no longer hungry.\n\r", ch );
        else if ( ch->condition[COND_FULL] > 50 )
        send_to_char( "You are full.\n\r", ch );
    }

    if (material_table[material_lookup(obj->material)].is_poison
    && !IS_ADMIN(ch))
    {
        /* The food was poisoned! */
        AFFECT_DATA af;

        act( "$n chokes and gags.", ch, 0, 0, TO_ROOM, 0 );
        send_to_char( "You choke and gag.\n\r", ch );

        af.where     = TO_AFFECTS;
        af.type      = gsn_poison;
        af.level     = number_fuzzy(obj->value[0]);
        af.duration  = 2 * get_obj_weight(obj);
        af.location  = APPLY_NONE;
        af.modifier  = 0;
        af.bitvector = AFF_POISON;
        affect_join( ch, &af );
    }
    break;
    }

    extract_obj( obj );
    return;
}



/*
 * Remove an object.
 */
bool remove_obj( CHAR_DATA *ch, int iWear, bool fReplace )
{
    OBJ_DATA *obj;

    if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
    return TRUE;

    if ( !fReplace )
    return FALSE;

    if ( IS_SET(obj->extra_flags, ITEM_NOREMOVE) )
    {
    act( "You can't remove $p.", ch, obj, NULL, TO_CHAR, 0 );
    return FALSE;
    }

    unequip_char( ch, obj );
    act( "$n stops using $p.", ch, obj, NULL, TO_ROOM, 0 );
    act( "You stop using $p.", ch, obj, NULL, TO_CHAR, 0 );
    return TRUE;
}



/*
 * Wear one object.
 * Optional replacement of existing objects.
 * Big repetitive code, ick.
 */
void wear_obj( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace, char *wear_loc )
{
    if(!str_cmp(wear_loc, "hagwrinkles"))
    {
    equip_char( ch, obj, WEAR_SKINFLAP );
    return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_FINGER ) )
    {
    if ( get_eq_char( ch, WEAR_FINGER_L ) != NULL
    &&   get_eq_char( ch, WEAR_FINGER_R ) != NULL
    &&   !remove_obj( ch, WEAR_FINGER_L, fReplace )
    &&   !remove_obj( ch, WEAR_FINGER_R, fReplace ) )
        return;

    if ( get_eq_char( ch, WEAR_FINGER_L ) == NULL )
    {
        act( "$n wears $p on $s left finger.",    ch, obj, NULL, TO_ROOM,0);
        act( "You wear $p on your left finger.",  ch, obj, NULL, TO_CHAR,0);
        equip_char( ch, obj, WEAR_FINGER_L );
        return;
    }

    if ( get_eq_char( ch, WEAR_FINGER_R ) == NULL )
    {
        act( "$n wears $p on $s right finger.",   ch, obj, NULL, TO_ROOM,0);
        act( "You wear $p on your right finger.", ch, obj, NULL, TO_CHAR,0);
        equip_char( ch, obj, WEAR_FINGER_R );
        return;
    }

    log_string(LOG_BUG, "Wear_obj: no free finger.");
    send_to_char( "You already wear two rings.\n\r", ch );
    return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_NECK ) )
    {
    if ( get_eq_char( ch, WEAR_NECK ) != NULL
    &&   !remove_obj( ch, WEAR_NECK, fReplace ) )
        return;

    if ( get_eq_char( ch, WEAR_NECK ) == NULL )
    {
        act( "$n wears $p around $s neck.",   ch, obj, NULL, TO_ROOM, 0 );
        act( "You wear $p around your neck.", ch, obj, NULL, TO_CHAR, 0 );
        equip_char( ch, obj, WEAR_NECK );
        return;
    }

    log_string(LOG_BUG, "Wear_obj: no free neck.");
    send_to_char( "You've already got something around your neck.\n\r", ch );
    return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_BODY ) )
    {
    if ( !remove_obj( ch, WEAR_BODY, fReplace ) )
        return;
    act( "$n wears $p on $s torso.",   ch, obj, NULL, TO_ROOM, 0 );
    act( "You wear $p on your torso.", ch, obj, NULL, TO_CHAR, 0 );
    equip_char( ch, obj, WEAR_BODY );
    return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_HEAD ) )
    {
    if ( !remove_obj( ch, WEAR_HEAD, fReplace ) )
        return;
    act( "$n wears $p on $s head.",   ch, obj, NULL, TO_ROOM, 0 );
    act( "You wear $p on your head.", ch, obj, NULL, TO_CHAR, 0 );
    equip_char( ch, obj, WEAR_HEAD );
    return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_LEGS ) )
    {
    if ( !remove_obj( ch, WEAR_LEGS, fReplace ) )
        return;
    act( "$n wears $p on $s legs.",   ch, obj, NULL, TO_ROOM, 0 );
    act( "You wear $p on your legs.", ch, obj, NULL, TO_CHAR, 0 );
    equip_char( ch, obj, WEAR_LEGS );
    return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_FEET ) )
    {
    if ( !remove_obj( ch, WEAR_FEET, fReplace ) )
        return;
    act( "$n wears $p on $s feet.",   ch, obj, NULL, TO_ROOM, 0 );
    act( "You wear $p on your feet.", ch, obj, NULL, TO_CHAR, 0 );
    equip_char( ch, obj, WEAR_FEET );
    return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_HANDS ) )
    {
    if ( !remove_obj( ch, WEAR_HANDS, fReplace ) )
        return;
    act( "$n wears $p on $s hands.",   ch, obj, NULL, TO_ROOM, 0 );
    act( "You wear $p on your hands.", ch, obj, NULL, TO_CHAR, 0 );
    equip_char( ch, obj, WEAR_HANDS );
    return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_ABOUT ) )
    {
    if ( !remove_obj( ch, WEAR_ABOUT, fReplace ) )
        return;
    act( "$n wears $p about $s torso.",   ch, obj, NULL, TO_ROOM, 0 );
    act( "You wear $p about your torso.", ch, obj, NULL, TO_CHAR, 0 );
    equip_char( ch, obj, WEAR_ABOUT );
    return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_WAIST ) )
    {
    if ( !remove_obj( ch, WEAR_WAIST, fReplace ) )
        return;
    act( "$n wears $p about $s waist.",   ch, obj, NULL, TO_ROOM, 0 );
    act( "You wear $p about your waist.", ch, obj, NULL, TO_CHAR, 0 );
    equip_char( ch, obj, WEAR_WAIST );
    return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_UNDERTOP ) )
    {
    if ( !remove_obj( ch, WEAR_UNDERTOP, fReplace ) )
        return;
    act( "$n wears $p as an undershirt.",   ch, obj, NULL, TO_ROOM, 0 );
    act( "You wear $p as an undershirt.", ch, obj, NULL, TO_CHAR, 0 );
    equip_char( ch, obj, WEAR_UNDERTOP );
    return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_UNDERPANTS ) )
    {
    if ( !remove_obj( ch, WEAR_UNDERPANTS, fReplace ) )
        return;
    act( "$n wears $p for underwear.",   ch, obj, NULL, TO_ROOM, 0 );
    act( "You wear $p for underwear.", ch, obj, NULL, TO_CHAR, 0 );
    equip_char( ch, obj, WEAR_UNDERPANTS );
    return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_WRIST ) )
    {
    if ( get_eq_char( ch, WEAR_WRIST_L ) != NULL
    &&   get_eq_char( ch, WEAR_WRIST_R ) != NULL
    &&   !remove_obj( ch, WEAR_WRIST_L, fReplace )
    &&   !remove_obj( ch, WEAR_WRIST_R, fReplace ) )
        return;

    if ( get_eq_char( ch, WEAR_WRIST_L ) == NULL )
    {
        act( "$n wears $p around $s left wrist.", ch, obj, NULL, TO_ROOM, 0 );
        act( "You wear $p around your left wrist.", ch, obj, NULL, TO_CHAR, 0 );
        equip_char( ch, obj, WEAR_WRIST_L );
        return;
    }

    if ( get_eq_char( ch, WEAR_WRIST_R ) == NULL )
    {
        act( "$n wears $p around $s right wrist.", ch, obj, NULL, TO_ROOM, 0 );
        act( "You wear $p around your right wrist.", ch, obj, NULL, TO_CHAR, 0 );
        equip_char( ch, obj, WEAR_WRIST_R );
        return;
    }

    log_string(LOG_BUG, "Wear_obj: no free wrist.");
    send_to_char( "You already wear two wrist items.\n\r", ch );
    return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_HIP ) )
    {
    if ( get_eq_char( ch, WEAR_LEFT_HIP ) != NULL
    &&   get_eq_char( ch, WEAR_RIGHT_HIP ) != NULL
    &&   !remove_obj( ch, WEAR_LEFT_HIP, fReplace )
    &&   !remove_obj( ch, WEAR_RIGHT_HIP, fReplace ) )
        return;

    if ( get_eq_char( ch, WEAR_LEFT_HIP ) == NULL )
    {
        act( "$n wears $p on $s left hip.", ch, obj, NULL, TO_ROOM, 0 );
        act( "You wear $p on your left hip.", ch, obj, NULL, TO_CHAR, 0 );
        equip_char( ch, obj, WEAR_LEFT_HIP );
        return;
    }

    if ( get_eq_char( ch, WEAR_RIGHT_HIP ) == NULL )
    {
        act( "$n wears $p on $s right hip.", ch, obj, NULL, TO_ROOM, 0 );
        act( "You wear $p on your right hip.", ch, obj, NULL, TO_CHAR, 0 );
        equip_char( ch, obj, WEAR_RIGHT_HIP );
        return;
    }

    log_string(LOG_BUG, "Wear_obj: no free hip.");
    send_to_char( "You already wear two hip items.\n\r", ch );
    return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_ARMPIT ) )
    {
    if ( get_eq_char( ch, WEAR_LEFT_PIT ) != NULL
    &&   get_eq_char( ch, WEAR_RIGHT_PIT ) != NULL
    &&   !remove_obj( ch, WEAR_LEFT_PIT, fReplace )
    &&   !remove_obj( ch, WEAR_RIGHT_PIT, fReplace ) )
        return;

    if ( get_eq_char( ch, WEAR_LEFT_PIT ) == NULL )
    {
        act( "$n wears $p under $s left armpit.", ch, obj, NULL, TO_ROOM, 0 );
        act( "You wear $p under your left armpit.", ch, obj, NULL, TO_CHAR, 0 );
        equip_char( ch, obj, WEAR_LEFT_PIT );
        return;
    }

    if ( get_eq_char( ch, WEAR_RIGHT_PIT ) == NULL )
    {
        act( "$n wears $p around $s right armpit.", ch, obj, NULL, TO_ROOM, 0 );
        act( "You wear $p around your right armpit.", ch, obj, NULL, TO_CHAR, 0 );
        equip_char( ch, obj, WEAR_RIGHT_PIT );
        return;
    }

    log_string(LOG_BUG, "Wear_obj: no free armpit." );
    send_to_char( "You already wear two armpit items.\n\r", ch );
    return;
    }

    if ( CAN_WEAR( obj, ITEM_WIELD ) )
    {
    int sn,skill;
    OBJ_DATA *held = NULL;

    if ( !remove_obj( ch, WEAR_WIELD, fReplace ) )
        return;

    if ( !IS_NPC(ch) 
    && get_obj_weight(obj) > (get_curr_stat(ch,STAT_STR)  
        * 100))
    {
        send_to_char( "It is too heavy for you to wield.\n\r", ch );
        return;
    }

    if (!IS_NPC(ch) && ch->size < SIZE_LARGE 
    &&  CAN_WEAR(obj,ITEM_TWO_HANDS)
    &&  get_eq_char(ch, WEAR_HOLD) != NULL)
    {
        send_to_char("You need two hands free for that weapon.\n\r",ch);
        return;
    }

    if (!IS_NPC(ch) && ch->size < SIZE_LARGE
    && (held = get_eq_char(ch, WEAR_HOLD)) != NULL
    && CAN_WEAR(held, ITEM_TWO_HANDS))
    {
        send_to_char("Your hands are already full.\n\r",ch);
        return;
    }

    act( "$n wields $p.", ch, obj, NULL, TO_ROOM, 0 );
    act( "You wield $p.", ch, obj, NULL, TO_CHAR, 0 );
    equip_char( ch, obj, WEAR_WIELD );

        sn = get_weapon_sn(ch);

    if (sn == gsn_brawl)
       return;

        skill = get_weapon_skill(ch,sn);
 
        if (skill > 5)
            act("$p feels like a part of you!",ch,obj,NULL,TO_CHAR, 0);
        else if (skill >= 4)
            act("You feel quite confident with $p.",ch,obj,NULL,TO_CHAR, 0);
        else if (skill == 3)
            act("You are skilled with $p.",ch,obj,NULL,TO_CHAR, 0);
        else if (skill == 2)
            act("Your skill with $p is adequate.",ch,obj,NULL,TO_CHAR, 0);
        else if (skill == 1)
            act("$p feels a little clumsy in your hands.", ch,obj,NULL,TO_CHAR,0);
        else
            act("You don't even know which end is up on $p.", ch,obj,NULL,TO_CHAR, 0);

    return;
    }

    if ( CAN_WEAR( obj, ITEM_HOLD ) )
    {
    OBJ_DATA *wield;

    if ( !remove_obj( ch, WEAR_HOLD, fReplace ) )
        return;

    if (!IS_NPC(ch) && ch->size < SIZE_LARGE 
    &&  CAN_WEAR(obj,ITEM_TWO_HANDS)
    &&  get_eq_char(ch, WEAR_WIELD) != NULL)
    {
        send_to_char("You need two hands free for that item.\n\r",ch);
        return;
    }

    if (!IS_NPC(ch) && ch->size < SIZE_LARGE
    && (wield = get_eq_char(ch, WEAR_WIELD)) != NULL
    && CAN_WEAR(wield, ITEM_TWO_HANDS))
    {
        send_to_char("Your hands are already full.\n\r",ch);
        return;
    }

    act( "$n holds $p in $s hand.",   ch, obj, NULL, TO_ROOM, 0 );
    act( "You hold $p in your hand.", ch, obj, NULL, TO_CHAR, 0 );
    equip_char( ch, obj, WEAR_HOLD );
    return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_FACE ) )
    {
    if ( !remove_obj( ch, WEAR_FACE, fReplace ) )
        return;
    act( "$n places $p on $s face.",   ch, obj, NULL, TO_ROOM, 0 );
    act( "You wear $p on your face.", ch, obj, NULL, TO_CHAR, 0 );
    equip_char( ch, obj, WEAR_FACE );
    return;
    }

    if ( fReplace )
    send_to_char( "You can't wear, wield, or hold that.\n\r", ch );

    return;
}



void do_wear( CHAR_DATA *ch, char *argument )
{
    char arg[MIL]={'\0'};
    OBJ_DATA *obj;

    CheckCH(ch);

    argument = one_argument( argument, arg );

    if ( IS_NULLSTR(arg) )
    {
        send_to_char( "Wear, wield, or hold what?\n\r", ch );
        return;
    }

    if (ch->shape == SHAPE_BAT || ch->shape == SHAPE_WOLF)
    {
        send_to_char( "You can't wear, wield or hold anything without hands.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
        OBJ_DATA *obj_next;

        for ( obj = ch->carrying; obj != NULL; obj = obj_next )
        {
            obj_next = obj->next_content;
            if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) )
                wear_obj( ch, obj, FALSE, "" );
        }
        return;
    }
    else
    {
        if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
        {
            send_to_char( "You do not have that item.\n\r", ch );
            return;
        }

        argument = one_argument(argument, arg);

        if(!IS_NULLSTR(arg) && flag_lookup(arg, wear_loc_flags) == -1)
        {
            send_to_char("No such wear location.\n\r", ch);
            return;
        }

        wear_obj( ch, obj, TRUE, "" );
    }

    return;
}



void do_remove( CHAR_DATA *ch, char *argument )
{
    char arg[MIL]={'\0'};
    OBJ_DATA *obj;

    CheckCH(ch);

    one_argument( argument, arg );

    if ( IS_NULLSTR(arg) )
    {
        send_to_char( "Remove what?\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
        OBJ_DATA *obj_next;

        for ( obj = ch->carrying; obj != NULL; obj = obj_next )
        {
            obj_next = obj->next_content;
            if ( obj->wear_loc != WEAR_NONE && can_see_obj( ch, obj ) )
                remove_obj( ch, obj->wear_loc, TRUE );
        }
        return;
    }
    else
    {
        if ( ( obj = get_obj_wear( ch, arg ) ) == NULL )
        {
            send_to_char( "You do not have that item.\n\r", ch );
            return;
        }

        remove_obj( ch, obj->wear_loc, TRUE );
    }
    return;
}



void do_quaff( CHAR_DATA *ch, char *argument )
{
    char arg[MIL]={'\0'};
    OBJ_DATA *obj;

    CheckCH(ch);

    one_argument( argument, arg );

    if ( IS_NULLSTR(arg) )
    {
    send_to_char( "Quaff what?\n\r", ch );
    return;
    }

    if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
    {
    send_to_char( "You do not have that potion.\n\r", ch );
    return;
    }

    if ( obj->item_type != ITEM_POTION )
    {
    send_to_char( "You can quaff only potions.\n\r", ch );
    return;
    }

    act( "$n quaffs $p.", ch, obj, NULL, TO_ROOM, 0 );
    act( "You quaff $p.", ch, obj, NULL ,TO_CHAR, 0 );

    obj_cast_spell( obj->value[1], obj->value[0], ch, ch, NULL );
    obj_cast_spell( obj->value[2], obj->value[0], ch, ch, NULL );
    obj_cast_spell( obj->value[3], obj->value[0], ch, ch, NULL );

    extract_obj( obj );
    return;
}



void do_recite( CHAR_DATA *ch, char *argument )
{
    char arg1[MIL]={'\0'};
    char arg2[MIL]={'\0'};
    CHAR_DATA *victim;
    OBJ_DATA *scroll;
    OBJ_DATA *obj;

    CheckCH(ch);

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( ( scroll = get_obj_carry( ch, arg1, ch ) ) == NULL )
    {
    send_to_char( "You do not have that scroll.\n\r", ch );
    return;
    }

    if ( scroll->item_type != ITEM_SCROLL )
    {
    send_to_char( "You can recite only scrolls.\n\r", ch );
    return;
    }

    send_to_char(
        "This scroll is too complex for you to comprehend.\n\r",ch);
    return;

    obj = NULL;
    if ( IS_NULLSTR(arg2) )
    {
    victim = ch;
    }
    else
    {
    if ( ( victim = get_char_room ( ch, arg2 ) ) == NULL
    &&   ( obj    = get_obj_here  ( ch, arg2 ) ) == NULL )
    {
        send_to_char( "You can't find it.\n\r", ch );
        return;
    }
    }

    act( "$n recites $p.", ch, scroll, NULL, TO_ROOM, 0 );
    act( "You recite $p.", ch, scroll, NULL, TO_CHAR, 0 );

    if (number_percent() >= 20 + get_skill(ch,gsn_scrolls) * 4/5)
    {
    send_to_char("You mispronounce a syllable.\n\r",ch);
    }

    else
    {
        obj_cast_spell( scroll->value[1], scroll->value[0], ch, victim, obj );
        obj_cast_spell( scroll->value[2], scroll->value[0], ch, victim, obj );
        obj_cast_spell( scroll->value[3], scroll->value[0], ch, victim, obj );
    }

    extract_obj( scroll );
    return;
}


void do_steal( CHAR_DATA *ch, char *argument )
{
    char buf  [MSL]={'\0'};
    char arg1 [MIL]={'\0'};
    char arg2 [MIL]={'\0'};
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int percent = 0, fail = 0;

    CheckCH(ch);

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( IS_NULLSTR(arg1) || IS_NULLSTR(arg2) )
    {
        send_to_char( "Steal what from whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( victim == ch )
    {
        send_to_char( "That's pointless.\n\r", ch );
        return;
    }

    WAIT_STATE( ch, skill_table[gsn_steal].beats );
    percent  = get_curr_stat(victim, STAT_DEX) + victim->ability[ALERTNESS].value;

    if (!IS_AWAKE(victim))
        percent -= 1;
    else if (!can_see(victim,ch))
        percent += 2;
    else
        percent += 3;

    if (percent > 10) percent = 10;
    if (percent < 6) percent = 6;

    fail = dice_rolls(ch, get_curr_stat(ch, STAT_DEX) + ch->ability[SUBTERFUGE].value, percent);

    if ( ( !IS_NPC(ch) && (fail <= 0) )
            || ( !IS_NPC(ch) && !is_clan(ch)) )
    {
        /*
         * Failure.
         */
        send_to_char( "Oops.\n\r", ch );
        affect_strip(ch,gsn_sneak);
        REMOVE_BIT(ch->affected_by,AFF_SNEAK);

        act( "$n tried to steal from you.\n\r", ch, NULL, victim, TO_VICT, 0 );
        act( "$n tried to steal from $N.\n\r",  ch, NULL, victim, TO_NOTVICT,0);
        send_to_char(Format("%s is a thief!", ch->name), ch);

        if (!IS_AWAKE(victim))
            do_function(victim, &do_wake, "");
        if (IS_AWAKE(victim))
            do_function(victim, &do_yell, buf);
        if ( !IS_NPC(ch) )
        {
            if ( IS_NPC(victim) )
            {
                multi_hit( victim, ch, TYPE_UNDEFINED );
            }
            else
            {
                wiznet((char *)Format("\tY[WIZNET]\tn $N tried to steal from %s.", victim->name), ch,NULL,WIZ_FLAGS,0,0);
            }
        }
        if(!IS_SET(victim->act2, ACT2_NOWARRANT))
            add_warrant(ch, 3, TRUE);

        return;
    }

    if(!IS_SET(victim->act2, ACT2_NOWARRANT))
        add_warrant(ch, 3, FALSE);

    if ( !str_cmp( arg1, "coin"  )
            ||   !str_cmp( arg1, "coins" )
            ||   !str_cmp( arg1, "note" )
            ||   !str_cmp( arg1, "notes" )
            ||   !str_cmp( arg1, "bill" )
            ||   !str_cmp( arg1, "bills" )
            ||   !str_cmp( arg1, "dollars"  )
            ||   !str_cmp( arg1, "cents"))
    {
        int dollars, cents;

        dollars = victim->dollars * number_range(1, 60) / 60;
        cents = victim->cents * number_range(1, 60) / 60;
        if ( dollars <= 0 && cents <= 0 )
        {
            send_to_char( "You couldn't get anything.\n\r", ch );
            return;
        }

        ch->dollars     += dollars;
        ch->cents       += cents;
        victim->cents   -= cents;
        victim->dollars -= dollars;
        if (cents <= 0)
        {
            send_to_char(Format("Bingo!  You got %d dollars.\n\r", dollars), ch);
        }
        else if (dollars <= 0)
        {
            send_to_char(Format("Bingo!  You got %d cents.\n\r", cents), ch);
        }
        else
        {
            send_to_char(Format("Bingo!  You got %d cents and %d dollars.\n\r", cents,dollars), ch);
        }

        return;
    }

    if ( ( obj = get_obj_carry( victim, arg1, ch ) ) == NULL )
    {
        send_to_char( "You can't find it.\n\r", ch );
        return;
    }

    if ( !can_drop_obj( victim, obj ) )
    {
        send_to_char( "You can't pry it away.\n\r", ch );
        return;
    }

    if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
        send_to_char( "You have your hands full.\n\r", ch );
        return;
    }

    if ( ch->carry_weight + get_obj_weight( obj ) > can_carry_w( ch ) )
    {
        send_to_char( "You can't carry that much weight.\n\r", ch );
        return;
    }

    obj_from_char( obj );
    obj_to_char( obj, ch );
    act("You pocket $p.",ch,obj,NULL,TO_CHAR, 0);
    send_to_char( "Got it!\n\r", ch );
    return;
}


void do_takeforgot( CHAR_DATA *ch, char *argument )
{
    char arg1 [MIL]={'\0'};
    char arg2 [MIL]={'\0'};
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int percent = 0, fail = 0;

    CheckCH(ch);

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if(ch->auspice != auspice_lookup("ragabash") || ch->disc[DISC_AUSPICE] < 4)
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    if(ch->power_timer > 0)
    {
        send_to_char("Your powers have not rejuvenated yet.\n\r", ch);
        return;
    }

    if ( IS_NULLSTR(arg1) || IS_NULLSTR(arg2) )
    {
        send_to_char( "Steal what from whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( victim == ch )
    {
        send_to_char( "That's pointless.\n\r", ch );
        return;
    }

    WAIT_STATE( ch, skill_table[gsn_steal].beats );
    percent  = get_curr_stat(victim, STAT_DEX) + victim->ability[ALERTNESS].value;

    if (!IS_AWAKE(victim))
        percent -= 1;
    else if (!can_see(victim,ch))
        percent += 2;
    else
        percent += 3;

    if (percent > 10) percent = 10;
    if (percent < 6) percent = 6;

    fail = dice_rolls(ch, get_curr_stat(ch, STAT_DEX) + ch->ability[SUBTERFUGE].value, percent);

    ch->power_timer = 1;

    if ( ( !IS_NPC(ch) && (fail <= 0) )
            || ( !IS_NPC(ch) && !is_clan(ch)) )
    {
        send_to_char("You fail to retrieve anything.\n\r", ch);
        return;
    }

    if(!IS_SET(victim->act2, ACT2_NOWARRANT))
        add_warrant(ch, 3, FALSE);

    if ( !str_cmp( arg1, "coin"  )
            ||   !str_cmp( arg1, "coins" )
            ||   !str_cmp( arg1, "note" )
            ||   !str_cmp( arg1, "notes" )
            ||   !str_cmp( arg1, "bill" )
            ||   !str_cmp( arg1, "bills" )
            ||   !str_cmp( arg1, "dollars"  )
            ||   !str_cmp( arg1, "cents"))
    {
        int dollars, cents;

        dollars = victim->dollars * number_range(1, 60) / 60;
        cents = victim->cents * number_range(1, 60) / 60;
        if ( dollars <= 0 && cents <= 0 )
        {
            send_to_char( "You couldn't get anything.\n\r", ch );
            return;
        }

        ch->dollars     += dollars;
        ch->cents       += cents;
        victim->cents   -= cents;
        victim->dollars -= dollars;
        if (cents <= 0)
        {
            send_to_char( Format("Bingo!  You got %d dollars.\n\r", dollars), ch );
        }
        else if (dollars <= 0)
        {
            send_to_char( Format("Bingo!  You got %d cents.\n\r", cents), ch );
        }
        else
        {
            send_to_char( Format("Bingo!  You got %d cents and %d dollars.\n\r", cents,dollars), ch );
        }


        return;
    }

    if ( ( obj = get_obj_carry( victim, arg1, ch ) ) == NULL )
    {
        send_to_char( "You can't find it.\n\r", ch );
        return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
        send_to_char( "You can't pry it away.\n\r", ch );
        return;
    }

    if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
        send_to_char( "You have your hands full.\n\r", ch );
        return;
    }

    if ( ch->carry_weight + get_obj_weight( obj ) > can_carry_w( ch ) )
    {
        send_to_char( "You can't carry that much weight.\n\r", ch );
        return;
    }

    obj_from_char( obj );
    obj_to_char( obj, ch );
    act("You pocket $p.",ch,obj,NULL,TO_CHAR,0);
    send_to_char( "Got it!\n\r", ch );
    return;
}

/*
 * Shopping commands.
 */
CHAR_DATA *find_keeper( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *keeper;
    SHOP_DATA *pShop;

    pShop = NULL;

    if(IS_NULLSTR(argument))
    {
        send_to_char( "You must supply a valid shopkeeper.\n\r", ch );
        return NULL;
    }

    for ( keeper = ch->in_room->people; keeper; keeper = keeper->next_in_room )
    {
        if ( ( IS_NPC(keeper) && (pShop = keeper->pIndexData->pShop) != NULL )
                || ( !IS_NPC(keeper) && !str_cmp(keeper->profession, "Salesman") )
                || ( !IS_NPC(keeper) && (!str_cmp(keeper->profession, "Chef")
                        || !str_cmp(keeper->profession, "Maker")) && !str_cmp(ch->profession, "Salesman") ) )
            if(ch != keeper && keeper == get_char_room(ch, argument))
                break;
    }

    if(keeper == NULL)
    {
        send_to_char( "You can't do that here.\n\r", ch );
        return NULL;
    }

    if ( pShop == NULL && !(!str_cmp(keeper->profession,"Chef") || !str_cmp(keeper->profession,"Salesman") || !str_cmp(keeper->profession,"Maker")) )
    {
        send_to_char( "You can't do that here.\n\r", ch );
        return NULL;
    }

    if(str_cmp(ch->profession,"Salesman") && (!str_cmp(keeper->profession, "Chef") || !str_cmp(keeper->profession, "Maker")))
    {
        send_to_char( "You can't do that here.\n\r", ch );
        return NULL;
    }

    /*
     * Shop hours.
     */
    if(pShop)
    {
        if ( time_info.hour < pShop->open_hour )
        {
            do_function(keeper, &do_say, "Sorry, I am closed. Come back later.");
            return NULL;
        }

        if ( time_info.hour > pShop->close_hour )
        {
            do_function(keeper, &do_say, "Sorry, I am closed. Come back tomorrow.");
            return NULL;
        }
    }

    /*
     * Invisible or hidden people.
     */
    if ( !can_see( keeper, ch ) )
    {
        do_function(keeper, &do_say, "I don't trade with folks I can't see.");
        return NULL;
    }

    return keeper;
}

/* insert an object at the right spot for the keeper */
void obj_to_keeper( OBJ_DATA *obj, CHAR_DATA *ch )
{
    OBJ_DATA *t_obj, *t_obj_next;

    /* see if any duplicates are found */
    for (t_obj = ch->carrying; t_obj != NULL; t_obj = t_obj_next)
    {
        t_obj_next = t_obj->next_content;

        if (obj->pIndexData == t_obj->pIndexData
                &&  !str_cmp(obj->short_descr,t_obj->short_descr))
        {
            /* if this is an unlimited item, destroy the new one */
            if (IS_OBJ_STAT(t_obj,ITEM_INVENTORY))
            {
                extract_obj(obj);
                return;
            }
            obj->cost = t_obj->cost; /* keep it standard */
            break;
        }
    }

    if (t_obj == NULL)
    {
        obj->next_content = ch->carrying;
        ch->carrying = obj;
    }
    else
    {
        obj->next_content = t_obj->next_content;
        t_obj->next_content = obj;
    }

    obj->carried_by      = ch;
    obj->in_room         = NULL;
    obj->in_obj          = NULL;
    ch->carry_number    += get_obj_number( obj );
    ch->carry_weight    += get_obj_weight( obj );
}

/* get an object from a shopkeeper's list */
OBJ_DATA *get_obj_keeper( CHAR_DATA *ch, CHAR_DATA *keeper, char *argument )
{
    char arg[MIL]={'\0'};
    OBJ_DATA *obj;
    int number = 0;
    int count = 0;
 
    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = keeper->carrying; obj != NULL; obj = obj->next_content )
    {
        if (obj->wear_loc == WEAR_NONE
        &&  can_see_obj( keeper, obj )
    &&  can_see_obj(ch,obj)
        &&  is_name( arg, obj->name ) )
        {
            if ( ++count == number )
                return obj;
    
        /* skip other objects of the same name */
        while (obj->next_content != NULL
        && obj->pIndexData == obj->next_content->pIndexData
        && !str_cmp(obj->short_descr,obj->next_content->short_descr))
        obj = obj->next_content;
        }
    }
 
    return NULL;
}

int get_cost( CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy )
{
    SHOP_DATA *pShop = NULL;
    int cost = 0;

    if ( obj == NULL || (IS_NPC(keeper) && ( pShop = keeper->pIndexData->pShop ) == NULL) )
        return 0;

    if ( fBuy )
    {
        if(IS_NPC(keeper))
            cost = (obj->cost * pShop->profit_buy  / 100) + (obj->cost * tax_rate / 100);
        else
            cost = (obj->cost * keeper->markup  / 100) + (obj->cost * tax_rate / 100);
    }
    else
    {
        OBJ_DATA *obj2;
        int itype;

        cost = 0;
        for ( itype = 0; itype < MAX_TRADE; itype++ )
        {
            if ( obj->item_type == pShop->buy_type[itype] )
            {
                cost = (obj->cost * pShop->profit_sell / 100) + (obj->cost * tax_rate / 100);
                break;
            }
        }

        if (!IS_OBJ_STAT(obj,ITEM_SELL_EXTRACT))
            for ( obj2 = keeper->carrying; obj2; obj2 = obj2->next_content )
            {
                if ( obj->pIndexData == obj2->pIndexData
                        &&   !str_cmp(obj->short_descr,obj2->short_descr) )
                {
                    if (IS_OBJ_STAT(obj2,ITEM_INVENTORY))
                        cost /= 2;
                    else
                        cost = cost * 3 / 4;
                }
            }
    }

    return cost;
}


int SELLS(CHAR_DATA *ch, int material)
{
    int raw_flags = ch->pIndexData->pShop->raw_materials;
    int result = FALSE;

    if(!IS_NPC(ch)) return FALSE;

    if(IS_SET(raw_flags, MATERIAL_EDIBLE)
    && material_table[material].is_edible) result = TRUE;

    if(IS_SET(raw_flags, MATERIAL_POISON)
    && material_table[material].is_poison) result = TRUE;

    if(IS_SET(raw_flags, MATERIAL_METAL)
    && material_table[material].is_metal) result = TRUE;

    if(IS_SET(raw_flags, MATERIAL_EXPLOSIVE)
    && material_table[material].is_explosive) result = TRUE;

    if(IS_SET(raw_flags, MATERIAL_LIQUID)
    && material_table[material].is_liquid) result = TRUE;

    if(IS_SET(raw_flags, MATERIAL_DRUGS)
      && (material_table[material].high || material_table[material].trip))
    result = TRUE;

    if(IS_SET(raw_flags, MATERIAL_ALCOHOL)
    && material_table[material].drunk) result = TRUE;

    if(IS_SET(raw_flags, MATERIAL_FABRIC)
    && material_table[material].is_fabric) result = TRUE;

    return result;
}


void buy_raw_material(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *keeper;
    int amount = 0, cost = 0;
    OBJ_DATA *obj;
    char arg[MIL]={'\0'};
    int material = 0;

    argument = one_argument(argument, arg);
    if((material = material_lookup(arg)) < 0)
    {
    send_to_char("No such material.\n\r", ch);
    return;
    }

    one_argument(argument, arg);
    if(is_number(arg))
    {
    amount = atoi(arg);
    argument = one_argument(argument, arg);
    } else amount = 1;

    if ( ( keeper = find_keeper( ch, argument ) ) == NULL )
    {
    send_to_char("Syntax: buy raw <material> <keeper>\n\r",
        ch);
    return;
    }

    if(!SELLS(keeper, material))
    {
    send_to_char("The shopkeeper doesn't sell that.\n\r", ch);
    return;
    }

    cost = number_fuzzy(material_table[material].value * 100 * amount);
    cost += cost * tax_rate;

    if(cost > (ch->dollars * 100) + ch->cents)
    {
    send_to_char("You don't have enough cash to buy that.\n\r", ch);
    return;
    }

    if((obj = get_obj_carry( ch, (char *)Format("pounds %s", material_table[material].name), ch )) == NULL)
    {
        obj = create_object(get_obj_index(OBJ_VNUM_DUMMY));
        obj->cost = cost;
        obj->weight = amount;
        PURGE_DATA(obj->name);
        obj->name = str_dup(Format("%d pounds %s", obj->weight,material_table[material].name));
        PURGE_DATA(obj->short_descr);
        obj->short_descr = str_dup(Format("%d pounds of %s", obj->weight, material_table[material].name));
        PURGE_DATA(obj->description);
        obj->description = str_dup(Format("a quantity of a %s%s%s material sits here",
                material_table[material].colour, material_table[material].is_metal?", metallic":"", material_table[material].is_liquid?", liquid":""));
        PURGE_DATA(obj->full_desc);
        obj->full_desc = str_dup(Format("%d pounds of a %s%s%s material sits here",
                obj->weight, material_table[material].colour, material_table[material].is_metal?", metallic":"", material_table[material].is_liquid?", liquid":""));
        obj->wear_flags = ITEM_TAKE|ITEM_HOLD;
        obj_to_char(obj, ch);
    }
    else
    {
        obj->weight += amount;
        PURGE_DATA(obj->name);
        obj->name = str_dup(Format("%d pounds %s", obj->weight,material_table[material].name));
        PURGE_DATA(obj->short_descr);
        obj->short_descr = str_dup(Format("%d pounds of %s", obj->weight, material_table[material].name));
        PURGE_DATA(obj->description);
        obj->description = str_dup(Format("a quantity of a %s%s%s material sits here",
                material_table[material].colour, material_table[material].is_metal?", metallic":"", material_table[material].is_liquid?", liquid":""));
        PURGE_DATA(obj->full_desc);
        obj->full_desc = str_dup(Format("%d pounds of a %s%s%s material sits here",
                obj->weight, material_table[material].colour, material_table[material].is_metal?", metallic":"", material_table[material].is_liquid?", liquid":""));
    }
    act("$N sells you $p.", ch, obj, keeper, TO_CHAR, 0);
}


void do_buy( CHAR_DATA *ch, char *argument )
{
    int cost = 0;
    CHAR_DATA *keeper;
    OBJ_DATA *obj,*t_obj;
    char arg[MIL]={'\0'};
    char arg2[MIL]={'\0'};
    int number = 0, count = 1;
    EXTRA_DESCR_DATA *ed, *objed;

    CheckCH(ch);

    if ( IS_NULLSTR(argument) )
    {
        send_to_char( "Buy what?\n\r", ch );
        return;
    };

    argument = one_argument(argument, arg2);

    if(!str_cmp(arg2, "raw"))
    {
        buy_raw_material(ch, argument);
        return;
    }

    number = mult_argument(arg2,arg);

    if ( ( keeper = find_keeper( ch, argument ) ) == NULL )
    {
        send_to_char( "You can't buy something from someone who isn't here.\n\r", ch);
        return;
    }

    obj  = get_obj_keeper( ch,keeper, arg );
    cost = get_cost( keeper, obj, TRUE );

    if (number < 1 || number > 99)
    {
        act("$n tells you 'Get real!",keeper,NULL,ch,TO_VICT,0);
        return;
    }

    if ( cost <= 0 || !can_see_obj( ch, obj ) )
    {
        act( "$n tells you 'I don't sell that -- try 'list''.", keeper, NULL, ch, TO_VICT, 0 );
        ch->reply = keeper;
        return;
    }

    if (!IS_OBJ_STAT(obj,ITEM_INVENTORY))
    {
        for (t_obj = obj->next_content;
                count < number && t_obj != NULL;
                t_obj = t_obj->next_content)
        {
            if (t_obj->pIndexData == obj->pIndexData
                    &&  !str_cmp(t_obj->short_descr,obj->short_descr))
                count++;
            else
                break;
        }

        if (count < number)
        {
            act("$n tells you 'I don't have that many in stock.", keeper,NULL,ch,TO_VICT,0);
            ch->reply = keeper;
            return;
        }
    }

    if ( (ch->cents + ch->dollars * 100) < cost * number )
    {
        if (number > 1)
            act("$n tells you 'You can't afford to buy that many.", keeper,obj,ch,TO_VICT,0);
        else
            act( "$n tells you 'You can't afford to buy $p'.", keeper, obj, ch, TO_VICT, 0 );
        ch->reply = keeper;
        return;
    }

    if (ch->carry_number +  number * get_obj_number(obj) > can_carry_n(ch))
    {
        send_to_char( "You can't carry that many items.\n\r", ch );
        return;
    }

    if ( ch->carry_weight + number * get_obj_weight(obj) > can_carry_w(ch))
    {
        send_to_char( "You can't carry that much weight.\n\r", ch );
        return;
    }

    if (number > 1)
    {
        act(Format("$n buys $p[%d].",number),ch,obj,NULL,TO_ROOM,0);
        act(Format("You buy $p[%d] for $$%d.%.2d.",number, (cost * number)/100, (cost * number)%100),ch,obj,NULL,TO_CHAR,0);
    }
    else
    {
        act( "$n buys $p.", ch, obj, NULL, TO_ROOM, 0 );
        act( Format("You buy $p for $$%d.%.2d.", cost/100, cost%100), ch, obj, NULL, TO_CHAR, 0 );
    }
    deduct_cost(ch,cost * number);
    keeper->dollars += cost * number/100;
    keeper->cents += cost * number - (cost * number/100) * 100;

    for (count = 0; count < number; count++)
    {
        if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) && IS_NPC(keeper) )
        {
            t_obj = create_object( obj->pIndexData );
            if(IS_SET(t_obj->extra_flags, ITEM_INVENTORY))
                REMOVE_BIT(t_obj->extra_flags, ITEM_INVENTORY);
            if(IS_SET(keeper->act2, ACT2_NEWS_STAND))
            {
                PURGE_DATA(t_obj->name);
                t_obj->name = str_dup(obj->name);
                PURGE_DATA(t_obj->short_descr);
                t_obj->short_descr = str_dup(obj->short_descr);
                PURGE_DATA(t_obj->description);
                t_obj->description = str_dup(obj->description);

                t_obj->cost = obj->cost;

                for(objed=obj->extra_descr;objed;objed=objed->next)
                {
                    /* Set extra descriptions for articles */
                    ALLOC_DATA (ed, EXTRA_DESCR_DATA, 1);
                    ed->keyword         = str_dup(objed->keyword);
                    ed->description     = str_dup(objed->description);
                    ed->next            = t_obj->extra_descr;
                    t_obj->extra_descr = ed;
                }

                t_obj->wear_loc = -1;
                PURGE_DATA(t_obj->full_desc);
                t_obj->full_desc = str_dup(obj->full_desc);
            }
        }
        else
        {
            t_obj = obj;
            obj = obj->next_content;
            obj_from_char( t_obj );
            if(IS_SET(t_obj->extra_flags, ITEM_INVENTORY))
                REMOVE_BIT(t_obj->extra_flags, ITEM_INVENTORY);
        }

        if (t_obj->timer > 0 && !IS_OBJ_STAT(t_obj,ITEM_HAD_TIMER))
            t_obj->timer = 0;
        REMOVE_BIT(t_obj->extra_flags,ITEM_HAD_TIMER);
        obj_to_char( t_obj, ch );
        if (cost < t_obj->cost)
            t_obj->cost = cost;
    }
}


void do_list( CHAR_DATA *ch, char *argument )
{
    CheckCH(ch);
    {
        CHAR_DATA *keeper;
        OBJ_DATA *obj;
        int cost = 0,count = 0;
        char arg[MIL]={'\0'};
        bool found = FALSE;
        bool fRaw = FALSE;

        argument = one_argument(argument,arg);

        if(!str_cmp(arg, "raw"))
        {
            fRaw = TRUE;
            argument = one_argument(argument,arg);
        }

        if ( ( keeper = find_keeper( ch, arg ) ) == NULL )
            return;

        if(fRaw)
        {
            send_to_char("Materials for sale:\n\r", ch);
            count = 0;
            for(cost = 0; material_table[cost].name != NULL; cost++)
                if(SELLS(keeper, cost))
                {
                    count++;
                    send_to_char(Format("%20s", material_table[cost].name), ch);
                    if(count % 4 == 0) send_to_char("\n\r", ch);
                }
            if(count % 4 != 0) send_to_char("\n\r", ch);
        }
        else
        {
            for ( obj = keeper->carrying; obj; obj = obj->next_content )
            {
                if ( obj->wear_loc == WEAR_NONE
                        &&   can_see_obj( ch, obj )
                        &&   ( cost = get_cost( keeper, obj, TRUE ) ) > 0
                        &&   ( IS_NULLSTR(argument) ||  is_name(argument,obj->name) ))
                {
                    if ( !found )
                    {
                        found = TRUE;
                        send_to_char( "[Price       Location ] Item\n\r", ch );
                    }

                    if (IS_OBJ_STAT(obj,ITEM_INVENTORY))
                    {
                        send_to_char( Format("[$%5d.%.2d   %s] %s\n\r", cost/100,cost - (cost/100)*100, flag_string( wear_flags, obj->wear_flags ), obj->short_descr), ch );
                    }
                    else if(IS_NPC(keeper))
                    {
                        count = 1;

                        while (obj->next_content != NULL
                                && obj->pIndexData == obj->next_content->pIndexData
                                && !str_cmp(obj->short_descr,
                                        obj->next_content->short_descr))
                        {
                            obj = obj->next_content;
                            count++;
                        }
                        send_to_char( Format("[$%5d.%.2d  %2d ] %s\n\r", cost/100,cost - (cost/100)*100,count,obj->short_descr), ch );
                    }
                }
            }
        }

        if ( !found )
            send_to_char( "You can't buy anything here.\n\r", ch );
        return;
    }
}


void do_sell( CHAR_DATA *ch, char *argument )
{
    char arg[MIL]={'\0'};
    CHAR_DATA *keeper;
    OBJ_DATA *obj;
    int cost = 0,roll = 0;

    CheckCH(ch);

    argument = one_argument( argument, arg );

    if ( IS_NULLSTR(arg) )
    {
        send_to_char( "Sell what?\n\r", ch );
        return;
    }

    if ( ( keeper = find_keeper( ch, argument ) ) == NULL )
        return;

    if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
    {
        act( "$n tells you 'You don't have that item'.", keeper, NULL, ch, TO_VICT, 0 );
        ch->reply = keeper;
        return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
        send_to_char( "You can't let go of it.\n\r", ch );
        return;
    }

    if (!IS_NPC(keeper))
    {
        send_to_char( "You can't force a player to buy something from you.\n\r", ch);
        return;
    }

    if (!can_see_obj(keeper,obj))
    {
        act("$n doesn't see what you are offering.",keeper,NULL,ch,TO_VICT,0);
        return;
    }

    if ( ( cost = get_cost( keeper, obj, FALSE ) ) <= 0 )
    {
        act( "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT, 0 );
        return;
    }
    if ( cost > (keeper-> cents + 100 * keeper->dollars) )
    {
        act("$n tells you 'I'm afraid I don't have enough wealth to buy $p.", keeper,obj,ch,TO_VICT,0);
        return;
    }

    act( "$n sells $p.", ch, obj, NULL, TO_ROOM, 0 );
    /* haggle */
    roll = number_percent();
    if (!IS_OBJ_STAT(obj,ITEM_SELL_EXTRACT) && roll < get_skill(ch,gsn_haggle))
    {
        send_to_char("You haggle with the shopkeeper.\n\r",ch);
        cost += obj->cost / 2 * roll / 100;
        cost = UMIN(cost,95 * get_cost(keeper,obj,TRUE) / 100);
        cost = UMIN(cost,(keeper->cents + 100 * keeper->dollars));
    }
    act( Format("You sell $p for $$%d.%.2d.", cost/100, cost - (cost/100) * 100), ch, obj, NULL, TO_CHAR, 0 );
    ch->dollars     += cost/100;
    ch->cents    += cost - (cost/100) * 100;
    deduct_cost(keeper,cost);
    if ( keeper->dollars < 0 )
        keeper->dollars = 0;
    if ( keeper->cents< 0)
        keeper->cents = 0;

    if( IS_SET(obj->extra_flags, ITEM_INVENTORY) )
        REMOVE_BIT(obj->extra_flags, ITEM_INVENTORY);
    if ( obj->item_type == ITEM_TRASH || IS_OBJ_STAT(obj,ITEM_SELL_EXTRACT))
    {
        extract_obj( obj );
    }
    else
    {
        obj_from_char( obj );
        if (obj->timer)
            SET_BIT(obj->extra_flags,ITEM_HAD_TIMER);
        else
            obj->timer = number_range(50,100);
        obj_to_keeper( obj, keeper );
    }

    return;
}


void do_value( CHAR_DATA *ch, char *argument )
{
    char arg[MIL]={'\0'};
    CHAR_DATA *keeper;
    OBJ_DATA *obj;
    int cost = 0;

    CheckCH(ch);

    argument = one_argument( argument, arg );

    if ( IS_NULLSTR(arg) )
    {
        send_to_char( "Value what?\n\r", ch );
        return;
    }

    if ( ( keeper = find_keeper( ch, argument ) ) == NULL )
        return;

    if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
    {
        act( "$n tells you 'You don't have that item'.", keeper, NULL, ch, TO_VICT, 0 );
        ch->reply = keeper;
        return;
    }

    if (!can_see_obj(keeper,obj))
    {
        act("$n doesn't see what you are offering.",keeper,NULL,ch,TO_VICT,0);
        return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
        send_to_char( "You can't let go of it.\n\r", ch );
        return;
    }

    if ( ( cost = get_cost( keeper, obj, FALSE ) ) <= 0 )
    {
        act( "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT, 0 );
        return;
    }

    act( Format("$n tells you 'I'll give you %d cents and %d dollars for $p'.", cost - (cost/100) * 100, cost/100), keeper, obj, ch, TO_VICT, 0 );
    ch->reply = keeper;

    return;
}

void do_conceal (CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    char arg[MSL]={'\0'};
    int fail = 0;

    CheckCH(ch);

    one_argument(argument, arg);
    if((obj = get_obj_here(ch, arg)) == NULL)
    {
        send_to_char("You can't see that to be able to hide it.\n\r", ch);
        return;
    }

    if(obj->carried_by != ch)
    {
        send_to_char("You aren't carrying that.\n\r", ch);
        return;
    }

    fail = dice_rolls(ch, get_curr_stat(ch, STAT_INT) + ch->ability[SUBTERFUGE].value, 7);

    if(fail > 0)
    {
        send_to_char(Format("You attempt to conceal %s", obj->short_descr), ch);

        if ( !can_drop_obj( ch, obj ) )
        {
            send_to_char( ", but you can't let go of it.\n\r", ch );
            return;
        }
        else
            send_to_char( ".\n\r", ch );

        obj_from_char( obj );
        obj_to_room( obj, ch->in_room );
        SET_BIT(obj->extra_flags, ITEM_HIDDEN);
        if (IS_OBJ_STAT(obj,ITEM_MELT_DROP))
        {
            act("$p dissolves into smoke.",ch,obj,NULL,TO_ROOM,0);
            act("$p dissolves into smoke.",ch,obj,NULL,TO_CHAR,0);
            extract_obj(obj);
        }
    }
    else
    {
        send_to_char(Format("You attempt to conceal %s.\n\r", obj->short_descr), ch);
        do_function(ch, &do_drop, argument);
    }
}

void do_shoplift( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    OBJ_DATA *t_obj;
    EXTRA_DESCR_DATA *ed, *objed;
    int percent = 0, fail = 0;
    char arg1 [MIL]={'\0'};
    char arg2 [MIL]={'\0'};

    CheckCH(ch);

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if(IS_SET(ch->plr_flags, PLR_RP_OK))
    {
        send_to_char("For your own safety, you can't shoplift in RP mode.\n\r", ch);
        return;
    }

    if ( IS_NULLSTR(arg1) || IS_NULLSTR(arg2) )
    {
        send_to_char( "Steal what from whom?\n\r", ch );
        return;
    }

    if ( ( victim = find_keeper( ch, arg2 ) ) == NULL )
    {
        send_to_char( "There's no shop here.\n\r", ch );
        return;
    }

    if ( victim == ch )
    {
        send_to_char( "That's pointless.\n\r", ch );
        return;
    }

    WAIT_STATE( ch, skill_table[gsn_steal].beats );
    percent  = get_curr_stat(victim, STAT_DEX) + victim->ability[ALERTNESS].value;

    if (!IS_AWAKE(victim))
        percent -= 1;
    else if (!can_see(victim,ch))
        percent += 2;
    else
        percent += 3;

    if (percent > 10) percent = 10;
    if (percent < 6) percent = 6;

    fail = dice_rolls(ch, get_curr_stat(ch, STAT_DEX) + ch->ability[SUBTERFUGE].value, percent);

    if ( ( !IS_NPC(ch) && (fail <= 0) )
            || ( !IS_NPC(ch) && !is_clan(ch)) )
    {
        /*
         * Failure.
         */
        send_to_char( "Oops.\n\r", ch );
        affect_strip(ch,gsn_sneak);
        REMOVE_BIT(ch->affected_by,AFF_SNEAK);

        act( "$n tried to steal from you.\n\r", ch, NULL, victim, TO_VICT, 0 );
        act( "$n tried to steal from $N.\n\r",  ch, NULL, victim, TO_NOTVICT,0);

        if (!IS_AWAKE(victim))
            do_function(victim, &do_wake, "");
        if (IS_AWAKE(victim) && IS_NPC(victim))
        {
            do_function(victim, &do_yell, (char *)Format("%s is a thief!", ch->name));
        }
        if ( !IS_NPC(ch) )
        {
            if ( IS_NPC(victim) )
            {
                multi_hit( victim, ch, TYPE_UNDEFINED );
            }
            else
            {
                wiznet((char *)Format("\tY[WIZNET]\tn $N tried to steal from %s.",victim->name),ch,NULL,WIZ_FLAGS,0,0);
            }
        }
        if(!IS_SET(victim->act2, ACT2_NOWARRANT))
            add_warrant(ch, 3, TRUE);

        return;
    }

    if(!IS_SET(victim->act2, ACT2_NOWARRANT))
        add_warrant(ch, 3, FALSE);

    if ( ( obj = get_obj_carry( victim, arg1, ch ) ) == NULL )
    {
        send_to_char( "You can't find it.\n\r", ch );
        return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
        send_to_char( "You can't pry it away.\n\r", ch );
        return;
    }

    if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
        send_to_char( "You have your hands full.\n\r", ch );
        return;
    }

    if ( ch->carry_weight + get_obj_weight( obj ) > can_carry_w( ch ) )
    {
        send_to_char( "You can't carry that much weight.\n\r", ch );
        return;
    }

    if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) && IS_NPC(victim) )
    {
        t_obj = create_object( obj->pIndexData );
        if(IS_SET(t_obj->extra_flags, ITEM_INVENTORY))
            REMOVE_BIT(t_obj->extra_flags, ITEM_INVENTORY);
        if(IS_SET(victim->act2, ACT2_NEWS_STAND))
        {
            PURGE_DATA(t_obj->name);
            t_obj->name = str_dup(obj->name);
            PURGE_DATA(t_obj->short_descr);
            t_obj->short_descr = str_dup(obj->short_descr);
            PURGE_DATA(t_obj->description);
            t_obj->description = str_dup(obj->description);

            t_obj->cost = obj->cost;

            for(objed=obj->extra_descr;objed;objed=objed->next)
            {
                /* Set extra descriptions for articles */
                ALLOC_DATA (ed, EXTRA_DESCR_DATA, 1);
                ed->keyword         = str_dup(objed->keyword);
                ed->description     = str_dup(objed->description);
                ed->next            = t_obj->extra_descr;
                t_obj->extra_descr = ed;
            }

            t_obj->wear_loc = -1;
            PURGE_DATA(t_obj->full_desc);
            t_obj->full_desc = str_dup(obj->full_desc);
        }
    }
    else
    {
        t_obj = obj;
        obj = obj->next_content;
        obj_from_char( t_obj );
        if(IS_SET(t_obj->extra_flags, ITEM_INVENTORY))
            REMOVE_BIT(t_obj->extra_flags, ITEM_INVENTORY);
    }

    if (t_obj->timer > 0 && !IS_OBJ_STAT(t_obj,ITEM_HAD_TIMER))
        t_obj->timer = 0;
    REMOVE_BIT(t_obj->extra_flags,ITEM_HAD_TIMER);
    obj_to_char( t_obj, ch );

    act("You pocket $p.",ch,obj,NULL,TO_CHAR,0);
    send_to_char( "Got it!\n\r", ch );
    return;
}


void do_search (CHAR_DATA *ch, char *argument)
{
    EXIT_DATA *pex;
    OBJ_DATA *pObj;
    int fail = 0;
    bool found = FALSE;
    int door = 0;

    CheckCH(ch);

    fail = dice_rolls(ch, get_curr_stat(ch, STAT_PER) + ch->ability[INVESTIGATION].value, 8);

    if(fail <= 0)
    {
        send_to_char("You fail to find anything hidden in the room.\n\r", ch);
        return;
    }
    else
    {
        send_to_char("You find:\n\r", ch);

        for(pObj = ch->in_room->contents; pObj != NULL; pObj = pObj->next_content)
        {
            if(IS_SET(pObj->extra_flags, ITEM_HIDDEN))
            {
                send_to_char(Format("%s\n\r", pObj->short_descr),ch);
                found = TRUE;
            }
        }

        for(door = 0; door < MAX_EXITS; door++)
        {
            pex = ch->in_room->exit[door];
            if(pex)
            {
                if(IS_SET(pex->exit_info, EX_HIDDEN))
                {
                    send_to_char(Format("There seems to be an exit to the %s.\n\r", dir_name[door]), ch);
                    found = TRUE;
                }
            }
        }

        if(!found)
        {
            send_to_char("You fail to find anything hidden in the room.\n\r", ch);
        }

        return;
    }
}

void do_reload(CHAR_DATA *ch, char *argument)
{
    char arg[MIL]={'\0'};
    OBJ_DATA *weapon, *ammo;

    CheckCH(ch);

    argument = one_argument(argument, arg);

    if(IS_NULLSTR(argument))
    {
        send_to_char("Which firearm are you trying to reload?\n\r", ch);
        return;
    }

    if((weapon = get_obj_carry( ch, arg, ch )) == NULL)
        if((weapon= get_eq_char(ch, WEAR_WIELD)) == NULL)
        {
            send_to_char("You don't have that weapon.\n\r", ch);
            return;
        }

    if((ammo = get_obj_carry(ch, argument, ch)) == NULL)
    {
        send_to_char("You don't have that ammunition.\n\r", ch);
        return;
    }

    if(weapon->value[5] != ammo->value[5])
    {
        send_to_char("The ammo doesn't fit.\n\r", ch);
        return;
    }

    if(ammo->value[0] + weapon->value[3] > weapon->value[4])
    {
        send_to_char("There isn't enough space for that much ammo.\n\r", ch);
        return;
    }

    weapon->value[2] = ammo->pIndexData->vnum;
    weapon->value[3] = weapon->value[3] + ammo->value[0];
    act("You load $T into $t.", ch, weapon->short_descr, ammo->short_descr, TO_CHAR,0);
    act("$n loads $T into $t.", ch, weapon->short_descr, ammo->short_descr, TO_ROOM,0);
    extract_obj(ammo);
}

void do_unload(CHAR_DATA *ch, char *argument)
{
    char arg[MIL]={'\0'};
    OBJ_DATA *weapon, *ammo;

    CheckCH(ch);

    if(IS_NULLSTR(argument))
    {
        send_to_char("Which firearm are you trying to unload?\n\r", ch);
        return;
    }

    argument = one_argument(argument, arg);

    if((weapon = get_obj_carry( ch, arg, ch )) == NULL)
        if((weapon= get_eq_char(ch, WEAR_WIELD)) == NULL)
        {
            send_to_char("You don't have that weapon.\n\r", ch);
            return;
        }

    if(weapon->value[3] == 0)
    {
        send_to_char("It isn't loaded.\n\r", ch);
        return;
    }

    if((ammo = create_object(get_obj_index(weapon->value[2]))) == NULL)
    {
        send_to_char("You can't seem to unload it.\n\r", ch);
        return;
    }
    ammo->value[0] = weapon->value[3];
    weapon->value[3] = 0;
    act("You unload $T from $t.", ch, weapon->short_descr, ammo->short_descr, TO_CHAR, 0);
    act("$n unloads $T from $t.", ch, weapon->short_descr, ammo->short_descr, TO_ROOM, 0);
    obj_to_char(ammo, ch);
}

void do_repair(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    int fail = 0;

    CheckCH(ch);

    if((obj = get_obj_carry(ch, argument, ch)) == NULL)
    {
        send_to_char("You don't have that item.\n\r", ch);
        return;
    }

    if(obj->condition > 90)
    {
        send_to_char("It doesn't seem to need any repairs.\n\r", ch);
        return;
    }

    fail = dice_rolls(ch, get_curr_stat(ch, STAT_DEX) + ch->ability[REPAIR].value, 7);

    if(fail > 0)
    {
        obj->condition += UMIN(100 - obj->condition, 10 * fail);
        send_to_char("You do some repairs and it looks better for it.\n\r", ch);
        return;
    }
    else if(fail < 0)
    {
        send_to_char("Ooops... I think you broke it even more.\n\r", ch);
        obj->condition += 10 * fail;
        return;
    }

    send_to_char("Well... that didn't seem to work...\n\r", ch);
}

void do_activate(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    char arg[MIL]={'\0'};
    int spirit = 0;
    bool n_r = FALSE;

    CheckCH(ch);

    argument = one_argument(argument, arg);
    if(IS_NULLSTR(arg) || IS_NULLSTR(argument))
    {
        send_to_char("What are you trying to activate?\n\r", ch);
        return;
    }

    if((obj = get_obj_wear(ch, arg)) == NULL)
    {
        send_to_char("What are you trying to activate?\n\r", ch);
        return;
    }

    argument = one_argument(argument, arg);
    if((spirit = flag_lookup(arg, spirit_table)) < 0)
    {
        send_to_char("What spirit are you trying to awaken?\n\r", ch);
        return;
    }

    if(IS_SET(obj->fetish_flags, spirit))
    {
        send_to_char("There is no such spirit in the fetish.\n\r", ch);
        return;
    }

    one_argument(argument, arg);
    if(!str_cmp(arg,"force") || !str_cmp(arg,"power") || !str_cmp(arg, "feed"))
    {
        send_to_char(Format("You provide your gnosis to the %s spirit in your %s.\n\r", spirit_table[spirit].name, obj->short_descr), ch);
        n_r = TRUE;
    }

    if((*spirit_power_table[spirit].func) (ch,argument,n_r,obj))
        WAIT_STATE(ch, obj->fetish_level);
}

void do_loot(CHAR_DATA *ch, char *argument)
{
    CheckCH(ch);
    do_function(ch, &do_get, (char *)Format("all %s", argument));
}

void do_trade(CHAR_DATA *ch, char *argument)
{
    STOCKS *st;
    char buf[MSL]={'\0'};
    bool Buy = FALSE;

    CheckCH(ch);

    if(!IS_SET(ch->in_room->room_flags, ROOM_BANK))
    {
        send_to_char("You need to trade stocks at a bank.\n\r", ch);
        return;
    }

    if(IS_NULLSTR(argument))
    {
        send_to_char("\tCSyntax: influence trade [buy/sell] [shares] [company]\tn\n\r", ch);
        return;
    }

    argument = one_argument(argument, buf);

    if(!str_prefix(buf, "buy"))
        Buy = TRUE;
    else if(str_prefix(buf, "sell"))
    {
        send_to_char("\tCSyntax: influence trade [buy/sell] [shares] [company]\tn\n\r", ch);
        return;
    }

    argument = one_argument(argument, buf);
    if((st = stock_lookup(argument)) == NULL)
    {
        send_to_char("There is no such company.\n\r", ch);
        return;
    }

    if(IS_NULLSTR(buf) || !is_number(buf))
    {
        send_to_char("How many shares do you want to buy?\n\r", ch);
        return;
    }

    trade_stocks(ch, st, atoi(buf), TRUE, Buy);

    return;
}

void do_deposit(CHAR_DATA *ch, char *argument)
{
    char *centstr;
    int cents = 0;
    int dollars = 0;

    CheckCH(ch);

    if(!IS_SET(ch->in_room->room_flags, ROOM_BANK))
    {
        send_to_char("You need to deposit funds at a bank.\n\r", ch);
        return;
    }

    if(IS_NULLSTR(argument))
    {
        send_to_char("\tCSyntax: deposit [money]\tn\n\r", ch);
        return;
    }

    if(!(centstr = strstr(argument, ".")))
        if(!is_number(argument))
        {
            send_to_char("You have to deposit an amount.", ch);
            return;
        }

    if(centstr)
    {
        centstr[0] = '\0';
        centstr++;
        cents = atoi(centstr);
    }

    dollars = atoi(argument);

    if(cents < 0 || dollars < 0)
    {
        send_to_char("You cannot deposit a negative amount.\n\r", ch);
        return;
    }

    if((ch->dollars * 100) + ch->cents < (dollars * 100) + cents)
    {
        send_to_char("You don't have that much to deposit.\n\r", ch);
        return;
    }

    ch->bank_account += (dollars * 100) + cents;
    ch->dollars -= dollars;
    ch->dollars -= cents;

    send_to_char(Format("You deposit $%d.%.2d into your account.\n\r", dollars, cents), ch);
    do_function(ch, &do_balance, "");
}

void do_withdrawal(CHAR_DATA *ch, char *argument)
{
    char *centstr;
    int cents = 0;
    int dollars = 0;

    CheckCH(ch);

    if(!IS_SET(ch->in_room->room_flags, ROOM_BANK))
    {
        send_to_char("You need to withdraw funds at a bank.\n\r", ch);
        return;
    }

    if(IS_NULLSTR(argument))
    {
        send_to_char("\tCSyntax: withdraw [money]\tn\n\r", ch);
        return;
    }

    if(!(centstr = strstr(argument, ".")))
        if(!is_number(argument))
        {
            send_to_char("You have to deposit an amount.", ch);
            return;
        }

    if(centstr)
    {
        centstr[0] = '\0';
        centstr++;
        cents = atoi(centstr);
    }

    dollars = atoi(argument);

    if(cents < 0 || dollars < 0)
    {
        send_to_char("You cannot withdraw a negative amount.\n\r", ch);
        return;
    }

    if(ch->bank_account < (dollars * 100) + cents)
    {
        send_to_char("You don't have that much in the bank.\n\r", ch);
        return;
    }

    ch->bank_account -= (dollars * 100) + cents;
    ch->dollars += dollars;
    ch->dollars += cents;

    send_to_char(Format("You withdraw $%d.%.2d into your account.\n\r", dollars,cents), ch);
    do_function(ch, &do_balance, "");
}

void do_balance(CHAR_DATA *ch, char *argument)
{
    CheckCH(ch);

    if(!IS_SET(ch->in_room->room_flags, ROOM_BANK))
    {
        send_to_char("You need to check your balance at a bank.\n\r", ch);
        return;
    }

    send_to_char(Format("Your balance comes to $%ld.%.2ld.\n\r", ch->bank_account/100, ch->bank_account%100), ch);
}

void do_wrap(CHAR_DATA *ch, char *argument)
{
    char arg[MIL]={'\0'};
    OBJ_DATA *obj;

    CheckCH(ch);

    argument = one_argument( argument, arg );

    if ( IS_NULLSTR(arg) )
    {
        send_to_char( "Wrap what?\n\r", ch );
        return;
    }

    if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
    {
        send_to_char( "You do not have that item.\n\r", ch );
        return;
    }

    if(IS_SET(obj->extra2, OBJ_PACKAGED))
    {
        send_to_char("It's already wrapped up!\n\r", ch);
        return;
    }

    SET_BIT(obj->extra2, OBJ_PACKAGED);
    act( "$n wraps $p.", ch, obj, NULL, TO_ROOM, 0 );
    act( "You wrap $p.", ch, obj, NULL, TO_CHAR, 0 );
}

void do_unwrap(CHAR_DATA *ch, char *argument)
{
    char arg[MIL]={'\0'};
    OBJ_DATA *obj;

    CheckCH(ch);

    argument = one_argument( argument, arg );

    if ( IS_NULLSTR(arg) )
    {
        send_to_char( "Unwrap what?\n\r", ch );
        return;
    }

    if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
    {
        send_to_char( "You do not have that item.\n\r", ch );
        return;
    }

    if(!IS_SET(obj->extra2, OBJ_PACKAGED))
    {
        send_to_char("That isn't wrapped up!\n\r", ch);
        return;
    }

    REMOVE_BIT(obj->extra2, OBJ_PACKAGED);
    act( "$n unwraps $p.", ch, obj, NULL, TO_ROOM, 0 );
    act( "You unwrap $p.", ch, obj, NULL, TO_CHAR, 0 );
}

void do_pullpin(CHAR_DATA *ch, char *argument)
{
    char arg[MIL]={'\0'};
    OBJ_DATA *obj;

    CheckCH(ch);

    argument = one_argument( argument, arg );

    if ( IS_NULLSTR(arg) )
    {
        send_to_char( "What are you trying to pull the pin on?\n\r", ch );
        return;
    }

    if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
    {
        send_to_char( "You do not have that item.\n\r", ch );
        return;
    }

    if(obj->item_type != ITEM_BOMB)
    {
        send_to_char("That isn't a bomb!\n\r", ch);
        return;
    }

    if(obj->value[1] != 0)
    {
        send_to_char("The time is already slipping away!\n\r", ch);
        return;
    }

    obj->value[1] = 1;
    act( "$n pulls the pin on $p.", ch, obj, NULL, TO_ROOM, 0 );
    act( "You pull the pin on $p.", ch, obj, NULL, TO_CHAR, 0 );
}

void do_defuse(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    char arg[MIL]={'\0'};
    int fail = 0;

    CheckCH(ch);

    argument = one_argument( argument, arg );

    if ( IS_NULLSTR(arg) )
    {
        send_to_char( "Defuse what?\n\r", ch );
        return;
    }

    if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
    {
        send_to_char( "You do not have that item.\n\r", ch );
        return;
    }

    if(obj->item_type != ITEM_BOMB)
    {
        send_to_char("That isn't a bomb!\n\r", ch);
        return;
    }

    if(obj->value[1] == 0)
    {
        send_to_char("The time is not counting down!\n\r", ch);
        return;
    }

    fail = dice_rolls(ch, get_curr_stat(ch, STAT_DEX), 7);
    fail = UMIN(dice_rolls(ch, get_curr_stat(ch, STAT_INT), 7), fail);
    fail = fail + dice_rolls(ch, ch->ability[SCIENCE].value, 7);
    WAIT_STATE(ch, 4);
    if(fail == 0)
    {
        send_to_char("You don't manage to defuse it.\n\r", ch);
        return;
    }
    else if(fail < 0)
    {
        send_to_char("Uh-oh...\n\r", ch);
        obj->value[0] = 0;
        return;
    }

    act( "$n defuses $p.", ch, obj, NULL, TO_ROOM, 0 );
    act( "You defuse $p.", ch, obj, NULL, TO_CHAR, 0 );
    obj->value[1] = 0;
}

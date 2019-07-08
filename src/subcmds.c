#if defined(Macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <time.h>
#include <stdlib.h>
#include "twilight.h"
#include "tables.h"
#include "subcmds.h"
#include "recycle.h"
#include "lookup.h"
#include "olc.h"

/*
 * Functions in act_obj.c
 */
#define CD CHAR_DATA
#define OD OBJ_DATA
bool    remove_obj      args( (CHAR_DATA *ch, int iWear, bool fReplace ) );
CD *    find_keeper     args( (CHAR_DATA *ch, char *argument ) );
int     get_cost        args( (CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy ) );
void    obj_to_keeper   args( (OBJ_DATA *obj, CHAR_DATA *ch ) );
OD *    get_obj_keeper  args( (CHAR_DATA *ch,CHAR_DATA *keeper,char *argument));
void save_notes			args( (int type) );

/*
 * Functions in act_move.c
 */
int  find_door       args( ( CHAR_DATA *ch, char *arg ) );
#undef OD
#undef  CD

DECLARE_DO_FUN(do_flee);
DECLARE_DO_FUN(do_look);

/*
 * Functions in olc.c
 */
void parse_note( CHAR_DATA *ch, char *argument, int type );
void save_papers();
void save_stocks();
bool is_note_to( CHAR_DATA *ch, NOTE_DATA *pnote );
NOTE_DATA * find_knowledge_keyword args( (char * arg) );

bool is_nominee(CHAR_DATA *ch, int office);
void fwrite_votes();
bool trade_stocks(CHAR_DATA *ch, STOCKS *st, int num, bool fees, bool buy);

int get_door(CHAR_DATA *ch, char *argument);

/* >>>>>>>>>>>>>>> Determine if they can be something <<<<<<<<<<<<<<< */
bool CAN_BE(CHAR_DATA *ch, int cmd)
{
	if(job_table[cmd].available == -1)
		return FALSE;

	if(job_table[cmd].available == 0)
		return TRUE;

	if(ch->race == race_lookup("human") && IS_SET(job_table[cmd].available, HU))
		return TRUE;

	if(ch->race == race_lookup("werewolf")
			&& IS_SET(job_table[cmd].available, WW))
		return TRUE;

	if(ch->race == race_lookup("vampire")
			&& IS_SET(job_table[cmd].available, VA))
		return TRUE;

	return FALSE;
}

/* >>>>>>>>>>>>>>> Maker Commands <<<<<<<<<<<<<<< */
int maker_creation(CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *pObj;
	char arg[MIL]={'\0'};
	char arg1[MIL]={'\0'};
	char arg2[MIL]={'\0'};
	char arg3[MIL]={'\0'};
	int material = 0, wear = 0, size = 0;
	int jn = 0;
	int cond = 0;
	int fail = 0;
	int fail1 = 0;
	int fail2 = 0;

	smash_tilde( argument );
	argument = one_argument(argument, arg);
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
	strncpy( arg3, argument, sizeof(arg3) );

	if(IS_NULLSTR(arg) || IS_NULLSTR(arg1) || IS_NULLSTR(arg2) || IS_NULLSTR(arg3))
	{
		send_to_char("Syntax: job make [size] [material] [worn] [name]\n\r",ch);
		return FALSE;
	}

	if(ch->markup == NO_FLAG)
	{
		send_to_char("You need to choose an item type specialty!\n\r", ch);
		return FALSE;
	}

	if((size = size_lookup(arg)) == -1)
	{
		send_to_char("That size does not exist.\n\r", ch);
		return FALSE;
	}

	if((material = material_lookup(arg1)) == -1)
	{
		send_to_char("That material does not exist.\n\r", ch);
		return FALSE;
	}

	if(material_table[material].is_edible && ch->markup != ITEM_FOOD)
	{
		send_to_char("You're no chef, try working with something inedible.\n\r", ch);
		return FALSE;
	}
	else if(!material_table[material].is_edible && ch->markup == ITEM_FOOD)
	{
		send_to_char("You're a chef, work with food!\n\r", ch);
		return FALSE;
	}

	if((pObj = get_obj_carry( ch, material_table[material].name, ch)) == NULL)
	{
		send_to_char("You aren't carrying any of that material to work with.\n\r", ch);
		return FALSE;
	}

	if(!str_cmp(material_table[material].name, pObj->material))
	{
		act(Format("$p is not made of %s.\n\r", material_table[material].name), ch, pObj, NULL, TO_CHAR, 1);
		return FALSE;
	}

	if(pObj->pIndexData->vnum != OBJ_VNUM_DUMMY)
	{
		act("$p is not a raw material.\n\r", ch, pObj, NULL, TO_CHAR, 1);
		return FALSE;
	}

	if(pObj->weight * material_table[material].weight
			< size_table[size].obj_weight)
	{
		send_to_char("You don't have enough of that material to make an object of that size.\n\r", ch);
		return FALSE;
	}

	if(( wear = flag_value( wear_flags, arg2 ) ) == NO_FLAG)
	{
		send_to_char("Where is this item to be worn?\n\r", ch);
		return FALSE;
	}

	jn = job_lookup(ch->profession);

	fail1 = dice_rolls(ch, get_curr_stat(ch, STAT_INT) + ch->ability[job_table[jn].abil].value, 7);
	fail2 = dice_rolls(ch, get_curr_stat(ch, STAT_DEX) + ch->ability[job_table[jn].abil].value, 7);

	fail = UMAX(fail1, fail2);

	/* @@@@@    fail += ch->profskill */
	if(fail <= 0)
	{
		send_to_char("You make a mess of it all and will have to start over.\n\r", ch);
		if(fail < 0)
		{
			extract_obj(pObj);
			send_to_char("In the mess, you destroy the material you were working with.\n\r", ch);
		}

		return TRUE;
	}

	if(!cond)
	{
		cond = UMAX(fail * 10, 100);
	}

	PURGE_DATA(pObj->name);
	pObj->name = str_dup(arg3);
	PURGE_DATA(pObj->short_descr);
	pObj->short_descr = str_dup(arg3);
	PURGE_DATA(pObj->description);
	pObj->description = str_dup("A shapeless item rests here.");
	PURGE_DATA(pObj->full_desc);
	pObj->full_desc = str_dup("A shapeless item rests here.");

	pObj->item_type = ch->markup;
	if(ch->markup == ITEM_FURNITURE || ch->markup == ITEM_FOUNTAIN)
		pObj->wear_flags = 0;
	else
		pObj->wear_flags = wear|ITEM_TAKE;
	pObj->cost *= QUALITY_OF_WORK(fail);
	pObj->condition = cond + (QUALITY_OF_WORK(fail) * 5);
	pObj->quality = QUALITY_OF_WORK(fail);

	return TRUE;
}

int maker_desc(CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *pObj;
	char arg[MIL]={'\0'};
	char arg2[MIL]={'\0'};

	smash_tilde( argument );
	one_argument( argument, arg );
	strncpy( arg2, argument, sizeof(arg2) );

	if ( IS_NULLSTR(arg) )
	{
		send_to_char( "Write a description for what?\n\r", ch );
		return FALSE;
	}

	if ( ( pObj = get_obj_carry( ch, arg, ch ) ) == NULL )
	{
		send_to_char("You don't have that.\n\r", ch);
		return FALSE;
	}

	string_append(ch, &pObj->full_desc);
	return TRUE;
}

int maker_long_desc(CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *pObj;
	char arg[MIL]={'\0'};
	char arg2[MIL]={'\0'};

	smash_tilde( argument );
	argument = one_argument( argument, arg );
	strncpy( arg2, argument, sizeof(arg2) );

	if ( IS_NULLSTR(arg) )
	{
		send_to_char( "Write a description for what?\n\r", ch );
		return FALSE;
	}

	if (IS_NULLSTR(argument))
	{
		send_to_char("You need to give a description.\n\r", ch);
		return FALSE;
	}

	if ( ( pObj = get_obj_carry( ch, arg, ch ) ) == NULL )
	{
		send_to_char("You don't have that.\n\r", ch);
		return FALSE;
	}

	PURGE_DATA(pObj->description);
	pObj->description = str_dup(arg2);
	return TRUE;
}

int maker_name(CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *pObj;
	char arg[MIL]={'\0'};
	char arg2[MIL]={'\0'};

	smash_tilde( argument );
	argument = one_argument( argument, arg );
	strncpy( arg2, argument, sizeof(arg2) );

	if ( IS_NULLSTR(arg) )
	{
		send_to_char( "Set a name for what?\n\r", ch );
		return FALSE;
	}

	if (IS_NULLSTR(argument))
	{
		send_to_char("You need to give a name.\n\r", ch);
		return FALSE;
	}

	if ( ( pObj = get_obj_carry( ch, arg, ch ) ) == NULL )
	{
		send_to_char("You don't have that.\n\r", ch);
		return FALSE;
	}

	PURGE_DATA(pObj->name);
	pObj->name = str_dup(arg2);
	return TRUE;
}

int maker_short_desc(CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *pObj;
	char arg[MIL]={'\0'};
	char arg2[MIL]={'\0'};

	smash_tilde( argument );
	argument = one_argument( argument, arg );
	strncpy( arg2, argument, sizeof(arg2) );

	if ( IS_NULLSTR(arg) )
	{
		send_to_char( "Set a short description for what?\n\r", ch );
		return FALSE;
	}

	if (IS_NULLSTR(argument))
	{
		send_to_char("You need to give a short description.\n\r", ch);
		return FALSE;
	}

	if ( ( pObj = get_obj_carry( ch, arg, ch ) ) == NULL )
	{
		send_to_char("You don't have that.\n\r", ch);
		return FALSE;
	}

	PURGE_DATA(pObj->short_descr);
	pObj->short_descr = str_dup(arg2);
	return TRUE;
}

int maker_type(CHAR_DATA *ch, char *argument)
{
	int type = 0;



	if((type = flag_value(type_flags, argument)) == NO_FLAG)
	{
		send_to_char("No such object type.\n\r", ch);
		return FALSE;
	}

	if(type == ITEM_FOOD)
	{
		send_to_char("You're a maker, not a chef!\n\r", ch);
		return FALSE;
	}

	ch->markup = type;
	return TRUE;
}

int maker_sell(CHAR_DATA *ch, char *argument)
{
	char arg1[MIL]={'\0'};
	OBJ_DATA *pObj;

	argument = one_argument(argument, arg1);

	if(IS_NULLSTR(arg1))
	{
		send_to_char("What are you looking to sell?\n\r", ch);
		return FALSE;
	}

	if((pObj = get_obj_carry(ch, arg1, ch)) == NULL)
	{
		send_to_char("You don't have that item.\n\r", ch);
		return FALSE;
	}

	if(!str_cmp(pObj->description, "A shapeless item rests here."))
	{
		send_to_char("You must give the object a string before sale.\n\r", ch);
		return FALSE;
	}

	if(!str_cmp(pObj->full_desc, "A shapeless item rests here."))
	{
		send_to_char("You must give the object a string before sale.\n\r", ch);
		return FALSE;
	}

	SET_BIT(pObj->extra_flags, ITEM_INVENTORY);
	send_to_char(Format("You prepare %s for sale.\n\r", pObj->short_descr), ch);
	if(!IS_SET(ch->act2, ACT2_RP_ING)) ch->xp_job_acts++;
	return TRUE;
}

bool show_help( CHAR_DATA *ch, char *argument );
int maker_wear(CHAR_DATA *ch, char *argument)
{
	show_help(ch,"wear");
	return TRUE;
}

int maker_size(CHAR_DATA *ch, char *argument)
{
	show_help(ch,"size");
	return TRUE;
}

int maker_materials(CHAR_DATA *ch, char *argument)
{
	show_help(ch,"material");
	return TRUE;
}

int maker_typehelp(CHAR_DATA *ch, char *argument)
{
	show_help(ch,"type");
	return TRUE;
}

/* >>>>>>>>>>>>>>> Janitor Section <<<<<<<<<<<<<<< */
int janitor_clean(CHAR_DATA *ch, char *argument)
{
	char arg1[MIL]={'\0'};
	OBJ_DATA *pObj;

	argument = one_argument(argument, arg1);

	if(IS_NULLSTR(arg1))
	{
		send_to_char("What are you looking to clean up?\n\r", ch);
		return FALSE;
	}

	if((pObj = get_obj_list( ch, arg1, ch->in_room->contents )) == NULL)
	{
		send_to_char("You don't see that here.\n\r", ch);
		return FALSE;
	}

	if(!IS_SET(pObj->wear_flags, ITEM_TAKE))
	{
		send_to_char("You can't pick it up to clean it away.\n\r", ch);
		return FALSE;
	}

	ch->dollars += pObj->cost/300;
	send_to_char(Format("You clean away %s\n\r", pObj->short_descr), ch);
	act("$n cleans away $o", ch, pObj, NULL, TO_ROOM, 0);
	extract_obj(pObj);
	if(!IS_SET(ch->act2, ACT2_RP_ING)) ch->xp_job_acts++;
	return TRUE;
}

int janitor_empty(CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *pObj;
	OBJ_DATA *content;
	OBJ_DATA *next;
	char arg1[MIL]={'\0'};

	argument = one_argument(argument, arg1);

	if(IS_NULLSTR(arg1))
	{
		send_to_char("What are you looking to empty?\n\r", ch);
		return FALSE;
	}

	if((pObj = get_obj_list( ch, arg1, ch->in_room->contents )) == NULL)
	{
		send_to_char("You don't see that here.\n\r", ch);
		return FALSE;
	}

	if(IS_SET(pObj->wear_flags, ITEM_TAKE))
	{
		send_to_char("You can't empty a mobile bin.\n\r", ch);
		return FALSE;
	}

	if(pObj->contains == NULL)
	{
		send_to_char("It's already empty!\n\r", ch);
		return FALSE;
	}

	send_to_char(Format("You empty away %s\n\r", pObj->short_descr), ch);
	act("$n empties $O", ch, pObj, NULL, TO_ROOM, 0);
	for(content = pObj->contains; content; content = next)
	{
		ch->dollars += content->cost/30;
		next = content->next_content;
		extract_obj(content);
	}
	if(!IS_SET(ch->act2, ACT2_RP_ING)) ch->xp_job_acts++;
	return TRUE;
}

/* >>>>>>>>>>>>>>> Salesman Commands <<<<<<<<<<<<<<< */
int sales_sell(CHAR_DATA *ch, char *argument)
{
	char arg1[MIL]={'\0'};
	OBJ_DATA *pObj;

	argument = one_argument(argument, arg1);

	if(IS_NULLSTR(arg1))
	{
		send_to_char("What are you looking to sell?\n\r", ch);
		return FALSE;
	}

	if((pObj = get_obj_carry(ch, arg1, ch)) == NULL)
	{
		send_to_char("You don't have that item.\n\r", ch);
		return FALSE;
	}

	SET_BIT(pObj->extra_flags, ITEM_INVENTORY);
	send_to_char(Format("You prepare %s for sale.\n\r", pObj->short_descr), ch);
	if(!IS_SET(ch->act2, ACT2_RP_ING)) ch->xp_job_acts++;
	return TRUE;
}

int sales_buy(CHAR_DATA *ch, char *argument)
{
	int cost = 0,roll = 0;
	CHAR_DATA *keeper;
	OBJ_DATA *obj,*t_obj;
	char arg[MIL]={'\0'};
	char arg2[MIL]={'\0'};
	int number = 0, count = 1;

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Buy what?\n\r", ch );
		return FALSE;
	};

	argument = one_argument(argument, arg2);
	number = mult_argument(arg2,arg);

	if ( ( keeper = find_keeper( ch, argument ) ) == NULL )
		return FALSE;

	obj  = get_obj_keeper( ch,keeper, arg );
	cost = get_cost( keeper, obj, TRUE );

	if (number < 1)
	{
		act("$n tells you 'Get real!",keeper,NULL,ch,TO_VICT,0);
		return FALSE;
	}

	if ( cost <= 0 || !can_see_obj( ch, obj ) )
	{
		act( "$n tells you 'I don't sell that -- try 'list''.", keeper, NULL, ch, TO_VICT, 0 );
		ch->reply = keeper;
		return FALSE;
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
			return FALSE;
		}
	}

	if ( (ch->cents + ch->dollars * 100) < cost * number )
	{
		if (number > 1)
			act("$n tells you 'You can't afford to buy that many.", keeper,obj,ch,TO_VICT,0);
		else
			act( "$n tells you 'You can't afford to buy $p'.", keeper, obj, ch, TO_VICT, 0 );
		ch->reply = keeper;
		return FALSE;
	}

	if (ch->carry_number +  number * get_obj_number(obj) > can_carry_n(ch))
	{
		send_to_char( "You can't carry that many items.\n\r", ch );
		return FALSE;
	}

	if ( ch->carry_weight + number * get_obj_weight(obj) > can_carry_w(ch))
	{
		send_to_char( "You can't carry that much weight.\n\r", ch );
		return FALSE;
	}

	/* haggle */
	roll = dice_rolls(ch, get_curr_stat(ch, STAT_CHA) + ch->ability[SUBTERFUGE].value,6);
	if (!IS_OBJ_STAT(obj,ITEM_SELL_EXTRACT)
			&& roll > 0 && IS_NPC(keeper))
	{
		cost -= obj->cost / roll;
		act("You haggle with $N.",ch,NULL,keeper,TO_CHAR,1);
	}

	if (number > 1)
	{
		act(Format("$n buys $p[%d].",number),ch,obj,NULL,TO_ROOM,0);
		act(Format("You buy $p[%d] for %d cents.",number,cost * number),ch,obj,NULL,TO_CHAR,1);
	}
	else
	{
		act( "$n buys $p.", ch, obj, NULL, TO_ROOM, 0 );
		act( Format("You buy $p for %d cents.",cost), ch, obj, NULL, TO_CHAR, 1 );
	}
	deduct_cost(ch,cost * number);
	keeper->dollars += cost * number/100;
	keeper->cents += cost * number - (cost * number/100) * 100;

	for (count = 0; count < number; count++)
	{
		if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) && IS_NPC(keeper) )
			t_obj = create_object( obj->pIndexData );
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
	return TRUE;
}

int sales_markup(CHAR_DATA *ch, char *argument)
{
	char arg1[MIL]={'\0'};

	argument = one_argument(argument, arg1);

	if(!is_number(arg1))
	{
		send_to_char("What do you want to set your markup at?\n\r", ch);
		return FALSE;
	}

	if(0 > atoi(arg1) || atoi(arg1) > 200)
	{
		send_to_char("Markup must be between 0% and 200%\n\r", ch);
		return FALSE;
	}

	ch->markup = atoi(arg1);
	send_to_char(Format("Markup set at %d%%", ch->markup), ch);
	return TRUE;
}

/* >>>>>>>>>>>>>>> Mayor Commands <<<<<<<<<<<<<<< */
int mayor_tax(CHAR_DATA *ch, char *argument)
{



	if(IS_NULLSTR(argument))
	{
		send_to_char(Format("The current tax rate is %d%%\n\r", tax_rate), ch);
		return FALSE;
	}

	if(is_number(argument))
	{
		tax_rate = atoi(argument);
		send_to_char(Format("You set the tax rate to %d%%\n\r", tax_rate), ch);
		if(!IS_SET(ch->act2, ACT2_RP_ING)) ch->xp_job_acts++;
		return TRUE;
	}
	send_to_char("Syntax: job tax [optional number]\n\r",ch);
	return FALSE;
}

int mayor_appoint(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *vch;
	char arg[MIL]={'\0'};
	char buf[MSL]={'\0'};
	int o = 0, online = 0, in_char_list = 0;
	char *position;

	argument = one_argument(argument, arg);

	if(IS_NULLSTR(argument) || IS_NULLSTR(arg))
	{
		send_to_char("Appoint whom to which position?\n\r", ch);
		return FALSE;
	}

	online = is_online(arg);
	in_char_list = pc_in_char_list(arg);

	if((vch = get_player(arg)) == NULL)
	{
		send_to_char("There's nobody in the city of that name.\n\r", ch);
		return FALSE;
	}

	if(!IS_NATURAL(vch))
	{
		send_to_char("Only the mortal should be in the public eye.\n\r", ch);
		if(!online && !in_char_list)
		{
			free_char(vch);
		}
		return FALSE;
	}

	if((o = office_lookup(argument)) <= 2)
	{
		send_to_char("What post are you trying to fill?\n\r", ch);
		return FALSE;
	}

	PURGE_DATA(vch->profession);
	vch->profession = str_dup(office_table[o].name);

	send_to_char(Format("You appoint %s to the post of %s.\n\r", vch->name, vch->profession), ch);

	switch(o)
	{
	default: position = NULL;
	break;
	case 3 : position = incumbent_pchief;
	break;
	}

	if(position == NULL)
	{
		send_to_char("Something messed up. Please try again.", ch);
		return FALSE;
	}

	if(!IS_SET(ch->act2, ACT2_RP_ING)) ch->xp_job_acts++;

	snprintf(buf, sizeof(buf), "%s", position);
	PURGE_DATA(position);
	position = str_dup(vch->name);
	vch->backgrounds[FAME_STATUS] += office_table[o].fame;
	if(!online && !in_char_list)
	{
		free_char(vch);
	}
	else
	{
		send_to_char(Format("You have been appointed to the post of %s.\n\r", vch->profession), ch);
	}

	online = is_online(buf);
	in_char_list = pc_in_char_list(buf);
	if((vch = get_player(buf)) == NULL)
	{
		return FALSE;
	}
	else
	{
		PURGE_DATA(vch->profession);
		vch->profession = str_dup("None");
		vch->backgrounds[FAME_STATUS] -= office_table[o].fame / 2;
		if(!online && !in_char_list)
		{
			free_char(vch);
		}
		else
		{
			send_to_char(Format("A new appointee has replaced you as %s", office_table[o].name), vch);
		}
	}
	return TRUE;
}


/* >>>>>>>>>>>>>>> Judge Commands <<<<<<<<<<<<<<< */
int judge_marry(CHAR_DATA *ch, char *argument)
{
	char arg[MIL]={'\0'};
	char arg2[MIL]={'\0'};
	CHAR_DATA *s1;
	CHAR_DATA *s2;

	if(IS_NULLSTR(argument))
	{
		send_to_char("Syntax: job marry [target1] [target2]\n\r",ch);
		return FALSE;
	}

	argument = one_argument(argument, arg);
	argument = one_argument(argument, arg2);

	if(IS_NULLSTR(arg) || IS_NULLSTR(arg2))
	{
		send_to_char("Syntax: job marry <target1> <target2>\n\r",ch);
		return FALSE;
	}

	if((s1 = get_char_room(ch, arg)) == NULL
			|| (s2 = get_char_room(ch, arg2)) == NULL)
	{
		send_to_char("One of those people are not present.\n\r", ch);
		return FALSE;
	}

	PURGE_DATA(s1->married);
	PURGE_DATA(s2->married);
	s1->married = str_dup(Format("%s", s2->name));
	s2->married = str_dup(Format("%s", s1->name));

	do_say(ch,"By the power vested in me, I declare that from this day forth this couple are bound in vows of marriage.");

	return TRUE;
}


/* >>>>>>>>>>>>>>> Job and Unemployment Commands <<<<<<<<<<<<<<< */
void do_job(CHAR_DATA *ch, char *argument)
{
	int jn = 0, cmd = 0;
	char arg[MIL]={'\0'};

	if(IS_NULLSTR(argument))
	{
		send_to_char("Perform which job function?\n\r", ch);
		return;
	}

	argument = one_argument(argument, arg);

	if((jn = job_lookup(ch->profession)) == -1)
	{
		jn = job_lookup("None");
	}

	if((cmd = job_cmd_lookup(arg, (void *)job_table[jn].cmd_table)) == -1)
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if(job_table[jn].cmd_table[cmd].pos_pts > ch->pospts)
	{
		send_to_char("You haven't got the energy to put in the effort yet.\n\r", ch);
		return;
	}

	if((*job_table[jn].cmd_table[cmd].func) ( ch, argument ))
	{
		ch->pospts -= job_table[jn].cmd_table[cmd].pos_pts;
		send_to_char("Ok.\n\r", ch);
	}
	WAIT_STATE(ch, job_table[jn].cmd_table[cmd].delay);
	if(ch->xp_job_acts > job_table[jn].acts_per_xp)
	{
		if(ch->ooc_xp_count < 3)
		{
			ch->oocxp++;
			ch->ooc_xp_count++;
			send_to_char("You learn from being on the job.\n\r", ch);
		}
		ch->dollars += number_fuzzy(10);
		ch->xp_job_acts = 0;
	}

	return;
}

int job_commands(CHAR_DATA *ch, char *argument)
{
	int cmd = 0;
	int col = 0;
	int jn = 0;



	if(ch->profession == NULL || (jn = job_lookup(ch->profession)) < 0)
	{
		jn = job_lookup("None");
	}

	col = 0;
	for ( cmd = 0; job_table[jn].cmd_table[cmd].name != NULL; cmd++ )
	{
		send_to_char( Format("%-12s ", job_table[jn].cmd_table[cmd].name), ch );
		if ( ++col % 4 == 0 )
			send_to_char( "\n\r", ch );
	}

	if ( col % 6 != 0 )
		send_to_char( "\n\r", ch );
	return TRUE;
}


int job_advance(CHAR_DATA *ch, char *argument)
{
	int cost = 0;
	int jn = 0;



	if(ch->profession == NULL || (jn = job_lookup(ch->profession)) < 0)
	{
		jn = job_lookup("None");
	}

	if(ch->job_skill[jn] <= 0) cost = 5;
	else cost = ch->job_skill[jn] * 3;

	if(ch->oocxp >= cost)
	{
		send_to_char("You improve your job skill.\n\r", ch);
		ch->job_skill[jn]++;
		ch->oocxp -= cost;
	}
	else
	{
		send_to_char("You don't have enough OOC experience to improve your job skills.\n\r", ch);
	}

	return TRUE;
}

int job_apply(CHAR_DATA *ch, char *argument)
{
	int job = 0;
	char arg1[MIL]={'\0'};
	char arg2[MIL]={'\0'};
	CHAR_DATA *vch;

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if(IS_NULLSTR(arg1))
	{
		send_to_char("Who're you trying to get a job with?\n\r", ch);
		return FALSE;
	}

	if((vch = get_char_room(ch, arg1)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return FALSE;
	}

	if(ch == vch)
	{
		send_to_char("You can't hire yourself.\n\r", ch);
		return FALSE;
	}

	if(IS_NPC(vch))
		if(!IS_SET(vch->act2, ACT2_EMPLOYER) && vch->pIndexData->pShop == NULL)
		{
			send_to_char("They can't employ you.\n\r", ch);
			return FALSE;
		}

	if(IS_NULLSTR(arg2))
	{
		int i = 0;
		int col = 0;
		send_to_char("What kind of job are you looking for?\n\r", ch);

		send_to_char("Options for employment are:\n\r", ch);
		for(i = 4; job_table[i].name != NULL; i++)
		{
			if(CAN_BE(ch, i))
			{
				send_to_char(job_table[i].name, ch);
				send_to_char(" ", ch);
				col++;
				if(col == 6)
					send_to_char("\n\r", ch);
			}
		}

		return FALSE;
	}

	if((job = job_lookup(arg2)) > 3 && CAN_BE(ch, job))
	{
		int diff = 7;
		int fail = 0;
		int whatever = 0;

		if(job == job_lookup(ch->profession))
		{
			diff -= 2;
		}

		if(job_lookup(ch->profession) < 4)
		{
			diff -= 3;
		}

		if(ch->ability[job_table[job].abil].value == 0)
		{
			do_say(vch, (char *)Format("I just don't think you've got the %s for this.", ability_table[job_table[job].abil].name));
			return TRUE;
		}

		whatever = get_curr_stat(ch, STAT_CHA) + ch->ability[job_table[job].abil].value;

		fail = dice_rolls(ch, whatever, diff);
		fail += dice_rolls(ch, whatever, diff);

		send_to_char(Format("You discuss job opportunities with %s\n\r", IS_NPC(vch) ? vch->short_descr : LOOKS_DIFFERENT(vch) ? vch->alt_name : vch->name), ch);
		if(!IS_NPC(vch))
		{
			if(IS_SET(vch->act2, ACT2_NO_HIRE))
			{
				do_say(vch, "I'm sorry I'm just not hiring right now.");
				return FALSE;
			}
		}

		if(fail > 1)
		{
			PURGE_DATA(ch->profession);
			ch->profession = str_dup(job_table[job].name);
			if(!str_cmp(ch->profession, "Chef")) ch->markup = ITEM_FOOD;
			ch->employer = IS_NPC(vch) ? vch->pIndexData->vnum : 0;
			do_say(vch, "Well then, welcome aboard.");
		}
		else
		{
			do_say(vch, "I just don't think you're right for this.");
		}

	}
	else
	{
		send_to_char("That profession is unavailable at this time.\n\r", ch);
	}
	return TRUE;
}

int job_nohire (CHAR_DATA *ch, char *argument)
{
	if(IS_SET(ch->act2,ACT2_NO_HIRE))
	{
		REMOVE_BIT(ch->act2,ACT2_NO_HIRE);
		send_to_char("You will no longer hire job applicants.\n\r", ch);
	}
	else
	{
		SET_BIT(ch->act2,ACT2_NO_HIRE);
		send_to_char("You will now introduce players to professions.\n\r", ch);
	}
	return TRUE;
}

int job_quit (CHAR_DATA *ch, char *argument)
{


	if(ch->employer > 0)
	{
		ch->employer = 0;
		send_to_char("You leave the service of your employer.\n\r", ch);
	}
	else
	{
		send_to_char("You aren't employed by anybody else!\n\r", ch);
	}

	return TRUE;
}


/* >>>>>>>>>>>>>>> Doctor Commands <<<<<<<<<<<<<<< */
int doc_heal (CHAR_DATA *ch, char *argument)
{
	return FALSE;
}

int doc_resuscitate (CHAR_DATA *ch, char *argument)
{
	return FALSE;
}

int doc_prescribe (CHAR_DATA *ch, char *argument)
{
	return FALSE;
}

/* >>>>>>>>>>>>>>> Pharmist Commands <<<<<<<<<<<<<<< */
int pharm_fill_script (CHAR_DATA *ch, char *argument)
{
	return FALSE;
}

/* >>>>>>>>>>>>>>> Police Commands <<<<<<<<<<<<<<< */
int police_arrest (CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	int fail = 0;



	if(IS_NULLSTR(argument))
	{
		send_to_char("Whack who?\n\r", ch);
		return FALSE;
	}

	if((victim = get_char_room(ch, argument)) == NULL)
	{
		send_to_char("They're not here.\n\r", ch);
		return FALSE;
	}

	if(victim->position != P_FIGHT
			&& !IS_DRUNK(victim)
			&& !IS_HIGH(victim)
			&& !IS_TRIPPING(victim)
			&& !IS_WANTED(victim)
			&& dice_rolls(ch,
					get_curr_stat(ch, STAT_INT) + ch->ability[LAW].value, 7) > 1)
	{
		send_to_char("They aren't doing any harm.\n\r", ch);
		return FALSE;
	}

	if((fail = dice_rolls(ch,
			get_curr_stat(ch, STAT_STR) + ch->ability[BRAWL].value,
			get_curr_stat(victim, STAT_STR) + victim->ability[DODGE].value)) < 3)
	{
		act("$n tries to arrest $N, but $M gets away.", ch, NULL, victim, TO_NOTVICT, 0);
		act("You try to arrest $N, but $M gets away.", ch, NULL, victim, TO_CHAR, 1);
		act("$n tries to arrest you, but you get away.", ch, NULL, victim, TO_VICT, 1);
		do_flee(victim, "");
		return TRUE;
	}
	else
	{
		if(IS_DRUNK(victim) || IS_HIGH(victim)
				|| IS_TRIPPING(victim) || victim->position == P_FIGHT)
		{
			act("$n arrests $N, but $M is innocent!", ch, NULL, victim, TO_NOTVICT, 0);
			act("You arrest $N, but $M is innocent!", ch, NULL, victim, TO_CHAR, 1);
			send_to_char("That'll go on your record for sure.", ch);
			act("$n arrests you, even though you're innocent.", ch, NULL, victim, TO_VICT, 1);
			send_to_char("Your lawyer's going to have a field day.", victim);
			act("$N sneers at $n for failing his duty as a constable.", ch, NULL, victim, TO_NOTVICT, 0);
			act("$N sneers at you.", ch, NULL, victim, TO_CHAR, 1);
			act("You sneer at $n.", ch, NULL, victim, TO_VICT, 1);
			ch->warrants += 7;
		}
		else
		{
			act("$n arrests $N, dragging $M into a squad car.", ch, NULL, victim, TO_NOTVICT, 0);
			act("You arrest $N and load $M into a squad car.", ch, NULL, victim, TO_CHAR, 1);
			act("$n arrests you and ships you off to gaol.", ch, NULL, victim, TO_VICT, 1);
			char_from_room(victim);
			char_to_room(victim, get_room_index(ROOM_VNUM_JAIL));
			do_look(victim, "");
		}
		return TRUE;
	}
	return FALSE;
}

int police_breathalise (CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	int fail = 0;



	if(IS_NULLSTR(argument))
	{
		send_to_char("Whack who?\n\r", ch);
		return FALSE;
	}

	if((victim = get_char_room(ch, argument)) == NULL)
	{
		send_to_char("They're not here.\n\r", ch);
		return FALSE;
	}

	if((fail = dice_rolls(ch,
			get_curr_stat(ch, STAT_PER) + ch->ability[MEDICINE].value, 8)) > 1)
	{
		if(IS_DRUNK(victim))
			act("$N is drunk.", ch, NULL, victim, TO_CHAR, 1);
		if(IS_HIGH(victim))
			act("$N seems to be high on something.", ch,NULL,victim,TO_CHAR,1);
		if(IS_TRIPPING(victim))
			act("$N seems to be out of touch with reality.", ch, NULL, victim, TO_CHAR,1);

		if(!IS_TRIPPING(victim) && !IS_HIGH(victim) && !IS_DRUNK(victim))
			act("If $N is under the affect of anything you can't detect it.", ch, NULL, victim, TO_CHAR,1);

		return TRUE;
	}
	else
	{
		act("If $N is under the affect of anything you can't detect it.", ch, NULL, victim, TO_CHAR,1);

		return TRUE;
	}

	return FALSE;
}

int police_background (CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	int fail = 0;

	if(IS_NULLSTR(argument))
	{
		send_to_char("Whack who?\n\r", ch);
		return FALSE;
	}

	if((victim = get_char_room(ch, argument)) == NULL)
	{
		send_to_char("They're not here.\n\r", ch);
		return FALSE;
	}

	if((fail = dice_rolls(ch,
			get_curr_stat(ch, STAT_PER) + ch->ability[LAW].value, 8)) > 1)
	{
		if(victim->warrants > 50)
			act("$N might as well have an all points bulletin out.", ch, NULL, victim, TO_CHAR, 1);
		else if(victim->warrants > 40)
			act("$N has made the local most wanted list.", ch, NULL, victim, TO_CHAR, 1);
		else if(victim->warrants > 30)
			act("$N has several warrants outstanding.", ch, NULL, victim, TO_CHAR, 1);
		else if(victim->warrants > 15)
			act("$N has some minor offences outstanding.", ch, NULL, victim, TO_CHAR, 1);
		else act("$N has no open warrants.", ch, NULL, victim, TO_CHAR, 1);

		return TRUE;
	}
	else if(fail < 0)
	{
		int i = number_range(1,5);
		switch(i)
		{
		case 5:	act("$N might as well have an all points bulletin out.", ch, NULL, victim, TO_CHAR, 1);
		break;
		case 4:	act("$N has made the local most wanted list.", ch, NULL, victim, TO_CHAR, 1);
		break;
		case 3:	act("$N has several warrants outstanding.", ch, NULL, victim, TO_CHAR, 1);
		break;
		case 2:	act("$N has some minor offenses outstanding.", ch, NULL, victim, TO_CHAR, 1);
		break;
		default:	act("$N has no open warrants.", ch, NULL, victim, TO_CHAR, 1);
		break;
		}

		return TRUE;
	}
	else
	{
		act("You can't find $N's record in the database.", ch, NULL, victim, TO_CHAR, 1);

		return TRUE;
	}

	return FALSE;
}

/* >>>>>>>>>>>>>>> Criminal Commands <<<<<<<<<<<<<<< */
int crim_pick (CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *gch;
	OBJ_DATA *obj;
	char arg[MIL]={'\0'};
	int door = 0;
	int diff = 0;

	one_argument( argument, arg );

	if ( IS_NULLSTR(arg) )
	{
		send_to_char( "Pick what?\n\r", ch );
		return FALSE;
	}

	WAIT_STATE( ch, skill_table[gsn_pick_lock].beats );

	/* look for guards */
	for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
	{
		if ( IS_NPC(gch) && IS_AWAKE(gch) )
		{
			act( "$N is standing too close to the lock.", ch, NULL, gch, TO_CHAR, 1 );
			return FALSE;
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

		if ( (!IS_NPC(ch) && (dice_rolls(ch, get_curr_stat(ch, STAT_DEX) + ch->ability[LARCENY].value,
						diff) <= 1)) || IS_SET(obj->value[1],EX_PICKPROOF))
		{
			send_to_char( "You failed.\n\r", ch);
			return TRUE;
		}

		/* portal stuff */
		if (obj->item_type == ITEM_PORTAL)
		{
			if (!IS_SET(obj->value[1],EX_ISDOOR))
			{
				send_to_char("You can't do that.\n\r",ch);
				return FALSE;
			}

			if (!IS_SET(obj->value[1],EX_CLOSED))
			{
				send_to_char("It's not closed.\n\r",ch);
				return FALSE;
			}

			if (obj->value[4] < 0)
			{
				send_to_char("It can't be unlocked.\n\r",ch);
				return FALSE;
			}

			if (IS_SET(obj->value[1],EX_PICKPROOF))
			{
				send_to_char("You failed.\n\r",ch);
				return TRUE;
			}

			REMOVE_BIT(obj->value[1],EX_LOCKED);
			act("You pick the lock on $p.",ch,obj,NULL,TO_CHAR,1);
			act("$n picks the lock on $p.",ch,obj,NULL,TO_ROOM,0);
			if(!IS_SET(ch->act2, ACT2_RP_ING)) ch->xp_job_acts++;
			return TRUE;
		}

		/* 'pick object' */
		if ( obj->item_type != ITEM_CONTAINER )
		{ send_to_char( "That's not a container.\n\r", ch ); return FALSE; }
		if ( !IS_SET(obj->value[1], CONT_CLOSED) )
		{ send_to_char( "It's not closed.\n\r",        ch ); return FALSE; }
		if ( obj->value[2] < 0 )
		{ send_to_char( "It can't be unlocked.\n\r",   ch ); return FALSE; }
		if ( !IS_SET(obj->value[1], CONT_LOCKED) )
		{ send_to_char( "It's already unlocked.\n\r",  ch ); return FALSE; }
		if ( IS_SET(obj->value[1], CONT_PICKPROOF) )
		{ send_to_char( "You failed.\n\r",             ch ); return TRUE; }

		REMOVE_BIT(obj->value[1], CONT_LOCKED);
		act("You pick the lock on $p.",ch,obj,NULL,TO_CHAR,1);
		act("$n picks the lock on $p.",ch,obj,NULL,TO_ROOM,0);
		if(!IS_SET(ch->act2, ACT2_RP_ING)) ch->xp_job_acts++;
		return TRUE;
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

		if ( (!IS_NPC(ch)
				&& (dice_rolls(ch,
						get_curr_stat(ch, STAT_DEX) + ch->ability[LARCENY].value,
						diff) <= 1)) || IS_SET(pexit->exit_info,EX_PICKPROOF))
		{
			send_to_char( "You failed.\n\r", ch);
			return TRUE;
		}

		if ( !IS_SET(pexit->exit_info, EX_CLOSED) && !IS_ADMIN(ch))
		{ send_to_char( "It's not closed.\n\r",        ch ); return FALSE; }
		if ( pexit->key < 0 && !IS_ADMIN(ch))
		{ send_to_char( "It can't be picked.\n\r",     ch ); return FALSE; }
		if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
		{ send_to_char( "It's already unlocked.\n\r",  ch ); return FALSE; }
		if ( IS_SET(pexit->exit_info, EX_PICKPROOF) && !IS_ADMIN(ch))
		{ send_to_char( "You failed.\n\r",             ch ); return TRUE; }

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

	if(!IS_SET(ch->act2, ACT2_RP_ING)) ch->xp_job_acts++;

	return TRUE;
}

int crim_whack (CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	int fail = 0;



	if(IS_NULLSTR(argument))
	{
		send_to_char("Whack who?\n\r", ch);
		return FALSE;
	}

	if((victim = get_char_room(ch, argument)) == NULL)
	{
		send_to_char("They're not here.\n\r", ch);
		return FALSE;
	}

	if(ch == victim)
	{
		send_to_char("You beat yourself up, amusingly it kinda hurts!\n\r",ch);
		ch->condition[COND_PAIN] += 2;
		return FALSE;
	}

	if(IS_SET(victim->act2, ACT2_RP_ING))
	{
		send_to_char("You cannot use this on a person in character... RP it.\n\r", ch);
		return FALSE;
	}

	if((fail = dice_rolls(ch,
			get_curr_stat(ch, STAT_DEX) + ch->ability[BRAWL].value,
			get_curr_stat(victim, STAT_WIT) + victim->ability[DODGE].value)) < 2)
	{
		act("You miss $N.\n\r", ch, NULL, victim, TO_CHAR, 1);
		act("$n takes a swing at you, but misses.", ch,NULL,victim,TO_VICT,1);
		act("$n takes a swing at $N, but misses.", ch,NULL,victim,TO_NOTVICT,0);
		if(IS_NPC(victim))
		{
			multi_hit( victim, ch, TYPE_UNDEFINED );
		}

		return TRUE;
	}
	else
	{
		act("You club $N over the head causing them to fall down, stunned.", ch, NULL, victim, TO_CHAR, 1);
		act("$n clubs you over the head and you fall to the ground stunned.", ch, NULL, victim, TO_VICT, 1);
		act("$n stuns $N with a blow to the head.", ch, NULL, victim, TO_NOTVICT, 0);

		victim->position = P_STUN;
		if(!IS_SET(ch->act2, ACT2_RP_ING)) ch->xp_job_acts++;

		return TRUE;
	}

	return FALSE;
}

int crim_hotwire (CHAR_DATA *ch, char *argument)
{


	send_to_char("Hotwire is coming with vehicles.\n\r", ch);

	return FALSE;
}

int crim_stickup (CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	int fail = 0;



	if(IS_NULLSTR(argument))
	{
		send_to_char("Stick up whom?\n\r", ch);
		return FALSE;
	}

	if((victim = get_char_room(ch, argument)) == NULL)
	{
		send_to_char("They're not here.\n\r", ch);
		return FALSE;
	}

	if(ch == victim)
	{
		send_to_char("Don't be daft! You can't stick yourself up!\n\r",ch);
		return FALSE;
	}

	if(IS_SET(victim->act2, ACT2_RP_ING))
	{
		send_to_char("You cannot use this on a person in character... RP it.\n\r", ch);
		return FALSE;
	}

	if((fail = dice_rolls(ch, get_curr_stat(ch, STAT_MAN) + ch->ability[INTIMIDATION].value, victim->max_willpower))  < 2)
	{
		act("You threaten $N, but they laugh and don't give up any cash.", ch, NULL, victim, TO_CHAR, 1);
		act("$n tries to bully you out of your money, but you laugh at $m.", ch, NULL, victim, TO_VICT, 1);
		act( "$n tries to bully $N out of some money, but gets laughed at instead.", ch, NULL, victim, TO_NOTVICT, 0);
		ch->warrants += 5;
		return TRUE;
	}

	else if(fail == 2)
	{
		act("You bully $N into giving you some money, but they don't look really scared.", ch, NULL, victim, TO_CHAR, 1);
		act("$n bullies you out of some cash, but you make plans to call the police.", ch, NULL, victim, TO_VICT, 1);
		act("$n bullies $N into giving $m some cash, but $N doesn't look all that scared.", ch, NULL, victim, TO_NOTVICT, 0);
		ch->warrants += 7;
		ch->dollars += victim->dollars/10;
		victim->dollars -= victim->dollars/10;
		if(!IS_SET(ch->act2, ACT2_RP_ING)) ch->xp_job_acts++;
		return TRUE;
	}

	else if(fail < 5)
	{
		act("You bully $N out of some cash.", ch, NULL, victim, TO_CHAR, 1);
		act("$n bullies you out of some cash.", ch, NULL, victim, TO_VICT, 1);
		act("$n bullies $N out of some cash.", ch, NULL, victim, TO_NOTVICT, 0);
		ch->warrants += 6;
		ch->dollars += victim->dollars/4;
		victim->dollars -= victim->dollars/4;
		if(!IS_SET(ch->act2, ACT2_RP_ING)) ch->xp_job_acts++;
		return TRUE;
	}

	else
	{
		act("You really shake $N up and they give you all their money.", ch, NULL, victim, TO_CHAR, 1);
		act("$n demands all your money, looking like they mean business.", ch, NULL, victim, TO_VICT, 1);
		act("You give $N all of your money so they won't hurt you.", ch, NULL, victim, TO_VICT, 1);
		act("$N hands over a bunch of money to $n looking quite scared.", ch, NULL, victim, TO_NOTVICT, 0);
		ch->warrants += 4;
		ch->dollars += victim->dollars;
		victim->dollars = 0;
		ch->cents += victim->cents;
		victim->cents = 0;
		if(!IS_SET(ch->act2, ACT2_RP_ING)) ch->xp_job_acts++;
		return TRUE;
	}

	return TRUE;
}

int crim_case(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	int fail = 0;

	if(IS_NULLSTR(argument))
	{
		send_to_char("Case whom?\n\r", ch);
		return FALSE;
	}

	if((victim = get_char_room(ch, argument)) == NULL)
	{
		send_to_char("They're not here.\n\r", ch);
		return FALSE;
	}

	if((fail = dice_rolls(ch, get_curr_stat(ch, STAT_PER) + ch->ability[INVESTIGATION].value, victim->ability[LARCENY].value + 5))  < 2)
	{
		send_to_char("You fail to discover anything about them.", ch);
	}
	else
	{
		send_to_char(Format("%s is carrying about $%d\n\r", IS_NPC(victim) ? victim->long_descr : LOOKS_DIFFERENT(victim) ? victim->alt_name : victim->name, victim->dollars), ch);
		do_peek(ch, argument);
		if(!IS_SET(ch->act2, ACT2_RP_ING)) ch->xp_job_acts++;
	}

	return TRUE;
}

/* >>>>>>>>>>>>>>> Teacher Commands <<<<<<<<<<<<<<< */
void do_instruct(CHAR_DATA *ch, char *argument)
{
	int gift = 0;
	char arg[MIL]={'\0'};
	CHAR_DATA *vch;

	argument = one_argument(argument, arg);

	if(IS_NULLSTR(argument) || IS_NULLSTR(arg))
	{
		send_to_char("Who do you want to instruct in what?\n\r", ch);
		return;
	}

	if((vch = get_char_room(ch, arg)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	if((gift = gift_lookup(argument)) < 0)
	{
		act("What are you trying to instruct $N in?", ch, NULL, vch, TO_CHAR, 1);
		return;
	}

	if(gift != gift_lookup(vch->to_learn))
	{
		act("$N is not seeking to learn that.", ch, NULL, vch, TO_CHAR, 1);
		return;
	}

	if(IS_SET(vch->powers[gift_table[gift].level], gift_table[gift].flag))
	{
		act("$N already knows about $t.", ch, gift_table[gift].name, vch, TO_CHAR, 1);
		return;
	}

	act("You instruct $N in $t.", ch, gift_table[gift].name, vch, TO_CHAR, 1);
	act("$n instructs you in $t.", ch, gift_table[gift].name, vch, TO_VICT, 1);
	return;
}


void do_teach(CHAR_DATA *ch, char *argument)
{
	int abil = 0;
	char arg[MIL]={'\0'};
	CHAR_DATA *vch;

	argument = one_argument(argument, arg);

	if(IS_NULLSTR(argument) || IS_NULLSTR(arg))
	{
		send_to_char("Who do you want to teach which discipline?\n\r", ch);
		return;
	}

	if((vch = get_char_room(ch, arg)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	if(ch == vch)
	{
		send_to_char("You can't teach that to yourself.\n\r", ch);
		return;
	}

	if((abil = disc_lookup(ch, argument)) < 0)
	{
		act("Which discipline are you trying to teach $N?", ch, NULL, vch, TO_CHAR, 1);
		return;
	}

	if(!vch->to_learn || vch->to_learn[0] == '\0' || abil != disc_lookup(ch, vch->to_learn))
	{
		act("$N is not seeking to learn that.", ch, NULL, vch, TO_CHAR, 1);
		return;
	}

	if(ch->disc[abil] <= vch->disc[abil])
	{
		act("You have nothing to teach $N about $t.", ch, disc_table[abil].vname, vch, TO_CHAR, 1);
		return;
	}

	vch->train_success = TRUE;
	act("You instruct $N with their $t.", ch, disc_table[abil].vname, vch, TO_CHAR, 1);
	act("$n instructs you in $t.", ch, disc_table[abil].vname, vch, TO_VICT, 1);
	return;
}


/* >>>>>>>>>>>>>>> Reporter Commands <<<<<<<<<<<<<<< */
int reporter_subject(CHAR_DATA *ch, char *argument)
{


	parse_note(ch, (char *)Format("subject %s", argument), NOTE_ARTICLE);

	return TRUE;
}

int reporter_category(CHAR_DATA *ch, char *argument)
{


	parse_note(ch, (char *)Format("to %s", argument), NOTE_ARTICLE);

	return TRUE;
}

int reporter_body(CHAR_DATA *ch, char *argument)
{


	parse_note(ch, (char *)Format("body %s", argument), NOTE_ARTICLE);

	return TRUE;
}

int reporter_post(CHAR_DATA *ch, char *argument)
{
	bool note_there = FALSE;

	if(ch->pnote) 
		note_there = TRUE;

	parse_note(ch, (char *)Format("post %s", argument), NOTE_ARTICLE);

	if(note_there)
	{
		if(!IS_SET(ch->act2, ACT2_RP_ING)) ch->xp_job_acts++;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/* >>>>>>>>>>>>>>> Musician Commands <<<<<<<<<<<<<<< */
int muso_jam(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *vch;
	// bool no_go = FALSE;

	/* Start playing... initiate music stuff. */
	if(IS_NULLSTR(argument))
	{
		if(!IS_SET(ch->in_room->room_flags, ROOM_JAMMIN))
		{
			send_to_char("You start jamming.\n\r", ch);
			act("$n starts an open jam session.", ch, NULL, NULL, TO_ROOM, 0);
			SET_BIT(ch->in_room->room_flags, ROOM_JAMMIN);
			SET_BIT(ch->comm, COMM_JAMMIN);
			if(!IS_SET(ch->act2, ACT2_RP_ING)) ch->xp_job_acts++;
			return TRUE;
		}
		else if(ch->leader != NULL && IS_SET(ch->leader->comm, COMM_JAMMIN) && IS_SET(ch->leader->comm, COMM_JAM_LEAD))
		{
			send_to_char("You join your band leader in the jam.\n\r", ch);
			return FALSE;
		}

		for(vch = ch->in_room->people;vch;vch = vch->next_in_room)
		{
			if(vch != ch && IS_SET(ch->comm, COMM_JAM_LEAD))
			{
				// no_go = TRUE;
			}
		}

		send_to_char("There seems to be a band only jam going on already.\n\r", ch);
		return FALSE;
	}

	if(!str_cmp(argument, "band") || !str_cmp(argument, "group"))
	{
		if(!IS_SET(ch->in_room->room_flags, ROOM_JAMMIN) && ch->leader == NULL) {
			send_to_char("You nod to the band and start the set.\n\r", ch);
			for(vch = ch->in_room->people; vch; vch = vch->next_in_room)
			{
				if(is_same_group(vch, ch))
				{
					act("$n nods to you and the band and you start the set.", ch, NULL, vch, TO_VICT, 0);
				}
				else
				{
					act("$n nods to the band and starts the set.", ch, NULL, vch, TO_VICT, 0);
				}
			}
			SET_BIT(ch->in_room->room_flags, ROOM_JAMMIN);
			SET_BIT(ch->comm, COMM_JAMMIN);
			SET_BIT(ch->comm, COMM_JAM_LEAD);
			if(!IS_SET(ch->act2, ACT2_RP_ING)) ch->xp_job_acts++;
			return TRUE;
		}
		else
		{
			send_to_char("You can't start a band-only set while someone else's set is jammin'.\n\r", ch);
			return FALSE;
		}
	}

	if(!str_cmp(argument, "end") || !str_cmp(argument, "done"))
	{
		send_to_char("You end your set.\n\r", ch);
		act("$n steps out of the jam.", ch, NULL, NULL, TO_ROOM, 0);
		if(IS_SET(ch->comm, COMM_JAMMIN))
		{
			REMOVE_BIT(ch->comm, COMM_JAMMIN);
		}
		if(IS_SET(ch->comm, COMM_JAM_LEAD))
		{
			REMOVE_BIT(ch->comm, COMM_JAM_LEAD);
			REMOVE_BIT(ch->in_room->room_flags, ROOM_JAMMIN);
		}
		return TRUE;
	}

	return FALSE;
}

void stanza(char *argument)
{
	char *letter;
	char temp[MSL]={'\0'};

	if((letter = strstr(argument, "/")) == NULL) return;

	strncpy(temp, argument, sizeof(temp));
	temp[strlen(argument) - strlen(letter)] = '\0';
	strncat(temp, "\n\r", sizeof(temp) - strlen(temp) - 1);

	letter++;

	if(strstr(letter, "/"))
		stanza(letter);
	strncat(temp, letter, sizeof(temp) - strlen(temp) - 1);

	/* Removed to try to fix name changing bug.
    return str_dup(temp);
	 */
	strncpy(argument, temp, sizeof(*argument));
	return;
}

int muso_sing(CHAR_DATA *ch, char *argument)
{
	int door = 0;

	/* Sing a line, or verse, from a song */
	if(IS_NULLSTR(argument))
	{
		send_to_char("What are you going to sing?\n\r", ch);
		return FALSE;
	}

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

	if(IS_AFFECTED(ch, AFF_HIDE) || IS_AFFECTED(ch, AFF_INVISIBLE) || IS_AFFECTED(ch, AFF_SNEAK))
		do_visible(ch, "");

	if(!IS_SET(ch->act2, ACT2_RP_ING)) ch->xp_job_acts++;
	return TRUE;
}

int muso_play(CHAR_DATA *ch, char *argument)
{
	int door = 0;

	/* Play a riff (similar to pmote) */
	if(IS_NULLSTR(argument))
	{
		send_to_char("What are you going to play?\n\r", ch);
		return FALSE;
	}

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

	if(IS_AFFECTED(ch, AFF_HIDE) || IS_AFFECTED(ch, AFF_INVISIBLE) || IS_AFFECTED(ch, AFF_SNEAK))
		do_visible(ch, "");

	if(!IS_SET(ch->act2, ACT2_RP_ING)) ch->xp_job_acts++;
	return TRUE;
}

/* >>>>>>>>>>>>>>> Real Estate Commands <<<<<<<<<<<<<<< */
void do_home (CHAR_DATA *ch, char *argument)
{
	char arg[MIL]={'\0'};
	int cmd = -1;

	if(IS_NULLSTR(argument))
	{
		send_to_char("What do you want to do?\n\r", ch);
		send_to_char("(Use the commands option for a list of options.)\n\r",ch);
		return;
	}

	argument = one_argument(argument, arg);

	if((cmd = home_cmd_lookup(arg)) < 0)
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if((*home_cmd_table[cmd].func) ( ch, argument, 0, NULL ))
	{
		save_area( ch->in_room->area );
		WAIT_STATE(ch, home_cmd_table[cmd].delay);
	}

	return;
}

void do_orgbuild (CHAR_DATA *ch, char *argument)
{
	ORG_DATA *org;
	ORGMEM_DATA *mem;
	int cmd = -1;
	char arg[MIL]={'\0'};

	if(IS_NULLSTR(argument))
	{
		send_to_char("build [org] [command] [argument(s)]\n\r", ch);
		send_to_char("(Use the commands option for a list of options.)\n\r",ch);
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

	if(!IS_SET(mem->auth_flags, AUTH_BUILD))
	{
		send_to_char("You don't have the authority to build.\n\r", ch);
		return;
	}

	argument = one_argument(argument, arg);

	if((cmd = home_cmd_lookup(arg)) < 0)
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if((*home_cmd_table[cmd].func) ( ch, argument, 1, org ))
	{
		save_area( ch->in_room->area );
		WAIT_STATE(ch, home_cmd_table[cmd].delay);
	}

	return;
}

int home_commands(CHAR_DATA *ch, char *argument, int type, ORG_DATA *org)
{
	int cmd = 0;
	int col = 0;

	send_to_char("Available commands for building are:\n\r", ch);
	for( cmd = 0; home_cmd_table[cmd].name != NULL; cmd++ )
	{
		send_to_char(Format("%-12s | ", home_cmd_table[cmd].name), ch);
		if ( ++col % 4 == 0 )
			send_to_char( "\n\r", ch );
	}

	send_to_char("\n\r", ch);
	return TRUE;
}

int home_prices(CHAR_DATA *ch, char *argument, int type, ORG_DATA *org)
{
	int i = 0;
	long cost = 0;

	send_to_char(Format("+ %30s + %15s + %20s +\n\r", "", "", ""), ch);
	send_to_char("+--------------------------------------------------\n\r",ch);

	for(i = 0; home_price_table[i].name; i++)
	{
		cost = home_price_table[i].cost * ch->in_room->area->pricemod;
		cost = cost + (cost * tax_rate);
		send_to_char(Format("| %30s | %15s | %20ld |\n\r", home_price_table[i].name, home_price_table[i].type, cost), ch);
	}

	return TRUE;
}

int home_buy(CHAR_DATA *ch, char *argument, int type, ORG_DATA *org)
{
	char arg[MIL]={'\0'};
	int door = 0, vnum = 0, cost = 0, cash = 0;

	argument = one_argument(argument, arg);

	if(IS_NULLSTR(arg) || IS_NULLSTR(argument))
	{
		send_to_char("What are you trying to do?\n\r", ch);
		if(type == 0)
		{
			send_to_char("Syntax: home buy home <dir> - Buy the first room.\n\r", ch);
			send_to_char("        home buy room <dir> - Buy another room.\n\r", ch);
		}
		else
		{
			send_to_char("Syntax: build buy room <dir> - Buy a room.\n\r", ch);
		}
		return FALSE;
	}

	if(str_prefix(arg, "room") && str_prefix(arg, "home"))
	{
		send_to_char("What are you trying to do?\n\r", ch);
		return FALSE;
	}

	if(!type && !str_prefix(arg, "home")
		&& (!IS_NULLSTR(ch->in_room->owner)
			|| !IS_SET(ch->in_room->area->area_flags, AREA_SUBURB)))
	{
		send_to_char("You can't build here.\n\r", ch);
		return FALSE;
	}

	if(!type && !str_prefix(arg, "room")
		&& (!IS_SET(ch->in_room->room_flags, ROOM_HOME)
			|| str_cmp(ch->name, ch->in_room->owner)))
	{
		send_to_char("You can't build a room off here.\n\r", ch);
		return FALSE;
	}

	if(type && !IS_SET(ch->in_room->room_flags, ROOM_HOME) && org
		&& !IS_SET(ch->in_room->area->area_flags, AREA_SUBURB)
		&& (!IS_NULLSTR(ch->in_room->owner)
			|| str_cmp(ch->in_room->owner, org->name)))
	{
		send_to_char("You can't build a room off here.\n\r", ch);
		return FALSE;
	}

	if((door = find_dir(ch, argument)) < 0)
	{
		send_to_char("That's not a direction!\n\r", ch);
		return FALSE;
	}

	if(ch->in_room->exit[door])
	{
		send_to_char("Something's already been built there.\n\r", ch);
		return FALSE;
	}

	if(ch->home > 0)
	{
		cost = home_price_lookup("room");
	}
	else
	{
		cost = home_price_lookup("home");
	}

	if(cost <= 0)
	{
		send_to_char("There has been an error with the cost.\n\r", ch);
		send_to_char("Please notify a staff member.\n\r", ch);
		return FALSE;
	}

	cost = cost * ch->in_room->area->pricemod;
	cost = cost + (cost * tax_rate);

	if(type && org)
	{
		cash = org->funds;
	}
	else
	{
		cash = ch->dollars + ch->balance;
	}

	if(cost > cash)
	{
		send_to_char("You don't have the money for that!\n\r", ch);
		return FALSE;
	}

	if(ch->home == MAX_OWNED_ROOMS)
	{
		send_to_char("You've reached the maximum number of rooms.\n\r", ch);
		return FALSE;
	}

	if((vnum = gen_room(ch, org)) == 0)
	{
		send_to_char("Unable to create room.\n\r", ch);
		return FALSE;
	}

	if(!gen_exit(ch, door, vnum))
	{
		send_to_char("No exit created.\n\r", ch);
		return FALSE;
	}

	if(type)
		org->funds -= cost;
	else
	{
		if(cost <= ch->dollars)
		{
			ch->dollars -= cost;
		}
		else if(cost > ch->dollars)
		{
			cost -= ch->dollars;
			ch->dollars = 0;
			ch->balance -= cost;
		}
	}

	if(!type)
	{
		ch->rooms[ch->home] = get_room_index(vnum);
		ch->home++;
	}
	SET_BIT(ch->rooms[ch->home-1]->room_flags, ROOM_HOME);
	send_to_char("\n\rYou build a room...\n\r", ch);
	send_to_char("Kewl huh?\n\r", ch);
	return TRUE;
}

int home_sell(CHAR_DATA *ch, char *argument, int type, ORG_DATA *org)
{
	char arg[MIL]={'\0'};
	int i = 0, j = 0, cost = 0, pricemod = 0;
	int found = 0;
	CHAR_DATA *vch, *vch_next;

	if(IS_NULLSTR(argument))
	{
		send_to_char("What are you trying to do?\n\r", ch);
		if(type)
		{
			send_to_char("Syntax: build sell room - Sell the current room.\n\r", ch);
		}
		else
		{
			send_to_char("Syntax: home sell home - Sell the whole home.\n\r", ch);
			send_to_char("        home sell room - Sell the current room.\n\r", ch);
		}
		return FALSE;
	}

	argument = one_argument(argument, arg);

	if(str_prefix(arg, "room") && str_prefix(arg, "home"))
	{
		send_to_char("What are you trying to do?\n\r", ch);
		return FALSE;
	}

	if(ch->home <= 0
		|| (org && str_cmp(ch->in_room->owner, org->name)))
	{
		send_to_char("You can't sell something you don't own.\n\r", ch);
		return FALSE;
	}

	if(!str_prefix(arg, "room")
		&& (!IS_SET(ch->in_room->room_flags, ROOM_HOME)
			|| str_cmp(ch->name, ch->in_room->owner)
			|| (org && str_cmp(ch->in_room->owner, org->name))))
	{
		send_to_char("You can't sell a room that you don't own.\n\r", ch);
		return FALSE;
	}

	/* Dispose of room/home. */
	if(!str_prefix(arg, "room"))
	{
		pricemod = ch->in_room->area->pricemod;
		for(i = 0; i < ch->home; i++)
		{
			if(ch->rooms[i] == NULL && !found)
				break;

			if(ch->in_room == ch->rooms[i])
			{
				found = 1;
				for(j = i+1; j < ch->home; j++)
				{
					ch->rooms[i] = ch->rooms[j];
					i++;
				}
				ch->rooms[i] = NULL;

				/* Everybody out! */
				for(vch = ch->in_room->people; vch != NULL; vch = vch_next)
				{
					vch_next = vch->next_in_room;
					if(vch != ch)
					{
						char_from_room(vch);
						char_to_room(vch, get_room_index(ROOM_VNUM_START));
					}
				}

				/* Even the astral folks! */
				for(vch = ch->in_room->listeners; vch != NULL; vch = vch_next)
				{
					vch_next = vch->next_listener;
					if(vch != ch)
					{
						char_from_listeners(vch);
						char_to_listeners(vch, get_room_index(ROOM_VNUM_START));
					}
				}

				/* Byebye room! */
				free_room_index(ch->in_room);
				if(ch->rooms[0])
				{
					char_from_room(ch);
					char_to_room(ch, ch->rooms[0]);
				}
				else char_to_room(ch, get_room_index(ROOM_VNUM_START));
				ch->home--;
				break;
			}
		}
	}
	else
	{
		pricemod = ch->rooms[0]->area->pricemod;
		for(i = 0; i < ch->home; i++)
		{
			/* Everybody out! */
			for(vch = ch->in_room->people; vch != NULL; vch = vch_next)
			{
				vch_next = vch->next;
				if(vch != ch)
				{
					char_from_room(vch);
					char_to_room(vch, get_room_index(ROOM_VNUM_START));
				}
			}

			/* Even the astral folks! */
			for(vch = ch->in_room->listeners; vch != NULL; vch = vch_next)
			{
				vch_next = vch->next;
				if(vch != ch)
				{
					char_from_listeners(vch);
					char_to_listeners(vch, get_room_index(ROOM_VNUM_START));
				}
			}

			free_room_index(ch->rooms[i]);
			ch->rooms[i] = NULL;
		}
		found = ch->home;
		ch->home = 0;
	}

	if(!found)
	{
		send_to_char("Something buggy this way comes...\n\r", ch);
		send_to_char("Please contact a staff member.\n\r", ch);
	}
	else
	{
		send_to_char("You sell some real estate.\n\r", ch);
		cost = home_price_lookup("room") * pricemod * found;
		cost -= cost * tax_rate;
		ch->balance += cost;
	}

	return TRUE;
}

int home_furnish(CHAR_DATA *ch, char *argument, int type, ORG_DATA *org)
{
	if(IS_NULLSTR(argument))
	{
		send_to_char("What are you trying to do to your home?\n\r", ch);
		send_to_char("Syntax: home furnish list - List options.\n\r", ch);
		send_to_char("        home furnish <item> - Purchase furnishings.\n\r", ch);
		return FALSE;
	}

	return TRUE;
}

int home_unfurnish(CHAR_DATA *ch, char *argument, int type)
{
	if(IS_NULLSTR(argument))
	{
		send_to_char("What are you trying to do to your home?\n\r", ch);
		send_to_char("Syntax: home unfurnish <item> - Sell/remove furnishings.\n\r", ch);
		send_to_char("        home unfurnish all - Sell current room's furnishings.\n\r", ch);
		return FALSE;
	}

	return TRUE;
}

int home_place(CHAR_DATA *ch, char *argument, int type, ORG_DATA *org)
{
	if(IS_NULLSTR(argument))
	{
		send_to_char("What are you trying to do to your home?\n\r", ch);
		send_to_char("Syntax: home place <item> - Place item into room resets.\n\r", ch);
		send_to_char("(You must have the item in your inventory.)\n\r", ch);
		return FALSE;
	}

	return TRUE;
}

int home_remove(CHAR_DATA *ch, char *argument, int type, ORG_DATA *org)
{
	if(IS_NULLSTR(argument))
	{
		send_to_char("What are you trying to do to your home?\n\r", ch);
		send_to_char("Syntax: home remove <item> - Remove an item from the room resets.\n\r", ch);
		return FALSE;
	}

	return TRUE;
}

int home_name(CHAR_DATA *ch, char *argument, int type, ORG_DATA *org)
{
	if(IS_NULLSTR(argument))
	{
		send_to_char("What are you trying to name your home?\n\r", ch);
		send_to_char("Syntax: home name <string> - Set the name of the current room.\n\r", ch);
		return FALSE;
	}

	if(str_cmp(ch->name, ch->in_room->owner) || (org && str_cmp(ch->in_room->owner, org->name)))
	{
		send_to_char("You don't own this room.\n\r", ch);
		return FALSE;
	}

	PURGE_DATA(ch->in_room->name);
	ch->in_room->name = str_dup(argument);
	PURGE_DATA(ch->in_room->uname);
	ch->in_room->uname = str_dup(argument);
	send_to_char("Room name set.\n\r", ch);

	return TRUE;
}

int home_desc(CHAR_DATA *ch, char *argument, int type, ORG_DATA *org)
{
	if(!IS_NULLSTR(argument))
	{
		send_to_char("You don't need any argument for that.\n\r", ch);
		send_to_char("Syntax: home desc - Edit the description of the current room.\n\r",ch);
		return FALSE;
	}

	if(str_cmp(ch->name, ch->in_room->owner)
		|| (org && str_cmp(ch->in_room->owner, org->name)))
	{
		send_to_char("You don't own this room.\n\r", ch);
		return FALSE;
	}

	string_append( ch, &ch->in_room->description );

	return TRUE;
}

int home_list(CHAR_DATA *ch, char *argument, int type, ORG_DATA *org)
{
	int i = 0;

	if(type)
	{
		send_to_char("At this time it is not possible to list the rooms\n\r", ch);
		send_to_char("owned by an organisation.\n\r", ch);
		return FALSE;
	}

	if(ch->home == 0)
	{
		send_to_char("You don't own a home.\n\r", ch);
		return FALSE;
	}

	send_to_char("You currently own the following rooms:\n\r", ch);
	for(i = 0; i < ch->home; i++)
	{
		send_to_char( Format("%s\n\r", ch->rooms[i]->name), ch );
	}

	return FALSE;
}


#if defined(Macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <time.h>
#include "twilight.h"
#include "tables.h"

QUEST_DATA *quest_list;

DECLARE_DO_FUN( do_at		);

char *format_obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch, bool fShort );

DECLARE_QUEST_FUN( quest_none		);
DECLARE_QUEST_FUN( quest_courier	);
DECLARE_QUEST_FUN( quest_hitman		);
DECLARE_QUEST_FUN( quest_thief		);
DECLARE_QUEST_FUN( quest_bodyguard	);
DECLARE_QUEST_FUN( quest_guard		);
DECLARE_QUEST_FUN( quest_rescue		);

/* Quest recycling functions */
QUEST_DATA *	new_quest  args( () );
void		free_quest args( (QUEST_DATA * quest) );

/* Description generator. */
void		desc_gen   args( (CHAR_DATA * ch) );

const   struct  qt_type  quest_table[] =
{
		/* { time_offset, quest_function }, */
		{	0,	quest_none	},
		{	0,	quest_courier	},
		{	0,	quest_hitman	},
		{	0,	quest_thief	},
		{	0,	quest_bodyguard	},
		{	0,	quest_guard	},
		{	0,	quest_rescue	}
};

CHAR_DATA * evil_twin (CHAR_DATA *ch)
{
	CHAR_DATA * twin = create_mobile(get_mob_index(MOB_VNUM_BLANKY));
	int i = 0;

	twin->name		= ch->name;
	twin->short_descr	= ch->name;
	twin->act		= ACT_TWIN|ch->act;
	twin->act2		= ch->act2;
	for(i=0; i<MAX_STATS; i++)
	{
		twin->perm_stat[i]	= ch->perm_stat[i];
	}
	for(i=0; i<MAX_STATS; i++)
	{
		twin->mod_stat[i]	= ch->mod_stat[i];
	}
	twin->shape		= ch->shape;
	twin->blood_timer	= 10;
	twin->material	= ch->material;
	twin->race		= ch->race;
	twin->clan		= ch->clan;
	twin->size		= ch->size;
	twin->dam_type	= ch->dam_type;
	twin->affected_by	= ch->affected_by;
	twin->affected_by2	= ch->affected_by2;
	twin->imm_flags	= ch->imm_flags;
	twin->vuln_flags	= ch->vuln_flags;
	twin->res_flags	= ch->res_flags;
	twin->form		= ch->form;
	twin->parts		= ch->parts;
	for(i=0; i<MAX_ABIL; i++)
	{
		twin->ability[i].value = ch->ability[i].value;
	}
	for(i=0; i<MAX_DISC; i++)
	{
		twin->disc[i]	= ch->disc[i];
	}
	twin->health	= MAX_HEALTH;
	twin->agghealth	= MAX_HEALTH;
	twin->position	= P_STAND;
	twin->in_room	= get_room_index(10000);
	twin->RBPG		= ch->max_RBPG;
	twin->max_RBPG	= ch->max_RBPG;
	twin->GHB		= ch->max_GHB;
	twin->max_GHB	= ch->max_GHB;
	twin->willpower	= ch->max_willpower;
	twin->max_willpower = ch->max_willpower;
	twin->saving_throw	= ch->saving_throw;
	twin->sex		= ch->sex;
	twin->trust		= ch->trust;
	for(i=0; i<(MAX_VIRTUES - 1); i++)
	{
		twin->virtues[i] = ch->virtues[i];
	}
	desc_gen(twin);

	return twin;
}

/* Mob should not be able to be a PC. */
CHAR_DATA * get_random_mob(CHAR_DATA *ch)
{
	CHAR_DATA * mob = NULL;
	/* 3000 is simply an arbitrarily chosen integer */
	int i = number_range(1, 3000);

	while(i>0)
	{
		for(mob = char_list; mob != NULL; mob = mob->next)
		{
			if(mob->fighting == NULL && mob != ch)
			{
				i--;
			}
			if(i<=0)
			{
				break;
			}
		}
	}

	while(!IS_NPC(mob) || mob == ch)
	{
		mob = mob->next;
		if(mob == NULL)
		{
			mob = char_list;
		}
	}

	/* Mob should be on the same plane. */
	if(!SAME_PLANE(ch, mob) || IS_ANIMAL(mob))
	{
		mob = get_random_mob(ch);
	}

	return mob;
}

OBJ_DATA * get_random_obj()
{
	OBJ_INDEX_DATA *pobj = NULL;
	OBJ_DATA *obj = NULL;
	/* 3000 is simply an arbitrarily chosen integer */
	int i = number_range(1, 3000);

	while (i>0)
	{
		for(obj = object_list; obj != NULL; obj = obj->next)
		{
			i--;
			if(i<=0)
				break;
		}
	}

	pobj = obj->pIndexData;

	obj = create_object(pobj);

	return obj;
}

ROOM_INDEX_DATA * random_room()
{
	ROOM_INDEX_DATA * rm = NULL;
	/* 3000 is simply an arbitrarily chosen integer */
	int i = number_range(1, 3000);

	while (i>0)
	{
		for(rm = get_room_index(10000); rm != NULL; rm = rm->next)
		{
			if(rm->vnum > 15 && !IS_SET(rm->room_flags,ROOM_NO_RECALL)
					&& !IS_SET(rm->room_flags, ROOM_IMP_ONLY)
					&& !IS_SET(rm->room_flags, ROOM_ADMIN)
					&& !IS_SET(rm->room_flags, ROOM_NOWHERE))
				i--;
			if(i<=0)
				break;
		}
	}

	return rm;
}

/* @@@@@ In some cases the object should not be random. */
OBJ_DATA * gen_quest_object()
{
	return get_random_obj();
}

CHAR_DATA * get_questor()
{
	CHAR_DATA *ch;

	for(ch = char_list; ch != NULL; ch = ch->next)
	{
		if(!IS_NPC(ch) && ch->quest == NULL
				&& IS_SET(ch->act2, ACT2_RP_ING) && number_range(0, 1000) == 567)
			return ch;
		/* @@@@@ Test condition only. Remove.
	if(!str_cmp(ch->name, "Dsarky") && ch->quest == NULL)
	return ch; */
	}

	return NULL;
}

long gen_quest_flags()
{
	int num = 0;
	long result = 0;

	if((num = number_percent()) < 30)
		result = Q_EASY;
	else if(num > 70)
		result = Q_HARD;

	if((num = number_percent()) < 20)
		result = result|Q_ARREST;

	if((num = number_percent()) < 10 && !IS_SET(result, Q_HARD))
		result = result|Q_SAFE;
	else if(num > 80 && !IS_SET(result, Q_EASY))
		result = result|Q_DEADLY;

	if((num = number_percent()) < 30 && !IS_SET(result, Q_EASY))
		result = result|Q_PUZZLE;

	if((num = number_percent()) < 15 && !IS_SET(result, Q_EASY) && !IS_SET(result, Q_SAFE))
		result = result|Q_HUNTER;

	return result;
}

/* @@@@@ Need to finish these two functions.
 * In some cases, the mob should not be entirely random.
 * It is possible for duplicates (mob = questor for instance)
 */
CHAR_DATA * gen_quest_victim(QUEST_DATA *pq)
{
	CHAR_DATA *mob;

	mob = get_random_mob(pq->questor);
	while(IS_SET(mob->act2, ACT2_NOQUEST)
			|| IS_SET(mob->act2, ACT2_NOQUESTVICT))
		mob = get_random_mob(pq->questor);

	return mob;
}

CHAR_DATA * gen_quest_aggressor(QUEST_DATA *pq)
{
	CHAR_DATA *mob;

	mob = get_random_mob(pq->questor);
	while(IS_SET(mob->act2, ACT2_NOQUEST)
			|| IS_SET(mob->act2, ACT2_NOQUESTAGGR))
		mob = get_random_mob(pq->questor);

	return mob;
}

int gen_time_limit(QUEST_DATA *q, int fAccepted)
{
	int result = 0;

	if(q == NULL)
		return -1;

	if(!fAccepted)
	{
		result = number_range(15, 30);
		return result;
	}
	else
	{
		result = 3000 + 10*quest_table[q->quest_type].time_offset;
		if(IS_SET(q->quest_flags, Q_EASY))
			result += 1500;
		if(IS_SET(q->quest_flags, Q_HARD))
			result -= 600;
		if(IS_SET(q->quest_flags, Q_SAFE))
			result -= 1000;
		if(IS_SET(q->quest_flags, Q_DEADLY))
			result += 500;
		if(IS_SET(q->quest_flags, Q_ARREST))
			result -= 1000;
		if(IS_SET(q->quest_flags, Q_PUZZLE))
			result += 3000;
		if(IS_SET(q->quest_flags, Q_HUNTER))
			result -= 1500;
		return result;
	}
	return 100;
}

void gen_quest (bool forced, CHAR_DATA *vch)
{
	QUEST_DATA *pquest;

	pquest = new_quest();

	if(!forced)
	{
		if((pquest->questor = get_questor()) == NULL)
		{
			free_quest(pquest);
			return;
		}
	}
	else
	{
		pquest->questor = vch;
	}

	pquest->quest_type = number_range(0, MAX_QUESTS);
	if(!str_cmp(pquest->questor->name, "Dsarky")) pquest->quest_type = 2;

	pquest->quest_flags = gen_quest_flags();

	pquest->victim = gen_quest_victim(pquest);
	while(pquest->victim == pquest->questor)
		pquest->victim = gen_quest_victim(pquest);

	pquest->aggressor = gen_quest_aggressor(pquest);
	while(pquest->aggressor == pquest->questor
			|| pquest->aggressor == pquest->victim)
		pquest->aggressor = gen_quest_aggressor(pquest);

	pquest->aggressor->quest = pquest;
	pquest->victim->quest = pquest;

	pquest->time_limit = gen_time_limit(pquest, 0);
	pquest->obj = gen_quest_object();//pquest);
	pquest->questor->quest = pquest;

	LINK_SINGLE(pquest, next, quest_list);

	(*quest_table[pquest->quest_type].q_fun) (pquest->questor, 0);
}

void do_accept(CHAR_DATA *ch, char * argument)
{
	CheckCH(ch);

	if(ch->quest == NULL)
	{
		send_to_char("Nobody's offered you any jobs.\n\r", ch);
		return;
	}

	if(ch->quest->state > 0)
	{
		send_to_char("Aren't you still on a mission?\n\r", ch);
		return;
	}

	(*quest_table[ch->quest->quest_type].q_fun) (ch, 1);
}

void remove_quest(QUEST_DATA *q)
{
	UNLINK_SINGLE(q, next, QUEST_DATA, quest_list);
}

void quest_none (CHAR_DATA *ch, int flag)
{
	QUEST_DATA *pq = ch->quest;

	remove_quest(pq);
	free_quest(pq);
	ch->quest = NULL;
}

void quest_courier (CHAR_DATA *ch, int flag)
{
	OBJ_DATA *new_obj;

	switch(flag)
	{
	case 0 :
		/*package item*/
		new_obj = create_object(ch->quest->obj->pIndexData);
		clone_object(ch->quest->obj,new_obj);
		ch->quest->obj = new_obj;
		SET_BIT(ch->quest->obj->extra2, OBJ_PACKAGED);
		if(!IS_SET(ch->quest->obj->wear_flags, ITEM_TAKE))
			SET_BIT(ch->quest->obj->wear_flags, ITEM_TAKE);
		if(ch->quest->victim->carry_weight + get_obj_weight(ch->quest->obj)
				> can_carry_w(ch))
			ch->quest->obj = 0;
		/*aggressor makes offer;*/
		if(ch->quest->aggressor != NULL)
		{
			act("$N asks, \"Can you run $p to someone for me?\"\n\r", ch, ch->quest->obj, ch->quest->aggressor, TO_CHAR, 1);
		}
		else quest_none(ch,0);
		break;
	case 1 :
		/*acceptance of offer;*/
		if(ch->quest->aggressor != NULL)
		{
			send_to_char("You accept the courier job.\n\r", ch);
			if(ch->quest->victim != NULL)
			{
				act("$N says, \"I need you to run this to $t.\"", ch, PERS( ch->quest->victim, ch ),ch->quest->aggressor,TO_CHAR,1);
				act("$N gives you $p.\n\r", ch, ch->quest->obj, ch->quest->aggressor, TO_CHAR, 1);
				obj_to_char(ch->quest->obj, ch);
			}
			else
			{
				act("$N says, \"It doesn't matter anymore.\"", ch, NULL, ch->quest->aggressor, TO_CHAR, 1);
			}
		}
		else
		{
			act("The offer doesn't seem to be open anymore.", ch, NULL, NULL, TO_CHAR, 1);
			quest_none(ch,0);
		}
		break;
	case 2 :
		/*mission success;*/
		if(ch->quest->victim != NULL)
		{
			act("$N thanks you, giving you a COD payment.\n\r", ch, NULL, ch->quest->victim, TO_CHAR, 1);
		}
		else send_to_char("You succeed.\n\r", ch);
		ch->exp++;
		extract_obj(ch->quest->obj);
		quest_none(ch, 0);
		break;
	case 3 :
		/*mission fail;*/
		if(ch->quest->state == 0)
		{
			if(ch->quest->aggressor != NULL)
			{
				act( "$N says, \"I guess I'll just find someone else to do it.\"\n\r", ch, NULL, ch->quest->aggressor, TO_CHAR, 1);
			}
		}
		else send_to_char("You fail to deliver the package.\n\r", ch);
		extract_obj(ch->quest->obj);
		quest_none(ch, 0);
		break;
	}

	if(ch->quest != NULL) ch->quest->state = flag;

}

void quest_hitman (CHAR_DATA *ch, int flag)
{

	switch(flag)
	{
	case 0 :
		/*aggressor makes offer;*/
		if(ch->quest->aggressor != NULL)
		{
			act("$N asks, \"I need a professional hit... interested?\"\n\r", ch, NULL, ch->quest->aggressor, TO_CHAR, 1);
		}
		else quest_none(ch,0);
		break;
	case 1 :
		/*acceptance of offer;*/
		if(ch->quest->aggressor != NULL)
		{
			send_to_char("You accept the hitman job.\n\r", ch);
			if(ch->quest->victim != NULL)
			{
				act("$N says, \"I need you to rub out $t.\"", ch, PERS( ch->quest->victim, ch ),ch->quest->aggressor,TO_CHAR,1);
			}
			else
			{
				act("$N says, \"It doesn't matter anymore.\"", ch, NULL, ch->quest->aggressor, TO_CHAR,1);
			}
		}
		else
		{
			act("The offer doesn't seem to be open anymore.", ch, NULL, NULL, TO_CHAR, 1);
			quest_none(ch,0);
		}
		break;
	case 2 :
		/*mission success;*/
		if(ch->quest->aggressor != NULL)
		{
			act("$N thanks you, delivering payment into your account.\n\r", ch, NULL, ch->quest->aggressor, TO_CHAR, 1);
		}
		else send_to_char("You succeed.\n\r", ch);
		ch->exp++;
		quest_none(ch, 0);
		break;
	case 3 :
		/*mission fail;*/
		if(ch->quest->state == 0)
		{
			if(ch->quest->aggressor != NULL)
			{
				act( "$N says, \"I guess I'll just find someone else to do it.\"\n\r", ch, NULL, ch->quest->aggressor, TO_CHAR, 1);
			}
		}
		else send_to_char("You fail to eliminate the target.\n\r", ch);
		quest_none(ch, 0);
		break;
	}

	if(ch->quest != NULL) ch->quest->state = flag;

}

void quest_thief (CHAR_DATA *ch, int flag)
{
	OBJ_DATA *new_obj;

	switch(flag)
	{
	case 0 :
		/*package item*/
		new_obj = create_object(ch->quest->obj->pIndexData);
		clone_object(ch->quest->obj,new_obj);
		ch->quest->obj = new_obj;
		SET_BIT(ch->quest->obj->extra2, OBJ_PACKAGED);
		if(!IS_SET(ch->quest->obj->wear_flags, ITEM_TAKE))
			SET_BIT(ch->quest->obj->wear_flags, ITEM_TAKE);
		if(ch->quest->victim->carry_weight + get_obj_weight(ch->quest->obj)
				> can_carry_w(ch))
			ch->quest->obj = 0;
		/*aggressor makes offer;*/
		if(ch->quest->aggressor != NULL)
		{
			act("$N asks, \"Can you... acquire something for me?\"\n\r", ch, NULL, ch->quest->aggressor, TO_CHAR, 1);
		}
		else quest_none(ch,0);
		break;
	case 1 :
		/*acceptance of offer;*/
		if(ch->quest->aggressor != NULL)
		{
			send_to_char("You accept the theft job.\n\r", ch);
			if(ch->quest->victim != NULL)
			{
				act(Format("$N says, \"I need you to steal %s from $t.\"", format_obj_to_char( ch->quest->obj, ch, TRUE )), ch, PERS( ch->quest->victim, ch ),ch->quest->aggressor,TO_CHAR,1);
				act("$N gives you $p.\n\r", ch->quest->victim,ch->quest->obj,ch->quest->aggressor, TO_CHAR, 1);
				obj_to_char(ch->quest->obj, ch->quest->victim);
			}
			else
			{
				act("$N says, \"It doesn't matter anymore.\"", ch, NULL, ch->quest->aggressor, TO_CHAR, 1);
			}
		}
		else
		{
			act("The offer doesn't seem to be open anymore.", ch, NULL, NULL, TO_CHAR, 1);
			quest_none(ch,0);
		}
		break;
	case 2 :
		/*mission success;*/
		if(ch->quest->aggressor != NULL)
		{
			act("$N thanks you, giving you a COD payment.\n\r", ch, NULL, ch->quest->aggressor, TO_CHAR, 1);
		}
		else send_to_char("You steal the item!\n\r", ch);
		ch->exp++;
		extract_obj(ch->quest->obj);
		quest_none(ch, 0);
		break;
	case 3 :
		/*mission fail;*/
		if(ch->quest->state == 0)
		{
			if(ch->quest->aggressor != NULL)
			{
				act( "$N says, \"I guess I'll just find someone else to do it.\"\n\r", ch, NULL, ch->quest->aggressor, TO_CHAR, 1);
			}
		}
		else send_to_char("You fail to acquire the item.\n\r", ch);
		extract_obj(ch->quest->obj);
		quest_none(ch, 0);
		break;
	}

	if(ch->quest != NULL) ch->quest->state = flag;

}

void quest_bodyguard (CHAR_DATA *ch, int flag)
{

	switch(flag)
	{
	case 0 :
		/*victim makes offer;*/
		send_to_char("You are offered a bodyguard job.\n\r", ch);
		break;
	case 1 :
		/*acceptance of offer;*/
		send_to_char("You accept the bodyguard job.\n\r", ch);
		break;
	case 2 :
		/*mission success;*/
		send_to_char("You succeed.\n\r", ch);
		quest_none(ch, 0);
		break;
	case 3 :
		/*mission fail;*/
		send_to_char("You fail.\n\r", ch);
		quest_none(ch, 0);
		break;
	}

	if(ch->quest != NULL) ch->quest->state = flag;

}

void quest_guard (CHAR_DATA *ch, int flag)
{

    switch(flag)
    {
	case 0 :
	    /*victim makes offer;*/
	    send_to_char("You are offered a guard job.\n\r", ch);
	    break;
	case 1 :
	    /*acceptance of offer;*/
	    send_to_char("You accept the guard job.\n\r", ch);
	    break;
	case 2 :
	    /*mission success;*/
	    send_to_char("You succeed.\n\r", ch);
	    quest_none(ch, 0);
	    break;
	case 3 :
	    /*mission fail;*/
	    send_to_char("You fail.\n\r", ch);
	    quest_none(ch, 0);
	    break;
    }

    if(ch->quest != NULL) ch->quest->state = flag;

}

void quest_rescue (CHAR_DATA *ch, int flag)
{

    switch(flag)
    {
	case 0 :
	    /*"associated" mob makes offer;*/
	    send_to_char("You are offered a rescue mission.\n\r", ch);
	    break;
	case 1 :
	    /*acceptance of offer;*/
	    send_to_char("You accept the rescue mission.\n\r", ch);
	    break;
	case 2 :
	    /*mission success;*/
	    send_to_char("You succeed.\n\r", ch);
	    quest_none(ch, 0);
	    break;
	case 3 :
	    /*mission fail;*/
	    send_to_char("You fail.\n\r", ch);
	    quest_none(ch, 0);
	    break;
    }

    if(ch->quest != NULL) ch->quest->state = flag;

}

void goal_obtain_item(CHAR_DATA *ch, int flag)
{

    switch(flag)
    {
	case 0 :
	    /*"associated" mob makes offer;*/
	    send_to_char("You are asked to obtain an item of quality.\n\r", ch);
	    break;
	case 1 :
	    /*acceptance of offer;*/
	    send_to_char("You accept the request.\n\r", ch);
	    break;
	case 2 :
	    /*mission success;*/
	    send_to_char("You succeed.\n\r", ch);
	    quest_none(ch, 0);
	    break;
	case 3 :
	    /*mission fail;*/
	    send_to_char("You fail.\n\r", ch);
	    quest_none(ch, 0);
	    break;
    }

    if(ch->quest != NULL) ch->quest->state = flag;

}

void do_random_at(CHAR_DATA *ch, char *argument)
{
	char arg2[MAX_INPUT_LENGTH]={'\0'};
	CHAR_DATA *mob = get_random_mob(ch);

	if(!IS_NULLSTR(argument))
	{
		/*gen_quest(TRUE, ch);*/
		return;
	}

	one_argument(mob->name, arg2);
	do_at(ch, (char *)Format("%s %s", arg2, argument));
}

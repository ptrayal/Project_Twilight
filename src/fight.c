/*
This file is (surprise surprise) completely original.
Use of the code contained in this file is allowed so long as I get credit for it.
Thanks to everyone who's sharin' code.
Peter aka. One Thousand Monkeys
 */

#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#endif

#include "twilight.h"
#include "tables.h"
#include "db.h"
#include "recycle.h"
#include "lookup.h"

#define DAMAGE_DIFFICULTY 6
#define SOAK_DIFFICULTY 4

/* Damage functions */
int do_soak (CHAR_DATA *victim, int damage, int aggravated);
void firearm_dam(CHAR_DATA *ch, CHAR_DATA *victim, int damage, BODY_DATA *target);

/* Botch functions */
void do_botch (CHAR_DATA *ch, int combo);
void firearm_botch(CHAR_DATA *ch);
void melee_botch(CHAR_DATA *ch, int combo);
void hit_botch(CHAR_DATA *ch, int combo);

int dice_rolls(CHAR_DATA *ch, int number, int difficulty);
bool hit_blocked(at_part_type strike_part, long flags);
int combo_strike_part(int combo);
void be_frenzied(CHAR_DATA *ch);

/* FIREARM WEAPON VALUES: 0 = type, 1 = diff, 2 = ammo vnum,
 *     3 = ammo current, 4 = ammo max
 * AMMO WEAPON VALUES: 0 = rounds, 1 = dam, 2 = range, 3 = type, 4 = agg
 */
void do_shoot (CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	bool not_in_room = FALSE;
	char arg[MIL]={'\0'};
	int dir = 0;
	OBJ_DATA *obj = get_eq_char(ch, WEAR_WIELD);
	OBJ_INDEX_DATA *ammo;

	CheckCH(ch);

	argument = one_argument(argument, arg);

	if(ch->combat_flag == -1 && ch->position == P_FIGHT)
	{
		send_to_char("You have cancelled your move for this combat round.\n\r",ch);
		return;
	}

	if(part_in_use(ch, MOVE_RBLOW))
	{
		send_to_char("You've already made use of that part this round!\n\r",ch);
		return;
	}

	if(obj==NULL)
	{
		send_to_char("You aren't wielding a weapon.\n\r", ch);
		return;
	}

	if(obj->value[0] != WEAPON_FIREARM)
	{
		send_to_char("You ain't totin' no gun mister!\n\r",ch);
		return;
	}

	if(obj->value[3] <= 0)
	{
		send_to_char("*Click* You're out of ammo!\n\r", ch);
		WAIT_STATE(ch, 3);
		return;
	}

	if((victim = get_char_room(ch, arg)) == NULL)
	{
		not_in_room = TRUE;
	}

	if(not_in_room == TRUE)
	{
		if(!str_prefix(arg, "north")) dir = 0;
		else if (!str_prefix(arg, "east")) dir = 1;
		else if (!str_prefix(arg, "south")) dir = 2;
		else if (!str_prefix(arg, "west")) dir = 3;
		else if (!str_prefix(arg, "up")) dir = 4;
		else if (!str_prefix(arg, "down")) dir = 5;
		else
		{
			send_to_char("Which direction are you trying to shoot in?\n\r", ch);
			return;
		}

		if((victim = get_char_dir(ch, dir, argument)) == NULL)
		{
			send_to_char("They aren't here.\n\r", ch);
			return;
		}

		if(IS_AFFECTED(ch, AFF_CHARM) && victim == ch->master)
		{
			send_to_char("You can't attack your dear master!\n\r", ch);
			return;
		}

		if(IS_SET(victim->act2, ACT2_RP_ING) && ch->fighting == NULL)
		{
			send_to_char("You cannot use ooc combat on a person in character.\n\r", ch);
			return;
		}

		if((ammo = get_obj_index(obj->value[2])) == NULL)
		{
			send_to_char("There's something wrong with it... it's as if the ammo's wrong.\n\r", ch);
			return;
		}

		if(not_in_room == TRUE && 2 * ammo->value[2] < range(ch, victim))
		{
			send_to_char("They aren't in range.\n\r", ch);
			return;
		}
	}

	if(victim == ch)
	{
		send_to_char("NO! Don't do it!\n\r", ch);
		return;
	}

	if(IS_SET(victim->act2, ACT2_RP_ING) && ch->fighting == NULL)
	{
		send_to_char("You cannot use ooc combat on a person in character.\n\r", ch);
		return;
	}

	WAIT_STATE(ch, 3);
	shooting(ch, victim, NULL);
	if(ch->in_room == victim->in_room)
	{
		if(ch->fighting == NULL) ch->fighting = victim;
		if(victim->fighting == NULL) victim->fighting = ch;
	}
}

int aggravated(CHAR_DATA *ch, CHAR_DATA *vch, void *pObj, int combo)
{
	OBJ_DATA *obj = (OBJ_DATA *) pObj;
	OBJ_INDEX_DATA *ammo = NULL;

	if(obj != NULL)
	{
		if(obj->item_type == ITEM_WEAPON && obj->value[0] != WEAPON_FIREARM)
		{
			switch(material_lookup(obj->material))
			{
			case 0 : return material_table[0].agg; break;
			}
			if(attack_table[obj->value[3]].agg) return 1;
		}

		if(obj->value[0] == WEAPON_FIREARM)
		{
			ammo = get_obj_index(obj->value[2]);
			if(attack_table[ammo->value[3]].agg) return 1;
			if(ammo->value[4] > 0) return ammo->value[4];
		}

		if(obj->value[0] != WEAPON_FIREARM && obj->material != NULL
				&& !str_cmp(obj->material, "silver")
				&& vch->race == race_lookup("werewolf"))
			return 1;

		if(ammo != NULL
				&& !IS_NULLSTR(ammo->material) && !str_cmp(ammo->material, "silver")
				&& vch->race == race_lookup("werewolf"))
			return 1;
	}

	if(combo > -1) {
		if(IS_AFFECTED(ch, AFF_CLAWED)) return 1;
		if(attack_table[ch->dam_type].agg) return 1;
	}

	return 0;
}

void shooting (CHAR_DATA *ch, CHAR_DATA *victim, BODY_DATA *target)
{
	OBJ_DATA *obj = get_eq_char(ch, WEAR_WIELD);
	OBJ_INDEX_DATA *ammo;
	int diff = 0, fail = 0, dam = 0, agg = 0;
	int dist = range(ch,victim);
	int dt = 0, dt1 = 0;

	if((ammo = get_obj_index(obj->value[2])) == NULL)
	{
		send_to_char("There's something wrong with the ammo...\n\r", ch);
		return;
	}

	if(obj->value[3] <= 0)
	{
		send_to_char("*Click* You're out of ammo!\n\r", ch);
		WAIT_STATE(ch, 3);
		return;
	}

	if (target != NULL) diff = diff + 3;
	diff = diff + obj->value[1];
	if(victim->race == race_lookup("faerie")
			&& !str_cmp(ammo->material, "iron"))
	{
		dt = attack_table[DAM_IRON].damage;
		dt1 = TYPE_HIT+DAM_IRON;
	}
	else
	{
		dt = attack_table[ammo->value[3]].damage;
		dt1 = TYPE_HIT+ammo->value[3];
	}
	agg = aggravated(ch, victim, (void *)obj, 0);

	if (can_see(ch,victim))
	{
		if ((ammo->value[2] < dist) && (dist < (2 * obj->value[2])))
			diff = diff + 1;
		else if (dist > (2 * obj->value[2]))
		{
			send_to_char("Your bullet fails to reach its target.\n\r", ch);
			obj->value[3]--;
			return;
		}
		diff = UMIN(10, diff);
		fail = dice_rolls(ch, (get_curr_stat(ch, STAT_DEX) + ch->ability[FIREARMS].value), diff);

		act("You shoot at $N.", ch, NULL, victim, TO_CHAR,1);
		if(ch->in_room != victim->in_room)
		{
			act("A shot flies in, striking you.", victim, NULL, ch, TO_CHAR,1);
			act("A shot flies in, striking $n.", victim, NULL, ch, TO_ROOM,1);
		}
		else
		{
			act("$N shoots at you.", victim, NULL, ch, TO_CHAR, 1);
			act("$n shoots at $N.", ch, NULL, victim, TO_NOTVICT, 0);
		}

		if (fail > 0)
		{
			dam = dice_rolls(ch, (ammo->value[1] + fail - 1), DAMAGE_DIFFICULTY);
			if (dam > 0)
			{
				if (target != NULL)
				{
					damage(ch,victim,dam,dt1,dt,TRUE,agg,9999);
				}
				else
				{
					damage(ch,victim,dam,dt1,dt,TRUE,agg,9999);
				}
			}
		}
		if (fail == 0)
		{
			send_to_char(Format("You missed %s.", IS_NPC(victim) ? victim->short_descr : victim->name), ch);
			if( ch->in_room != victim->in_room )
			{
				act("$n fires a shot at a target some distance away.", ch, NULL, NULL, TO_ROOM, 0);
			}
			if( (ch->in_room == victim->in_room) && (can_see(victim,ch)) )
			{
				send_to_char(Format("%s shot at you.\n\r", IS_NPC(ch) ? ch->short_descr : ch->name), victim);
			}
			else
			{
				send_to_char(Format("Someone shot at you.\n\r"), victim);
			}
		}
		if (fail < 0)
		{
			do_botch(ch, -1);
		}
	}
	obj->value[3]--;
}

int combo_damage(CHAR_DATA *ch, int combo)
{
	if(combo > -1)
	{
		return (get_curr_stat(ch, STAT_STR) + combo_table[combo].damage);
	}

	return get_curr_stat(ch, STAT_STR);
}

void close_combat (CHAR_DATA *ch, CHAR_DATA *victim, int combo)
{
	OBJ_DATA *obj = get_eq_char(ch, WEAR_WIELD);
	int diff = 0, fail = ch->combo_success, dam = 0, agg = 0;
	int dist = range(ch,victim);
	int pos = -1;
	int dt = 0, dt1 = 0;
	if(obj != NULL) {
		if(victim->race == race_lookup("faerie") && !str_cmp(obj->material, "iron"))
		{
			dt = attack_table[DAM_IRON].damage;
			dt1 = TYPE_HIT+DAM_IRON;
		}
		else
		{
			dt = attack_table[obj->value[3]].damage;
			dt1 = TYPE_HIT + obj->value[3];
		}
	} 
	else
	{
		dt = attack_table[ch->dam_type+1].damage;
		dt1 = TYPE_HIT + ch->dam_type+1;
	}

	if((combo != 4 || combo != 5)
			&& (combo < 8 || combo > 10)
			&& (combo < 15 || combo > 35))
		return;

	if(obj != NULL && obj->item_type == ITEM_WEAPON
			&& obj->value[0] == WEAPON_FIREARM)
	{
		shooting(ch, victim, NULL);
		return;
	}

	if(IS_SET(victim->form, FORM_MIST) || IS_SET(victim->form, FORM_INTANGIBLE))
	{
		send_to_char("You can't do any damage to a mist!", ch);
		act("$n's blow passes right through $N.", ch,NULL,victim,TO_NOTVICT,0);
		act("$n's blow passes right through you.", ch,NULL,victim,TO_VICT,1);
		return;
	}

	if(IS_SET(victim->form, FORM_BLOOD))
	{
		send_to_char("You can't do any damage to a puddle!", ch);
		act("$n's blow passes right through $N.", ch,NULL,victim,TO_NOTVICT,0);
		act("$n's blow passes right through you.", ch,NULL,victim,TO_VICT,1);
		return;
	}

	if(IS_SET(victim->form, FORM_SHADOW) && dt != DAM_FIRE)
	{
		send_to_char("You can't do any damage to a shadow!", ch);
		act("$n's blow passes right through $N.", ch,NULL,victim,TO_NOTVICT,0);
		act("$n's blow passes right through you.", ch,NULL,victim,TO_VICT,1);
		return;
	}

	if(hit_blocked((at_part_type)combo_strike_part(combo), victim->combat_flag))
	{
		act("$N blocks your strike!",ch, NULL, victim, TO_CHAR,1);
		act("You block $n's strike!",ch, NULL, victim, TO_VICT,1);
		act("$N blocks $n's strike!",ch, NULL, victim, TO_NOTVICT,0);
		victim->condition[COND_PAIN] += 3;
		return;
	}

	if (obj != NULL && obj->item_type == ITEM_WEAPON && !combo_table[combo].footstrike)
	{
		diff = obj->value[1] + combo_table[combo].diff + ch->balance;
		if (diff > 10) diff = 10;
		if (diff < 5) diff = 5;
		agg = aggravated(ch, victim, obj, combo);
	}
	else
	{
		diff = 6 + combo_table[combo].diff + ch->balance;
		if (diff > 10) diff = 10;
		if (diff < 4) diff = 4;
		agg = aggravated(ch, victim, obj, combo);
	}

	if (can_see(ch,victim))
	{
		if (dist > 0)
		{
			send_to_char("You aren't close enough to hit them!\n\r", ch);
			fail = 0;
			return;
		}
		if(IS_NPC(ch))
		{
			if (obj != NULL && dist == 0 && !combo_table[combo].footstrike)
				fail = dice_rolls(ch, (get_curr_stat(ch, STAT_DEX) + ch->ability[MELEE].value), diff);
			else
				fail = dice_rolls(ch, (get_curr_stat(ch, STAT_DEX) + ch->ability[BRAWL].value), diff);
		}
		if (fail > 0)
		{
			if (obj != NULL && !combo_table[combo].footstrike)
			{
				dam = dice_rolls(ch, (obj->value[2] + combo_damage(ch, combo)), DAMAGE_DIFFICULTY);
				if (dam > 0)
				{
					pos = damage(ch,victim,dam,dt1,dt,TRUE,agg,combo);
					victim->condition[COND_FRENZY] += UMAX(1, victim->condition[COND_PAIN] + victim->condition[COND_ANGER] - 30);
					gain_condition(ch, COND_ANGER, -5);
					if(get_points(ch) > get_points(victim))
					{
						gain_condition(victim, COND_FEAR, 5);
						gain_condition(victim, COND_FRENZY, victim->condition[COND_FEAR]);
					}
				}
			}
			else if (obj == NULL || obj->item_type != ITEM_WEAPON)
			{
				dam = dice_rolls(ch, combo_damage(ch, combo), DAMAGE_DIFFICULTY);
				if (dam > 0)
				{
					if(ch->race==race_lookup("werewolf")
							&& ch->auspice==auspice_lookup("ahroun")
							&& ch->disc[DISC_AUSPICE] >1)
						dam++;
					pos = damage(ch,victim, dam, dt1, dt, TRUE, agg, combo);
					victim->condition[COND_FRENZY] += UMAX(1, victim->condition[COND_PAIN] + victim->condition[COND_ANGER] - 30);
					gain_condition(ch, COND_ANGER, -5);
					if(get_points(ch) > get_points(victim))
					{
						gain_condition(victim, COND_FEAR, 5);
						gain_condition(victim, COND_FRENZY, victim->condition[COND_FEAR]);
					}
				}
			}
			victim->condition[COND_PAIN] += 5;
		}
		if (fail == 0)
		{
			send_to_char(Format("You missed %s.\n\r", IS_NPC(victim) ? victim->short_descr : victim->name), ch);
			if( (can_see(victim,ch)) )
			{
				send_to_char(Format("%s missed you.\n\r", IS_NPC(ch) ? ch->short_descr : ch->name), victim);
			}
			else
			{
				send_to_char(Format("Someone took a swing at you.\n\r"), victim);
			}
			pos = P_FIGHT;
		}
		if (fail < 0)
		{
			do_botch(ch, combo);
			pos = P_FIGHT;
			ch->condition[COND_ANGER] += 3;
		}
	}

	if(pos == P_FIGHT)
	{
		if( victim != ch )
		{
			if(victim->position > P_STUN)
			{
				if(victim->fighting == NULL)
					set_fighting( victim, ch );
				if(victim->timer <= 4)
					victim->position = P_FIGHT;
			}

			if(victim->position > P_STUN)
			{
				if(ch->fighting == NULL)
					set_fighting( ch, victim );
			}

			if(victim->master == ch)
			{
				stop_follower( victim );
			}
		}
	}

}

int mob_combo(CHAR_DATA *ch)
{
	int i = 5, k = 0;

	if(get_eq_char(ch, WEAR_WIELD))
		switch(ch->ability[MELEE].value)
		{
		case 0:
		case 1:
			if(number_range(1,2) == 1 )
				i = 5;
			else
				i = 4;
			break;
		case 2:
		case 3:
			if((k = number_range(1,4)) == 1)
				i = 5;
			else if(k == 2)
				i = 4;
			else if(k == 3)
				i = 22;
			else if(k == 4)
				i = 23;
			break;
		case 4:
		case 5:
		default :
			if((k = number_range(1,8)) == 1)
				i = 5;
			else if(k == 2)
				i = 4;
			else if(k == 3)
				i = 8;
			else if(k == 4)
				i = 9;
			else if(k == 5)
				i = 22;
			else if(k == 6)
				i = 23;
			else if(k == 7)
				i = 25;
			else if(k == 8)
				i = 26;
			break;
		}
	else
		switch(ch->ability[BRAWL].value)
		{
		case 0:
		case 1:
			if(number_range(1,2) == 1 )
				i = 5;
			else
				i = 4;
			break;
		case 2:
		case 3:
			if((k = number_range(1,4)) == 1)
				i = 5;
			else if(k == 2)
				i = 4;
			else if(k == 3)
				i = 8;
			else if(k == 4)
				i = 9;
			break;
		case 4:
		case 5:
		default :
			if((k = number_range(1,8)) == 1)
				i = 5;
			else if(k == 2)
				i = 4;
			else if(k == 3)
				i = 8;
			else if(k == 4)
				i = 9;
			else if(k == 5)
				i = 17;
			else if(k == 6)
				i = 18;
			else if(k == 7)
				i = 25;
			else if(k == 8)
				i = 26;
			break;
		}

	return i;
}

int get_combo(CHAR_DATA *ch)
{
	int i=0,move_list[4];
	COMBAT_DATA *pmove;
	COMBAT_DATA *next;

	for(i=0;i<4;i++)
	{
		move_list[i] = -2;
	}

	if(ch->combat_flag == -1 && !IS_SET(ch->act2, ACT2_FRENZY))
	{
		if(ch->combo != NULL)
		{
			while(ch->combo != NULL && ch->combo->next != NULL)
			{
				pmove = ch->combo;
				ch->combo = ch->combo->next;
				free_combat_move(pmove);
				if(ch->combo == NULL)
					break;
			}
			if(ch->combo != NULL || ch->combo->next != NULL)
			{
				pmove = ch->combo;
				ch->combo = ch->combo->next;
				free_combat_move(pmove);
			}
			ch->combat_flag = 0;
		}
		return -1;
	}

	if(ch->combo == NULL || IS_SET(ch->act2, ACT2_FRENZY))
	{
		if(!IS_NPC(ch) && !IS_SET(ch->act2, ACT2_FRENZY))
			return -1;
		else
			return mob_combo(ch);
	}

	i = 0;
	for(pmove = ch->combo; pmove != NULL; pmove = next)
	{
		next = pmove->next;

		if(i<4)
			move_list[i] = pmove->move;
		else
			break;

		if(combat_move_table[pmove->move].endmove == TRUE || pmove == NULL)
		{
			while(ch->combo != NULL)
			{
				pmove = ch->combo;
				ch->combo = ch->combo->next;
				free_combat_move(pmove);
			}
			break;
		}

		i++;
	}

	for(i=0;combo_table[i].to_char !=NULL;i++)
	{
		if(move_list[0] == combo_table[i].moves[0])
			if(move_list[1] == combo_table[i].moves[1])
				if(move_list[2] == combo_table[i].moves[2])
					if(move_list[3] == combo_table[i].moves[3])
						return i;
	}

	return 0;
}

void strike(CHAR_DATA *ch, CHAR_DATA *vch)
{
	int combo = get_combo(ch);

	if(ch == NULL)
	{
		return;
	}
	if(vch == NULL)
	{
		return;
	}

	if(combo > -1)
	{
		close_combat(ch, vch, combo);
		if(vch->fighting == NULL && IS_NPC(vch))
			set_fighting(vch, ch);
		if((NEAR_FRENZY(vch) && !dice_rolls(vch, vch->virtues[1], ch->clan == clan_lookup("brujah") ? 8 : 7))
				|| IS_SET(vch->act2, ACT2_FRENZY))
			be_frenzied(vch);
		if((NEAR_FRENZY(ch) && !dice_rolls(ch, ch->virtues[1], ch->clan == clan_lookup("brujah") ? 7 : 6))
				|| IS_SET(ch->act2, ACT2_FRENZY))
			be_frenzied(ch);
		ch->condition[COND_FRENZY] += UMAX(1, ch->condition[COND_PAIN] + ch->condition[COND_ANGER] - 30);
		return;
	}
	else
		return;
}

int do_soak (CHAR_DATA *victim, int damage, int aggravated)
{
	int reduction = 0;

	if(is_affected(victim, skill_lookup("skin of the adder")))
		reduction = SOAK_DIFFICULTY - 5;

	if (victim->race == race_lookup("vampire"))
		damage = damage - dice_rolls(victim,
				(get_curr_stat(victim, STAT_STA) + victim->disc[DISC_FORTITUDE]),
				(SOAK_DIFFICULTY + aggravated - reduction));
	else if (victim->race == race_lookup("werewolf"))
		damage = damage - dice_rolls(victim,
				(get_curr_stat(victim, STAT_STA) + victim->RBPG),
				(SOAK_DIFFICULTY + aggravated - reduction));
	else
		damage = damage;
	return damage;
}

void do_botch (CHAR_DATA *ch, int combo)
{
	OBJ_DATA *obj = get_eq_char(ch, WEAR_WIELD);

	if(obj != NULL)
	{
		switch (obj->value[0]) {
		case WEAPON_FIREARM : firearm_botch(ch); break;
		case WEAPON_BLADE   : melee_botch(ch, combo);   break;
		case WEAPON_BLUNT   : melee_botch(ch, combo);   break;
		case WEAPON_WHIP    : melee_botch(ch, combo);   break;
		case WEAPON_GRAPPLE : melee_botch(ch, combo);   break;
		default             : hit_botch(ch, combo);   break;
		}
	}
	else
	{
		hit_botch(ch,combo);
	}
}


int dice_rolls ( CHAR_DATA *ch, int dice, int difficulty )
{
	int i, k = 0, a, health = 0, max = 500;

	dice += ch->dice_mod;
	difficulty += ch->diff_mod;

	if(ch->race == race_lookup("vampire") && !IS_NIGHT())
	{
		dice -= 3;
		max = ch->GHB;
	}

	health = ch->health + ch->agghealth - 7;

	if(!IS_AFFECTED(ch, AFF_RESIST_PAIN))
	{
		if(health == 6 || health == 5) dice--;
		else if(health == 4 || health == 3) dice -= 2;
		else if(health == 2 || health == 1) dice -= 5;
		else if(health <= 0) dice = 0;
	}

	if(ch->fighting != NULL)
		if(IS_AFFECTED(ch->fighting, AFF_ODOUR))
		{
			if(difficulty < 10)
				difficulty++;
		}

	if(dice <= 0) dice = 1;

	for(i = 0; i < dice; i++)
	{
		a = number_range(1, 10);
		if(IS_SET(ch->off_flags, LOADED_DICE_POS))
			a = UMAX(number_fuzzy(a), a);
		if(IS_SET(ch->off_flags, LOADED_DICE_NEG))
			a = UMIN(number_fuzzy(a), a);
		if (a >= difficulty) k++;
		else if (a == 1) k--;
	}

	if(health <= 0) k = 0;

	return UMIN(k,max);
}

void melee_botch ( CHAR_DATA *ch, int combo )
{
}

void firearm_botch ( CHAR_DATA *ch )
{
}

void hit_botch ( CHAR_DATA *ch, int combo )
{
}

int range (CHAR_DATA *ch, CHAR_DATA *victim)
{
	ROOM_INDEX_DATA *scan_room;
	EXIT_DATA *pExit;
	int door, depth, vision;

	if(ch->in_room == victim->in_room)
		return 0;

	vision = get_curr_stat(ch, STAT_PER);
	if(IS_SET(ch->affected_by, AFF_HSENSES))
		vision = vision * 2;
	if(ch->race == race_lookup("vampire"))
		vision += ch->disc[DISC_AUSPEX];

	for (door=0;door<6;door++)
	{
		scan_room = ch->in_room;
		for (depth = 1; depth <= vision; depth++)
		{
			if ((pExit = scan_room->exit[door]) != NULL)
			{
				scan_room = scan_room->exit[door]->u1.to_room;
			}
			if(scan_room == victim->in_room)
				return depth;
		}
	}

	return -1;
}

void deaded_char (CHAR_DATA *ch)
{

	send_to_char("Character terminated.\n\r",ch);
	log_string( LOG_GAME, Format("Death has marked the end for %s", ch->name) );
	wiznet("\tY[WIZNET]\tn Death has marked the end for $N.", ch,NULL,WIZ_DEATHS,0,get_trust(ch));
	close_socket(ch->desc);
	unlink( (char *)Format("%s%s", PLAYER_DIR, capitalize( ch->name )));
	return;
}

void add_to_combo(CHAR_DATA *ch, CHAR_DATA *vch, int move)
{
	COMBAT_DATA *last;
	COMBAT_DATA *pmove;

	last = ch->combo;

	pmove = new_combat_move();
	pmove->move = move;
	pmove->victim = vch;
	pmove->part = combat_move_table->part;

	if(last != NULL)
	{
		while(last->next != NULL)
			last = last->next;
		ch->combo->next = pmove;
	}
	else
	{
		ch->combo = pmove;
	}
}

void do_move_stop (CHAR_DATA *ch, char *string)
{
	int fail = dice_rolls(ch, get_curr_stat(ch, STAT_DEX) + ch->ability[DODGE].value, 3);

	if(ch->combo == NULL)
	{
		send_to_char("You weren't making a move!\n\r", ch);
		return;
	}

	if(ch->act_points < 1)
	{
		send_to_char("You don't have enough action points left!", ch);
		return;
	}

	if(fail<=0)
	{
		act_new("You stop off balance.",
				ch,NULL,NULL,TO_CHAR,P_REST,1);
		act_new("$n stops suddenly, slightly off balance.",
				ch,NULL,NULL,TO_ROOM,P_REST,0);
		ch->balance -= 1;
		ch->combat_flag = -1;
	}
	else
	{
		act_new("You stop dead.",
				ch,NULL,NULL,TO_CHAR,P_REST,1);
		act_new("$n stops in $m tracks.",
				ch,NULL,NULL,TO_ROOM,P_REST,0);
		add_to_combo(ch, NULL, MOVE_NONE);
		ch->combat_flag = -1;
	}

	return;
}

void do_move_stand (CHAR_DATA *ch, char *string)
{
	int fail = dice_rolls(ch, get_curr_stat(ch, STAT_DEX) + ch->ability[ATHLETICS].value, 3);

	if(ch->combat_flag == -1 && ch->position == P_FIGHT)
	{
		send_to_char("You have cancelled your move for this combat round.\n\r", ch);
		return;
	}

	if(part_in_use(ch, MOVE_STAND))
	{
		send_to_char("You've already made use of that part in this combo!\n\r", ch);
		return;
	}

	if(ch->act_points < 1)
	{
		send_to_char("You don't have enough action points left!", ch);
		return;
	}

	if(fail<=0)
	{
		act_new(combo_table[MOVE_STAND].botch_to_char,
				ch,NULL,NULL,TO_CHAR,P_REST,1);
		act_new(combo_table[MOVE_STAND].botch_to_room,
				ch,NULL,NULL,TO_ROOM,P_REST,0);
		if(ch->position > P_STUN)
			ch->position = P_STAND;
		ch->balance -= 1;
	}
	else
	{
		act_new(combo_table[MOVE_STAND].to_char,
				ch,NULL,NULL,TO_CHAR,P_REST,1);
		act_new(combo_table[MOVE_STAND].to_room,
				ch,NULL,NULL,TO_ROOM,P_REST,0);
		add_to_combo(ch, NULL, MOVE_STAND);
		if(ch->position > P_STUN)
			ch->position = P_STAND;
		ch->balance = 1;
	}
}

void do_move_flip (CHAR_DATA *ch, char *string)
{
	int d = 3;
	int fail = 0;

	d -= ch->balance;
	if(d > 10)
		d = 10;

	fail = dice_rolls(ch, get_curr_stat(ch, STAT_STR) + ch->ability[DODGE].value, d);

	if(ch->combat_flag == -1 && ch->position == P_FIGHT)
	{
		send_to_char("You have cancelled your move for this combat round.\n\r",
				ch);
		return;
	}

	if(part_in_use(ch, MOVE_FLIP))
	{
		send_to_char("You've already made use of that part in this combo!\n\r",
				ch);
		return;
	}

	if(ch->act_points < 3)
	{
		send_to_char("You don't have enough action points left!", ch);
		return;
	}

	if(fail<=0)
	{
		act_new(combo_table[MOVE_FLIP].botch_to_char,
				ch,NULL,NULL,TO_CHAR,P_REST,1);
		act_new(combo_table[MOVE_FLIP].botch_to_room,
				ch,NULL,NULL,TO_ROOM,P_REST,0);
		if(ch->position > P_STUN)
			ch->position = P_STUN;
		ch->balance -= 5;
	}
	else
	{
		act_new(combo_table[MOVE_FLIP].to_char,
				ch,NULL,NULL,TO_CHAR,P_REST,1);
		act_new(combo_table[MOVE_FLIP].to_room,
				ch,NULL,NULL,TO_ROOM,P_REST,0);
		if(ch->position > P_STUN)
			ch->position = P_STAND;
		add_to_combo(ch, NULL, MOVE_FLIP);
	}
}

void do_move_roll (CHAR_DATA *ch, char *string)
{
	int d = 3;
	int fail = 0;

	d -= ch->balance;
	if(d > 10)
		d = 10;

	fail = dice_rolls(ch, get_curr_stat(ch, STAT_DEX)
			+ ch->ability[ATHLETICS].value, d);

	if(ch->combat_flag == -1 && ch->position == P_FIGHT)
	{
		send_to_char("You have cancelled your move for this combat round.\n\r", ch);
		return;
	}

	if(part_in_use(ch, MOVE_ROLL))
	{
		send_to_char("You've already made use of that part in this combo!\n\r", ch);
		return;
	}

	if(ch->act_points < 3)
	{
		send_to_char("You don't have enough action points left!", ch);
		return;
	}

	if(fail<=0)
	{
		act_new(combo_table[MOVE_ROLL].botch_to_char,
				ch,NULL,NULL,TO_CHAR,P_REST,1);
		act_new(combo_table[MOVE_ROLL].botch_to_room,
				ch,NULL,NULL,TO_ROOM,P_REST,0);
		ch->balance -= 4;
	}
	else
	{
		act_new(combo_table[MOVE_ROLL].to_char,
				ch,NULL,NULL,TO_CHAR,P_REST,1);
		act_new(combo_table[MOVE_ROLL].to_room,
				ch,NULL,NULL,TO_ROOM,P_REST,0);
		add_to_combo(ch, NULL, MOVE_ROLL);
	}
}

void do_move_jump (CHAR_DATA *ch, char *string)
{
	int d = 3;
	int fail;

	d -= ch->balance;
	if(d > 10)
		d = 10;

	fail = dice_rolls(ch, get_curr_stat(ch, STAT_STR)
			+ ch->ability[ATHLETICS].value, d);

	if(ch->combat_flag == -1 && ch->position == P_FIGHT)
	{
		send_to_char("You have cancelled your move for this combat round.\n\r", ch);
		return;
	}

	if(part_in_use(ch, MOVE_JUMP))
	{
		send_to_char("You've already made use of that part in this combo!\n\r", ch);
		return;
	}

	if(ch->act_points < 2)
	{
		send_to_char("You don't have enough action points left!", ch);
		return;
	}

	if(fail<=0)
	{
		act_new(combo_table[MOVE_JUMP].botch_to_char,
				ch,NULL,NULL,TO_CHAR,P_REST,1);
		act_new(combo_table[MOVE_JUMP].botch_to_room,
				ch,NULL,NULL,TO_ROOM,P_REST,0);
		ch->balance -= 5;
		if (ch->position > P_STUN)
			ch->position = P_REST;
	}
	else
	{
		act_new(combo_table[MOVE_JUMP].to_char,
				ch,NULL,NULL,TO_CHAR,P_REST,1);
		act_new(combo_table[MOVE_JUMP].to_room,
				ch,NULL,NULL,TO_ROOM,P_REST,0);
		add_to_combo(ch, NULL, MOVE_JUMP);
	}
}

int brawl_or_melee(CHAR_DATA *ch)
{
	OBJ_DATA *pObj = get_eq_char(ch, WEAR_WIELD);

	if(pObj)
	{
		if(pObj->item_type == ITEM_WEAPON)
		{
			if(pObj->value[0] == WEAPON_FIREARM)
				return ch->ability[FIREARMS].value;
			else
				return ch->ability[MELEE].value;
		}
	}

	return ch->ability[BRAWL].value;
}

void do_move_lblow (CHAR_DATA *ch, char *string)
{
	CHAR_DATA *victim;
	int fail = dice_rolls(ch, get_curr_stat(ch, STAT_DEX) + brawl_or_melee(ch),6);

	if(ch->combat_flag == -1 && ch->position == P_FIGHT)
	{
		send_to_char("You have cancelled your move for this combat round.\n\r",	ch);
		return;
	}

	if(part_in_use(ch, MOVE_LBLOW))
	{
		send_to_char("You've already made use of that part in this combo!\n\r", ch);
		return;
	}

	if(ch->act_points < 1)
	{
		send_to_char("You don't have enough action points left!", ch);
		return;
	}

	if((victim = get_char_room(ch, string)) == NULL
			&& ch->fighting == NULL)
	{
		send_to_char("Who're you attacking?\n\r", ch);
		return;
	}

	if(victim == NULL && ch->fighting != NULL)
		victim = ch->fighting;

	if(victim == ch)
	{
		send_to_char("Don't go hitting yourself like that.\n\r", ch);
		return;
	}

	if(IS_AFFECTED(ch, AFF_CHARM) && victim == ch->master)
	{
		send_to_char("You can't attack your dear master!\n\r", ch);
		return;
	}

	if(IS_SET(victim->act2, ACT2_RP_ING) && ch->fighting != victim)
	{
		send_to_char("You cannot use ooc combat on a person in character.\n\r", ch);
		return;
	}

	if(ch->fighting == NULL || ch->fighting != victim)
		set_fighting(ch, victim);

	if(fail<=0)
	{
		act_new(combo_table[MOVE_LBLOW].botch_to_char, ch,NULL,victim,TO_CHAR,P_REST,1);
		act_new(combo_table[MOVE_LBLOW].botch_to_vict, ch,NULL,victim,TO_VICT,P_REST,1);
		act_new(combo_table[MOVE_LBLOW].botch_to_room, ch,NULL,victim,TO_NOTVICT,P_REST,0);
		ch->balance -= 2;
	}
	else
	{
		act_new(combo_table[MOVE_LBLOW].to_char, ch,NULL,victim,TO_CHAR,P_REST,1);
		act_new(combo_table[MOVE_LBLOW].to_vict, ch,NULL,victim,TO_VICT,P_REST,1);
		act_new(combo_table[MOVE_LBLOW].to_room, ch,NULL,victim,TO_NOTVICT,P_REST,0);
		add_to_combo(ch,victim, MOVE_LBLOW);
	}
	ch->combo_success = fail;
}

void do_move_rblow (CHAR_DATA *ch, char *string)
{
	CHAR_DATA *victim;
	int fail = dice_rolls(ch, get_curr_stat(ch, STAT_DEX) + brawl_or_melee(ch),6);
	if(ch->combat_flag == -1 && ch->position == P_FIGHT)
	{
		send_to_char("You have cancelled your move for this combat round.\n\r",ch);
		return;
	}

	if(part_in_use(ch, MOVE_RBLOW))
	{
		send_to_char("You've already made use of that part in this combo!\n\r", ch);
		return;
	}

	if(ch->act_points < 1)
	{
		send_to_char("You don't have enough action points left!", ch);
		return;
	}

	if((victim = get_char_room(ch, string)) == NULL
			&& ch->fighting == NULL)
	{
		send_to_char("Who're you attacking?\n\r", ch);
		return;
	}

	if(victim == NULL && ch->fighting != NULL)
		victim = ch->fighting;

	if(victim == NULL)
	{
		send_to_char("Who're you attacking?\n\r", ch);
		return;
	}

	if(victim == ch)
	{
		send_to_char("Don't go hitting yourself like that.\n\r", ch);
		return;
	}

	if(IS_AFFECTED(ch, AFF_CHARM) && victim == ch->master)
	{
		send_to_char("You can't attack your dear master!\n\r", ch);
		return;
	}

	if(IS_SET(victim->act2, ACT2_RP_ING) && ch->fighting == NULL)
	{
		send_to_char("You cannot use ooc combat on a person in character.\n\r", ch);
		return;
	}

	if(ch->fighting == NULL || ch->fighting != victim)
		set_fighting(ch, victim);

	if(fail<=0)
	{
		act_new(combo_table[MOVE_RBLOW].botch_to_char,
				ch,NULL,victim,TO_CHAR,P_REST,1);
		act_new(combo_table[MOVE_RBLOW].botch_to_vict,
				ch,NULL,victim,TO_VICT,P_REST,1);
		act_new(combo_table[MOVE_RBLOW].botch_to_room,
				ch,NULL,victim,TO_NOTVICT,P_REST,0);
		ch->balance -= 2;
	}
	else
	{
		act_new(combo_table[MOVE_RBLOW].to_char,
				ch,NULL,victim,TO_CHAR,P_REST,1);
		act_new(combo_table[MOVE_RBLOW].to_vict,
				ch,NULL,victim,TO_VICT,P_REST,1);
		act_new(combo_table[MOVE_RBLOW].to_room,
				ch,NULL,victim,TO_NOTVICT,P_REST,0);
		add_to_combo(ch,victim,MOVE_RBLOW);
	}
	ch->combo_success = fail;
}

void do_move_lgrab (CHAR_DATA *ch, char *string)
{
CHAR_DATA *victim;
int fail = dice_rolls(ch, get_curr_stat(ch, STAT_DEX) + ch->ability[BRAWL].value, 8);
    if(ch->combat_flag == -1 && ch->position == P_FIGHT)
    {
	send_to_char("You have cancelled your move for this combat round.\n\r",ch);
	return;
    }

    if(part_in_use(ch, MOVE_LGRAB))
    {
	send_to_char("You've already made use of that part in this combo!\n\r", ch);
	return;
    }

    if(ch->act_points < 1)
    {
	send_to_char("You don't have enough action points left!", ch);
	return;
    }

    if((victim = get_char_room(ch, string)) == NULL
	&& ch->fighting == NULL)
    {
	send_to_char("Who're you attacking?\n\r", ch);
	return;
    }

    if(victim == NULL && ch->fighting != NULL)
	victim = ch->fighting;

    if(victim == NULL)
    {
	send_to_char("Who're you attacking?\n\r", ch);
	return;
    }

    if(victim == ch)
    {
	send_to_char("Don't go hitting yourself like that.\n\r", ch);
	return;
    }

    if(IS_AFFECTED(ch, AFF_CHARM) && victim == ch->master)
    {
	send_to_char("You can't attack your dear master!\n\r", ch);
	return;
    }

    if(IS_SET(victim->act2, ACT2_RP_ING) && ch->fighting == NULL)
    {
	send_to_char(
	    "You cannot use ooc combat on a person in character.\n\r", ch);
	return;
    }

    if(ch->fighting == NULL || ch->fighting != victim)
    set_fighting(ch, victim);

    if(fail<=0)
    {
	act_new(combo_table[MOVE_LGRAB].botch_to_char,
		ch,NULL,victim,TO_CHAR,P_REST,1);
	act_new(combo_table[MOVE_LGRAB].botch_to_vict,
		ch,NULL,victim,TO_VICT,P_REST,1);
	act_new(combo_table[MOVE_LGRAB].botch_to_room,
		ch,NULL,victim,TO_NOTVICT,P_REST,0);
	ch->balance -= 3;
    }
    else
    {
	act_new(combo_table[MOVE_LGRAB].to_char,
		ch,NULL,victim,TO_CHAR,P_REST,1);
	act_new(combo_table[MOVE_LGRAB].to_vict,
		ch,NULL,victim,TO_VICT,P_REST,1);
	act_new(combo_table[MOVE_LGRAB].to_room,
		ch,NULL,victim,TO_NOTVICT,P_REST,0);
	add_to_combo(ch,victim, MOVE_LGRAB);
    }
    ch->combo_success = fail;
}

void do_move_rgrab (CHAR_DATA *ch, char *string)
{
CHAR_DATA *victim;
int fail = dice_rolls(ch, get_curr_stat(ch, STAT_DEX) + ch->ability[BRAWL].value, 8);
    if(ch->combat_flag == -1 && ch->position == P_FIGHT)
    {
	send_to_char("You have cancelled your move for this combat round.\n\r",ch);
	return;
    }

    if(part_in_use(ch, MOVE_RGRAB))
    {
	send_to_char("You've already made use of that part in this combo!\n\r", ch);
	return;
    }

    if(ch->act_points < 1)
    {
	send_to_char("You don't have enough action points left!", ch);
	return;
    }

    if((victim = get_char_room(ch, string)) == NULL
	&& ch->fighting == NULL)
    {
	send_to_char("Who're you attacking?\n\r", ch);
	return;
    }

    if(victim == NULL && ch->fighting != NULL)
	victim = ch->fighting;

    if(victim == NULL)
    {
	send_to_char("Who're you attacking?\n\r", ch);
	return;
    }

    if(victim == ch)
    {
	send_to_char("Don't go hitting yourself like that.\n\r", ch);
	return;
    }

    if(IS_AFFECTED(ch, AFF_CHARM) && victim == ch->master)
    {
	send_to_char("You can't attack your dear master!\n\r", ch);
	return;
    }

    if(IS_SET(victim->act2, ACT2_RP_ING) && ch->fighting == NULL)
    {
	send_to_char(
	    "You cannot use ooc combat on a person in character.\n\r", ch);
	return;
    }

    if(victim == NULL && ch->fighting != NULL)
	victim = ch->fighting;

    if(ch->fighting == NULL || ch->fighting != victim)
    set_fighting(ch, victim);

    if(fail<=0)
    {
	act_new(combo_table[MOVE_RGRAB].botch_to_char,
		ch,NULL,victim,TO_CHAR,P_REST,1);
	act_new(combo_table[MOVE_RGRAB].botch_to_vict,
		ch,NULL,victim,TO_VICT,P_REST,1);
	act_new(combo_table[MOVE_RGRAB].botch_to_room,
		ch,NULL,victim,TO_NOTVICT,P_REST,0);
	ch->balance -= 3;
    }
    else
    {
	act_new(combo_table[MOVE_RGRAB].to_char,
		ch,NULL,victim,TO_CHAR,P_REST,1);
	act_new(combo_table[MOVE_RGRAB].to_vict,
		ch,NULL,victim,TO_VICT,P_REST,1);
	act_new(combo_table[MOVE_RGRAB].to_room,
		ch,NULL,victim,TO_NOTVICT,P_REST,0);
	add_to_combo(ch,victim, MOVE_RGRAB);
    }
    ch->combo_success = fail;
}

void do_move_kick (CHAR_DATA *ch, char *string)
{
CHAR_DATA *victim;
int d = 7;
int fail = 0;

    d -= ch->balance;
    if(d > 10)
	d = 10;

    fail = dice_rolls(ch, get_curr_stat(ch, STAT_DEX) + ch->ability[BRAWL].value, d);

    if(ch->combat_flag == -1 && ch->position == P_FIGHT)
    {
	send_to_char("You have cancelled your move for this combat round.\n\r",ch);
	return;
    }

    if(part_in_use(ch, MOVE_KICK))
    {
	send_to_char("You've already made use of that part in this combo!\n\r", ch);
	return;
    }

    if(ch->act_points < 1)
    {
	send_to_char("You don't have enough action points left!", ch);
	return;
    }

    if((victim = get_char_room(ch, string)) == NULL
	&& ch->fighting == NULL)
    {
	send_to_char("Who're you attacking?\n\r", ch);
	return;
    }

    if(victim == NULL && ch->fighting != NULL)
	victim = ch->fighting;

    if(victim == NULL)
    {
	send_to_char("Who're you attacking?\n\r", ch);
	return;
    }

    if(victim == ch)
    {
	send_to_char("Don't go hitting yourself like that.\n\r", ch);
	return;
    }

    if(IS_AFFECTED(ch, AFF_CHARM) && victim == ch->master)
    {
	send_to_char("You can't attack your dear master!\n\r", ch);
	return;
    }

    if(IS_SET(victim->act2, ACT2_RP_ING) && ch->fighting == NULL)
    {
	send_to_char(
	    "You cannot use ooc combat on a person in character.\n\r", ch);
	return;
    }

    if(ch->fighting == NULL || ch->fighting != victim)
    set_fighting(ch, victim);

    if(fail<=0)
    {
	act_new(combo_table[MOVE_KICK].botch_to_char,
		ch,NULL,victim,TO_CHAR,P_REST,1);
	act_new(combo_table[MOVE_KICK].botch_to_vict,
		ch,NULL,victim,TO_VICT,P_REST,1);
	act_new(combo_table[MOVE_KICK].botch_to_room,
		ch,NULL,victim,TO_NOTVICT,P_REST,0);
	ch->balance -= 3;
    }
    else
    {
	act_new(combo_table[MOVE_KICK].to_char,
		ch,NULL,victim,TO_CHAR,P_REST,1);
	act_new(combo_table[MOVE_KICK].to_vict,
		ch,NULL,victim,TO_VICT,P_REST,1);
	act_new(combo_table[MOVE_KICK].to_room,
		ch,NULL,victim,TO_NOTVICT,P_REST,0);
	add_to_combo(ch,victim, MOVE_KICK);
    }
    ch->combo_success = fail;
}

void do_move_skick (CHAR_DATA *ch, char *string)
{
CHAR_DATA *victim;
int d = 8;
int fail = 0;

    d -= ch->balance;
    if(d > 10)
	d = 10;

    fail = dice_rolls(ch, get_curr_stat(ch, STAT_DEX) + ch->ability[BRAWL].value, d);

    if(ch->combat_flag == -1 && ch->position == P_FIGHT)
    {
	send_to_char("You have cancelled your move for this combat round.\n\r",ch);
	return;
    }

    if(part_in_use(ch, MOVE_SKICK))
    {
	send_to_char("You've already made use of that part in this combo!\n\r", ch);
	return;
    }

    if(ch->act_points < 2)
    {
	send_to_char("You don't have enough action points left!", ch);
	return;
    }

    if((victim = get_char_room(ch, string)) == NULL
	&& ch->fighting == NULL)
    {
	send_to_char("Who're you attacking?\n\r", ch);
	return;
    }

    if(victim == NULL && ch->fighting != NULL)
	victim = ch->fighting;

    if(victim == NULL)
    {
	send_to_char("Who're you attacking?\n\r", ch);
	return;
    }

    if(victim == ch)
    {
	send_to_char("Don't go hitting yourself like that.\n\r", ch);
	return;
    }

    if(IS_AFFECTED(ch, AFF_CHARM) && victim == ch->master)
    {
	send_to_char("You can't attack your dear master!\n\r", ch);
	return;
    }

    if(IS_SET(victim->act2, ACT2_RP_ING) && ch->fighting == NULL)
    {
	send_to_char(
	    "You cannot use ooc combat on a person in character.\n\r", ch);
	return;
    }

    if(ch->fighting == NULL || ch->fighting != victim)
    set_fighting(ch, victim);

    if(fail<=0)
    {
	act_new(combo_table[MOVE_SKICK].botch_to_char,
		ch,NULL,victim,TO_CHAR,P_REST,1);
	act_new(combo_table[MOVE_SKICK].botch_to_vict,
		ch,NULL,victim,TO_VICT,P_REST,1);
	act_new(combo_table[MOVE_SKICK].botch_to_room,
		ch,NULL,victim,TO_NOTVICT,P_REST,0);
	ch->balance -= 3;
    }
    else
    {
	act_new(combo_table[MOVE_SKICK].to_char,
		ch,NULL,victim,TO_CHAR,P_REST,1);
	act_new(combo_table[MOVE_SKICK].to_vict,
		ch,NULL,victim,TO_VICT,P_REST,1);
	act_new(combo_table[MOVE_SKICK].to_room,
		ch,NULL,victim,TO_NOTVICT,P_REST,0);
	add_to_combo(ch,victim, MOVE_SKICK);
    }
    ch->combo_success = fail;
}

void do_move_bite (CHAR_DATA *ch, char *string)
{
CHAR_DATA *victim;
int fail = dice_rolls(ch, get_curr_stat(ch, STAT_DEX) + ch->ability[BRAWL].value, 8);
    if(ch->combat_flag == -1 && ch->position == P_FIGHT)
    {
	send_to_char("You have cancelled your move for this combat round.\n\r",ch);
	return;
    }

    if(part_in_use(ch, MOVE_BITE))
    {
	send_to_char("You've already made use of that part in this combo!\n\r", ch);
	return;
    }

    if(ch->act_points < 1)
    {
	send_to_char("You don't have enough action points left!", ch);
	return;
    }

    if((victim = get_char_room(ch, string)) == NULL
	&& ch->fighting == NULL)
    {
	send_to_char("Who're you attacking?\n\r", ch);
	return;
    }

    if(victim == NULL && ch->fighting != NULL)
	victim = ch->fighting;

    if(victim == NULL)
    {
	send_to_char("Who're you attacking?\n\r", ch);
	return;
    }

    if(victim == ch)
    {
	send_to_char("Don't go hitting yourself like that.\n\r", ch);
	return;
    }

    if(IS_AFFECTED(ch, AFF_CHARM) && victim == ch->master)
    {
	send_to_char("You can't attack your dear master!\n\r", ch);
	return;
    }

    if(IS_SET(victim->act2, ACT2_RP_ING) && ch->fighting == NULL)
    {
	send_to_char(
	    "You cannot use ooc combat on a person in character.\n\r", ch);
	return;
    }

    if(ch->fighting == NULL || ch->fighting != victim)
    set_fighting(ch, victim);

    if(fail<=0)
    {
	act_new(combo_table[MOVE_BITE].botch_to_char,
		ch,NULL,victim,TO_CHAR,P_REST,1);
	act_new(combo_table[MOVE_BITE].botch_to_vict,
		ch,NULL,victim,TO_VICT,P_REST,1);
	act_new(combo_table[MOVE_BITE].botch_to_room,
		ch,NULL,victim,TO_NOTVICT,P_REST,0);
    }
    else
    {
	act_new(combo_table[MOVE_BITE].to_char,
		ch,NULL,victim,TO_CHAR,P_REST,1);
	act_new(combo_table[MOVE_BITE].to_vict,
		ch,NULL,victim,TO_VICT,P_REST,1);
	act_new(combo_table[MOVE_BITE].to_room,
		ch,NULL,victim,TO_NOTVICT,P_REST,0);
	add_to_combo(ch,victim, MOVE_BITE);
    }
    ch->combo_success = fail;
}

void do_move_lblock (CHAR_DATA *ch, char *string)
{
int fail = dice_rolls(ch, get_curr_stat(ch, STAT_DEX)
				+ ch->ability[DODGE].value, 3);
    if(ch->combat_flag == -1 && ch->position == P_FIGHT)
    {
	send_to_char("You have cancelled your move for this combat round.\n\r",ch);
	return;
    }

    if(part_in_use(ch, MOVE_LBLOCK))
    {
	send_to_char("You've already made use of that part in this combo!\n\r", ch);
	return;
    }

    if(ch->act_points < 1)
    {
	send_to_char("You don't have enough action points left!", ch);
	return;
    }

    if(fail<=0)
    {
	act_new(combo_table[MOVE_LBLOCK].botch_to_char,
		ch,NULL,NULL,TO_CHAR,P_REST,1);
	act_new(combo_table[MOVE_LBLOCK].botch_to_room,
		ch,NULL,NULL,TO_ROOM,P_REST,0);
	ch->balance -= 1;
    }
    else
    {
	act_new(combo_table[MOVE_LBLOCK].to_char,
		ch,NULL,NULL,TO_CHAR,P_REST,1);
	act_new(combo_table[MOVE_LBLOCK].to_room,
		ch,NULL,NULL,TO_ROOM,P_REST,0);
	add_to_combo(ch, NULL, MOVE_LBLOCK);
	SET_BIT(ch->combat_flag,A);
    }
}

void do_move_rblock (CHAR_DATA *ch, char *string)
{
int fail = dice_rolls(ch, get_curr_stat(ch, STAT_DEX)
			+ ch->ability[DODGE].value, 3);
    if(ch->combat_flag == -1 && ch->position == P_FIGHT)
    {
	send_to_char("You have cancelled your move for this combat round.\n\r",ch);
	return;
    }

    if(part_in_use(ch, MOVE_RBLOCK))
    {
	send_to_char("You've already made use of that part in this combo!\n\r", ch);
	return;
    }

    if(ch->act_points < 1)
    {
	send_to_char("You don't have enough action points left!", ch);
	return;
    }

    if(fail<=0)
    {
	act_new(combo_table[MOVE_RBLOCK].botch_to_char,
		ch,NULL,NULL,TO_CHAR,P_REST,1);
	act_new(combo_table[MOVE_RBLOCK].botch_to_room,
		ch,NULL,NULL,TO_ROOM,P_REST,0);
	ch->balance -= 1;
    }
    else
    {
	act_new(combo_table[MOVE_RBLOCK].to_char,
		ch,NULL,NULL,TO_CHAR,P_REST,1);
	act_new(combo_table[MOVE_RBLOCK].to_room,
		ch,NULL,NULL,TO_ROOM,P_REST,0);
	add_to_combo(ch, NULL, MOVE_RBLOCK);
	SET_BIT(ch->combat_flag,B);
    }
}

void do_move_fblock (CHAR_DATA *ch, char *string)
{
int d = 3;
int fail = 0;

    d -= ch->balance;
    if(d > 10)
	d = 10;

    fail = dice_rolls(ch, get_curr_stat(ch, STAT_DEX) + ch->ability[DODGE].value, d);

    if(ch->combat_flag == -1 && ch->position == P_FIGHT)
    {
	send_to_char("You have cancelled your move for this combat round.\n\r",ch);
	return;
    }

    if(part_in_use(ch, MOVE_FBLOCK))
    {
	send_to_char("You've already made use of that part in this combo!\n\r", ch);
	return;
    }

    if(ch->act_points < 1)
    {
	send_to_char("You don't have enough action points left!", ch);
	return;
    }

    if(fail<=0)
    {
	act_new(combo_table[MOVE_FBLOCK].botch_to_char,
		ch,NULL,NULL,TO_CHAR,P_REST,1);
	act_new(combo_table[MOVE_FBLOCK].botch_to_room,
		ch,NULL,NULL,TO_ROOM,P_REST,0);
	ch->balance -= 2;
    }
    else
    {
	act_new(combo_table[MOVE_FBLOCK].to_char,
		ch,NULL,NULL,TO_CHAR,P_REST,1);
	act_new(combo_table[MOVE_FBLOCK].to_room,
		ch,NULL,NULL,TO_ROOM,P_REST,0);
	add_to_combo(ch, NULL, MOVE_FBLOCK);
	SET_BIT(ch->combat_flag,C);
    }
}

void do_move_spin (CHAR_DATA *ch, char *string)
{
int d = 3;
int fail = 0;

    d -= ch->balance;
    if(d > 10)
	d = 10;

    fail = dice_rolls(ch, get_curr_stat(ch, STAT_DEX) + ch->ability[ATHLETICS].value, d);

    if(ch->combat_flag == -1 && ch->position == P_FIGHT)
    {
	send_to_char("You have cancelled your move for this combat round.\n\r",ch);
	return;
    }

    if(part_in_use(ch, MOVE_SPIN))
    {
	send_to_char("You've already made use of that part in this combo!\n\r", ch);
	return;
    }

    if(ch->act_points < 3)
    {
	send_to_char("You don't have enough action points left!", ch);
	return;
    }

    if(fail<=0)
    {
	act_new(combo_table[MOVE_SPIN].botch_to_char,
		ch,NULL,NULL,TO_CHAR,P_REST,1);
	act_new(combo_table[MOVE_SPIN].botch_to_room,
		ch,NULL,NULL,TO_ROOM,P_REST,0);
	ch->balance -= 4;
    }
    else
    {
	act_new(combo_table[MOVE_SPIN].to_char,
		ch,NULL,NULL,TO_CHAR,P_REST,1);
	act_new(combo_table[MOVE_SPIN].to_room,
		ch,NULL,NULL,TO_ROOM,P_REST,0);
	add_to_combo(ch, NULL, MOVE_SPIN);
    }
}

void do_move_sweep (CHAR_DATA *ch, char *string)
{
CHAR_DATA *victim;
int d = 6;
int fail = 0;

    d -= ch->balance;
    if(d > 10)
	d = 10;

    fail = dice_rolls(ch, get_curr_stat(ch, STAT_DEX)
				+ ch->ability[BRAWL].value, d);

    if(ch->combat_flag == -1 && ch->position == P_FIGHT)
    {
	send_to_char("You have cancelled your move for this combat round.\n\r",
				ch);
	return;
    }

    if(part_in_use(ch, MOVE_SWEEP))
    {
	send_to_char("You've already made use of that part in this combo!\n\r",
				ch);
	return;
    }

    if(ch->act_points < 2)
    {
	send_to_char("You don't have enough action points left!", ch);
	return;
    }

    if((victim = get_char_room(ch, string)) == NULL
	&& ch->fighting == NULL)
    {
	send_to_char("Who're you attacking?\n\r", ch);
	return;
    }

    if(victim == NULL && ch->fighting != NULL)
	victim = ch->fighting;

    if(victim == NULL)
    {
	send_to_char("Who're you attacking?\n\r", ch);
	return;
    }

    if(victim == ch)
    {
	send_to_char("Don't go hitting yourself like that.\n\r", ch);
	return;
    }

    if(IS_AFFECTED(ch, AFF_CHARM) && victim == ch->master)
    {
	send_to_char("You can't attack your dear master!\n\r", ch);
	return;
    }

    if(IS_SET(victim->act2, ACT2_RP_ING) && ch->fighting == NULL)
    {
	send_to_char(
	    "You cannot use ooc combat on a person in character.\n\r", ch);
	return;
    }

    if(victim == NULL && ch->fighting != NULL)
	victim = ch->fighting;

    if(ch->fighting == NULL || ch->fighting != victim)
    set_fighting(ch, victim);

    if(fail<=0)
    {
	act_new(combo_table[MOVE_SWEEP].botch_to_char,
		ch,NULL,victim,TO_CHAR,P_REST,1);
	act_new(combo_table[MOVE_SWEEP].botch_to_vict,
		ch,NULL,victim,TO_VICT,P_REST,1);
	act_new(combo_table[MOVE_SWEEP].botch_to_room,
		ch,NULL,victim,TO_NOTVICT,P_REST,0);
	ch->balance -= 3;
    }
    else
    {
	act_new(combo_table[MOVE_SWEEP].to_char,
		ch,NULL,victim,TO_CHAR,P_REST,1);
	act_new(combo_table[MOVE_SWEEP].to_vict,
		ch,NULL,victim,TO_VICT,P_REST,1);
	act_new(combo_table[MOVE_SWEEP].to_room,
		ch,NULL,victim,TO_NOTVICT,P_REST,0);
	add_to_combo(ch,victim, MOVE_SWEEP);
	victim->balance -= 4;
    }
    ch->combo_success = fail;
}

void do_move_dodge (CHAR_DATA *ch, char *string)
{
	CHAR_DATA *victim;
	int d = 6;
	int fail = 0;

	d -= ch->balance;
	if(d > 10)
		d = 10;

	fail = dice_rolls(ch, get_curr_stat(ch, STAT_DEX) + ch->ability[BRAWL].value, d);

	if(ch->combat_flag == -1 && ch->position == P_FIGHT)
	{
		send_to_char("You have cancelled your move for this combat round.\n\r",ch);
		return;
	}

	if(ch->combo != NULL)
	{
		send_to_char("You're already working through a combo!\n\r", ch);
		return;
	}

	victim = ch->fighting;

	if(ch->fighting == NULL)
	{
		send_to_char("You can't dodge if you aren't fighting.\n\r", ch);
		return;
	}

	if(ch->act_points < 1)
	{
		send_to_char("You don't have enough action points left!", ch);
		return;
	}

	if(fail<=0)
	{
		act_new(combo_table[MOVE_DODGE].botch_to_char, ch,NULL,victim,TO_CHAR,P_REST,1);
		act_new(combo_table[MOVE_DODGE].botch_to_vict, ch,NULL,victim,TO_VICT,P_REST,1);
		act_new(combo_table[MOVE_DODGE].botch_to_room, ch,NULL,victim,TO_NOTVICT,P_REST,0);
		ch->balance -= 3;
	}
	else
	{
		act_new(combo_table[MOVE_DODGE].to_char, ch,NULL,victim,TO_CHAR,P_REST,1);
		act_new(combo_table[MOVE_DODGE].to_vict, ch,NULL,victim,TO_VICT,P_REST,1);
		act_new(combo_table[MOVE_DODGE].to_room, ch,NULL,victim,TO_NOTVICT,P_REST,0);
		add_to_combo(ch,victim, MOVE_DODGE);
		victim->balance -= 4;
	}
}

void do_move_touch (CHAR_DATA *ch, char *string)
{
CHAR_DATA *victim;
int d = 4;
int fail;

    d -= ch->balance;
    if(d > 10)
	d = 10;

    fail = dice_rolls(ch, get_curr_stat(ch, STAT_DEX)
				+ ch->ability[BRAWL].value, d);

    if(ch->combat_flag == -1 && ch->position == P_FIGHT)
    {
	send_to_char("You have cancelled your move for this combat round.\n\r",
				ch);
	return;
    }

    if(part_in_use(ch, MOVE_TOUCH))
    {
	send_to_char("You've already made use of that part in this combo!\n\r",
				ch);
	return;
    }

    if(ch->act_points < 1)
    {
	send_to_char("You don't have enough action points left!", ch);
	return;
    }

    if((victim = get_char_room(ch, string)) == NULL
	&& ch->fighting == NULL)
    {
	send_to_char("Who're you attacking?\n\r", ch);
	return;
    }

    if(victim == NULL && ch->fighting != NULL)
	victim = ch->fighting;

    if(victim == NULL)
    {
	send_to_char("Who're you attacking?\n\r", ch);
	return;
    }

    if(victim == ch)
    {
	send_to_char("Don't go hitting yourself like that.\n\r", ch);
	return;
    }

    if(IS_AFFECTED(ch, AFF_CHARM) && victim == ch->master)
    {
	send_to_char("You can't attack your dear master!\n\r", ch);
	return;
    }

    if(IS_SET(victim->act2, ACT2_RP_ING) && ch->fighting == NULL)
    {
	send_to_char(
	    "You cannot use ooc combat on a person in character.\n\r", ch);
	return;
    }

    if(victim == NULL && ch->fighting != NULL)
	victim = ch->fighting;

    if(ch->fighting == NULL || ch->fighting != victim)
    set_fighting(ch, victim);

    if(fail<=0)
    {
	act_new(combo_table[MOVE_TOUCH].botch_to_char,
		ch,NULL,victim,TO_CHAR,P_REST,1);
	act_new(combo_table[MOVE_TOUCH].botch_to_vict,
		ch,NULL,victim,TO_VICT,P_REST,1);
	act_new(combo_table[MOVE_TOUCH].botch_to_room,
		ch,NULL,victim,TO_NOTVICT,P_REST,0);
	ch->balance -= 1;
    }
    else
    {
	act_new(combo_table[MOVE_TOUCH].to_char,
		ch,NULL,victim,TO_CHAR,P_REST,1);
	act_new(combo_table[MOVE_TOUCH].to_vict,
		ch,NULL,victim,TO_VICT,P_REST,1);
	act_new(combo_table[MOVE_TOUCH].to_room,
		ch,NULL,victim,TO_NOTVICT,P_REST,0);
	add_to_combo(ch,victim, MOVE_TOUCH);
    }
    ch->combo_success = fail;
}

void do_move_tongue (CHAR_DATA *ch, char *string)
{
	CHAR_DATA *victim;
	int d = 6;
	int fail;

	if(!IS_SET(ch->parts, PART_LONG_TONGUE))
	{
		send_to_char("You could stick your tongue out, but it wouldn't do much good.\n\r", ch);
		return;
	}

	d -= ch->balance;
	if(d > 10)
		d = 10;

	fail = dice_rolls(ch, get_curr_stat(ch, STAT_DEX)
			+ ch->ability[BRAWL].value, d);

	if(ch->combat_flag == -1 && ch->position == P_FIGHT)
	{
		send_to_char("You have cancelled your move for this combat round.\n\r", ch);
		return;
	}

	if(part_in_use(ch, MOVE_TONGUE))
	{
		send_to_char("You've already made use of that part in this combo!\n\r", ch);
		return;
	}

	if(ch->act_points < 1)
	{
		send_to_char("You don't have enough action points left!", ch);
		return;
	}

	if((victim = get_char_room(ch, string)) == NULL
			&& ch->fighting == NULL)
	{
		send_to_char("Who're you attacking?\n\r", ch);
		return;
	}

	if(victim == NULL && ch->fighting != NULL)
		victim = ch->fighting;

	if(victim == NULL)
	{
		send_to_char("Who're you attacking?\n\r", ch);
		return;
	}

	if(victim == ch)
	{
		send_to_char("Don't go hitting yourself like that.\n\r", ch);
		return;
	}

	if(IS_AFFECTED(ch, AFF_CHARM) && victim == ch->master)
	{
		send_to_char("You can't attack your dear master!\n\r", ch);
		return;
	}

	if(IS_SET(victim->act2, ACT2_RP_ING) && ch->fighting == NULL)
	{
		send_to_char("You cannot use ooc combat on a person in character.\n\r", ch);
		return;
	}

	if(victim == NULL && ch->fighting != NULL)
		victim = ch->fighting;

	if(ch->fighting == NULL || ch->fighting != victim)
		set_fighting(ch, victim);

	if(fail<=0)
	{
		act_new(combo_table[MOVE_TONGUE].botch_to_char, ch,NULL,victim,TO_CHAR,P_REST,1);
		act_new(combo_table[MOVE_TONGUE].botch_to_vict, ch,NULL,victim,TO_VICT,P_REST,1);
		act_new(combo_table[MOVE_TONGUE].botch_to_room, ch,NULL,victim,TO_NOTVICT,P_REST,0);
		ch->balance -= 1;
	}
	else
	{
		act_new(combo_table[MOVE_TONGUE].to_char, ch,NULL,victim,TO_CHAR,P_REST,1);
		act_new(combo_table[MOVE_TONGUE].to_vict, ch,NULL,victim,TO_VICT,P_REST,1);
		act_new(combo_table[MOVE_TONGUE].to_room, ch,NULL,victim,TO_NOTVICT,P_REST,0);
		add_to_combo(ch,victim, MOVE_TONGUE);
	}
	ch->combo_success = fail;
}


void do_challenge(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *vch;
	char arg[MAX_INPUT_LENGTH]={'\0'};

	if(IS_NULLSTR(argument))
	{
		send_to_char("Who are you challenging over what?\n\r", ch);
		return;
	}

	argument = one_argument(argument, arg);
	if((vch = get_char_world(ch, arg)) == NULL)
	{
		send_to_char("They aren't online right now.\n\r", ch);
		return;
	}

	argument = one_argument(argument, arg);
	if(!str_prefix(arg, "leadership"))
	{
	}
	else if(!str_prefix(arg, "status"))
	{
	}
}

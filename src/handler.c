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
#include <ctype.h>
#include <time.h>
#include "twilight.h"
#include "magic.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
#include "olc.h"
#include "interp.h"

SCRIPT_DATA * new_script args( () );

void	rand_name	args( (CHAR_DATA *ch) );
void	desc_gen	args( ( CHAR_DATA *ch ) );
void	free_quest	args( (QUEST_DATA * quest) );
void	fwrite_org	args( (ORG_DATA *org) );

CHAR_DATA *worldExtractedCharacters;
OBJ_DATA *worldExtractedObjects;

/*
 * Local functions.
 */
void	affect_modify	args( ( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd ) );


/* friend stuff -- for NPC's mostly */
bool is_friend(CHAR_DATA *ch,CHAR_DATA *victim)
{
    if (is_same_group(ch,victim))
	return TRUE;
    
    if (!IS_NPC(ch))
	return FALSE;

    if (IS_AFFECTED(ch,AFF_CHARM))
	return FALSE;

    if (IS_SET(ch->off_flags,ASSIST_ALL))
	return TRUE;

    if (ch->group && ch->group == victim->group)
	return TRUE;

    if (IS_SET(ch->off_flags,ASSIST_VNUM) 
    &&  ch->pIndexData == victim->pIndexData)
	return TRUE;

    return FALSE;
}

/* returns number of people on an object */
int count_users(OBJ_DATA *obj)
{
    CHAR_DATA *fch;
    int icountuser = 0;

    if (obj->in_room == NULL)
	return 0;

    for (fch = obj->in_room->people; fch != NULL; fch = fch->next_in_room)
	if (fch->on == obj)
	    icountuser++;

    return icountuser;
}
     
/* returns material number */
int material_lookup (const char *name)
{
	int iMat = 0;

	if(IS_NULLSTR(name))
		return 0;

	for ( iMat = 0; material_table[iMat].name != NULL; iMat++)
	{
		if (LOWER(name[0]) == LOWER(material_table[iMat].name[0]) &&  !str_prefix( name,material_table[iMat].name))
			return iMat;
	}

	return 0;
}

/* returns race number */
int race_lookup (const char *name)
{
   int iRace = 0;

	if(IS_NULLSTR(name))
		return 0;

   for ( iRace = 0; race_table[iRace].name != NULL; iRace++)
   {
	if (LOWER(name[0]) == LOWER(race_table[iRace].name[0])
	&&  !str_prefix( name,race_table[iRace].name))
	    return iRace;
   }

   return 0;
} 

/* returns auspice number */
int auspice_lookup (const char *name)
{
   int iAuspice = 0;

	if(IS_NULLSTR(name))
		return 0;

   for ( iAuspice = 0; auspice_table[iAuspice].name != NULL; iAuspice++)
   {
	if (LOWER(name[0]) == LOWER(auspice_table[iAuspice].name[0])
	&&  !str_prefix( name,auspice_table[iAuspice].name))
	    return iAuspice;
   }

   return 0;
} 

/* returns breed number */
int breed_lookup (const char *name)
{
   int iBreed = 0;

	if(IS_NULLSTR(name))
		return 0;

   for ( iBreed = 0; breed_table[iBreed].name != NULL; iBreed++)
   {
	if (LOWER(name[0]) == LOWER(breed_table[iBreed].name[0])
	&&  !str_prefix( name,breed_table[iBreed].name))
	    return iBreed;
   }

   return 0;
} 

int liq_lookup (const char *name)
{
    int iLiquid = 0;

	if(IS_NULLSTR(name))
		return 0;

    for ( iLiquid = 0; liq_table[iLiquid].liq_name != NULL; iLiquid++)
    {
	if (LOWER(name[0]) == LOWER(liq_table[iLiquid].liq_name[0])
	&& !str_prefix(name,liq_table[iLiquid].liq_name))
	    return iLiquid;
    }

    return -1;
}


int item_lookup(const char *name)
{
    int type = 0;

	if(IS_NULLSTR(name))
		return 0;

    for (type = 0; item_table[type].name != NULL; type++)
    {
        if (LOWER(name[0]) == LOWER(item_table[type].name[0])
        &&  !str_prefix(name,item_table[type].name))
            return item_table[type].type;
    }
 
    return -1;
}

char *item_name(int item_type)
{
    int type = 0;

    for (type = 0; item_table[type].name != NULL; type++)
	if (item_type == item_table[type].type)
	    return item_table[type].name;
    return "none";
}

int attack_lookup  (const char *name)
{
    int iAttack = 0;

	if(IS_NULLSTR(name))
		return 0;

    for ( iAttack = 0; attack_table[iAttack].name != NULL; iAttack++)
    {
	if (LOWER(name[0]) == LOWER(attack_table[iAttack].name[0])
	&&  !str_prefix(name,attack_table[iAttack].name))
	    return iAttack;
    }

    return 0;
}

/* returns a flag for wiznet */
long wiznet_lookup (const char *name)
{
	int flag = 0;

	for (flag = 0; wiznet_table[flag].name != NULL; flag++)
	{
		if (LOWER(name[0]) == LOWER(wiznet_table[flag].name[0]) && !str_prefix(name,wiznet_table[flag].name))
			return flag;
	}

	return -1;
}

/* returns clan number */
int clan_lookup (const char *name)
{
	int iClan = 0;

	if(IS_NULLSTR(name))
		return 0;

	for ( iClan = 0; iClan < MAX_CLAN; iClan++)
	{
		if (LOWER(name[0]) == LOWER(clan_table[iClan].name[0])
			&&  !str_prefix( name,clan_table[iClan].name)
			&&  str_prefix( name,""))
			return iClan;
	}

	return -1;
}

/* for immunity, vulnerabiltiy, and resistant
   the 'globals' (magic and weapons) may be overriden
   three other cases -- wood, silver, and iron -- are checked in fight.c */

int check_immune(CHAR_DATA *ch, int dam_type)
{
    int immune = -1, def = 0;
    int bit = 0;

    def = IS_NORMAL;

    if (dam_type == DAM_NONE)
	return immune;

    if (dam_type <= 3)
    {
    	if (IS_SET(ch->imm_flags,IMM_WEAPON))
    		def = IS_IMMUNE;
    	else if (IS_SET(ch->res_flags,RES_WEAPON))
    		def = IS_RESISTANT;
    	else if (IS_SET(ch->vuln_flags,VULN_WEAPON))
    		def = IS_VULNERABLE;
    }
    else /* magical attack */
    {	
	if (IS_SET(ch->imm_flags,IMM_MAGIC))
	    def = IS_IMMUNE;
	else if (IS_SET(ch->res_flags,RES_MAGIC))
	    def = IS_RESISTANT;
	else if (IS_SET(ch->vuln_flags,VULN_MAGIC))
	    def = IS_VULNERABLE;
    }

    /* set bits to check -- VULN etc. must ALL be the same or this will fail */
    switch (dam_type)
    {
	case(DAM_BASH):		bit = IMM_BASH;		break;
	case(DAM_PIERCE):	bit = IMM_PIERCE;	break;
	case(DAM_SLASH):	bit = IMM_SLASH;	break;
	case(DAM_FIRE):		bit = IMM_FIRE;		break;
	case(DAM_COLD):		bit = IMM_COLD;		break;
	case(DAM_LIGHTNING):	bit = IMM_LIGHTNING;	break;
	case(DAM_ACID):		bit = IMM_ACID;		break;
	case(DAM_POISON):	bit = IMM_POISON;	break;
	case(DAM_NEGATIVE):	bit = IMM_NEGATIVE;	break;
	case(DAM_HOLY):		bit = IMM_HOLY;		break;
	case(DAM_MENTAL):	bit = IMM_MENTAL;	break;
	case(DAM_DISEASE):	bit = IMM_DISEASE;	break;
	case(DAM_DROWNING):	bit = IMM_DROWNING;	break;
	default:		return def;
    }

    if (IS_SET(ch->imm_flags,bit))
	immune = IS_IMMUNE;
    else if (IS_SET(ch->res_flags,bit) && immune != IS_IMMUNE)
	immune = IS_RESISTANT;
    else if (IS_SET(ch->vuln_flags,bit))
    {
	if (immune == IS_IMMUNE)
	    immune = IS_RESISTANT;
	else if (immune == IS_RESISTANT)
	    immune = IS_NORMAL;
	else
	    immune = IS_VULNERABLE;
    }

    if (immune == -1)
	return def;
    else
      	return immune;
}

bool is_clan(CHAR_DATA *ch)
{
    return ch->clan;
}

bool is_same_clan(CHAR_DATA *ch, CHAR_DATA *victim)
{
	if (clan_table[ch->clan].independent)
		return FALSE;
	else 
		return (ch->clan == victim->clan);
}

/* for returning skill information */
int get_skill(CHAR_DATA *ch, int sn)
{
    int skill = 0;

    if (sn == -1) /* retrofit to cover mob skills */
	    skill = get_curr_stat(ch, STAT_DEX) + ch->ability[GENERAL].value;

    else if (sn < -1 || sn > MAX_SKILL)
    {
	bug("Bad sn %d in get_skill.",sn);
	skill = 0;
    }

        if ((skill_table[sn].spell_fun != spell_null)
	&&  IS_NPC(ch))
	    skill = get_curr_stat(ch, STAT_DEX) + ch->ability[GENERAL].value;

	else if (sn == gsn_sneak || sn == gsn_hide)
	    skill = get_curr_stat(ch, STAT_DEX) + ch->ability[STEALTH].value;

        else if (sn == gsn_dodge && IS_SET(ch->off_flags,OFF_DODGE))
	    skill = get_curr_stat(ch, STAT_DEX) + ch->ability[DODGE].value;
 
	else if (sn == gsn_parry && IS_SET(ch->off_flags,OFF_PARRY))
	    skill = get_curr_stat(ch, STAT_DEX) + ch->ability[MELEE].value;

	else if (sn == gsn_brawl)
	    skill = get_curr_stat(ch, STAT_DEX) + ch->ability[BRAWL].value;

 	else if (sn == gsn_trip && IS_SET(ch->off_flags,OFF_TRIP))
	    skill = get_curr_stat(ch, STAT_DEX) + ch->ability[BRAWL].value;

 	else if (sn == gsn_bash && IS_SET(ch->off_flags,OFF_BASH))
	    skill = get_curr_stat(ch, STAT_DEX) + ch->ability[BRAWL].value;

	else if (sn == gsn_disarm 
	     &&  (IS_SET(ch->off_flags,OFF_DISARM)))
	    skill = get_curr_stat(ch, STAT_DEX) + ch->ability[MELEE].value;

	else if (sn == gsn_kick)
	    skill = get_curr_stat(ch, STAT_DEX) + ch->ability[BRAWL].value;

	else if (sn == gsn_backstab)
	    skill = get_curr_stat(ch, STAT_DEX) + ch->ability[MELEE].value;

  	else if (sn == gsn_rescue)
	    skill = get_curr_stat(ch, STAT_DEX) + ch->ability[ATHLETICS].value;

	else if (sn == gsn_firearm
	||  sn == gsn_blade
	||  sn == gsn_blunt
	||  sn == gsn_grapple
	||  sn == gsn_whip)
	    skill = get_curr_stat(ch, STAT_DEX) + ch->ability[sn].value;

	else 
	   skill = 0;

    if (ch->daze > 0)
    {
	if (skill_table[sn].spell_fun != spell_null)
	    skill /= 2;
	else
	    skill = 2 * skill / 3;
    }

    if ( !IS_NPC(ch) && ch->condition[COND_DRUNK]  > 10 )
	skill = 9 * skill / 10;
    if ( !IS_NPC(ch) && ch->condition[COND_HIGH]  > 10 )
	skill = 6 * skill / 10;
    if ( !IS_NPC(ch) && ch->condition[COND_TRIPPING]  > 10 )
	skill = 4 * skill / 10;

    return skill;
}

/* for returning weapon information */
int get_weapon_sn(CHAR_DATA *ch)
{
    OBJ_DATA *wield;
    int sn = 0;

    wield = get_eq_char( ch, WEAR_WIELD );
    if (wield == NULL || wield->item_type != ITEM_WEAPON)
        sn = gsn_brawl;
    else switch (wield->value[0])
    {
        default :               sn = -1;                break;
        case(WEAPON_FIREARM):   sn = gsn_firearm;         break;
        case(WEAPON_BLADE):     sn = gsn_blade;        break;
        case(WEAPON_BLUNT):     sn = gsn_blunt;         break;
        case(WEAPON_GRAPPLE):   sn = gsn_grapple;          break;
        case(WEAPON_WHIP):      sn = gsn_whip;          break;
   }
   return sn;
}

int get_weapon_skill(CHAR_DATA *ch, int sn)
{
     int skill = 0;

     /* -1 is exotic */
    if (IS_NPC(ch))
    {
	if (sn == -1)
	    skill = get_curr_stat(ch, STAT_DEX) + ch->ability[MELEE].value;
	else if (sn == gsn_brawl)
	    skill = get_curr_stat(ch, STAT_DEX) + ch->ability[BRAWL].value;
	else if (sn == gsn_melee)
	    skill = get_curr_stat(ch, STAT_DEX) + ch->ability[MELEE].value;
	else if (sn == gsn_firearm)
	    skill = get_curr_stat(ch, STAT_DEX) + ch->ability[FIREARMS].value;
    }
    
    else
    {
	if (sn == -1)
	    skill = get_curr_stat(ch, STAT_DEX) + ch->ability[MELEE].value;
	else
	    skill = get_curr_stat(ch, STAT_DEX) + ch->ability[sn].value;
    }

    return skill;
} 


/* used to de-screw characters */
void reset_char(CHAR_DATA *ch)
{
     int loc = 0,mod,stat = 0;
     OBJ_DATA *obj;
     AFFECT_DATA *af;
     /*int i;*/

     if (IS_NPC(ch))
	return;

    if ( ch->pcdata->full_reset )
    {
    /* do a FULL reset */
	for (loc = 0; loc < MAX_WEAR; loc++)
	{
	    obj = get_eq_char(ch,loc);
	    if (obj == NULL)
		continue;
	    if (!obj->enchanted)
	    for ( af = obj->pIndexData->affected; af != NULL; af = af->next )
	    {
		mod = af->modifier;
		switch(af->location)
		{
		    case APPLY_SEX:	ch->sex		-= mod;
					if (ch->sex < 0 || ch->sex >2)
					    ch->sex = IS_NPC(ch) ?
						0 :
						ch->pcdata->true_sex;
									break;
/*		    case APPLY_AGE:	ch->age -= mod;
					break;
		    case APPLY_HEIGHT:
					ch->height -= mod;
					break;
		    case APPLY_WEIGHT:
					ch->weight -= mod;
					 break;
*/		    case APPLY_GHB:
					ch->GHB -= mod;
					 break;
		    case APPLY_RBPG:
					ch->RBPG -= mod;
					 break;
		    case APPLY_WILLPOWER:
					ch->willpower -= mod;
					 break;
		    case APPLY_CONSCIENCE:
					break;
		    case APPLY_SELF_CONTROL:
					break;
		    case APPLY_COURAGE:
					break;
		    case APPLY_PAIN:
					ch->condition[COND_PAIN] -= mod;
					break;
		    case APPLY_ANGER:
					ch->condition[COND_ANGER] -= mod;
					break;
		    case APPLY_FEAR:
					ch->condition[COND_FEAR] -= mod;
					break;
		    case APPLY_FRENZY:
					ch->condition[COND_FRENZY] -= mod;
					break;
		    case APPLY_HEALTH:
					ch->health -= mod;
					ch->agghealth -= mod;
					break;
		    case APPLY_DICE:
					ch->dice_mod -= mod;
					break;
		    case APPLY_DIFFICULTY:
					ch->diff_mod -= mod;
					break;
		    case APPLY_SKILL:
					ch->ability[af->bitvector].value -= mod;
					break;
		}
	    }

            for ( af = obj->affected; af != NULL; af = af->next )
            {
                mod = af->modifier;
                switch(af->location)
                {
                    case APPLY_SEX:     ch->sex         -= mod; break;
/*		    case APPLY_AGE:	ch->age -= mod; break;
		    case APPLY_HEIGHT: ch->height -= mod; break;
		    case APPLY_WEIGHT: ch->weight -= mod; break;
*/		    case APPLY_GHB: ch->GHB -= mod; break;
		    case APPLY_RBPG: ch->RBPG -= mod; break;
		    case APPLY_WILLPOWER: ch->willpower -= mod; break;
		    case APPLY_CONSCIENCE: break;
		    case APPLY_SELF_CONTROL: break;
		    case APPLY_COURAGE: break;
		    case APPLY_PAIN: ch->condition[COND_PAIN] -= mod; break;
		    case APPLY_ANGER: ch->condition[COND_ANGER] -= mod; break;
		    case APPLY_FEAR: ch->condition[COND_FEAR] -= mod; break;
		    case APPLY_FRENZY: ch->condition[COND_FRENZY] -= mod; break;
		    case APPLY_HEALTH: ch->health -= mod; ch->agghealth -= mod;
					break;
		    case APPLY_DICE: ch->dice_mod -= mod; break;
		    case APPLY_DIFFICULTY: ch->diff_mod -= mod; break;
		    case APPLY_SKILL: ch->ability[af->bitvector].value -= mod;
					break;
                }
            }
	}
	/* now reset the permanent stats */
	if (ch->pcdata->true_sex < 0 || ch->pcdata->true_sex > 2)
	{
		if (ch->sex > 0 && ch->sex < 3)
	    	    ch->pcdata->true_sex	= ch->sex;
		else
		    ch->pcdata->true_sex 	= 0;
	}
    }

    /* now restore the character to his/her true condition */
    for (stat = 0; stat < MAX_STATS; stat++)
	ch->mod_stat[stat] = 0;

    if (ch->pcdata->true_sex < 0 || ch->pcdata->true_sex > 2)
	ch->pcdata->true_sex = 0;
    ch->sex		= ch->pcdata->true_sex;
   
    /* now start adding back the effects */
    for (loc = 0; loc < MAX_WEAR; loc++)
    {
        obj = get_eq_char(ch,loc);
        if (obj == NULL)
            continue;

        if (!obj->enchanted)
	for ( af = obj->pIndexData->affected; af != NULL; af = af->next )
        {
            mod = af->modifier;
            switch(af->location)
            {
                case APPLY_STR:         ch->mod_stat[STAT_STR]  += mod; break;
                case APPLY_DEX:         ch->mod_stat[STAT_DEX]  += mod; break;
                case APPLY_STA:         ch->mod_stat[STAT_STA]  += mod; break;
                case APPLY_CHA:         ch->mod_stat[STAT_CHA]  += mod; break;
                case APPLY_MAN:         ch->mod_stat[STAT_MAN]  += mod; break;
                case APPLY_APP:         ch->mod_stat[STAT_APP]  += mod; break;
                case APPLY_PER:         ch->mod_stat[STAT_PER]  += mod; break;
                case APPLY_INT:         ch->mod_stat[STAT_INT]  += mod; break;
                case APPLY_WIT:         ch->mod_stat[STAT_WIT]  += mod; break;

		case APPLY_SEX:		ch->sex			+= mod; break;
/*		case APPLY_AGE:		ch->age			+= mod; break;
		case APPLY_HEIGHT:	ch->height		+= mod; break;
		case APPLY_WEIGHT:	ch->weight		+= mod; break;
*/		case APPLY_GHB:		ch->GHB			+= mod;	break;
		case APPLY_RBPG:	ch->RBPG		+= mod; break;
		case APPLY_WILLPOWER:	ch->willpower		+= mod; break;
		case APPLY_CONSCIENCE:					break;
		case APPLY_SELF_CONTROL:				break;
		case APPLY_COURAGE:					break;
		case APPLY_PAIN:	ch->condition[COND_PAIN] += mod;break;
		case APPLY_ANGER:	ch->condition[COND_ANGER]+= mod;break;
		case APPLY_FEAR:	ch->condition[COND_FEAR] += mod;break;
		case APPLY_FRENZY:	ch->condition[COND_FRENZY]+= mod;break;
		case APPLY_HEALTH:	ch->health += mod;
					ch->agghealth += mod;
					break;
		case APPLY_DICE:	ch->dice_mod		+= mod; break;
		case APPLY_DIFFICULTY:	ch->diff_mod		+= mod; break;
		case APPLY_SKILL: ch->ability[af->bitvector].value += mod;
					break;
	    }
        }
 
        for ( af = obj->affected; af != NULL; af = af->next )
        {
            mod = af->modifier;
            switch(af->location)
            {
                case APPLY_STR:         ch->mod_stat[STAT_STR]  += mod; break;
                case APPLY_DEX:         ch->mod_stat[STAT_DEX]  += mod; break;
                case APPLY_STA:         ch->mod_stat[STAT_STA]  += mod; break;
                case APPLY_CHA:         ch->mod_stat[STAT_CHA]  += mod; break;
                case APPLY_MAN:         ch->mod_stat[STAT_MAN]  += mod; break;
                case APPLY_APP:         ch->mod_stat[STAT_APP]  += mod; break;
                case APPLY_PER:         ch->mod_stat[STAT_PER]  += mod; break;
                case APPLY_INT:         ch->mod_stat[STAT_INT]  += mod; break;
                case APPLY_WIT:         ch->mod_stat[STAT_WIT]  += mod; break;
 
                case APPLY_SEX:         ch->sex                 += mod; break;
/*		case APPLY_AGE:		ch->age			+= mod; break;
		case APPLY_HEIGHT:	ch->height		+= mod; break;
		case APPLY_WEIGHT:	ch->weight		+= mod; break;
*/		case APPLY_GHB:		ch->GHB			+= mod; break;
		case APPLY_RBPG:	ch->RBPG		+= mod; break;
		case APPLY_WILLPOWER:	ch->willpower		+= mod; break;
		case APPLY_CONSCIENCE:					break;
		case APPLY_SELF_CONTROL:				break;
		case APPLY_COURAGE:					break;
		case APPLY_PAIN:	ch->condition[COND_PAIN]+= mod; break;
		case APPLY_ANGER:	ch->condition[COND_ANGER]+= mod;break;
		case APPLY_FEAR:	ch->condition[COND_FEAR]+= mod; break;
		case APPLY_FRENZY:	ch->condition[COND_FRENZY]+=mod;break;
		case APPLY_HEALTH:	ch->health += mod;
					ch->agghealth += mod;
					break;
		case APPLY_DICE:	ch->dice_mod		+= mod; break;
		case APPLY_DIFFICULTY:	ch->diff_mod		+= mod; break;
		case APPLY_SKILL: ch->ability[af->bitvector].value += mod;
					break;
            }
	}
    }
  
    /* now add back spell effects */
    for (af = ch->affected; af != NULL; af = af->next)
    {
        mod = af->modifier;
        switch(af->location)
        {
                case APPLY_STR:         ch->mod_stat[STAT_STR]  += mod; break;
                case APPLY_DEX:         ch->mod_stat[STAT_DEX]  += mod; break;
                case APPLY_STA:         ch->mod_stat[STAT_STA]  += mod; break;
                case APPLY_CHA:         ch->mod_stat[STAT_CHA]  += mod; break;
                case APPLY_MAN:         ch->mod_stat[STAT_MAN]  += mod; break;
                case APPLY_APP:         ch->mod_stat[STAT_APP]  += mod; break;
                case APPLY_PER:         ch->mod_stat[STAT_PER]  += mod; break;
                case APPLY_INT:         ch->mod_stat[STAT_INT]  += mod; break;
                case APPLY_WIT:         ch->mod_stat[STAT_WIT]  += mod; break;
 
                case APPLY_SEX:         ch->sex                 += mod; break;
/*		case APPLY_AGE:         ch->age                 += mod; break;
		case APPLY_HEIGHT:      ch->height              += mod; break;
		case APPLY_WEIGHT:      ch->weight              += mod; break;
*/		case APPLY_GHB:         ch->GHB                 += mod; break;
		case APPLY_RBPG:        ch->RBPG                += mod; break;
		case APPLY_WILLPOWER:   ch->willpower           += mod; break;
		case APPLY_CONSCIENCE:                                  break;
		case APPLY_SELF_CONTROL:                                break;
		case APPLY_COURAGE:                                     break;
		case APPLY_PAIN:        ch->condition[COND_PAIN]+= mod; break;
		case APPLY_ANGER:       ch->condition[COND_ANGER]+=mod; break;
		case APPLY_FEAR:        ch->condition[COND_FEAR] +=mod; break;
		case APPLY_FRENZY:      ch->condition[COND_FRENZY]+=mod;break;
		case APPLY_HEALTH:	ch->health		+= mod;
					ch->agghealth		+= mod; break;
		case APPLY_DICE:	ch->dice_mod		+= mod; break;
		case APPLY_DIFFICULTY:	ch->diff_mod		+= mod; break;
		case APPLY_SKILL: ch->ability[af->bitvector].value += mod;
					break;
        } 
    }

    /* make sure sex is RIGHT!!!! */
    if (ch->sex < 0 || ch->sex > 2)
	ch->sex = ch->pcdata->true_sex;

    REMOVE_BIT(ch->act, ACT_REINCARNATE);
}


/*
 * Retrieve a character's trusted level for permission checking.
 */
int get_trust( CHAR_DATA *ch )
{
    assert(ch);

    if ( ch->desc != NULL && ch->desc->original != NULL )
	ch = ch->desc->original;

	return ch->trust;
}


/*
 * Retrieve a character's age.
 */
int get_age( CHAR_DATA *ch )
{
	if(ch->char_age <= 0)
		return 15 + ( ch->played + (int) (current_time - ch->logon) ) / 72000;
	else return ch->char_age;
}

/* command for retrieving stats */
int get_curr_stat( CHAR_DATA *ch, int stat )
{
    int max = 0, shapemod = 0;
    if(ch->race == race_lookup("vampire")) 
    	max = 10;
    else if(ch->race == race_lookup("werewolf"))
	{
	    if(stat == stat_lookup("strength", ch)
	      || stat == stat_lookup("dexterity", ch)
	      || stat == stat_lookup("stamina", ch))
		max = 25;
	    else
		max = 10;
	}
    else if(ch->race == race_lookup("human"))
	if(IS_SET(ch->act2, ACT2_GHOUL) && stat == stat_lookup("strength", ch))
	{
	    shapemod = 1;
	    max = 6;
	}
	else
	    max = 5;
    else max = 10;

    if(ch->shape == SHAPE_CRINOS)
    {
	if(stat == stat_lookup("strength", ch))
	    shapemod = 4 + ch->max_RBPG/2;
	else if(stat == stat_lookup("stamina", ch))
	    shapemod = 1 + ch->max_RBPG/2;
	else if(stat == stat_lookup("dexterity", ch))
	    shapemod = 3 + ch->max_RBPG/2;
	else if(stat == stat_lookup("perception", ch))
	    shapemod = 1;
    }
    if(ch->race == race_lookup("werewolf") && ch->shape == SHAPE_WOLF)
    {
	if(stat == stat_lookup("strength", ch))
	    shapemod = ch->max_RBPG/2;
	else if(stat == stat_lookup("stamina", ch))
	    shapemod = ch->max_RBPG/4;
	else if(stat == stat_lookup("dexterity", ch))
	    shapemod = ch->max_RBPG/2;
	else if(stat == stat_lookup("perception", ch))
	    shapemod = 2;
    }

    if(IS_SET(ch->parts, PART_BIGNOSE)
    && stat == stat_lookup("appearance", ch))
	shapemod -= 1;

    if(IS_SET(ch->parts, PART_DISTORTION)
    && stat == stat_lookup("appearance", ch))
	shapemod -= 3;

    return UMIN(ch->perm_stat[stat] + ch->mod_stat[stat] + shapemod, max);
}

/*
 * Retrieve a character's carry capacity.
 */
int can_carry_n( CHAR_DATA *ch )
{
	if ( !IS_NPC(ch) && ch->trust >= LEVEL_IMMORTAL )
		return 1000;

	if ( IS_NPC(ch) && IS_SET(ch->act, ACT_PET) )
		return 0;

	return 100;
}



/*
 * Retrieve a character's carry capacity.
 */
int can_carry_w( CHAR_DATA *ch )
{
	if ( !IS_NPC(ch) && ch->trust >= LEVEL_IMMORTAL )
		return 10000000;

	if ( IS_NPC(ch) && IS_SET(ch->act, ACT_PET) )
		return 0;

	if(ch->race == race_lookup("vampire"))
		return ((get_curr_stat(ch,STAT_STR) + ch->disc[DISC_POTENCE]) * 1100);
	else if(ch->race == race_lookup("werewolf") && ch->shape == SHAPE_CRINOS)
		return ((get_curr_stat(ch,STAT_STR)+ch->ability[PRIMAL_URGE].value)*1400);
	else if(ch->race == race_lookup("human") && IS_SET(ch->act2, ACT2_GHOUL))
		return ((get_curr_stat(ch,STAT_STR) + 1) * 1100);
	else
		return (get_curr_stat(ch,STAT_STR) * 1100);
}


/*
 * See if a string is one of the names of an object.
 */

bool is_name ( char *str, char *namelist )
{
	char name[MAX_INPUT_LENGTH]={'\0'};
	char part[MAX_INPUT_LENGTH]={'\0'};
	char *list, *string;

	/* fix crash on NULL namelist */
	if (IS_NULLSTR(namelist))
		return FALSE;

	/* fixed to prevent is_name on "" returning TRUE */
	if (str[0] == '\0')
		return FALSE;

	string = str;
	/* we need ALL parts of string to match part of namelist */
	for ( ; ; )  /* start parsing string */
	{
		str = one_argument(str,part);

		if (IS_NULLSTR(part) )
			return TRUE;

		/* check to see if this is part of namelist */
		list = namelist;
		for ( ; ; )  /* start parsing namelist */
		{
			list = one_argument(list,name);
			if (IS_NULLSTR(name))  /* this name was not found */
				return FALSE;

			if (!str_prefix(string,name))
				return TRUE; /* full pattern match */

			if (!str_prefix(part,name))
				break;
		}
	}
}

bool is_exact_name(char *str, char *namelist )
{
	char name[MAX_INPUT_LENGTH]={'\0'};

	if (namelist == NULL)
		return FALSE;

	for ( ; ; )
	{
		namelist = one_argument( namelist, name );
		if ( IS_NULLSTR(name) )
			return FALSE;
		if ( !str_cmp( str, name ) )
			return TRUE;
	}
}

/* enchanted stuff for eq */
void affect_enchant(OBJ_DATA *obj)
{
    /* okay, move all the old flags into new vectors if we have to */
    if (!obj->enchanted)
    {
        AFFECT_DATA *paf, *af_new;
        obj->enchanted = TRUE;

        for (paf = obj->pIndexData->affected;
             paf != NULL; paf = paf->next)
        {
	    af_new = new_affect();

            af_new->next = obj->affected;
            obj->affected = af_new;
 
	    af_new->where	= paf->where;
            af_new->type        = UMAX(0,paf->type);
            af_new->level       = paf->level;
            af_new->duration    = paf->duration;
            af_new->location    = paf->location;
            af_new->modifier    = paf->modifier;
            af_new->bitvector   = paf->bitvector;
        }
    }
}


/*
 * Apply or remove an affect to a character.
 */
void affect_modify( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd )
{
    OBJ_DATA *wield;
    int mod = 0;

    mod = paf->modifier;

    if ( fAdd )
    {
	switch (paf->where)
	{
	default: break;
	case TO_AFFECTS:
	    SET_BIT(ch->affected_by, paf->bitvector);
	    break;
	case TO_AFF2:
	    SET_BIT(ch->affected_by2, paf->bitvector);
	    break;
	case TO_IMMUNE:
	    SET_BIT(ch->imm_flags,paf->bitvector);
	    break;
	case TO_RESIST:
	    SET_BIT(ch->res_flags,paf->bitvector);
	    break;
	case TO_VULN:
	    SET_BIT(ch->vuln_flags,paf->bitvector);
	    break;
	}
    }
    else
    {
        switch (paf->where)
        {
	default: break;
        case TO_AFFECTS:
            REMOVE_BIT(ch->affected_by, paf->bitvector);
            break;
        case TO_AFF2:
            REMOVE_BIT(ch->affected_by2, paf->bitvector);
            break;
        case TO_IMMUNE:
            REMOVE_BIT(ch->imm_flags,paf->bitvector);
            break;
        case TO_RESIST:
            REMOVE_BIT(ch->res_flags,paf->bitvector);
            break;
        case TO_VULN:
            REMOVE_BIT(ch->vuln_flags,paf->bitvector);
            break;
        }
	mod = 0 - mod;
    }

    switch ( paf->location )
    {
    default:
	bug( "Affect_modify: unknown location %d.", paf->location );
	return;

    case APPLY_NONE:						break;
    case APPLY_STR:           ch->mod_stat[STAT_STR]	+= mod;	break;
    case APPLY_DEX:           ch->mod_stat[STAT_DEX]	+= mod;	break;
    case APPLY_STA:           ch->mod_stat[STAT_STA]	+= mod;	break;
    case APPLY_CHA:           ch->mod_stat[STAT_CHA]	+= mod;	break;
    case APPLY_MAN:           ch->mod_stat[STAT_MAN]	+= mod;	break;
    case APPLY_APP:           ch->mod_stat[STAT_APP]	+= mod;	break;
    case APPLY_PER:           ch->mod_stat[STAT_PER]	+= mod;	break;
    case APPLY_INT:           ch->mod_stat[STAT_INT]	+= mod;	break;
    case APPLY_WIT:           ch->mod_stat[STAT_WIT]	+= mod;	break;
    case APPLY_SEX:           ch->sex			+= mod;	break;
/*  case APPLY_AGE:	      ch->age			+= mod; break;
    case APPLY_HEIGHT:	      ch->height		+= mod; break;
    case APPLY_WEIGHT:	      ch->weight		+= mod; break;
*/  case APPLY_SPELL_AFFECT:  					break;
    case APPLY_GHB:	      ch->GHB			+= mod; break;
    case APPLY_RBPG:	      ch->RBPG			+= mod; break;
    case APPLY_WILLPOWER:     ch->willpower		+= mod; break;
    case APPLY_CONSCIENCE:					break;
    case APPLY_SELF_CONTROL:					break;
    case APPLY_COURAGE:						break;
    case APPLY_PAIN:	      ch->condition[COND_PAIN]  += mod; break;
    case APPLY_ANGER:	      ch->condition[COND_ANGER] += mod; break;
    case APPLY_FEAR:	      ch->condition[COND_FEAR]	+= mod; break;
    case APPLY_FRENZY:	      ch->condition[COND_FRENZY]+= mod; break;
    case APPLY_HEALTH: ch->health += mod; ch->agghealth += mod; break;
    case APPLY_DICE:	 	ch->dice_mod		+= mod; break;
    case APPLY_DIFFICULTY:	ch->diff_mod		+= mod; break;
    case APPLY_SKILL: ch->ability[paf->bitvector].value += mod; break;
    }

    /*
     * Check for weapon wielding.
     * Guard against recursion (for weapons with affects).
     */
    if ( !IS_NPC(ch) && ( wield = get_eq_char( ch, WEAR_WIELD ) ) != NULL
    &&   get_obj_weight(wield) > (get_curr_stat(ch,STAT_STR)*100))
    {
	static int depth = 0;

	if ( depth == 0 )
	{
	    depth++;
	    act( "You drop $p.", ch, wield, NULL, TO_CHAR, 1 );
	    act( "$n drops $p.", ch, wield, NULL, TO_ROOM, 0 );
	    obj_from_char( wield );
	    obj_to_room( wield, ch->in_room );
	    depth--;
	}
    }

    return;
}


/*
 * Apply or remove an affect to a room.
 */
void affect_modify_room( ROOM_INDEX_DATA *rm, AFFECT_DATA *paf, bool fAdd )
{
	int mod = 0;

	mod = paf->modifier;

	if ( fAdd )
	{
		switch (paf->where)
		{
		case TO_ROOM:
			SET_BIT(rm->room_flags, paf->bitvector);
			break;
		}
	}
	else
	{
		switch (paf->where)
		{
		case TO_ROOM:
			REMOVE_BIT(rm->room_flags, paf->bitvector);
			break;
		}
		mod = 0 - mod;
	}

	switch ( paf->location )
	{
	default:
		bug( "Affect_modify_room: unknown location %d.", paf->location );
		return;

	case APPLY_NONE:						break;
	case APPLY_ROOM_LIGHT:         rm->light		+= mod;	break;
	}

	return;
}


/* find an effect in an affect list */
AFFECT_DATA  *affect_find(AFFECT_DATA *paf, int sn)
{
	AFFECT_DATA *paf_find;

	for ( paf_find = paf; paf_find != NULL; paf_find = paf_find->next )
	{
		if ( paf_find->type == sn )
			return paf_find;
	}

	return NULL;
}

/* fix object affects when removing one */
void affect_check(CHAR_DATA *ch,int where,int vector)
{
    AFFECT_DATA *paf;
    OBJ_DATA *obj;

    if (where == TO_OBJECT || where == TO_WEAPON || vector == 0)
	return;

    for (paf = ch->affected; paf != NULL; paf = paf->next)
	if (paf->where == where && paf->bitvector == vector)
	{
	    switch (where)
	    {
	        case TO_AFFECTS:
		    SET_BIT(ch->affected_by,vector);
		    break;
	        case TO_AFF2:
		    SET_BIT(ch->affected_by2,vector);
		    break;
	        case TO_IMMUNE:
		    SET_BIT(ch->imm_flags,vector);   
		    break;
	        case TO_RESIST:
		    SET_BIT(ch->res_flags,vector);
		    break;
	        case TO_VULN:
		    SET_BIT(ch->vuln_flags,vector);
		    break;
	    }
	    return;
	}

    for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
    {
	if (obj->wear_loc == -1)
	    continue;

            for (paf = obj->affected; paf != NULL; paf = paf->next)
            if (paf->where == where && paf->bitvector == vector)
            {
                switch (where)
                {
                    case TO_AFFECTS:
                        SET_BIT(ch->affected_by,vector);
                        break;
                    case TO_AFF2:
                        SET_BIT(ch->affected_by2,vector);
                        break;
                    case TO_IMMUNE:
                        SET_BIT(ch->imm_flags,vector);
                        break;
                    case TO_RESIST:
                        SET_BIT(ch->res_flags,vector);
                        break;
                    case TO_VULN:
                        SET_BIT(ch->vuln_flags,vector);
                  
                }
                return;
            }

        if (obj->enchanted)
	    continue;

        for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
            if (paf->where == where && paf->bitvector == vector)
            {
                switch (where)
                {
                    case TO_AFFECTS:
                        SET_BIT(ch->affected_by,vector);
                        break;
                    case TO_AFF2:
                        SET_BIT(ch->affected_by2,vector);
                        break;
                    case TO_IMMUNE:
                        SET_BIT(ch->imm_flags,vector);
                        break;
                    case TO_RESIST:
                        SET_BIT(ch->res_flags,vector);
                        break;
                    case TO_VULN:
                        SET_BIT(ch->vuln_flags,vector);
                        break;
                }
                return;
            }
    }
}

/*
 * Give an affect to a char.
 */
void affect_to_char( CHAR_DATA *ch, AFFECT_DATA *paf )
{
	AFFECT_DATA *paf_new;

	paf_new = new_affect();

	*paf_new		= *paf;

	paf_new->next	= ch->affected;
	ch->affected	= paf_new;

	affect_modify( ch, paf_new, TRUE );
	return;
}

/* give an affect to an object */
void affect_to_obj(OBJ_DATA *obj, AFFECT_DATA *paf)
{
	AFFECT_DATA *paf_new;

	paf_new = new_affect();

	*paf_new		= *paf;

	paf_new->next	= obj->affected;
	obj->affected	= paf_new;

	/* apply any affect vectors to the object's extra_flags */
	if (paf->bitvector)
		switch (paf->where)
		{
		case TO_OBJECT:
			SET_BIT(obj->extra_flags,paf->bitvector);
			break;
		case TO_WEAPON:
			if (obj->item_type == ITEM_WEAPON)
				SET_BIT(obj->value[4],paf->bitvector);
			break;
		}


	return;
}


/* give an affect to a room */
void affect_to_room(ROOM_INDEX_DATA *room, AFFECT_DATA *paf)
{
	AFFECT_DATA *paf_new;

	paf_new = new_affect();

	*paf_new		= *paf;

	paf_new->next	= room->affects;
	room->affects	= paf_new;

	affect_modify_room( room, paf_new, TRUE );

	return;
}



/*
 * Remove an affect from a char.
 */
void affect_remove( CHAR_DATA *ch, AFFECT_DATA *paf )
{
	int where = 0;
	int vector = 0;

	if ( ch->affected == NULL )
	{
		bug( "Affect_remove: no affect.", 0 );
		return;
	}

	affect_modify( ch, paf, FALSE );
	where = paf->where;
	vector = paf->bitvector;

	if ( paf == ch->affected )
	{
		ch->affected	= paf->next;
	}
	else
	{
		AFFECT_DATA *prev;

		for ( prev = ch->affected; prev != NULL; prev = prev->next )
		{
			if ( prev->next == paf )
			{
				prev->next = paf->next;
				break;
			}
		}

		if ( prev == NULL )
		{
			bug( "Affect_remove: cannot find paf.", 0 );
			return;
		}
	}

	free_affect(paf);

	affect_check(ch,where,vector);
	return;
}

void affect_remove_obj( OBJ_DATA *obj, AFFECT_DATA *paf)
{
    int where = 0, vector = 0;
    if ( obj->affected == NULL )
    {
        bug( "Affect_remove_object: no affect.", 0 );
        return;
    }

    if (obj->carried_by != NULL && obj->wear_loc != -1)
	affect_modify( obj->carried_by, paf, FALSE );

    where = paf->where;
    vector = paf->bitvector;

    /* remove flags from the object if needed */
    if (paf->bitvector)
	switch( paf->where)
        {
        case TO_OBJECT:
            REMOVE_BIT(obj->extra_flags,paf->bitvector);
            break;
        case TO_WEAPON:
            if (obj->item_type == ITEM_WEAPON)
                REMOVE_BIT(obj->value[4],paf->bitvector);
            break;
        }

    if ( paf == obj->affected )
    {
        obj->affected    = paf->next;
    }
    else
    {
        AFFECT_DATA *prev;

        for ( prev = obj->affected; prev != NULL; prev = prev->next )
        {
            if ( prev->next == paf )
            {
                prev->next = paf->next;
                break;
            }
        }

        if ( prev == NULL )
        {
            bug( "Affect_remove_object: cannot find paf.", 0 );
            return;
        }
    }

    free_affect(paf);

    if (obj->carried_by != NULL && obj->wear_loc != -1)
	affect_check(obj->carried_by,where,vector);
    return;
}

/*
 * Remove affects from a room.
 */
void affect_remove_room( ROOM_INDEX_DATA *room, AFFECT_DATA *paf)
{
    int where = 0, vector = 0;
    if ( room->affects == NULL )
    {
        bug( "Affect_remove_room: no affect.", 0 );
        return;
    }

    where = paf->where;
    vector = paf->bitvector;

    /* remove flags from the room if needed */
    if (paf->bitvector)
        switch( paf->where)
        {
	    default:
            REMOVE_BIT(room->room_flags,paf->bitvector);
	    break;
        }

    if ( paf == room->affects )
    {
        room->affects    = paf->next;
    }
    else
    {
        AFFECT_DATA *prev;

        for ( prev = room->affects; prev != NULL; prev = prev->next )
        {
            if ( prev->next == paf )
            {
                prev->next = paf->next;
                break;
            }
        }

        if ( prev == NULL )
        {
            bug( "Affect_remove_room: cannot find paf.", 0 );
            return;
        }
    }

    free_affect(paf);

    return;
}

/*
 * Strip all affects of a given sn.
 */
void affect_strip( CHAR_DATA *ch, int sn )
{
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;

	for ( paf = ch->affected; paf != NULL; paf = paf_next )
	{
		paf_next = paf->next;
		if ( paf->type == sn )
			affect_remove( ch, paf );
	}

	return;
}



/*
 * Return true if a char is affected by a spell.
 */
bool is_affected( CHAR_DATA *ch, int sn )
{
	AFFECT_DATA *paf;

	for ( paf = ch->affected; paf != NULL; paf = paf->next )
	{
		if ( paf->type == sn )
			return TRUE;
	}

	if(ch->in_room != NULL) {
		for ( paf = ch->in_room->affects; paf != NULL; paf = paf->next )
		{
			if ( paf->type == sn
					&& skill_table[sn].target == TAR_ROOM_CHAR)
				return TRUE;
		}
	}

	return FALSE;
}


/*
 * Return true if an obj is affected by a spell.
 */
bool is_affected_obj( OBJ_DATA *obj, int sn )
{
	AFFECT_DATA *paf;

	for ( paf = obj->affected; paf != NULL; paf = paf->next )
	{
		if ( paf->type == sn )
			return TRUE;
	}

	/*
    if(obj->in_room != NULL) {
	for ( paf = obj->in_room->affects; paf != NULL; paf = paf->next )
	{
	    if ( paf->type == sn
		&& skill_table[sn].target == TAR_ROOM_OBJ)
		return TRUE;
	}
    }
	 */

	return FALSE;
}



/*
 * Add or enhance an affect.
 */
void affect_join( CHAR_DATA *ch, AFFECT_DATA *paf )
{
	AFFECT_DATA *paf_old;
	bool found = FALSE;

	found = FALSE;
	for ( paf_old = ch->affected; paf_old != NULL; paf_old = paf_old->next )
	{
		if ( paf_old->type == paf->type )
		{
			paf->level = (paf->level += paf_old->level) / 2;
			paf->duration += paf_old->duration;
			paf->modifier += paf_old->modifier;
			affect_remove( ch, paf_old );
			break;
		}
	}

	affect_to_char( ch, paf );
	return;
}

/*
 * Remove a char from char_list. (Necessary for reincarnation to work.)
 */
void char_from_list( CHAR_DATA *ch )
{
	UNLINK_SINGLE(ch, next, CHAR_DATA, char_list);
}

/*
 * Move a char out of a room's listeners.
 */
void char_from_listeners( CHAR_DATA *ch )
{

	if ( ch->listening == NULL )
	{
		bug( "Char_from_listeners: NULL.", 0 );
		return;
	}

	if ( ch == ch->listening->listeners )
	{
		ch->listening->listeners = ch->next_listener;
	}
	else
	{
		CHAR_DATA *prev;

		for (prev = ch->listening->listeners; prev; prev = prev->next_listener)
		{
			if ( prev->next_in_room == ch )
			{
				prev->next_listener = ch->next_listener;
				break;
			}
		}

		if ( prev == NULL )
			bug( "Char_from_listeners: ch not found.", 0 );
	}

	ch->listening     = NULL;
	ch->next_listener = NULL;
	return;
}



/*
 * Move a char into a room's listeners.
 */
void char_to_listeners( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex )
{
	if ( pRoomIndex == NULL )
	{
		ROOM_INDEX_DATA *room;

		bug( "Char_to_listeners: NULL.", 0 );

		if ((room = get_room_index(ROOM_VNUM_START)) != NULL)
			char_to_listeners(ch,room);

		return;
	}

	ch->listening		= pRoomIndex;
	ch->next_listener		= pRoomIndex->listeners;
	pRoomIndex->listeners	= ch;

	return;
}


/*
 * Move a char out of a room.
 */
void char_from_room( CHAR_DATA *ch )
{
	OBJ_DATA *obj;

	if ( ch->in_room == NULL )
	{
		bug( "Char_from_room: NULL.", 0 );
		return;
	}

	if ( !IS_NPC(ch) )
		--ch->in_room->area->nplayer;

	if ( ( obj = get_eq_char( ch, WEAR_HOLD ) ) != NULL
			&&   obj->item_type == ITEM_LIGHT
			&&   obj->value[2] != 0
			&&   ch->in_room->light > 0 )
		--ch->in_room->light;

	UNLINK_SINGLE(ch, next_in_room, CHAR_DATA, ch->in_room->people);

	ch->in_room      = NULL;
	ch->next_in_room = NULL;
	ch->on 	     = NULL;  /* sanity check! */
	return;
}



/*
 * Move a char into a room.
 */
void char_to_room( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex )
{
    OBJ_DATA *obj;

    if ( pRoomIndex == NULL )
    {
	ROOM_INDEX_DATA *room;

	bug( "Char_to_room: NULL.", 0 );

	if ((room = get_room_index(ROOM_VNUM_START)) != NULL)
	    char_to_room(ch,room);

	return;
    }

    ch->in_room = pRoomIndex;
    LINK_SINGLE(ch, next_in_room, pRoomIndex->people);

    if ( !IS_NPC(ch) )
    {
	if (ch->in_room->area->empty)
	{
	    ch->in_room->area->empty = FALSE;
	    ch->in_room->area->age = 0;
	}
	++ch->in_room->area->nplayer;
    }

    if ( ( obj = get_eq_char( ch, WEAR_HOLD ) ) != NULL
    &&   obj->item_type == ITEM_LIGHT
    &&   obj->value[2] != 0 )
	++ch->in_room->light;

    if(!room_is_dim(ch->in_room) && is_affected(ch, gsn_shadow_play))
    {
	affect_strip( ch, gsn_shadow_play );
    }

    if (IS_AFFECTED(ch,AFF_PLAGUE))
    {
        AFFECT_DATA *af, plague;
        CHAR_DATA *vch;
 
        for ( af = ch->affected; af != NULL; af = af->next )
        {
            if (af->type == gsn_plague)
                break;
        }
 
        if (af == NULL)
        {
            REMOVE_BIT(ch->affected_by,AFF_PLAGUE);
            return;
        }
 
        if (af->level == 1)
            return;
 
	plague.where		= TO_AFFECTS;
        plague.type 		= gsn_plague;
        plague.level 		= af->level - 1;
        plague.duration 	= number_range(1,2 * plague.level);
        plague.location		= APPLY_STR;
        plague.modifier 	= -5;
        plague.bitvector 	= AFF_PLAGUE;

        for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
        {
	    if (!IS_ADMIN(vch) &&
            	!IS_AFFECTED(vch,AFF_PLAGUE) && number_bits(6) == 0)
            {
            	send_to_char("You feel hot and feverish.\n\r",vch);
            	act("$n shivers and looks very ill.",vch,NULL,NULL,TO_ROOM, 0);
            	affect_join(vch,&plague);
            }
        }
    }


    return;
}



/*
 * Give an obj to a char.
 */
void obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch )
{
    obj->next_content	 = ch->carrying;
    ch->carrying	 = obj;
    obj->carried_by	 = ch;
    obj->in_room	 = NULL;
    obj->in_obj		 = NULL;
    ch->carry_number	+= get_obj_number( obj );
    ch->carry_weight	+= get_obj_weight( obj );

    if(IS_SET(ch->act, ACT_UMBRA))
	obj_to_plane(obj, 1);
    else if(IS_SET(ch->act, ACT_DREAMING))
	obj_to_plane(obj, 2);
    else
	obj_to_plane(obj, 0);

    if(IS_SET(ch->form, FORM_MIST) || IS_SET(ch->form, FORM_BLOOD)
	|| IS_SET(ch->form, FORM_SHADOW) || IS_SET(ch->form, FORM_ASH))
    {
	act("$o passes right through you.", ch, obj, NULL, TO_CHAR, 1);
	act("$o passes right through $n.", ch, obj, NULL, TO_ROOM, 0);
	do_drop(ch, obj->name);
	return;
    }

    /* Quest test inserted here to make sure that object transference
     * is successfully captured in all cases.
     */
    if(ch->quest != NULL)
    {
	if(ch->quest->quest_type == 3)
	{
	    if(ch->quest->questor == ch)
	    {
		(*quest_table[ch->quest->quest_type].q_fun) (ch, 2);
	    }
	}
	else if(ch->quest->quest_type == 5)
	{
	    if(ch->quest->aggressor == ch)
	    {
		(*quest_table[ch->quest->quest_type].q_fun) (ch, 3);
	    }
	    else
	    {
		send_to_char("Drop that!\n\r", ch);
		do_drop(ch, obj->name);
	    }
	}
    }
}



/*
 * Take an obj from its character.
 */
void obj_from_char( OBJ_DATA *obj )
{
    CHAR_DATA *ch;

    if ( ( ch = obj->carried_by ) == NULL )
    {
	bug( "Obj_from_char: null ch.", 0 );
	return;
    }

    if ( obj->wear_loc != WEAR_NONE )
	unequip_char( ch, obj );

    if ( ch->carrying == obj )
    {
	ch->carrying = obj->next_content;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = ch->carrying; prev != NULL; prev = prev->next_content )
	{
	    if ( prev->next_content == obj )
	    {
		prev->next_content = obj->next_content;
		break;
	    }
	}

	if ( prev == NULL )
	    bug( "Obj_from_char: obj not in list.", 0 );
    }

    if(!IS_SET(obj->extra2, OBJ_ATTUNED) && !IS_BUILDERMODE(obj))
    {
    	PURGE_DATA(obj->owner);
	if(IS_NPC(ch))
	    obj->owner	 = str_dup(ch->short_descr);
	else
	    obj->owner	 = str_dup(ch->name);
    }
    obj->carried_by	 = NULL;
    obj->next_content	 = NULL;
    ch->carry_number	-= get_obj_number( obj );
    ch->carry_weight	-= get_obj_weight( obj );

    if(IS_SET(ch->act, ACT_UMBRA))
	obj_to_plane(obj, 1);
    else if(IS_SET(ch->act, ACT_DREAMING))
	obj_to_plane(obj, 2);
    else
	obj_to_plane(obj, 0);

    return;
}


/*
 * Shunt an object (and its contents) to a different plane.
 */
void obj_to_plane(OBJ_DATA *obj, int plane)
{
	OBJ_DATA *pobj;

	switch(plane)
	{
	case 1:
		if(!IS_SET(obj->extra_flags, ITEM_UMBRAL))
			SET_BIT(obj->extra_flags, ITEM_UMBRAL);
		break;
	case 2:
		if(!IS_SET(obj->extra_flags, ITEM_DREAM))
			SET_BIT(obj->extra_flags, ITEM_DREAM);
		break;
	default:
		if(IS_SET(obj->extra_flags, ITEM_UMBRAL))
			REMOVE_BIT(obj->extra_flags, ITEM_UMBRAL);
		if(IS_SET(obj->extra_flags, ITEM_DREAM))
			REMOVE_BIT(obj->extra_flags, ITEM_DREAM);
		break;
	}

	if(obj->contains != NULL)
		for(pobj=obj->contains;pobj!=NULL;pobj=pobj->next_content)
			obj_to_plane(pobj, plane);
}


/*
 * Find the ac value of an obj, including position effect.
 */
int apply_ac( OBJ_DATA *obj, int iWear, int type )
{
	if ( obj->item_type != ITEM_ARMOR )
		return 0;

	switch ( iWear )
	{
	case WEAR_BODY:	return 3 * obj->value[type];
	case WEAR_HEAD:	return 2 * obj->value[type];
	case WEAR_LEGS:	return 2 * obj->value[type];
	case WEAR_FEET:	return     obj->value[type];
	case WEAR_HANDS:	return     obj->value[type];
	case WEAR_NECK:	return     obj->value[type];
	case WEAR_ABOUT:	return 2 * obj->value[type];
	case WEAR_WAIST:	return     obj->value[type];
	case WEAR_WRIST_L:	return     obj->value[type];
	case WEAR_WRIST_R:	return     obj->value[type];
	case WEAR_HOLD:	return     obj->value[type];
	}

	return 0;
}



/*
 * Find a piece of eq on a character.
 */
OBJ_DATA *get_eq_char( CHAR_DATA *ch, int iWear )
{
	OBJ_DATA *obj;

	if (ch == NULL)
		return NULL;

	for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
	{
		if ( obj->wear_loc == iWear )
			return obj;
	}

	return NULL;
}



/*
 * Equip a char with an obj.
 */
void equip_char( CHAR_DATA *ch, OBJ_DATA *obj, int iWear )
{
    AFFECT_DATA *paf;
    int i = 0;

    if ( get_eq_char( ch, iWear ) != NULL && iWear != WEAR_SKINFLAP )
    {
	bug( "Equip_char: already equipped (%d).", iWear );
	return;
    }

    for (i = 0; i < 4; i++)
    	ch->armor[i]      	-= apply_ac( obj, iWear,i );
    obj->wear_loc	 = iWear;

    if (!obj->enchanted)
	for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
	    if ( paf->location != APPLY_SPELL_AFFECT )
	        affect_modify( ch, paf, TRUE );
    for ( paf = obj->affected; paf != NULL; paf = paf->next )
	if ( paf->location == APPLY_SPELL_AFFECT )
    	    affect_to_char ( ch, paf );
	else
	    affect_modify( ch, paf, TRUE );

    if ( obj->item_type == ITEM_LIGHT
    &&   iWear != WEAR_SKINFLAP
    &&   obj->value[2] != 0
    &&   ch->in_room != NULL )
	++ch->in_room->light;

    return;
}



/*
 * Unequip a char with an obj.
 */
void unequip_char( CHAR_DATA *ch, OBJ_DATA *obj )
{
    AFFECT_DATA *paf = NULL;
    AFFECT_DATA *lpaf = NULL;
    AFFECT_DATA *lpaf_next = NULL;
    int i = 0, iWear = 0;

    if ( obj->wear_loc == WEAR_NONE )
    {
	bug( "Unequip_char: already unequipped.", 0 );
	return;
    }

    for (i = 0; i < 4; i++)
    	ch->armor[i]	+= apply_ac( obj, obj->wear_loc,i );
    iWear		 = obj->wear_loc;
    obj->wear_loc	 = -1;

    if (!obj->enchanted)
    {
	for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
	    if ( paf->location == APPLY_SPELL_AFFECT )
	    {
	        for ( lpaf = ch->affected; lpaf != NULL; lpaf = lpaf_next )
	        {
		    lpaf_next = lpaf->next;
		    if ((lpaf->type == paf->type) &&
		        (lpaf->level == paf->level) &&
		        (lpaf->location == APPLY_SPELL_AFFECT))
		    {
		        affect_remove( ch, lpaf );
			lpaf_next = NULL;
		    }
	        }
	    }
	    else
	    {
	        affect_modify( ch, paf, FALSE );
		affect_check(ch,paf->where,paf->bitvector);
	    }
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
	if ( paf->location == APPLY_SPELL_AFFECT )
	{
	    bug ( "Norm-Apply: %d", 0 );
	    for ( lpaf = ch->affected; lpaf != NULL; lpaf = lpaf_next )
	    {
		lpaf_next = lpaf->next;
		if ((lpaf->type == paf->type) &&
		    (lpaf->level == paf->level) &&
		    (lpaf->location == APPLY_SPELL_AFFECT))
		{
		    bug ( "location = %d", lpaf->location );
		    bug ( "type = %d", lpaf->type );
		    affect_remove( ch, lpaf );
		    lpaf_next = NULL;
		}
	    }
	}
	else
	{
	    affect_modify( ch, paf, FALSE );
	    affect_check(ch,paf->where,paf->bitvector);	
	}

    if ( obj->item_type == ITEM_LIGHT
    &&   obj->value[2] != 0
    &&   ch->in_room != NULL
    &&   iWear != WEAR_SKINFLAP
    &&   ch->in_room->light > 0 )
	--ch->in_room->light;

    return;
}



/*
 * Count occurrences of an obj in a list.
 */
int count_obj_list( OBJ_INDEX_DATA *pObjIndex, OBJ_DATA *list )
{
	OBJ_DATA *obj;
	int nMatch = 0;

	nMatch = 0;
	for ( obj = list; obj != NULL; obj = obj->next_content )
	{
		if ( obj->pIndexData == pObjIndex )
			nMatch++;
	}

	return nMatch;
}



/*
 * Move an obj out of a room.
 */
void obj_from_room( OBJ_DATA *obj )
{
    ROOM_INDEX_DATA *in_room;
    CHAR_DATA *ch;

    if ( ( in_room = obj->in_room ) == NULL )
    {
	bug( "obj_from_room: NULL.", 0 );
	return;
    }

    for (ch = in_room->people; ch != NULL; ch = ch->next_in_room)
	if (ch->on == obj)
	    ch->on = NULL;

    if ( obj == in_room->contents )
    {
	in_room->contents = obj->next_content;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = in_room->contents; prev; prev = prev->next_content )
	{
	    if ( prev->next_content == obj )
	    {
		prev->next_content = obj->next_content;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Obj_from_room: obj not found.", 0 );
	    return;
	}
    }

    obj->in_room      = NULL;
    obj->next_content = NULL;
    return;
}



/*
 * Move an obj into a room.
 */
void obj_to_room( OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex )
{
    obj->next_content		= pRoomIndex->contents;
    pRoomIndex->contents	= obj;
    obj->in_room		= pRoomIndex;
    obj->carried_by		= NULL;
    obj->in_obj			= NULL;
    return;
}



/*
 * Move an object into an object.
 */
void obj_to_obj( OBJ_DATA *obj, OBJ_DATA *obj_to )
{
    obj->next_content		= obj_to->contains;
    obj_to->contains		= obj;
    obj->in_obj			= obj_to;
    obj->in_room		= NULL;
    obj->carried_by		= NULL;

    if(IS_SET(obj->extra_flags, ITEM_UMBRAL))
	obj_to_plane(obj, 1);
    else if(IS_SET(obj->extra_flags, ITEM_DREAM))
	obj_to_plane(obj, 2);
    else
	obj_to_plane(obj, 0);

    for ( ; obj_to != NULL; obj_to = obj_to->in_obj )
    {
	if ( obj_to->carried_by != NULL )
	{
	    obj_to->carried_by->carry_number += get_obj_number( obj );
	    obj_to->carried_by->carry_weight += get_obj_weight( obj )
		* WEIGHT_MULT(obj_to) / 100;
	}
    }

    return;
}



/*
 * Move an object out of an object.
 */
void obj_from_obj( OBJ_DATA *obj )
{
    OBJ_DATA *obj_from;

    if ( ( obj_from = obj->in_obj ) == NULL )
    {
	bug( "Obj_from_obj: null obj_from.", 0 );
	return;
    }

    if ( obj == obj_from->contains )
    {
	obj_from->contains = obj->next_content;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = obj_from->contains; prev; prev = prev->next_content )
	{
	    if ( prev->next_content == obj )
	    {
		prev->next_content = obj->next_content;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Obj_from_obj: obj not found.", 0 );
	    return;
	}
    }

    obj->next_content = NULL;
    obj->in_obj       = NULL;

    if(IS_SET(obj->extra_flags, ITEM_UMBRAL))
	obj_to_plane(obj, 1);
    else if(IS_SET(obj->extra_flags, ITEM_DREAM))
	obj_to_plane(obj, 2);
    else
	obj_to_plane(obj, 0);

    for ( ; obj_from != NULL; obj_from = obj_from->in_obj )
    {
	if ( obj_from->carried_by != NULL )
	{
	    obj_from->carried_by->carry_number -= get_obj_number( obj );
	    obj_from->carried_by->carry_weight -= get_obj_weight( obj ) 
		* WEIGHT_MULT(obj_from) / 100;
	}
    }

    return;
}

void return_key(OBJ_DATA *key)
{
	CHAR_DATA *pch;

	for(pch = char_list;pch;pch = pch->next)
	{
		if(pch->pIndexData->vnum == key->value[1])
		{
			obj_to_char(key, pch);
			break;
		}
	}

	if(!pch)
		free_obj(key);
}

/*
 * Extract an obj from the world.
 */
void extract_obj( OBJ_DATA *obj )
{
	OBJ_DATA *obj_content;
	OBJ_DATA *obj_next;

	if ( obj->in_room != NULL )
		obj_from_room( obj );
	else if ( obj->carried_by != NULL )
		obj_from_char( obj );
	else if ( obj->in_obj != NULL )
		obj_from_obj( obj );

	for ( obj_content = obj->contains; obj_content; obj_content = obj_next )
	{
		obj_next = obj_content->next_content;
		extract_obj( obj_content );
	}

	UNLINK_SINGLE(obj, next, OBJ_DATA, object_list);

	if(obj->pIndexData != NULL) --obj->pIndexData->count;
	if(obj->item_type == ITEM_KEY && (obj->value[0] || obj->value[1]))
		return_key(obj);
	else
	    LINK_SINGLE(obj, next, worldExtractedObjects);
	return;
}



/*
 * Extract a char from the world.
 */
void extract_char( CHAR_DATA *ch, bool fPull )
{
	CHAR_DATA *wch;
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;

	if ( ch->in_room == NULL )
	{
		bug( "Extract_char: NULL room.", 0 );
		return;
	}

	// if (ch->master == NULL )
	// {
	// 	bug( "Extract_Char: NULL Master.", 0);
	// 	return;
	// }

	if(ch->pet != NULL)
	{
		ch->pet->master = NULL;
		ch->pet = NULL;  /* just in case */
	}

	if ( fPull )
		die_follower( ch );

	stop_fighting( ch, TRUE );

	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
		obj_next = obj->next_content;
		extract_obj( obj );
	}

	if (ch->listening != NULL)
	{
		bug("It's this one.", 0);
		char_from_listeners( ch );
	}

	if (ch->in_room != NULL)
		char_from_room( ch );

	if ( IS_NPC(ch) )
		--ch->pIndexData->count;

	if ( ch->desc != NULL && ch->desc->original != NULL )
	{
		do_function(ch, &do_return, "" );
		ch->desc = NULL;
	}

	CHAR_DATA *wch_next;
	for ( wch = char_list; wch != NULL; wch = wch_next )
	{
		wch_next = wch->next;
		if ( wch->reply == ch )
			wch->reply = NULL;
		if ( ch->mprog_target == wch )
			wch->mprog_target = NULL;
	}

	for ( wch = char_list; wch != NULL; wch = wch->next )
	{
		if ( wch->summonner == ch )
			wch->summonner = NULL;
	}


	UNLINK_SINGLE(ch, next, CHAR_DATA, char_list);

	if ( ch->desc != NULL )
		ch->desc->character = NULL;
    LINK_SINGLE(ch, next, worldExtractedCharacters);
	return;
}



/*
 * Find a char in visual range in a direction.
 */
CHAR_DATA *get_char_dir( CHAR_DATA *ch, int dir, char *argument )
{
    char arg[MAX_INPUT_LENGTH]={'\0'};
    CHAR_DATA *rch;
    int number = 0;
    int vision = 0;
    int count = 0, depth = 0;
    ROOM_INDEX_DATA *scan_room;
    EXIT_DATA *pExit;

    vision = get_curr_stat(ch, STAT_PER);
    if(IS_SET(ch->affected_by, AFF_HSENSES))
        vision = vision * 2;
    if(ch->race == race_lookup("vampire"))
        vision += ch->disc[DISC_AUSPEX];

    number = number_argument( argument, arg );

    scan_room = ch->in_room;
    for (depth = 1; depth <= vision; depth++)
    {
        if ((pExit = scan_room->exit[dir]) != NULL)
        {
            scan_room = scan_room->exit[dir]->u1.to_room;
        }
        if ( !str_cmp( arg, "self" ) )
            return ch;
        for ( rch = scan_room->people; rch != NULL; rch = rch->next_in_room )
        {
            if ( !can_see( ch, rch ) || (!is_name( arg, rch->name )
	    && !is_name(arg, rch->alt_name)) )
                continue;
            if ( ++count == number )
                return rch;
        }
    }

    return NULL;
}


/*
 * Find a char in a remote room.
 */
CHAR_DATA *get_char_oroom( CHAR_DATA *ch, ROOM_INDEX_DATA *room, char *argument )
{
	char arg[MAX_INPUT_LENGTH]={'\0'};
	CHAR_DATA *rch;
	int number = 0;
	int count = 0;

	number = number_argument( argument, arg );
	if ( !str_cmp( arg, "self" ) )
		return ch;
	for ( rch = room->people; rch != NULL; rch = rch->next_in_room )
	{
		if ( !can_see( ch, rch ) || (!is_name( arg, rch->name ) && !is_name(arg, rch->alt_name)) )
			continue;
		if ( ++count == number )
			return rch;
	}

	return NULL;
}


/*
 * Find a char in the room.
 */
CHAR_DATA *get_char_room( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH]={'\0'};
	CHAR_DATA *rch;
	int number = 0;
	int count = 0;

	number = number_argument( argument, arg );
	if ( !str_cmp( arg, "self" ) || !str_prefix( arg, "ch->name" ) )
		return ch;

	if (ch->in_room != NULL && !IS_SET(ch->act2, ACT2_ASTRAL))
	{
		for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
		{
			if ( !can_see( ch, rch ) || (!is_name( arg, rch->name ) && !is_name(arg, rch->alt_name)) )
				continue;
			if ( ++count == number )
				return rch;
		}
	}

	if (ch->listening != NULL && IS_SET(ch->act2, ACT2_ASTRAL))
	{
		for ( rch = ch->listening->people; rch != NULL; rch = rch->next_in_room )
		{
			if ( !can_see( ch, rch ) || (!is_name( arg, rch->name ) && !is_name(arg, rch->alt_name)) )
				continue;
			if ( ++count == number )
				return rch;
		}
	}

	return NULL;
}


/*
 * Find a char in the world.
 */
CHAR_DATA *get_char_world( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH]={'\0'};
	CHAR_DATA *wch;
	int number = 0;
	int count = 0;

	if(argument == NULL)
	{
		return NULL;
	}

	if ( ( wch = get_char_room( ch, argument ) ) != NULL )
		return wch;

	number = number_argument( argument, arg );
	for ( wch = char_list; wch != NULL ; wch = wch->next )
	{
		if ( wch->in_room == NULL || !can_see( ch, wch ) ||   !is_name( arg, wch->name ))
			continue;
		if ( ++count == number )
			return wch;
	}

	return NULL;
}



/*
 * Find some object with a given index data.
 * Used by area-reset 'P' command.
 */
OBJ_DATA *get_obj_type( OBJ_INDEX_DATA *pObjIndex )
{
    OBJ_DATA *obj;

    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if ( obj->pIndexData == pObjIndex )
	    return obj;
    }

    return NULL;
}


/*
 * Find an obj in a list.
 */
OBJ_DATA *get_obj_list( CHAR_DATA *ch, char *argument, OBJ_DATA *list )
{
	char arg[MAX_INPUT_LENGTH]={'\0'};
	OBJ_DATA *obj;
	int number = 0;
	int count = 0;

	number = number_argument( argument, arg );
	for ( obj = list; obj != NULL; obj = obj->next_content )
	{
		if ( can_see_obj_main( ch, obj ) && (is_name( arg, obj->name )
						|| (!str_prefix(arg, "package") && IS_SET(obj->extra2, OBJ_PACKAGED))))
		{
			if ( ++count == number )
				return obj;
		}
	}

	return NULL;
}


/*
 * Find an obj in player's inventory.
 */
OBJ_DATA *get_obj_carry( CHAR_DATA *ch, char *argument, CHAR_DATA *viewer )
{
	char arg[MAX_INPUT_LENGTH]={'\0'};
	OBJ_DATA *obj;
	int number = 0;
	int count = 0;

	number = number_argument( argument, arg );
    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
	{
		if ( obj->wear_loc == WEAR_NONE && (can_see_obj_main( viewer, obj ) )
				&& (is_name( arg, obj->name ) || (!str_prefix(arg, "package") && IS_SET(obj->extra2, OBJ_PACKAGED))))
		{
			if ( ++count == number )
				return obj;
		}
	}

	return NULL;
}



/*
 * Find an obj in player's equipment.
 */
OBJ_DATA *get_obj_wear( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH]={'\0'};
	OBJ_DATA *obj;
	int number = 0;
	int count = 0;

	number = number_argument( argument, arg );
	for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
	{
		if ( obj->wear_loc != WEAR_NONE &&   can_see_obj_main( ch, obj )
		&& (is_name( arg, obj->name ) || (!str_prefix(arg, "package") && IS_SET(obj->extra2, OBJ_PACKAGED))))
		{
			if ( ++count == number )
				return obj;
		}
	}

	return NULL;
}


/*
 * Find an obj in the room or in inventory.
 */
OBJ_DATA *get_obj_here( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;

    if ( !IS_SET(ch->act2, ACT2_ASTRAL)
	&& ( obj = get_obj_list(ch, argument, ch->in_room->contents) ) != NULL )
	return obj;

    if ( IS_SET(ch->act2, ACT2_ASTRAL)
       && ( obj = get_obj_list(ch, argument, ch->listening->contents) ) != NULL)
	return obj;

    if ( ( ch->listening == NULL || ch->listening == ch->in_room )
	&& ( obj = get_obj_carry( ch, argument, ch ) ) != NULL )
	return obj;

    if ( ( ch->listening == NULL || ch->listening == ch->in_room )
	&& ( obj = get_obj_wear( ch, argument ) ) != NULL )
	return obj;

    return NULL;
}



/*
 * Find an obj in the world.
 */
OBJ_DATA *get_obj_world( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH]={'\0'};
	OBJ_DATA *obj;
	int number = 0;
	int count = 0;

	if ( ( obj = get_obj_here( ch, argument ) ) != NULL )
		return obj;

	number = number_argument( argument, arg );
	for ( obj = object_list; obj != NULL; obj = obj->next )
	{
		if ( can_see_obj( ch, obj )
			&& (is_name( arg, obj->name ) || (!str_prefix(arg, "package") && IS_SET(obj->extra2, OBJ_PACKAGED))))
		{
			if ( ++count == number )
				return obj;
		}
	}

	return NULL;
}

/* deduct cost from a character */

void deduct_cost(CHAR_DATA *ch, int cost)
{
	int dollars = 0;
	int cents = UMIN(ch->cents,cost);

	if (cents < cost)
	{
		dollars = ((cost - cents + 99) / 100);
		cents = cost - 100 * dollars;
	}

	ch->dollars -= dollars;
	ch->cents -= cents;

	if (ch->dollars < 0)
	{
		bug("deduct costs: dollars %d < 0",ch->dollars);
		ch->dollars = 0;
	}
	if (ch->cents < 0)
	{
		bug("deduct costs: cents %d < 0",ch->cents);
		ch->cents = 0;
	}
}   
/*
 * Create a 'money' obj.
 */
OBJ_DATA *create_money( int dollars, int cents )
{
	char buf[MSL]={'\0'};
	OBJ_DATA *obj;

	if ( dollars < 0 || cents < 0 || (dollars == 0 && cents == 0) )
	{
		bug( "Create_money: zero or negative money.",UMIN(dollars,cents));
		dollars = UMAX(1,dollars);
		cents = UMAX(1,cents);
	}

	if (dollars == 0 && cents == 1)
	{
		obj = create_object( get_obj_index( OBJ_VNUM_CENT_ONE ));
	}
	else if (dollars == 1 && cents == 0)
	{
		obj = create_object( get_obj_index( OBJ_VNUM_DOLLAR_ONE));
	}
	else if (cents == 0)
	{
		obj = create_object( get_obj_index( OBJ_VNUM_DOLLARS_SOME ));
		snprintf( buf, sizeof(buf), obj->short_descr, dollars );
		PURGE_DATA( obj->short_descr );
		obj->short_descr        = str_dup( buf );
		obj->value[1]           = dollars;
		obj->cost               = dollars;
		obj->weight		= dollars/5000;
	}
	else if (dollars == 0)
	{
		obj = create_object( get_obj_index( OBJ_VNUM_CENTS_SOME ));
		snprintf( buf, sizeof(buf), obj->short_descr, cents );
		PURGE_DATA( obj->short_descr );
		obj->short_descr        = str_dup( buf );
		obj->value[0]           = cents;
		obj->cost               = cents;
		obj->weight		= cents/200;
	}

	else
	{
		obj = create_object( get_obj_index( OBJ_VNUM_COINS ));
		snprintf( buf, sizeof(buf), obj->short_descr, cents, dollars );
		PURGE_DATA( obj->short_descr );
		obj->short_descr	= str_dup( buf );
		obj->value[0]		= cents;
		obj->value[1]		= dollars;
		obj->cost		= 100 * dollars + cents;
		obj->weight		= dollars / 50 + cents / 20;
	}

	return obj;
}



/*
 * Return # of objects which an object counts as.
 * Thanks to Tony Chamberlain for the correct recursive code here.
 */
int get_obj_number( OBJ_DATA *obj )
{
    int number = 0;
 
    if (obj->item_type == ITEM_CONTAINER || obj->item_type == ITEM_MONEY)
        number = 0;
    else
        number = 1;
 
    for ( obj = obj->contains; obj != NULL; obj = obj->next_content )
        number += get_obj_number( obj );
 
    return number;
}


/*
 * Return weight of an object, including weight of contents.
 */
int get_obj_weight( OBJ_DATA *obj )
{
    int weight = 0;
    OBJ_DATA *tobj;

    weight = obj->weight * material_table[material_lookup(obj->material)].weight;
    for ( tobj = obj->contains; tobj != NULL; tobj = tobj->next_content )
	weight += get_obj_weight( tobj ) * WEIGHT_MULT(obj) / 100;

    return weight;
}

int get_true_weight(OBJ_DATA *obj)
{
    int weight;
 
    weight = obj->weight * material_table[material_lookup(obj->material)].weight;
    for ( obj = obj->contains; obj != NULL; obj = obj->next_content )
        weight += get_obj_weight( obj );
 
    return weight;
}

/*
 * True if room is dark.
 */
bool room_is_dark( ROOM_INDEX_DATA *pRoomIndex )
{
    if ( pRoomIndex == NULL ) return FALSE;

    if ( pRoomIndex->light > 0 )
	return FALSE;

    if ( IS_SET(pRoomIndex->room_flags, ROOM_DARK) )
	return TRUE;

    if ( pRoomIndex->sector_type == SECT_INSIDE
    ||   pRoomIndex->sector_type == SECT_CITY )
	return FALSE;

    if ( weather_info.sunlight == SUN_SET
    ||   weather_info.sunlight == SUN_DARK )
	return TRUE;

    return FALSE;
}


/*
 * True if room has low light.
 */
bool room_is_dim( ROOM_INDEX_DATA *pRoomIndex )
{
	if ( pRoomIndex == NULL ) return FALSE;

	if ( IS_SET(pRoomIndex->room_flags, ROOM_DARK) )
		return TRUE;

	if ( pRoomIndex->light <= 5 )
		return TRUE;

	if ( pRoomIndex->sector_type == SECT_INSIDE
			||   pRoomIndex->sector_type == SECT_CITY )
		return FALSE;

	if ( weather_info.sunlight == SUN_SET
			||   weather_info.sunlight == SUN_DARK )
		return TRUE;

	return FALSE;
}


bool is_room_owner(CHAR_DATA *ch, ROOM_INDEX_DATA *room)
{
    if (room->owner == NULL || room->owner[0] == '\0')
	return FALSE;

    return is_name(ch->name,room->owner);
}

/*
 * True if room is private.
 */
bool room_is_private( ROOM_INDEX_DATA *pRoomIndex )
{
    CHAR_DATA *rch;
    int count = 0;

    for ( rch = pRoomIndex->people; rch != NULL; rch = rch->next_in_room )
	count++;

    if ( IS_SET(pRoomIndex->room_flags, ROOM_PRIVATE)  && count >= 2 )
        return TRUE;

    if ( IS_SET(pRoomIndex->room_flags, ROOM_SOLITARY) && count >= 1 )
        return TRUE;

    return FALSE;
}

/* visibility on a room -- for entering and exits */
bool can_see_room( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex )
{
    if (IS_SET(pRoomIndex->room_flags, ROOM_IMP_ONLY) 
    &&  get_trust(ch) < MAX_LEVEL)
	return FALSE;

    if (IS_SET(pRoomIndex->room_flags, ROOM_ADMIN)
    &&  !IS_ADMIN(ch))
	return FALSE;

    if (!IS_ADMIN(ch) && pRoomIndex->clan && ch->clan != pRoomIndex->clan)
	return FALSE;

    return TRUE;
}


/*
 * True if char can see victim.
 */
bool can_see_main( CHAR_DATA *ch, CHAR_DATA *victim )
{
/* RT changed so that WIZ_INVIS has levels */
    if ( ch == victim )
	return TRUE;
    
    if ( get_trust(ch) < victim->invis_level)
	return FALSE;

    if (get_trust(ch) < victim->incog_level && ch->in_room != victim->in_room)
	return FALSE;

    if ( (!IS_NPC(ch) && IS_SET(ch->plr_flags, PLR_HOLYLIGHT)) 
    ||   (IS_NPC(ch) && IS_ADMIN(ch)))
	return TRUE;

    if ( IS_AFFECTED(ch, AFF_BLIND) )
	return FALSE;

    if ( room_is_dark( ch->in_room ) && !IS_AFFECTED(ch, AFF_INFRARED)
	&& !IS_AFFECTED(ch, AFF_DARK_VISION) )
	return FALSE;

    if ( room_is_dark( ch->in_room ) && IS_AFFECTED(ch, AFF_INFRARED)
	&& IS_SET(ch->form, FORM_COLD_BLOOD) )
	return FALSE;

    if ( IS_AFFECTED(victim, AFF_INVISIBLE)
	&& (!IS_AFFECTED(ch, AFF_DETECT_INVIS)
     || (!IS_NPC(ch) && ch->race == race_lookup("vampire")
	&& ch->disc[DISC_AUSPEX] < affect_level(victim, AFF_INVISIBLE))
     || (!IS_NPC(ch) && ch->race == race_lookup("faerie")
	&& (ch->disc[DISC_ACTOR] < 1 || ch->disc[DISC_ART])
	&& ch->ability[KENNING].value < affect_level(victim, AFF_INVISIBLE))
     || (!IS_NPC(ch) && ch->race == race_lookup("werewolf")
	&& affect_level(ch, AFF_TRUE_SCENT)
	    < affect_level(victim, AFF_INVISIBLE))) )
		return FALSE;

    if ( IS_SET(victim->act, ACT_CHIMERAE)
	&& (!IS_AFFECTED(ch, AFF_ENCHANTED)
	    || race_lookup("faerie") != ch->race))
	return FALSE;

    /* sneaking */
    if ( IS_AFFECTED(victim, AFF_SNEAK)
    &&   !IS_AFFECTED(ch,AFF_DETECT_HIDDEN)
    &&   victim->fighting == NULL)
    {
	int fail = 0;
	fail = dice_rolls(ch,
	   (get_curr_stat(ch, STAT_DEX) + ch->ability[STEALTH].value),
	   (get_curr_stat(victim,STAT_PER) + victim->ability[ALERTNESS].value));

	if (fail > 0)
	    return FALSE;
	if (fail <= 0)
	    return TRUE;
    }

    if ( IS_AFFECTED(victim, AFF_HIDE)
    &&   victim->fighting == NULL
    &&   (!IS_AFFECTED(ch, AFF_DETECT_HIDDEN)
    ||   (ch->race != race_lookup("vampire") && ch->disc[DISC_AUSPEX] < 1) ))
	    if((!IS_NATURAL(victim)
	    && !IS_NATURAL(ch))
	    && (ch->disc[DISC_AUSPEX] < victim->disc[DISC_OBFUSCATE]))
		return FALSE;

    return TRUE;
}



/*
 * True if char can see victim and victim is on same plane.
 */
bool can_see( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if(!can_see_main(ch, victim)) return FALSE;

    if (!SAME_PLANE(victim, ch) && !IS_ADMIN(ch))
	return FALSE;

    if (IS_SET(ch->form, FORM_ASH)
    && dice_rolls(ch, get_curr_stat(ch, STAT_PER)
		+ ch->ability[ALERTNESS].value, 9) < 1)
	return FALSE;

    return TRUE;
}


/*
 * True if char can see obj.
 */
bool can_see_obj_main( CHAR_DATA *ch, OBJ_DATA *obj )
{
	CHAR_DATA *owner;

	if ( !IS_NPC(ch) && IS_SET(ch->plr_flags, PLR_HOLYLIGHT) )
		return TRUE;

	if ( IS_SET(obj->extra_flags,ITEM_VIS_DEATH))
		return FALSE;

	if ( IS_AFFECTED( ch, AFF_BLIND ) && obj->item_type != ITEM_POTION)
		return FALSE;

	if ( !IS_SET( ch->act, ACT_DREAMING ) && IS_SET(obj->extra_flags, ITEM_DREAM) && !IS_ADMIN(ch))
		return FALSE;

	if ( !IS_SET( ch->act, ACT_UMBRA ) && IS_SET(obj->extra_flags, ITEM_UMBRAL) && !IS_ADMIN(ch))
		return FALSE;

	if ((owner = get_char_world(ch, obj->owner)) != NULL)
	{
		if ( ch != owner && IS_SET(obj->extra_flags, ITEM_HIDDEN) && !IS_ADMIN(ch))
			return FALSE;
	}

	if ( IS_SET( ch->act, ACT_DREAMING ) && !IS_SET(obj->extra_flags, ITEM_DREAM) && !IS_ADMIN(ch))
		return FALSE;

	if ( IS_SET( ch->act, ACT_UMBRA ) && !IS_SET(obj->extra_flags, ITEM_UMBRAL) && !IS_ADMIN(ch))
		return FALSE;

	if ( obj->item_type == ITEM_LIGHT && obj->value[2] != 0 )
		return TRUE;

	if ( IS_SET(obj->extra_flags, ITEM_INVIS)
			&&   !IS_AFFECTED(ch, AFF_DETECT_INVIS) )
		return FALSE;

	if(obj->carried_by != ch)
	{
		if (obj == get_eq_char(obj->carried_by, WEAR_UNDERPANTS) && get_eq_char(obj->carried_by, WEAR_LEGS))
			return FALSE;

		if (obj == get_eq_char(obj->carried_by, WEAR_UNDERTOP) && get_eq_char(obj->carried_by, WEAR_BODY))
			return FALSE;
	}

	if ( IS_OBJ_STAT(obj,ITEM_GLOW))
		return TRUE;

	if ( room_is_dark( ch->in_room ) && !IS_AFFECTED(ch, AFF_DARK_VISION) )
		return FALSE;

	return TRUE;
}


/*
 * True if char can see obj and object is in same plane.
 */
bool can_see_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
	if ( !can_see_obj_main(ch, obj) )
		return FALSE;

	if ( IS_SET(obj->extra_flags, ITEM_UMBRAL)
			&&	 !IS_SET(ch->act, ACT_UMBRA) )
		return FALSE;

	if ( IS_SET(obj->extra_flags, ITEM_DREAM)
			&&	 !IS_SET(ch->act, ACT_DREAMING) )
		return FALSE;

	return TRUE;
}



/*
 * True if char can drop obj.
 */
bool can_drop_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
	if ( !IS_SET(obj->extra_flags, ITEM_NODROP) )
		return TRUE;

	if ( !IS_NPC(ch) && ch->trust >= LEVEL_IMMORTAL )
		return TRUE;

	return FALSE;
}


CHAR_DATA * get_char_area( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH]={'\0'};
	CHAR_DATA *ach;
	int number = 0, count = 0;

	if ((ach = get_char_room( ch, argument )) != NULL)
		return ach;

	number = number_argument( argument, arg );
	for ( ach = char_list; ach != NULL; ach = ach->next )
	{
		if (ach->in_room->area != ch->in_room->area ||  !can_see( ch, ach ) || !is_name( arg, ach->name ))
			continue;
		if (++count == number)
			return ach;
	}
	return NULL;
}


bool IS_ATTRIB_AVAILABLE(int race, int sn)
{
	if(ability_table[sn].race == 0)
		return TRUE;
	if( race == race_lookup("human")
			&& IS_SET(ability_table[sn].race, HU) )
		return TRUE;
	if( race == race_lookup("werewolf")
			&& IS_SET(ability_table[sn].race, WW) )
		return TRUE;
	if( race == race_lookup("faerie")
			&& IS_SET(ability_table[sn].race, FA) )
		return TRUE;
	if( race == race_lookup("vampire")
			&& IS_SET(ability_table[sn].race, VA) )
		return TRUE;
	return FALSE;
}

bool IS_CLASS_AVAILABLE(int race, int sn)
{
	if(clan_table[sn].available == -1)
		return FALSE;

	if(clan_table[sn].available == 0)
		return TRUE;

	if( race == race_lookup("human") && IS_SET(clan_table[sn].available, HU) )
		return TRUE;

	if( race == race_lookup("werewolf") && IS_SET(clan_table[sn].available, WW) )
		return TRUE;

	if( race == race_lookup("faerie") && IS_SET(clan_table[sn].available, FA) )
		return TRUE;

	if( race == race_lookup("vampire") && IS_SET(clan_table[sn].available, VA) )
		return TRUE;

	return FALSE;
}

bool IS_ALONE(CHAR_DATA *ch)
{
	if(ch->next_in_room == NULL && ch->in_room->people == ch)
		return TRUE;
	return FALSE;
}

bool IS_GROUP_ALONE(CHAR_DATA *ch)
{
	CHAR_DATA *wch;
	bool yes = TRUE;

	for(wch = ch->in_room->people; wch; wch = wch->next_in_room)
	{
		if(!is_same_group(ch, wch))
			yes = FALSE;
	}

	return yes;
}

bool IS_ANIMAL(CHAR_DATA *ch)
{
	if(ch->race >= 5 && ch->race <= 11)
		return TRUE;

	if(ch->race == 13 || ch->race == 20 || ch->race == 21 )
		return TRUE;

	if(ch->race >= 15 && ch->race <= 18)
		return TRUE;

	if(IS_SET(ch->form, FORM_ANIMAL))
		return TRUE;

	return FALSE;
}

bool SAME_PLANE(CHAR_DATA *ch, CHAR_DATA *vch)
{

	if(ch == NULL || vch == NULL) return FALSE;

	if( (IS_SET(ch->act, ACT_UMBRA) && IS_SET(vch->act, ACT_UMBRA))
			|| (IS_SET(ch->act, ACT_DREAMING) && IS_SET(vch->act, ACT_DREAMING)) )
		return TRUE;

	if( (!IS_SET(ch->act, ACT_UMBRA) && !IS_SET(vch->act, ACT_UMBRA))
			&& (!IS_SET(ch->act, ACT_DREAMING) && !IS_SET(vch->act, ACT_DREAMING)) )
		return TRUE;

	return FALSE;
}

bool can_comprehend(CHAR_DATA *ch, CHAR_DATA *vch)
{
	if(IS_ADMIN(ch))
		return TRUE;

	if(ch->shape == vch->shape)
		return TRUE;

	if(ch->race == race_lookup("werewolf") && vch->shape == SHAPE_WOLF)
		return TRUE;

	if(ch->race == race_lookup("vampire") && ch->disc[DISC_ANIMALISM])
		return TRUE;

	if(ch->race == race_lookup("werewolf") && ch->auspice == auspice_lookup("galliard") && ch->disc[DISC_AUSPICE] >= 1)
		return TRUE;

	if(vch->shape == SHAPE_HUMAN || vch->shape == SHAPE_NONE || vch->shape == SHAPE_CRINOS)
		return TRUE;

	return FALSE;
}

int affect_level (CHAR_DATA *ch, int sn)
{
	AFFECT_DATA *af;
	int i = 0;

	af = affect_find(ch->affected, sn);

	if(af)
		i = af->level;
	else
		i = 0;

	return i;

}

void add_warrant(CHAR_DATA *ch, int warrant, bool caught)
{
	if(caught)
	{
		if(number_percent() < 60) number_fuzzy(warrant);
	}
	else
	{
		number_fuzzy(warrant);
	}

	return;
}

bool part_in_use(CHAR_DATA *ch, int move)
{
	COMBAT_DATA *pmove;
	int count = 0;

	for(pmove=ch->combo;pmove!=NULL;pmove=pmove->next)
	{
		if(combat_move_table[move].part == pmove->part)
		{
			return TRUE;
		}
		count++;
	}
	return FALSE;
}

int combo_strike_part(int combo)
{
	int i = 0;
	for(i=0; combo_table[combo].moves[i] != -2; i++);
	return combat_move_table[combo_table[combo].moves[i-1]].part;
}

bool hit_blocked(at_part_type strike_part, long flags)
{
	/* @@@@@ */
	return FALSE;

	if(strike_part == AP_LEFT_HAND && IS_SET(flags, A))
		return TRUE;
	if(strike_part == AP_RIGHT_HAND && IS_SET(flags, B))
		return TRUE;
	if(strike_part == AP_FEET && IS_SET(flags, C))
		return TRUE;

	return FALSE;
}

/*
 * Artificial Intelligence module.
 */
MEMORY *new_memory	args( () );

MEMORY * memory_search(CHAR_DATA *ch, char *key)
{
	return NULL;
}


MEMORY * gen_memory(CHAR_DATA *ch, CHAR_DATA *stranger)
{
	MEMORY *mem;
	MEMORY *pm;

	mem = new_memory();
	mem->attitude = race_predisp_table[ch->race][stranger->race];
	if((pm = memory_search(ch, clan_table[stranger->clan].name)) != NULL)
	{
		mem->attitude += pm->attitude;
		mem->reaction = pm->attitude;
	}
	mem->when = current_time;
	PURGE_DATA(mem->name);
	mem->name = str_dup(stranger->name);

	return mem;
}


int response_level(CHAR_DATA *ch, CHAR_DATA *mobile)
{
int curr = 0;
MEMORY *mem;

    if((mem = memory_search(mobile, ch->name)) == NULL)
	mem = gen_memory(mobile, ch);

    curr = mem->attitude - mobile->condition[COND_PAIN] - mobile->condition[COND_FRENZY] - mobile->condition[COND_ANGER] + mobile->condition[COND_FEAR];
    curr = UMAX(curr, 1);
    curr = UMIN(curr, 10);
    curr--;

    return curr;
}


char * extract_name(char *string, char *name)
{

name = "";

return string;
}


void response(REACT *pReact)
{
}


void AI(CHAR_DATA *ch, char *string)
{
	REACT *r;
	CHAR_DATA *vch;
	char name[MSL]={'\0'};

	string = extract_name(string, name);

	vch = get_char_room(ch, name);

	for(r = ch->personality->matrix[(response_level(vch, ch))]; r;
			r = r->next_in_matrix_loc)
	{
		if(is_name(r->trig, string))
		{
			response(r);
		}
	}
}

/*
 * Test to see if a string received is a triggering value.
 */
void trigger_test( const char *string, CHAR_DATA *ch, CHAR_DATA *vict )
{
SCRIPT_DATA *ps;

    if(ch != NULL && ch->in_room != NULL) {
	if(ch->in_room->event != NULL)
	{
		if(!ch->in_room->event->stop)
		{
		for(ps = ch->in_room->event->script_list; ps; ps = ps->next_in_event)
		{
		    if(strstr((char *)string, ps->trigger) && ps->actor == ch->pIndexData->vnum && ch->pIndexData != NULL)
			{
				ch->script = new_script();
				ch->script->delay = ps->delay;
				PURGE_DATA(ch->script->reaction);
				ch->script->reaction = str_dup(ps->reaction);
				PURGE_DATA(ch->script->trigger);
				ch->script->trigger = str_dup(ps->trigger);
				PURGE_DATA(ch->script->author);
				ch->script->author = str_dup(ps->author);
				ch->script->active = TRUE;
				ch->script->event = ps->event;
				ch->script->next = NULL;
				ch->script->next_in_event = NULL;
			}
		}
		}
	}
	else if(IS_NPC(ch) && IS_SET(ch->act, ACT_INTELLIGENT))
	{
	    /* Here is where will be the AI wrappers */
	    AI(ch, (char *)string);
	}
	else if(ch->pIndexData && ch->pIndexData->triggers != NULL && ch->script == NULL)
	{
	    for(ps = ch->pIndexData->triggers; ps; ps = ps->next_in_event)
	    {
		if(strstr((char *)string, ps->trigger))
		{
		    ch->script = new_script();
		    ch->script->delay = ps->delay;
		    if(ch->script->delay <= 0)
		    {
			ch->script->delay = 1;
		    }
		    PURGE_DATA(ch->script->reaction);
		    if (!IS_NULLSTR(ps->reaction))
		    	ch->script->reaction = str_dup(ps->reaction);
		    else
		    	ch->script->reaction = str_dup("");
		    PURGE_DATA(ch->script->trigger);
		    ch->script->trigger = str_dup(ps->trigger);
		    ch->script->active = TRUE;
		    ch->script->next = NULL;
		    ch->script->next_in_event = NULL;
		}
	    }
	}
	else if(ch->pIndexData && HAS_TRIGGER( ch, TRIG_ACT ) && MOBtrigger )
		mp_act_trigger((char *)string, ch, vict, NULL, NULL, TRIG_ACT);
    }
}

void clear_character(CHAR_DATA *ch)
{
	int i = 0;
	AFFECT_DATA *af;
	QUEST_DATA *q;
	TRAIT_DATA *trait;

	PURGE_DATA(ch->pcdata->bamfin);
	PURGE_DATA(ch->pcdata->bamfout);
	PURGE_DATA(ch->pcdata->title);
	ch->pcdata->bamfin = NULL;
	ch->pcdata->bamfout = NULL;
	ch->pcdata->title = str_dup(" is new to the city.");

	ch->pcdata->last_note = current_time - 60*60*24*14;

	ch->summonner = NULL;
	if(ch->pet)
	{
		ch->pet->master = NULL;
		ch->pet = NULL;
	}
	while(ch->affected)
	{
		af = ch->affected;
		ch->affected = ch->affected->next;
		free_affect(af);
	}
	ch->pnote = NULL;
	ch->in_room = NULL;
	ch->was_in_room = NULL;
	if(ch->quest)
	{
		for(q = quest_list; q; q = q->next)
		{
			if(q->next == ch->quest)
				q->next = ch->quest->next;
		}
		free_quest(ch->quest);
		ch->quest = NULL;
	}
	while(ch->traits)
	{
		trait = ch->traits;
		ch->traits = ch->traits->next;
		free_trait(trait);
	}
    ch->on_phone = 0;
    ch->attitude = 0;
    ch->shape = 0;
    ch->balance = 1;
    ch->act = 0;
    ch->act2 = 0;
    ch->gen = 13;
    ch->imm_flags = 0;
    ch->res_flags = 0;
    ch->vuln_flags = 0;
    ch->affected_by = 0;
    ch->affected_by2 = 0;
    for(i = 0; i < MAX_DISC; i++)
    {
	ch->disc[i] = 0;
    }
    for(i = 0; i < MAX_SKILL; i++)
    {
	ch->learned[i] = 0;
    }
    ch->saving_throw = 0;
    ch->carry_weight = 0;
    ch->carry_number = 0;
    ch->combat_flag = 0;
    ch->torpor_timer = 0;
    ch->power_timer = 0;
    ch->blood_timer = 0;
    for(i=0;i<MAX_ABIL;i++)
    {
	ch->ability[i].value = 0;
    }
    for(i=0;i<10;i++)
    {
	ch->condition[i] = 0;
    }
    for(i=0;i<MAX_BG;i++)
    {
	ch->backgrounds[i] = 0;
    }
    for(i=0;i<MAX_INFL;i++)
    {
	ch->influences[i] = 0;
    }
    ch->daze = 0;
    ch->wait = 0;
    ch->timer = 0;
    PURGE_DATA(ch->prefix);
    PURGE_DATA(ch->alt_name);
    PURGE_DATA(ch->alt_long_descr);
    PURGE_DATA(ch->alt_description);
    PURGE_DATA(ch->switch_desc);
    PURGE_DATA(ch->description);
    PURGE_DATA(ch->long_descr);
    PURGE_DATA(ch->short_descr);
    ch->prefix = NULL;
    ch->alt_name = NULL;
    ch->alt_long_descr = NULL;
    ch->alt_description = NULL;
    ch->switch_desc = NULL;
    ch->description = NULL;
    ch->long_descr = NULL;
    ch->short_descr = NULL;
    ch->warrants = 0;
    ch->position = P_STAND;
    ch->health = MAX_HEALTH;
    ch->agghealth = MAX_HEALTH;
    clear_rite(ch);
}

void be_frenzied(CHAR_DATA *ch)
{
    if(IS_SET(ch->act2, ACT2_FRENZY))
    {
	if((ch->condition[COND_FRENZY] -= 5) < 20)
	{
	    REMOVE_BIT(ch->act2, ACT2_FRENZY);
	    return;
	}
    }
    else
    {
	SET_BIT(ch->act2, ACT2_FRENZY);
    }

    if(IS_SET(ch->act2, ACT2_RESIST))
    {
	send_to_char(
	    "You exert your will to prevent the frenzy from taking over.\n\r",
	        ch);
	if(IS_SET(ch->act2, ACT2_FRENZY)) REMOVE_BIT(ch->act2, ACT2_FRENZY);
	ch->condition[COND_FRENZY] -= 70;
	return;
    }

    if(IS_ALONE(ch) && dice_rolls(ch,ch->virtues[1], 8))
    {
	send_to_char("The red gradually clears, but the rage boils just under the surface.\n\r", ch);
    }
    else if(IS_ALONE(ch))
    {
	do_flee(ch, "");
	if(IS_ALONE(ch))
	    gain_condition(ch, COND_FRENZY, -20);
    }
    else
    {
	if(ch->fighting)
	{
	    if((ch->condition[COND_FEAR] > ch->condition[COND_FRENZY]
	    || ch->health + ch->agghealth - 7
		< ch->fighting->health + ch->fighting->agghealth - 7)
	    && !IS_SET(ch->act2, ACT2_RESIST))
	    {
		do_flee(ch, "");
		do_flee(ch, "");
		if(!IS_ALONE(ch)) do_flee(ch, "");
		gain_condition(ch, COND_FRENZY, -40);
	    }
	}
    }
}

CHAR_DATA * get_player_file( char *name )
{
	char buf[MSL]={'\0'};
	DESCRIPTOR_DATA d;
	CHAR_DATA *ch = NULL;

	d.character = NULL;

	if ( !load_char_obj( &d, name, FALSE, FALSE, FALSE ) )
	{
		free_char( d.character );
		return NULL;
	}

	ch = d.character;
	d.character = NULL;
	ch->desc = NULL;

	strncpy( buf, ch->name, sizeof(buf) );
	PURGE_DATA( ch->name );
	ch->name = str_dup( capitalize( buf ) );

	return ch;
}

CHAR_DATA *get_player( char *argument )
{
	CHAR_DATA *ch;

	for ( ch = char_list; ch; ch = ch->next )
	{
		if ( IS_NPC( ch ) )
			continue;
		if ( is_name( argument, ch->name ) )
			return ch;
	}

	return get_player_file( argument );
}

bool is_online(char *argument)
{
DESCRIPTOR_DATA *d;

    for( d = descriptor_list; d; d = d->next )
    {
	if(d->character)
	{
	    if(is_name(d->character->name, argument) && !IS_NPC(d->character))
	    {
		return TRUE;
	    }
	}
    }

    return FALSE;
}

bool pc_in_char_list(char *argument)
{
CHAR_DATA *ch;

    for( ch = char_list; ch; ch = ch->next )
    {
	if(is_name(ch->name, argument) && !IS_NPC(ch))
	    {
		return TRUE;
	    }
    }

    return FALSE;
}

char *health_string(CHAR_DATA *ch)
{
	int k = 0;
	char *tmp;

	k = ch->health + ch->agghealth - 7;
	if(k>7)
		k = 7;
	if(k>=0)
		tmp = health_table[k].display_name;
	/*
    tmp = str_dup(health_table[k].display_name);
	 */
	else if(ch->position == P_TORPOR)
		tmp = "torpored";

	else
		tmp = "at death's door";
	/*
    tmp = str_dup("at death's door");
	 */

	return tmp;
}

bool IS_FEARFUL(CHAR_DATA *ch)
{
	/* Act flags */
	if(IS_SET(ch->act, ACT_WIMPY)) return TRUE;
	if(IS_SET(ch->act, ACT_SENTINEL)) return FALSE;
	if(IS_SET(ch->act, ACT_AGGRESSIVE)) return FALSE;
	if(IS_SET(ch->act, ACT_INTELLIGENT)) return FALSE;
	if(IS_SET(ch->act, ACT_VAMPIRE)) return FALSE;
	if(IS_SET(ch->act, ACT_WEREWOLF)) return FALSE;
	if(IS_SET(ch->act, ACT_CHANGELING)) return FALSE;
	if(IS_SET(ch->act, ACT_IS_HEALER)) return FALSE;
	if(IS_SET(ch->act, ACT_IS_CHANGER)) return FALSE;
	if(IS_SET(ch->act, ACT_TWIN)) return FALSE;

	/* Act2 flags */
	if(IS_SET(ch->act2, ACT2_POLICE)) return FALSE;
	if(IS_SET(ch->act2, ACT2_TWILIGHT)) return FALSE;
	if(IS_SET(ch->act2, ACT2_FRENZY)) return FALSE;
	if(IS_SET(ch->act2, ACT2_RESIST)) return FALSE;
	if(IS_SET(ch->act2, ACT2_WYRM)) return FALSE;
	if(IS_SET(ch->act2, ACT2_WEAVER)) return FALSE;
	if(IS_SET(ch->act2, ACT2_WYLD)) return FALSE;
	if(IS_SET(ch->act2, ACT2_STORY)) return FALSE;
	if(IS_SET(ch->act2, ACT2_GHOUL)) return FALSE;

	return TRUE;
}

bool trade_stocks(CHAR_DATA *ch, STOCKS *st, int num, bool fees, bool buy)
{
	int cash = 0, cost = 0, tax = 0;
	bool found = FALSE;
	STOCKS *chst, *next = NULL, *prev = NULL;

	if(num < 1)
	{
		send_to_char("You can't buy or sell negative stocks.\n\r", ch);
		return FALSE;
	}

	for(chst = ch->stocks; chst != NULL; chst = chst->next)
	{
		if(chst->ticker == st->ticker)
		{
			found = TRUE;
			break;
		}
	}

	cash = (ch->dollars * 100) + ch->cents;
	cost = st->cost * num;
	tax = cost * 0.03;

	if(buy)
	{
		if(fees) cost += tax;

		if(cost > cash)
		{
			send_to_char("You don't have enough money.\n\r", ch);
			return FALSE;
		}

		if(!found)
		{
			/* Make stocks */
			chst = new_stock();
			chst->ticker = st->ticker;
			chst->cost = 0;
			chst->next = ch->stocks;
			ch->stocks = chst;
		}

		ch->dollars -= cost/100;
		if (cost%100 > ch->cents)
		{
			ch->dollars -= 1;
			ch->cents += 100 - (cost % 100);
		} else {
			ch->cents -= cost%100;
		}

		chst->cost += num;

		send_to_char(Format("You buy %d shares of %s for $%d.%.2d.\n\r", num, st->name, cost/100, cost%100), ch);
	}
	else
	{
		if(!found)
		{
			send_to_char("You don't own any of those shares.\n\r", ch);
			return FALSE;
		}

		if(chst->cost < num)
		{
			send_to_char("You don't have that many of those shares.\n\r", ch);
			return FALSE;
		}

		if(fees) cost -= tax;

		if(chst->cost == num)
		{
			/* Remove chstock entry */
			for(chst = ch->stocks; chst != NULL; chst = next)
			{
				next = chst->next;
				if(chst->ticker == st->ticker)
				{
					if(chst == ch->stocks)
					{
						ch->stocks = chst->next;
					}
					else
					{
						for(prev=ch->stocks;prev->next!=chst;prev=prev->next);
						prev->next = next;
					}
					free_stock(chst);
				}
			}
		}

		if(chst != NULL && chst->cost > num) chst->cost -= num;

		ch->dollars += cost/100;
		ch->cents   += cost%100;

		send_to_char(Format("You sell %d shares of %s for $%d.%.2d.\n\r", num, st->name, cost/100, cost%100), ch);
	}

	send_to_char(Format("You now have %d shares of %s.\n\r", chst?chst->cost:0, st->name), ch);

	return FALSE;
}

int factorial (int n)
{
	if (n == 0)
		return 1;
	else
		return (n * factorial (n-1));
}

int compound (int n)
{
	if (n == 0)
		return 0;
	else
		return (n + compound (n-1));
}

bool IS_NATURAL( CHAR_DATA *ch )
{
	if(ch->race == race_lookup("werewolf") || ch->race == race_lookup("vampire"))
		return FALSE;

	return TRUE;
}


/* 
** Function to count the number of words in a string 
** Returns an integer. 
*/ 
int count_words(char *string) 
{ 
    int i = 0;
    int looking_for_word = 1;
    int word_count = 0; 

    if(!string)
	return 0; 
 
    for (i = 0; string[i] != '\0'; ++i) 
        if (isalpha(string[i])) /* isalpha() requires <ctype.h> */ 
        { 
            if (looking_for_word) 
            { 
                ++word_count; 
                looking_for_word = 0; 
            } 
        } 
        else 
            looking_for_word = 1; 
    return word_count; 
}

bool bare_earth(ROOM_INDEX_DATA *rm)
{
	return TRUE;
}

int gen_room( CHAR_DATA *ch, ORG_DATA *org )
{
    AREA_DATA *pArea;
    ROOM_INDEX_DATA *pRoom;
    int iHash = 0;
    int value = 0;

    if(ch == NULL) return FALSE;

    pArea = ch->in_room->area;
    if ( !pArea )
    {
	send_to_char(
	"An error has occurred, unable to find your area.\n\r", ch);
	send_to_char(
	"Please contact someone on staff to rectify the problem.\n\r", ch);
	return FALSE;
    }

    for(value = pArea->min_vnum; value <= pArea->max_vnum; value++)
    {
	if(get_room_index(value) == NULL)
	    break;
    }

    if ( value > pArea->max_vnum )
    {
	send_to_char(
	"An error has occurred, no available vnums in zone.\n\r", ch );
	send_to_char(
	"You can try again, or contact a staff member for help.\n\r", ch);
	return FALSE;
    }

    pRoom                       = new_room_index();
    pRoom->area                 = pArea;
    pRoom->vnum                 = value;

    if ( value > top_vnum_room )
	top_vnum_room = value;

    iHash                       = value % MAX_KEY_HASH;
    pRoom->next                 = room_index_hash[iHash];
    room_index_hash[iHash]      = pRoom;
    pRoom->room_flags		= ROOM_INDOORS;
    PURGE_DATA(pRoom->owner);
    if(org)
	pRoom->owner = str_dup(org->name);
    else
	pRoom->owner = str_dup(ch->name);

    send_to_char( "Room created.\n\r", ch );
    return pRoom->vnum;
}

int gen_exit( CHAR_DATA *ch, int door, int value )
{
    ROOM_INDEX_DATA *pRoom;
    EXIT_DATA *pExit;

    if ( (pRoom = get_room_index( value )) == NULL )
    {
	send_to_char("Cannot link to non-existant room.\n\r", ch );
	return FALSE;
    }

    if ( pRoom->exit[rev_dir[door]] )
    {
	send_to_char("Remote side's exit already exists.\n\r", ch);
	send_to_char("Please contact a staff member for assistance.\n\r", ch);
        return FALSE;
    }

    pRoom = ch->in_room;

    if ( !pRoom->exit[door] )
    {
	pRoom->exit[door] = new_exit();
    }

    pRoom->exit[door]->u1.to_room = get_room_index( value );
    pRoom->exit[door]->orig_door = door;
    pRoom->exit[door]->exit_info = EX_ISDOOR|EX_CLOSED;

    pRoom                   = get_room_index( value );
    door                    = rev_dir[door];
    pExit                   = new_exit();
    pExit->u1.to_room       = ch->in_room;
    pExit->orig_door        = door;
    pExit->exit_info        = EX_ISDOOR|EX_CLOSED;
    pRoom->exit[door]       = pExit;

    send_to_char( "Two-way link established.\n\r", ch );
    return TRUE;
}

char * insult(CHAR_DATA *ch)
{
	int i = 0, j = 0;

	while(insult_table[i].name != NULL)
	{
		i++;
	}

	j = number_range(0, i-1);
	if(insult_table[j].bit == 0 || insult_table[j].bit == ch->sex)
		return insult_table[j].name;
	else
		return insult(ch);
}

void arm_hunter(CHAR_DATA *hch)
{
	OBJ_DATA *obj;

	if(hch->sex == SEX_FEMALE)
	{
		obj = create_object(get_obj_index(OBJ_VNUM_HUNTER_TANKERBOOTS	));
		obj_to_char(obj, hch);
		equip_char(hch, obj, WEAR_FEET);
		obj = create_object(get_obj_index(OBJ_VNUM_HUNTER_CARGOPANTS	));
		obj_to_char(obj, hch);
		equip_char(hch, obj, WEAR_LEGS);
		obj = create_object(get_obj_index(OBJ_VNUM_HUNTER_MESHJACKET	));
		obj_to_char(obj, hch);
		equip_char(hch, obj, WEAR_ABOUT);
		obj = create_object(get_obj_index(OBJ_VNUM_HUNTER_TSHIRT	));
		obj_to_char(obj, hch);
		equip_char(hch, obj, WEAR_BODY);
	}
	else
	{
		obj = create_object(get_obj_index(OBJ_VNUM_HUNTER_DRESSSHOES	));
		obj_to_char(obj, hch);
		equip_char(hch, obj, WEAR_FEET);
		obj = create_object(get_obj_index(OBJ_VNUM_HUNTER_DRESSPANTS	));
		obj_to_char(obj, hch);
		equip_char(hch, obj, WEAR_LEGS);
		obj = create_object(get_obj_index(OBJ_VNUM_HUNTER_DRESSSHIRT	));
		obj_to_char(obj, hch);
		equip_char(hch, obj, WEAR_BODY);
		obj = create_object(get_obj_index(OBJ_VNUM_HUNTER_ROMANCOLLAR	));
		obj_to_char(obj, hch);
		equip_char(hch, obj, WEAR_NECK);
		obj = create_object(get_obj_index(OBJ_VNUM_HUNTER_CREAMJACKET	));
		obj_to_char(obj, hch);
		equip_char(hch, obj, WEAR_ABOUT);
	}

	if(hch->summonner->race == race_lookup("werewolf"))
	{
		obj = create_object(get_obj_index(OBJ_VNUM_HUNTER_SILVERSWORD	));
		obj_to_char(obj, hch);
		equip_char(hch, obj, WEAR_WIELD);
	}
	else if(hch->summonner->race == race_lookup("vampire"))
	{
		obj = create_object(get_obj_index(OBJ_VNUM_HUNTER_FLAMETHROWER	));
		obj_to_char(obj, hch);
		equip_char(hch, obj, WEAR_WIELD);
	}
	else if(hch->summonner->race == race_lookup("faerie"))
	{
		obj = create_object(get_obj_index(OBJ_VNUM_HUNTER_IRONPOKER	));
		obj_to_char(obj, hch);
		equip_char(hch, obj, WEAR_WIELD);
	}
	else if(hch->summonner->race == race_lookup("wraith"))
	{
		obj = create_object(get_obj_index(OBJ_VNUM_HUNTER_SILVERSWORD	));
		obj_to_char(obj, hch);
		equip_char(hch, obj, WEAR_WIELD);
	}
	else
	{
		obj = create_object(get_obj_index(OBJ_VNUM_HUNTER_RIFLE		));
		obj_to_char(obj, hch);
		equip_char(hch, obj, WEAR_WIELD);
	}
}

CHAR_DATA * gen_hunter (CHAR_DATA *ch)
{
	CHAR_DATA * hunter = create_mobile(get_mob_index(MOB_VNUM_BLANKY));
	int i = 0;

	PURGE_DATA(hunter->name);
	PURGE_DATA(hunter->short_descr);
	hunter->name          = str_dup("hunter $f $l");
	hunter->short_descr   = str_dup("$f $l is out hunting...");
	rand_name(hunter);
	hunter->act           = hunter->act;
	hunter->act2          = ACT2_HUNTER|ACT2_TWILIGHT|hunter->act2;
	for(i=0; i<MAX_STATS; i++)
		hunter->perm_stat[i]      = number_fuzzy(ch->perm_stat[i]);
	for(i=0; i<MAX_STATS; i++)
		hunter->mod_stat[i]       = number_fuzzy(ch->mod_stat[i]);
	hunter->blood_timer   = 10;
	hunter->material      = ch->material;
	hunter->race          = race_lookup("human");
	hunter->clan          = clan_lookup("None");
	hunter->size          = ch->size;
	hunter->dam_type      = ch->dam_type;
	hunter->affected_by   = ch->affected_by;
	hunter->affected_by2  = ch->affected_by2;
	for(i=0; i<MAX_ABIL; i++)
		hunter->ability[i].value = number_fuzzy(ch->ability[i].value);
	hunter->health        = MAX_HEALTH;
	hunter->agghealth     = MAX_HEALTH;
	hunter->position      = P_STAND;
	hunter->in_room       = get_room_index(ch->in_room->area->min_vnum);
	hunter->RBPG          = number_fuzzy(ch->max_RBPG);
	hunter->max_RBPG      = number_fuzzy(ch->max_RBPG);
	hunter->GHB           = number_fuzzy(ch->max_GHB);
	hunter->max_GHB       = number_fuzzy(ch->max_GHB);
	hunter->willpower     = number_fuzzy(ch->max_willpower);
	hunter->max_willpower = number_fuzzy(ch->max_willpower);
	hunter->saving_throw  = ch->saving_throw;
	hunter->sex           = number_range(1,2);
	hunter->trust         = 0;
	for(i=0; i<(MAX_VIRTUES - 1); i++)
		hunter->virtues[i] = number_fuzzy(ch->virtues[i]);
	desc_gen(hunter);

	hunter->summonner = ch;
	ch->hunted = TRUE;

	arm_hunter(hunter);

	char_to_room(hunter, hunter->in_room);
	return hunter;
}

bool IS_HUNTED(CHAR_DATA *ch)
{
    if(ch->hunted)
	return TRUE;

    return FALSE;
}

int QUALITY_OF_WORK(int fail)
{
    if(fail <= 2) return 1;
    if(fail <= 4) return 2;
    if(fail == 5) return 3;
    if(fail == 6) return 4;
    if(fail == 7) return 6;
    if(fail == 8) return 8;
    if(fail == 9) return 10;
    return 15;
}

bool is_in_group(CHAR_DATA *ch)
{
	CHAR_DATA *wch;

	for(wch = char_list; wch != NULL; wch = wch->next)
	{
		if(ch == wch) continue;

		if(is_same_group(wch, ch)) return TRUE;
	}

	return FALSE;
}

void extract_mem(ORG_DATA *org, ORGMEM_DATA *mem)
{
	ORGMEM_DATA *p;

	if(mem == org->members)
	{
		org->members = org->members->next;
		free_orgmem(mem);
	}
	else
	{
		for(p = org->members; p; p = p->next)
		{
			if(p->next == mem)
			{
				p->next = mem->next;
				free_orgmem(mem);
			}
		}
	}
}

void clear_orgs(CHAR_DATA *ch)
{
	ORG_DATA *org;
	ORGMEM_DATA *mem;

	for(org = org_list; org != NULL; org = org->next)
	{
		if((mem = mem_lookup(org, ch->name)) != NULL)
		{
			extract_mem(org, mem);
			fwrite_org(org);
		}
	}
}

int current_dots(CHAR_DATA *ch)
{
	int k = 0,i = 0,j = 0;

	for( i = 0; i < MAX_ABIL; i++ )
		k = ch->ability[i].value + k;
	for( i = 0; i < MAX_STATS; i++ )
		k = ch->perm_stat[i] + k;
	for( i = 0; i < MAX_DISC; i++ )
		k = ch->disc[i] + k;
	for( i = 0; i < MAX_BG; i++ )
		k = ch->backgrounds[i] + k;
	for( i = 0; i < MAX_VIRTUES; i++ )
		k = ch->virtues[i] + k;
	for( i = 0; i < 5; i++ )
		for(j=0;j<31;j++)
			if(IS_SET(ch->powers[i], flag_list[j].bit))
				k++;
	if(ch->max_willpower > 5) k += ch->max_willpower - 5;
	if(ch->max_GHB > 5) k += ch->max_willpower - 5;
	if(ch->max_RBPG > 5) k += ch->max_willpower - 5;

	return k;
}

int xp_cost_mod(CHAR_DATA *ch, int cost, int curdot)
{
	int dots = current_dots(ch);
	int inc = 1;

	if(dots > START_DOTS && curdot < 3)
		inc = 1 + ((dots-START_DOTS)*400)/(START_DOTS*100);

	return /*inc * */cost;
}

int trait_count(CHAR_DATA *ch, int type)
{
	TRAIT_DATA *t;
	int icount = 0;

	for(t = ch->traits; t != NULL; t = t->next)
	{
		if((t->value < 0 && type < 0)
				|| (t->value > 0 && type > 0)
				|| (t->value == 0 && type == 0))
			icount++;
	}

	return icount;
}

int has_trait(CHAR_DATA *ch, TRAIT_DATA *p)
{
	TRAIT_DATA *t;

	for(t = ch->traits; t != NULL; t = t->next)
	{
		if(t->value == p->value && t->type == p->type && !str_cmp(t->qualifier, p->qualifier)
				&& !str_cmp(t->detail, p->detail))
			return TRUE;
	}

	return FALSE;
}

CHAR_DATA *sat_on(OBJ_DATA *obj)
{
	CHAR_DATA *gch;

	if(obj->in_room != NULL)
	{
		for (gch = obj->in_room->people; gch != NULL; gch = gch->next_in_room)
			if (gch->on == obj)
			{
				return gch;
			}
	}

	return NULL;
}

bool LOOKS_DIFFERENT(CHAR_DATA *ch)
{
    Assert(ch, "Character was NULL: Crash incoming!");

    if(IS_AFFECTED(ch, AFF_OBFUSCATE3)) 
		return TRUE;
    if(IS_AFFECTED2(ch, AFF2_VICISSITUDE1)) 
		return TRUE;

    return FALSE;
}

bool coat_obj_in_liquid_by_name(OBJ_DATA *obj, char *liquid)
{
    LIQUID_DATA *coating;
    int i = 0;

    if((i = material_lookup(liquid)) < 0)
	return TRUE;

    if(material_table[i].is_liquid == FALSE)
	return TRUE;

    coating = new_liqdata();
    coating->liquid = liq_lookup(liquid);
    coating->quantity = 1;

    coating->next = obj->coating;
    obj->coating = coating;

    return FALSE;
}

bool coat_char_in_liquid_by_name(CHAR_DATA *ch, char *liquid)
{
    LIQUID_DATA *coating;
    int i = 0;

    if((i = material_lookup(liquid)) < 0)
	return TRUE;

    if(material_table[i].is_liquid == FALSE)
	return TRUE;

    coating = new_liqdata();
    coating->liquid = liq_lookup(liquid);
    coating->quantity = 1;

    coating->next = ch->coating;
    ch->coating = coating;

    return FALSE;
}


bool coat_obj_in_liquid(OBJ_DATA *obj, OBJ_DATA *liquid)
{
    LIQUID_DATA *coating;

    if(liquid->item_type != ITEM_LIQUID
    && material_table[material_lookup(liquid->material)].is_liquid == FALSE)
	return TRUE;

    if(liquid->weight > 0) liquid->weight--;

    coating = new_liqdata();
    coating->liquid = liq_lookup(liquid->material);
    coating->quantity = 1;

    coating->next = obj->coating;
    obj->coating = coating;

    return FALSE;
}

bool coat_char_in_liquid(CHAR_DATA *ch, OBJ_DATA *liquid)
{
	LIQUID_DATA *coating;

	if(liquid->item_type != ITEM_LIQUID && material_table[material_lookup(liquid->material)].is_liquid == FALSE)
		return TRUE;

	if(liquid->weight > 0) liquid->weight--;

	coating = new_liqdata();
	coating->liquid = liq_lookup(liquid->material);
	coating->quantity = 1;

	coating->next = ch->coating;
	ch->coating = coating;

	return FALSE;
}

/* create_puddle_from_obj takes the object to create it from and the
 * vnum of the object to create (Generally OBJ_VNUM_PUDDLE or
 * OBJ_VNUM_POOL) */
OBJ_DATA * create_puddle_from_obj(OBJ_DATA *obj, int vnum, bool scrap_obj)
{
    OBJ_DATA *puddle;
    AFFECT_DATA *paf;
    char buf[MSL]={'\0'};
    int i = 0;

    if(obj == NULL) return NULL; /* Sanity check. */

    if(obj->item_type != ITEM_LIQUID
    && material_table[material_lookup(obj->material)].is_liquid == FALSE)
	return NULL;

    i = liq_lookup(obj->material);
    if(i < 0) return NULL;

    puddle = create_object(get_obj_index(vnum));

    PURGE_DATA(puddle->name);
    snprintf(buf, sizeof(buf), puddle->pIndexData->name, liq_table[i].liq_color,
	liq_table[i].liq_name);
    puddle->name = str_dup(buf);
    PURGE_DATA(puddle->short_descr);
    snprintf(buf, sizeof(buf), puddle->pIndexData->short_descr, liq_table[i].liq_color);
    puddle->short_descr = str_dup(buf);
    PURGE_DATA(puddle->description);
    snprintf(buf, sizeof(buf), puddle->pIndexData->description, liq_table[i].liq_color);
    puddle->description = str_dup(buf);
    PURGE_DATA(puddle->full_desc);
    snprintf(buf, sizeof(buf), puddle->pIndexData->full_desc, liq_table[i].liq_color);
    puddle->full_desc = str_dup(buf);

    PURGE_DATA(puddle->material);
    puddle->material = str_dup(obj->material);
    puddle->weight = obj->weight;
    puddle->item_type = ITEM_LIQUID;
    puddle->extra_flags = obj->extra_flags;
    puddle->wear_flags = 0;
    puddle->cost = obj->cost;
    puddle->condition = 100;
    puddle->timer = 150 * puddle->weight;
    puddle->enchanted = obj->enchanted;
    PURGE_DATA(puddle->owner);
    puddle->owner = str_dup(obj->owner);

    for (paf = obj->affected; paf != NULL; paf = paf->next)
	affect_to_obj(puddle,paf);

    if(scrap_obj)
	extract_obj(obj);

    return puddle;
}

OBJ_DATA * create_puddle_from_liq(char *liq, int vnum, int volume)
{
    OBJ_DATA *puddle;
    char buf[MSL]={'\0'};
    int i = 0;

    if(material_table[material_lookup(liq)].is_liquid == FALSE)
	return NULL;

    puddle = create_object(get_obj_index(vnum));

    i = liq_lookup(liq);

    PURGE_DATA(puddle->name);
    snprintf(buf, sizeof(buf), puddle->pIndexData->name, liq_table[i].liq_color,
	liq_table[i].liq_name);
    puddle->name = str_dup(buf);
    PURGE_DATA(puddle->short_descr);
    snprintf(buf, sizeof(buf), puddle->pIndexData->short_descr, liq_table[i].liq_color);
    puddle->short_descr = str_dup(buf);
    PURGE_DATA(puddle->description);
    snprintf(buf, sizeof(buf), puddle->pIndexData->description, liq_table[i].liq_color);
    puddle->description = str_dup(buf);
    PURGE_DATA(puddle->full_desc);
    snprintf(buf, sizeof(buf), puddle->pIndexData->full_desc, liq_table[i].liq_color);
    puddle->full_desc = str_dup(buf);

    PURGE_DATA(puddle->material);
    puddle->material = str_dup(material_table[i].name);
    puddle->weight = volume;
    puddle->item_type = ITEM_LIQUID;
    puddle->wear_flags = 0;
    puddle->cost = material_table[i].value * volume;
    puddle->condition = 100;
    puddle->timer = 150 * puddle->weight;

    return puddle;
}

int count_multi_args(char *argument, char *cEnd)
{
	int count = 0;
	char *spot;

	spot = argument;
	if(spot[0] == '\0') return count;
	else count++;

	while ( *spot != '\0' )
	{
		if ( (spot = strstr(spot, cEnd)) != '\0')
		{
			count++;
		}
	}

	return count;
}


char *appearance_string(CHAR_DATA *ch)
{
	int app = 0;
	char *tmp;

	app = get_curr_stat(ch, STAT_APP);
	if (app == 0)
		tmp = "Hideous";
	else if (app == 1)
		tmp = "Ugly";
	else if (app == 2)
		tmp = "Plain";
	else if (app == 3)
		tmp = "Average";
	else if (app == 4)
		tmp = "Attractive";
	else
		tmp = "Radiant";

	return tmp;
}

char *gender_string(CHAR_DATA *ch)
{
	int gender = 0;
	char *tmp;

	gender = ch->sex;
	if (gender == 0)
		tmp = "Sexless";
	else if (gender == 1)
		tmp = "\t[F025]\t[U9794/Male]\tn";
	else
		tmp = "\t[F414]\t[U9792/Female]\tn";

	return tmp;
}

void purgeExtractedWorldData(void)
{
	CHAR_DATA *ch, *ch_next;
	OBJ_DATA *obj, *obj_next;

	for(ch = worldExtractedCharacters; ch; ch = ch_next)
	{
		ch_next = ch->next;
		UNLINK_SINGLE(ch, next, CHAR_DATA, worldExtractedCharacters);
		free_char(ch);
	}
	for(obj = worldExtractedObjects; obj; obj = obj_next)
	{
		obj_next = obj->next;
		UNLINK_SINGLE(obj, next, OBJ_DATA, worldExtractedObjects);
		free_obj(obj);
	}
}

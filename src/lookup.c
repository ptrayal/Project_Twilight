#if defined(Macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <time.h>
#include "twilight.h"
#include "tables.h"

int flag_lookup (const char *name, const struct flag_type *flag_table)
{
	int flag;

	for (flag = 0; flag_table[flag].name != NULL; flag++)
	{
		if (LOWER(name[0]) == LOWER(flag_table[flag].name[0]) &&  !str_prefix(name,flag_table[flag].name))
			return flag_table[flag].bit;
	}

	return -1;
}

int position_lookup (const char *name)
{
	int pos;

	for (pos = 0; position_table[pos].name != NULL; pos++)
	{
		if (LOWER(name[0]) == LOWER(position_table[pos].name[0]) &&  !str_prefix(name,position_table[pos].name))
			return pos;
	}

	return -1;
}

int sex_lookup (const char *name)
{
	int sex;

	for (sex = 0; sex_table[sex].name != NULL; sex++)
	{
		if (LOWER(name[0]) == LOWER(sex_table[sex].name[0]) &&  !str_prefix(name,sex_table[sex].name))
			return sex;
	}

	return -1;
}

int size_lookup (const char *name)
{
	int size;

	for ( size = 0; size_table[size].name != NULL; size++)
	{
		if (LOWER(name[0]) == LOWER(size_table[size].name[0]) &&  !str_prefix( name,size_table[size].name))
			return size;
	}

	return -1;
}

int abil_lookup (const char *name, CHAR_DATA *ch)
{
	int abil = 0;

	for (abil = 0; ability_table[abil].name != NULL; abil++)
	{
		if (LOWER(name[0]) == LOWER(ability_table[abil].name[0]) &&  !str_prefix(name,ability_table[abil].name))
		{
			if (IS_ATTRIB_AVAILABLE(ch->race, abil))
				return abil;
		}
	}

	return -1;
}

int stat_lookup (const char *name, CHAR_DATA *ch)
{
	int abil;

	for (abil = 0; stat_table[abil].name != NULL; abil++)
	{
		if (LOWER(name[0]) == LOWER(stat_table[abil].name[0]) &&  !str_prefix(name,stat_table[abil].name))
			return abil;
	}

	return -1;
}

int virtue_lookup (const char *name)
{
	int pos;

	for (pos = 0; pos < 3; pos++)
	{
		if (LOWER(name[0]) == LOWER(virtue_table[pos].name[0]) &&  !str_prefix(name,virtue_table[pos].name))
			return pos;
	}

	return -1;
}

int disc_lookup (CHAR_DATA *ch, const char *name)
{
	int pos = 0;

	for (pos = 0; disc_table[pos].vname != NULL; pos++)
	{
		if(ch->race == race_lookup("vampire"))
		{
			if (LOWER(name[0]) == LOWER(disc_table[pos].vname[0]) &&  !str_prefix(name,disc_table[pos].vname))
				return pos;
		}
		else if(ch->race == race_lookup("werewolf"))
		{
			if(disc_table[pos].wname == NULL)
			{
				break;
			}
			if (LOWER(name[0]) == LOWER(disc_table[pos].wname[0])
				&&  !str_prefix(name,disc_table[pos].wname))
			{
				return pos;
			}
		}
		else if(ch->race == race_lookup("human"))
		{
			if(disc_table[pos].hname == NULL)
			{
				break;
			}
			if (LOWER(name[0]) == LOWER(disc_table[pos].hname[0]) &&  !str_prefix(name,disc_table[pos].hname))
			{
				return pos;
			}
		}
	}

	return -1;
}

int exit_lookup(CHAR_DATA *ch, const char *dir)
{
	return -1;
}

int get_points(CHAR_DATA *ch)
{
	int k,i,total = 0;

	k = 0;
	for( i = 0; i < MAX_ABIL; i++ )
		k = ch->ability[i].value + k;
	total = (k - 13 - 9 - 5) * 2;
	k = 0;
	for( i = 0; i < MAX_STATS; i++ )
		k = ch->perm_stat[i] + k;
	total += (k - 7 - 5 - 3) * 4;
	k = 0;
	for( i = 0; i < MAX_DISC; i++ )
		k = ch->disc[i] + k;
	total += (k - 3) * 7;
	total += ch->exp;

	total = total/2;

	return total;
}

/*char * weapon_type(int v1)
{
switch(v1)
    {
	case 0: return "exotic"; break;
	case 1: return "firearm"; break;
	case 2: return "blade"; break;
	case 3: return "blunt"; break;
	case 4: return "whip"; break;
	case 5: return "grapple"; break;
    }
}*/


int job_lookup (char *name)
{
	int job;

	if(name == NULL)
	{
		name = str_dup("None");
	}

	for (job = 0; job_table[job].name != NULL; job++)
	{
		if (LOWER(name[0]) == LOWER(job_table[job].name[0])
				&&  !str_prefix(name,job_table[job].name))
			return job;
	}

	return -1;
}

int job_cmd_lookup (char *name, void * table)
{
	const struct job_cmd_type *jcmd_table = (const struct job_cmd_type *)table;
	int job;

	for (job = 0; jcmd_table[job].name != NULL; job++)
	{
		if (LOWER(name[0]) == LOWER(jcmd_table[job].name[0]) &&  !str_prefix(name,jcmd_table[job].name))
			return job;
	}

	return -1;
}

int office_lookup (char *name)
{
    int office;

    for (office = 0; office_table[office].name != NULL; office++)
    {
	if (LOWER(name[0]) == LOWER(office_table[office].name[0])
	&&  !str_prefix(name,office_table[office].name))
	    return office;
    }

    return -1;
}

int influence_cmd_lookup (char *name)
{
    int i;

    for (i = 0; influence_cmd_table[i].name != NULL; i++)
    {
	if (LOWER(name[0]) == LOWER(influence_cmd_table[i].name[0])
	&&  !str_prefix(name,influence_cmd_table[i].name))
	    return i;
    }

    return -1;
}

int bg_cmd_lookup (char *name)
{
    int i;

    for (i = 0; bg_cmd_table[i].name != NULL; i++)
    {
	if (LOWER(name[0]) == LOWER(bg_cmd_table[i].name[0])
	&&  !str_prefix(name,bg_cmd_table[i].name))
	    return i;
    }

    return -1;
}

int influence_lookup (char *name)
{
    int i;

    for (i = 0; influence_table[i].name != NULL; i++)
    {
	if (LOWER(name[0]) == LOWER(influence_table[i].name[0])
	&&  !str_prefix(name,influence_table[i].name))
	    return i;
    }

    return -1;
}

int background_lookup (char *name)
{
    int i;

    for (i = 0; background_table[i].name != NULL; i++)
    {
	if (LOWER(name[0]) == LOWER(background_table[i].name[0])
	&&  !str_prefix(name,background_table[i].name))
	    return i;
    }

    return -1;
}

int news_cmd_lookup (char *name)
{
	int i;

	for (i = 0; news_cmd_table[i].name != NULL; i++)
	{
		if (LOWER(name[0]) == LOWER(news_cmd_table[i].name[0]) &&  !str_prefix(name,news_cmd_table[i].name))
			return i;
	}

	return -1;
}

NEWSPAPER *newspaper_lookup (char *name)
{
	NEWSPAPER *paper;

	for (paper = paper_list; paper; paper = paper->next)
	{
		if (LOWER(name[0]) == LOWER(paper->name[0]) &&  !str_prefix(name,paper->name))
			return paper;
	}

	return NULL;
}

int smarket_cmd_lookup (char *name)
{
	int i;

	for (i = 0; smarket_cmd_table[i].name != NULL; i++)
	{
		if (LOWER(name[0]) == LOWER(smarket_cmd_table[i].name[0]) &&  !str_prefix(name,smarket_cmd_table[i].name))
			return i;
	}

	return -1;
}

STOCKS *stock_lookup (char *name)
{
	STOCKS *stock;

	for (stock = stock_list; stock; stock = stock->next)
	{
		if (LOWER(name[0]) == LOWER(stock->name[0]) &&  !str_prefix(name,stock->name))
			return stock;
	}

	for (stock = stock_list; stock; stock = stock->next)
	{
		if (LOWER(name[0]) == LOWER(stock->ticker[0]) &&  !str_prefix(name,stock->ticker))
			return stock;
	}

	return NULL;
}

ORG_DATA *org_lookup (char *name)
{
	ORG_DATA *org;

	for (org = org_list; org; org = org->next)
	{
		if (LOWER(name[0]) == LOWER(org->name[0]) && !str_prefix(name,org->name))
			return org;
	}

	return NULL;
}

PACK_DATA *pack_lookup (char *name)
{
	PACK_DATA *pack;

	for (pack = pack_list; pack; pack = pack->next)
	{
		if (LOWER(name[0]) == LOWER(pack->name[0]) &&  !str_prefix(name,pack->name))
			return pack;
	}

	return NULL;
}

int gift_lookup (char *name)
{
	int i;

	for (i = 0; gift_table[i].name != NULL; i++)
	{
		if (LOWER(name[0]) == LOWER(gift_table[i].name[0]) &&  !str_prefix(name,gift_table[i].name))
			return i;
	}

	return -1;
}

long group_gift_lookup (char *name, int level)
{
	long i;

	for (i = 0; group_gift_table[i].name != NULL; i++)
	{
		if (LOWER(name[0]) == LOWER(group_gift_table[i].name[0]) &&  !str_prefix(name,group_gift_table[i].name))
			return group_gift_table[i].flag[level];
	}

	return 0;
}

char * colour_lookup (char name, bool colour)
{
	int flag;

	for (flag = 0; colour_table[flag].code != NULL; flag++)
	{
		if ((colour == colour_table[flag].iscolour || !colour_table[flag].iscolour) && name == colour_table[flag].code[0])
			return colour_table[flag].coltag;
	}

	return CLEAR;
}

ORGMEM_DATA * mem_lookup(ORG_DATA *org, char *name)
{
	ORGMEM_DATA *mem;

	for (mem = org->members; mem; mem = mem->next)
	{
		if (LOWER(name[0]) == LOWER(mem->name[0]) && !str_prefix(name,mem->name))
			return mem;
	}

	return NULL;
}

int home_cmd_lookup (char *name)
{
	int i;

	for (i = 0; home_cmd_table[i].name != NULL; i++)
	{
		if (LOWER(name[0]) == LOWER(home_cmd_table[i].name[0]) &&  !str_prefix(name,home_cmd_table[i].name))
			return i;
	}

	return -1;
}

int home_price_lookup (char *name)
{
	int i = 0;

	for (i = 0; home_price_table[i].name != NULL; i++)
	{
		if (LOWER(name[0]) == LOWER(home_price_table[i].name[0]) &&  !str_prefix(name,home_price_table[i].name))
			return home_price_table[i].cost;
	}

	return 0;
}

/*
 * Trait table lookups.
 * ONLY to be used for flaws, merits and derangements NOT quirks.
 */
int trait_lookup (const char *name, const struct trait_struct *trait_table)
{
	int t = 0;

	for (t = 0; trait_table[t].name != NULL; t++)
	{
		if (LOWER(name[0]) == LOWER(trait_table[t].name[0]) &&  !str_prefix(name,trait_table[t].name))
			return trait_table[t].bit;
	}

	return -1;
}

/*
 * Ritual table lookups.
 */
int rite_lookup (CHAR_DATA *ch)
{
	int t = 0;
	int i = 0;
	bool found = TRUE;

	for (t = 0; ritual_table[t].name != NULL; t++)
	{
		found = TRUE;
		for(i = 0; i < MAX_RITE_STEPS; i++)
		{
			if (ch->riteacts[i] != ritual_table[t].actions[i])
				found = FALSE;
		}
		if(found == TRUE) break;
	}

	if(found == FALSE) return -1;

	return t;
}

int riteaction_lookup (const char *name)
{
	int t = 0;

	// prevent negative exposure here; null name = this thing blow up on LOWER(name[0])
	if(name == NULL || name[0] == '\0')
		return -1;

	for (t = 0; rite_actions[t].name != NULL; t++)
	{
		if (LOWER(name[0]) == LOWER(rite_actions[t].name[0]) &&  !str_prefix(name,rite_actions[t].name))
			return t;
	}

	return -1;
}

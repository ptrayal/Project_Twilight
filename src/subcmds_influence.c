/***************************************************************************
 * Much of the code is the original work of Peter Fitzgerald who turned    *
 * it over to Brandon Morrison who has adopted and improved the code.      *
 * Copyright (C) 2012 - 2019                                               *
 **************************************************************************/
 
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


/* >>>>>>>>>>>>>>> Influence Commands <<<<<<<<<<<<<<< */
void do_influences (CHAR_DATA *ch, char *argument)
{
	char arg[MIL]={'\0'};
	int in = 0, cmd = -1;

	CheckCH(ch);

	if(IS_NULLSTR(argument))
	{
		send_to_char("How do you wish to use your influence?\n\r", ch);
		send_to_char("If you wish to see what you can do with influence, use \t(influence commands\t).\n\r", ch);
		return;
	}

	if(ch->infl_timer > 0 && str_prefix(argument, "commands") && str_prefix(argument, "advance"))
	{
		send_to_char("You're going to have to wait a bit or people won't respect your influence.\n\r", ch);
		return;
	}

	argument = one_argument(argument, arg);

	if((cmd = influence_cmd_lookup(arg)) > -1)
		{
			in = influence_cmd_table[cmd].type;
		}

	if(cmd <= -1 || ch->influences[in] < influence_cmd_table[cmd].level)
	{
		if(IS_ADMIN(ch))
		{
			if(!str_prefix(arg, "synopsis"))
			{
				influence_adminlist(ch, argument);
				return;
			}
		}
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if((*influence_cmd_table[cmd].func) ( ch, argument ))
		WAIT_STATE(ch, influence_cmd_table[cmd].delay);

	return;
}

int influence_commands(CHAR_DATA *ch, char *argument)
{
	int cmd = 0;
	int col = 0;
	int j = NO_FLAG;

	if(!IS_NULLSTR(argument) && (j = flag_value(influence_table, argument)) == NO_FLAG)
	{
		send_to_char("No such influence type.\n\r", ch);
		return FALSE;
	}

	for( cmd = 0; influence_cmd_table[cmd].name != NULL; cmd++ )
	{
		int i = 0;
		i = influence_cmd_table[cmd].type;
		if(ch->influences[i] >= influence_cmd_table[cmd].level)
		{
			if(j == NO_FLAG || i == j)
			{
				send_to_char(Format("\t<send href='influence %s'>%-12s\t</send> | ", influence_cmd_table[cmd].name, influence_cmd_table[cmd].name), ch);
				if(++col % 4 == 0)
					send_to_char("\n\r", ch);
			}
		}
	}

	if(IS_ADMIN(ch))
	{
		send_to_char(Format("%-12s", "synopsis"), ch);
		col++;
	}

	if(col %6 != 0)
		send_to_char("\n\r", ch);
	return TRUE;
}

int influence_advance(CHAR_DATA *ch, char *argument)
{
	int max_stat = 5;
	int in = 0, cost = 0;

	if(IS_NPC(ch))
		return FALSE;

	if(IS_NULLSTR(argument))
	{
		send_to_char("Which influence do you wish to advance?\n\r", ch);
		return FALSE;
	}

	if((in = influence_lookup(argument)) == -1 )
	{
		send_to_char("There is no influence of that name.\n\r", ch);
		return FALSE;
	}

	if(ch->influences[in] >= max_stat)
	{
		send_to_char("That influence is already maxed out.\n\r", ch);
		return FALSE;
	}

	if(ch->influences[in] == 0)
		cost = 7;
	else
		cost = ch->influences[in] * 5;

	if(ch->exp < cost)
	{
		send_to_char("You don't have enough experience.\n\r", ch);
		return FALSE;
	}

	ch->exp -= cost;
	ch->influences[in]++;
	act("Your $t increases.", ch, influence_table[in].name, NULL, TO_CHAR, 1);

	return TRUE;
}

int influence_adminlist(CHAR_DATA *ch, char *argument)
{
	DESCRIPTOR_DATA *vd;
	int inf = 0;

	if(IS_NULLSTR(argument))
	{
		send_to_char("Syntax: \tCinfluence synopsis [influence type]\tn\n\r", ch);
		return FALSE;
	}

	if((inf = influence_lookup(argument)) == -1)
	{
		send_to_char("Influence not recognised.\n\r", ch);
		return FALSE;
	}

	send_to_char(Format("\tW%s Influence\tn\n\r\tW------------------------\tn\n\r", influence_table[inf].name), ch);
	for(vd = descriptor_list; vd != NULL; vd = vd->next)
	{
		if(vd->connected == CON_PLAYING)
		{
			if(IS_NPC(vd->character) && vd->original == NULL)
				continue;
			if(IS_NPC(vd->character) && vd->original != NULL && IS_NPC(vd->original))
				continue;

			if(vd->original != NULL)
			{
				send_to_char(Format("\tY%-20s %d\tn\n\r", vd->original->name, vd->original->influences[inf]), ch);
			}
			else
			{
				send_to_char(Format("\tY%-20s %d\tn\n\r", vd->character->name,	vd->character->influences[inf]), ch);
			}
		}
	}
	return FALSE;
}


/* Church Influence */
int church_collection(CHAR_DATA *ch, char *argument)
{
	int fail = dice_rolls(ch, get_curr_stat(ch, STAT_CHA) + ch->ability[LEADERSHIP].value, 7);

	if(fail > 0)
	{
		send_to_char(Format("\tGSuccess\tn: You manage to collect $%d.\n\r", fail * 100), ch);
		ch->dollars += fail * 100;
		ch->infl_timer = 1;
	}
	else if(fail == 0)
	{
		send_to_char("\tYFailure\tn: You fail to move people to donate.\n\r", ch);
		ch->infl_timer = 2;
	}
	else
	{
		send_to_char("\tRBOTCH\tn: You offend some of the group.\n\r", ch);
		ch->influences[INFL_CHURCH]--;
		ch->infl_timer = 4;

	}

	return TRUE;
}

int church_research(CHAR_DATA *ch, char *argument)
{
	int fail = 0;
	NOTE_DATA *pbg;

	/* 5 rolls making up an extended research attempt. */
	if(IS_NULLSTR(argument))
	{
		send_to_char("Research what?\n\r", ch);
		return FALSE;
	}
	else
		{
			pbg = find_knowledge_keyword(argument);
		}

	if(pbg == NULL)
	{
		send_to_char(Format("You find out nothing about %s.", argument), ch);
		return TRUE;
	}

	fail = UMIN(dice_rolls(ch, get_curr_stat(ch, STAT_INT)
			+ ch->ability[OCCULT].value, 7), 4 + ch->influences[INFL_CHURCH]);
	fail = fail + UMIN(dice_rolls(ch, get_curr_stat(ch, STAT_INT)
			+ ch->ability[OCCULT].value, 7), 4 + ch->influences[INFL_CHURCH]);
	fail = fail + UMIN(dice_rolls(ch, get_curr_stat(ch, STAT_INT)
			+ ch->ability[OCCULT].value, 7), 4 + ch->influences[INFL_CHURCH]);
	fail = fail + UMIN(dice_rolls(ch, get_curr_stat(ch, STAT_INT)
			+ ch->ability[OCCULT].value, 7), 4 + ch->influences[INFL_CHURCH]);
	fail = fail + UMIN(dice_rolls(ch, get_curr_stat(ch, STAT_INT)
			+ ch->ability[OCCULT].value, 7), 4 + ch->influences[INFL_CHURCH]);

	if(fail >= pbg->successes)
	{
		page_to_char( pbg->text, ch );
		return TRUE;
	}
	else
	{
		send_to_char(Format("You find out nothing about %s.", argument), ch);
	}

	ch->infl_timer = 2;
	return TRUE;
}

int church_tipoff(CHAR_DATA *ch, char *argument)
{
	int fail = dice_rolls(ch, get_curr_stat(ch, STAT_CHA) + ch->ability[SUBTERFUGE].value, 7);
	CHAR_DATA *vch;

	if((vch = get_char_world(ch, argument)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return FALSE;
	}

	if(vch->hunter_vis > 0)
	{
		fail += vch->hunter_vis/10;
	}

	/* Fail conditions. */
	if(fail < 0)
	{
		send_to_char("Your story is openly ridiculed by your peers.\n\r", ch);
		ch->influences[INFL_CHURCH] -= 2;
		return FALSE;
	}
	else if(fail < 2)
	{
		send_to_char("Nobody believes your story.\n\r", ch);
		ch->influences[INFL_CHURCH]--;
		return FALSE;
	}

	/* Generate hunter and set on target. */
	send_to_char("The word is spread about that individual.\n\r", ch);
	gen_hunter(vch);
	ch->infl_timer = 3;
	ch->influences[INFL_CHURCH]--;
	return TRUE;
}

int church_findrelic(CHAR_DATA *ch, char *argument)
{
	extern int top_obj_index;
	OBJ_INDEX_DATA      *pObjIndex;
	BUFFER              *buf1;
	bool found;
	int vnum = 0;
	// long largest = 0;
	int  lsize = 0;
	int  col = 0;
	int nMatch = 0;

	buf1 = new_buf();
	found   = FALSE;
	nMatch      = 0;

	/*
	 * Yeah, so iterating over all vnum's takes 10,000 loops.
	 * Get_obj_index is fast, and I don't feel like threading another link.
	 * Do you?
	 * -- Furey
	 */
	for ( vnum = 0; nMatch < top_obj_index; vnum++ )
	{
		if ( ( pObjIndex = get_obj_index( vnum ) ) )
		{
			nMatch++;
			if(pObjIndex->item_type == ITEM_RELIC)
			{
				found = TRUE;
				send_to_char("You know of the following relics which exist:\n\r", ch);
				add_buf( buf1, (char *)Format("%-17.16s", capitalize( pObjIndex->short_descr )) );
				if ( ++col % 3 == 0 )
				{
					add_buf( buf1, "\n\r" );
				}
				if(pObjIndex->weight > lsize)
				{
					lsize = pObjIndex->weight;
					// largest = pObjIndex->vnum;
				}
			}
		}
	}

	if ( !found )
	{
		send_to_char( "No object(s) found.\n\r", ch);
		return TRUE;
	}

	if ( col % 3 != 0 )
		add_buf( buf1, "\n\r" );

	page_to_char( buf_string(buf1), ch );
	free_buf(buf1);
	return TRUE;
}


/* Criminal Influence */
/* Use criminal influence to raise money */
int criminal_racket(CHAR_DATA *ch, char *argument)
{
	int fail = dice_rolls(ch, get_curr_stat(ch, STAT_MAN) + ch->ability[LARCENY].value, 7);

	if(fail > 0)
	{
		fail = number_fuzzy(fail*600);
		send_to_char(Format("\tGSuccess\tn: You manage to get $%d out of the racket.\n\r",fail), ch);
		ch->dollars += fail;
		ch->infl_timer = 1;
	}
	else if(fail == 0)
	{
		send_to_char("\tYFailure\tn: You fail to get ahead on the deal.\n\r", ch);
		ch->infl_timer = 2;
	}
	else
	{
		send_to_char("\tRBOTCH\tn: You offend some interested parties.\n\r", ch);
		ch->influences[INFL_CRIMINAL]--;
		ch->warrants += 5;
		ch->infl_timer = 4;
	}

	return TRUE;
}

int criminal_scout(CHAR_DATA *ch, char *argument)
{
	DESCRIPTOR_DATA *vd;
	int inf = 0;
	int fail = dice_rolls(ch, get_curr_stat(ch, STAT_CHA) + ch->ability[STREETWISE].value, 7);

	if(IS_NULLSTR(argument))
	{
		send_to_char("Syntax: \tCinfluence scout [influence type]\tn\n\r", ch);
		return FALSE;
	}

	if((inf = influence_lookup(argument)) == -1)
	{
		send_to_char("Influence not recognized.\n\r", ch);
		return FALSE;
	}

	if(fail > 0)
	{
		send_to_char(Format("\tW%s Influence\tn\n\r\tW------------------------\tn\n\r",influence_table[inf].name), ch);
		for(vd = descriptor_list; vd != NULL; vd = vd->next)
		{
			if(vd->connected == CON_PLAYING)
			{
				if(IS_NPC(vd->character) && vd->original == NULL)
					continue;
				if(IS_NPC(vd->character) && vd->original != NULL
						&& IS_NPC(vd->original))
					continue;

				if(vd->original != NULL)
				{
					send_to_char(Format("\tY%-20s %d\tn\n\r", vd->original->name, vd->original->influences[inf]), ch);
				}
				else
				{
					send_to_char(Format("\tY%-20s %d\tn\n\r", vd->character->name, vd->character->influences[inf]), ch);
				}
				ch->infl_timer = 3;
			}
		}
	}
	else if(fail == 0)
	{
		send_to_char("\tYFailure\tn: You fail to get any scouting reports.\n\r", ch);
		ch->infl_timer = 3;
	}
	else
	{
		send_to_char("\tRBOTCH\tn: You offend some interested parties.\n\r", ch);
		ch->influences[INFL_CRIMINAL]--;
		ch->infl_timer = 6;
	}

	return TRUE;
}

/* Political Influence */
/*  Use political influence to raise money */
int political_raise(CHAR_DATA *ch, char *argument)
{
	int fail = dice_rolls(ch, get_curr_stat(ch, STAT_CHA) + ch->ability[LEADERSHIP].value, 7);

	if(fail > 0)
	{
		fail = number_fuzzy(fail*100);
		send_to_char(Format("\tGSuccess\tn: You manage to collect $%d.\n\r", fail), ch);
		ch->dollars += fail;
	}
	else if(fail == 0)
	{
		send_to_char("\tYFailure\tn: You fail to move people to donate.\n\r", ch);
	}
	else
	{
		send_to_char("\tRBOTCH\tn: You offend some core supporters.\n\r", ch);
		ch->influences[INFL_POLITICAL]--;
	}
	ch->infl_timer = 2;

	return TRUE;
}


/* Get more votes for someone who is campaigning for office. */
int political_campaign(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *nominee;
	char arg[MIL]={'\0'};
	bool online = FALSE;
	bool in_char_list = FALSE;
	int i = 0,j = 0;
	int fail = dice_rolls(ch, get_curr_stat(ch, STAT_MAN) + ch->ability[POLITICS].value, 7);

	argument = one_argument(argument, arg);

	if(IS_NULLSTR(argument) || IS_NULLSTR(arg))
	{
		send_to_char("Vote whom for which position?\n\r", ch);
		return FALSE;
	}

	if(is_online(arg)) online = TRUE;
	if(pc_in_char_list(arg)) in_char_list = TRUE;

	if((nominee = get_player(arg)) == NULL)
	{
		send_to_char("No such person.\n\r", ch);
		return FALSE;
	}

	if(!str_prefix(argument, "mayor"))
	{
		if(!is_nominee(nominee, 0))
		{
			send_to_char("No such nominee.\n\r", ch);
			return FALSE;
		}

		for(i = 0; i < 6; i++)
		{
			if(!str_cmp(vote_tally[0][i].name, nominee->name))
			{
				vote_tally[0][i].votes += fail;
				break;
			}
		}
	}
	else if(!str_prefix(argument, "alderman"))
	{
		if(!is_nominee(nominee, 1))
		{
			send_to_char("No such nominee.\n\r", ch);
			return FALSE;
		}

		for(i = 0; i < 6; i++)
		{
			if(!str_cmp(vote_tally[1][i].name, nominee->name))
			{
				vote_tally[1][i].votes += fail;
				break;
			}
		}
	}
	else if(!str_prefix(argument, "judge"))
	{
		if(!is_nominee(nominee, 2))
		{
			send_to_char("No such nominee.\n\r", ch);
			return FALSE;
		}

		for(i = 0; i < 6; i++)
		{
			if(!str_cmp(vote_tally[2][i].name, nominee->name))
			{
				vote_tally[2][i].votes += fail;
				break;
			}
		}
	}
	else
	{
		send_to_char("Positions available are: judge, alderman, mayor.\n\r", ch);
		return FALSE;
	}

	if(fail > 0)
	{
		fwrite_votes();
		act(Format("\tGSuccess\tn: You manage to add %d votes to $N's cause.\n\r", fail), ch, NULL, nominee, TO_CHAR, 1);
		ch->influences[INFL_POLITICAL]--;
	}
	else if (fail < 0)
	{
		for(j = 0; j < 3; j++)
			for(i = 0; i < 6; i++)
			{
				if(!str_cmp(vote_tally[j][i].name, nominee->name))
				{
					vote_tally[j][i].votes += -1 * fail;
					break;
				}
			}
		send_to_char("\tRBOTCH\tn: You offend some core supporters.\n\r", ch);
		ch->influences[INFL_POLITICAL]--;
	}
	else
	{
		send_to_char("\tYFailure\tn: You fail to move people to vote.\n\r", ch);
	}

	if(!online && !in_char_list)
		free_char(nominee);
	ch->infl_timer = 1;

	return TRUE;
}

int political_negcampaign(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *nominee;
	char arg[MIL]={'\0'};
	bool online = FALSE;
	bool in_char_list = FALSE;
	int i = 0,j = 0;
	int fail = dice_rolls(ch, get_curr_stat(ch, STAT_MAN) + ch->ability[POLITICS].value, 7);

	argument = one_argument(argument, arg);

	if(IS_NULLSTR(argument) || IS_NULLSTR(arg))
	{
		send_to_char("Vote whom for which position?\n\r", ch);
		return FALSE;
	}

	if(is_online(arg)) online = TRUE;
	if(pc_in_char_list(arg)) in_char_list = TRUE;

	if((nominee = get_player(arg)) == NULL)
	{
		send_to_char("No such person.\n\r", ch);
		return FALSE;
	}

	if(!str_prefix(argument, "mayor"))
	{
		if(!is_nominee(nominee, 0))
		{
			send_to_char("No such nominee.\n\r", ch);
			return FALSE;
		}

		for(i = 0; i < 6; i++)
		{
			if(!str_cmp(vote_tally[0][i].name, nominee->name))
			{
				vote_tally[0][i].votes -= UMIN(fail,vote_tally[0][i].votes);
				break;
			}
		}
	}
	else if(!str_prefix(argument, "alderman"))
	{
		if(!is_nominee(nominee, 1))
		{
			send_to_char("No such nominee.\n\r", ch);
			return FALSE;
		}

		for(i = 0; i < 6; i++)
		{
			if(!str_cmp(vote_tally[1][i].name, nominee->name))
			{
				vote_tally[1][i].votes -= UMIN(fail,vote_tally[1][i].votes);
				break;
			}
		}
	}
	else if(!str_prefix(argument, "judge"))
	{
		if(!is_nominee(nominee, 2))
		{
			send_to_char("No such nominee.\n\r", ch);
			return FALSE;
		}

		for(i = 0; i < 6; i++)
		{
			if(!str_cmp(vote_tally[2][i].name, nominee->name))
			{
				vote_tally[2][i].votes -= UMIN(fail,vote_tally[2][i].votes);
				break;
			}
		}
	}
	else
	{
		send_to_char("Positions available are: judge, alderman, mayor.\n\r", ch);
		return FALSE;
	}

	if(fail > 0)
	{
		fwrite_votes();
		act(Format("\tGSuccess\tn: You manage to remove %d votes to $N's cause.\n\r", fail), ch, NULL, nominee, TO_CHAR, 1);
	}
	else if (fail < 0)
	{
		for(j = 0; j < 3; j++)
			for(i = 0; i < 6; i++)
			{
				if(!str_cmp(vote_tally[j][i].name, nominee->name))
				{
					vote_tally[j][i].votes -= fail;
					break;
				}
			}
		send_to_char("\tRBOTCH\tn: Your actions backfire.\n\r", ch);
		ch->influences[INFL_POLITICAL]--;
	}
	else
	{
		send_to_char("\tYFailure\tn: You fail to move people to vote.\n\r", ch);
	}

	if(!online && !in_char_list)
		free_char(nominee);
	ch->infl_timer = 4;

	return TRUE;
}

/* Police Influence */
int police_warrant(CHAR_DATA *ch, char *argument)
{
	int fail = dice_rolls(ch, get_curr_stat(ch, STAT_MAN) + ch->ability[LAW].value, 7);
	CHAR_DATA *victim;
	char buf[MSL]={'\0'};
	int UpOrDown = 0;

	if(IS_NULLSTR(argument))
	{
		send_to_char("Syntax: \tCinfluence warrant [increase/decrease] [target]\tn\n\r", ch);
		return FALSE;
	}

	argument = one_argument(argument, buf);

	if(!str_prefix(buf, "increase"))
		UpOrDown = 1;
	else if(!str_prefix(buf, "decrease"))
		UpOrDown = -1;
	else
	{
		send_to_char("Syntax: \tCinfluence warrant [increase/decrease] [target]\tn\n\r", ch);
		return FALSE;
	}

	if((victim = get_char_world(ch, argument)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return FALSE;
	}

	if(fail > 0)
	{
		act(Format("\tGSuccess\tn: $N's criminal record is altered.\n\r"), ch, NULL, victim, TO_CHAR, 1);
		victim->warrants += UpOrDown * fail * 5;
	}
	else if(fail == 0)
	{
		send_to_char("\tYFailure\tn: You fail to change the criminal records.\n\r", ch);
	}
	else
	{
		send_to_char("\tRBOTCH\tn: You offend a number of important people.\n\r", ch);
		ch->influences[INFL_POLICE]--;
	}

	ch->infl_timer = 1;
	return TRUE;
}


int police_apb(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	int fail = dice_rolls(ch, get_curr_stat(ch, STAT_MAN) + ch->ability[LAW].value, 7);

	if(IS_NULLSTR(argument))
	{
		send_to_char("Syntax: \tCinfluence apb [target]\tn\n\r", ch);
		return FALSE;
	}

	if((victim = get_char_world(ch, argument)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return FALSE;
	}

	if(fail > 0)
	{
		act("\tGSuccess\tn: $N will be arrested on sight.", ch, NULL, victim, TO_CHAR, 1);
		victim->warrants = 100;
	}
	else if(fail == 0)
	{
		act("\tYFailure\tn: You fail to convince the authorities to arrest $N.", ch, NULL, victim, TO_CHAR, 1);
	}
	else
	{
		send_to_char("\tRBOTCH\tn: You offend a number of important people.\n\r", ch);
		ch->influences[INFL_POLICE]--;
	}

	ch->infl_timer = 4;
	return TRUE;
}

/* Judicial Influence */
int judicial_sentence(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	char buf[MSL]={'\0'};
	int fail = dice_rolls(ch, get_curr_stat(ch, STAT_MAN) + ch->ability[LAW].value, 7);
	int UpOrDown = 0;

	if(IS_NULLSTR(argument))
	{
		send_to_char("Syntax: \tCinfluence sentence [increase/decrease] [target]\tn\n\r", ch);
		return FALSE;
	}

	argument = one_argument(argument, buf);

	if(!str_prefix(buf, "increase"))
		UpOrDown = 1;
	else if(!str_prefix(buf, "decrease"))
		UpOrDown = -1;
	else
	{
		send_to_char("Syntax: \tCinfluence sentence [increase/decrease] [target]\tn\n\r", ch);
		return FALSE;
	}

	if((victim = get_char_world(ch, argument)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return FALSE;
	}

	if(fail > 0)
	{
		act(Format("\tGSuccess\tn: $N's sentence is altered.\n\r"), ch, NULL, victim, TO_CHAR, 1);
		victim->warrants += UpOrDown * fail * 5;
	}
	else if(fail == 0)
	{
		send_to_char("\tYFailure\tn: You fail to change the sentence.\n\r", ch);
	}
	else
	{
		send_to_char("\tRBOTCH\tn: You offend a number of important people.\n\r", ch);
		ch->influences[INFL_JUDICIAL]--;
	}

	ch->infl_timer = 1;
	return TRUE;
}

int judicial_pardon(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	int fail = dice_rolls(ch, get_curr_stat(ch, STAT_MAN) + ch->ability[LAW].value, 7);

	if(IS_NULLSTR(argument))
	{
		send_to_char("Syntax: \tCinfluence pardon [target]\tn\n\r", ch);
		return FALSE;
	}

	if((victim = get_char_world(ch, argument)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return FALSE;
	}

	if(fail > 0)
	{
		act("\tGSuccess\tn: $N's is pardoned.\n\r", ch, NULL, victim, TO_CHAR, 1);
		act("$n arranges a pardon for you.\n\r", ch, NULL, victim, TO_VICT, 1);
		victim->warrants = 0;
	}
	else if(fail == 0)
	{
		send_to_char("\tYFailure\tn: You fail to change the sentence.\n\r", ch);
	}
	else
	{
		send_to_char("\tRBOTCH\tn: You offend a number of important people trying to obtain a pardon.\n\r",	ch);
		ch->influences[INFL_JUDICIAL]--;
	}

	ch->infl_timer = 4;
	return TRUE;
}


/* Media Influence */
int media_articles(CHAR_DATA *ch, char *argument)
{
	int fail = dice_rolls(ch, get_curr_stat(ch, STAT_MAN) + ch->ability[EXPRESSION].value, 7);

	if(fail > 0)
	{
		newspaper_articles(ch, "list");
	}
	else if(fail == 0)
	{
		send_to_char("\tYFailure\tn: Your wiles fail to allow you to read the articles.\n\r", ch);
	}
	else
	{
		send_to_char("\tRBOTCH\tn: You offend some members of the media.\n\r", ch);
		ch->influences[INFL_MEDIA]--;
	}

	ch->infl_timer = 1;
	return TRUE;
}

int media_suppress(CHAR_DATA *ch, char *argument)
{
	NOTE_DATA *pnote;
	int fail = dice_rolls(ch, get_curr_stat(ch, STAT_MAN) + ch->ability[EXPRESSION].value, 7);

	pnote = news_list;

	if(IS_NULLSTR(argument) || !is_number(argument))
	{
		send_to_char("Which article are you trying to suppress?\n\r", ch);
		return FALSE;
	}

	if(fail > 0)
	{
		int anum = 0;
		int vnum = 0;
		anum = atoi( argument );
		for ( pnote = news_list; pnote != NULL; pnote = pnote->next )
		{
			if ( is_note_to( ch, pnote ) && vnum++ == anum )
			{
				send_to_char(Format("\tGSuccess\tn: You arrange the suppression of '%s'.\n\r", pnote->subject), ch);
				pnote->successes += fail;
				ch->influences[INFL_MEDIA]--;

				/* Added this so it will force the saving of the suppression */
				save_notes(pnote->type);
				return TRUE;
			}
		}

		send_to_char("There aren't that many articles.\n\r",ch);
		return FALSE;
	}
	else if(fail == 0)
	{
		send_to_char("\tYFailure\tn: Your wiles fail to suppress the article.\n\r", ch);
	}
	else
	{
		send_to_char("\tRBOTCH\tn: You offend some members of the media.\n\r", ch);
		ch->influences[INFL_MEDIA]--;
	}

	ch->infl_timer = 2;
	return TRUE;
}

int media_promote(CHAR_DATA *ch, char *argument)
{
	NOTE_DATA *pnote;
	int fail = dice_rolls(ch, get_curr_stat(ch, STAT_MAN) + ch->ability[EXPRESSION].value, 7);

	if(IS_NULLSTR(argument) || !is_number(argument))
	{
		send_to_char("Which article are you trying to promote?\n\r", ch);
		return FALSE;
	}

	if(fail > 0)
	{
		int anum = 0;
		anum = atoi( argument );
		int vnum = 0;
		for ( pnote = news_list; pnote != NULL; pnote = pnote->next )
		{
			if ( is_note_to( ch, pnote ) && vnum++ == anum )
			{
				send_to_char(Format("\tGSuccess\tn: You arrange the release of '%s'.\n\r",	pnote->subject), ch);
				pnote->successes -= fail;
				if(pnote->successes < 0) pnote->successes = 0;
				ch->influences[INFL_MEDIA]--;
				/* Added this so it will force the saving of the suppression */
				save_notes(pnote->type);
				return TRUE;
			}
		}

		send_to_char("There aren't that many articles.\n\r",ch);
		return FALSE;
	}
	else if(fail == 0)
	{
		send_to_char("\tYFailure\tn: Your wiles fail to release the article.\n\r", ch);
	}
	else
	{
		send_to_char("\tRBOTCH\tn: You offend some members of the media.\n\r", ch);
		ch->influences[INFL_MEDIA]--;
	}

	ch->infl_timer = 3;
	return TRUE;
}

/* Economic Influence */
int economic_raise(CHAR_DATA *ch, char *argument)
{
	int success = 0;

	success = dice_rolls(ch, get_curr_stat(ch, STAT_CHA) + ch->ability[FINANCE].value, 7);

	if(success > 0)
	{
		success = number_fuzzy(success*1000);
		send_to_char(Format("\tGSuccess\tn: You manage to raise $%d.\n\r", success), ch);
		ch->dollars += success;
	}
	else if(success == 0)
	{
		send_to_char("\tYFailure\tn: You fail to raise any capital.\n\r", ch);
	}
	else
	{
		send_to_char("\tRBOTCH\tn: You offend some venture capitalist.\n\r", ch);
		ch->influences[INFL_ECONOMIC]--;
	}
	ch->infl_timer = 1;

	return TRUE;
}


int economic_market(CHAR_DATA *ch, char *argument)
{
	STOCKS *st;
	char buf[MSL]={'\0'};
	int success = 0;
	int UpOrDown = 0;

	success = dice_rolls(ch, get_curr_stat(ch, STAT_MAN) + ch->ability[FINANCE].value, 7);

	if(IS_NULLSTR(argument))
	{
		send_to_char("Syntax: \tCinfluence market [increase/decrease] [company]\tn\n\r", ch);
		return FALSE;
	}

	argument = one_argument(argument, buf);

	if(!str_prefix(buf, "increase"))
	{
		UpOrDown = 1;
	}
	else if(!str_prefix(buf, "decrease"))
	{
		UpOrDown = -1;
	}
	else
	{
		send_to_char("Syntax: \tCinfluence market [increase/decrease] [company]\tn\n\r", ch);
		return FALSE;
	}

	if((st = stock_lookup(argument)) == NULL)
	{
		send_to_char("There is no such company.\n\r", ch);
		return FALSE;
	}

	if(success > 0)
	{
		st->cost += number_fuzzy(UpOrDown * success * 10);
		st->upordown = UpOrDown;
		st->phase = 1;
		save_stocks();
		act(Format("\tGSuccess\tn: The value of a share of %s is now: $$%d.%.2d\n\r", st->name, st->cost/100?st->cost/100:0, st->cost%100), ch, NULL, NULL, TO_CHAR, 1);
	}
	else if(success == 0)
	{
		send_to_char("\tYFailure\tn: You fail to cause a shift in the market.\n\r", ch);
	}
	else
	{
		send_to_char("\tRBOTCH\tn: You damage your credibility in the market.\n\r", ch);
		ch->influences[INFL_ECONOMIC]--;
	}

	ch->infl_timer = 4;
	return TRUE;
}


int economic_trade(CHAR_DATA *ch, char *argument)
{
	STOCKS *st;
	char buf[MSL]={'\0'};
	int fail = dice_rolls(ch, get_curr_stat(ch, STAT_MAN) + ch->ability[FINANCE].value, 7);
	bool Buy = FALSE;

	if(IS_NULLSTR(argument))
	{
		send_to_char("Syntax: \tCinfluence trade [buy/sell] [shares] [company]\tn\n\r", ch);
		return FALSE;
	}

	argument = one_argument(argument, buf);

	if(!str_prefix(buf, "buy"))
	{
		Buy = TRUE;
	}
	else if(str_prefix(buf, "sell"))
	{
		send_to_char("Syntax: \tCinfluence trade [buy/sell] [shares] [company]\tn\n\r", ch);
		return FALSE;
	}

	argument = one_argument(argument, buf);
	if((st = stock_lookup(argument)) == NULL)
	{
		send_to_char("There is no such company.\n\r", ch);
		return FALSE;
	}

	if(IS_NULLSTR(buf) || !is_number(buf))
	{
		send_to_char("How many shares do you want to buy?\n\r", ch);
		return FALSE;
	}

	if(fail > 0)
	{
		send_to_char("\tGSuccess\tn: You manage to avoid paying trade fees.\n\r", ch);
		if(!trade_stocks(ch, st, atoi(buf), FALSE, Buy))
		{
			return FALSE;
		}
	}
	else if(fail == 0)
	{
		send_to_char("\tYFailure\tn: You fail to avoid the trade fees.\n\r", ch);
		if(!trade_stocks(ch, st, atoi(buf), TRUE, Buy))
		{
			return FALSE;
		}
	}
	else
	{
		send_to_char("\tRBOTCH\tn: You damage your credibility in the market.\n\r", ch);
		ch->influences[INFL_ECONOMIC]--;
		if(!trade_stocks(ch, st, atoi(buf), TRUE, Buy))
		{
			return FALSE;
		}
	}

	ch->infl_timer = 2;
	return TRUE;
}


/* Scientific Influence */
int scientific_materials(CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *obj;
	char arg[MIL]={'\0'};
	int fail = dice_rolls(ch, get_curr_stat(ch, STAT_CHA) + ch->ability[SCIENCE].value, 7);
	int material = 0;

	if(IS_NULLSTR(argument))
	{
		send_to_char("Syntax: \tCinfluence materials [material]\tn\n\r", ch);
		return FALSE;
	}

	argument = one_argument(argument, arg);

	if((material = material_lookup(arg)) <= 0)
	{
		send_to_char("There is no such material.\n\r", ch);
		return FALSE;
	}

	if(fail > 0)
	{
		int size = 0;
		size = UMIN(fail/2, 7);
		send_to_char("\tGSuccess\tn: You manage to acquire some of the material.\n\r", ch);

		if((obj = get_obj_carry( ch, (char *)Format("pounds %s", material_table[material].name), ch )) == NULL)
		{
			obj = create_object(get_obj_index(OBJ_VNUM_DUMMY));
			obj->weight = size_table[size].obj_weight;
			obj->cost = number_fuzzy(material_table[material].value * 100 * obj->weight);
			PURGE_DATA(obj->name);
			obj->name = str_dup(Format("%d pounds %s", obj->weight, material_table[material].name));
			PURGE_DATA(obj->short_descr);
			obj->short_descr = str_dup(Format("%d pounds of %s", obj->weight, material_table[material].name));
			PURGE_DATA(obj->description);
			obj->description = str_dup(Format("a quantity of a %s%s%s material sits here",
					material_table[material].colour, material_table[material].is_metal?", metallic":"", material_table[material].is_liquid?", liquid":""));
			PURGE_DATA(obj->full_desc);
			obj->full_desc = str_dup(Format("%d pounds of a %s%s%s material sits here",
					obj->weight, material_table[material].colour, material_table[material].is_metal?", metallic":"", material_table[material].is_liquid?", liquid":""));
			obj->wear_flags = ITEM_TAKE|ITEM_HOLD;
			PURGE_DATA(obj->material);
			obj->material = str_dup(Format("%s",material_table[material].name));
			obj_to_char(obj, ch);
		}
		else
		{
			obj->weight += size_table[size].obj_weight;
			obj->cost = number_fuzzy(material_table[material].value * 100
					* obj->weight);
			PURGE_DATA(obj->name);
			obj->name = str_dup(Format("%d pounds %s", obj->weight, material_table[material].name));
			PURGE_DATA(obj->short_descr);
			obj->short_descr = str_dup(Format("%d pounds of %s", obj->weight, material_table[material].name));
			PURGE_DATA(obj->description);
			obj->description = str_dup(Format("a quantity of a %s%s%s material sits here",
					material_table[material].colour, material_table[material].is_metal?", metallic":"", material_table[material].is_liquid?", liquid":""));
			PURGE_DATA(obj->full_desc);
			obj->full_desc = str_dup(Format("%d pounds of a %s%s%s material sits here", obj->weight, material_table[material].colour, material_table[material].is_metal?", metallic":"", material_table[material].is_liquid?", liquid":""));
		}
	}
	else if(fail == 0)
	{
		send_to_char("\tYFailure\tn: You fail to avoid the trade fees.\n\r", ch);
	}
	else
	{
		send_to_char("\tRBOTCH\tn: You damage your credibility in the market.\n\r", ch);
		ch->influences[INFL_SCIENTIFIC]--;
	}

	ch->infl_timer = 2;
	return TRUE;
}

int scientific_tipoff(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *vch;
	int successes = 0;
	int difficulty = 7;
	int power_stat = 0;
	int power_ability = 0;

	power_stat = get_curr_stat(ch, STAT_MAN);
	power_ability = ch->ability[SUBTERFUGE].value;

	successes = dice_rolls(ch, power_stat + power_ability, difficulty);

	if(IS_NULLSTR(argument))
	{
		send_to_char("Syntax: \tCinfluence document [target]\tn\n\r", ch);
		return FALSE;
	}

	if((vch = get_char_world(ch, argument)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return FALSE;
	}

	if(vch->hunter_vis > 0)
	{
		successes += vch->hunter_vis/10;
	}

	/* Fail conditions. */
	if(successes < 0)
	{
		send_to_char("Your story is openly ridiculed by your peers.\n\r", ch);
		ch->influences[INFL_CHURCH] -= 2;
		return FALSE;
	}
	else if(successes < 2)
	{
		send_to_char("Nobody believes your story.\n\r", ch);
		ch->influences[INFL_CHURCH]--;
		return FALSE;
	}

	/* Generate hunter and set on target. */
	send_to_char("The word is spread about that individual.\n\r", ch);
	gen_hunter(vch);
	ch->infl_timer = 5;
	ch->influences[INFL_SCIENTIFIC]--;
	return TRUE;
}


/*
 * Background command parser.
 */
void do_backgrounds (CHAR_DATA *ch, char *argument)
{
	char arg[MIL]={'\0'};
	int bg = 0, cmd = -1;

	if(IS_NULLSTR(argument))
	{
		send_to_char("What are you trying to do with your background?\n\r", ch);
		send_to_char("If you want to see the commands for backgrounds, use \t(background commands\t).\n\r", ch);
		return;
	}

	argument = one_argument(argument, arg);

	if((cmd = bg_cmd_lookup(arg)) > -1 )
	{
		bg = bg_cmd_table[cmd].type;
	}

	if(cmd <= -1 || ch->backgrounds[bg] < bg_cmd_table[cmd].level)
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if((*bg_cmd_table[cmd].func) ( ch, argument ))
	{
		WAIT_STATE(ch, bg_cmd_table[cmd].delay);
	}

	return;
}

int background_commands(CHAR_DATA *ch, char *argument)
{
	int cmd = 0;
	int col = 0;

	for( cmd = 0; bg_cmd_table[cmd].name != NULL; cmd++ )
	{
		int i = 0;
		i = bg_cmd_table[cmd].type;
		if(ch->backgrounds[i] <= bg_cmd_table[cmd].level)
		{
			send_to_char(Format("\t<send href='background %s'>%-11s\t</send> | ", bg_cmd_table[cmd].name, bg_cmd_table[cmd].name), ch);
			if(++col % 4 == 0)
			{
				send_to_char("\n\r", ch);
			}
		}
	}

	if(col %6 != 0)
	{
		send_to_char("\n\r", ch);
	}
	return TRUE;
}

int background_advance(CHAR_DATA *ch, char *argument)
{
	int max_stat = 5;
	int in = 0;
	int cost = 0;

	if(IS_NPC(ch))
	{
		return FALSE;
	}

	if(IS_NULLSTR(argument))
	{
		send_to_char("Which background do you wish to advance?\n\r", ch);
		return FALSE;
	}

	if((in = background_lookup(argument)) == -1 )
	{
		send_to_char("There is no background of that name.\n\r", ch);
		return FALSE;
	}

	if(ch->bg_timer <= 0 && background_table[in].settable == FALSE)
	{
		send_to_char("You cannot advance that background at this time.\n\r",ch);
		return FALSE;
	}

	if(ch->backgrounds[in] >= max_stat)
	{
		send_to_char("That background is already maxed out.\n\r", ch);
		return FALSE;
	}

	if(ch->backgrounds[in] == 0)
	{
		cost = 10;
	}
	else
	{
		cost = ch->backgrounds[in] * 8;
	}

	cost = xp_cost_mod(ch, cost, ch->backgrounds[in]);
	if(ch->bg_count < 5)
	{
		cost = 0;
	}
	if(ch->exp < cost)
	{
		send_to_char("You don't have enough experience.\n\r", ch);
		return FALSE;
	}

	if(in == GENERATION)
	{
		if(ch->gen < 13 && 13-ch->gen > ch->backgrounds[in])
		{
			send_to_char("You must have been embraced into a lower generation.\n\r", ch);
			send_to_char("Please see a staff member for assistance.\n\r", ch);
			return FALSE;
		}

		ch->gen--;
		if(ch->race == race_lookup("vampire"))
		{
			ch->max_RBPG = 23 - ch->gen;
			if(ch->max_RBPG < 10) ch->max_RBPG = 10;
		}
	}

	ch->exp -= cost;
	ch->backgrounds[in]++;
	ch->bg_timer--;
	ch->bg_count++;
	act("Your $t increases.", ch, background_table[in].name, NULL, TO_CHAR, 1);

	return TRUE;
}

int background_herd(CHAR_DATA *ch, char *argument)
{
	int i = 0;
	int successes = 0;
	int difficulty = 6;

	if(IS_NULLSTR(argument) || !is_number(argument))
	{
		send_to_char("Syntax: \tCbackground herd [amount of blood to draw upon]\tn\n\r", ch);
		send_to_char("How much of your herd do you wish to draw on?\n\r", ch);
		return FALSE;
	}

	if(ch->herd_timer > 0)
	{
		send_to_char("\tOYour herd has not recovered from your last feeding.\tn\n\r", ch);
		return FALSE;
	}

	if(ch->backgrounds[HERD] < (i = atoi(argument)))
	{
		send_to_char("\tOYou do not have that much herd to draw on.\tn\n\r", ch);
		return FALSE;
	}

	successes = dice_rolls(ch, ch->backgrounds[HERD], difficulty);

	if(successes < 0)
	{
		send_to_char("\tRBOTCH\tn: You unsettle your herd, losing some members.\n\r", ch);
		ch->backgrounds[HERD]--;
	}
	else if(successes == 0)
	{
		send_to_char("\tYFailure\tn: You do not manage to get in touch with your herd.\n\r", ch);
	}
	else
	{
		ch->RBPG += UMIN(ch->max_RBPG - ch->RBPG, successes);
	}
	ch->herd_timer = i * 4;
	return TRUE;
}

/* >>>>>>>>>>>>>>> Newspaper Commands <<<<<<<<<<<<<<< */
int newspaper_commands(CHAR_DATA *ch, char *argument)
{
	int cmd = 0;
	int col = 0;

	send_to_char("Newspaper Commands.\n\rSyntax:newspaper <command> <arguments>\n\r", ch);
	for( cmd = 0; news_cmd_table[cmd].name != NULL; cmd++ )
	{
		if(ch->trust >= news_cmd_table[cmd].level)
		{
			send_to_char(Format("%-12s | ", news_cmd_table[cmd].name), ch);
			if(++col % 4 == 0)
			{
				send_to_char("\n\r", ch);
			}
		}
	}

	if(col %4 != 0)
	{
		send_to_char("\n\r", ch);
	}
	return TRUE;
}

int newspaper_new(CHAR_DATA *ch, char *argument)
{
	NEWSPAPER *news = new_newspaper();
	char arg[MIL]={'\0'};

	if(IS_NULLSTR(argument))
	{
		send_to_char("Syntax: \tCnewspaper create [cost_in_cents] [name]\tn", ch);
		return FALSE;
	}

	argument = one_argument(argument, arg);
	if(!is_number(arg))
	{
		send_to_char("Syntax: \tCnewspaper create [cost_in_cents] [name]\tn", ch);
		return FALSE;
	}

	news->cost = atoi(arg);
	news->name = str_dup(argument);
	news->next = paper_list;
	paper_list = news;
	send_to_char(Format("Paper '%s' created.\n\r", news->name), ch);
	return TRUE;
}

int newspaper_list(CHAR_DATA *ch, char *argument)
{
	NEWSPAPER *news;
	int on_stands = -1;
	int count = 0;

	if(!str_cmp(argument, "out now") || !str_cmp(argument, "on stands"))
	{
		on_stands = 1;
	}
	else if(!str_cmp(argument, "off stands"))
	{
		on_stands = 0;
	}

	send_to_char("\tWNumber | Name            | Price  | Stands\tn\n\r", ch);
	for(news = paper_list; news; news = news->next)
	{
		if(on_stands == -1 || news->on_stands == on_stands)
		{
			send_to_char(Format("[%4d] | %-15s | $%2d.%.2d | [%s]\n\r", count, news->name, news->cost/100, news->cost%100, news->on_stands?"On Stands":"Off Stands"), ch);
		}
		count++;
	}

	return TRUE;
}

int newspaper_clear (CHAR_DATA *ch, char *arg)
{
	NEWSPAPER *paper = NULL;
	int i = 0;

	if(IS_NULLSTR(arg))
	{
		send_to_char("Which newspaper do you wish to clear?\n\r", ch);
		return FALSE;
	}

	if(!is_number(arg) && (paper = newspaper_lookup(arg)) == NULL)
	{
		send_to_char("That newspaper doesn't exist.\n\r", ch);
		return FALSE;
	}

	if(paper == NULL)
	{
		paper = paper_list;
		for(i=0;i==atoi(arg);i++)
		{
			paper = paper->next;
		}
	}

	if(paper == NULL)
	{
		send_to_char("That newspaper doesn't exist.\n\r", ch);
		return FALSE;
	}

	if(paper->on_stands)
	{
		send_to_char("You cannot clear a paper that is on stands.\n\r", ch);
		return FALSE;
	}

	for(i=0; i < MAX_ARTICLES; i++)
	{
		paper->articles[i] = -1;
	}
	send_to_char("Articles cleared.\n\r", ch);
	return TRUE;
}

int newspaper_show (CHAR_DATA *ch, char *arg)
{
	NEWSPAPER *paper = NULL;
	NEWSPAPER *tmp;
	NOTE_DATA *article = NULL;
	int i = 0,j = 0;

	if(IS_NULLSTR(arg))
	{
		send_to_char("Which newspaper do you wish to show?\n\r", ch);
		return FALSE;
	}

	if(!is_number(arg) && (paper = newspaper_lookup(arg)) == NULL)
	{
		send_to_char("That newspaper doesn't exist.\n\r", ch);
		return FALSE;
	}

	if(paper == NULL)
	{
		paper = paper_list;
		for(i=0;i<atoi(arg);i++)
		{
			paper = paper->next;
		}
	}

	if(paper == NULL)
	{
		send_to_char("That newspaper doesn't exist.\n\r", ch);
		return FALSE;
	}

	if(is_number(arg))
	{
		i = atoi(arg);
	}
	else
	{
		i=0;
		for(tmp=paper_list;tmp;tmp=tmp->next)
		{
			if(tmp == paper) break;
			i++;
		}
	}

	send_to_char(Format("[%4d] %-15s $%d.%.2d [%s]\n\r", i, paper->name, paper->cost/100, paper->cost%100, paper->on_stands?"On Stands":"Off Stands"), ch);

	for(i=0;i<MAX_ARTICLES;i++)
	{
		if(paper->articles[i] == -1)
		{
			send_to_char(Format("%d: None\n\r", i), ch);
			continue;
		}

		for(article=news_list;article;article=article->next)
		{
			if(j == paper->articles[i])
			{
				send_to_char(Format("%d: [%3d] %s: %s (%s)%s\n\r", i, j, article->sender, article->subject, article->to_list,
						article->successes ? " (\tYSuppressed\tn)" : ""), ch);
				break;
			}
			j++;
		}
	}

	return TRUE;
}

int newspaper_articles(CHAR_DATA *ch, char *arg)
{

	parse_note(ch,arg,NOTE_ARTICLE);

	return TRUE;
}

int newspaper_delete (CHAR_DATA *ch, char *arg)
{
	NEWSPAPER *paper = NULL;
	NEWSPAPER *tmp;

	if(IS_NULLSTR(arg))
	{
		send_to_char("Which newspaper do you wish to delete?\n\r", ch);
		return FALSE;
	}

	if(!is_number(arg) && (paper = newspaper_lookup(arg)) == NULL)
	{
		send_to_char("That newspaper doesn't exist.\n\r", ch);
		return FALSE;
	}

	if(paper == NULL)
	{
		int i = 0;
		paper = paper_list;
		for(i=0;i<atoi(arg);i++)
		{
			paper = paper->next;
		}
	}

	if(paper == NULL)
	{
		send_to_char("That newspaper doesn't exist.\n\r", ch);
		return FALSE;
	}

	if(paper->on_stands)
	{
		send_to_char("You cannot delete a paper that is on stands.\n\r", ch);
		return FALSE;
	}

	if(paper_list == paper)
	{
		paper_list = paper->next;
	}
	else
	{
		for(tmp=paper_list;tmp->next==paper;tmp=tmp->next)
		{
			tmp->next = paper->next;
		}
	}
	free_newspaper(paper);
	send_to_char("Newspaper deleted.\n\r", ch);

	return TRUE;
}

int newspaper_rename(CHAR_DATA *ch, char *argument)
{
	NEWSPAPER *paper = NULL;
	char arg[MIL]={'\0'};

	if(IS_NULLSTR(argument))
	{
		send_to_char("Which newspaper do you wish to rename?\n\r", ch);
		return FALSE;
	}

	argument = one_argument(argument, arg);

	if(!is_number(arg) && (paper = newspaper_lookup(arg)) == NULL)
	{
		send_to_char("That newspaper doesn't exist.\n\r", ch);
		return FALSE;
	}

	if(paper == NULL)
	{
		int i = 0;
		paper = paper_list;
		for(i=0;i<atoi(arg);i++)
		{
			paper = paper->next;
		}
	}

	if(paper == NULL)
	{
		send_to_char("That newspaper doesn't exist.\n\r", ch);
		return FALSE;
	}

	if(paper->on_stands)
	{
		send_to_char("You cannot rename a paper that is on stands.\n\r", ch);
		return FALSE;
	}

	send_to_char(Format("%s renamed to %s.\n\r", paper->name, argument), ch);

	PURGE_DATA(paper->name);
	paper->name = str_dup(argument);

	return TRUE;
}

int newspaper_price(CHAR_DATA *ch, char *argument)
{
	NEWSPAPER *paper = NULL;
	char arg[MIL]={'\0'};

	if(IS_NULLSTR(argument))
	{
		send_to_char("Which newspaper do you wish to set the price of?\n\r", ch);
		return FALSE;
	}

	argument = one_argument(argument, arg);

	if(!is_number(arg) && (paper = newspaper_lookup(arg)) == NULL)
	{
		send_to_char("That newspaper doesn't exist.\n\r", ch);
		return FALSE;
	}

	if(paper == NULL)
	{
		int i = 0;
		paper = paper_list;
		for(i=0;i<atoi(arg);i++)
		{
			paper = paper->next;
		}
	}

	if(paper == NULL)
	{
		send_to_char("That newspaper doesn't exist.\n\r", ch);
		return FALSE;
	}

	if(paper->on_stands)
	{
		send_to_char("You cannot price a paper that is on stands.\n\r", ch);
		return FALSE;
	}

	if(IS_NULLSTR(argument))
	{
		send_to_char(Format("The cost of an issue of %s is $%d.%d.\n\r", paper->name, paper->cost/100, paper->cost%100), ch);
		return TRUE;
	}

	if(!is_number(argument))
	{
		send_to_char("You must supply a number for the cost.\n\r", ch);
		return FALSE;
	}

	paper->cost = atoi(argument);
	send_to_char(Format("The price of a copy of %s set to $%d.%d.\n\r", paper->name, paper->cost/100, paper->cost%100), ch);

	return TRUE;
}

int newspaper_place(CHAR_DATA *ch, char *argument)
{
	NOTE_DATA *pnote;
	NEWSPAPER *paper = NULL;
	char arg[MIL]={'\0'};
	char position[MIL]={'\0'};
	char article[MIL]={'\0'};
	int anum = 0, vnum = 0;

	if(IS_NULLSTR(argument))
	{
		send_to_char("Which newspaper do you wish to arrange?\n\r", ch);
		return FALSE;
	}

	argument = one_argument(argument, arg);
	argument = one_argument(argument, position);
	argument = one_argument(argument, article);

	if(!is_number(arg) && (paper = newspaper_lookup(arg)) == NULL)
	{
		send_to_char("That newspaper doesn't exist.\n\r", ch);
		return FALSE;
	}

	if(paper == NULL)
	{
		int i = 0;
		paper = paper_list;
		for(i=0;i<atoi(arg);i++)
		{
			paper = paper->next;
		}
	}

	if(paper == NULL)
	{
		send_to_char("That newspaper doesn't exist.\n\r", ch);
		return FALSE;
	}

	if(paper->on_stands)
	{
		send_to_char("You cannot rearrange a paper that is on stands.\n\r", ch);
		return FALSE;
	}

	if(IS_NULLSTR(article) || IS_NULLSTR(position) || !is_number(article) || !is_number(position))
	{
		send_to_char("Syntax: newspaper place <newspaper> <position> <article>\n\r", ch);
		return FALSE;
	}

	/*test for position*/
	if(atoi(position) < 0 || atoi(position) > MAX_ARTICLES)
	{
		send_to_char(Format("The target position must be between 0 and %d.\n\r", MAX_ARTICLES), ch);
		return FALSE;
	}

	anum = atoi( article );

	for ( pnote = news_list; pnote != NULL; pnote = pnote->next )
	{
		if ( vnum++ == anum )
		{
			send_to_char(Format("Article %d in %s is now %s by %s.\n\r", atoi(position), paper->name, pnote->subject, pnote->sender), ch);
			paper->articles[atoi(position)] = anum;
			return TRUE;
		}
	}

	send_to_char("There aren't that many articles.\n\r",ch);

	return FALSE;
}

int newspaper_save(CHAR_DATA *ch, char *arg)
{

	save_papers();
	send_to_char("Papers saved.\n\r", ch);
	return TRUE;
}

int newspaper_start_stop(CHAR_DATA *ch, char *argument, int on_stands)
{
	NEWSPAPER *paper = NULL;
	char arg[MIL]={'\0'};

	if(IS_NULLSTR(argument))
	{
		send_to_char("Which newspaper do you wish to mess with?\n\r", ch);
		return FALSE;
	}

	argument = one_argument(argument, arg);

	if(!is_number(arg) && (paper = newspaper_lookup(arg)) == NULL)
	{
		send_to_char("That newspaper doesn't exist.\n\r", ch);
		return FALSE;
	}

	if(paper == NULL)
	{
		int i = 0;
		paper = paper_list;
		for(i=0;i<atoi(arg);i++)
		{
			paper = paper->next;
		}
	}

	if(paper == NULL)
	{
		send_to_char("That newspaper doesn't exist.\n\r", ch);
		return FALSE;
	}

	if(on_stands == paper->on_stands)
	{
		send_to_char(Format("%s is already %s stands.\n\r", paper->name, on_stands?"on":"off"), ch);
		return FALSE;
	}

	if(paper->on_stands == 0)
	{
		paper->on_stands = 1;
		send_to_char(Format("%s has been placed on stands.\n\r", paper->name), ch);
	}
	else
	{
		paper->on_stands = 0;
		send_to_char(Format("%s has been removed from stands.\n\r", paper->name), ch);
	}

	update_news_stands();

	return TRUE;
}

int newspaper_stop(CHAR_DATA *ch, char *argument)
{
	return newspaper_start_stop(ch, argument, 0);
}

int newspaper_release(CHAR_DATA *ch, char *argument)
{
	return newspaper_start_stop(ch, argument, 1);
}

/*
 * Stockmarket Subcommands. (Command parser in act_wiz.c)
 */
int smarket_commands(CHAR_DATA *ch, char *argument)
{
	int cmd = 0;
	int col = 0;

	send_to_char("Stock Market Commands.\n\rSyntax: \tCsmarket [command] [arguments]\tn\n\r", ch);
	for( cmd = 0; smarket_cmd_table[cmd].name != NULL; cmd++ )
	{
		if(ch->trust >= smarket_cmd_table[cmd].level)
		{
			send_to_char(Format("%-12s | ", smarket_cmd_table[cmd].name), ch);
			if(++col % 4 == 0)
				{
					send_to_char("\n\r", ch);
				}
		}
	}

	if(col %6 != 0)
		{
			send_to_char("\n\r", ch);
		}
	return TRUE;
}

int smarket_create(CHAR_DATA *ch, char *argument)
{
	STOCKS *stock = new_stock();
	char arg[MIL]={'\0'};
	char arg2[MIL]={'\0'};

	if(IS_NULLSTR(argument))
	{
		send_to_char("Syntax: \tCsmarket create [cost_in_cents] [ticker tag] [name]\tn\n\r", ch);
		return FALSE;
	}

	argument = one_argument(argument, arg);
	if(!is_number(arg))
	{
		send_to_char("Syntax: \tCsmarket create [cost_in_cents] [ticker tag] [name]\tn\n\r", ch);
		return FALSE;
	}

	argument = one_argument(argument, arg2);
	if(strlen(arg2) > 4)
	{
		send_to_char("A ticker tag cannot have more than four characters.\n\r",	ch);
		return FALSE;
	}

	stock->cost = atoi(arg);
	stock->name = str_dup(argument);
	stock->ticker = str_dup(arg2);
	stock->next = stock_list;
	stock_list = stock;
	send_to_char(Format("Company '%s' created.\n\r", stock->name), ch);
	return TRUE;
}

int smarket_list(CHAR_DATA *ch, char *argument)
{
	STOCKS *stock;
	int count = 0;

	for(stock = stock_list; stock; stock = stock->next)
	{
		send_to_char(Format("[%4d] %-25s $%d.%.2d\n\r", count, stock->name, stock->cost/100, (stock->cost - (stock->cost/100)*100)), ch);
		count++;
	}

	return TRUE;
}

int smarket_show (CHAR_DATA *ch, char *arg)
{
	STOCKS *stock = NULL;
	STOCKS *tmp;
	int i = 0;
	int count = 0;

	if(IS_NULLSTR(arg))
	{
		send_to_char("Which stock do you wish to see the details of?\n\r", ch);
		return FALSE;
	}

	if(!is_number(arg) && (stock = stock_lookup(arg)) == NULL)
	{
		send_to_char("That company doesn't exist.\n\r", ch);
		return FALSE;
	}

	if(stock == NULL)
	{
		stock = stock_list;
		for(i=0;i==atoi(arg);i++)
		{
			stock = stock->next;
		}
	}

	if(stock == NULL)
	{
		send_to_char("That company doesn't exist.\n\r", ch);
		return FALSE;
	}

	if(is_number(arg))
	{
		i = atoi(arg);
	}
	else
	{
		i=0;
		for(tmp=stock_list;tmp;tmp=tmp->next)
		{
			if(tmp == stock) break;
			i++;
		}
	}
	send_to_char(Format("[%4d] %-25s $%d.%.2d\n\r", count, stock->name, stock->cost/100, (stock->cost - (stock->cost/100)*100)), ch);

	return TRUE;
}

int smarket_delete (CHAR_DATA *ch, char *arg)
{
	STOCKS *stock = NULL;
	STOCKS *tmp;

	if(IS_NULLSTR(arg))
	{
		send_to_char("Which company do you wish to delete?\n\r", ch);
		return FALSE;
	}

	if(!is_number(arg) && (stock = stock_lookup(arg)) == NULL)
	{
		send_to_char("That company doesn't exist.\n\r", ch);
		return FALSE;
	}

	if(stock == NULL)
	{
		int i = 0;
		stock = stock_list;
		for(i=0;i==atoi(arg);i++)
		{
			stock = stock->next;
		}
	}

	if(stock == NULL)
	{
		send_to_char("That company doesn't exist.\n\r", ch);
		return FALSE;
	}

	if(stock_list == stock)
	{
		stock_list = stock->next;
	}
	else
	{
		for( tmp = stock_list ; tmp ; tmp = tmp->next )
			{     if( tmp->next == stock )
			              tmp->next = stock->next;
			}
	}
	free_stock(stock);
	send_to_char("Stock deleted.\n\r", ch);

	return TRUE;
}

int smarket_rename(CHAR_DATA *ch, char *argument)
{
	STOCKS *stock = NULL;
	char arg[MIL]={'\0'};

	if(IS_NULLSTR(argument))
	{
		send_to_char("Which company do you wish to rename?\n\r", ch);
		return FALSE;
	}

	argument = one_argument(argument, arg);

	if(!is_number(arg) && (stock = stock_lookup(arg)) == NULL)
	{
		send_to_char("That company doesn't exist.\n\r", ch);
		return FALSE;
	}

	if(stock == NULL)
	{
		int i = 0;
		stock = stock_list;
		for(i=0;i==atoi(arg);i++)
		{
			stock = stock->next;
		}
	}

	if(stock == NULL)
	{
		send_to_char("That company doesn't exist.\n\r", ch);
		return FALSE;
	}

	if(IS_NULLSTR(argument))
	{
		send_to_char("What do you want to rename it to?\n\r", ch);
		return FALSE;
	}

	send_to_char(Format("%s renamed to %s.\n\r", stock->name, argument), ch);

	PURGE_DATA(stock->name);
	stock->name = str_dup(argument);

	return TRUE;
}

int smarket_ticker(CHAR_DATA *ch, char *argument)
{
	STOCKS *stock = NULL;
	char arg[MIL]={'\0'};

	if(IS_NULLSTR(argument))
	{
		send_to_char("Which company do you wish to set the ticker tag for?\n\r", ch);
		return FALSE;
	}

	argument = one_argument(argument, arg);

	if(!is_number(arg) && (stock = stock_lookup(arg)) == NULL)
	{
		send_to_char("That company doesn't exist.\n\r", ch);
		return FALSE;
	}

	if(stock == NULL)
	{
		int i = 0;
		stock = stock_list;
		for(i=0;i==atoi(arg);i++)
		{
			stock = stock->next;
		}
	}

	if(stock == NULL)
	{
		send_to_char("That company doesn't exist.\n\r", ch);
		return FALSE;
	}

	if(IS_NULLSTR(argument))
	{
		send_to_char("What do you want to change the ticker to?\n\r", ch);
		return FALSE;
	}

	if(strlen(argument) > 4)
	{
		send_to_char("A ticker tag cannot have more than four characters.\n\r", ch);
		return FALSE;
	}

	send_to_char(Format("%s ticker renamed to %s.\n\r", stock->name, argument), ch);

	PURGE_DATA(stock->ticker);
	stock->ticker = str_dup(argument);

	return TRUE;
}

int smarket_price(CHAR_DATA *ch, char *argument)
{
	STOCKS *stock = NULL;
	char arg[MIL]={'\0'};

	if(IS_NULLSTR(argument))
	{
		send_to_char("Which stock do you wish to set the price of?\n\r", ch);
		return FALSE;
	}

	argument = one_argument(argument, arg);

	if(!is_number(arg) && (stock = stock_lookup(arg)) == NULL)
	{
		send_to_char("That company doesn't exist.\n\r", ch);
		return FALSE;
	}

	if(stock == NULL)
	{
		int i = 0;
		stock = stock_list;
		for(i=0;i==atoi(arg);i++)
		{
			stock = stock->next;
		}
	}

	if(stock == NULL)
	{
		send_to_char("That stock doesn't exist.\n\r", ch);
		return FALSE;
	}

	if(IS_NULLSTR(argument))
	{
		send_to_char(Format("The cost of a share of %s is $%d.%.2d.\n\r", stock->name, stock->cost/100, stock->cost%100), ch);
		return TRUE;
	}

	if(!is_number(argument))
	{
		send_to_char("You must supply a number for the cost.\n\r", ch);
		return FALSE;
	}

	stock->cost = atoi(argument);
	send_to_char(Format("The price of a share of %s set to $%d.%.2d.\n\r", stock->name, stock->cost/100, stock->cost%100), ch);

	return TRUE;
}

int smarket_save(CHAR_DATA *ch, char *arg)
{
	save_stocks();
	send_to_char("Stock market saved.\n\r", ch);
	return TRUE;
}


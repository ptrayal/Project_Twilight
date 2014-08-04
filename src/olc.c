/***************************************************************************
 *  File: olc.c                                                            *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/



#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include "twilight.h"
#include "tables.h"
#include "olc.h"
#include "recycle.h"
#include "lookup.h"


DECLARE_DO_FUN( do_asave	);
DECLARE_DO_FUN( do_look		);
void do_function args((CHAR_DATA *ch, DO_FUN *do_fun, char *argument));

/*
 * Local functions.
 */
AREA_DATA *get_area_data	args( ( int vnum ) );
PLOT_DATA *get_plot_index	args( ( int vnum ) );
EVENT_DATA *get_event_index	args( ( int vnum ) );
SCRIPT_DATA *get_script_index	args( ( int vnum ) );
PERSONA *get_persona_index	args( ( int vnum ) );
REACT *get_react_index		args( ( int x, int vnum ) );
void save_notes			args( (int type) );

/* Executed from comm.c.  Minimizes compiling when changes are made. */
bool run_olc_editor( DESCRIPTOR_DATA *d )
{
	switch ( d->editor )
	{
	case ED_AREA:
	aedit( d->character, d->incomm );
	break;
	case ED_ROOM:
	redit( d->character, d->incomm );
	break;
	case ED_OBJECT:
	oedit( d->character, d->incomm );
	break;
	case ED_MOBILE:
	medit( d->character, d->incomm );
	break;
	case ED_PLOT:
	pedit( d->character, d->incomm );
	break;
	case ED_EVENT:
	eedit( d->character, d->incomm );
	break;
	case ED_SCRIPT:
	sedit( d->character, d->incomm );
	break;
	case ED_PERSONA:
	aiedit( d->character, d->incomm );
	break;
	case ED_REACT:
	rsedit( d->character, d->incomm );
	break;
	case ED_HELP:
	hedit( d->character, d->incomm, 0 );
	break;
	case ED_KNOW:
	kbedit( d->character, d->incomm, NOTE_KNOWLEDGE );
	break;
	case ED_BG:
	kbedit( d->character, d->incomm, NOTE_BACKGROUND );
	break;
	case ED_TIP:
	hedit( d->character, d->incomm, 1 );
	break;
	case ED_MPCODE:
	mpedit( d->character, d->incomm );
	break;
	default:
	return FALSE;
	}
	return TRUE;
}



char *olc_ed_name( CHAR_DATA *ch )
{
	static char buf[10]={'\0'};

	if(ch->desc == NULL)
	{
		send_to_char(" ", ch);
		return buf;
	}

	buf[0] = '\0';
	switch (ch->desc->editor)
	{
	case ED_AREA:
		send_to_char("AEdit", ch);
		break;
	case ED_ROOM:
		send_to_char("REdit", ch);
		break;
	case ED_OBJECT:
		send_to_char("OEdit", ch);
		break;
	case ED_MOBILE:
		send_to_char("MEdit", ch);
		break;
	case ED_PLOT:
		send_to_char("PEdit", ch);
		break;
	case ED_EVENT:
		send_to_char("EEdit", ch);
		break;
	case ED_SCRIPT:
		send_to_char("SEdit", ch);
		break;
	case ED_PERSONA:
		send_to_char("AIEdit", ch);
		break;
	case ED_REACT:
		send_to_char("RSEdit", ch);
		break;
	case ED_HELP:
		send_to_char("HEdit", ch);
		break;
	case ED_KNOW:
		send_to_char("Knowledges", ch);
		break;
	case ED_BG:
		send_to_char("Background", ch);
		break;
	case ED_MPCODE:
		send_to_char("MPEdit", ch);
		break;
	default:
		send_to_char(" ", ch);
		break;
	}
	return buf;
}



char *olc_ed_vnum( CHAR_DATA *ch )
{
	AREA_DATA *pArea;
	ROOM_INDEX_DATA *pRoom;
	OBJ_INDEX_DATA *pObj;
	MOB_INDEX_DATA *pMob;
	HELP_DATA *pHelp;
	PLOT_DATA *pPlot;
	EVENT_DATA *pEvent;
	SCRIPT_DATA *pScript;
	PERSONA *pPersona;
	REACT *pMatrix;
	NOTE_DATA *pKnow;
	MPROG_CODE *pMprog;
	static char buf[10]={'\0'};

	buf[0] = '\0';
	switch ( ch->desc->editor )
	{
	case ED_AREA:
		pArea = (AREA_DATA *)ch->desc->pEdit;
		send_to_char(Format("%d", pArea ? pArea->vnum : 0), ch);
		break;
	case ED_ROOM:
		pRoom = ch->in_room;
		send_to_char(Format("%d", pRoom ? pRoom->vnum : 0), ch);
		break;
	case ED_OBJECT:
		pObj = (OBJ_INDEX_DATA *)ch->desc->pEdit;
		send_to_char(Format("%d", pObj ? pObj->vnum : 0), ch);
		break;
	case ED_MOBILE:
		pMob = (MOB_INDEX_DATA *)ch->desc->pEdit;
		send_to_char(Format("%d", pMob ? pMob->vnum : 0), ch);
		break;
	case ED_PLOT:
		pPlot = (PLOT_DATA *)ch->desc->pEdit;
		send_to_char(Format("%d", pPlot ? pPlot->vnum : 0), ch);
		break;
	case ED_EVENT:
		pEvent = (EVENT_DATA *)ch->desc->pEdit;
		send_to_char(Format("%d", pEvent ? pEvent->vnum : 0), ch);
		break;
	case ED_SCRIPT:
		pScript = (SCRIPT_DATA *)ch->desc->pEdit;
		send_to_char(Format("%d", pScript ? pScript->vnum : 0), ch);
		break;
	case ED_PERSONA:
		pPersona = (PERSONA *)ch->desc->pEdit;
		send_to_char(Format("%d", pPersona ? pPersona->vnum : 0), ch);
		break;
	case ED_REACT:
		pMatrix = (REACT *)ch->desc->pEdit;
		send_to_char(Format("%d", pMatrix ? pMatrix->attitude : 0), ch);
		break;
	case ED_HELP:
		pHelp = (HELP_DATA *)ch->desc->pEdit;
		send_to_char(Format("%s", pHelp ? pHelp->keyword : "(null)"), ch);
		break;
	case ED_KNOW:
		pKnow = (NOTE_DATA *)ch->desc->pEdit;
		send_to_char(Format("%s", pKnow ? pKnow->to_list : "(null)"), ch);
		break;
	case ED_BG:
		pKnow = (NOTE_DATA *)ch->desc->pEdit;
		send_to_char(Format("%s", pKnow ? pKnow->to_list : "(null)"), ch);
		break;
	case ED_MPCODE:
		pMprog = (MPROG_CODE *)ch->desc->pEdit;
		send_to_char(Format("%d", pMprog ? pMprog->vnum : 0), ch);
		break;
	default:
		send_to_char(" ", ch);
		break;
	}

	return buf;
}



/*****************************************************************************
 Name:		show_olc_cmds
 Purpose:	Format up the commands from given table.
 Called by:	show_commands(olc_act.c).
 ****************************************************************************/
void show_olc_cmds( CHAR_DATA *ch, const struct olc_cmd_type *olc_table )
{
	char buf  [ MSL ]={'\0'};
	char buf1 [ MSL ]={'\0'};
	int  cmd;
	int  col;

	buf1[0] = '\0';
	col = 0;
	for (cmd = 0; olc_table[cmd].name != NULL; cmd++)
	{
		send_to_char(Format("%-15.15s", olc_table[cmd].name), ch);
		strncat( buf1, buf, sizeof(buf1) );
		if ( ++col % 5 == 0 )
			strncat( buf1, "\n\r", sizeof(buf1) );
	}

	if ( col % 5 != 0 )
		strncat( buf1, "\n\r", sizeof(buf1) );

	send_to_char( buf1, ch );
	return;
}



/*****************************************************************************
 Name:		show_commands
 Purpose:	Display all olc commands.
 Called by:	olc interpreters.
 ****************************************************************************/
bool show_commands( CHAR_DATA *ch, char *argument )
{
	switch (ch->desc->editor)
	{
	case ED_AREA:
		show_olc_cmds( ch, aedit_table );
		break;
	case ED_ROOM:
		show_olc_cmds( ch, redit_table );
		break;
	case ED_OBJECT:
		show_olc_cmds( ch, oedit_table );
		break;
	case ED_MOBILE:
		show_olc_cmds( ch, medit_table );
		break;
	case ED_PLOT:
		show_olc_cmds( ch, pedit_table );
		break;
	case ED_EVENT:
		show_olc_cmds( ch, eedit_table );
		break;
	case ED_SCRIPT:
		show_olc_cmds( ch, sedit_table );
		break;
	case ED_PERSONA:
		show_olc_cmds( ch, aiedit_table );
		break;
	case ED_REACT:
		show_olc_cmds( ch, rsedit_table );
		break;
	case ED_HELP:
		show_olc_cmds( ch, hedit_table );
		break;
	case ED_KNOW:
		show_olc_cmds( ch, kbedit_table );
		break;
	case ED_BG:
		show_olc_cmds( ch, kbedit_table );
		break;
	case ED_MPCODE:
		show_olc_cmds( ch, mpedit_table );
		break;
	}

	return FALSE;
}



/*****************************************************************************
 *                           Interpreter Tables.                             *
 *****************************************************************************/
const struct olc_cmd_type aedit_table[] =
{
/*  {   command		function	}, */

	{   "age",		aedit_age	},
	{   "builder",	aedit_builder	}, /* s removed -- Hugin */
	{   "commands",	show_commands	},
	{   "create",	aedit_create	},
	{   "filename",	aedit_file	},
	{   "name",		aedit_name	},
/*  {   "recall",	aedit_recall	},   ROM OLC */
	{	"reset",	aedit_reset	},
	{   "security",	aedit_security	},
	{	"show",		aedit_show	},
	{   "vnum",		aedit_vnum	},
	{   "lvnum",	aedit_lvnum	},
	{   "uvnum",	aedit_uvnum	},
	{   "credits",	aedit_credits	},
	{	"flags",	aedit_flags	},

	{   "?",		show_help	},
	{   "version",	show_version	},

	{	NULL,		0,		}
};



const struct olc_cmd_type redit_table[] =
{
/*  {   command		function	}, */

	{   "commands",	show_commands	},
	{   "create",	redit_create	},
	{   "desc",		redit_desc	},
	{   "udesc",	redit_udesc	},
	{   "ddesc",	redit_ddesc	},
	{   "ed",		redit_ed	},
	{   "flags",	redit_flags	},
	{   "format",	redit_format	},
	{   "name",		redit_name	},
	{   "uname",	redit_uname	},
	{   "dname",	redit_dname	},
	{	"show",		redit_show	},
	{   "heal",		redit_heal	},
	{	"mana",		redit_mana	},
	{   "clan",		redit_clan	},
	{   "sector",	redit_sector	},
	{   "stops",	redit_stop	},

	{   "north",	redit_north	},
	{   "south",	redit_south	},
	{   "east",		redit_east	},
	{   "west",		redit_west	},
	{   "up",		redit_up	},
	{   "down",		redit_down	},

	{   "nearnorth",	redit_nearnorth	},
	{   "nearsouth",	redit_nearsouth	},
	{   "neareast",	redit_neareast	},
	{   "nearwest",	redit_nearwest	},
	{   "nearup",	redit_nearup	},
	{   "neardown",	redit_neardown	},

	/* New reset commands. */
	{	"mreset",	redit_mreset	},
	{	"oreset",	redit_oreset	},
	{	"mlist",	redit_mlist	},
	{	"rlist",	redit_rlist	},
	{	"olist",	redit_olist	},
	{	"mshow",	redit_mshow	},
	{	"oshow",	redit_oshow	},

	{   "?",		show_help	},
	{   "version",	show_version	},

	{	NULL,		0,		}
};



const struct olc_cmd_type oedit_table[] =
{
/*  {   command		function	}, */

	{   "addaffect",	oedit_addaffect	},
	{   "commands",	show_commands	},
	{   "cost",		oedit_cost	},
	{   "create",	oedit_create	},
	{   "delaffect",	oedit_delaffect	},
	{   "ed",		oedit_ed	},
	{   "long",		oedit_long	},
	{   "name",		oedit_name	},
	{   "short",	oedit_short	},
	{   "desc",		oedit_desc	},
	{	"show",		oedit_show	},
	{   "v0",		oedit_value0	},
	{   "v1",		oedit_value1	},
	{   "v2",		oedit_value2	},
	{   "v3",		oedit_value3	},
	{   "v4",		oedit_value4	},  /* ROM */
	{   "weight",	oedit_weight	},
	{	"fetish",	oedit_fetish	},  /* PT */
	{	"fetishlevel",	oedit_fetlevel	},  /* PT */
	{	"quality",	oedit_quality	},  /* PT */

	{   "extra",        oedit_extra     },  /* ROM */
	{   "extra2",       oedit_extra2    },  /* PT */
	{   "wear",         oedit_wear      },  /* ROM */
	{   "type",         oedit_type      },  /* ROM */
	{   "material",     oedit_material  },  /* ROM */
	{   "condition",    oedit_condition },  /* ROM */
	{	"liqlist",	oedit_liqlist	},  /* ROM */
	{	"use",		oedit_use_strings },
	{	"company",	oedit_company	},

	{   "?",		show_help	},
	{   "version",	show_version	},

	{	NULL,		0,		}
};



const struct olc_cmd_type medit_table[] =
{
/*  {   command		function	}, */

	{   "act",		medit_act	},
	{   "act2",		medit_act2	},
	{   "commands",	show_commands	},
	{   "create",	medit_create	},
	{   "desc",		medit_desc	},
	{   "level",	medit_level	},
	{   "long",		medit_long	},
	{   "name",		medit_name	},
	{   "shop",		medit_shop	},
	{   "short",	medit_short	},
	{	"show",		medit_show	},
	{   "spec",		medit_spec	},
	{   "triggers",	medit_triggers	},

	{   "sex",          medit_sex       },  /* ROM */
	{   "affect",       medit_affect    },  /* ROM */
	{   "form",         medit_form      },  /* ROM */
	{   "part",         medit_part      },  /* ROM */
	{   "imm",          medit_imm       },  /* ROM */
	{   "res",          medit_res       },  /* ROM */
	{   "vuln",         medit_vuln      },  /* ROM */
	{   "material",     medit_material  },  /* ROM */
	{   "off",          medit_off       },  /* ROM */
	{   "size",         medit_size      },  /* ROM */
	{   "hitdice",      medit_hitdice   },  /* ROM */
	{   "race",         medit_race      },  /* ROM */
	{   "position",     medit_position  },  /* ROM */
	{   "wealth",       medit_gold      },  /* ROM */
	{	"damtype",	medit_damtype	},
	{	"stats",	medit_stat	},  /* PT */
	{	"addmprog",	medit_addmprog	},  /* mprogs */
	{	"delmprog",	medit_delmprog	},  /* mprogs */

	{   "?",		show_help	},
	{   "version",	show_version	},

	{	NULL,		0,		}
};

const struct olc_cmd_type pedit_table [] =
{
/*  {   command		function		}, */

	{	"create",	pedit_create		},
	{	"remove",	pedit_remove		},
	{	"addevent",	pedit_addevent		},
	{	"delevent",	pedit_delevent		},
	{	"assignevent",	pedit_assignevent	},
	{	"races",	pedit_races		},
	{	"title",	pedit_title		},
	{	"show",		pedit_show		},

	{   "?",		show_help		},
	{   "version",	show_version		},

	{	NULL,		0,			}
};

const struct olc_cmd_type eedit_table [] =
{
/*  {   command		function	},

	{	"create",	eedit_create		}, */
	{	"remove",	eedit_remove		},
	{	"races",	eedit_races		},
	{	"title",	eedit_title		},
	{	"actors",	eedit_actors		},
	{	"scripts",	eedit_scripts		},
	{	"delscript",	eedit_delscript		},
	{	"location",	eedit_location		},
	{	"show",		eedit_show		},

	{   "?",		show_help		},
	{   "version",	show_version		},

	{	NULL,		0,			}
};

const struct olc_cmd_type sedit_table [] =
{
/*  {   command		function		}, */

	{	"create",	sedit_create		},
	{	"remove",	sedit_remove		},
	{	"trigger",	sedit_trigger		},
	{	"reaction",	sedit_reaction		},
	{	"actor",	sedit_actor		},
	{	"event",	sedit_event		},
	{   "delay",	sedit_delay		},
	{   "eventfirst",	sedit_eventfirst	},
	{	"show",		sedit_show		},

	{   "?",		show_help		},
	{   "version",	show_version		},

	{	NULL,		0,			}
};

const struct olc_cmd_type aiedit_table [] =
{
/*  {   command		function		}, */

	{	"create",	aiedit_create		},
	{	"remove",	aiedit_remove		},
	{	"show",		aiedit_show		},
	{	"name",		aiedit_name		},
	{	"triggeradd",	aiedit_trigger		},

	{   "?",		show_help		},
	{   "version",	show_version		},

	{	NULL,		0,			}
};

const struct olc_cmd_type rsedit_table [] =
{
/*  {   command		function		}, */

	{	"create",	rsedit_create		},
	{	"remove",	rsedit_remove		},
	{	"trigger",	rsedit_trigger		},
	{	"reaction",	rsedit_reaction		},
	{	"show",		rsedit_show		},
	{	"attitude",	rsedit_attitude		},
	{	"attitude",	rsedit_persona		},

	{   "?",		show_help		},
	{   "version",	show_version		},

	{	NULL,		0,			}
};

const struct olc_cmd_type hedit_table [] =
{
/*  {   command		function		}, */

	{	"create",	hedit_create		},
	{	"keyword",	hedit_keyword		},
	{	"trust",	hedit_trust		},
	{	"races",	hedit_races		},
	{	"clans",	hedit_clans		},
	{	"topic",	hedit_topic		},
	{	"quote",	hedit_quote		},
	{	"syntax",	hedit_syntax		},
	{	"website",	hedit_website		},
	{	"see",		hedit_see_also		},
	{	"unformatted",	hedit_no_format		},
	{	"body",		hedit_body		},

	{   "?",		show_help		},
	{   "version",	show_version		},

	{	NULL,		0,			}
};

const struct olc_cmd_type tipedit_table [] =
{
/*  {   command		function		}, */

	{	"create",	tipedit_create		},
	{	"keyword",	hedit_keyword		},
	{	"trust",	hedit_trust		},
/*  {	"races",	hedit_races		},
	{	"clans",	hedit_clans		},
	{	"topic",	hedit_topic		},
	{	"quote",	hedit_quote		},
	{	"syntax",	hedit_syntax		},
	{	"website",	hedit_website		},
	{	"see",		hedit_see_also		},
	{	"unformatted",	hedit_no_formatted	}, */
	{	"body",		hedit_body		},

	{   "?",		show_help		},
	{   "version",	show_version		},

	{	NULL,		0,			}
};

const struct olc_cmd_type kbedit_table [] =
{
/*  {   command		function		}, */

	{	"create",	kbedit_create		},
	{	"keyword",	kbedit_keyword		},
	{	"difficulty",	kbedit_diff		},
	{	"text",		kbedit_body		},
	{	"success",	kbedit_successes	},
	{	"race",		kbedit_races		},

	{   "?",		show_help		},
	{   "version",	show_version		},

	{	NULL,		0,			}
};



/*****************************************************************************
 *                          End Interpreter Tables.                          *
 *****************************************************************************/



/*****************************************************************************
 Name:		get_area_data
 Purpose:	Returns pointer to area with given vnum.
 Called by:	do_aedit(olc.c).
 ****************************************************************************/
AREA_DATA *get_area_data( int vnum )
{
	AREA_DATA *pArea;

	for (pArea = area_first; pArea; pArea = pArea->next )
	{
		if (pArea->vnum == vnum)
			return pArea;
	}

	return 0;
}



/*****************************************************************************
 Name:		get_plot_index
 Purpose:	Returns pointer to plot with given vnum.
 Called by:	do_pedit(olc.c).
 ****************************************************************************/
PLOT_DATA *get_plot_index( int vnum )
{
	PLOT_DATA *pPlot;

	for (pPlot = plot_list; pPlot; pPlot = pPlot->next )
	{
		if (pPlot->vnum == vnum)
			return pPlot;
	}

	return 0;
}



/*****************************************************************************
 Name:		get_event_index
 Purpose:	Returns pointer to event with given vnum.
 Called by:	do_eedit(olc.c).
 ****************************************************************************/
EVENT_DATA *get_event_index( int vnum )
{
	EVENT_DATA *pEvent;

	for (pEvent = event_list; pEvent; pEvent = pEvent->next )
	{
		if (pEvent->vnum == vnum)
			return pEvent;
	}

	return 0;
}



/*****************************************************************************
 Name:		get_script_index
 Purpose:	Returns pointer to script with given vnum.
 Called by:	do_sedit(olc.c).
 ****************************************************************************/
SCRIPT_DATA *get_script_index( int vnum )
{
	SCRIPT_DATA *pScript;

	for (pScript = script_first; pScript; pScript = pScript->next )
	{
		if (pScript->vnum == vnum)
			return pScript;
	}

	return 0;
}


/*****************************************************************************
 Name:		get_persona_index
 Purpose:	Returns pointer to persona with given vnum.
 Called by:	do_aiedit(olc.c).
 ****************************************************************************/
PERSONA *get_persona_index( int vnum )
{
	PERSONA *pPersona;

	for (pPersona = persona_first; pPersona; pPersona = pPersona->next )
	{
		if (pPersona->vnum == vnum)
			return pPersona;
	}

	return 0;
}


/*****************************************************************************
 Name:		get_react_index
 Purpose:	Returns pointer to react with given vnum.
 Called by:	do_rsedit(olc.c).
 ****************************************************************************/
REACT *get_react_index( int x, int vnum )
{
	REACT *pScript;
	int i = 0;

	for (pScript = react_first; pScript; pScript = pScript->next )
	{
		if (pScript->attitude == x)
	{
			i++;
		if(i == vnum)
		{
		return pScript;
		}
	}
	}

	return 0;
}


/*****************************************************************************
 Name:		get_help
 Purpose:	Returns pointer to help with given keyword(s).
 Called by:	do_hedit(olc.c).
 ****************************************************************************/
HELP_DATA * get_help(char *argument)
{
	HELP_DATA *pHelp;
	char argall[MIL]={'\0'};
	char argone[MIL]={'\0'};

	/* this parts handles help a b so that it returns help 'a b' */
	argall[0] = '\0';
	while (!IS_NULLSTR(argument) )
	{
		argument = one_argument(argument,argone);
		if (!IS_NULLSTR(argall))
			strncat(argall," ", sizeof(argall));
		strncat(argall,argone, sizeof(argall));
	}

	for ( pHelp = help_list; pHelp != NULL; pHelp = pHelp->next )
	{
		if ( is_exact_name( argall, pHelp->keyword ) )
		{
		return pHelp;
	}
	}

	return NULL;
}


/*****************************************************************************
 Name:		get_tip
 Purpose:	Returns pointer to tip with given number.
 Called by:	do_tipedit(olc.c).
 ****************************************************************************/
HELP_DATA * get_tip(char *argument)
{
	HELP_DATA *pHelp;

	if(!is_number(argument)) return NULL;

	for ( pHelp = tip_list; pHelp != NULL; pHelp = pHelp->next )
	{
		if ( is_exact_name( argument, pHelp->keyword ) )
		{
		return pHelp;
	}
	}

	return NULL;
}


/*****************************************************************************
 Name:		get_kbg
 Purpose:	Returns pointer to help or background with given keyword(s).
 Called by:	something I'm sure :}
 ****************************************************************************/
NOTE_DATA * get_kbg(char *argument, int type)
{
	NOTE_DATA *pNote;
	NOTE_DATA *thread;
	char argall[MIL]={'\0'};
	char argone[MIL]={'\0'};

	/* this parts handles note a b so that it returns note 'a b' */
	argall[0] = '\0';
	while (!IS_NULLSTR(argument) )
	{
		argument = one_argument(argument,argone);
		if (!IS_NULLSTR(argall))
			strncat(argall," ", sizeof(argall));
		strncat(argall,argone, sizeof(argall));
	}

	if(type == NOTE_BACKGROUND)
		thread = bg_list;
	else if(type == NOTE_KNOWLEDGE)
		thread = know_list;
	else thread = NULL;

	for ( pNote = thread; pNote != NULL; pNote = pNote->next )
	{
		if ( is_exact_name( argall, pNote->to_list ) )
		{
			return pNote;
		}
	}

	return NULL;
}


/*****************************************************************************
 Name:		edit_done
 Purpose:	Resets builder information on completion.
 Called by:	aedit, redit, oedit, medit(olc.c)
 ****************************************************************************/
bool edit_done( CHAR_DATA *ch )
{
SCRIPT_DATA *pscript = NULL;

	if(ch->desc->editor == ED_SCRIPT)
	{
	pscript = (SCRIPT_DATA *)ch->desc->pEdit;
	if(!pscript->event)
	{
		send_to_char("You must associate the script with an event.\n\r",ch);
		return TRUE;
	}
	}

	ch->desc->pEdit = NULL;
	ch->desc->editor = 0;
	return FALSE;
}



/*****************************************************************************
 *                              Interpreters.                                *
 *****************************************************************************/


/* Area Interpreter, called by do_aedit. */
void aedit( CHAR_DATA *ch, char *argument )
{
	AREA_DATA *pArea;
	char command[MIL]={'\0'};
	char arg[MIL]={'\0'};
	int  cmd = 0;
	int  value = 0;

	EDIT_AREA(ch, pArea);
	smash_tilde( argument );
	strncpy( arg, argument, sizeof(arg) );
	argument = one_argument( argument, command );

	if ( !IS_BUILDER( ch, pArea ) )
	{
		send_to_char( "AEdit:  Insufficient security to modify area.\n\r", ch );
		edit_done( ch );
		return;
	}

	if ( !str_cmp(command, "done") )
	{
		edit_done( ch );
		return;
	}

	if ( !IS_BUILDER( ch, pArea ) )
	{
		interpret( ch, arg );
		return;
	}

	if ( IS_NULLSTR(command) )
	{
		aedit_show( ch, argument );
		send_to_char("\tOType 'done' to exit the editor.\tn\n\r", ch);
		return;
	}

	if ( ( value = flag_value( area_flags, command ) ) != NO_FLAG )
	{
		TOGGLE_BIT(pArea->area_flags, value);

		send_to_char( "Flag toggled.\n\r", ch );
		return;
	}

	/* Search Table and Dispatch Command. */
	for ( cmd = 0; aedit_table[cmd].name != NULL; cmd++ )
	{
		if ( !str_prefix( command, aedit_table[cmd].name ) )
		{
			if ( (*aedit_table[cmd].olc_fun) ( ch, argument ) )
			{
				SET_BIT( pArea->area_flags, AREA_CHANGED );
				return;
			}
			else
				return;
		}
	}

	/* Default to Standard Interpreter. */
	interpret( ch, arg );
	return;
}


/* Room Interpreter, called by do_redit. */
void redit( CHAR_DATA *ch, char *argument )
{
	ROOM_INDEX_DATA *pRoom;
	AREA_DATA *pArea;
	char arg[MSL]={'\0'};
	char command[MIL]={'\0'};
	int  cmd = 0;
	int  value = 0;

	EDIT_ROOM(ch, pRoom);
	pArea = pRoom->area;

	smash_tilde( argument );
	strncpy( arg, argument, sizeof(arg) );
	argument = one_argument( argument, command );

	if ( !IS_BUILDER( ch, pArea ) )
	{
		send_to_char( "REdit:  Insufficient security to modify room.\n\r", ch );
	edit_done( ch );
	return;
	}

	if ( !str_cmp(command, "done") )
	{
	edit_done( ch );
	return;
	}

	if ( !IS_BUILDER( ch, pArea ) )
	{
		interpret( ch, arg );
		return;
	}

	if ( IS_NULLSTR(command) )
	{
	redit_show( ch, argument );
	send_to_char("\tOType 'done' to exit the editor.\tn\n\r", ch);
	return;
	}

	if ( ( value = flag_value( room_flags, command ) ) != NO_FLAG )
	{
		TOGGLE_BIT(pRoom->room_flags, value);

		SET_BIT( pArea->area_flags, AREA_CHANGED );
		send_to_char( "Room flag toggled.\n\r", ch );
		return;
	}

	if ( ( value = flag_value( sector_flags, command ) ) != NO_FLAG)
	{
		pRoom->sector_type  = value;

		SET_BIT( pArea->area_flags, AREA_CHANGED );
		send_to_char( "Sector type set.\n\r", ch );
		return;
	}

	/* Search Table and Dispatch Command. */
	for ( cmd = 0; redit_table[cmd].name != NULL; cmd++ )
	{
	if ( !str_prefix( command, redit_table[cmd].name ) )
	{
		if ( (*redit_table[cmd].olc_fun) ( ch, argument ) )
		{
		SET_BIT( pArea->area_flags, AREA_CHANGED );
		return;
		}
		else
		return;
	}
	}

	/* Default to Standard Interpreter. */
	interpret( ch, arg );
	return;
}



/* Object Interpreter, called by do_oedit. */
void oedit( CHAR_DATA *ch, char *argument )
{
	AREA_DATA *pArea;
	OBJ_INDEX_DATA *pObj;
	char arg[MSL]={'\0'};
	char command[MIL]={'\0'};
	int  cmd = 0;
/*  int  value;   ROM */

	smash_tilde( argument );
	strncpy( arg, argument, sizeof(arg) );
	argument = one_argument( argument, command );

	EDIT_OBJ(ch, pObj);
	pArea = pObj->area;

	if ( !IS_BUILDER( ch, pArea ) )
	  {
	send_to_char( "OEdit: Insufficient security to modify area.\n\r", ch );
	edit_done( ch );
	return;
	  }

	if ( !str_cmp(command, "done") )
	{
	edit_done( ch );
	return;
	}

	if ( !IS_BUILDER( ch, pArea ) )
	{
	interpret( ch, arg );
	return;
	}

	if ( IS_NULLSTR(command) )
	{
	oedit_show( ch, argument );
	send_to_char("\tOType 'done' to exit the editor.\tn\n\r", ch);
	return;
	}

	/* Search Table and Dispatch Command. */
	for ( cmd = 0; oedit_table[cmd].name != NULL; cmd++ )
	{
	if ( !str_prefix( command, oedit_table[cmd].name ) )
	{
		if ( (*oedit_table[cmd].olc_fun) ( ch, argument ) )
		{
		SET_BIT( pArea->area_flags, AREA_CHANGED );
		return;
		}
		else
		return;
	}
	}

	/* Default to Standard Interpreter. */
	interpret( ch, arg );
	return;
}



/* Mobile Interpreter, called by do_medit. */
void medit( CHAR_DATA *ch, char *argument )
{
	AREA_DATA *pArea;
	MOB_INDEX_DATA *pMob;
	char command[MIL]={'\0'};
	char arg[MSL]={'\0'};
	int  cmd = 0;
/*  int  value;    ROM */

	smash_tilde( argument );
	strncpy( arg, argument, sizeof(arg) );
	argument = one_argument( argument, command );

	EDIT_MOB(ch, pMob);
	pArea = pMob->area;

	if ( !IS_BUILDER( ch, pArea ) )
	  {
	send_to_char( "MEdit: Insufficient security to modify area.\n\r", ch );
	edit_done( ch );
	return;
	  }

	if ( !str_cmp(command, "done") )
	{
	edit_done( ch );
	return;
	}

	if ( !IS_BUILDER( ch, pArea ) )
	{
	interpret( ch, arg );
	return;
	}

	if ( IS_NULLSTR(command) )
	{
		medit_show( ch, argument );
	send_to_char("\tOType 'done' to exit the editor.\tn\n\r", ch);
		return;
	}

	/* Search Table and Dispatch Command. */
	for ( cmd = 0; medit_table[cmd].name != NULL; cmd++ )
	{
	if ( !str_prefix( command, medit_table[cmd].name ) )
	{
		if ( (*medit_table[cmd].olc_fun) ( ch, argument ) )
		{
		SET_BIT( pArea->area_flags, AREA_CHANGED );
		return;
		}
		else
		return;
	}
	}

	/* Default to Standard Interpreter. */
	interpret( ch, arg );
	return;
}


/* Plot Interpreter, called by do_pedit. */
void pedit( CHAR_DATA *ch, char *argument )
{
	PLOT_DATA *pPlot;
	char command[MIL]={'\0'};
	char arg[MIL]={'\0'};
	int  cmd = 0;
	int  value = 0;

	EDIT_PLOT(ch, pPlot);
	smash_tilde( argument );
	strncpy( arg, argument, sizeof(arg) );
	argument = one_argument( argument, command );

	if ( !IS_STORYTELLER( ch, pPlot ) )
	  {
	send_to_char( "PEdit:  Insufficient security to modify plot.\n\r", ch );
	edit_done( ch );
	return;
	  }

	if ( !str_cmp(command, "done") )
	{
	edit_done( ch );
	return;
	}

	if ( !IS_STORYTELLER( ch, pPlot ) )
	{
	interpret( ch, arg );
	return;
	}

	if ( IS_NULLSTR(command) )
	{
	pedit_show( ch, argument );
	send_to_char("\tOType 'done' to exit the editor.\tn\n\r", ch);
	return;
	}

	if ( ( value = flag_value( plot_races, command ) ) != NO_FLAG )
	{
	TOGGLE_BIT(pPlot->races, value);

	send_to_char( "Race toggled.\n\r", ch );
	return;
	}

	/* Search Table and Dispatch Command. */
	for ( cmd = 0; pedit_table[cmd].name != NULL; cmd++ )
	{
	if ( !str_prefix( command, pedit_table[cmd].name ) )
	{
		if ( (*pedit_table[cmd].olc_fun) ( ch, argument ) )
		return;
	}
	}

	/* Default to Standard Interpreter. */
	interpret( ch, arg );
	return;
}


/* Event Interpreter, called by do_eedit. */
void eedit( CHAR_DATA *ch, char *argument )
{
	EVENT_DATA *pEvent;
	char command[MIL]={'\0'};
	char arg[MIL]={'\0'};
	int  cmd = 0;
	int  value = 0;

	EDIT_EVENT(ch, pEvent);
	smash_tilde( argument );
	strncpy( arg, argument, sizeof(arg) );
	argument = one_argument( argument, command );

	if ( !IS_STORYTELLER( ch, pEvent ) )
	  {
	send_to_char( "EEdit:  Insufficient security to modify event.\n\r", ch );
	edit_done( ch );
	return;
	  }

	if ( !str_cmp(command, "done") )
	{
	edit_done( ch );
	return;
	}

	if ( !IS_STORYTELLER( ch, pEvent ) )
	{
	interpret( ch, arg );
	return;
	}

	if ( IS_NULLSTR(command) )
	{
	eedit_show( ch, argument );
	send_to_char("\tOType 'done' to exit the editor.\tn\n\r", ch);
	return;
	}

	if ( ( value = flag_value( plot_races, command ) ) != NO_FLAG )
	{
	TOGGLE_BIT(pEvent->races, value);

	send_to_char( "Race toggled.\n\r", ch );
	return;
	}

	/* Search Table and Dispatch Command. */
	for ( cmd = 0; eedit_table[cmd].name != NULL; cmd++ )
	{
	if ( !str_prefix( command, eedit_table[cmd].name ) )
	{
		if ( (*eedit_table[cmd].olc_fun) ( ch, argument ) )
		return;
	}
	}

	/* Default to Standard Interpreter. */
	interpret( ch, arg );
	return;
}


/* Script Unit Interpreter, called by do_sedit. */
void sedit( CHAR_DATA *ch, char *argument )
{
	SCRIPT_DATA *pScript;
	char command[MIL]={'\0'};
	char arg[MIL]={'\0'};
	int  cmd = 0;

	EDIT_SCRIPT(ch, pScript);
	smash_tilde( argument );
	strncpy( arg, argument, sizeof(arg) );
	argument = one_argument( argument, command );

	if ( !IS_STORYTELLER( ch, pScript ) )
	  {
	send_to_char( "SEdit:  Insufficient security to modify script.\n\r", ch );
	edit_done( ch );
	return;
	  }

	if ( !str_cmp(command, "done") )
	{
	edit_done( ch );
	return;
	}

	if ( !IS_STORYTELLER( ch, pScript ) )
	{
	interpret( ch, arg );
	return;
	}

	if ( IS_NULLSTR(command) )
	{
	sedit_show( ch, argument );
	send_to_char("\tOType 'done' to exit the editor.\tn\n\r", ch);
	return;
	}

	/* Search Table and Dispatch Command. */
	for ( cmd = 0; sedit_table[cmd].name != NULL; cmd++ )
	{
	if ( !str_prefix( command, sedit_table[cmd].name ) )
	{
		if ( (*sedit_table[cmd].olc_fun) ( ch, argument ) )
		return;
	}
	}

	/* Default to Standard Interpreter. */
	interpret( ch, arg );
	return;
}


/* Persona Unit Interpreter, called by do_aiedit. */
void aiedit( CHAR_DATA *ch, char *argument )
{
	PERSONA *pPersona;
	char command[MIL]={'\0'};
	char arg[MIL]={'\0'};
	int  cmd = 0;

	EDIT_PERSONA(ch, pPersona);
	smash_tilde( argument );
	strncpy( arg, argument, sizeof(arg) );
	argument = one_argument( argument, command );

	if ( !str_cmp(command, "done") )
	{
	edit_done( ch );
	return;
	}

	if ( IS_NULLSTR(command) )
	{
	aiedit_show( ch, argument );
	send_to_char("\tOType 'done' to exit the editor.\tn\n\r", ch);
	return;
	}

	/* Search Table and Dispatch Command. */
	for ( cmd = 0; aiedit_table[cmd].name != NULL; cmd++ )
	{
	if ( !str_prefix( command, aiedit_table[cmd].name ) )
	{
		if ( (*aiedit_table[cmd].olc_fun) ( ch, argument ) )
		return;
	}
	}

	/* Default to Standard Interpreter. */
	interpret( ch, arg );
	return;
}


/* AI Reaction Unit Interpreter, called by do_rsedit. */
void rsedit( CHAR_DATA *ch, char *argument )
{
	REACT *pScript;
	char command[MIL]={'\0'};
	char arg[MIL]={'\0'};
	int  cmd = 0;

	EDIT_REACT(ch, pScript);
	smash_tilde( argument );
	strncpy( arg, argument, sizeof(arg) );
	argument = one_argument( argument, command );

	if ( !str_cmp(command, "done") )
	{
	edit_done( ch );
	return;
	}

	if ( IS_NULLSTR(command) )
	{
	rsedit_show( ch, argument );
	send_to_char("\tOType 'done' to exit the editor.\tn\n\r", ch);
	return;
	}

	/* Search Table and Dispatch Command. */
	for ( cmd = 0; rsedit_table[cmd].name != NULL; cmd++ )
	{
	if ( !str_prefix( command, rsedit_table[cmd].name ) )
	{
		if ( (*rsedit_table[cmd].olc_fun) ( ch, argument ) )
		return;
	}
	}

	/* Default to Standard Interpreter. */
	interpret( ch, arg );
	return;
}


/* Help Interpreter, called by do_hedit. */
void hedit( CHAR_DATA *ch, char *argument, int type )
{
	HELP_DATA *pHelp;
	int  cmd = 0;
	char command[MIL]={'\0'};
	char arg[MIL]={'\0'};

	EDIT_HELP(ch, pHelp);
	smash_tilde( argument );
	strncpy( arg, argument, sizeof(arg) );
	argument = one_argument( argument, command );

	if ( !str_cmp(command, "done") )
	{
	edit_done( ch );
	return;
	}

	if ( IS_NULLSTR(command) )
	{
	hedit_show( ch, argument );
	send_to_char("\tOType 'done' to exit the editor.\tn\n\r", ch);
	return;
	}

	/* Search Table and Dispatch Command. */
	if(!type)
	{
	  for ( cmd = 0; hedit_table[cmd].name != NULL; cmd++ )
	  {
	if ( !str_prefix( command, hedit_table[cmd].name ) )
	{
		if ( (*hedit_table[cmd].olc_fun) ( ch, argument ) )
		return;
	}
	  }
	}
	else
	{
	  for ( cmd = 0; tipedit_table[cmd].name != NULL; cmd++ )
	  {
	if ( !str_prefix( command, tipedit_table[cmd].name ) )
	{
		if ( (*tipedit_table[cmd].olc_fun) ( ch, argument ) )
		return;
	}
	  }
	}

	/* Default to Standard Interpreter. */
	interpret( ch, arg );
	return;
}


/* Background and Knowledge Interpreter, called by bg and know. */
void kbedit( CHAR_DATA *ch, char *argument, int type )
{
	NOTE_DATA *pNote;
	int  cmd = 0;
	char command[MIL]={'\0'};
	char arg[MIL]={'\0'};

	EDIT_NOTE(ch, pNote);
	smash_tilde( argument );
	strncpy( arg, argument, sizeof(arg) );
	argument = one_argument( argument, command );

	if ( !str_cmp(command, "done") )
	{
	edit_done( ch );
	save_notes(type);
	return;
	}

	if ( IS_NULLSTR(command) )
	{
	kbedit_show( ch, argument );
	send_to_char("\tOType 'done' to exit the editor.\tn\n\r", ch);
	return;
	}

	/* Search Table and Dispatch Command. */
	for ( cmd = 0; kbedit_table[cmd].name != NULL; cmd++ )
	{
	if ( !str_prefix( command, kbedit_table[cmd].name ) )
	{
		if ( (*kbedit_table[cmd].olc_fun) ( ch, argument ) )
		return;
	}
	}

	/* Default to Standard Interpreter. */
	interpret( ch, arg );
	return;
}


const struct editor_cmd_type editor_table[] =
{
/*  {   command		function	}, */

	{   "area",		do_aedit	},
	{   "room",		do_redit	},
	{   "object",	do_oedit	},
	{   "mobile",	do_medit	},
	{   "plot",		do_pedit	},
	{   "event",	do_eedit	},
	{   "script",	do_sedit	},
	{   "persona",	do_aiedit	},
	{   "react",	do_rsedit	},
	{   "help",		do_hedit	},
	{   "tip",		do_tipedit	},
	{   "bg",		do_bg		},
	{   "know",		do_know		},
	{   "mprog",	do_mpedit	},

	{	NULL,		0,		}
};


/* Entry point for all editors. */
void do_olc( CHAR_DATA *ch, char *argument )
{
	int  cmd = 0;
	char command[MIL]={'\0'};

	argument = one_argument( argument, command );

	if ( IS_NULLSTR(command) )
	{
		do_help( ch, "olc" );
		return;
	}
 
	/* Search Table and Dispatch Command. */
	for ( cmd = 0; editor_table[cmd].name != NULL; cmd++ )
	{
	if ( !str_prefix( command, editor_table[cmd].name ) )
	{
		(*editor_table[cmd].do_fun) ( ch, argument );
		return;
	}
	}

	/* Invalid command, send help. */
	do_help( ch, "olc" );
	return;
}



/* Entry point for editing area_data. */
void do_aedit( CHAR_DATA *ch, char *argument )
{
	AREA_DATA *pArea;
	int value = 0;
	char value2[MSL]={'\0'};
	char arg[MSL]={'\0'};

	CheckChNPC(ch);

	pArea = ch->in_room->area;

	argument = one_argument(argument,arg);
	if ( is_number( arg ) )
	{
	value = atoi( arg );
	if ( !( pArea = get_area_data( value ) ) )
	{
		send_to_char( "That area vnum does not exist.\n\r", ch );
		return;
	}
	}
	else
	{
	if ( !str_cmp( arg, "create" ) )
	{
		if (!IS_NPC(ch) && (ch->pcdata->security < 9) )
		{
		   send_to_char("Insufficient security to create areas.\n\r",ch);
		   return;
		}
		argument    	=   one_argument(argument,value2);
		value = atoi (value2);
		if (get_area_data(value) != NULL)
		   {
				send_to_char("That area exists!",ch);
				return;
		   }
		pArea               =   new_area();
		area_last->next     =   pArea;
		area_last		=   pArea;	/* Thanks, Walker. */
		SET_BIT( pArea->area_flags, AREA_ADDED );
		send_to_char("Area created.\n\r",ch);
	}
	}

	if (!IS_BUILDER(ch,pArea))
	{
		send_to_char("Insuficiente seguridad para editar areas.\n\r",ch);
		return;
	}

	ch->desc->pEdit = (void *)pArea;
	ch->desc->editor = ED_AREA;
	return;
}



/* Entry point for editing room_index_data. */
void do_redit( CHAR_DATA *ch, char *argument )
{
	ROOM_INDEX_DATA *pRoom, *pRoom2;
	char arg1[MSL]={'\0'};

	CheckChNPC(ch);

	argument = one_argument( argument, arg1 );

	pRoom = ch->in_room;

	if ( !str_cmp( arg1, "reset" ) )
	{
	if ( !IS_BUILDER( ch, pRoom->area ) )
	{
		send_to_char( "You do not have access to edit this area.\n\r" , ch );
			return;
	}

	forced_reset_room( pRoom );
	send_to_char( "Room reset.\n\r", ch );
	return;
	}
	else
	if ( !str_cmp( arg1, "create" ) )
	{
	if ( IS_NULLSTR(argument) || atoi( argument ) == 0 )
	{
		send_to_char( "Syntax:  edit room create [vnum]\n\r", ch );
		return;
	}

	if ( redit_create( ch, argument ) )
	{
		char_from_room( ch );
		char_to_room( ch, (ROOM_INDEX_DATA *)ch->desc->pEdit );
		SET_BIT( pRoom->area->area_flags, AREA_CHANGED );
		pRoom = ch->in_room;
	}
	}
	else
	{
		pRoom2 = get_room_index(atoi(arg1));
	
		if ( (pRoom2 != NULL) && IS_BUILDER(ch,pRoom2->area) )
		{
		   char_from_room( ch );
		   char_to_room( ch, pRoom2 );
		   pRoom = ch->in_room;
		}
		else
		if (atoi(arg1) != 0)
		{
		   send_to_char("Either you do not have authority to edit that room, or it does not exist..\n\r",ch);
		   return;
		}   
	}

	if ( !IS_BUILDER( ch, pRoom->area ) )
	{
	send_to_char( "You do not have authority to edit that room.\n\r" , ch );
		return;
	}

	ch->desc->editor = ED_ROOM;
	return;
}



/* Entry point for editing obj_index_data. */
void do_oedit( CHAR_DATA *ch, char *argument )
{
	OBJ_INDEX_DATA *pObj;
	AREA_DATA *pArea;
	char arg1[MSL]={'\0'};
	int value = 0;

	CheckChNPC(ch);

	argument = one_argument( argument, arg1 );

	if ( is_number( arg1 ) )
	{
	value = atoi( arg1 );
	if ( !( pObj = get_obj_index( value ) ) )
	{
		send_to_char( "OEdit:  That vnum does not exist.\n\r", ch );
		return;
	}

	if ( !IS_BUILDER( ch, pObj->area ) )
		{
		send_to_char( "You do not have the authority to edit that object.\n\r" , ch );
			return;
		}

	ch->desc->pEdit = (void *)pObj;
	ch->desc->editor = ED_OBJECT;
	return;
	}
	else
	{
	if ( !str_cmp( arg1, "create" ) )
	{
		value = atoi( argument );
		if ( IS_NULLSTR(argument) || value == 0 )
		{
		send_to_char( "Syntax:  edit object create [vnum]\n\r", ch );
		return;
		}

		pArea = get_vnum_area( value );

		if ( !pArea )
		{
		send_to_char( "OEdit:  That vnum is not assigned an area.\n\r", ch );
		return;
		}

		if ( !IS_BUILDER( ch, pArea ) )
		{
		send_to_char( "You do not have the authority to edit objects.\n\r" , ch );
			return;
		}

		if ( oedit_create( ch, argument ) )
		{
		SET_BIT( pArea->area_flags, AREA_CHANGED );
		ch->desc->editor = ED_OBJECT;
		}
		return;
	}
	}

	send_to_char( "OEdit:  There is no default object to edit.\n\r(Use oedit create <vnum> to create a new object.)\n\r", ch );
	return;
}



/* Entry point for editing mob_index_data. */
void do_medit( CHAR_DATA *ch, char *argument )
{
	MOB_INDEX_DATA *pMob;
	AREA_DATA *pArea;
	int value = 0;
	char arg1[MSL]={'\0'};

	CheckChNPC(ch);

	argument = one_argument( argument, arg1 );

	if ( is_number( arg1 ) )
	{
	value = atoi( arg1 );
	if ( !( pMob = get_mob_index( value ) ))
	{
		send_to_char( "MEdit:  That vnum does not exist.\n\r", ch );
		return;
	}

	if ( !IS_BUILDER( ch, pMob->area ) )
	{
		send_to_char( "Insufficient security to edit mobs.\n\r" , ch );
			return;
	}

	ch->desc->pEdit = (void *)pMob;
	ch->desc->editor = ED_MOBILE;
	return;
	}
	else
	{
	if ( !str_cmp( arg1, "create" ) )
	{
		value = atoi( argument );
		if ( IS_NULLSTR(arg1) || value == 0 )
		{
		send_to_char( "Syntax:  edit mobile create [vnum]\n\r", ch );
		return;
		}

		pArea = get_vnum_area( value );

		if ( !pArea )
		{
		send_to_char( "MEdit:  That vnum is not assigned an area.\n\r", ch );
		return;
		}

		if ( !IS_BUILDER( ch, pArea ) )
		{
		send_to_char( "Insuficiente seguridad para modificar mobs.\n\r" , ch );
			return;
		}

		if ( medit_create( ch, argument ) )
		{
		SET_BIT( pArea->area_flags, AREA_CHANGED );
		ch->desc->editor = ED_MOBILE;
		}
		return;
	}
	}

	send_to_char( "MEdit:  There is no default mobile to edit.\n\r(Use medit create <vnum> to create a new mobile.)\n\r", ch );
	return;
}

/* Entry point for editing plot_data. */
void do_pedit( CHAR_DATA *ch, char *argument )
{
	PLOT_DATA *pPlot = NULL;
	int value = 0;
	char arg1[MSL]={'\0'};

	CheckChNPC(ch);

	argument = one_argument( argument, arg1 );

	if ( is_number( arg1 ) )
	{
	value = atoi( arg1 );
	if ( !( pPlot = get_plot_index( value ) ))
	{
		send_to_char( "PEdit:  That vnum does not exist.\n\r", ch );
		return;
	}

	if ( !IS_STORYTELLER( ch, pPlot ) )
	{
		send_to_char( "Insufficient security to edit plot.\n\r" , ch );
			return;
	}

	ch->desc->pEdit = (void *)pPlot;
	ch->desc->editor = ED_PLOT;
	return;
	}
	else
	{
	if ( !str_cmp( arg1, "create" ) )
	{
		if ( IS_NULLSTR(arg1) )
		{
		send_to_char( "Syntax:  edit plot create\n\r", ch );
		return;
		}

		if ( !IS_STORYTELLER( ch, pPlot ) )
		{
		send_to_char( "Insuficiente seguridad para modificar plots.\n\r" , ch );
			return;
		}

		if ( pedit_create( ch, "" ) )
		{
		ch->desc->editor = ED_PLOT;
		}
		return;
	}
	}

	send_to_char( "PEdit:  There is no default plot to edit.\n\r(Use pedit create to create a new plot.)\n\r", ch );
	return;
}


/* Entry point for editing event_data. */
void do_eedit( CHAR_DATA *ch, char *argument )
{
	EVENT_DATA *pEvent = NULL;
	int value;
	char arg1[MSL]={'\0'};

	CheckChNPC(ch);

	argument = one_argument( argument, arg1 );

	if ( is_number( arg1 ) )
	{
	value = atoi( arg1 );
	if ( !( pEvent = get_event_index( value ) ))
	{
		send_to_char( "EEdit:  That vnum does not exist.\n\r", ch );
		return;
	}

	if ( !IS_STORYTELLER( ch, pEvent ) )
	{
		send_to_char( "Insufficient security to edit events.\n\r" , ch );
			return;
	}

	ch->desc->pEdit = (void *)pEvent;
	ch->desc->editor = ED_EVENT;
	return;
	}
	else
	{
/*
	if ( !str_cmp( arg1, "create" ) )
	{
		value = atoi( argument );
		if ( IS_NULLSTR(arg1) )
		{
		send_to_char( "Syntax:  edit event create\n\r", ch );
		return;
		}

		if ( !IS_STORYTELLER( ch, pEvent ) )
		{
		send_to_char( "Insuficiente seguridad para modificar events.\n\r" , ch );
			return;
		}

		if ( eedit_create( ch, "" ) )
		{
		ch->desc->editor = ED_EVENT;
		}
		return;
	}
*/
	send_to_char("edit event <event vnum>\n\r", ch);
	send_to_char("(To create a new event, please create it from within the appropriate plot.)\n\r", ch);
	}

	send_to_char( "EEdit:  There is no default event to edit.\n\r", ch );
	send_to_char( "(Use event_edit create to create a new event.)\n\r", ch);
	return;
}


/* Entry point for editing script_data. */
void do_sedit( CHAR_DATA *ch, char *argument )
{
	SCRIPT_DATA *pScript = NULL;
	int value;
	char arg1[MSL]={'\0'};

	CheckChNPC(ch);

	argument = one_argument( argument, arg1 );

	if ( is_number( arg1 ) )
	{
	value = atoi( arg1 );
	if ( !( pScript = get_script_index( value ) ))
	{
		send_to_char( "SEdit:  That vnum does not exist.\n\r", ch );
		return;
	}

	if ( !IS_STORYTELLER( ch, pScript ) )
	{
		send_to_char( "Insufficient security to edit scripts.\n\r" , ch );
			return;
	}

	ch->desc->pEdit = (void *)pScript;
	ch->desc->editor = ED_SCRIPT;
	return;
	}
	else
	{
	if ( !str_cmp( arg1, "create" ) )
	{
		value = atoi( argument );
		if ( IS_NULLSTR(arg1) )
		{
		send_to_char( "Syntax:  edit script create\n\r", ch );
		return;
		}

		if ( !IS_STORYTELLER( ch, pScript ) )
		{
		send_to_char( "Insuficiente seguridad para modificar scripts.\n\r" , ch );
			return;
		}

		if ( sedit_create( ch, "" ) )
		{
		ch->desc->editor = ED_SCRIPT;
		}
		return;
	}
	}

	send_to_char( "SEdit:  There is no default script to edit.\n\r(Use sedit create to create a new script.)\n\r", ch );
	return;
}


/* Entry point for editing personas. */
void do_aiedit( CHAR_DATA *ch, char *argument )
{
	PERSONA *pPersona = NULL;
	int value;
	char arg1[MSL]={'\0'};

	CheckChNPC(ch);

	argument = one_argument( argument, arg1 );

	if ( is_number( arg1 ) )
	{
	value = atoi( arg1 );
	if ( !( pPersona = get_persona_index( value ) ))
	{
		send_to_char( "AIEdit:  That vnum does not exist.\n\r", ch );
		return;
	}

	ch->desc->pEdit = (void *)pPersona;
	ch->desc->editor = ED_PERSONA;
	return;
	}
	else
	{
	if ( !str_cmp( arg1, "create" ) )
	{
		if ( IS_NULLSTR(arg1) )
		{
		send_to_char( "Syntax:  edit persona create\n\r", ch );
		return;
		}

		if ( aiedit_create( ch, "" ) )
		{
		ch->desc->editor = ED_PERSONA;
		}
		return;
	}
	}

	send_to_char( "AIEdit:  There is no default persona to edit.\n\r(Use aiedit create to create a new persona.)\n\r", ch );
	return;
}


/* Entry point for editing reacts. */
void do_rsedit( CHAR_DATA *ch, char *argument )
{
	REACT *pScript = NULL;
	int value, vnum;
	char arg1[MSL]={'\0'};

	CheckChNPC(ch);

	argument = one_argument( argument, arg1 );

	if ( is_number( arg1 ) && is_number( argument ) )
	{
	value = atoi( arg1 );
	vnum = atoi( argument );
	if ( !( pScript = get_react_index( value, vnum ) ))
	{
		send_to_char( "RSEdit:  That location or vnum does not exist.\n\r", ch );
		return;
	}

	ch->desc->pEdit = (void *)pScript;
	ch->desc->editor = ED_REACT;
	return;
	}
	else
	{
	if ( !str_cmp( arg1, "create" ) )
	{
		value = atoi( argument );
		if ( IS_NULLSTR(arg1) || value < 0 )
		{
		send_to_char( "Syntax:  edit react create [attitude num]\n\r", ch );
		return;
		}

		if ( rsedit_create( ch, argument ) )
		{
		ch->desc->editor = ED_REACT;
		}
		return;
	}
	}

	send_to_char( "RSEdit:  There is no default reaction to edit.\n\r(Use rsedit create to create a new reaction set.)\n\r", ch );
	return;
}


/* Entry point for editing helps. */
void do_hedit( CHAR_DATA *ch, char *argument )
{
	HELP_DATA *pHelp = NULL;
	char arg1[MSL]={'\0'};
	char *fullarg;

	CheckChNPC(ch);

	fullarg = str_dup(argument);
	argument = one_argument( argument, arg1 );

	if ( !str_cmp( arg1, "create" ) )
	{
	if ( IS_NULLSTR(arg1) )
	{
		send_to_char( "Syntax:  edit help create [keywords]\n\r", ch );
		return;
	}

	if ( hedit_create( ch, argument ) )
	{
		ch->desc->editor = ED_HELP;
	}
	return;
	}
	else if(arg1[0] !='\0')
	{
	
	if ( (pHelp = get_help( arg1 )) == NULL )
	{
		send_to_char( "HEdit:  That help does not exist.\n\r", ch );
		return;
	}

	ch->desc->pEdit = (void *)pHelp;
	ch->desc->editor = ED_HELP;
	return;
	}

	send_to_char( "HEdit:  There is no default help to edit.\n\r(Use hedit create to create a new help.)\n\r", ch );
	return;
}


/* Entry point for editing tips. */
void do_tipedit( CHAR_DATA *ch, char *argument )
{
	HELP_DATA *pHelp = NULL;
	char arg1[MSL]={'\0'};
	char *fullarg;

	CheckChNPC(ch);

	fullarg = str_dup(argument);
	argument = one_argument( argument, arg1 );

	if ( !str_cmp( arg1, "create" ) )
	{
		if ( IS_NULLSTR(arg1) )
		{
			send_to_char( "Syntax:  edit tip create\n\r", ch );
			return;
		}

		if ( tipedit_create( ch, (char *)Format("%d", top_tip) ) )
		{
			ch->desc->editor = ED_TIP;
		}
		return;
	}
	else if(arg1[0] !='\0')
	{

		if ( (pHelp = get_tip( arg1 )) == NULL )
		{
			send_to_char( "TipEdit:  That tip does not exist.\n\r", ch );
			return;
		}

		ch->desc->pEdit = (void *)pHelp;
		ch->desc->editor = ED_TIP;
		return;
	}

	send_to_char( "TipEdit:  There is no default tip to edit.\n\r(Use tipedit create to create a new tip.)\n\r", ch );
	return;
}

/* Entry point for editing knowledge and background entries. */
void do_kbedit( CHAR_DATA *ch, char *argument, int type )
{
	NOTE_DATA *pNote = NULL;
	char arg1[MSL]={'\0'};
	char *fullarg;

	CheckChNPC(ch);

	fullarg = str_dup(argument);
	argument = one_argument( argument, arg1 );

	if ( !str_cmp( arg1, "create" ) )
	{
	if ( IS_NULLSTR(arg1) )
	{
		send_to_char( "Syntax:  edit know create [keywords]\n\r", ch );
		send_to_char( "     OR  edit bg create [keywords]\n\r", ch );
		return;
	}

		if(type == NOTE_KNOWLEDGE)
		{
		if ( kedit_create( ch, argument ) )
			ch->desc->editor = ED_KNOW;
		}
		else if(type == NOTE_BACKGROUND)
		{
		if ( bedit_create( ch, argument ) )
			ch->desc->editor = ED_BG;
		}
	return;
	}
	else if(arg1[0] !='\0')
	{
	
	if ( (pNote = get_kbg( arg1, type )) == NULL )
	{
		if(type == NOTE_KNOWLEDGE)
		send_to_char( "Knowledge:  That knowledge does not exist.\n\r", ch );
		else if(type == NOTE_BACKGROUND)
		send_to_char( "Background: That background does not exist.\n\r", ch );
		return;
	}

	ch->desc->pEdit = (void *)pNote;
	if(type == NOTE_KNOWLEDGE)
		ch->desc->editor = ED_KNOW;
	else if(type == NOTE_BACKGROUND)
		ch->desc->editor = ED_BG;
	return;
	}

	if(type == NOTE_KNOWLEDGE)
	send_to_char( "Knowledge:  There is no default knowledge to edit.\n\r(Use know create to create a new knowledge.)\n\r", ch );
	else if(type == NOTE_BACKGROUND)
	send_to_char( "Background: There is no default background to edit.\n\r(Use bg create to create a new background.)\n\r", ch );
	return;
}


void display_resets( CHAR_DATA *ch )
{
	ROOM_INDEX_DATA	*pRoom;
	RESET_DATA		*pReset;
	MOB_INDEX_DATA	*pMob = NULL;
	char 		final [ MSL ]={'\0'};
	int 		iReset = 0;

	EDIT_ROOM(ch, pRoom);
	
	send_to_char(" No.   Loads   Description       Location         Vnum   Mx Mn Description\n\r", ch);
	send_to_char("==== ======== ============= =================== ======== ===== ===========\n\r", ch);

	for ( pReset = pRoom->reset_first; pReset; pReset = pReset->next )
	{
	OBJ_INDEX_DATA  *pObj;
	MOB_INDEX_DATA  *pMobIndex;
	OBJ_INDEX_DATA  *pObjIndex;
	OBJ_INDEX_DATA  *pObjToIndex;
	ROOM_INDEX_DATA *pRoomIndex;

	final[0] = '\0';
	snprintf( final, sizeof(final), "[%2d] ", ++iReset );

	switch ( pReset->command )
	{
	default:
		strncat( final, Format("Bad reset command: %c.", pReset->command), sizeof(final) );
		break;

	case 'M':
		if ( !( pMobIndex = get_mob_index( pReset->arg1 ) ) )
		{
				strncat( final, Format("Load Mobile - Bad Mob %d\n\r", pReset->arg1), sizeof(final) );
				continue;
		}

		if ( !( pRoomIndex = get_room_index( pReset->arg3 ) ) )
		{
				strncat( final, Format("Load Mobile - Bad Room %d\n\r", pReset->arg3), sizeof(final) );
				continue;
		}

			pMob = pMobIndex;
			strncat( final, Format("M[%5d] %-13.13s in room             R[%5d] %2d-%2d %-15.15s\n\r",
					   pReset->arg1, pMob->short_descr, pReset->arg3, pReset->arg2, pReset->arg4, pRoomIndex->name), sizeof(final) );

		break;

	case 'O':
		if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) )
		{
				strncat( final, Format("Load Object - Bad Object %d\n\r", pReset->arg1), sizeof(final) );
				continue;
		}

			pObj       = pObjIndex;

		if ( !( pRoomIndex = get_room_index( pReset->arg3 ) ) )
		{
				strncat( final, Format("Load Object - Bad Room %d\n\r", pReset->arg3), sizeof(final) );
				continue;
		}

			strncat( final, Format("O[%5d] %-13.13s in room             R[%5d]       %-15.15s\n\r",
						  pReset->arg1, pObj->short_descr, pReset->arg3, pRoomIndex->name), sizeof(final) );

		break;

	case 'P':
		if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) )
		{
				strncat( final, Format("Put Object - Bad Object %d\n\r", pReset->arg1), sizeof(final) );
				continue;
		}

			pObj       = pObjIndex;

		if ( !( pObjToIndex = get_obj_index( pReset->arg3 ) ) )
		{
				strncat( final, Format("Put Object - Bad To Object %d\n\r", pReset->arg3), sizeof(final) );
				continue;
		}

		strncat( final, Format("O[%5d] %-13.13s inside              O[%5d] %2d-%2d %-15.15s\n\r",
				pReset->arg1, pObj->short_descr, pReset->arg3, pReset->arg2, pReset->arg4, pObjToIndex->short_descr), sizeof(final) );

		break;

	case 'G':
	case 'E':
		if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) )
		{
				strncat( final, Format("Give/Equip Object - Bad Object %d\n\r", pReset->arg1), sizeof(final) );
				continue;
		}

			pObj       = pObjIndex;

		if ( !pMob )
		{
				strncat( final, Format("Give/Equip Object - No Previous Mobile\n\r"), sizeof(final) );
				break;
		}

		if ( pMob->pShop )
		{
			send_to_char(Format("O[%5d] %-13.13s in the inventory of S[%5d]       %-15.15s\n\r",
					pReset->arg1, pObj->short_descr, pMob->vnum, pMob->short_descr), ch);
		}
		else
		strncat( final, Format("O[%5d] %-13.13s %-19.19s M[%5d]       %-15.15s\n\r",
					pReset->arg1, pObj->short_descr, (pReset->command == 'G') ? flag_string( wear_loc_strings, WEAR_NONE ) : flag_string( wear_loc_strings, pReset->arg3 ),
							pMob->vnum, pMob->short_descr), sizeof(final) );

		break;

	/*
	 * Doors are set in rs_flags don't need to be displayed.
	 * If you want to display them then uncomment the new_reset
	 * line in the case 'D' in load_resets in db.c and here.
	 */
	case 'D':
		pRoomIndex = get_room_index( pReset->arg1 );
		strncat( final, Format("R[%5d] %s door of %-19.19s reset to %s\n\r",
				pReset->arg1, capitalize( dir_name[ pReset->arg2 ] ), pRoomIndex->name, flag_string( door_resets, pReset->arg3 )), sizeof(final) );

		break;
		/*
		 * End Doors Comment.
		 */
	case 'R':
		if ( !( pRoomIndex = get_room_index( pReset->arg1 ) ) )
		{
			strncat( final, Format("Randomize Exits - Bad Room %d\n\r", pReset->arg1), sizeof(final) );
			continue;
		}

		strncat( final, Format("R[%5d] Exits are randomized in %s\n\r", pReset->arg1, pRoomIndex->name), sizeof(final) );

		break;
	}
	send_to_char( final, ch );
	}

	return;
}



/*****************************************************************************
 Name:		add_reset
 Purpose:	Inserts a new reset in the given index slot.
 Called by:	do_resets(olc.c).
 ****************************************************************************/
void add_reset( ROOM_INDEX_DATA *room, RESET_DATA *pReset, int index )
{
	RESET_DATA *reset;
	int iReset = 0;

	if ( !room->reset_first )
	{
	room->reset_first	= pReset;
	room->reset_last	= pReset;
	pReset->next		= NULL;
	return;
	}

	index--;

	if ( index == 0 )	/* First slot (1) selected. */
	{
	pReset->next = room->reset_first;
	room->reset_first = pReset;
	return;
	}

	/*
	 * If negative slot( <= 0 selected) then this will find the last.
	 */
	for ( reset = room->reset_first; reset->next; reset = reset->next )
	{
	if ( ++iReset == index )
		break;
	}

	pReset->next	= reset->next;
	reset->next		= pReset;
	if ( !pReset->next )
	room->reset_last = pReset;
	return;
}

/*
 * To lessen the number of times a builder's resets crashes the mud.
 * C/O One Thousand Monkeys
 */
bool mob_reset_before( ROOM_INDEX_DATA *room, int num )
{
	RESET_DATA *pr;
	int tag = 1;

	for(pr = room->reset_first;tag < num;pr = pr->next)
	{
		if(pr->command == 'M')
			return TRUE;
		tag++;
	}

	return FALSE;
}


void do_resets( CHAR_DATA *ch, char *argument )
{
	char arg1[MIL]={'\0'};
	char arg2[MIL]={'\0'};
	char arg3[MIL]={'\0'};
	char arg4[MIL]={'\0'};
	char arg5[MIL]={'\0'};
	char arg6[MIL]={'\0'};
	char arg7[MIL]={'\0'};
	RESET_DATA *pReset = NULL;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );
	argument = one_argument( argument, arg4 );
	argument = one_argument( argument, arg5 );
	argument = one_argument( argument, arg6 );
	argument = one_argument( argument, arg7 );

	if ( !IS_BUILDER( ch, ch->in_room->area ) )
	{
	send_to_char( "Resets: Invalid security for editing this area.\n\r", ch );
	return;
	}

	/*
	 * Display resets in current room.
	 * -------------------------------
	 */
	if ( IS_NULLSTR(arg1) )
	{
	if ( ch->in_room->reset_first )
	{
		send_to_char( "Resets: M = mobile, R = room, O = object, "
		"P = pet, S = shopkeeper\n\r", ch );
		display_resets( ch );
	}
	else
		send_to_char( "No resets in this room.\n\r", ch );
	}


	/*
	 * Take index number and search for commands.
	 * ------------------------------------------
	 */
	if ( is_number( arg1 ) )
	{
	ROOM_INDEX_DATA *pRoom = ch->in_room;

	/*
	 * Delete a reset.
	 * ---------------
	 */
	if ( !str_cmp( arg2, "delete" ) )
	{
		int insert_loc = atoi( arg1 );

		if ( !ch->in_room->reset_first )
		{
		send_to_char( "No resets in this area.\n\r", ch );
		return;
		}

		if ( insert_loc-1 <= 0 )
		{
		pReset = pRoom->reset_first;
		pRoom->reset_first = pRoom->reset_first->next;
		if ( !pRoom->reset_first )
			pRoom->reset_last = NULL;
		}
		else
		{
		int iReset = 0;
		RESET_DATA *prev = NULL;

		for ( pReset = pRoom->reset_first;
		  pReset;
		  pReset = pReset->next )
		{
			if ( ++iReset == insert_loc )
			break;
			prev = pReset;
		}

		if ( !pReset )
		{
			send_to_char( "Reset not found.\n\r", ch );
			return;
		}

		if ( prev )
			prev->next = prev->next->next;
		else
			pRoom->reset_first = pRoom->reset_first->next;

		for ( pRoom->reset_last = pRoom->reset_first;
		  pRoom->reset_last->next;
		  pRoom->reset_last = pRoom->reset_last->next );
		}

		free_reset_data( pReset );
		send_to_char( "Reset deleted.\n\r", ch );
	}
	else
	/*
	 * Add a reset.
	 * ------------
	 */
	if ( (!str_cmp( arg2, "mob" ) && is_number( arg3 ))
	  || (!str_cmp( arg2, "obj" ) && is_number( arg3 )) )
	{
		/*
		 * Check for Mobile reset.
		 * -----------------------
		 */
		if ( !str_cmp( arg2, "mob" ) )
		{
		pReset = new_reset_data();
		pReset->command = 'M';
		if (get_mob_index( is_number(arg3) ? atoi( arg3 ) : 1 ) == NULL)
		  {
			send_to_char("That mob does not exist.\n\r",ch);
			return;
		  }
		pReset->arg1    = atoi( arg3 );
		pReset->arg2    = is_number( arg4 ) ? atoi( arg4 ) : 1; /* Max # */
		pReset->arg3    = ch->in_room->vnum;
		pReset->arg4	= is_number( arg5 ) ? atoi( arg5 ) : 1; /* Min # */
		}
		else
		/*
		 * Check for Object reset.
		 * -----------------------
		 */
		if ( !str_cmp( arg2, "obj" ) )
		{
		pReset = new_reset_data();
		pReset->arg1    = atoi( arg3 );
		/*
		 * Inside another object.
		 * ----------------------
		 */
		if ( !str_prefix( arg4, "inside" ) )
		{
			OBJ_INDEX_DATA *temp;
			pReset->command = 'P';
			pReset->arg2    = 0;
			temp = get_obj_index(is_number(arg5)? atoi(arg5) : 1 );
			if (temp == NULL)
			{
			send_to_char("Unable to locate object 2.\n\r", ch);
			return;
			}
			if (temp->item_type != ITEM_CONTAINER)
			{
			send_to_char( "Object 2 is not a container\n\r", ch);
			return;
			}
			pReset->arg2    = is_number( arg6 ) ? atoi( arg6 ) : 1;
			pReset->arg3    = is_number( arg5 ) ? atoi( arg5 ) : 1;
			pReset->arg4    = is_number( arg7 ) ? atoi( arg7 ) : 1;
		}
		else
		/*
		 * Inside the room.
		 * ----------------
		 */
		if ( !str_cmp( arg4, "room" ) )
		{
			pReset           = new_reset_data();
			pReset->command  = 'O';
			if (get_obj_index(atoi(arg3)) == NULL)
			  {
				 send_to_char( "That vnum does not exist.\n\r",ch);
				 return;
			  }
			pReset->arg1     = atoi ( arg3 );
			pReset->arg2    = is_number( arg5 ) ? atoi( arg6 ) : 1;
			pReset->arg3     = ch->in_room->vnum;
			pReset->arg4    = is_number( arg6 ) ? atoi( arg6 ) : 1;
		}
		else
		/*
		 * Into a Mobile's inventory.
		 * --------------------------
		 */
		{
		  if ( flag_value( wear_loc_flags, arg4 ) == NO_FLAG )
		  {
			send_to_char( "Resets: '? wear-loc'\n\r", ch );
			return;
		  }
		  if( !mob_reset_before(ch->in_room, atoi(arg1)) )
		  {
			send_to_char("You must have a mob reset BEFORE an object can be given to one.\n\r", ch);
			return;
		  }
		  pReset = new_reset_data();
		  if (get_obj_index(atoi(arg3)) == NULL)
		  {
			send_to_char( "That vnum does not exist.\n\r",ch);
			return;
		  }
		  pReset->arg1 = atoi(arg3);
		  pReset->arg3 = flag_value( wear_loc_flags, arg4 );
		  if ( pReset->arg3 == WEAR_NONE )
			pReset->command = 'G';
		  else
			pReset->command = 'E';
		}
		}

		add_reset( ch->in_room, pReset, atoi( arg1 ) );
		SET_BIT( ch->in_room->area->area_flags, AREA_CHANGED );
		send_to_char( "Reset added.\n\r", ch );
	}
	else
	{
		send_to_char( "Syntax: RESET <number> OBJ <vnum> <wear_loc>\n\r", ch );
		send_to_char( "        RESET <number> OBJ <vnum> inside <vnum> [limit] [count]\n\r", ch );
		send_to_char( "        RESET <number> OBJ <vnum> room [limit] [count]\n\r", ch );
		send_to_char( "        RESET <number> MOB <vnum> [max # area] [max # room]\n\r", ch );
		send_to_char( "        RESET <number> DELETE\n\r", ch );
	}
	}

	return;
}


/*
 * For builders to return to their areas for building.
 */
void do_gotoarea( CHAR_DATA *ch, char *argument )
{
	ROOM_INDEX_DATA *location;
	AREA_DATA *pArea;
	CHAR_DATA *rch;
	int count = 0;
	int value = 0;

	if ( IS_NULLSTR(argument) ||  !is_number( argument ) )
	{
		send_to_char( "agoto which area?\n\r", ch );
		return;
	}

	value = atoi( argument );
	if ( !( pArea = get_area_data( value ) ) )
	{
		send_to_char( "That area vnum does not exist.\n\r", ch );
		return;
	}

	if(!IS_BUILDER(ch, pArea))
	{
	send_to_char("That is not your area.\n\r", ch);
	return;
	}

	value = pArea->min_vnum;
	if ( ( location = get_room_index( value ) ) == NULL )
	{
		send_to_char( "No such location.\n\r", ch );
		return;
	}

	count = 0;
	for(rch = location->people; rch != NULL; rch = rch->next_in_room)
		count++;

	if (!is_room_owner(ch,location) && room_is_private(location)
	&&  (count > 1 || get_trust(ch) < MAX_LEVEL))
	{
		send_to_char( "That room is private right now.\n\r", ch );
		return;
	}

	if ( ch->fighting != NULL )
		stop_fighting( ch, TRUE );

	for(rch = ch->in_room->people; rch != NULL; rch=rch->next_in_room)
	{
		if (get_trust(rch) >= ch->invis_level)
		{
			if (ch->pcdata != NULL && !IS_NULLSTR(ch->pcdata->bamfout))
				act("$t",ch,ch->pcdata->bamfout,rch,TO_VICT, 0);
			else
				act("$n leaves in a swirling mist.", ch,NULL,rch,TO_VICT,0);
		}
	}

	char_from_room( ch );
	char_to_room( ch, location );

	for(rch = ch->in_room->people; rch != NULL; rch=rch->next_in_room)
	{
		if (get_trust(rch) >= ch->invis_level)
		{
			if (ch->pcdata != NULL && !IS_NULLSTR(ch->pcdata->bamfin))
				act("$t",ch,ch->pcdata->bamfin,rch,TO_VICT,0);
			else
				act("$n appears in a swirling mist.", ch,NULL,rch,TO_VICT,0);
		}
	}

	do_look( ch, "auto" );
	return;
}


/*****************************************************************************
 Name:		do_myalist
 Purpose:	Normal command to list areas and display area information.
 Called by:	interpreter(interp.c)
 ****************************************************************************/
void do_myalist( CHAR_DATA *ch, char *argument )
{
	AREA_DATA *pArea;
	char result [ MSL*2 ]={'\0'};	/* May need tweaking. */

	if(IS_NPC(ch))
		return;

	if(ch->pcdata->security <= 0)
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	send_to_char(Format("\n\r[%3s] [%-27s] (%-5s-%5s) [%-10s] %3s [%-10s]\n\r", "Num", "Area Name", "lvnum", "uvnum", "Filename", "Sec", "Builders"), ch);

	for ( pArea = area_first; pArea; pArea = pArea->next )
	{
		if(strstr(pArea->builders, ch->name))
		{
			strncat( result, Format("[%3d] %-29.29s (%-5d-%5d) %-12.12s [%d] [%-10.10s]\n\r",
					pArea->vnum, pArea->name, pArea->min_vnum, pArea->max_vnum, pArea->file_name, pArea->security, pArea->builders), sizeof(result) );
		}
	}

	send_to_char( result, ch );
	return;
}

/*****************************************************************************
 Name:		do_alist
 Purpose:	Normal command to list areas and display area information.
 Called by:	interpreter(interp.c)
 ****************************************************************************/
void do_alist( CHAR_DATA *ch, char *argument )
{
	AREA_DATA *pArea;

	if(IS_NPC(ch))
		return;

	send_to_char(Format("\n\r[%3s] [%-27s] [%-5s-%5s] [%-10s] %3s [%-10s]\n\r",
			"Num", "Area Name", "lvnum", "uvnum", "Filename", "Sec", "Builders"), ch);

	for ( pArea = area_first; pArea; pArea = pArea->next )
	{
		send_to_char(Format("[%3d] %-29.29s (%-5d-%5d) %-12.12s [%d] [%-10.10s]\n\r",
				pArea->vnum, pArea->name, pArea->min_vnum, pArea->max_vnum, pArea->file_name, pArea->security, pArea->builders), ch);
	}

	return;
}

/*****************************************************************************
 Name:		do_plist
 Purpose:	Normal command to list plots and display plot information.
 Called by:	interpreter(interp.c)
 ****************************************************************************/
void do_plist( CHAR_DATA *ch, char *argument )
{
	PLOT_DATA *pPlot;

	if(IS_NPC(ch))
		return;

	send_to_char(Format("\n\r[%3s] [%-27s] [%-10s]\n\r", "Num", "Plot Title", "Authors"), ch);

	for ( pPlot = plot_list; pPlot; pPlot = pPlot->next )
	{
		send_to_char (Format("[%3d] %-29.29s [%-10.10s]\n\r", pPlot->vnum, pPlot->title, pPlot->author), ch);
	}

	return;
}

/*****************************************************************************
 Name:		do_elist
 Purpose:	Normal command to list events and display event information.
 Called by:	interpreter(interp.c)
 ****************************************************************************/
void do_elist( CHAR_DATA *ch, char *argument )
{
	EVENT_DATA *pEvent;

	send_to_char(Format("\n\r[%3s] [%-27s] [%-10s]\n\r", "Num", "Title", "Authors"), ch);

	for ( pEvent = event_list; pEvent; pEvent = pEvent->next )
	{
		send_to_char(Format("[%3d] %-29.29s [%-10.10s]\n\r", pEvent->vnum, pEvent->title, pEvent->author), ch);
	}

	return;
}

/*****************************************************************************
 Name:		do_slist
 Purpose:	Normal command to list scripts and display script information.
 Called by:	interpreter(interp.c)
 ****************************************************************************/
void do_slist( CHAR_DATA *ch, char *argument )
{
	SCRIPT_DATA *pScript;
	MOB_INDEX_DATA *mob;

	send_to_char(Format("\n\r[%3s] [%-27s] (%-27s) [%-10s] [%-10s]\n\r", "Num", "Trigger", "Reaction", "Event", "Actor"), ch);

	for ( pScript = script_first; pScript; pScript = pScript->next )
	{
		if(pScript->active)
		{
			continue;
		}
		if((mob = get_mob_index(pScript->actor)) != NULL)
		{
			send_to_char( Format("[%3d] %-29.29s (%-29.29s) [%d] [%-29.29s]\n\r",
					pScript->vnum, pScript->trigger, pScript->reaction,
					pScript->event ? pScript->event->vnum : -1, pScript->actor > 0 ? mob->short_descr : "None"), ch);
		}
	}

	return;
}


/*****************************************************************************
 Name:		do_ailist
 Purpose:	Normal command to list personas and display persona information.
 Called by:	interpreter(interp.c)
 ****************************************************************************/
void do_ailist( CHAR_DATA *ch, char *argument )
{
	PERSONA *pPersona;

	send_to_char(Format("\n\r[%3s] [%-27s]\n\r", "Num", "Name"), ch);

	for ( pPersona = persona_first; pPersona; pPersona = pPersona->next )
	{
		send_to_char(Format("[%3d] %-29.29s\n\r", pPersona->vnum, pPersona->name), ch);
	}

	return;
}

int count_reactions(REACT_DATA *r1)
{
	REACT_DATA *p;
	int i = 0;

	for(p = r1; p; p = p->next)
	{
		i++;
	}

	return i;
}

/*****************************************************************************
 Name:		do_rslist
 Purpose:	Normal command to list scripts and display script information.
		Scripts listed in a fairly messy way, working on this.
 Called by:	interpreter(interp.c)
 ****************************************************************************/
void do_rslist( CHAR_DATA *ch, char *argument )
{
	REACT *pScript;
	char result [ MSL*2 ]={'\0'};	/* May need tweaking. */
	int i = 0;

	send_to_char(Format("\n\r[%-27s] [%3s] (%3s) [%-27s] [%-7s]\n\r", "Persona", "Num", "Att", "Trigger", "Reacts"), ch);

	for ( pScript = react_first; pScript; pScript = pScript->next )
	{
		i++;
		strncat( result, Format("[%-29.29s] [%3d] (%3d) %-29.29s [%7d]\n\r",
				pScript->persona ? pScript->persona->name : "None", i, pScript->attitude, pScript->trig, count_reactions(pScript->react)), sizeof(result) );
	}

	send_to_char( result, ch );
	return;
}

void format_descriptions (AREA_DATA *pArea)
{
	ROOM_INDEX_DATA *rm;
	OBJ_INDEX_DATA *obj;
	MOB_INDEX_DATA *mob;
	int i = 0;

	for(i = pArea->min_vnum; i <= pArea->max_vnum; i++)
	{
		if((rm = get_room_index(i)) != NULL)
		{
			rm->description = format_string( rm->description );
			if(rm->udescription != NULL)
				rm->udescription = format_string( rm->udescription );
			if(rm->ddescription != NULL)
				rm->ddescription = format_string( rm->ddescription );
		}
		if((obj = get_obj_index(i)) != NULL)
		{
			obj->full_desc = format_string( obj->full_desc );
		}
		if((mob = get_mob_index(i)) != NULL)
		{
			mob->description = format_string( mob->description );
		}
	}
}

void check_area(AREA_DATA *pArea, CHAR_DATA *ch)
{
	ROOM_INDEX_DATA *rm;
	/*
	OBJ_INDEX_DATA *obj;
	MOB_INDEX_DATA *mob;
	 */
	int i = 0, j = 0;
	char buf[MSL]={'\0'};
	int Found = FALSE;

	for(i = pArea->min_vnum; i <= pArea->max_vnum; i++)
	{
		Found = FALSE;
		if((rm = get_room_index(i)) != NULL)
		{
			snprintf(buf, sizeof(buf), "Room %d has ", i);
			if(!str_cmp(rm->name, "(null)") || rm->name == NULL)
			{
				Found = TRUE;
				strncat(buf, "no name", sizeof(buf));
			}
			if(!str_cmp(rm->description, "(null)") || rm->description == NULL)
			{
				if(Found == TRUE)
				{
					strncat(buf, ", ", sizeof(buf));
				}
				Found = TRUE;
				strncat(buf, "no description", sizeof(buf));
			}
			if(!str_cmp(rm->uname, "(null)") || rm->uname == NULL)
			{
				if(Found == TRUE)
				{
					strncat(buf, ", ", sizeof(buf));
				}
				Found = TRUE;
				strncat(buf, "no umbra name", sizeof(buf));
			}
			if(!str_cmp(rm->udescription, "(null)") || rm->udescription == NULL)
			{
				if(Found == TRUE)
				{
					strncat(buf, ", ", sizeof(buf));
				}
				Found = TRUE;
				strncat(buf, "no umbra description", sizeof(buf));
			}
			if(!str_cmp(rm->dname, "(null)") || rm->dname == NULL)
			{
				if(Found == TRUE)
				{
					strncat(buf, ", ", sizeof(buf));
				}
				Found = TRUE;
				strncat(buf, "no dream name", sizeof(buf));
			}
			if(!str_cmp(rm->ddescription, "(null)") || rm->ddescription == NULL)
			{
				if(Found == TRUE)
				{
					strncat(buf, ", ", sizeof(buf));
				}
				Found = TRUE;
				strncat(buf, "no dream description", sizeof(buf));
			}
			if(Found == TRUE)
			{
				strncat(buf, ".\n\r", sizeof(buf));
				send_to_char(buf, ch);
			}
			Found = FALSE;
			for(j = 0; j < MAX_EXITS; j++)
			{
				snprintf(buf, sizeof(buf), "%s Exit has ", exit_table[j].name);
				if(rm->exit[j] != NULL)
				{
					if(rm->exit[j]->u1.to_room == NULL)
					{
						Found = TRUE;
						strncat(buf, "no target room", sizeof(buf));
					}
				}
				if(Found == TRUE)
				{
					strncat(buf, ".\n\r", sizeof(buf));
					send_to_char(buf, ch);
				}
			}
		}
	}
}

void do_area_checkup (CHAR_DATA *ch, char *argument)
{
	AREA_DATA *pArea;

	if(IS_NULLSTR(argument))
	{
		send_to_char("Which area are you giving a checkup?\n\r", ch);
		return;
	}

	if(!str_cmp(argument, "all"))
	{
		for(pArea = area_first; pArea; pArea = pArea->next)
		{
			format_descriptions(pArea);
			check_area(pArea, ch);
		}
		send_to_char("All areas formatted.\n\r", ch);
		do_asave(ch, "world");
		return;
	}
	else if(is_number(argument))
	{
		if((pArea = get_area_data(atoi(argument))) == NULL)
		{
			send_to_char("No area has that vnum.\n\r", ch);
			return;
		}

		format_descriptions(pArea);
		send_to_char("Area formatted.\n\r", ch);
		check_area(pArea, ch);
		do_asave(ch, "world");
		return;
	}

	send_to_char("Which area?\n\r", ch);
	return;
}

void available_room_vnums(CHAR_DATA *ch, AREA_DATA *pArea)
{
	ROOM_INDEX_DATA *rm;
	int i = 0, last_vnum = 0, place = 0;

	last_vnum = pArea->min_vnum;
	send_to_char("Room vnums available:\n\r", ch);
	for(i = pArea->min_vnum; i <= pArea->max_vnum + 1; i++)
	{

		if((rm = get_room_index(i)) == NULL || i > pArea->max_vnum)
		{
			if(last_vnum == (i - 1) && i <= pArea->max_vnum)
			{
				if(!place)
					place = last_vnum;
				last_vnum = i;
			}
			else
			{
				if(place)
				{
					send_to_char(Format("-%d", last_vnum), ch);
					place = 0;
					last_vnum = i;
				}
				else
				{
					send_to_char(Format(" %d", i), ch);
					last_vnum = i;
					place = i;
				}
			}
		}
		else
		{
			if(place)
			{
				send_to_char(Format("-%d", last_vnum), ch);
				place = 0;
			}
		}
	}
	send_to_char("\n\r", ch);
}


void available_obj_vnums(CHAR_DATA *ch, AREA_DATA *pArea)
{
	OBJ_INDEX_DATA *obj;
	int i = 0, last_vnum = 0, place = 0;

	last_vnum = pArea->min_vnum;
	send_to_char("Object vnums available:\n\r", ch);
	for(i = pArea->min_vnum; i <= pArea->max_vnum + 1; i++)
	{
		if((obj = get_obj_index(i)) == NULL || i > pArea->max_vnum)
		{
			if(last_vnum == (i - 1) && i <= pArea->max_vnum)
			{
				if(!place)
					place = last_vnum;
				last_vnum = i;
			}
			else
			{
				if(place)
				{
					send_to_char(Format("-%d", last_vnum), ch);
					place = 0;
					last_vnum = i;
				}
				else
				{
					send_to_char(Format(" %d", i), ch);
					last_vnum = i;
					place = i;
				}
			}
		}
		else
		{
			if(place)
			{
				send_to_char(Format("-%d", last_vnum), ch);
				place = 0;
			}
		}
	}
	send_to_char("\n\r", ch);
}


void available_mob_vnums(CHAR_DATA *ch, AREA_DATA *pArea)
{
	MOB_INDEX_DATA *mob;
	int i = 0, last_vnum = 0, place = 0;

	last_vnum = pArea->min_vnum;
	send_to_char("Mobile vnums available:\n\r", ch);
	for(i = pArea->min_vnum; i <= pArea->max_vnum + 1; i++)
	{
		if((mob = get_mob_index(i)) == NULL || i > pArea->max_vnum)
		{
			if(last_vnum == (i - 1) && i <= pArea->max_vnum)
			{
				if(!place)
					place = last_vnum;
				last_vnum = i;
			}
			else
			{
				if(place)
				{
					send_to_char(Format("-%d", last_vnum), ch);
					place = 0;
					last_vnum = i;
				}
				else
				{
					send_to_char(Format(" %d", i), ch);
					last_vnum = i;
					place = i;
				}
			}
		}
		else
		{
			if(place)
			{
				send_to_char(Format("-%d", last_vnum), ch);
				place = 0;
			}
		}
	}
	send_to_char("\n\r", ch);
}


void do_area_vnums (CHAR_DATA *ch, char *argument)
{
	AREA_DATA *pArea;
	int j = 0;

	if(ch->pcdata->security <= 0)
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if(IS_NULLSTR(argument))
	{
		send_to_char("Which area do you want the available vnums for?\n\r", ch);
		return;
	}

	if(!str_cmp(argument, "all"))
	{
		for(j = 0; (pArea = get_area_data(j)) != NULL; j++)
		{
			if(IS_BUILDER(ch, pArea))
			{
				send_to_char(Format("Area: [%d] %s\n\r", pArea->vnum, pArea->name), ch);
				available_room_vnums(ch, pArea);
				available_obj_vnums(ch, pArea);
				available_mob_vnums(ch, pArea);
				send_to_char("\n\r", ch);
			}
		}

		return;
	}
	if(!str_cmp(argument, "my"))
	{
		for(j = 0; (pArea = get_area_data(j)) != NULL; j++)
		{
			if(strstr(pArea->builders, ch->name))
			{
				send_to_char(Format("Area: [%d] %s\n\r", pArea->vnum, pArea->name), ch);
				available_room_vnums(ch, pArea);
				available_obj_vnums(ch, pArea);
				available_mob_vnums(ch, pArea);
				send_to_char("=====================================================\n\r", ch);
			}
		}

		return;
	}
	else if(is_number(argument))
	{
		if((pArea = get_area_data(atoi(argument))) == NULL)
		{
			send_to_char("No area has that vnum.\n\r", ch);
			return;
		}

		if(!IS_BUILDER(ch, pArea))
		{
			send_to_char("You can't see available vnums in that area.\n\r", ch);
			return;
		}

		send_to_char(Format("Area: [%d] %s\n\r", pArea->vnum, pArea->name), ch);
		available_room_vnums(ch, pArea);
		available_obj_vnums(ch, pArea);
		available_mob_vnums(ch, pArea);

		return;
	}

	send_to_char("Which area?\n\r", ch);
	return;
}


void do_myavnum (CHAR_DATA *ch, char *argument)
{
	do_function(ch, do_area_vnums, "my");
}


/* What was note.c */

/* globals from db.c for load_notes */
#if !defined(macintosh)
extern  int     _filbuf         args( (FILE *) );
#endif
extern FILE *                  fpArea;
extern char                    strArea[MAX_INPUT_LENGTH];

/* local procedures */
void load_thread(char *name, NOTE_DATA **list, int type, time_t free_time);
void parse_note(CHAR_DATA *ch, char *argument, int type);
bool hide_note(CHAR_DATA *ch, NOTE_DATA *pnote);

extern NOTE_DATA *note_list;
NOTE_DATA *bg_list;
NOTE_DATA *know_list;
NOTE_DATA *news_list;
NEWSPAPER *paper_list;

int count_spool(CHAR_DATA *ch, NOTE_DATA *spool)
{
	int count = 0;
	NOTE_DATA *pnote;

	for (pnote = spool; pnote != NULL; pnote = pnote->next)
		if (!hide_note(ch,pnote))
			count++;

	return count;
}

void do_unread(CHAR_DATA *ch)
{
	int count = 0;
	bool found = FALSE;

	if (IS_NPC(ch))
		return;

	if ((count = count_spool(ch,note_list)) > 0)
	{
		found = TRUE;
		send_to_char(Format("You have %d new note%s waiting.\n\r", count, count > 1 ? "s" : ""), ch);
	}

	if (!found)
		send_to_char("You have no unread notes.\n\r",ch);
}

void do_note(CHAR_DATA *ch,char *argument)
{
	parse_note(ch,argument,NOTE_NOTE);
}

void do_bg(CHAR_DATA *ch,char *argument)
{
	NOTE_DATA *pNote;
	char arg1[MIL]={'\0'};
	char *fullarg;

	fullarg = str_dup(argument);
	argument = one_argument( argument, arg1 );

	if ( !str_cmp( arg1, "create" ) )
	{
		if ( IS_NULLSTR(arg1) )
		{
			send_to_char( "Syntax:  bg create [keywords]\n\r", ch );
			return;
		}

		if ( bedit_create( ch, argument ) )
		{
			ch->desc->editor = ED_BG;
		}
		return;
	}
	else if(!str_prefix(arg1, "list"))
	{
		parse_note(ch, "list", NOTE_BACKGROUND);
		return;
	}
	else if(!str_prefix(arg1, "delete"))
	{
		if(IS_TRUSTED(ch, MASTER))
		{
			parse_note(ch, (char *)Format("delete %s", argument), NOTE_BACKGROUND);
		}
		else
		{
			send_to_char("You can't do that.\n\r", ch);
		}
		return;
	}
	else if(arg1[0] !='\0')
	{

		if ( (pNote = get_kbg( arg1, NOTE_BACKGROUND )) == NULL )
		{
			send_to_char( "Background:  That background does not exist.\n\r", ch );
			return;
		}

		ch->desc->pEdit = (void *)pNote;
		ch->desc->editor = ED_BG;
		return;
	}

	send_to_char( "Background:  There is no default background to edit.\n\r(Use bg create to create a new background.)\n\r", ch );
	return;
}

void do_know(CHAR_DATA *ch,char *argument)
{
	NOTE_DATA *pNote;
	char arg1[MIL]={'\0'};
	char *fullarg;

	fullarg = str_dup(argument);
	argument = one_argument( argument, arg1 );

	if ( !str_cmp( arg1, "create" ) )
	{
		if ( IS_NULLSTR(arg1) )
		{
			send_to_char( "Syntax:  fact create [keywords]\n\r", ch );
			return;
		}

		if ( kedit_create( ch, argument ) )
		{
			ch->desc->editor = ED_KNOW;
		}
		return;
	}
	else if(!str_prefix(arg1, "list"))
	{
		parse_note(ch, "list", NOTE_KNOWLEDGE);
		return;
	}
	else if(!str_prefix(arg1, "submit"))
	{
		parse_note(ch, "post", NOTE_KNOWLEDGE);
		return;
	}
	else if(!str_prefix(arg1, "delete"))
	{
		if(IS_TRUSTED(ch, MASTER))
		{
			parse_note(ch, (char *)Format("delete %s", argument), NOTE_KNOWLEDGE);
		}
		else
		{
			send_to_char("You can't do that.\n\r", ch);
		}
		return;
	}
	else if(arg1[0] !='\0')
	{

		if ( (pNote = get_kbg( arg1, NOTE_KNOWLEDGE )) == NULL )
		{
			send_to_char( "Knowledge:  That knowledge does not exist.\n\r", ch );
			return;
		}

		ch->desc->pEdit = (void *)pNote;
		ch->desc->editor = ED_KNOW;
		return;
	}

	send_to_char( "Fact:  There is no default fact to edit.\n\r(Use fact create to create a new fact entry.)\n\r", ch );
	return;
}

void save_notes(int type)
{
	FILE *fp;
	NOTE_DATA *pnote;
	char *name;

	switch (type)
	{
	default:
		return;
	case NOTE_NOTE:
		name = NOTE_FILE;
		pnote = note_list;
		break;
	case NOTE_BACKGROUND:
		name = BG_FILE;
		pnote = bg_list;
		break;
	case NOTE_KNOWLEDGE:
		name = KNOW_FILE;
		pnote = know_list;
		break;
	case NOTE_ARTICLE:
		name = NEWS_FILE;
		pnote = news_list;
		break;
	}

	closeReserve();
	if ( ( fp = fopen( name, "w" ) ) == NULL )
	{
		perror( name );
	}
	else
	{
		for ( ; pnote != NULL; pnote = pnote->next )
		{
			if(type == NOTE_NOTE) {
				fprintf( fp, "Sender  %s~\n", pnote->sender);
				fprintf( fp, "Date    %s~\n", pnote->date);
				fprintf( fp, "Stamp   %ld\n", pnote->date_stamp);
				fprintf( fp, "To      %s~\n", pnote->to_list);
				fprintf( fp, "Subject %s~\n", pnote->subject);
			}
			else if(type == NOTE_BACKGROUND || type == NOTE_KNOWLEDGE)
			{
				fprintf( fp, "Author  %s~\n", pnote->sender);
				fprintf( fp, "Date    %s~\n", pnote->date);
				fprintf( fp, "Stamp   %ld\n", pnote->date_stamp);
				fprintf( fp, "Keyword %s~\n", pnote->to_list);
				fprintf( fp, "Diff    %s~\n", pnote->subject);
			}
			else
			{
				fprintf( fp, "Author  %s~\n", pnote->sender);
				fprintf( fp, "Date    %s~\n", pnote->date);
				fprintf( fp, "Stamp   %ld\n", pnote->date_stamp);
				fprintf( fp, "Categ   %s~\n", pnote->to_list);
				fprintf( fp, "Subject %s~\n", pnote->subject);
			}
			fprintf( fp, "Success %d~\n", pnote->successes);
			fprintf( fp, "Text\n%s~\n",   pnote->text);
		}
		fclose( fp );
		openReserve();
		return;
	}
}

void load_notes(void)
{
	load_thread(NOTE_FILE,&note_list, NOTE_NOTE, 14*24*60*60);
	load_thread(BG_FILE,&bg_list, NOTE_BACKGROUND, 14*24*60*60);
	load_thread(KNOW_FILE,&know_list, NOTE_KNOWLEDGE, 14*24*60*60);
	load_thread(NEWS_FILE,&news_list, NOTE_ARTICLE, 14*24*60*60);
}

void load_thread(char *name, NOTE_DATA **list, int type, time_t free_time)
{
	FILE *fp;
	NOTE_DATA *pnotelast;
	char *tmp;

//	ALLOC_DATA(pnotelast, NOTE_DATA, 1);

	if ( ( fp = fopen( name, "r" ) ) == NULL ) {
//		PURGE_DATA(pnotelast);
		return;
	}

	pnotelast = NULL;
	for ( ; ; )
	{
		NOTE_DATA *pnote;
		char letter;

		do
		{
			letter = getc( fp );
			if ( feof(fp) )
			{
				fclose( fp );
				return;
			}
		}
		while ( isspace(letter) );
		ungetc( letter, fp );

		ALLOC_DATA(pnote, NOTE_DATA, 1);

		tmp = fread_word(fp);
		if ( str_cmp( tmp, "sender" ) && str_cmp( tmp, "author" ) )
		{
			free_note(pnote);
			break;
		}

		pnote->sender   = fread_string( fp );

		if ( str_cmp( fread_word( fp ), "date" ) )
				{
						free_note(pnote);
						break;
				}
		pnote->date     = fread_string( fp );

		if ( str_cmp( fread_word( fp ), "stamp" ) )
				{
						free_note(pnote);
						break;
				}
		pnote->date_stamp = fread_number(fp);

		tmp = fread_word(fp);
		if ( str_cmp( tmp, "to" )
				&& str_cmp( tmp, "keyword" )
				&& str_cmp( tmp, "categ" ) )
				{
						free_note(pnote);
						break;
				}
		pnote->to_list  = fread_string( fp );

		tmp = fread_word(fp);
		if ( str_cmp( tmp, "subject" )
				&& str_cmp( tmp, "diff" ) )
				{
						free_note(pnote);
						break;
				}
		pnote->subject  = fread_string( fp );

		if ( str_cmp( fread_word( fp ), "success" ) )
			pnote->successes = 1;
		else {
			const char * s = fread_string(fp);
			pnote->successes = atoi(s);
			PURGE_DATA(s);
		}
		if ( str_cmp( fread_word( fp ), "text" ) )
				{
						free_note(pnote);
						break;
				}
		pnote->text     = fread_string( fp );

		if (type == NOTE_NOTE && free_time
				&& pnote->date_stamp < current_time - free_time)
		{
			free_note(pnote);
			continue;
		}

		pnote->type = type;

		if (*list == NULL)
			*list = pnote;
		else
			pnotelast->next = pnote;

		pnotelast       = pnote;
	}

	if(type == NOTE_NOTE)
		strncpy( strArea, NOTE_FILE, sizeof(strArea) );
	else if(type == NOTE_BACKGROUND)
		strncpy( strArea, BG_FILE, sizeof(strArea) );
	else if(type == NOTE_KNOWLEDGE)
		strncpy( strArea, KNOW_FILE, sizeof(strArea) );
	else if(type == NOTE_ARTICLE)
		strncpy( strArea, NEWS_FILE, sizeof(strArea) );
	fpArea = fp;
	log_string(LOG_BUG, "Load_notes: bad key word.");
	exit( 1 );
	return;
}

void append_note(NOTE_DATA *pnote)
{
	FILE *fp;
	NOTE_DATA **list;
	NOTE_DATA *last;
	char *name;

	switch(pnote->type)
	{
	default:
		return;
	case NOTE_NOTE:
		name = NOTE_FILE;
		list = &note_list;
		break;
	case NOTE_BACKGROUND:
		name = BG_FILE;
		list = &bg_list;
		break;
	case NOTE_KNOWLEDGE:
		name = KNOW_FILE;
		list = &know_list;
		break;
	case NOTE_ARTICLE:
		name = NEWS_FILE;
		list = &news_list;
		break;
	}

	if (*list == NULL)
		*list = pnote;
	else
	{
		for ( last = *list; last->next != NULL; last = last->next);
		last->next = pnote;
	}

	closeReserve();
	if ( ( fp = fopen(name, "a" ) ) == NULL )
	{
		perror(name);
	}
	else
	{
		fprintf( fp, "Sender  %s~\n", pnote->sender);
		fprintf( fp, "Date    %s~\n", pnote->date);
		fprintf( fp, "Stamp   %ld\n", pnote->date_stamp);
		fprintf( fp, "To      %s~\n", pnote->to_list);
		fprintf( fp, "Subject %s~\n", pnote->subject);
		fprintf( fp, "Success %d~\n", pnote->successes);
		fprintf( fp, "Text\n%s~\n", pnote->text);
		fclose( fp );
	}
	openReserve();
}

bool is_note_to( CHAR_DATA *ch, NOTE_DATA *pnote )
{
	ORG_DATA *org;

	if ( !str_cmp( ch->name, pnote->sender ) )
		return TRUE;

	if ( is_exact_name( "all", pnote->to_list ) )
		return TRUE;

	if ( IS_ADMIN(ch)
			&& (is_exact_name( "admin", pnote->to_list )
					|| is_exact_name("staff", pnote->to_list)
					|| is_exact_name("immortal", pnote->to_list) ))
		return TRUE;

	if ( (IS_ADMIN(ch) || ch->pcdata->security > 0)
			&& is_exact_name( "builder", pnote->to_list ) )
		return TRUE;

	if ( IS_ADMIN(ch)
			&& ( pnote->type == NOTE_BACKGROUND || pnote->type == NOTE_KNOWLEDGE
					|| pnote->type == NOTE_ARTICLE ) )
		return TRUE;

	if ( is_exact_name( race_table[ch->race].name, pnote->to_list ) )
		return TRUE;

	if ( is_exact_name( clan_table[ch->clan].name, pnote->to_list ) )
		return TRUE;

	for(org = org_list; org; org = org->next)
	{
		if( mem_lookup(org, ch->name) )
		{
			if ( is_exact_name( org->name, pnote->to_list ) )
				return TRUE;
		}
	}

	if ( is_exact_name( ch->pack, pnote->to_list ) )
		return TRUE;

	if ( ch->trust > 3 )
		return TRUE;

	if( IS_SET(ch->plr_flags, PLR_ELDER)
			&& (is_exact_name("elder",pnote->to_list)
					|| is_exact_name("elders",pnote->to_list)) )
		return TRUE;

	if ( is_exact_name( ch->name, pnote->to_list ) )
		return TRUE;

	return FALSE;
}


void note_attach( CHAR_DATA *ch, int type )
{
	NOTE_DATA *pnote;

	if ( ch->pnote != NULL )
	return;

	pnote = new_note();

	pnote->next		= NULL;
	PURGE_DATA( pnote->sender );
	PURGE_DATA( pnote->date );
	PURGE_DATA( pnote->to_list );
	PURGE_DATA( pnote->subject );
	PURGE_DATA( pnote->text );
	pnote->sender	= str_dup( ch->name );
	pnote->date		= NULL;
	pnote->to_list	= NULL;
	pnote->subject	= NULL;
	pnote->text		= NULL;
	pnote->type		= type;
	if(type == NOTE_ARTICLE)
	pnote->successes = 0;
	ch->pnote		= pnote;
	return;
}



void note_remove( CHAR_DATA *ch, NOTE_DATA *pnote, bool bdelete)
{
	NOTE_DATA *prev;
	NOTE_DATA **list;
	char to_new[MIL]={'\0'};
	char to_one[MIL]={'\0'};
	char *to_list;

	if (!bdelete)
	{
	/* make a new list */
		to_new[0]	= '\0';
		to_list	= pnote->to_list;
		while ( *to_list != '\0' )
		{
			to_list	= one_argument( to_list, to_one );
			if ( !IS_NULLSTR(to_one) && str_cmp( ch->name, to_one ) )
		{
			strncat( to_new, " ", sizeof(to_new) );
			strncat( to_new, to_one, sizeof(to_new) );
		}
		}
		/* Just a simple recipient removal? */
	   if ( str_cmp( ch->name, pnote->sender ) && !IS_NULLSTR(to_new) )
	   {
		   PURGE_DATA( pnote->to_list );
	   pnote->to_list = str_dup( to_new + 1 );
	   return;
	   }
	}
	/* nuke the whole note */

	switch(pnote->type)
	{
	default:
		return;
	case NOTE_NOTE:
		list = &note_list;
		break;
	case NOTE_BACKGROUND:
		list = &bg_list;
		break;
	case NOTE_KNOWLEDGE:
		list = &know_list;
		break;
	case NOTE_ARTICLE:
		list = &news_list;
		break;
	}

	/*
	 * Remove note from linked list.
	 */
	if ( pnote == *list )
	{
	*list = pnote->next;
	}
	else
	{
	for ( prev = *list; prev != NULL; prev = prev->next )
	{
		if ( prev->next == pnote )
		break;
	}

	if ( prev == NULL )
	{
		log_string(LOG_BUG, "Note_remove: pnote not found.");
		return;
	}

	prev->next = pnote->next;
	}

	save_notes(pnote->type);
	free_note(pnote);
	return;
}

bool hide_note (CHAR_DATA *ch, NOTE_DATA *pnote)
{
	time_t last_read;

	if (IS_NPC(ch))
	return TRUE;

	switch (pnote->type)
	{
	default:
		return TRUE;
	case NOTE_NOTE:
		last_read = ch->pcdata->last_note;
		break;
	case NOTE_BACKGROUND:
	case NOTE_KNOWLEDGE:
		if(IS_ADMIN(ch))
		return FALSE;
		else
		return TRUE;
	}
	
	if (pnote->date_stamp <= last_read)
	return TRUE;

	if (!str_cmp(ch->name,pnote->sender))
	return TRUE;

	if (!is_note_to(ch,pnote))
	return TRUE;

	return FALSE;
}

void update_read(CHAR_DATA *ch, NOTE_DATA *pnote)
{
	time_t stamp;

	CheckChNPC(ch);

	stamp = pnote->date_stamp;

	switch (pnote->type)
	{
		default:
			return;
		case NOTE_NOTE:
		ch->pcdata->last_note = UMAX(ch->pcdata->last_note,stamp);
			break;
	}
}

void parse_note( CHAR_DATA *ch, char *argument, int type )
{
	NOTE_DATA *pnote;
	NOTE_DATA **list;
	DESCRIPTOR_DATA *d;
	int vnum = 0;
	int anum = 0;
	char arg[MIL]={'\0'};
	char arg2[MIL]={'\0'};
	char *list_name;

	CheckChNPC(ch);

	switch(type)
	{
	default:
		return;
	case NOTE_NOTE:
		list = &note_list;
		list_name = "notes";
		break;
	case NOTE_BACKGROUND:
		list = &bg_list;
		list_name = "backgrounds";
		break;
	case NOTE_KNOWLEDGE:
		list = &know_list;
		list_name = "knowledges";
		break;
	case NOTE_ARTICLE:
		list = &news_list;
		list_name = "articles";
		break;
	}

	argument = one_argument( argument, arg );
	smash_tilde( argument );

	if ( IS_NULLSTR(arg) || !str_prefix( arg, "read" ) )
	{
		bool fAll;

		if ( !str_cmp( argument, "all" ) )
		{
			fAll = TRUE;
			anum = 0;
		}

		else if ( IS_NULLSTR(argument) || !str_prefix(argument, "next"))
			/* read next unread note */
		{
			vnum = 0;
			for ( pnote = *list; pnote != NULL; pnote = pnote->next)
			{
				if (!hide_note(ch,pnote))
				{
					send_to_char( Format("From: %s\n\rTo: %s\n\rDate: %s\n\rMessage Number: [%3d]\n\rSubject Line:%s\n\r",
							pnote->sender, pnote->to_list, pnote->date, vnum, pnote->subject), ch );
					page_to_char( pnote->text, ch );
					update_read(ch,pnote);
					return;
				}
				else if (is_note_to(ch,pnote))
					vnum++;
			}
			if(type == NOTE_NOTE)
			{
				send_to_char(Format("You have no unread %s.\n\r",list_name), ch);
			}
			else if(type == NOTE_BACKGROUND || type == NOTE_KNOWLEDGE || type == NOTE_ARTICLE)
			{
				send_to_char(Format("Try using %s read [number] for reading.\n\r", list_name), ch);
			}
			return;
		}

		else if ( is_number( argument ) )
		{
			fAll = FALSE;
			anum = atoi( argument );
		}
		else
		{
			send_to_char( "Read which number?\n\r", ch );
			return;
		}

		vnum = 0;
		for ( pnote = *list; pnote != NULL; pnote = pnote->next )
		{
			if ( is_note_to( ch, pnote ) && ( vnum++ == anum || fAll ) )
			{
				if(type == NOTE_ARTICLE)
				{
					send_to_char( Format("[%3d] %s: %s\n\r", vnum -1, pnote->sender, pnote->subject), ch );
					send_to_char( Format("%s\n\r", pnote->date), ch );
					send_to_char( Format("Category: %s\n\r", pnote->to_list), ch);
					send_to_char( Format("Suppression Level: %d\n\r", pnote->successes), ch);
				}
				else
				{
					send_to_char( Format("From: %s\n\r", pnote->sender), ch );
					send_to_char( Format("To:   %s\n\r", pnote->to_list), ch );
					send_to_char( Format("Date: %s\n\r", pnote->date), ch );
					send_to_char( Format("Message Number: [%3d]", vnum - 1), ch );
					send_to_char( Format("Subject Line: %s\n\r", pnote->subject), ch);
					send_to_char("\tW----------\tn\n\r", ch);
				}
				page_to_char( pnote->text, ch );
				update_read(ch,pnote);
				return;
			}
		}

		send_to_char(Format("There aren't that many %s.\n\r", list_name), ch);
		return;
	}

	if ( !str_prefix( arg, "list" ) )
	{
		vnum = 0;

		if(type ==NOTE_NOTE)
		{
			send_to_char( "\tWMessage | From            | To          | Subject Line\tn\n\r", ch );
			send_to_char( "\tW------------------------------------------------------\tn\n\r", ch );
		}

		if(type == NOTE_BACKGROUND || type == NOTE_KNOWLEDGE)
		{
			send_to_char( "\tW ID #   | Keywords\tn\n\r", ch);
			send_to_char( "\tW------------------------------------------------------\tn\n\r", ch );
		}

		if(type == NOTE_ARTICLE)
		{
			send_to_char("\tWArticle | Author | Category | Subject\tn\n\r", ch);
			send_to_char( "\tW------------------------------------------------------\tn\n\r", ch );

		}

	for ( pnote = *list; pnote != NULL; pnote = pnote->next )
	{
		if ( is_note_to( ch, pnote ) )
		{
			if(type == NOTE_NOTE)
			{
				send_to_char( Format("\tY[%3d%s]  \tW|\tY %-15s \tW|\tY %-11s \tW|\tY %s\tn\n\r", vnum, hide_note(ch,pnote) ? " " : "N", pnote->sender, pnote->to_list, pnote->subject), ch );
			}
			else if(type == NOTE_BACKGROUND || type == NOTE_KNOWLEDGE)
			{
				send_to_char( Format("\tY%3d%s    \tW|\tY %s\tn\n\r", vnum, hide_note(ch,pnote) ? " " : "N", pnote->to_list), ch );
			}
			else if(type == NOTE_ARTICLE)
			{
				send_to_char( Format("\tY %6d \tW|\tY %s  \tW|\tY %s: %s %s\tn\n\r", vnum, pnote->sender, pnote->to_list, pnote->subject, pnote->successes ? " (\tYSuppressed\tn)" : ""), ch );
			}
			vnum++;
		}
	}
	if (!vnum)
	{
		switch(type)
		{
		case NOTE_NOTE:	
			send_to_char("There are no notes for you.\n\r",ch);
			break;
		case NOTE_BACKGROUND:
			if(IS_ADMIN(ch))
				send_to_char("There are no background entries.\n\r",ch);
			else
				send_to_char("Huh?\n\r",ch);
			break;
		case NOTE_KNOWLEDGE:
			if(IS_ADMIN(ch))
				send_to_char("There are no fact entries.\n\r",ch);
			else
				send_to_char("Huh?\n\r",ch);
			break;
		case NOTE_ARTICLE:
			if(IS_ADMIN(ch))
				send_to_char("There are no newspaper articles.\n\r",ch);
			else
				send_to_char("Huh?\n\r",ch);
			break;
		}
	}
	return;
	}

	if ( !str_prefix( arg, "suppression" ))
	{
		if(type == NOTE_ARTICLE)
		{
			argument = one_argument(argument, arg2);
			if ( !is_number( argument ) || !is_number( arg2 )  )
			{
				send_to_char(
						"Which article's suppression are you trying to affect?\n\r",
						ch );
				send_to_char(
						"And what number of successes are you setting it to?\n\r", ch);
				send_to_char(
						"(Use 0 for unsuppressed)\n\r", ch);
				return;
			}

			anum = atoi( arg2 );

			vnum = 0;
			for ( pnote = *list; pnote != NULL; pnote = pnote->next )
			{
				if ( is_note_to( ch, pnote ) && ( vnum++ == anum ) )
				{
					pnote->successes = atoi(argument);
					save_notes(pnote->type);
					send_to_char("Suppression level set.\n\r", ch);
					return;
				}
			}

			send_to_char("There aren't that many articles.\n\r",ch);
			return;
		}
	}

	if ( !str_prefix( arg, "search" ) )
	{
	argument = one_argument(argument, arg2);
	vnum = 0;
	for ( pnote = *list; pnote != NULL; pnote = pnote->next )
	{
		if ( is_note_to( ch, pnote ) )
		{
		  if(((!str_cmp(arg2, "to")
		|| !str_cmp(arg2, "keywords")
		|| !str_cmp(arg2, "category"))
		&& strstr(pnote->to_list, argument))
		  || ((!str_cmp(arg2, "subject")
		|| !str_cmp(arg2, "difficulty"))
		&& strstr(pnote->subject, argument))
		  || ((!str_cmp(arg2, "text")
		|| !str_cmp(arg2, "body")
		|| !str_cmp(arg2, "contents"))
		&& strstr(pnote->text, argument))
		  || ((!str_cmp(arg2, "from")
		|| !str_cmp(arg2, "by")
		|| !str_cmp(arg2, "sender")
		|| !str_cmp(arg2, "author"))
		&& !str_cmp(pnote->sender, argument))
		  || (!str_cmp(arg2, "suppressed")
		&& type == NOTE_ARTICLE
		&& pnote->successes))
		  {
			  if(type == NOTE_NOTE)
			  {
				  send_to_char( Format("[%3d%s] %s: %s\n\r", vnum, hide_note(ch,pnote) ? " " : "N", pnote->sender, pnote->subject), ch );
			  }
			  else if(type == NOTE_BACKGROUND || type == NOTE_KNOWLEDGE)
			  {
				  send_to_char( Format("[%3d%s] %s: %s\n\r", vnum, hide_note(ch,pnote) ? " " : "N", pnote->sender, pnote->to_list), ch );
			  }
			  else if(type == NOTE_ARTICLE)
			  {
				  send_to_char( Format("[%3d %s] %s: %s%s\n\r", vnum, pnote->sender, pnote->to_list, pnote->subject, pnote->successes ? " (\tYSuppressed\tn)" : ""), ch );
			  }
		  }
		  vnum++;
		}
	}
	if (!vnum)
	{
		switch(type)
		{
		case NOTE_NOTE:	
			send_to_char("Search found no notes.\n\r",ch);
			break;
		case NOTE_BACKGROUND:
			if(IS_ADMIN(ch))
			send_to_char("Search found no background entries.\n\r",ch);
			else
			send_to_char("Huh?\n\r",ch);
			break;
		case NOTE_KNOWLEDGE:
			if(IS_ADMIN(ch))
			send_to_char("Search found no knowledge entries.\n\r",ch);
			else
			send_to_char("Huh?\n\r",ch);
			break;
		case NOTE_ARTICLE:
			if(IS_ADMIN(ch))
			send_to_char("Search found no newspaper articles.\n\r",ch);
			else
			send_to_char("Huh?\n\r",ch);
			break;
		}
	}
	return;
	}

	if ( !str_prefix( arg, "remove" ) )
	{
		if ( !is_number( argument ) )
		{
			send_to_char( "Remove which number?\n\r", ch );
			return;
		}
 
		anum = atoi( argument );
		vnum = 0;
		for ( pnote = *list; pnote != NULL; pnote = pnote->next )
		{
			if ( is_note_to( ch, pnote ) && vnum++ == anum )
			{
				note_remove( ch, pnote, FALSE );
				send_to_char( "Ok.\n\r", ch );
				return;
			}
		}
 
	send_to_char(Format("There aren't that many %s.",list_name),ch);
		return;
	}
 
	if ( !str_prefix( arg, "delete" ) && get_trust(ch) >= MAX_LEVEL - 1)
	{
		if ( !is_number( argument ) )
		{
			send_to_char( "Note delete which number?\n\r", ch );
			return;
		}
 
		anum = atoi( argument );
		vnum = 0;
		for ( pnote = *list; pnote != NULL; pnote = pnote->next )
		{
			if ( is_note_to( ch, pnote ) && vnum++ == anum )
			{
				note_remove( ch, pnote,TRUE );
				send_to_char( "Ok.\n\r", ch );
				return;
			}
		}

	send_to_char(Format("There aren't that many %s.",list_name), ch);
		return;
	}

	if ( !str_prefix( arg, "body" ) )
	{
	note_attach( ch,type );
	if (ch->pnote->type != type)
	{
		send_to_char("You already have a note, background, knowledge, or newspaper\n\r",ch);
		send_to_char("article in progress.\n\r",ch);
		return;
	}

	if(IS_NULLSTR(argument))
	{
		string_append( ch, &ch->pnote->text );
		return;
	}

	send_to_char("Syntax: note body\n\rThis will load the string editor.\n\r", ch);
	return;
	}

	if ( !str_prefix( arg, "subject" ) || !str_prefix(arg, "difficulty") )
	{
	note_attach( ch,type );
		if (ch->pnote->type != type)
		{
		send_to_char("You already have a note, background, knowledge, or newspaper\n\r",ch);
		send_to_char("article in progress.\n\r",ch);
			return;
		}

		if (!is_number(argument) && (ch->pnote->type == NOTE_BACKGROUND
		|| ch->pnote->type == NOTE_KNOWLEDGE))
	{
		send_to_char("Difficulty must be a number from 1 to 10.\n\r",ch);
		return;
	}
	else if(is_number(argument) && (ch->pnote->type == NOTE_BACKGROUND
		|| ch->pnote->type == NOTE_KNOWLEDGE))
	{
		if( 1 > atoi(argument) || atoi(argument) > 10 )
		{
		send_to_char("Difficulty must be a number from 1 to 10.\n\r",ch);
		return;
		}
	}

		PURGE_DATA( ch->pnote->subject );
	ch->pnote->subject = str_dup( argument );
	send_to_char( "Ok.\n\r", ch );
	return;
	}

	if ( !str_prefix( arg, "to" ) || !str_prefix( arg, "keywords" )
	|| !str_prefix( arg, "category" ) )
	{
	note_attach( ch,type );
		if (ch->pnote->type != type)
		{
		send_to_char("You already have a note, background, knowledge, or newspaper\n\r",ch);
		send_to_char("article in progress.\n\r",ch);
			return;
		}
		PURGE_DATA( ch->pnote->to_list );
	ch->pnote->to_list = str_dup( argument );
	send_to_char( "Ok.\n\r", ch );
	return;
	}

	if ( !str_prefix( arg, "clear" ) )
	{
	if ( ch->pnote != NULL )
	{
		free_note(ch->pnote);
		ch->pnote = NULL;
	}

	send_to_char( "Ok.\n\r", ch );
	return;
	}

	if ( !str_prefix( arg, "show" ) )
	{
	if ( ch->pnote == NULL )
	{
		send_to_char( "You have no note, background, knowledge, or newspaper\n\r",ch);
		send_to_char("article in progress.\n\r",ch);
		return;
	}

	if (ch->pnote->type != type)
	{
		send_to_char("You aren't working on that kind of text.\n\r",ch);
		return;
	}

	if(type == NOTE_NOTE)
	{
		send_to_char(Format("%s: %s\n\rTo: %s\n\r", ch->pnote->sender, ch->pnote->subject, ch->pnote->to_list), ch);
	}
	else if (type == NOTE_BACKGROUND || type == NOTE_KNOWLEDGE)
	{
		send_to_char(Format("Author: %s Difficulty: %s\n\rKeywords: %s\n\r", ch->pnote->sender, ch->pnote->subject, ch->pnote->to_list), ch);
	}
	else if (type == NOTE_ARTICLE)
	{
		send_to_char(Format("Author: %s Subject: %s\n\rCategory: %s\n\r", ch->pnote->sender, ch->pnote->subject, ch->pnote->to_list), ch);
	}
	send_to_char( ch->pnote->text, ch );
	return;
	}

	if ( !str_prefix( arg, "post" ) || !str_prefix(arg, "send"))
	{
	char *strtime;

	if ( ch->pnote == NULL )
	{
		send_to_char( "You have no note in progress.\n\r", ch );
		return;
	}

		if (ch->pnote->type != type)
		{
			send_to_char("You aren't working on that kind of note.\n\r",ch);
			return;
		}

	if (!str_cmp(ch->pnote->to_list,""))
	{
		if(type == NOTE_NOTE)
		send_to_char(
		"You need to provide a recipient (name, all, or admin).\n\r",
		ch);
		else if(type == NOTE_BACKGROUND)
		send_to_char("You need to provide at least one keyword or room vnum.\n\r", ch);
		else if(type == NOTE_KNOWLEDGE)
		send_to_char("You need to provide at least one keyword.\n\r", ch);
		else if(type == NOTE_ARTICLE) {
		send_to_char("Article placed in general news stream.\n\r", ch);
		PURGE_DATA(ch->pnote->to_list);
		ch->pnote->to_list = str_dup("General");
		}
		return;
	}

	if (!str_cmp(ch->pnote->subject,""))
	{
		if(type == NOTE_NOTE || type == NOTE_ARTICLE)
		send_to_char("You need to provide a subject.\n\r",ch);
		else if(type == NOTE_BACKGROUND || type == NOTE_KNOWLEDGE)
		send_to_char("You need to provide a difficulty.\n\r", ch);
		return;
	}

	ch->pnote->next			= NULL;
	strtime				= ctime( &current_time );
	strtime[strlen(strtime)-1]	= '\0';
	PURGE_DATA( ch->pnote->date );
	ch->pnote->date			= str_dup( strtime );
	ch->pnote->date_stamp		= current_time;

	if(type == NOTE_NOTE || type == NOTE_ARTICLE)
		send_to_char("Posted.\n\r",ch);
	else if(type == NOTE_BACKGROUND || type == NOTE_KNOWLEDGE)
		send_to_char("Added.\n\r", ch);

		for(d = descriptor_list; d != NULL; d = d->next) {
		if((type == NOTE_ARTICLE || type == NOTE_BACKGROUND
		|| type == NOTE_KNOWLEDGE)
		&& (IS_ADMIN(d->character)
		  || (d->original != NULL && IS_ADMIN(d->original)))) {
		send_to_char(Format("A new item has been added to %s by %s\n\r", list_name, ch->name), d->character);
		}
		else
		{
		if(d->character && is_note_to(d->character, ch->pnote))
			send_to_char("A new note has been delivered.\n\r", d->character);
		}
	}

		append_note(ch->pnote);
		ch->pnote = NULL;
		return;
	}

	send_to_char( "You can't do that.\n\r", ch );
	return;
}

NOTE_DATA * find_bg_keyword(char *arg)
{
	NOTE_DATA *pbg;

	for(pbg = bg_list; pbg != NULL; pbg = pbg->next)
	{
		if(is_name(arg, pbg->to_list))
			return pbg;
	}

	return NULL;
}

NOTE_DATA * find_knowledge_keyword(char *arg)
{
	NOTE_DATA *pbg;

	for(pbg = know_list; pbg != NULL; pbg = pbg->next)
	{
		if(is_name(arg, pbg->to_list))
			return pbg;
	}

	return NULL;
}

void load_papers()
{
	FILE *fp;
	NEWSPAPER *pnewslast;
	int i = 0;
	bool No_Problem = FALSE;

	if ( ( fp = fopen( PAPER_FILE, "r" ) ) == NULL )
		return;

	pnewslast = NULL;
	for ( ; ; )
	{
		NEWSPAPER *pnews;
		char letter;

		No_Problem = FALSE;

		do
		{
			letter = getc( fp );
			if ( feof(fp) )
			{
				fclose( fp );
				update_news_stands();
				return;
			}
		}
		while ( isspace(letter) );
		ungetc( letter, fp );

		ALLOC_DATA(pnews, NEWSPAPER, 1);

		if ( str_cmp( fread_word( fp ), "Name" ))
			break;
		pnews->name   = fread_string( fp );

		if ( str_cmp( fread_word( fp ), "States" ))
			break;
		pnews->on_stands   = fread_number( fp );
		pnews->cost	   = fread_number( fp );

		if ( str_cmp( fread_word( fp ), "Articles" ))
			break;
		for(i=0;i<MAX_ARTICLES;i++)
			pnews->articles[i]   = fread_number( fp );

		if (paper_list == NULL)
			paper_list           = pnews;
		else
			pnewslast->next     = pnews;

		pnewslast       = pnews;

		No_Problem = TRUE;
	}

	if(!No_Problem)
	{
		strncpy( strArea, PAPER_FILE, sizeof(strArea) );
		fpArea = fp;
		log_string(LOG_BUG, "Load_papers: bad key word.");
		exit( 1 );
	}

	return;
}

void save_papers()
{
	FILE *fp;
	NEWSPAPER *pnews;
	int i = 0;

	closeReserve();
	if ( ( fp = fopen( PAPER_FILE, "w" ) ) == NULL )
	{
		perror( PAPER_FILE );
	}
	else
	{
		for ( pnews = paper_list ; pnews != NULL; pnews = pnews->next )
		{
			fprintf( fp, "Name  %s~\n", pnews->name);
			fprintf( fp, "States %d %d\n", pnews->on_stands, pnews->cost);
			fprintf( fp, "Articles");
			for(i=0;i<MAX_ARTICLES;i++)
				fprintf(fp, " %ld", pnews->articles[i]);
			fprintf( fp, "\n");
		}
		fclose( fp );
		openReserve();
		return;
	}
}

void load_stocks()
{
	FILE *fp;
	STOCKS *pstocklast;
	bool No_Problem = FALSE;

	if ( ( fp = fopen( STOCK_FILE, "r" ) ) == NULL )
		return;

	pstocklast = NULL;
	for ( ; ; )
	{
		STOCKS *pstock;
		char letter;

		No_Problem = FALSE;

		do
		{
			letter = getc( fp );
			if ( feof(fp) )
			{
				fclose( fp );
				return;
			}
		}
		while ( isspace(letter) );
		ungetc( letter, fp );

		ALLOC_DATA(pstock, STOCKS, 1);

		if ( str_cmp( fread_word( fp ), "Name" ))
			break;
		pstock->name   = fread_string(fp);

		if ( str_cmp( fread_word( fp ), "Tick" ))
			break;
		pstock->ticker = fread_string(fp);

		if ( str_cmp( fread_word( fp ), "Last_Change" ))
			break;
		pstock->last_change =fread_number( fp );

		if ( str_cmp( fread_word( fp ), "Stock_Price" ))
			break;
		pstock->cost =fread_number( fp );

		if ( str_cmp( fread_word( fp ), "Stock_Phase" ))
			break;
		pstock->phase =fread_number( fp );

		if ( str_cmp( fread_word( fp ), "UporDown" ))
			break;
		pstock->upordown =fread_number( fp );

		if (stock_list == NULL)
			stock_list           = pstock;
		else
			pstocklast->next     = pstock;

		pstocklast       = pstock;

		No_Problem = TRUE;
	}

	if(!No_Problem)
	{
		strncpy( strArea, STOCK_FILE, sizeof(strArea) );
		fpArea = fp;
		log_string(LOG_BUG, "Load_stocks: bad key word.");
		exit( 1 );
	}

	return;
}

void save_stocks()
{
	FILE *fp;
	STOCKS *pstock;

	closeReserve();
	if ( ( fp = fopen( STOCK_FILE, "w" ) ) == NULL )
	{
		perror( STOCK_FILE );
	}
	else
	{
		for ( pstock = stock_list ; pstock != NULL; pstock = pstock->next )
		{
			fprintf( fp, "Name  %s~\n", pstock->name);
			fprintf( fp, "Tick  %s~\n", pstock->ticker);
			fprintf( fp, "Last_Change  %ld\n", pstock->last_change);
			fprintf( fp, "Stock_Price  %d\n", pstock->cost);
			fprintf( fp, "Stock_Phase  %d\n", pstock->phase);
			fprintf( fp, "UporDown  %d\n", pstock->upordown);
		}
		fclose( fp );
		openReserve();
		return;
	}
}


void cleanse_builder_stuff(CHAR_DATA *ch)
{
	CHAR_DATA *mob, *nextmob;
	OBJ_DATA *obj, *nextobj;

	for(mob = char_list; mob; mob = nextmob)
	{
		nextmob = mob->next;

		if(IS_BUILDERMODE(mob) && mob->master == ch)
		{
			extract_char(mob, TRUE);
		}
	}

	for(obj = object_list; obj; obj = nextobj)
	{
		nextobj = obj->next;

		if(IS_BUILDERMODE(obj) && !str_cmp(obj->owner, ch->name))
		{
			extract_obj(obj);
		}
	}

	send_to_char("Builder mode objects and mobs purged.\n\r", ch);
}

void do_buildmode(CHAR_DATA *ch, char *argument)
{
	if(IS_SET(ch->comm, COMM_BUILDER))
	{
		REMOVE_BIT(ch->comm, COMM_BUILDER);
		send_to_char("Builder mode off.\n\r", ch);
		cleanse_builder_stuff(ch);
		return;
	}

	if(ch->pcdata->security <= 0)
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if(IS_SET(ch->act2, ACT2_STORY))
	{
		send_to_char("You must turn off storyteller mode first.\n\r", ch);
		return;
	}
	SET_BIT(ch->comm, COMM_BUILDER);
	send_to_char("Builder mode on.\n\r", ch);
}

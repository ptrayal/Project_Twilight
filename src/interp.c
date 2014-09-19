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
#include <stdlib.h>
#include <time.h>
#include "twilight.h"
#include "tables.h"
#include "interp.h"


bool	check_social	args( ( CHAR_DATA *ch, char *command,
				char *argument ) );

/*
* Command logging types.
*/
#define L_NRM	0 /* Normal */
#define L_ALL	1 /* Always */
#define L_NEV	2 /* Never  */
#define L_COM	3 /* Communications */
#define L_SCOM	4 /* Staff Communications */

/*
* Log-all switch.
*/
bool				fLogAll		= FALSE;

/*
* Log-coms switch.
*/
bool				fLogCom		= TRUE;

/*
* Log-all coms switch.
*/
bool				fLogAllCom	= TRUE;

/*
* Command table.
*/
const	struct	cmd_type	cmd_table	[] =
{
		/* Commented out commands go here */
		/*  { "clan",			do_clantalk,	P_SLEEP,	0, L_NRM, 1, 1,  		0},*/
		/* 	{ "spells",			do_spells,		P_DEAD,		0, L_NRM, 1, 0, 0},*/
		/*  { "channels",		do_channels,	P_DEAD,		0, L_NRM, 1, A|B, 0}, */
		/*  { "scroll",			do_lines,		P_DEAD,		0, L_NRM, 1, A|B, 0},*/
		/*  { "unlock",			do_unlock,		P_REST,		0, L_NRM, 1, B|H, 0}, */
		/*  { "backstab",		do_backstab,	P_FIGHT,	0, L_NRM, 1, H, 0}, */
		/*  { "bs",				do_backstab,	P_FIGHT,	0, L_NRM, 0, H, 0}, */
		/*  { "dirt",			do_dirt,		P_FIGHT,	0, L_NRM, 1, H, 0}, */
		/*  { "disarm",			do_disarm,		P_FIGHT,	0, L_NRM, 1, H, 0}, */
		/*  { "kick",			do_kick,		P_FIGHT,	0, L_NRM, 1, H, 0}, */
		/*  { "rescue",			do_rescue,		P_FIGHT,	0, L_NRM, 0, H, 0}, */
		/*  { "trip",			do_trip,		P_FIGHT,	0, L_NRM, 1, H, 0}, */
		/*  { "group",			do_group,		P_SLEEP,	0, L_COM, 1, B, 0}, */
		/*  { "hide",			do_hide,		P_REST,		0, L_NRM, 1, B, 0},*/

		/*
		 * Commands which are not immortal nor race specific
		 */
		{	"east",			do_east,		P_STAND,	0,	L_NEV,	0,	A|B|H,	0 },
		{	"west",			do_west,		P_STAND,	0,	L_NEV,	0,	A|B|H,	0 },
		{	"north",		do_north,		P_STAND,	0,	L_NEV,	0,	A|B|H,	0 },
		{	"south",		do_south,		P_STAND,	0,	L_NEV,	0,	A|B|H,	0 },
		{	"up",			do_up,			P_STAND,	0,	L_NEV,	0,	A|B|H,	0 },
		{	"down",			do_down,		P_STAND,	0,	L_NEV,	0,	A|B|H,	0 },


		{	"'",			do_say,			P_REST,		0,	L_NRM,	0,	B|H,	0 },
		{	"-",			do_dsay,		P_REST,		0,	L_NRM,	0,	B|H,	0 },
		{	",",			do_emote,		P_REST,		0,	L_NRM,	0,	B,	0 },
		{	".",			do_yell,		P_REST,		0,	L_NRM,	0,	B|H,	0 },
		{	"+",			do_buildertalk,	P_DEAD,		0,	L_SCOM,	0,	A|B|E,	0 },
		{	"=",			do_oocsay,		P_REST,		0,	L_COM,	0,	B,	0 },
		{	">",			do_oocchan,		P_DEAD,		0,	L_COM,	0,	A|B|E,	0 },
		{	"abilities",	do_abilities,	P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"accept",		do_accept,		P_REST,		0,	L_NRM,	1,	0,	0 },
		{	"address",		do_addy,		P_DEAD,		0,	L_ALL,	1,	A|B|E,	0 },
		{	"advance",		do_train_power,	P_STAND,	0,	L_NRM,	1,	B|H,	0 },
		{	"aedit",		do_aedit,		P_DEAD,		0,	L_NRM,	2,	A|B|E,	0 },
		{	"affects",		do_affects,		P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"afk",			do_afk,			P_TORPOR,	0,	L_NRM,	1,	A|B,	0 },
		{	"ageset",		do_setage,		P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"agoto",		do_gotoarea,	P_DEAD,		0,	L_NRM,	2,	A|B|E,	0 },
		{	"alias",		do_alias,		P_DEAD,		0,	L_COM,	1,	A|B|E,	0 },
		{	"applicants",	do_applicants,	P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"apply",		do_apply,		P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"archetypes",	do_archetypes,	P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"areas",		do_areas,		P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"asave",		do_asave,		P_DEAD,		0,	L_NRM,	2,	A|B|E,	0 },
		{	"attack",		do_kill,		P_FIGHT,	0,	L_NRM,	0,	A|B,	0 },
		{	"attacks",		do_fight,		P_STAND,	0,	L_NRM,	0,	A|B,	0 },
		{	"authorise",	do_authflag,	P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"avnums",		do_area_vnums,	P_DEAD,		0,	L_NRM,	2,	A|B|E,	0 },
		{	"backgrounds",	do_backgrounds,	P_REST,		0,	L_NRM,	1,	B|H,	0 },
		{	"backstab",		do_fight,		P_STAND,	0,	L_NRM,	0,	A|B,	0 },
		{	"bail",			do_bail,		P_REST,		0,	L_NRM,	1,	A|B|H,	0 },
		{	"balance",		do_balance,		P_DEAD,		0,	L_NRM,	1,	B,	0 },
		{	"bandage",		do_bandage,		P_SIT,		0,	L_NRM,	1,	B,	0 },
		{	"bite",			do_move_bite,	P_FIGHT,	0,	L_NRM,	1,	H,	0 },
		{	"block",		do_block,		P_DEAD,		0,	L_NRM,	1,	A|B|H,	0 },
		{	"boot",			do_boot,		P_SLEEP,	0,	L_NRM,	1,	A|B|E,	0 },
		{	"brawl",		do_fight,		P_STAND,	0,	L_NRM,	0,	A|B,	0 },
		{	"bribe",		do_bribe,		P_REST,		0,	L_NRM,	1,	B|H,	0 },
		{	"brief",		do_brief,		P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"bug",			do_bug,			P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"build",		do_orgbuild,	P_SIT,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"builderhelp",	do_buildhelp,	P_DEAD,		0,	L_NRM,	2,	A|B|E,	0 },
		{	"buildermode",	do_buildmode,	P_DEAD,		0,	L_NRM,	2,	A|B|E,	0 },
		{	"buildertalk",	do_buildertalk,	P_DEAD,		0,	L_SCOM,	2,	A|B|E,	0 },
		{	"bury",			do_conceal,		P_REST,		0,	L_NRM,	1,	B|H,	0 },
		{	"buy",			do_buy,			P_REST,		0,	L_NRM,	1,	B|H,	0 },
		{	"buygiftxp",	do_ooctogift,	P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"call",			do_call,		P_REST,		0,	L_COM,	1,	B|H,	0 },
		{	"calm",			do_calm,		P_FIGHT,	0,	L_NRM,	1,	A|B|E,	0 },
		{	"cash",			do_cash,		P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"cast",			do_cast,		P_FIGHT,	0,	L_NRM,	1,	A|B|E|H,	0 },
		{	"celebrities",	do_celebs,		P_TORPOR,	0,	L_NRM,	1,	A|B|E,	0 },
		{	"cflip",		do_move_flip,	P_FIGHT,	0,	L_NRM,	1,	H,	0 },
		{	"channels",		do_channels,	P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"chase",		do_chase,		P_STAND,	0,	L_NRM,	1,	A|B|H,	0 },
		{	"cjump",		do_move_jump,	P_FIGHT,	0,	L_NRM,	1,	H,	0 },
		{	"close",		do_close,		P_REST,		0,	L_NRM,	1,	B|H,	0 },
		{	"combat",		do_fight,		P_STAND,	0,	L_NRM,	0,	A|B,	0 },
		{	"combine",		do_combine,		P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"commands",		do_commands,	P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"compact",		do_compact,		P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"compare",		do_compare,		P_STAND,	0,	L_NRM,	0,	A|B,	0 },
		{	"conceal",		do_conceal,		P_REST,		0,	L_NRM,	1,	B|H,	0 },
		{	"concede",		do_concede,		P_SLEEP,	0,	L_ALL,	1,	A|B|H,	0 },
		{	"consider",		do_consider,	P_REST,		0,	L_NRM,	1,	A|B,	0 },
		{	"costcalc",		do_trainingcost,	P_STAND,	0,	L_NRM,	1,	A|B|E,	0 },
		{	"count",		do_count,		P_SLEEP,	0,	L_NRM,	1,	A|B|E,	0 },
		{	"credits",		do_credits,		P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"croll",		do_move_roll,	P_FIGHT,	0,	L_NRM,	1,	H,	0 },
		{	"cstand",		do_move_stand,	P_SLEEP,	0,	L_NRM,	1,	H,	0 },
		{	"cstop",		do_move_stop,	P_FIGHT,	0,	L_NRM,	1,	0,	0 },
		{	"cx",			do_move_stop,	P_FIGHT,	0,	L_NRM,	1,	0,	0 },
		{	"defuse",		do_defuse,		P_SIT,		0,	L_NRM,	1,	A|B|H,	0 },
		{	"delete",		do_delete,		P_STAND,	0,	L_ALL,	1,	B|E,	0 },
		{	"demeanor",		do_demeanor,	P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"deposit",		do_deposit,		P_DEAD,		0,	L_NRM,	1,	B|H,	0 },
		{	"description",	do_description,	P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"disclaimers",	do_disclaim,	P_DEAD,		0,	L_NRM,	0,	A|B|E|G,	0 },
		{	"discreet",		do_discreet,	P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"dmote",		do_dmote,		P_REST,		0,	L_COM,	1,	B,	0 },
		{	"donate",		do_donate,		P_REST,		0,	L_NRM,	1,	A|B,	0 },
		{	"donate",		do_sacrifice,	P_DEAD,		0,	L_NRM,	0,	B|H,	0 },
		{	"dpmote",		do_dpmote,		P_REST,		0,	L_COM,	1,	B,	0 },
		{	"drink",		do_drink,		P_REST,		0,	L_NRM,	1,	B|H,	0 },
		{	"droll",		do_diceroll,	P_DEAD,		0,	L_NRM,	1,	A|B,	0 },
		{	"drop",			do_drop,		P_REST,		0,	L_NRM,	1,	B|H,	0 },
		{	"dsay",			do_dsay,		P_REST,		0,	L_COM,	1,	B|H,	0 },
		{	"eat",			do_eat,			P_REST,		0,	L_NRM,	1,	B|H,	0 },
		{	"edit",			do_olc,			P_DEAD,		0,	L_NRM,	2,	A|B,	0 },
		{	"email",		do_email,		P_DEAD,		0,	L_COM,	1,	A|B|E,	0 },
		{	"emote",		do_dmote,		P_REST,		0,	L_NRM,	1,	B,	0 },
		{	"enter",		do_enter,		P_STAND,	0,	L_NRM,	1,	A|B|H,	0 },
		{	"envenom",		do_envenom,		P_REST,		0,	L_NRM,	1,	B|H,	0 },
		{	"equipment",	do_equipment,	P_DEAD,		0,	L_NRM,	1,	B|E,	0 },
		{	"examine",		do_examine,		P_REST,		0,	L_NRM,	1,	A|B,	0 },
		{	"exits",		do_exits,		P_REST,		0,	L_NRM,	1,	A|B,	0 },
		{	"experience",	do_experience,	P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"fblock",		do_move_fblock,	P_FIGHT,	0,	L_NRM,	1,	H,	0 },
		{	"feed",			do_feed,		P_REST,		0,	L_NRM,	1,	B|D|H,	0 },
		{	"fight",		do_fight,		P_STAND,	0,	L_NRM,	0,	A|B,	0 },
		{	"fill",			do_fill,		P_REST,		0,	L_NRM,	1,	B|H,	0 },
		{	"finger",		do_whois,		P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"flee",			do_flee,		P_FIGHT,	0,	L_NEV,	1,	A|B|H,	0 },
		{	"follow",		do_follow,		P_REST,		0,	L_NRM,	1,	A|B|H,	0 },
		{	"frenzy",		do_fight,		P_STAND,	0,	L_NRM,	0,	A|B,	0 },
		{	"fx",			do_move_fblock,	P_FIGHT,	0,	L_NRM,	1,	H,	0 },
		{	"generation",	do_generation,	P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"get",			do_get,			P_REST,		0,	L_NRM,	1,	B|H,	0 },
		{	"gift",			do_xpgift,		P_DEAD,		0,	L_ALL,	1,	A|B|E,	0 },
		{	"give",			do_give,		P_REST,		0,	L_NRM,	1,	B|H,	0 },
		{	"glance",		do_glance,		P_REST,		0,	L_NRM,	1,	A|B,	0 },
		{	"group",		do_group,		P_SLEEP,	0,	L_COM,	1,	A|B|E,	0 },
		{	"gtell",		do_gtell,		P_DEAD,		0,	L_COM,	1,	B,	0 },
		{	"hangup",		do_hangup,		P_DEAD,		0,	L_COM,	1,	A|B|H,	0 },
		{	"help",			do_help,		P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"helpline",		do_helpline,	P_DEAD,		0,	L_COM,	1,	A|B|E,	0 },
		{	"hit",			do_kill,		P_FIGHT,	0,	L_NRM,	0,	A|B,	0 },
		{	"hold",			do_wear,		P_REST,		0,	L_NRM,	1,	B|H,	0 },
		{	"home",			do_home,		P_STAND,	0,	L_NRM,	1,	B,	0 },
		{	"hunt",			do_hunt,		P_STAND,	0,	L_NRM,	0,	B|H,	0 },
		{	"ignore",		do_ignore,		P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"incumbents",	do_incumbent,	P_REST,		0,	L_NRM,	1,	B,	0 },
		{	"induct",		do_induct,		P_SLEEP,	0,	L_NRM,	1,	A|B|E,	0 },
		{	"influences",	do_influences,	P_REST,		0,	L_NRM,	1,	B|H,	0 },
		{	"inventory",	do_inventory,	P_DEAD,		0,	L_NRM,	1,	B|E,	0 },
		{	"investigate",	do_investigate,	P_STAND,	0,	L_NRM,	1,	B|H,	0 },
		{	"jobs",			do_job,			P_REST,		0,	L_NRM,	1,	B|H,	0 },
		{	"jump",			do_jump,		P_REST,		0,	L_NRM,	1,	A|B|H,	0 },
		{	"kick",			do_move_kick,	P_FIGHT,	0,	L_NRM,	1,	H,	0 },
		{	"kill",			do_kill,		P_FIGHT,	0,	L_NRM,	1,	0,	0 },
		{	"knock",		do_knock,		P_STAND,	0,	L_NRM,	1,	B|H,	0 },
		{	"knowledges",	do_knowstats,	P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"look",			do_look,		P_REST,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"lastboot",		do_lastboot,	P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"lastname",		do_surname,		P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"laston",		do_laston,		P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"lblock",		do_move_lblock,	P_FIGHT,	0,	L_NRM,	1,	H,	0 },
		{	"lblow",		do_move_lblow,	P_FIGHT,	0,	L_NRM,	1,	H,	0 },
		{	"leave",		do_enter,		P_STAND,	0,	L_NRM,	1,	A|B|H,	0 },
		{	"levels",		do_level,		P_DEAD,		0,	L_NRM,	0,	A|B|E,	0 },
		{	"lgrab",		do_move_lgrab,	P_FIGHT,	0,	L_NRM,	1,	H,	0 },
		{	"lines",		do_lines,		P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"list",			do_list,		P_REST,		0,	L_NRM,	1,	B|E,	0 },
		{	"lock",			do_lock,		P_REST,		0,	L_NRM,	1,	B|H,	0 },
		{	"loot",			do_loot,		P_REST,		0,	L_NRM,	0,	B|H,	0 },
		{	"lx",			do_move_lblock,	P_FIGHT,	0,	L_NRM,	1,	H,	0 },
		{	"map",			do_map,			P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"mark",			do_mark,		P_REST,		0,	L_NRM,	1,	B|H,	0 },
		{	"medit",		do_medit,		P_DEAD,		0,	L_NRM,	2,	A|B,	0 },
		{	"melee",		do_fight,		P_STAND,	0,	L_NRM,	0,	A|B,	0 },
		{	"members",		do_members,		P_SLEEP,	0,	L_NRM,	1,	A|B|E,	0 },
		{	"mob",			do_mob,			P_DEAD,		0,	L_NEV,	0,	0,	0 },
		{	"motd",			do_motd,		P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"mpdump",		do_mpdump,		P_DEAD,		0,	L_NRM,	2,	A|B|E,	0 },
		{	"mpedit",		do_mpedit,		P_DEAD,		0,	L_NRM,	2,	A|B,	0 },
		{	"mpstat",		do_mpstat,		P_DEAD,		0,	L_NRM,	2,	A|B|E,	0 },
		{	"myareas",		do_myalist,		P_DEAD,		0,	L_NRM,	2,	A|B|E,	0 },
		{	"myavnums",		do_myavnum,		P_DEAD,		0,	L_NRM,	2,	A|B|E,	0 },
		{	"nature",		do_nature,		P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"newbie",		do_newbie,		P_STAND,	0,	L_NRM,	1,	B|H,	0 },
		{	"nofollow",		do_nofollow,	P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"nominate",		do_nominate,	P_REST,		0,	L_NRM,	1,	B,	0 },
		{	"nominees",		do_nominees,	P_REST,		0,	L_NRM,	1,	B,	0 },
		{	"notes",		do_note,		P_SLEEP,	0,	L_NRM,	1,	A|B|E,	0 },
		{	"occupation",	do_job,			P_REST,		0,	L_NRM,	1,	B|H,	0 },
		{	"oedit",		do_oedit,		P_DEAD,		0,	L_NRM,	2,	A|B,	0 },
		{	"olc",			do_olc,			P_DEAD,		0,	L_ALL,	2,	A|B,	0 },
		{	"oochannel",	do_oocchan,		P_DEAD,		0,	L_COM,	0,	A|B|E,	0 },
		{	"oocsay",		do_oocsay,		P_REST,		0,	L_COM,	1,	B,	0 },
		{	"open",			do_open,		P_REST,		0,	L_NRM,	1,	B|H,	0 },
		{	"order",		do_order,		P_REST,		0,	L_COM,	1,	H,	0 },
		{	"password",		do_password,	P_DEAD,		0,	L_NEV,	1,	A|B|E,	0 },
		{	"phone",		do_phone,		P_DEAD,		0,	L_NRM,	1,	A|B|E|H,	0 },
		{	"picklock",		do_pick,		P_REST,		0,	L_NRM,	1,	B|H,	0 },
		{	"pmote",		do_pmote,		P_REST,		0,	L_COM,	1,	B,	0 },
		{	"pour",			do_pour,		P_REST,		0,	L_NRM,	1,	B|H,	0 },
		{	"powers",		do_gifts,		P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"prefix",		do_prefix,		P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"premise",		do_premise,		P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"press",		do_button_press,	P_REST,	0,	L_NRM,	1,	B|H,	0 },
		{	"profession",	do_job,			P_REST,		0,	L_NRM,	1,	B|H,	0 },
		{	"prompt",		do_prompt,		P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"pullpin",		do_pullpin,		P_REST,		0,	L_NRM,	1,	A|B|H,	0 },
		{	"put",			do_put,			P_REST,		0,	L_NRM,	1,	B|H,	0 },
		{	"quaff",		do_quaff,		P_REST,		0,	L_NRM,	1,	B|H,	0 },
		{	"quiet",		do_quiet,		P_SLEEP,	0,	L_NRM,	1,	A|B|E,	0 },
		{	"quit",			do_quit,		P_DEAD,		0,	L_NRM,	1,	A|B,	0 },
		{	"racecommands",	do_racecmds,	P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"rblock",		do_move_rblock,	P_FIGHT,	0,	L_NRM,	1,	H,	0 },
		{	"rblow",		do_move_rblow,	P_FIGHT,	0,	L_NRM,	1,	H,	0 },
		{	"read",			do_read,		P_REST,		0,	L_NRM,	1,	A|B,	0 },
		{	"recite",		do_recite,		P_REST,		0,	L_NRM,	1,	B|H,	0 },
		{	"redit",		do_redit,		P_DEAD,		0,	L_NRM,	2,	A|B,	0 },
		{	"refuse",		do_refuse,		P_SLEEP,	0,	L_NRM,	1,	A|B|E,	0 },
		{	"reincarnate",	do_reinc,		P_SLEEP,	0,	L_NRM,	0,	A|B|E,	0 },
		{	"reject",		do_reject,		P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"reload",		do_reload,		P_REST,		0,	L_NRM,	1,	H,	0 },
		{	"remove",		do_remove,		P_REST,		0,	L_NRM,	1,	B|H,	0 },
		{	"repair",		do_repair,		P_SIT,		0,	L_NRM,	1,	B|H,	0 },
		{	"replay",		do_replay,		P_TORPOR,	0,	L_NRM,	1,	A|B|E,	0 },
		{	"reply",		do_reply,		P_DEAD,		0,	L_COM,	1,	A|B|E,	0 },
		{	"research",		do_research,	P_SIT,		0,	L_NRM,	1,	B|H,	0 },
		{	"resets",		do_resets,		P_DEAD,		0,	L_NRM,	2,	A|B,	0 },
		{	"resist",		do_resist,		P_DEAD,		0,	L_NRM,	1,	B|C|E,	0 },
		{	"rest",			do_rest,		P_SLEEP,	0,	L_NRM,	1,	B|H,	0 },
		{	"rgrab",		do_move_rgrab,	P_FIGHT,	0,	L_NRM,	1,	H,	0 },
		{	"rituals",		do_rituals,		P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"roomecho",		do_dpmote,		P_REST,		0,	L_COM,	0,	B,	0 },
		{	"rpavailable",	do_rp_ok,		P_DEAD,		0,	L_COM,	1,	A|B|E,	0 },
		{	"rpjoin",		do_rp_join,		P_REST,		0,	L_NRM,	1,	B,	0 },
		{	"rpmode",		do_rpmode,		P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"rpstep",		do_rp_join,		P_REST,		0,	L_NRM,	0,	B,	0 },
		{	"rpwho",		do_whorp,		P_DEAD,		0,	L_NRM,	0,	A|B|E,	0 },
		{	"rules",		do_rules,		P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"run",			do_run,			P_STAND,	0,	L_NEV,	1,	A|B|H,	0 },
		{	"rx",			do_move_rblock,	P_FIGHT,	0,	L_NRM,	1,	H,	0 },
		{	"sacrifice",	do_sacrifice,	P_DEAD,		0,	L_NRM,	0,	B|H,	0 },
		{	"save",			do_save,		P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"say",			do_say,			P_REST,		0,	L_COM,	1,	B|H,	0 },
		{	"sayto",		do_dsay,		P_REST,		0,	L_COM,	1,	B|H,	0 },
		{	"scan",			do_scan,		P_REST,		0,	L_NRM,	1,	A|B,	0 },
		{	"school",		do_goschool,	P_DEAD,		0,	L_NRM,	0,	A|B|H,	0 },
		{	"score",		do_score,		P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"search",		do_search,		P_SIT,		0,	L_NRM,	1,	A|B|H,	0 },
		{	"sell",			do_sell,		P_REST,		0,	L_NRM,	1,	B|H,	0 },
		{	"sentence",		do_sentence,	P_SLEEP,	0,	L_NRM,	1,	A|B|E,	0 },
		{	"shadow",		do_shadow,		P_REST,		0,	L_NRM,	1,	B|H,	0 },
		{	"shoot",		do_shoot,		P_SIT,		0,	L_NRM,	1,	H,	0 },
		{	"shoplift",		do_shoplift,	P_STAND,	0,	L_NRM,	1,	B|H,	0 },
		{	"sit",			do_sit,			P_SLEEP,	0,	L_NRM,	1,	B|H,	0 },
		{	"skick",		do_move_skick,	P_FIGHT,	0,	L_NRM,	1,	H,	0 },
		{	"skills",		do_skills,		P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"sleep",		do_sleep,		P_SLEEP,	0,	L_NRM,	1,	B|H,	0 },
		{	"smash",		do_smash,		P_STAND,	0,	L_NRM,	1,	B|H,	0 },
		{	"sneak",		do_sneak,		P_STAND,	0,	L_NRM,	1,	B|H,	0 },
		{	"socials",		do_socials,		P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"spin",			do_move_spin,	P_FIGHT,	0,	L_NRM,	1,	H,	0 },
		{	"staff",		do_wizlist,		P_DEAD,		0,	L_NRM,	0,	A|B|E,	0 },
		{	"stand",		do_stand,		P_SLEEP,	0,	L_NRM,	1,	B|H,	0 },
		{	"stats",		do_stats,		P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"status",		do_status,		P_SLEEP,	0,	L_NRM,	0,	A|B,	0 },
		{	"steal",		do_steal,		P_STAND,	0,	L_NRM,	1,	B|H,	0 },
		{	"step",			do_step,		P_STAND,	0,	L_NRM,	1,	B|H,	0 },
		{	"stocks",		do_stocks,		P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"storyteller",	do_storyteller,	P_DEAD,		0,	L_ALL,	1,	A|B|E,	0 },
		{	"strike",		do_fight,		P_STAND,	0,	L_NRM,	0,	A|B,	0 },
		{	"surrender",	do_surrender,	P_FIGHT,	0,	L_NRM,	1,	0,	0 },
		{	"sweep",		do_move_sweep,	P_FIGHT,	0,	L_NRM,	1,	H,	0 },
		{	"switchdesc",	do_switchdesc,	P_SLEEP,	0,	L_NRM,	1,	A|B|E,	0 },
		{	"take",			do_get,			P_REST,		0,	L_NRM,	1,	B|H,	0 },
		{	"talents",		do_talents,		P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"taxi",			do_taxi,		P_STAND,	0,	L_NRM,	0,	B|H,	0 },
		{	"tell",			do_tell,		P_REST,		0,	L_COM,	1,	A|B|E,	0 },
		{	"theme",		do_theme,		P_DEAD,		0,	L_NRM,	0,	A|B|E,	0 },
		{	"think",		do_think,		P_SLEEP,	0,	L_COM,	0,	A|B,	0 },
		{	"time",			do_time,		P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"tips",			do_tips,		P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"title",		do_title,		P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"toggle",		do_toggle,		P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"tongue",		do_move_tongue,	P_FIGHT,	0,	L_NRM,	1,	H,	0 },
		{	"touch",		do_move_touch,	P_FIGHT,	0,	L_NRM,	1,	H,	0 },
		{	"trade",		do_trade,		P_DEAD,		0,	L_NRM,	1,	B,	0 },
		{	"train",		do_train,		P_STAND,	0,	L_NRM,	1,	B|H,	0 },
		{	"traits",		do_traits,		P_STAND,	0,	L_NRM,	1,	B,	0 },
		{	"trip",			do_move_sweep,	P_FIGHT,	0,	L_NRM,	1,	H,	0 },
		{	"unalias",		do_unalias,		P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"unload",		do_unload,		P_REST,		0,	L_NRM,	1,	H,	0 },
		{	"unlock",		do_unlock,		P_REST,		0,	L_NRM,	1,	B|H,	0 },
		{	"unread",		do_unread,		P_SLEEP,	0,	L_NRM,	1,	A|B|E,	0 },
		{	"unwrap",		do_unwrap,		P_REST,		0,	L_NRM,	1,	A|B|H,	0 },
		{	"updatetime",	do_updatetime,	P_DEAD,		0,	L_NRM,	0,	A|B|E,	0 },
		{	"use",			do_use,			P_REST,		0,	L_NRM,	1,	A|B|H,	0 },
		{	"value",		do_value,		P_REST,		0,	L_NRM,	1,	B,	0 },
		{	"visible",		do_visible,		P_SLEEP,	0,	L_NRM,	1,	B,	0 },
		{	"vote",			do_vote,		P_REST,		0,	L_NRM,	1,	B,	0 },
		{	"wake",			do_wake,		P_SLEEP,	0,	L_NRM,	1,	B|H,	0 },
		{	"wear",			do_wear,		P_REST,		0,	L_NRM,	1,	B|H,	0 },
		{	"weather",		do_weather,		P_REST,		0,	L_NRM,	1,	A|B,	0 },
		{	"where",		do_where,		P_REST,		0,	L_NRM,	1,	B,	0 },
		{	"whisper",		do_whisper,		P_REST,		0,	L_NRM,	1,	A|B|H,	0 },
		{	"who",			do_who	,		P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"whoavailable",	do_whorp,		P_DEAD,		0,	L_NRM,	0,	A|B|E,	0 },
		{	"whois",		do_whois,		P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"whorp",		do_whorp,		P_DEAD,		0,	L_NRM,	0,	A|B|E,	0 },
		{	"wield",		do_wear,		P_REST,		0,	L_NRM,	1,	B|H,	0 },
		{	"wimpy",		do_nocommand,	P_DEAD,		0,	L_NRM,	0,	A|B,	0 },
		{	"withdraw",		do_withdrawal,	P_DEAD,		0,	L_NRM,	1,	B|H,	0 },
		{	"wizlist",		do_wizlist,		P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"wrap",			do_wrap,		P_REST,		0,	L_NRM,	1,	A|B|H,	0 },
		{	"xp",			do_experience,	P_DEAD,		0,	L_NRM,	1,	A|B|E,	0 },
		{	"yell",			do_yell,		P_REST,		0,	L_COM,	1,	B|H,	0 },


	/*
	* Class & Clan Commands.
	*/
	/* I put these in order of function name so I can keep like disciplines together. */
	/* Modified Dreamwalk for testing purposes */
	{	"activate",			do_activate,		P_REST,		0,	L_NRM,	1,	A|B|G|H,	WW },
	{	"ancestors",		do_ancestor,		P_STAND,	0,	L_NRM,	1,	A|B|C,	WW },
	{	"feralwhisper",		do_animalism1,		P_REST,		0,	L_NRM,	1,	B|C|G|H,	VA },
	{	"beckoning",		do_animalism2,		P_REST,		0,	L_NRM,	1,	A|B|C|G,	VA|WW },
	{	"song",				do_animalism3,		P_REST,		0,	L_NRM,	1,	A|B|C|G|H,	VA },
	{	"cobraeyes",		do_animalism3,		P_REST,		0,	L_NRM,	1,	A|B|C|D,	WW },
	{	"sharing",			do_animalism4,		P_REST,		0,	L_NRM,	1,	A|B|C,	VA },
	{	"beastdraw",		do_animalism5,		P_REST,		0,	L_NRM,	1,	A|B|C,	VA },
	{	"heightened",		do_auspex1,			P_REST,		0,	L_NRM,	1,	B|C|G,	VA|WW },
	{	"aura",				do_auspex2,			P_REST,		0,	L_NRM,	1,	B|C|G,	VA },
	{	"spirittouch",		do_auspex3,			P_REST,		0,	L_NRM,	1,	B|C|G|H,	VA },
	{	"telepathy",		do_auspex4,			P_REST,		0,	L_NRM,	1,	B|C|E,	VA },
	{	"project",			do_auspex5,			P_REST,		0,	L_NRM,	1,	A|B|C,	VA },
	{	"auspice",			do_auspice,			P_DEAD,		0,	L_NRM,	1,	A|B|G,	WW },
	{	"basedisc",			do_basediscs,		P_DEAD,		0,	L_NRM,	1,	A|B|E|G,	VA },
	{	"blood",			do_blood,			P_SLEEP,	0,	L_NRM,	1,	A|B|E,	VA },
	{	"rage",				do_blood,			P_SLEEP,	0,	L_NRM,	1,	A|B|E,	WW },
	{	"gnosis",			do_blood,			P_SLEEP,	0,	L_NRM,	1,	A|B|E,	WW },
	{	"breed",			do_breed,			P_DEAD,		0,	L_NRM,	1,	A|B|G,	WW },
	{	"wyldcall",			do_callwyld,		P_REST,		0,	L_COM,	1,	B|C|D|G|H,	WW },
	{	"celerity",			do_celerity,		P_REST,		0,	L_NRM,	1,	B|C|D|H,	VA },
	{	"ragemove",			do_celerity,		P_REST,		0,	L_NRM,	1,	B|C|D|H,	WW },
	{	"igniusfatuus",		do_chimerstry1,		P_REST,		0,	L_NRM,	1,	B|C|D,	VA },
	{	"fatamorgana",		do_chimerstry2,		P_REST,		0,	L_NRM,	1,	B|C|D,	VA },
	{	"apparition",		do_chimerstry3,		P_REST,		0,	L_NRM,	1,	B|C|D,	VA },
	{	"permanency",		do_chimerstry4,		P_REST,		0,	L_NRM,	1,	B|C|D,	VA },
	{	"claws",			do_claws,			P_REST,		0,	L_NRM,	1,	B|D|H,	WW },
	{	"dominate_compel",	do_dominate1,		P_REST,		0,	L_COM,	1,	B|C|G|H,	VA },
	{	"mesmerise",		do_dominate2,		P_REST,		0,	L_COM,	1,	B|C|G|H,	VA },
	{	"mesmerize",		do_dominate2,		P_REST,		0,	L_COM,	0,	B|C|G|H,	VA },
	{	"condition",		do_dominate4,		P_REST,		0,	L_NRM,	1,	B|C|G|H,	VA },
	{	"possess",			do_dominate5,		P_REST,		0,	L_NRM,	1,	A|B|C|G|H,	VA },
	{	"dreamspeak",		do_dreamspeak,		P_REST,		0,	L_COM,	1,	B|C|G,	WW },
	{	"awaken",			do_embrace,			P_STAND,	0,	L_NRM,	1,	B|C|D|H,	WW },
	{	"embrace",			do_embrace,			P_STAND,	0,	L_NRM,	1,	B|C|D|H,	VA },
	{	"silverclaws",		do_falcon3,			P_STAND,	0,	L_NRM,	1,	B|C|D|H,	WW },
	{	"fangs",			do_fangs,			P_REST,		0,	L_NRM,	1,	B|D|H,	VA|WW },
	{	"ghoul",			do_ghoul,			P_STAND,	0,	L_NRM,	1,	B|C|D|H,	VA },
	{	"regenerate",		do_heal,			P_TORPOR,	0,	L_NRM,	1,	B|E,	VA },
	{	"heal",				do_heal,			P_SIT,		0,	L_NRM,	1,	B|E,	HU|WW },
	{	"instruct",			do_instruct,		P_DEAD,		0,	L_NRM,	1,	A|B|G|H,	WW },
	{	"learn",			do_learn,			P_DEAD,		0,	L_NRM,	1,	A|B|H,	WW },
	{	"meditate",			do_meditate,		P_REST,		0,	L_NRM,	1,	A|B,	WW },
	{	"missingvoice",		do_melpominee1,		P_REST,		0,	L_COM,	1,	A|B|C|E|H,	VA },
	{	"madrigal",			do_melpominee3,		P_REST,		0,	L_COM,	1,	A|B|C|E|H,	VA },
	{	"sirenbeckon",		do_melpominee4,		P_REST,		0,	L_COM,	1,	A|B|C|E|H,	VA },
	{	"virtuosa",			do_melpominee5,		P_REST,		0,	L_COM,	1,	A|B|C|E|H,	VA },
	{	"mindspeak",		do_mentalspeech,	P_REST,		0,	L_COM,	1,	A|B|C|G,	WW },
	{	"blur",				do_obfuscate1,		P_REST,		0,	L_NRM,	1,	B|C|G,	WW },
	{	"cloak",			do_obfuscate1,		P_REST,		0,	L_NRM,	1,	B|C|G,	VA },
	{	"unseen",			do_obfuscate2,		P_REST,		0,	L_NRM,	1,	B|C|G,	VA|WW },
	{	"mask",				do_obfuscate3,		P_REST,		0,	L_NRM,	1,	B|C|D|G,	VA },
	{	"doppleganger",		do_obfuscate3,		P_REST,		0,	L_NRM,	1,	B|C|D|G,	WW },
	{	"vanish",			do_obfuscate4,		P_REST,		0,	L_NRM,	1,	B|C|G,	VA|WW },
	{	"gathering",		do_obfuscate5,		P_REST,		0,	L_NRM,	1,	B|C|G,	VA },
	{	"shadowplay",		do_obtenebration1,	P_REST,		0,	L_NRM,	1,	B|C|D|G,	VA },
	{	"shroud",			do_obtenebration2,	P_REST,		0,	L_NRM,	1,	B|C|D|G,	VA },
	{	"abyssarms",		do_obtenebration3,	P_REST,		0,	L_NRM,	1,	B|C|D|G|H,	VA },
	{	"blackmetamorph",	do_obtenebration4,	P_REST,		0,	L_NRM,	1,	B|C|D|G|H,	VA },
	{	"shadowform",		do_obtenebration5,	P_REST,		0,	L_NRM,	1,	B|C|D|G,	VA },
	{	"pack",				do_pack,			P_DEAD,		0,	L_NRM,	1,	A|B|E,	WW },
	{	"disciplines",		do_powers,			P_DEAD,		0,	L_NRM,	1,	A|B|E|G,	VA },
	{	"discs",			do_powers,			P_DEAD,		0,	L_NRM,	1,	A|B|E|G,	VA },
	{	"gifts",			do_powers,			P_DEAD,		0,	L_NRM,	1,	A|B|E|G,	WW },
	{	"awe",				do_presence1,		P_REST,		0,	L_NRM,	1,	B|C|G,	VA },
	{	"gaze",				do_presence2,		P_REST,		0,	L_NRM,	1,	B|C|D,	VA },
	{	"entrance",			do_presence3,		P_REST,		0,	L_NRM,	1,	B|C|G,	VA },
	{	"summon",			do_presence4,		P_REST,		0,	L_NRM,	1,	A|B|C|G,	VA },
	{	"majesty",			do_presence5,		P_REST,		0,	L_NRM,	1,	B|C|G,	VA },
	{	"gleam",			do_protean1,		P_REST,		0,	L_NRM,	1,	A|B|C|D,	VA },
	{	"wolfclaws",		do_protean2,		P_REST,		0,	L_NRM,	1,	B|C|D|H,	VA },
	{	"earthmeld",		do_protean3,		P_REST,		0,	L_NRM,	1,	B|C|D|E,	VA },
	{	"beastform",		do_protean4,		P_REST,		0,	L_NRM,	1,	B|C|D|H,	VA },
	{	"mistform",			do_protean5,		P_REST,		0,	L_NRM,	1,	B|C|D,	VA },
	{	"pump",				do_pump,			P_SIT,		0,	L_NRM,	1,	B|C,	VA },
	{	"staredown",		do_scare,			P_STAND,	0,	L_NRM,	1,	B|C|D|G,	WW },
	{	"truescent",		do_scent_trueform,	P_REST,		0,	L_NRM,	1,	A|B|C|G,	WW },
	{	"serpenteyes",		do_serpentis1,		P_STAND,	0,	L_NRM,	1,	A|B|C|E,	VA },
	{	"serpenttongue",	do_serpentis2,		P_STAND,	0,	L_NRM,	1,	A|B|C|D|E|H,	VA },
	{	"adderskin",		do_serpentis3,		P_STAND,	0,	L_NRM,	1,	A|B|C|D|E|H,	VA },
	{	"serpentform",		do_serpentis4,		P_STAND,	0,	L_NRM,	1,	A|B|C|D|E|H,	VA },
	{	"filch",			do_takeforgot,		P_REST,		0,	L_NRM,	1,	A|B|C|G|H,	WW },
	{	"teach",			do_teach,			P_STAND,	0,	L_NRM,	1,	B|H,	VA },
	{	"hagwrinkle",		do_thanatosis1,		P_REST,		0,	L_NRM,	1,	A|B|C|D|E|H,	VA },
	{	"putrefaction",		do_thanatosis2,		P_STAND,	0,	L_NRM,	1,	A|B|C|D|E|H,	VA },
	{	"ashform",			do_thanatosis3,		P_STAND,	0,	L_NRM,	1,	A|B|C|D|E|H,	VA },
	{	"bloodrage",		do_thaumaturgy2,	P_SIT,		0,	L_NRM,	1,	A|B|C,	VA },
	{	"bloodpotency",		do_thaumaturgy3,	P_SIT,		0,	L_NRM,	1,	A|B|C|E,	VA },
	{	"vitaetheft",		do_thaumaturgy4,	P_STAND,	0,	L_NRM,	1,	A|B|C|E|H,	VA },
	{	"bloodcauldron",	do_thaumaturgy5,	P_STAND,	0,	L_NRM,	1,	A|B|C|E|H,	VA },
	{	"totems",			do_totems,			P_DEAD,		0,	L_NRM,	1,	A|B|E|G,	WW },
	{	"uwalk",			do_umbral_walk,		P_REST,		0,	L_NRM,	1,	B|C|H,	WW },
	{	"sidestep",			do_umbral_walk,		P_REST,		0,	L_NRM,	0,	B|C|H,	WW },
	{	"changeling",		do_vicissitude1,	P_REST,		0,	L_NRM,	1,	A|B|C|D,	VA },
	{	"horridform",		do_vicissitude4,	P_REST,		0,	L_NRM,	1,	A|B|C|D|H,	VA },
	{	"bloodform",		do_vicissitude5,	P_REST,		0,	L_NRM,	1,	A|B|C|D,	VA },
	{	"shift",			do_ww_transform,	P_REST,		0,	L_NRM,	0,	B|D|H,	WW },
	{	"transform",		do_ww_transform,	P_REST,		0,	L_NRM,	1,	B|D|H,	WW },
	{	"shapechange",		do_ww_transform,	P_REST,		0,	L_NRM,	0,	B|D|H,	WW },
	{	"cook",				do_wwgift1_1,		P_REST,		0,	L_NRM,	1,	B|C|G|H,	WW },
	{	"shroud",			do_wwgift1_10,		P_STAND,	0,	L_NRM,	1,	B|C|D|G,	WW },
	{	"sensewyrm",		do_wwgift1_2,		P_REST,		0,	L_NRM,	1,	B|C|G,	WW },
	{	"mansmell",			do_wwgift1_4,		P_SIT,		0,	L_NRM,	1,	A|B|C,	WW },
	{	"resistpain",		do_wwgift1_6,		P_REST,		0,	L_NRM,	1,	A|B|C,	WW },
	{	"painresist",		do_wwgift1_6,		P_REST,		0,	L_NRM,	1,	A|B|C,	WW },
	{	"persuasion",		do_wwgift1_9,		P_SIT,		0,	L_NRM,	1,	A|B|C|G|H,	WW },
	{	"falltouch",		do_wwgift1_ahr,		P_REST,		0,	L_NRM,	1,	B|C|G|H,	WW },
	{	"odour",			do_wwgift2_1,		P_REST,		0,	L_NRM,	1,	B|C|D|G,	WW },
	{	"moonshield",		do_wwgift2_2,		P_REST,		0,	L_NRM,	1,	B|C|G,	WW },
	{	"siphon",			do_wwgift2_3,		P_REST,		0,	L_NRM,	1,	B|C|G|H,	WW },
	{	"shed",				do_wwgift2_4,		P_SIT,		0,	L_NRM,	1,	A|B|C|D|H,	WW },
	{	"termite",			do_wwgift3_1,		P_REST,		0,	L_NRM,	1,	B|C|D|G|H,	WW },
	{	"heatmetal",		do_wwgift3_3,		P_REST,		0,	L_NRM,	1,	B|C|D|G,	WW },
	{	"disquiet",			do_wwgift3_4,		P_SIT,		0,	L_NRM,	1,	A|B|C|G,	WW },
	{	"weakarm",			do_wwgift3_7,		P_SIT,		0,	L_NRM,	1,	A|B|C|G|H,	WW },
	{	"sensesilver",		do_wwgift3_9,		P_REST,		0,	L_NRM,	1,	B|C|G,	WW },
	{	"compel",			do_wwgift3_the,		P_REST,		0,	L_NRM,	1,	B|C|G|H,	WW },
	{	"attune",			do_wwgift4_1,		P_REST,		0,	L_NRM,	1,	B|C|G|H,	WW },
	{	"wrath",			do_wwgift4_2,		P_STAND,	0,	L_NRM,	1,	B|C|G|H,	WW },
	{	"grovel",			do_wwgift4_5,		P_REST,		0,	L_NRM,	1,	A|B|C|G|H,	WW },
	{	"sharewill",		do_wwgift4_ahr,		P_REST,		0,	L_NRM,	1,	B|C|G,	WW },
	{	"sniffumbra",		do_wwgift4_phi,		P_REST,		0,	L_NRM,	1,	B|C|G,	WW },
	{	"exorcism",			do_wwgift4_the,		P_REST,		0,	L_NRM,	1,	A|B|C|G|H,	WW },
	{	"trueform",			do_wwgift4b_phi,	P_REST,		0,	L_NRM,	1,	A|B|C|G|H,	WW },
	{	"lunasavenger",		do_wwgift5_2,		P_SIT,		0,	L_NRM,	1,	A|B|C|D,	WW },
	{	"ptravel",			do_wwgift5_3,		P_REST,		0,	L_NRM,	1,	B|C|D|G|H,	WW },
	{	"lunasblessing",	do_wwgift5_5,		P_SIT,		0,	L_NRM,	1,	A|B|C|G,	WW },
	{	"bridgewalk",		do_wwgift5_gal,		P_REST,		0,	L_NRM,	1,	A|B|C|D|G|H,	WW },
	{	"spiritdrain",		do_wwgift5_the,		P_SIT,		0,	L_NRM,	1,	A|B|C|G,	WW },

	/*
	* Immortal commands.
	*/
/*		{ "noshout",do_noshout,	P_DEAD,		WA, L_ALL, 1, A|B, 0}, */
/* 		{ "vlist",	do_vlist,	P_DEAD,		WA, L_NRM, 1, A|B, 0},*/
	{	":",				do_immtalk,			P_DEAD,	WA,	L_SCOM,	0,	A|B|E,	0 },
	{	"addlag",			do_addlag,			P_DEAD,	ST,	L_ALL,	1,	A|B|E,	0 },
	{	"ai_edit",			do_aiedit,			P_DEAD,	MA,	L_NRM,	1,	A|B|E,	0 },
	{	"ai_list",			do_ailist,			P_DEAD,	ST,	L_NRM,	1,	A|B|E,	0 },
	{	"ai_save",			do_save_ai,			P_DEAD,	MA,	L_NRM,	1,	A|B|E,	0 },
	{	"alist",			do_alist,			P_DEAD,	WA,	L_NRM,	1,	A|B|E,	0 },
	{	"allow",			do_allow,			P_DEAD,	WA,	L_ALL,	1,	A|B|E,	0 },
	{	"articles",			do_articles,		P_DEAD,	WA,	L_NRM,	1,	A|B|E,	0 },
	{	"at",				do_at,				P_DEAD,	WA,	L_NRM,	1,	A|B,	0 },
	{	"backupmud",		do_backup,			P_DEAD,	MA,	L_NRM,	1,	A|B|E,	0 },
	{	"ban",				do_ban,				P_DEAD,	ST,	L_ALL,	1,	A|B|E,	0 },
	{	"being",			do_as,				P_DEAD,	WA,	L_ALL,	1,	A|B|E,	0 },
	{	"bg",				do_bg,				P_DEAD,	WA,	L_NRM,	1,	A|B|E,	0 },
	{	"bonus",			do_bonus,			P_DEAD,	WA,	L_NRM,	1,	A|B|E,	0 },
	{	"clone",			do_clone,			P_DEAD,	ST,	L_ALL,	1,	A|B|E,	0 },
	{	"deny",				do_deny,			P_DEAD,	MA,	L_ALL,	1,	A|B|E,	0 },
	{	"disconnect",		do_disconnect,		P_DEAD,	WA,	L_ALL,	1,	A|B|E,	0 },
	{	"dwalk",			do_dream_walk,		P_SLEEP,	WA,	L_NRM,	1,	B|C|G|H,	0 },
	{	"echo",				do_echo,			P_DEAD,	WA,	L_ALL,	1,	A|B|E,	0 },
	{	"elder",			do_elder,			P_DEAD,	MA,	L_ALL,	1,	A|B|E,	0 },
	{	"election",			do_election,		P_DEAD,	MA,	L_ALL,	1,	A|B|E,	0 },
	{	"event_edit",		do_eedit,			P_DEAD,	ST,	L_NRM,	1,	A|B|E,	0 },
	{	"event_end",		do_end_event,		P_DEAD,	ST,	L_NRM,	1,	A|B,	0 },
	{	"event_list",		do_elist,			P_DEAD,	ST,	L_NRM,	1,	A|B|E,	0 },
	{	"exlist",			do_exlist,			P_DEAD,	WA,	L_ALL,	1,	A|B,	0 },
	{	"facts",			do_know,			P_DEAD,	WA,	L_NRM,	1,	A|B|H,	0 },
	{	"fixchar",			do_fixchar,			P_DEAD,	ST,	L_ALL,	1,	A|B|E,	0 },
	{	"flag",				do_flag,			P_DEAD,	WA,	L_ALL,	1,	A|B|E,	0 },
	{	"for",				do_for,				P_DEAD,	MA,	L_ALL,	1,	A|B,	0 },
	{	"force",			do_force,			P_DEAD,	ST,	L_ALL,	1,	A|B|E,	0 },
	{	"freeze",			do_freeze,			P_DEAD,	ST,	L_ALL,	1,	A|B|E,	0 },
	{	"gecho",			do_echo,			P_DEAD,	WA,	L_ALL,	1,	A|B|E,	0 },
	{	"goto",				do_goto,			P_DEAD,	WA,	L_NRM,	1,	A|B,	0 },
	{	"help_edit",		do_hedit,			P_DEAD,	WA,	L_NRM,	1,	A|B|E,	0 },
	{	"help_save",		do_hsave,			P_DEAD,	WA,	L_NRM,	1,	A|B|E,	0 },
	{	"help_save2",		do_hsave2,			P_DEAD,	WA,	L_NRM,	1,	A|B|E,	0 },
	{	"holylight",		do_holylight,		P_DEAD,	WA,	L_NRM,	1,	A|B|E,	0 },
	{	"hunted",			do_hunted,			P_DEAD,	MA,	L_NRM,	1,	A|B|E,	0 },
	{	"immtalk",			do_immtalk,			P_DEAD,	WA,	L_SCOM,	1,	A|B|E,	0 },
	{	"imotd",			do_imotd,			P_DEAD,	WA,	L_NRM,	1,	A|B|E,	0 },
	{	"incognito",		do_incognito,		P_DEAD,	WA,	L_NRM,	1,	A|B|E,	0 },
	{	"interrupt",		do_interrupt,		P_DEAD,	ST,	L_NRM,	1,	A|B,	0 },
	{	"load",				do_load,			P_DEAD,	ST,	L_ALL,	1,	A|B|F,	0 },
	{	"loaddice",			do_loaddice,		P_DEAD,	MA,	L_ALL,	1,	A|B|F,	0 },
	{	"log_pc",			do_log,				P_DEAD,	MA,	L_ALL,	1,	A|B|E,	0 },
	{	"marry",			do_marry,			P_REST,	WA,	L_NRM,	1,	A|B|E,	0 },
	{	"memory",			do_memory,			P_DEAD,	WA,	L_NRM,	1,	A|B|E,	0 },
	{	"mob_find",			do_mfind,			P_DEAD,	WA,	L_NRM,	1,	A|B|E,	0 },
	{	"mob_stat",			do_mstat,			P_DEAD,	WA,	L_NRM,	1,	A|B|E,	0 },
	{	"mob_where",		do_mwhere,			P_DEAD,	WA,	L_NRM,	1,	A|B|E,	0 },
	{	"newlock",			do_newlock,			P_DEAD,	MA,	L_ALL,	1,	A|B|E,	0 },
	{	"newspaper",		do_newspaper,		P_DEAD,	WA,	L_NRM,	1,	A|B|E,	0 },
	{	"nochannels",		do_nochannels,		P_DEAD,	WA,	L_ALL,	1,	A|B|E,	0 },
	{	"noemote",			do_noemote,			P_DEAD,	WA,	L_ALL,	1,	A|B|E,	0 },
	{	"nogiftxp",			do_nogiftxp,		P_DEAD,	WA,	L_ALL,	1,	A|B|E,	0 },
	{	"notell",			do_notell,			P_DEAD,	WA,	L_ALL,	1,	A|B|E,	0 },
	{	"offerquest",		do_goquest,			P_DEAD,	WA,	L_ALL,	1,	A|B|E,	0 },
	{	"obj_find",			do_ofind,			P_DEAD,	WA,	L_NRM,	1,	A|B|E,	0 },
	{	"obj_stat",			do_ostat,			P_DEAD,	WA,	L_NRM,	1,	A|B|E,	0 },
	{	"obj_where",		do_owhere,			P_DEAD,	WA,	L_NRM,	1,	A|B|E,	0 },
	{	"olc_set",			do_set,				P_DEAD,	ST,	L_ALL,	1,	A|B|E,	0 },
	{	"organisation",		do_org,				P_DEAD,	WA,	L_NRM,	1,	A|B,	0 },
	{	"pardon",			do_pardon,			P_DEAD,	WA,	L_NRM,	1,	A|B|E,	0 },
	{	"peace",			do_peace,			P_DEAD,	WA,	L_NRM,	1,	A|B|E,	0 },
	{	"pecho",			do_pecho,			P_DEAD,	WA,	L_ALL,	1,	A|B|E,	0 },
	{	"pedit",			do_pedit,			P_DEAD,	ST,	L_NRM,	1,	A|B|E,	0 },
	{	"peek",				do_peek,			P_REST,	WA,	L_NRM,	1,	A|B|E,	0 },
	{	"permban",			do_permban,			P_DEAD,	MA,	L_ALL,	1,	A|B|E,	0 },
	{	"plist",			do_plist,			P_DEAD,	ST,	L_NRM,	1,	A|B|E,	0 },
	{	"plotsave",			do_save_plots,		P_DEAD,	ST,	L_NRM,	1,	A|B|E,	0 },
	{	"poofin",			do_bamfin,			P_DEAD,	WA,	L_NRM,	1,	A|B|E,	0 },
	{	"poofout",			do_bamfout,			P_DEAD,	WA,	L_NRM,	1,	A|B|E,	0 },
	{	"protocol",			do_protocol,		P_DEAD,	WA,	L_NRM,	1,	A|B|E,	0 },
	{	"purge",			do_purge,			P_DEAD,	WA,	L_ALL,	1,	A|B|E|F,	0 },
	{	"randat",			do_random_at,		P_DEAD,	WA,	L_COM,	1,	A|B,	0 },
	{	"reboot",			do_reboot,			P_DEAD,	MA,	L_ALL,	1,	A|B|E,	0 },
	{	"restore",			do_restore,			P_DEAD,	WA,	L_ALL,	1,	A|B|E,	0 },
	{	"return",			do_return,			P_DEAD,	ST,	L_NRM,	1,	A|B|E,	0 },
	{	"ritemoves",		do_ritemoves,		P_DEAD,	WA,	L_NRM,	1,	A|B|E,	0 },
	{	"rsedit",			do_rsedit,			P_DEAD,	MA,	L_NRM,	1,	A|B|E,	0 },
	{	"rslist",			do_rslist,			P_DEAD,	ST,	L_NRM,	1,	A|B|E,	0 },
	{	"rstat",			do_rstat,			P_DEAD,	WA,	L_NRM,	1,	A|B|E,	0 },
	{	"runevent",			do_run_event,		P_DEAD,	ST,	L_NRM,	1,	A|B,	0 },
	{	"runscript",		do_run_script,		P_DEAD,	ST,	L_NRM,	1,	A|B,	0 },
	{	"sayat",			do_sayat,			P_DEAD,	WA,	L_COM,	1,	A|B|E,	0 },
	{	"sedit",			do_sedit,			P_DEAD,	ST,	L_NRM,	1,	A|B|E,	0 },
	{	"shutdown",			do_shutdown,		P_DEAD,	IM, L_ALL,	1,	A|B|E,	0 },
	{	"slay",				do_slay,			P_DEAD,	MA,	L_ALL,	1,	A|B|E,	0 },
	{	"slist",			do_slist,			P_DEAD,	ST,	L_NRM,	1,	A|B|E,	0 },
	{	"smarket",			do_stockmarket,		P_DEAD,	WA,	L_NRM,	1,	A|B|E,	0 },
	{	"snoop",			do_snoop,			P_DEAD,	WA,	L_ALL,	1,	A|B|E,	0 },
	{	"socedit",			do_socedit,			P_DEAD,	WA,	L_NRM,	1,	A|B|E,	0 },
	{	"sockets",			do_sockets,			P_DEAD,	WA,	L_NRM,	1,	A|B|E,	0 },
	{	"string",			do_string,			P_DEAD,	WA,	L_ALL,	1,	A|B|E,	0 },
	{	"switch",			do_switch,			P_DEAD,	ST,	L_ALL,	1,	A|B|E,	0 },
	{	"telepath",			do_telepath,		P_DEAD,	WA,	L_NRM,	1,	A|B|E,	0 },
	{	"timerclear",		do_clear_timers,	P_DEAD,	ST,	L_ALL,	1,	A|B|E,	0 },
	{	"timeset",			do_timeset,			P_DEAD,	ST,	L_ALL,	1,	A|B|E,	0 },
	{	"timestop",			do_timestop,		P_DEAD,	ST,	L_ALL,	1,	A|B|E,	0 },
	{	"tipedit",			do_tipedit,			P_DEAD,	WA,	L_NRM,	1,	A|B|E,	0 },
	{	"trackbuffer",		do_trackbuffer,		P_DEAD,	MA,	L_ALL,	0,	A|B|E,	0 },
	{	"transfer",			do_transfer,		P_DEAD,	WA,	L_ALL,	1,	A|B,	0 },
	{	"tsave",			do_tsave,			P_DEAD,	WA,	L_NRM,	1,	A|B|E,	0 },
	{	"unpak",			do_unpak,			P_DEAD,	MA,	L_NRM,	1,	A|B|E,	0 },
	{	"vnum",				do_vnum,			P_DEAD,	WA,	L_NRM,	1,	A|B|E,	0 },
	{	"wizhelp",			do_wizhelp,			P_DEAD,	WA,	L_NRM,	1,	A|B|E,	0 },
	{	"wizinvis",			do_invis,			P_DEAD,	WA,	L_NRM,	1,	A|B|E,	0 },
	{	"wizlock",			do_wizlock,			P_DEAD,	ST,	L_ALL,	1,	A|B|E,	0 },
	{	"wiznet",			do_wiznet,			P_DEAD,	WA,	L_NRM,	1,	A|B|E,	0 },
	{	"xpclear",			do_resetoocxp,		P_DEAD,	ST,	L_ALL,	1,	A|B|E,	0 },
	{	"testscore",		do_score_revised,	P_DEAD, IM, L_NRM,	1,	A|B|E,	0 },
	{	"testceleb",		do_testceleb,		P_DEAD, IM, L_NRM,	1,	A|B|E,	0 },
	{	"charsheet",		do_charsheet,		P_DEAD, IM, L_NRM,	1,	A|B|E,	0 },


	/* Moved to resolve conflicts. */
	{ "meditate",do_meditate,	P_REST,		 0, L_NRM, 1, A|B, WW},

	/*
	* End of list.
	*/
	{ "",	0,		P_DEAD,		 0, L_NRM, 0, A|B, 0}
};

bool MY_RACE_CMD(CHAR_DATA *ch, int cmd)
{
	int race = ch->race;

	if(ch->desc != NULL && ch->desc->original != NULL)
		race = ch->desc->original->race;

	if(race == race_lookup("human") && IS_SET(cmd_table[cmd].race, HU))
		return TRUE;

	if(race == race_lookup("werewolf") && IS_SET(cmd_table[cmd].race, WW))
		return TRUE;

	if(race == race_lookup("vampire") && IS_SET(cmd_table[cmd].race, VA))
		return TRUE;

	if(race == race_lookup("faerie") && IS_SET(cmd_table[cmd].race, FA))
		return TRUE;

	return FALSE;
}

bool CAN_USE_CMD(CHAR_DATA *ch, int cmd)
{
	if(cmd_table[cmd].race == 0)
		return TRUE;

	if(MY_RACE_CMD(ch, cmd))
		return TRUE;

	return FALSE;
}

/*
* Command lookups. (Created so that commands can be searched by do_use)
*/
int command_lookup(char *command)
{
	int cmd;

	for ( cmd = 0; !IS_NULLSTR(cmd_table[cmd].name); cmd++ )
	{
		if ( command[0] == cmd_table[cmd].name[0] &&   !str_prefix( command, cmd_table[cmd].name ) )
		{
			return cmd;
		}
	}

	return -1;
}

int command_available(CHAR_DATA *ch, char *command)
{
	int cmd;
	int trust = get_trust(ch);

	for ( cmd = 0; !IS_NULLSTR(cmd_table[cmd].name); cmd++ )
	{
		if ( command[0] == cmd_table[cmd].name[0] &&   !str_prefix( command, cmd_table[cmd].name )
				&&   (cmd_table[cmd].level <= trust || (IS_SET(cmd_table[cmd].flags, BUILDERMODE_CMD)
						&& IS_SET(ch->comm, COMM_BUILDER))) )
		{
			return cmd;
		}
	}

	return -1;
}


/*
* The main entry point for executing commands.
* Can be recursively called from 'at', 'order', 'force'.
*/
void interpret( CHAR_DATA *ch, char *argument )
{
	char command[MAX_INPUT_LENGTH]={'\0'};
	char logline[MAX_INPUT_LENGTH]={'\0'};
	int cmd;
	bool found, exists;
	CHAR_DATA *vch, *next;

	assert(ch);

	/*
	 * Strip leading spaces.
	 */
	while ( isspace(*argument) )
		argument++;
	if ( IS_NULLSTR(argument) )
		return;

	/*
	 * No hiding.
	 */
	REMOVE_BIT( ch->affected_by, AFF_HIDE );

	/*
	 * Implement freeze command.
	 */
	if ( !IS_NPC(ch) && IS_SET(ch->plr_flags, PLR_FREEZE) )
	{
		send_to_char( "You're totally frozen!\n\r", ch );
		return;
	}

	/*
	 * Can't act when dead.
	 */
	if (IS_SET(ch->act, ACT_REINCARNATE))
	{
		send_to_char( "The shadow of death engulfs you.\n\r", ch);
		return;
	}

	/*
	 * Grab the command word.
	 * Special parsing so ' can be a command,
	 *   also no spaces needed after punctuation.
	 */
	strncpy( logline, argument, MIL );
	if ( !isalpha(argument[0]) && !isdigit(argument[0]) )
	{
		command[0] = argument[0];
		command[1] = '\0';
		argument++;
		while ( isspace(*argument) )
			argument++;
	}
	else
	{
		argument = one_argument( argument, command );
	}

	/*
	 * Look for command in command table.
	 */
	found = FALSE;
	exists = FALSE;
	if((cmd = command_lookup(command)) > -1)
	{
		exists = TRUE;
	}

	if((cmd = command_available(ch, command)) > -1)
	{
		found = TRUE;
	}

	/*
	 * Log and snoop.
	 */
	if ( cmd_table[cmd].log == L_NEV )
		strncpy( logline, "", MIL );

	if ( ( !IS_NPC(ch) && IS_SET(ch->plr_flags, PLR_LOG) )
			||   fLogAll
			||   cmd_table[cmd].log == L_ALL
			||	 (fLogAllCom
					&& (cmd_table[cmd].log == L_COM || cmd_table[cmd].log == L_SCOM))
					||	 (fLogCom && cmd_table[cmd].log == L_COM) )
	{
		char    s[2*MAX_INPUT_LENGTH],*ps;
		int     i;

		ps=s;
		/*send_to_char(Format("Log %s: %s", ch->name, logline), ch); */

		log_string(LOG_SECURITY, Format("Log %s: %s", ch->name, logline));

		/* Make sure that was is displayed is what is typed */
		for (i=0;logline[i];i++) {
			*ps++=logline[i];
			if (logline[i]=='$')
				*ps++='$';
			if (logline[i]=='{')
				*ps++='{';
		}
		*ps=0;
		if(cmd_table[cmd].log == L_COM)
			wiznet(s,ch,NULL,WIZ_COMM,0,get_trust(ch));
		else if(cmd_table[cmd].log == L_COM)
			wiznet(s,ch,NULL,WIZ_STAFF_COMM,0,get_trust(ch));
		else
			wiznet(s,ch,NULL,WIZ_SNOOPS,0,get_trust(ch));
	}

	if ( ch->desc != NULL && ch->desc->snoop_by != NULL )
	{
		write_to_buffer( ch->desc->snoop_by, "% ",    2 );
		write_to_buffer( ch->desc->snoop_by, logline, 0 );
		write_to_buffer( ch->desc->snoop_by, "\n\r",  2 );
	}

	if ( !found
			|| !CAN_USE_CMD(ch, cmd))
	{
		/*
		 * Look for command in socials table.
		 */
		if ( !check_social( ch, command, argument ) )
		{
			send_to_char( "Huh?\n\r", ch );
			if(!exists)
			{
				log_string(LOG_BUG, (char *)Format("[%d] %s : Command failure: %s %s - %s", ch->in_room ? ch->in_room->vnum : 0, ch->name, command, argument, ctime(&current_time)));
			}
		}
		return;
	}

	if(IS_SET(ch->act2, ACT2_ASTRAL)
	&& !IS_SET(cmd_table[cmd].flags, ASTRAL_OK))
	{
	send_to_char("You can't do that while wandering the astral plane!\n\r", ch);
	return;
	}

	if(IS_SET(ch->affected_by2, AFF2_EARTHMELD)
	&& !IS_SET(cmd_table[cmd].flags, EARTHMELD_OK))
	{
	send_to_char("You can't do that while bound inside the earth!\n\r", ch);
	return;
	}

	if(ch->desc != NULL && ch->desc->original != NULL
	&& !IS_SET(cmd_table[cmd].flags, POSSESS_OK)
	&& !IS_ADMIN(ch->desc->original)
	&& !CAN_USE_CMD(ch, cmd))
	{
	send_to_char( "You can't do that while possessing someone!\n\r", ch);
	return;
	}

	if(IS_SET(ch->act2, ACT_FAST)
	&& IS_SET(cmd_table[cmd].flags, POWER_CMD))
	{
	send_to_char( "You can't do that while moving so rapidly.\n\r", ch);
	return;
	}

	if(IS_SET(ch->act2, ACT2_RP_ING) && !IS_SET(cmd_table[cmd].flags, RP_OK))
	{
	send_to_char( "You can't do that while in roleplaying mode!\n\r", ch);
	return;
	}

	/*
	* Implement AFF2_IMMOBILIZED.
	*/
	if ( IS_AFFECTED2(ch, AFF2_IMMOBILIZED)
	&& !IS_SET(cmd_table[cmd].flags, MOTION) )
	{
	send_to_char( "You can't move a muscle!\n\r", ch );
	return;
	}

	/*
	* Character not in position for command?
	*/
	if ( ch->position < cmd_table[cmd].position
	&& !(ch->position == P_SLEEP && IS_SET(ch->act2, ACT2_ASTRAL)
	&& IS_SET(cmd_table[cmd].flags, ASTRAL_OK)) )
	{
	found = FALSE;
	switch( ch->position )
	{
	case P_DEAD:
		send_to_char( "Lie still; you are DEAD.\n\rIf you don't lie still you won't die properly.\n\r", ch );
		break;

	case P_MORT:
	case P_INCAP:
		send_to_char( "You are hurt far too bad for that.\n\r", ch );
		break;

	case P_STUN:
		send_to_char( "You are too stunned to do that.\n\r", ch );
		break;

	case P_SLEEP:
		if(IS_SET(ch->act2, ACT2_ASTRAL) && !IS_SET(cmd_table[cmd].flags, ASTRAL_OK))
			found = TRUE;
		else
			send_to_char( "In your dreams, or what?\n\r", ch );
		break;

	case P_REST:
		send_to_char( "Nah... You feel too relaxed...\n\r", ch);
		break;

	case P_SIT:
		send_to_char( "Better stand up first.\n\r",ch);
		break;

	case P_FIGHT:
		send_to_char( "No way!  You are still fighting!\n\r", ch);
		break;

	}
	if(!found)
		return;
	}

	/*
	 * Dispatch the command.
	 */
	(*cmd_table[cmd].do_fun) ( ch, argument );

	if(IS_SET(cmd_table[cmd].flags, POWER_DISPLAY))
	{
		ROOM_INDEX_DATA *to_room = NULL;

		if(ch->in_room)
			to_room = ch->in_room;
		else if(ch->was_in_room)
			to_room = ch->was_in_room;

		if(to_room)
		{
			for(vch = to_room->people; vch; vch = next)
			{
				next = vch->next_in_room;
				if(vch == ch || !IS_NPC(vch)) continue;
				if( HAS_TRIGGER(vch, TRIG_POWERDISP) )
				{
					mp_percent_trigger( vch, ch, NULL, NULL, TRIG_POWERDISP );
					continue;
				}
				if(IS_NATURAL(vch) && IS_NPC(vch) && vch->master == NULL
						&& IS_FEARFUL(vch) && SAME_PLANE(vch, ch)
						&& !IS_SET(vch->act2, ACT2_TWILIGHT))
				{
					do_flee(vch, "");
					ch->hunter_vis += number_fuzzy(5);
				}
				if(IS_NPC(vch) && IS_SET(vch->act2, ACT2_TWILIGHT)
						&& SAME_PLANE(vch, ch))
				{
					act("$N leaps at you as if your very existence enrages $M.", ch, NULL, vch, TO_CHAR, 1);
					act("$N leaps at $n as if $s very existence enrages $M.", ch, NULL, vch, TO_CHAR, 1);
					strike(vch, ch);
				}
			}
		}
	}

	tail_chain( );
	return;
}


/* function to keep argument safe in all commands -- no static strings */
void do_function (CHAR_DATA *ch, DO_FUN *do_fun, char *argument)
{
//	char *command_string;

	// NO point in this b/s...
	/* copy the string */
	//command_string = str_dup(argument);

	/* dispatch the command */
	(*do_fun) (ch, argument);

	/* free the string */
	//PURGE_DATA(command_string);
}

bool check_social( CHAR_DATA *ch, char *command, char *argument )
{
	char arg[MAX_INPUT_LENGTH]={'\0'};
	CHAR_DATA *victim;
	int cmd;
	bool found;

	found  = FALSE;
	for ( cmd = 0; !IS_NULLSTR(social_table[cmd].name); cmd++ )
	{
	if ( command[0] == social_table[cmd].name[0]
	&&   !str_prefix( command, social_table[cmd].name ) )
	{
		found = TRUE;
		break;
	}
	}

	if ( !found )
	return FALSE;

	if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) )
	{
	send_to_char( "You are anti-social!\n\r", ch );
	return TRUE;
	}

	switch ( ch->position )
	{
	case P_DEAD:
	send_to_char( "Lie still; you are DEAD.\n\r", ch );
	return TRUE;

	case P_INCAP:
	case P_MORT:
	send_to_char( "You are hurt far too bad for that.\n\r", ch );
	return TRUE;

	case P_STUN:
	send_to_char( "You are too stunned to do that.\n\r", ch );
	return TRUE;

	case P_SLEEP:
	/*
	* I just know this is the path to a 12" 'if' statement.  :(
	* But two players asked for it already!  -- Furey
	*/
	if ( !str_cmp( social_table[cmd].name, "snore" ) )
		break;
	send_to_char( "In your dreams, or what?\n\r", ch );
	return TRUE;

	}

	one_argument( argument, arg );
	victim = NULL;
	if ( IS_NULLSTR(arg) )
	{
		if(social_table[cmd].others_no_arg != NULL
				&& social_table[cmd].char_no_arg)
		{
			act( Format("\tGOOC: \tg%s\tn", social_table[cmd].others_no_arg), ch, NULL, victim, TO_ROOM, 1 );
			act( Format("\tGOOC: \tg%s\tn", social_table[cmd].char_no_arg), ch, NULL, victim, TO_CHAR, 1 );
		}
	}
	else if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
	}
	else if ( victim == ch )
	{
		if(social_table[cmd].others_auto != NULL
				&& social_table[cmd].char_auto)
		{
			act( Format("\tGOOC: \tg%s\tn", social_table[cmd].others_auto), ch, NULL, victim, TO_ROOM, 1 );
			act( Format("\tGOOC: \tg%s\tn", social_table[cmd].char_auto), ch, NULL, victim, TO_CHAR, 1 );
		}
	}
	else
	{
		if(social_table[cmd].others_found != NULL && social_table[cmd].char_found && social_table[cmd].vict_found)
		{
			act( Format("\tGOOC: \tg%s\tn", social_table[cmd].others_found), ch, NULL, victim, TO_NOTVICT, 1 );
			act( Format("\tGOOC: \tg%s\tn", social_table[cmd].char_found), ch, NULL, victim, TO_CHAR, 1 );
			act( Format("\tGOOC: \tg%s\tn", social_table[cmd].vict_found), ch, NULL, victim, TO_VICT, 1 );
		}

		if ( !IS_NPC(ch) && IS_NPC(victim) &&   !IS_AFFECTED(victim, AFF_CHARM) &&   IS_AWAKE(victim) &&   victim->desc == NULL)
		{
			switch ( number_bits( 4 ) )
			{
			case 0:

			case 1: case 2: case 3: case 4:
			case 5: case 6: case 7: case 8:
				if(social_table[cmd].others_found != NULL
						&& social_table[cmd].char_found
						&& social_table[cmd].vict_found)
				{
					act( Format("\tGOOC: \tg%s\tn", social_table[cmd].others_found), victim, NULL, ch, TO_NOTVICT, 1 );
					act( Format("\tGOOC: \tg%s\tn", social_table[cmd].char_found), victim, NULL, ch, TO_CHAR, 1 );
					act( Format("\tGOOC: \tg%s\tn", social_table[cmd].vict_found), victim, NULL, ch, TO_VICT, 1 );
				}
				break;

			case 9: case 10: case 11: case 12:
				act( "\tGOOC: \tg$n slaps $N.\tn",  victim,NULL,ch,TO_NOTVICT,1);
				act( "\tGOOC: \tgYou slap $N.\tn",  victim,NULL,ch,TO_CHAR, 1 );
				act( "\tGOOC: \tg$n slaps you.\tn", victim,NULL,ch,TO_VICT, 1 );
				break;
			}
		}
	}
	return TRUE;
}



/*
* Return true if an argument is completely numeric.
*/
bool is_number ( char *arg )
{

	if ( *arg == '\0' )
		return FALSE;

	if ( *arg == '+' || *arg == '-' )
		arg++;

	for ( ; *arg != '\0'; arg++ )
	{
		if ( !isdigit( *arg ) )
			return FALSE;
	}

	return TRUE;
}



/*
* Given a string like 14.foo, return 14 and 'foo'
*/
int number_argument( char *argument, char *arg )
{
	char *pdot;
	int number;

	for ( pdot = argument; *pdot != '\0'; pdot++ )
	{
		if ( *pdot == '.' )
		{
			*pdot = '\0';
			number = atoi( argument );
			*pdot = '.';
			strncpy( arg, pdot+1, sizeof(arg) );
			return number;
		}
	}

	strncpy( arg, argument, sizeof(arg) );
	return 1;
}

/*
* Given a string like 14*foo, return 14 and 'foo'
*/
int mult_argument(char *argument, char *arg)
{
	char *pdot;
	int number;

	for ( pdot = argument; *pdot != '\0'; pdot++ )
	{
		if ( *pdot == '*' )
		{
			*pdot = '\0';
			number = atoi( argument );
			*pdot = '*';
			strncpy( arg, pdot+1, sizeof(arg) );
			return number;
		}
	}

	strncpy( arg, argument, sizeof(arg) );
	return 1;
}



/*
* Pick off one argument from a string and return the rest.
* Understands quotes.
*/
char *one_argument( char *argument, char *arg_first )
{
	char cEnd;

	while ( isspace(*argument) )
		argument++;

	cEnd = ' ';
	if ( *argument == '\'' || *argument == '"' )
		cEnd = *argument++;

	while ( *argument != '\0' )
	{
		if ( *argument == cEnd )
		{
			argument++;
			break;
		}
		*arg_first = LOWER(*argument);
		arg_first++;
		argument++;
	}
	*arg_first = '\0';

	while ( isspace(*argument) )
		argument++;

	return argument;
}

/*
* Pick off one argument from a string and return the rest.
* Understands quotes and doesn't make all lower case.
*/
char *exact_argument( char *argument, char *arg_first )
{
	char cEnd;

	while ( isspace(*argument) )
		argument++;

	cEnd = ' ';
	if ( *argument == '\'' || *argument == '"' )
		cEnd = *argument++;

	while ( *argument != '\0' )
	{
		if ( *argument == cEnd )
		{
			argument++;
			break;
		}
		*arg_first = *argument;
		arg_first++;
		argument++;
	}
	*arg_first = '\0';

	while ( isspace(*argument) )
		argument++;

	return argument;
}

/*
* Pick off one chunk from a string seperated by 'meep'.
*/
char *one_chunk( char *argument, char *arg_first, char cEnd )
{
	while ( isspace(*argument) )
		argument++;

	if ( *argument == '\'' || *argument == '"' )
		cEnd = *argument++;

	while ( *argument != '\0' )
	{
		if ( *argument == cEnd )
		{
			argument++;
			break;
		}
		*arg_first = *argument; /*LOWER(*argument);*/
		arg_first++;
		argument++;
	}
	*arg_first = '\0';

	while ( isspace(*argument) )
		argument++;

	return argument;
}

/*
* Stuff to chomp the first line off a string.
*/
char *one_line( char *argument, char *arg_first )
{
	char cEnd;

	while ( isspace(*argument) )
		argument++;

	cEnd = '\n';

	while ( *argument != '\0' )
	{
		if ( *argument == cEnd )
		{
			argument++;
			break;
		}
		*arg_first = *argument;
		arg_first++;
		argument++;
	}
	*arg_first = '\0';

	while ( isspace(*argument) )
		argument++;

	return argument;
}

/*
* Contributed by Alander.
*/
void do_commands( CHAR_DATA *ch, char *argument )
{
	int cmd = 0;
	int col = 0;

	for ( cmd = 0; !IS_NULLSTR(cmd_table[cmd].name); cmd++ )
	{
		if ( cmd_table[cmd].level <  LEVEL_IMMORTAL
				&&   cmd_table[cmd].level <= get_trust( ch )
				&&   cmd_table[cmd].show == 1
				&&   cmd_table[cmd].race == 0
				&&   CAN_USE_CMD(ch, cmd))
		{
			send_to_char( Format("%-15s", cmd_table[cmd].name), ch );
			if ( ++col % 5 == 0 )
				send_to_char( "\n\r", ch );
		}
	}

	if ( col % 5 != 0 )
		send_to_char( "\n\r", ch );
	return;
}

void do_wizhelp( CHAR_DATA *ch, char *argument )
{
	int cmd = 0;
	int col = 0;

	for ( cmd = 0; !IS_NULLSTR(cmd_table[cmd].name); cmd++ )
	{
		if ( cmd_table[cmd].level >= LEVEL_IMMORTAL
				&&   cmd_table[cmd].level <= get_trust( ch )
				&&   cmd_table[cmd].show)
		{
			send_to_char( Format("\t<send href='%s'>%-17s\t</send> |", cmd_table[cmd].name, cmd_table[cmd].name), ch );
			if ( ++col % 4 == 0 )
				send_to_char( "\n\r", ch );
		}
	}

	if ( col % 4 != 0 )
		send_to_char( "\n\r", ch );
	return;
}

void do_buildhelp( CHAR_DATA *ch, char *argument )
{
	int cmd = 0;
	int col = 0;

	if(IS_NPC(ch)) return;
	if(ch->pcdata->security <= 0)
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	for ( cmd = 0; !IS_NULLSTR(cmd_table[cmd].name); cmd++ )
	{
		if ( cmd_table[cmd].level <= get_trust( ch )
				&&   cmd_table[cmd].show == 2)
		{
			send_to_char( Format("\t<send href='help %s' hint='Click to see help on %s'>%-15s\t</send> |", cmd_table[cmd].name, cmd_table[cmd].name, cmd_table[cmd].name), ch );
			if ( ++col % 4 == 0 )
				send_to_char( "\n\r", ch );
		}
	}

	if ( col % 5 != 0 )
		send_to_char( "\n\r", ch );
	return;
}

void do_racecmds( CHAR_DATA *ch, char *argument )
{
	int cmd = 0;
	int col = 0;

	for ( cmd = 0; !IS_NULLSTR(cmd_table[cmd].name); cmd++ )
	{
		if ( cmd_table[cmd].level <  LEVEL_IMMORTAL
				&&   cmd_table[cmd].level <= get_trust( ch )
				&&   cmd_table[cmd].show == 1
				&&   MY_RACE_CMD(ch, cmd))
		{
			send_to_char( Format("\t<send href='%s'>%-15s\t</send> |", cmd_table[cmd].name, cmd_table[cmd].name), ch );
			if ( ++col % 4 == 0 )
				send_to_char( "\n\r", ch );
		}
	}

	if ( col % 4 != 0 )
		send_to_char( "\n\r", ch );
	return;
}

void mob_interpret(CHAR_DATA *ch, char *argument)
{
	char arg[MSL]={'\0'};
	char *argtemp;

    if(IS_NULLSTR(argument))
            return;
    
	argtemp = argument;
	while(argtemp[0] != '\0')
	{
		argtemp = one_line(argtemp, arg);
		if(!IS_NULLSTR(arg))
			interpret(ch, arg);
	}
}

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

#if defined(Macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <time.h>
#include "twilight.h"
#include "magic.h"
#include "interp.h"


/* item type list */
const struct item_type		item_table	[]	=
{
    {	ITEM_LIGHT,		"light"		},
    {	ITEM_SCROLL,	"scroll"	},
    {   ITEM_WEAPON,	"weapon"	},
    {   ITEM_TREASURE,	"treasure"	},
    {   ITEM_ARMOR,		"armor"		},
    {	ITEM_POTION,	"potion"	},
    {	ITEM_CLOTHING,	"clothing"	},
    {   ITEM_FURNITURE,	"furniture"	},
    {	ITEM_TRASH,		"trash"		},
    {	ITEM_CONTAINER,	"container"	},
    {	ITEM_DRINK_CON, "drink"		},
    {	ITEM_KEY,		"key"		},
    {	ITEM_FOOD,		"food"		},
    {	ITEM_MONEY,		"money"		},
    {	ITEM_BOAT,		"boat"		},
    {	ITEM_CORPSE_NPC,"npc_corpse"	},
    {	ITEM_CORPSE_PC,	"pc_corpse"	},
    {   ITEM_FOUNTAIN,	"fountain"	},
    {	ITEM_PILL,		"pill"		},
    {	ITEM_MAP,		"map"		},
    {	ITEM_PORTAL,	"portal"	},
    {	ITEM_TELEPHONE, "phone"		},
    {	ITEM_AMMO,		"ammo"		},
    {	ITEM_BOMB,		"bomb"		},
    {   0,				NULL		}
};


/* wiznet table and prototype for future flag setting */
const   struct wiznet_type      wiznet_table    []              =
{
   {    "on",			WIZ_ON,         WA },
   {    "prefix",		WIZ_PREFIX,		ST },
   {    "logins",		WIZ_LOGINS,     WA },
   {    "sites",		WIZ_SITES,      WA },
   {    "links",		WIZ_LINKS,      WA },
   {	"newbies",		WIZ_NEWBIE,		WA },
   {	"spam",			WIZ_SPAM,		ST },
   {    "deaths",		WIZ_DEATHS,     ST },
   {    "resets",		WIZ_RESETS,     ST },
   {    "mobdeaths",	WIZ_MOBDEATHS,  ST },
   {    "flags",		WIZ_FLAGS,		MA },
   {	"penalties",	WIZ_PENALTIES,	ST },
   {	"load",			WIZ_LOAD,		ST },
   {	"restore",		WIZ_RESTORE,	MA },
   {	"snoops",		WIZ_SNOOPS,		WA },
   {	"switches",		WIZ_SWITCHES,	MA },
   {	"secure",		WIZ_SECURE,		MA },
   {	"communication",WIZ_COMM,		WA },
   {	"staffcomms",	WIZ_STAFF_COMM,	MA },
   {	NULL,			0,				0  }
};

/* attack table  -- not very organized :( */
const 	struct attack_type	attack_table	[MAX_DAMAGE_MESSAGE]	=
{
    { 	"none",		"hit",		-1,		FALSE	},  /*  0 */
    {	"slice",	"slice", 	DAM_SLASH,	FALSE	},	
    {   "stab",		"stab",		DAM_PIERCE,	FALSE	},
    {	"slash",	"slash",	DAM_SLASH,	FALSE	},
    {	"whip",		"whip",		DAM_SLASH,	FALSE	},
    {   "claw",		"claw",		DAM_SLASH,	TRUE	},  /*  5 */
    {	"blast",	"blast",	DAM_BASH,	FALSE	},
    {   "pound",	"pound",	DAM_BASH,	FALSE	},
    {	"crush",	"crush",	DAM_BASH,	FALSE	},
    {   "grep",		"grep",		DAM_SLASH,	FALSE	},
    {	"bite",		"bite",		DAM_PIERCE,	TRUE	},  /* 10 */
    {   "pierce",	"pierce",	DAM_PIERCE,	FALSE	},
    {   "suction",	"suction",	DAM_BASH,	FALSE	},
    {	"beating",	"beating",	DAM_BASH,	FALSE	},
    {   "digestion",	"digestion",	DAM_ACID,	TRUE	},
    {	"charge",	"charge",	DAM_BASH,	FALSE	},  /* 15 */
    { 	"slap",		"slap",		DAM_BASH,	FALSE	},
    {	"punch",	"punch",	DAM_BASH,	FALSE	},
    {	"wrath",	"wrath",	DAM_HOLY,	TRUE	},
    {	"magic",	"magic",	DAM_HOLY,	TRUE	},
    {   "divine",	"divine power",	DAM_HOLY,	TRUE	},  /* 20 */
    {	"cleave",	"cleave",	DAM_SLASH,	FALSE	},
    {	"scratch",	"scratch",	DAM_PIERCE,	FALSE	},
    {   "peck",		"peck",		DAM_PIERCE,	TRUE	},
    {   "peckb",	"peck",		DAM_BASH,	FALSE	},
    {   "chop",		"chop",		DAM_SLASH,	FALSE	},  /* 25 */
    {   "sting",	"sting",	DAM_PIERCE,	TRUE	},
    {   "smash",	"smash",	DAM_BASH,	FALSE	},
    {   "shbite",	"shocking bite",DAM_LIGHTNING,	TRUE	},
    {	"flbite",	"flaming bite", DAM_FIRE,	TRUE	},
    {	"frbite",	"freezing bite",DAM_COLD,	TRUE	},  /* 30 */
    {	"acbite",	"acidic bite", 	DAM_ACID,	TRUE	},
    {	"chomp",	"chomp",	DAM_PIERCE,	TRUE	},
    {  	"drain",	"life drain",	DAM_NEGATIVE,	TRUE	},
    {   "thrust",	"thrust",	DAM_PIERCE,	FALSE	},
    {   "slime",	"slime",	DAM_ACID,	TRUE	},
    {	"shock",	"shock",	DAM_LIGHTNING,	TRUE	},
    {   "thwack",	"thwack",	DAM_BASH,	FALSE	},
    {   "flame",	"flame",	DAM_FIRE,	TRUE	},
    {   "chill",	"chill",	DAM_COLD,	TRUE	},
    {   NULL,		NULL,		0,		0}
};

/* race table */
const 	struct	race_type	race_table	[]		=
{
/*
    {
	name,		pc_race?,
	act bits,	aff_by bits,	off bits,
	imm,		res,		vuln,
	form,		parts 
    },
*/
    {
	"unique",		FALSE,
	0,		0,		0,
	0,		0,		0,		
	0,		0
    },

    { 
	"human",		TRUE, 
	0,		0, 		0,
	0, 		0,		0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    { 
	"werewolf",		TRUE, 
	0,		0, 		0,
	0, 		0,		0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    { 
	"vampire",		TRUE, 
	0,		0, 		0,
	0, 		0,		0,
	A|H|M|V|cc,	A|B|C|D|E|F|G|H|I|J|K
    },

	/* Made Faerie, Mage, Hunter, Wraith all FALSE for right now.  Need to make
	   existing races work properly before adding new ones. */
    { 
	"faerie",		FALSE, 
	0,		0, 		0,
	0, 		0,		0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    { 
	"mage",			FALSE, 
	0,		0, 		0,
	0, 		0,		0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    { 
	"hunter",		FALSE, 
	0,		0, 		0,
	0, 		0,		0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    { 
	"wraith",		FALSE, 
	0,		0, 		0,
	0, 		0,		0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    { 
	"risen",		FALSE, 
	0,		0, 		0,
	0, 		0,		0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    { 
	"demon",		FALSE, 
	0,		0, 		0,
	0, 		0,		0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    { 
	"mummy",		FALSE, 
	0,		0, 		0,
	0, 		0,		0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
	"bat",			FALSE,
	0,		AFF_FLYING|AFF_DARK_VISION,	OFF_DODGE|OFF_FAST,
	0,		0,		VULN_LIGHT,
	A|G|V,		A|C|D|E|F|H|J|K|P
    },

    {
	"bear",			FALSE,
	0,		0,		OFF_CRUSH|OFF_DISARM,
	0,		RES_BASH|RES_COLD,	0,
	A|G|V,		A|B|C|D|E|F|H|J|K|U|V
    },

    {
		"cat",			FALSE,
		0,		AFF_DARK_VISION,	OFF_FAST|OFF_DODGE,
		0,		0,		0,
		A|G|V,		A|C|D|E|F|H|J|K|Q|U|V
    },

    {
		"centipede",		FALSE,
		0,		AFF_DARK_VISION,	0,
		0,		RES_PIERCE|RES_COLD,	VULN_BASH,
		A|B|G|O,		A|C|K
    },

    {
		"dog",			FALSE,
		0,		0,		OFF_FAST,
		0,		0,		0,
		A|G|V,		A|C|D|E|F|H|J|K|U|V
    },

    {
		"dragon", 		FALSE,
		0, 			AFF_INFRARED|AFF_FLYING,	0,
		0,			RES_FIRE|RES_BASH|RES_CHARM,
		VULN_PIERCE|VULN_COLD,
		A|H|Z,		A|C|D|E|F|G|H|I|J|K|P|Q|U|V|X
    },

    {
		"fox",			FALSE,
		0,		AFF_DARK_VISION,	OFF_FAST|OFF_DODGE,
		0,		0,		0,
		A|G|V,		A|C|D|E|F|H|J|K|Q|V
    },

    {
	"goblin",		FALSE,
	0,		AFF_INFRARED,	0,
	0,		RES_DISEASE,	VULN_LIGHT,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
	"lizard",		FALSE,
	0,		0,		0,
	0,		RES_POISON,	VULN_COLD,
	A|G|X|cc,	A|C|D|E|F|H|K|Q|V
    },

    {
	"orc",			FALSE,
	0,		AFF_INFRARED,	0,
	0,		RES_DISEASE,	VULN_LIGHT,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
	"pig",			FALSE,
	0,		0,		0,
	0,		0,		0,
	A|G|V,	 	A|C|D|E|F|H|J|K
    },	

    {
	"rodent",		FALSE,
	0,		0,		OFF_DODGE|OFF_FAST,
	0,		0,		0,
	A|G|V,		A|C|D|E|F|H|J|K
    },
    
    {
	"snake",		FALSE,
	0,		0,		0,
	0,		RES_POISON,	VULN_COLD,
	A|G|X|Y|cc,	A|D|E|F|K|L|Q|V|X
    },
 
    {
	"song bird",		FALSE,
	0,		AFF_FLYING,		OFF_FAST|OFF_DODGE,
	0,		0,		0,
	A|G|W,		A|C|D|E|F|H|K|P
    },

    {
	"troll",		FALSE,
	0,		AFF_REGENERATION|AFF_INFRARED|AFF_DETECT_HIDDEN,
        0,
 	0,	RES_CHARM|RES_BASH,	VULN_FIRE|VULN_ACID,
	A|B|H|M|V,		A|B|C|D|E|F|G|H|I|J|K|U|V
    },

    {
	"water fowl",		FALSE,
	0,		AFF_SWIM|AFF_FLYING,	0,
	0,		RES_DROWNING,		0,
	A|G|W,		A|C|D|E|F|H|K|P
    },		
  
    {
	"wolf",			FALSE,
	0,		AFF_DARK_VISION,	OFF_FAST|OFF_DODGE,
	0,		0,		0,	
	A|G|V,		A|C|D|E|F|J|K|Q|V
    },

    { 
    		"formori",		FALSE,
    		0,		0, 		0,
    		0, 		0,		0,
    		A|H|M|V|cc,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
	NULL, 0, 0, 0, 0, 0, 0
    }
};

const	struct	pc_race_type	pc_race_table	[]	=
{
    { "null race", "", "", "", "",
      { 1, 1, 1, 1, 1, 1, 1, 1, 1 }, 0 },
 
/*
    {
	"race name", 	"short name",	"clan word", "GHB", "RBPG",
	{ max stats },				size 
    },
*/

    {
	"human",	"Hum",	"Group",  "Humanity", "Faith",
	{ 10, 10, 10, 10, 10, 10, 10, 10, 10 },	SIZE_MEDIUM
    },

    {
	"werewolf",	"Wlf",	"Tribe",  "Gnosis",  "Rage",
	{ 10, 10, 10, 10, 10, 10, 10, 10, 10 },	SIZE_MEDIUM
    },

    {
	"vampire",	"Vmp",	"Clan",   "Humanity", "Blood",
	{ 10, 10, 10, 10, 10, 10, 10, 10, 10 },	SIZE_MEDIUM
    },

    {
	"faerie",	"Fae",	"Kith",   "Banality", "Glamour",
	{ 10, 10, 10, 10, 10, 10, 10, 10, 10 },	SIZE_MEDIUM
    },

    {
	"mage",		"Mag",	"Tradition",   "Paradox", "Arete",
	{ 10, 10, 10, 10, 10, 10, 10, 10, 10 },	SIZE_MEDIUM
    },

    {
	"hunter",	"Hun",	"Creed",   "N/A", "Conviction",
	{ 10, 10, 10, 10, 10, 10, 10, 10, 10 },	SIZE_MEDIUM
    },

    {
	"wraith",	"Wth",	"Guild",   "Corpus", "Pathos",
	{ 10, 10, 10, 10, 10, 10, 10, 10, 10 },	SIZE_MEDIUM
    },

    {
	"risen",	"Ris",	"Guild",   "Corpus", "Pathos",
	{ 10, 10, 10, 10, 10, 10, 10, 10, 10 },	SIZE_MEDIUM
    },

    {
	"demon",	"Dem",	"",   "", "",
	{ 10, 10, 10, 10, 10, 10, 10, 10, 10 },	SIZE_MEDIUM
    },

    {
	"mummy",	"Mum",	"Tem-Akh",   "Balance", "Sekhem",
	{ 10, 10, 10, 10, 10, 10, 10, 10, 10 },	SIZE_MEDIUM
    }
};
	
/*
 * Attribute bonus tables.
 */
const	struct	str_app_type	str_app		[26]		=
{
    { -5, -4,   0,  0 },  /* 0  */
    { -5, -4,   3,  1 },  /* 1  */
    { -3, -2,   3,  2 },
    { -3, -1,  10,  3 },  /* 3  */
    { -2, -1,  25,  4 },
    { -2, -1,  55,  5 },  /* 5  */
    { -1,  0,  80,  6 },
    { -1,  0,  90,  7 },
    {  0,  0, 100,  8 },
    {  0,  0, 100,  9 },
    {  0,  0, 115, 10 }, /* 10  */
    {  0,  0, 115, 11 },
    {  0,  0, 130, 12 },
    {  0,  0, 130, 13 }, /* 13  */
    {  0,  1, 140, 14 },
    {  1,  1, 150, 15 }, /* 15  */
    {  1,  2, 165, 16 },
    {  2,  3, 180, 22 },
    {  2,  3, 200, 25 }, /* 18  */
    {  3,  4, 225, 30 },
    {  3,  5, 250, 35 }, /* 20  */
    {  4,  6, 300, 40 },
    {  4,  6, 350, 45 },
    {  5,  7, 400, 50 },
    {  5,  8, 450, 55 },
    {  6,  9, 500, 60 }  /* 25   */
};



/*
 * Liquid properties.
 * Used in converting ITEM_DRINK_CON objects to new ITEM_CONTAINER/WATERPROOF.
 */
const	struct	liq_type	liq_table	[]	=
{
/*    name			color	proof, full, thirst, food, ssize */
    { "water",			"clear",	{   0, 1, 10, 0, 16 }	},
    { "beer",			"amber",	{  12, 1,  8, 1, 12 }	},
    { "red wine",		"burgundy",	{  30, 1,  8, 1,  5 }	},
    { "ale",			"brown",	{  15, 1,  8, 1, 12 }	},
    { "dark ale",		"dark",		{  16, 1,  8, 1, 12 }	},

    { "whisky",			"golden",	{ 120, 1,  5, 0,  2 }	},
    { "lemonade",		"pink",		{   0, 1,  9, 2, 12 }	},
    { "rotgut",			"boiling",	{ 190, 0,  4, 0,  2 }	},
    { "specialty",		"clear",	{ 151, 1,  3, 0,  2 }	},
    { "ooze",			"green",	{   0, 2, -8, 1,  2 }	},

    { "milk",			"white",	{   0, 2,  9, 3, 12 }	},
    { "tea",			"black",	{   0, 1,  8, 0,  6 }	},
    { "coffee",			"black",	{   0, 1,  8, 0,  6 }	},
    { "blood",			"red",		{   0, 2, -10, 2,  6 }	},
    { "salt water",		"clear",	{   0, 1, -2, 0,  1 }	},

    { "coke",			"brown",	{   0, 2,  9, 2, 12 }	}, 
    { "cordial",		"clear",	{ 100, 1,  5, 0,  2 }   },
    { "root beer",		"brown",	{   0, 2,  9, 2, 12 }   },
    { "white wine",		"golden",	{  28, 1,  8, 1,  5 }   },
    { "champagne",		"golden",	{  32, 1,  8, 1,  5 }   },

    { "mead",			"honey-colored",{  34, 2,  8, 2, 12 }   },
    { "rose wine",		"pink",		{  26, 1,  8, 1,  5 }	},
    { "benedictine wine",	"burgundy",	{  40, 1,  8, 1,  5 }   },
    { "vodka",			"clear",	{ 130, 1,  5, 0,  2 }   },
    { "cranberry juice",	"red",		{   0, 1,  9, 2, 12 }	},

    { "orange juice",		"orange",	{   0, 2,  9, 3, 12 }   }, 
    { "absinthe",		"green",	{ 200, 1,  4, 0,  2 }	},
    { "brandy",			"golden",	{  80, 1,  5, 0,  4 }	},
    { "aquavit",		"clear",	{ 140, 1,  5, 0,  2 }	},
    { "schnapps",		"clear",	{  90, 1,  5, 0,  2 }   },

    { "icewine",		"purple",	{  50, 2,  6, 1,  5 }	},
    { "amontillado",		"burgundy",	{  35, 2,  8, 1,  5 }	},
    { "sherry",			"red",		{  38, 2,  7, 1,  5 }   },	
    { "framboise",		"red",		{  50, 1,  7, 1,  5 }   },
    { "rum",			"amber",	{ 151, 1,  4, 0,  2 }	},

    { "ink",			"black",	{   0, 0,  0, 0,  16 }  },
    { "paint",			"white",	{   0, 0,  0, 0,  16 }	},

    { NULL,			NULL,		{   0, 0,  0, 0,  0 }	}
};



/*
 * The skill and spell table.
 * Slot numbers must never be changed as they appear in #OBJECTS sections.
 */
#define SLOT(n)	n

const	struct	skill_type	skill_table	[MAX_SKILL]	=
{

/*
 * Magic spells.
 */

    {
	"reserved",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	0,			TAR_IGNORE,		P_STAND,
	NULL,			SLOT( 0),	 0,	 
        "",			"",		""
    },

    {
	"blindness",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_blindness,	TAR_CHAR_OFFENSIVE,	P_FIGHT,
	&gsn_blindness,		SLOT( 4),	 5,	
	"",			"You can see again.",	""
    },

    {
	"blood amp",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_null,	TAR_IGNORE,		P_STAND,
	NULL,			SLOT(37),	20,	
	"",			"Your blood subsides.",
	""
    },
    
    {
	"blood of potency",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_null,	TAR_IGNORE,		P_STAND,
	NULL,			SLOT(47),	20,	
	"",			"Your blood weakens.",
	""
    },
    
    {
	"charm",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_charm_person,	TAR_CHAR_OFFENSIVE,	P_STAND,
	&gsn_charm_person,	SLOT( 7),	 5,	
	"",			"You feel more self-confident.",	""
    },

    {
	"create food",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_create_food,	TAR_IGNORE,		P_STAND,
	NULL,			SLOT(12),	 5,	
	"",			"!Create Food!",	""
    },

    {
	"cure blindness",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_cure_blindness,	TAR_CHAR_DEFENSIVE,	P_FIGHT,
	NULL,			SLOT(14),	 5,	
	"",			"!Cure Blindness!",	""
    },

    {
	"cure disease",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_cure_disease,	TAR_CHAR_DEFENSIVE,	P_STAND,
	NULL,			SLOT(501),	20,	
	"",			"!Cure Disease!",	""
    },

    {
	"cure poison",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_cure_poison,	TAR_CHAR_DEFENSIVE,	P_STAND,
	NULL,			SLOT(43),	 5,	
	"",			"!Cure Poison!",	""
    },

    {
	"curse",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_curse,		TAR_OBJ_CHAR_OFF,	P_FIGHT,
	&gsn_curse,		SLOT(17),	20,	
	"curse",		"The curse wears off.", 
	"$p is no longer impure."
    },

    {
	"detect hidden",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_detect_hidden,	TAR_CHAR_SELF,		P_STAND,
	NULL,			SLOT(44),	 5,	
	"",			"You feel less aware of your surroundings.",	
	""
    },

    {
	"detect invis",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_detect_invis,	TAR_CHAR_SELF,		P_STAND,
	NULL,			SLOT(19),	 5,	
	"",			"You no longer see invisible objects.",
	""
    },

    {
	"detect poison",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_detect_poison,	TAR_OBJ_INV,		P_STAND,
	NULL,			SLOT(21),	 5,	
	"",			"!Detect Poison!",	""
    },

    {
	"dispel evil",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_dispel_evil,	TAR_CHAR_OFFENSIVE,	P_FIGHT,
	NULL,			SLOT(22),	15,	
	"dispel evil",		"!Dispel Evil!",	""
    },

    {
	"disquiet",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_null,		TAR_CHAR_OFFENSIVE,	P_FIGHT,
	NULL,			SLOT(24),	15,	
	"disquiet",		"You no longer feel so depressed.",	""
    },

    {
	"fly",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_fly,		TAR_CHAR_DEFENSIVE,	P_STAND,
	NULL,			SLOT(56),	10,	
	"",			"You slowly float to the ground.",	""
    },

    {
	"heat metal",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_null,		TAR_CHAR_OFFENSIVE,	P_STAND,
	NULL,			SLOT(23),	10,	
	"heal metal",		"!heat metal!",		""
    },

    {
	"identify",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_identify,		TAR_OBJ_INV,		P_STAND,
	NULL,			SLOT(53),	12,	
	"",			"!Identify!",		""
    },

    {
	"invisibility",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_invis,		TAR_OBJ_CHAR_DEF,	P_STAND,
	&gsn_invis,		SLOT(29),	 5,	
	"",			"You are no longer invisible.",		
	"$p fades into view."
    },

    {
	"locate object",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_locate_object,	TAR_IGNORE,		P_STAND,
	NULL,			SLOT(31),	20,	
	"",			"!Locate Object!",	""
    },

    {
	"luna's avenger",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_null,	TAR_IGNORE,		P_STAND,
	NULL,			SLOT(32),	20,	
	"",			"You are no longer cloaked in silver.",
	"$p is no longer cloaked in silver."
    },

    {
	"luna's blessing",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_null,	TAR_IGNORE,		P_STAND,
	NULL,			SLOT(37),	20,	
	"",			"You no longer feel ambivalent towards silver.",
	""
    },

    {
	"persuasion",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_null,		TAR_IGNORE,		P_STAND,
	NULL,			SLOT(39),	20,	
	"",			"You fell less persuasive.",	""
    },

    {
	"plague",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_plague,		TAR_CHAR_OFFENSIVE,	P_FIGHT,
	&gsn_plague,		SLOT(503),	20,	
	"sickness",		"Your sores vanish.",	""
    },

    {
	"poison",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_poison,		TAR_OBJ_CHAR_OFF,	P_FIGHT,
	&gsn_poison,		SLOT(33),	10,	
	"poison",		"You feel less sick.",	
	"The poison on $p dries up."
    },

    {
	"protection evil",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_protection_evil,	TAR_CHAR_SELF,		P_STAND,
	NULL,			SLOT(34), 	5,	
	"",			"You feel less protected.",	""
    },

    {
	"blood pump",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_null,		TAR_CHAR_SELF,		P_SIT,
	NULL,			SLOT(40),	 	5,
	"",			"The pumping of the blood slows.",""
    },

    {
	"remove curse",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_remove_curse,	TAR_OBJ_CHAR_DEF,	P_STAND,
	NULL,			SLOT(35),	 5,	
	"",			"!Remove Curse!",	""
    },

    {
	"resist pain",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_null,		TAR_OBJ_CHAR_DEF,	P_STAND,
	NULL,			SLOT(36),	 5,	
	"",			"You feel less resistant to injury.",	""
    },

    {
	"shed",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_null,		TAR_IGNORE,		P_STAND,
	NULL,			SLOT(52),	15,	
	"",			"Your new skin toughens up.",	""
    },

    {
	"shroud",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_null,		TAR_ROOM,		P_STAND,
	NULL,			SLOT(54),	15,	
	"",			"The dark shroud lifts from the room.",	""
    },

    {
	"sleep",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_sleep,		TAR_CHAR_OFFENSIVE,	P_STAND,
	&gsn_sleep,		SLOT(38),	15,	
	"",			"You feel less tired.",	""
    },

    {
        "slow",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        spell_slow,             TAR_CHAR_OFFENSIVE,     P_FIGHT,
        NULL,                   SLOT(515),      30,     
        "",                     "You feel yourself speed up.",	""
    },

    {
        "spirit drain",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        spell_null,             TAR_IGNORE,	     P_FIGHT,
        NULL,                   SLOT(516),      30,     
        "",                     "",		""
    },

    {
	"weak arm",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_null,		TAR_CHAR_OFFENSIVE,	P_FIGHT,
	NULL,			SLOT(45),	 	12,
	"",			"You feel stronger.",	""
    },

    {
	"ventriloquate",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_ventriloquate,	TAR_IGNORE,		P_STAND,
	NULL,			SLOT(41),	 	12,
	"",			"!Ventriloquate!",	""
    },

/* Rituals */

    {
	"rite of attunement",
	{0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
	rite_attune,		TAR_IGNORE,		P_REST,
	NULL,			SLOT(47),	 	12,
	"",			"!Attunement!",		""
    },

    {
	"rite of introduction",
	{0, 0, 1, 1, 0, 0, 0, 0, 0, 0},
	rite_introduction,	TAR_IGNORE,		P_REST,
	NULL,			SLOT(48),	 	12,
	"",			"!Introduction!",	""
    },

    {
	"rite of pack creation",
	{0, 0, 1, 1, 0, 0, 0, 0, 0, 0},
	rite_pack,		TAR_IGNORE,		P_STAND,
	NULL,			SLOT(49),	 	12,
	"",			"!Pack create!",	""
    },

    {
	"rite of recognition",
	{0, 0, 1, 1, 0, 0, 0, 0, 0, 0},
	rite_recognition,	TAR_CHAR_DEFENSIVE,	P_STAND,
	NULL,			SLOT(50),	 	12,
	"",			"!Recognition!",	""
    },

    {
	"rite of the returning hero",
	{0, 0, 1, 1, 0, 0, 0, 0, 0, 0},
	rite_hero,		TAR_IGNORE,		P_STAND,
	NULL,			SLOT(51),	 	12,
	"",			"!returning hero!",	""
    },

/* spirit listings */

    {
	"ancestor spirit",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_null,		TAR_CHAR_SELF,		P_FIGHT,
	NULL,			SLOT(42),	 	12,
	"",			"Your ancestor leaves your company",	""
    },

    {
	"war spirit",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_null,		TAR_CHAR_SELF,		P_FIGHT,
	NULL,			SLOT(46),	 	12,
	"",			"The war spirit leaves your company",	""
    },



/* combat and weapons skills */


    {
        "firearm",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        spell_null,             TAR_IGNORE,             P_FIGHT,
        &gsn_firearm,            SLOT( 0),              0,
        "",                     "!Firearm!",		""
    },
 
    {
	"blade",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        spell_null,             TAR_IGNORE,             P_FIGHT,
        &gsn_melee,            	SLOT( 0),               0,
        "",                     "!Blade!",		""
    },

    {
	"blunt",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        spell_null,             TAR_IGNORE,             P_FIGHT,
        &gsn_melee,            	SLOT( 0),               0,
        "",                     "!Blunt!",		""
    },

    {
	"grapple",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        spell_null,             TAR_IGNORE,             P_FIGHT,
        &gsn_melee,            	SLOT( 0),               0,
        "",                     "!Grapple!",		""
    },

    {
	"whip",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        spell_null,             TAR_IGNORE,             P_FIGHT,
        &gsn_melee,            	SLOT( 0),               0,
        "",                     "!Whip!",		""
    },

    {
        "backstab",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        spell_null,             TAR_IGNORE,             P_STAND,
        &gsn_backstab,          SLOT( 0),               2,
        "backstab",             "!Backstab!",		""
    },

    {
	"body slam",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        spell_null,             TAR_IGNORE,             P_FIGHT,
        &gsn_brawl,            	SLOT( 0),               4,
        "Body Slam",            "!Body Slam!",		""
    },

    {
	"dirt kicking", 
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_null,		TAR_IGNORE,		P_FIGHT,
	&gsn_brawl,		SLOT( 0),		4,
	"kicked dirt",		"You rub the dirt out of your eyes.",	""
    },

    {
        "disarm",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        spell_null,             TAR_IGNORE,             P_FIGHT,
        &gsn_disarm,            SLOT( 0),               2,
        "",                     "!Disarm!",		""
    },
 
    {
        "dodge",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        spell_null,             TAR_IGNORE,             P_FIGHT,
        &gsn_dodge,             SLOT( 0),               0,
        "",                     "!Dodge!",		""
    },

    {
        "dread gaze",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        spell_null,             TAR_IGNORE,             P_FIGHT,
        &gsn_dread_gaze,             SLOT( 0),               0,
        "",                     "!Dodge!",		""
    },

    {
	"envenom",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_null,		TAR_IGNORE,	  	P_REST,
	&gsn_envenom,		SLOT(0),		5,
	"",			"!Envenom!",		""
    },

    {
	"brawl",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_null,		TAR_IGNORE,		P_FIGHT,
	&gsn_brawl,		SLOT( 0),		0,
	"",			"!Brawl!",		""
    },

    {
        "kick",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        spell_null,             TAR_CHAR_OFFENSIVE,     P_FIGHT,
        &gsn_brawl,             SLOT( 0),               0,
        "kick",                 "!Kick!",		""
    },

    {
        "parry",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        spell_null,             TAR_IGNORE,             P_FIGHT,
        &gsn_parry,             SLOT( 0),               0,
        "",                     "!Parry!",		""
    },

    {
        "rescue",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        spell_null,             TAR_IGNORE,             P_FIGHT,
        &gsn_rescue,            SLOT( 0),               0,
        "",                     "!Rescue!",		""
    },

    {
	"trip",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_null,		TAR_IGNORE,		P_FIGHT,
	&gsn_brawl,		SLOT( 0),		0,
	"trip",			"!Trip!",		""
    },

/* non-combat skills */


    {
        "darkness",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        spell_null,             TAR_IGNORE,             P_REST,
        &gsn_darkness,          SLOT( 0),               0,
        "",                     "!darkness!",		""
    },

    {
	"haggle",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_null,		TAR_IGNORE,		P_REST,
	&gsn_haggle,		SLOT( 0),		0,
	"",			"!Haggle!",		""
    },

    {
	"hide",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_null,		TAR_IGNORE,		P_REST,
	&gsn_hide,		SLOT( 0),	 	0,
	"",			"!Hide!",		""
    },

    {
	"heighten",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_null,		TAR_IGNORE,		P_REST,
	&gsn_hsenses,		SLOT(902),		0,
	"",			"Your senses return to normal",
	""
    },

    {
	"illusion",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_null,		TAR_IGNORE,		P_REST,
	&gsn_illusion,		SLOT(0),		0,
	"",			"",			""
    },

    {
	"lore",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_null,		TAR_IGNORE,		P_REST,
	&gsn_lore,		SLOT( 0),		0,
	"",			"!Lore!",		""
    },

    {
	"meditation",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_null,		TAR_IGNORE,		P_SLEEP,
	&gsn_meditation,	SLOT( 0),		0,
	"",			"Meditation",		""
    },

    {
        "majesty",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        spell_null,             TAR_IGNORE,             P_REST,
        &gsn_majesty,             SLOT( 0),               0,
        "",                     "You no longer feel majestic.",		""
    },

    {
	"mask",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_null,		TAR_IGNORE,		P_STAND,
	&gsn_mask,		SLOT(901),		0,
	"",			"",			""
    },

    {
	"peek",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_null,		TAR_IGNORE,		P_STAND,
	&gsn_peek,		SLOT( 0),	 	0,
	"",			"!Peek!",		""
    },

    {
	"pick lock",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_null,		TAR_IGNORE,		P_STAND,
	&gsn_pick_lock,		SLOT( 0),	 	0,
	"",			"!Pick!",		""
    },

    {
	"sneak",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_null,		TAR_IGNORE,		P_STAND,
	&gsn_sneak,		SLOT( 0),	 	0,
	"",			"You no longer feel stealthy.",	""
    },

    {
	"steal",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_null,		TAR_IGNORE,		P_STAND,
	&gsn_steal,		SLOT( 0),	 	0,
	"",			"!Steal!",		""
    },

    {
	"scrolls",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_null,		TAR_IGNORE,		P_STAND,
	&gsn_scrolls,		SLOT( 0),		0,
	"",			"!Scrolls!",		""
    },

    {
        "shadow play",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        spell_null,             TAR_IGNORE,             P_REST,
        &gsn_shadow_play,       SLOT( 0),               0,
        "",                     "!shadow play!",	""
    },

    {
        "arms of the abyss",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        spell_null,             TAR_IGNORE,             P_REST,
        &gsn_abyss_arms,        SLOT( 0),               0,
        "",                     "!arms of the abyss!",	""
    },

    {
        "horrid form",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        spell_null,             TAR_IGNORE,             P_STAND,
        &gsn_horrid_form,       SLOT( 0),               0,
        "",                     "!horrid form!",	""
    },

    {
	"eyes of the serpent",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_null,		TAR_IGNORE,		P_REST,
	&gsn_serpent_eyes,	SLOT(0),		0,
	"",			"",			""
    },

    {
	"tongue of the serpent",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_null,		TAR_IGNORE,		P_REST,
	&gsn_serpent_tongue,	SLOT(0),		0,
	"",			"",			""
    },

    {
	"skin of the adder",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_null,		TAR_IGNORE,		P_REST,
	&gsn_adder_skin,	SLOT(0),		0,
	"",			"",			""
    },

    {
	"siren's beckoning",
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_null,		TAR_IGNORE,		P_REST,
	&gsn_siren_beckon,	SLOT(0),		0,
	"",			"",			""
    }

};

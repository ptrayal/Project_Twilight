#if defined(Macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <time.h>
#include "twilight.h"
#include "tables.h"
#include "subcmds.h"
#include "powers.h"
#include "interp.h"

/* >>>>>>>>>>>>>>> Race Table <<<<<<<<<<<<<<< */
const struct clan_type clan_table[MAX_CLAN] =
{
	/*  name,		who entry,	independent */
	/* independent should be FALSE if is a real clan */
	{	"",		"",		TRUE,	-1,	-1,
	{-1, -1, -1},	"",	""     },

	{	"none",		"NONE ",	TRUE,	0,	-1,
	{-1, -1, -1},	"",	""     },

	{	"FBI",		"FBI  ",	FALSE,	HU,	-1,
	{-1, -1, -1},	"",	""     },

	{	"pentex",	"PNTX ",	FALSE,	HU,	P,
	{-1, -1, -1},	"",	""     },

	{	"soc. of Leopold","LEOP ",	FALSE,	HU,	-1,
	{-1, -1, -1},	"",	""     },

	{	"assamite",	"ASSM ",	FALSE,	VA,	-1,
	{DISC_CELERITY, DISC_OBFUSCATE, DISC_QUIETUS},	"",	""  },

	{	"brujah",	"BRUJ ",	FALSE,	VA,	-1,
	{DISC_CELERITY, DISC_POTENCE, DISC_PRESENCE},	"",	""  },

	{	"gangrel",	"GANG ",	FALSE,	VA,	-1,
	{DISC_ANIMALISM, DISC_FORTITUDE, DISC_PROTEAN},	"",	""  },

	{	"giovanni",	"GIOV ",	FALSE,	VA,	-1,
	{DISC_DOMINATE, DISC_NECROMANCY, DISC_POTENCE},	"",	""  },

	{	"lasombra",	"LSMB ",	FALSE,	VA,	-1,
	{DISC_DOMINATE, DISC_OBTENEBRATION, DISC_POTENCE},	"",	""  },

	{	"malkavian",	"MALK ",	FALSE,	VA,	-1,
	{DISC_AUSPEX, DISC_DOMINATE, DISC_OBFUSCATE},	"",	""  },

	{	"nosferatu",	"NOSF ",	FALSE,	VA,	-1,
	{DISC_ANIMALISM, DISC_OBFUSCATE, DISC_POTENCE}, "",	""  },

	{	"ravnos",	"RAVN ",	FALSE,	VA,	-1,
	{DISC_ANIMALISM, DISC_CHIMERSTRY, DISC_FORTITUDE},	"",	""  },

	{	"settite",	"SET  ",	FALSE,	VA,	-1,
	{DISC_OBFUSCATE, DISC_PRESENCE, DISC_SERPENTIS},	"",	""  },

	{	"toreador",	"TORE ",	FALSE,	VA,	-1,
	{DISC_AUSPEX, DISC_CELERITY, DISC_PRESENCE},	"",	""  },

	{	"tremere",	"TREM ",	FALSE,	VA,	-1,
	{DISC_AUSPEX, DISC_DOMINATE, DISC_THAUMATURGY},	"",	""  },

	{	"tzimisce",	"TZIM ",	FALSE,	VA,	-1,
	{DISC_ANIMALISM, DISC_AUSPEX, DISC_VICISSITUDE},	"",	""  },

	{	"ventrue",	"VENT ",	FALSE,	VA,	-1,
	{DISC_DOMINATE, DISC_FORTITUDE, DISC_PRESENCE},	"",	""  },

	{	"black furies",	"FURY ",	FALSE,	WW,	18,
	{DISC_TRIBE, -1, -1},	"wolf",	"werewolf" },

	{	"bone gnawers",	"BGNW ",	FALSE,	WW,	2,
	{DISC_TRIBE, -1, -1},	"wolf",	"werewolf" },

	{	"children of gaia","GAIA ",	FALSE,	WW,	13,
	{DISC_TRIBE, -1, -1},	"wolf",	"werewolf" },

	{	"fenrir",	"FENR ",	FALSE,	WW,	12,
	{DISC_TRIBE, -1, -1},	"wolf",	"werewolf" },

	{	"fianna",	"FIAN ",	FALSE,	WW,	19,
	{DISC_TRIBE, -1, -1},	"wolf",	"werewolf" },

	{	"glass walkers","GLSW ",	FALSE,	WW,	1,
	{DISC_TRIBE, -1, -1},	"wolf",	"werewolf" },

	{	"red talons",	"RTAL ",	FALSE,	WW,	22,
	{DISC_TRIBE, -1, -1},	"wolf",	"werewolf" },

	{	"shadow lords",	"SHDW ",	FALSE,	WW,	17,
	{DISC_TRIBE, -1, -1},	"wolf",	"werewolf" },

	{	"silent striders","STRD ",	FALSE,	WW,	14,
	{DISC_TRIBE, -1, -1},	"wolf",	"werewolf" },

	{	"silver fangs",	"SLVF ",	FALSE,	WW,	3,
	{DISC_TRIBE, -1, -1},	"wolf",	"werewolf" },

	{	"uktena",	"UKTN ",	FALSE,	WW,	25,
	{DISC_TRIBE, -1, -1},	"wolf",	"werewolf" },

	{	"wendigo",	"WNDG ",	FALSE,	WW,	23,
	{DISC_TRIBE, -1, -1},	"wolf",	"werewolf" },
};

/* for powers */
const struct disc_type disc_table[] =
{
	{	"animalism",	"",	"",	"",
	DISC_ANIMALISM },

	{	"auspex",	"",	"",	"",
	DISC_AUSPEX },

	{	"celerity",	"",	"",	"",
	DISC_CELERITY },

	{	"chimerstry",	"",	"",	"",
	DISC_CHIMERSTRY },

	{	"dementation",	"",	"",	"",
	DISC_DEMENTATION },

	{	"dominate",	NULL,	NULL,	"",
	DISC_DOMINATE },

	{	"fortitude",	"",	"",	"",
	DISC_FORTITUDE },

	{	"melpominee",	"",	"",	"",
	DISC_MELPOMINEE },

	{	"necromancy",	"",	"",	"",
	DISC_NECROMANCY },

	{	"obeah",	"",	"",	"",
	DISC_OBEAH },

	{	"obfuscate",	"",	"",	"",
	DISC_OBFUSCATE },

	{	"obtenebration","",	"",	"",
	DISC_OBTENEBRATION },

	{	"potence",	"",	"",	"",
	DISC_POTENCE },

	{	"presence",	"",	"",	"",
	DISC_PRESENCE },

	{	"protean",	"",	"",	"",
	DISC_PROTEAN },

	{	"quietus",	"",	"",	"",
	DISC_QUIETUS },

	{	"serpentis",	"",	"",	"",
	DISC_SERPENTIS },

	{	"thanatosis",	"",	"",	"",
	DISC_THANATOSIS },

	{	"thaumaturgy",	"",	"",	"",
	DISC_THAUMATURGY },

	{	"vicissitude",	"",	"",	"",
	DISC_VICISSITUDE },

	{	NULL,	"",	"",	"",
	-1 }
};

/* for position */
const struct position_type position_table[] =
{
	{	"dead",				"dead"	 },
	{	"mortally wounded",	"mort"	 },
	{   "petrified",        "petrify"},
	{   "torpored",         "torpor" },
	{	"incapacitated",	"incap"	 },
	{	"stunned",			"stun"	 },
	{	"sleeping",			"sleep"	 },
	{	"resting",			"rest"	 },
	{   "sitting",			"sit"    },
	{	"fighting",			"fight"	 },
	{	"standing",			"stand"	 },
	{	NULL,				NULL	 }
};

/* for sex */
const struct sex_type sex_table[] =
{
	{	"none"		},
	{	"male"		},
	{	"female"	},
	{	"random"	},
	{	NULL		}
};

/* for sizes */
const struct size_type size_table[] =
{
	{   "minute",		1,		1	},
	{	"tiny",			5,		2	},
	{	"small", 		10,		100	},
	{	"medium",		20,		110	},
	{	"large",		50,		250	},
	{	"huge", 		100,	350	},
	{	"giant", 		300,	500	},
	{	"gargantuan",	1000,	750	},
	{	NULL,			0,		0	}
};

/* for health levels */
const struct health_type health_table[] =
{
	{   "incapacitated",  -20    },
	{   "crippled",        -5    },
	{   "mauled",          -2    },
	{   "wounded",         -2    },
	{   "injured",         -1    },
	{   "hurt",            -1    },
	{   "bruised",          0    },
	{   "unhurt",           0    }
};

/* for virtues */
const struct virtue_type virtue_table[MAX_VIRTUES] =
{
	{	"conscience"	},
	{	"self-control"	},
	{	"courage"		},
	{	"NULL"			}
};

/* various flag tables */
const struct flag_type act_flags[] =
{
	{	"npc",				A,	FALSE	},
	{	"sentinel",			B,	TRUE	},
	{	"scavenger",		C,	TRUE	},
	{	"aggressive",		F,	TRUE	},
	{	"stay_area",		G,	TRUE	},
	{	"wimpy",			H,	TRUE	},
	{	"pet",				I,	TRUE	},
	{	"intelligent",		J,	FALSE	},
	{	"vampire",			K,	TRUE	},
	{	"werewolf",			O,	TRUE	},
	{	"changeling",		Q,	TRUE	},
	{	"chimerae",			R,	TRUE	},
	{	"fast",				S,	TRUE	},
	{	"umbra",			T,	TRUE	},
	{	"dream",			U,	TRUE	},
	{	"nopurge",			V,	TRUE	},
	{	"outdoors",			W,	TRUE	},
	{	"indoors",			Y,	TRUE	},
	{	"reincarnate",		Z,	TRUE	},
	{	"healer",			aa,	TRUE	},
	{	"update_always",	cc,	TRUE	},
	{	"changer",			dd,	TRUE	},
	{	"twin",				ee,	TRUE	},
	{	NULL,				0,	FALSE	}
};

const struct flag_type act2_flags[] =
{
	{	"police",				A,	TRUE	},
	{	"court",				B,	TRUE	},
	{	"twilight",				C,	TRUE	},
	{	"frenzy",				D,	TRUE	},
	{	"astral",				E,	TRUE	},
	{	"no_hire",				F,	TRUE	},
	{	"employer",				G,	TRUE	},
	{	"resist",				H,	TRUE	},
	{	"wyrm",					I,	TRUE	},
	{	"weaver",				J,	TRUE	},
	{	"wyld",					K,	TRUE	},
	{	"spirit",				L,	TRUE	},
	{	"story",				M,	TRUE	},
	{	"paperboy",				N,	TRUE	},
	{	"shopper",				Q,	TRUE	},
	{	"ghouled",				R,	TRUE	},
	{	"no_warrant",			S,	TRUE	},
	{	"hunter",				T,	FALSE	},
	{	"no_gift_xp",			U,	TRUE	},
	{	"no_quest",				aa,	TRUE	},
	{	"no_quest_aggressor",	bb,	TRUE	},
	{	"no_quest_victim",		cc,	TRUE	},
	{	NULL,					0,	FALSE	}
};

const struct flag_type plr_flags[] =
{
	{	"npc",			A,	FALSE	},
	{	"autoexit",		B,	FALSE	},
	{	"autosplit",	C,	FALSE	},
	{	"holylight",	D,	FALSE	},
	{	"nofollow",		E,	FALSE	},
	{	"permit",		F,	TRUE	},
	{	"log",			G,	FALSE	},
	{	"deny",			H,	FALSE	},
	{	"freeze",		I,	FALSE	},
	{	"colour",		J,	FALSE	},
	{	"rp_ok",		K,	FALSE	},
	{	NULL,			0,	0	}
};

const struct flag_type affect_flags[] =
{
	{	"blind",			A,	TRUE	},
	{	"invisible",		B,	TRUE	},
	{	"dream",			C,	TRUE	},
	{	"detect_invis",		D,	TRUE	},
	{	"enchanted",		E,	TRUE	},
	{	"detect_hidden",	F,	TRUE	},
	{	"umbral",			G,	TRUE	},
	{	"infrared",			J,	TRUE	},
	{	"curse",			K,	TRUE	},
	{	"unused",			L,	TRUE	},
	{	"poison",			M,	TRUE	},
	{	"protection",		N,	TRUE	},
	{	"sneak",			P,	TRUE	},
	{	"hide",				Q,	TRUE	},
	{	"sleep",			R,	TRUE	},
	{	"charm",			S,	TRUE	},
	{	"flying",			T,	TRUE	},
	{	"plague",			X,	TRUE	},
	{	"weaken",			Y,	TRUE	},
	{	"dark_vision",		Z,	TRUE	},
	{	"swim",				bb,	TRUE	},
	{	"regeneration",		cc,	TRUE	},
	{	"slow",				dd,	TRUE	},
	{	"resistpain",		ee,	TRUE	},
	{	NULL,				0,	0	}
};

const struct flag_type affect_flags2[] =
{
	{	"earthmeld",		A,	TRUE	},
	{	NULL,			0,	0	}
};

const struct flag_type off_flags[] =
{
	{	"area_attack",		OFF_AREA_ATTACK,	TRUE	},
	{	"backstab",			OFF_BACKSTAB,		TRUE	},
	{	"bash",				OFF_BASH,			TRUE	},
	{	"disarm",			OFF_DISARM,			TRUE	},
	{	"dodge",			OFF_DODGE,			TRUE	},
	{	"fade",				OFF_FADE,			TRUE	},
	{	"kick",				OFF_KICK,			TRUE	},
	{	"dirt_kick",		OFF_KICK_DIRT,		TRUE	},
	{	"parry",			OFF_PARRY,			TRUE	},
	{	"rescue",			OFF_RESCUE,			TRUE	},
	{	"tail",				OFF_TAIL,			TRUE	},
	{	"trip",				OFF_TRIP,			TRUE	},
	{	"crush",			OFF_CRUSH,			TRUE	},
	{	"fast",				OFF_FAST,			TRUE	},
	{	"tough",			OFF_TOUGH,			TRUE	},
	{	"strong",			OFF_STRONG,			TRUE	},
	{	"assist_all",		ASSIST_ALL,			TRUE	},
	{	"assist_race",		ASSIST_RACE,		TRUE	},
	{	"assist_",			ASSIST_PLAYERS,		TRUE	},
	{	"assist_vnum",		ASSIST_VNUM,		TRUE	},
	{	NULL,			0,	0	}
};

const struct flag_type imm_flags[] =
{
	{	"summon",		A,	TRUE	},
	{	"charm",		B,	TRUE	},
	{	"magic",		C,	TRUE	},
	{	"weapon",		D,	TRUE	},
	{	"bash",			E,	TRUE	},
	{	"pierce",		F,	TRUE	},
	{	"slash",		G,	TRUE	},
	{	"fire",			H,	TRUE	},
	{	"cold",			I,	TRUE	},
	{	"lightning",	J,	TRUE	},
	{	"acid",			K,	TRUE	},
	{	"poison",		L,	TRUE	},
	{	"negative",		M,	TRUE	},
	{	"holy",			N,	TRUE	},
	{	"energy",		O,	TRUE	},
	{	"mental",		P,	TRUE	},
	{	"disease",		Q,	TRUE	},
	{	"drowning",		R,	TRUE	},
	{	"light",		S,	TRUE	},
	{	"sound",		T,	TRUE	},
	{	"wood",			X,	TRUE	},
	{	"silver",		Y,	TRUE	},
	{	"iron",			Z,	TRUE	},
	{	NULL,			0,	0	}
};

const struct flag_type form_flags[] =
{
	{	"edible",			FORM_EDIBLE,		TRUE	},
	{	"poison",			FORM_POISON,		TRUE	},
	{	"magical",			FORM_MAGICAL,		TRUE	},
	{	"instant_decay",	FORM_INSTANT_DECAY,	TRUE	},
	{	"other",			FORM_OTHER,			TRUE	},
	{	"blood",			FORM_BLOOD,			TRUE	},
	{	"animal",			FORM_ANIMAL,		TRUE	},
	{	"sentient",			FORM_SENTIENT,		TRUE	},
	{	"undead",			FORM_UNDEAD,		TRUE	},
	{	"construct",		FORM_CONSTRUCT,		TRUE	},
	{	"mist",				FORM_MIST,			TRUE	},
	{	"intangible",		FORM_INTANGIBLE,	TRUE	},
	{	"biped",			FORM_BIPED,			TRUE	},
	{	"centaur",			FORM_CENTAUR,		TRUE	},
	{	"insect",			FORM_INSECT,		TRUE	},
	{	"spider",			FORM_SPIDER,		TRUE	},
	{	"crustacean",		FORM_CRUSTACEAN,	TRUE	},
	{	"worm",				FORM_WORM,			TRUE	},
	{	"blob",				FORM_BLOB,			TRUE	},
	{	"mammal",			FORM_MAMMAL,		TRUE	},
	{	"bird",				FORM_BIRD,			TRUE	},
	{	"reptile",			FORM_REPTILE,		TRUE	},
	{	"snake",			FORM_SNAKE,			TRUE	},
	{	"dragon",			FORM_DRAGON,		TRUE	},
	{	"amphibian",		FORM_AMPHIBIAN,		TRUE	},
	{	"fish",				FORM_FISH ,			TRUE	},
	{	"cold_blood",		FORM_COLD_BLOOD,	TRUE	},
	{	"shadow",			FORM_SHADOW,		TRUE	},
	{	"horrid_form",		FORM_HORRID,		FALSE	},
	{	"plant",			FORM_PLANT,			TRUE	},
	{	"ashes",			FORM_ASH,			TRUE	},
	{	NULL,			0,			0	}
};

const struct flag_type part_flags[] =
{
	{	"head",					PART_HEAD,			TRUE	},
	{	"arms",					PART_ARMS,			TRUE	},
	{	"legs",					PART_LEGS,			TRUE	},
	{	"heart",				PART_HEART,			TRUE	},
	{	"brains",				PART_BRAINS,		TRUE	},
	{	"guts",					PART_GUTS,			TRUE	},
	{	"hands",				PART_HANDS,			TRUE	},
	{	"feet",					PART_FEET,			TRUE	},
	{	"fingers",				PART_FINGERS,		TRUE	},
	{	"ear",					PART_EAR,			TRUE	},
	{	"eye",					PART_EYE,			TRUE	},
	{	"long tongue",			PART_LONG_TONGUE,	TRUE	},
	{	"eyestalks",			PART_EYESTALKS,		TRUE	},
	{	"tentacles",			PART_TENTACLES,		TRUE	},
	{	"fins",					PART_FINS,			TRUE	},
	{	"wings",				PART_WINGS,			TRUE	},
	{	"tail",					PART_TAIL,			TRUE	},
	{	"webbed hands",			PART_WEBBING,		TRUE	},
	{	"big nose",				PART_BIGNOSE,		TRUE	},
	{	"distorted features",	PART_DISTORTION,	TRUE	},
	{	"claws",				PART_CLAWS,			TRUE	},
	{	"fangs",				PART_FANGS,			TRUE	},
	{	"horns",				PART_HORNS,			TRUE	},
	{	"scales",				PART_SCALES,		TRUE	},
	{	"tusks",				PART_TUSKS,			TRUE	},
	{	"scaled skin",			PART_SCALED_SKIN,	TRUE	},
	{	NULL,					0,					0	}
};


const struct flag_type comm_flags[] =
{
	{	"quiet",		COMM_QUIET,			TRUE	},
	{   "deaf",			COMM_DEAF,			TRUE	},
	{   "nowiz",		COMM_NOWIZ,			FALSE	},
	{   "noclangossip",	COMM_NOAUCTION,		FALSE	},
	{   "noyell",		COMM_NOGOSSIP,		TRUE	},
	{   "noquestion",	COMM_NOQUESTION,	FALSE	},
	{   "noclan",		COMM_NOCLAN,		FALSE	},
	{   "olc",			COMM_OLC,			FALSE	},
	{   "discreet",		COMM_DISCREET,		TRUE	},
	{   "jam_lead",		COMM_JAM_LEAD,		FALSE	},
	{   "jammin",		COMM_JAMMIN,		FALSE	},
	{   "compact",		COMM_COMPACT,		TRUE	},
	{   "brief",		COMM_BRIEF,			TRUE	},
	{   "prompt",		COMM_PROMPT,		TRUE	},
	{   "combine",		COMM_COMBINE,		TRUE	},
	{   "telnet_ga",	COMM_TELNET_GA,		TRUE	},
	{   "nophone",		COMM_NOPHONE,		FALSE	},
	{   "noemote",		COMM_NOEMOTE,		FALSE	},
	{   "noshout",		COMM_NOSHOUT,		FALSE	},
	{   "notell",		COMM_NOTELL,		FALSE	},
	{   "nochannels",	COMM_NOCHANNELS,	FALSE	},
	{   "builder",		COMM_BUILDER,		FALSE	},
	{   "noooc",		COMM_OOC_OFF,		TRUE	},
	{   "telepath",		COMM_THINK_ON,		FALSE	},
	{   "afk",			COMM_AFK,			TRUE	},
	{   "tips",			COMM_TIPS,			TRUE	},
	{	NULL,			0,					0		}
};


const struct flag_type admin_comm_flags[] =
{
	{   "admintalk",		COMM_NOWIZ,		TRUE	},
	{   "olcchannels",		COMM_OLC,		TRUE	},
	{   "builder",			COMM_BUILDER,	TRUE	},
	{   "telepath",			COMM_THINK_ON,	MA		},
	{	NULL,				0,				0	}
};

const struct flag_type plot_races	[] =
{
	{	"human",	HU,		TRUE	},
	{	"werewolf",	WW,		TRUE	},
	{	"vampire",	VA,		TRUE	},
	{	"faerie",	FA,		TRUE	},
	{	NULL,		0,		0	}
};

const struct attrib_type stat_table [MAX_STATS] = {

 { "strength"		},
 { "dexterity"		},
 { "stamina"		},
 { "charisma"		},
 { "manipulation"	},
 { "appearance"		},
 { "perception"		},
 { "intelligence"	},
 { "wits"			},
 { NULL				}

};

const struct skill_list_entry ability_table [MAX_ABIL] = {

/* TALENTS */

 { "acting",		ACTING,			0		},
 { "alertness",		ALERTNESS,		0		},
 { "athletics",		ATHLETICS,		0		},
 { "brawl",			BRAWL,			0		},
 { "dodge",			DODGE,			0		},
 { "empathy",		EMPATHY,		0		},
 { "expression",	EXPRESSION,		0		},
 { "intimidation",	INTIMIDATION,	0		},
 { "kenning",		KENNING,		FA		},
 { "leadership",	LEADERSHIP,		0		},
 { "primal urge",	PRIMAL_URGE,	WW		},
 { "streetwise",	STREETWISE,		0		},
 { "subterfuge",	SUBTERFUGE,		0		},

/* SKILLS */

 { "animal ken",	ANIMAL_KEN,		WW|VA	},
 { "crafts",		CRAFTS,			0		},
 { "drive",			DRIVE,			0		},
 { "etiquette",		ETIQUETTE,		0		},
 { "firearms",		FIREARMS,		0		},
 { "melee",			MELEE,			0		},
 { "performance",	PERFORMANCE,	0		},
 { "repair",		REPAIR,			0		},
 { "larceny",		LARCENY,		0		},
 { "stealth",		STEALTH,		0		},
 { "survival",		SURVIVAL,		0		},

/* KNOWLEDGES */

 { "academics",		ACADEMICS,		0		},
 { "computer",		COMPUTER,		0		},
 { "enigmas",		ENIGMAS,		WW|FA	},
 { "finance",		FINANCE,		0		},
 { "investigation",	INVESTIGATION,	0		},
 { "law",			LAW,			0		},
 { "technology",	TECHNOLOGY,		0		},
 { "medicine",		MEDICINE,		0		},
 { "mythlore",		MYTHLORE,		FA		},
 { "occult",		OCCULT,			0		},
 { "politics",		POLITICS,		0		},
 { "rituals",		RITUALS,		WW		},
 { "science",		SCIENCE,		0		},
 { NULL,			0,				0		}

};

const struct flag_type area_flags[] =
{
	{	"none",			AREA_NONE,		FALSE	},
	{	"added",		AREA_ADDED,		TRUE	},
	{	"changed",		AREA_CHANGED,	TRUE	},
	{	"loading",		AREA_LOADING,	FALSE	},
	{	"slum",			AREA_SLUM,		TRUE	},
	{	"bohemian",		AREA_BOHEMIAN,	TRUE	},
	{	"downtown",		AREA_DOWNTOWN,	TRUE	},
	{	"warehouse",	AREA_WAREHOUSE,	TRUE	},
	{	"suburb",		AREA_SUBURB,	TRUE	},
	{	"patrolled",	AREA_PATROLLED,	TRUE	},
	{	NULL,			0,				0		}
};

const struct flag_type sex_flags[] =
{
	{	"male",			SEX_MALE,		TRUE	},
	{	"female",		SEX_FEMALE,		TRUE	},
	{	"neutral",		SEX_NEUTRAL,	TRUE	},
	{   "random",       3,              TRUE    },   /* ROM */
	{	"none",			SEX_NEUTRAL,	TRUE	},
	{	NULL,			0,				0		}
};

const struct flag_type exit_flags[] =
{
	{   "door",			EX_ISDOOR,		TRUE    },
	{	"closed",		EX_CLOSED,		TRUE	},
	{	"locked",		EX_LOCKED,		TRUE	},
	{	"pickproof",	EX_PICKPROOF,	TRUE	},
	{   "nopass",		EX_NOPASS,		TRUE	},
	{   "easy",			EX_EASY,		TRUE	},
	{   "hard",			EX_HARD,		TRUE	},
	{	"infuriating",	EX_INFURIATING,	TRUE	},
	{	"noclose",		EX_NOCLOSE,		TRUE	},
	{	"nolock",		EX_NOLOCK,		TRUE	},
	{	"hidden",		EX_HIDDEN,		TRUE	},
	{	"window",		EX_WINDOW,		TRUE	},
	{	"broken",		EX_BROKEN,		TRUE	},
	{	"umbra",		EX_UMBRA,		TRUE	},
	{	"dream",		EX_DREAM,		TRUE	},
	{	"nobreak",		EX_NOBREAK,		TRUE	},
	{	"fallthrough",	EX_FALLING,		TRUE	},
	{	"narrow",		EX_NARROW,		TRUE	},
	{	"sealed",		EX_SEALED,		TRUE	},
	{	"squeeze",		EX_SQUEEZE,		TRUE	},
	{	"chompkey",		EX_KEYCHOMP,	TRUE	},
	{	NULL,			0,				0		}
};

const struct flag_type door_resets[] =
{
	{	"open and unlocked",	0,		TRUE	},
	{	"closed and unlocked",	1,		TRUE	},
	{	"closed and locked",	2,		TRUE	},
	{	NULL,					0,		0		}
};

const struct flag_type room_flags[] =
{
	{	"dark",			ROOM_DARK,			TRUE	},
	{	"bank",			ROOM_BANK,			TRUE	},
	{	"no_mob",		ROOM_NO_MOB,		TRUE	},
	{	"indoors",		ROOM_INDOORS,		TRUE	},
	{	"no_phone",		ROOM_NOPHONE,		TRUE	},
	{	"home",			ROOM_HOME,			TRUE	},
	{	"opendoors",	ROOM_OPENDOORS,		FALSE	},
	{	"elevator",		ROOM_ELEVATOR,		TRUE	},
	{	"train",		ROOM_TRAIN,			TRUE	},
	{	"stop",			ROOM_STOP,			TRUE	},
	{	"solitary",		ROOM_SOLITARY,		TRUE    },
	{	"private",		ROOM_PRIVATE,		TRUE    },
	{	"no_recall",	ROOM_NO_RECALL,		TRUE	},
	{	"imp_only",		ROOM_IMP_ONLY,		TRUE    },
	{	"admin_only",	ROOM_ADMIN,			TRUE    },
	{	"polluted",	    ROOM_POLLUTED,		TRUE    },
	{	"corrupted",	ROOM_CORRUPTED,		TRUE    },
	{	"vehicle",		ROOM_VEHICLE,		TRUE	},
	{   "nowhere",		ROOM_NOWHERE,		TRUE	},
	{	NULL,			0,					0		}
};

const struct flag_type sector_flags[] =
{
	{	"inside",	SECT_INSIDE,		TRUE	},
	{	"street",	SECT_STREET,		TRUE	},
	{	"field",	SECT_FIELD,			TRUE	},
	{	"forest",	SECT_FOREST,		TRUE	},
	{	"hills",	SECT_HILLS,			TRUE	},
	{	"mountain",	SECT_MOUNTAIN,		TRUE	},
	{	"swim",		SECT_WATER_SWIM,	TRUE	},
	{	"noswim",	SECT_WATER_NOSWIM,	TRUE	},
	{   "polluted",	SECT_POLLUTED,		TRUE	},
	{	"air",		SECT_AIR,			TRUE	},
	{	"desert",	SECT_DESERT,		TRUE	},
	{	"city",		SECT_CITY,			TRUE	},
	{	NULL,		0,					0		}
};

const struct flag_type type_flags[] =
{
	{	"light",			ITEM_LIGHT,			TRUE	},
	{	"scroll",			ITEM_SCROLL,		TRUE	},
	{	"weapon",			ITEM_WEAPON,		TRUE	},
	{	"treasure",			ITEM_TREASURE,		TRUE	},
	{	"armor",			ITEM_ARMOR,			TRUE	},
	{	"potion",			ITEM_POTION,		TRUE	},
	{	"clothing",			ITEM_CLOTHING,		TRUE	},
	{	"furniture",		ITEM_FURNITURE,		TRUE	},
	{	"trash",			ITEM_TRASH,			TRUE	},
	{	"container",		ITEM_CONTAINER,		TRUE	},
	{	"drinkcontainer",	ITEM_DRINK_CON,		TRUE	},
	{	"key",				ITEM_KEY,			TRUE	},
	{	"food",				ITEM_FOOD,			TRUE	},
	{	"money",			ITEM_MONEY,			FALSE	},
	{	"boat",				ITEM_BOAT,			TRUE	},
	{	"npccorpse",		ITEM_CORPSE_NPC,	TRUE	},
	{	"pc corpse",		ITEM_CORPSE_PC,		FALSE	},
	{	"fountain",			ITEM_FOUNTAIN,		FALSE	},
	{	"pill",				ITEM_PILL,			TRUE	},
	{	"map",				ITEM_MAP,			TRUE	},
	{   "portal",			ITEM_PORTAL,		TRUE	},
	{	"phone",			ITEM_TELEPHONE,		TRUE	},
	{	"ammo",				ITEM_AMMO,			TRUE	},
	{	"bomb",				ITEM_BOMB,			FALSE	},
	{	"liquid",			ITEM_LIQUID,		TRUE	},
	{	"relic",			ITEM_RELIC,			TRUE	},
	{	NULL,				0,					0		}
};



const struct flag_type extra_flags[] =
{
	{	"glow",				ITEM_GLOW,				TRUE	},
	{	"antihuman",		ITEM_ANTI_HUMAN,		TRUE	},
	{	"dark",				ITEM_DARK,				TRUE	},
	{	"lock",				ITEM_LOCK,				TRUE	},
	{	"evil",				ITEM_EVIL,				TRUE	},
	{	"invis",			ITEM_INVIS,				TRUE	},
	{	"magic",			ITEM_MAGIC,				TRUE	},
	{	"nodrop",			ITEM_NODROP,			TRUE	},
	{	"bless",			ITEM_BLESS,				TRUE	},
	{	"antivamp",			ITEM_ANTI_VAMPIRE,		TRUE	},
	{	"antiwolf",			ITEM_ANTI_WEREWOLF,		TRUE	},
	{	"antichangeling",	ITEM_ANTI_CHANGELING,	TRUE	},
	{	"noremove",			ITEM_NOREMOVE,			TRUE	},
	{	"inventory",		ITEM_INVENTORY,			TRUE	},
	{	"nopurge",			ITEM_NOPURGE,			TRUE	},
	{	"rotdeath",			ITEM_ROT_DEATH,			TRUE	},
	{	"visdeath",			ITEM_VIS_DEATH,			TRUE	},
	{   "nonmetal",			ITEM_NONMETAL,			TRUE	},
	{	"nolocate",			ITEM_NOLOCATE,			TRUE	},
	{	"meltdrop",			ITEM_MELT_DROP,			TRUE	},
	{	"hadtimer",			ITEM_HAD_TIMER,			TRUE	},
	{	"sellextract",		ITEM_SELL_EXTRACT,		TRUE	},
	{	"burnproof",		ITEM_BURN_PROOF,		TRUE	},
	{	"nouncurse",		ITEM_NOUNCURSE,			TRUE	},
	{	"draggable",		ITEM_DRAGGABLE,			TRUE	},
	{	"umbral",			ITEM_UMBRAL,			TRUE	},
	{	"dreaming",			ITEM_DREAM,				TRUE	},
	{	"chimerical",		ITEM_CHIMERICAL,		TRUE	},
	{	"hidden",			ITEM_HIDDEN,			TRUE	},
	{	NULL,				0,						0		}
};

const struct flag_type extra2_flags[] =
{
	{	"burning",		OBJ_BURNING,		FALSE	},
	{	"slow_burn",	OBJ_SLOW_BURN,		TRUE	},
	{	"fast_burn",	OBJ_FAST_BURN,		TRUE	},
	{	"no_decay",		OBJ_NO_DECAY,		TRUE	},
	{	"packaged",		OBJ_PACKAGED,		TRUE	},
	{	"waterproof",	OBJ_WATERPROOF,		TRUE	},
	{	"attuned",		OBJ_ATTUNED,		TRUE	},
	{	"furnishing",	OBJ_FURNISHING,		TRUE	},
	{	"keep",			OBJ_KEEP,			TRUE	},
	{	NULL,			0,					0		}
};

const struct flag_type wear_flags[] =
{
	{	"take",			ITEM_TAKE,				TRUE	},
	{	"finger",		ITEM_WEAR_FINGER,		TRUE	},
	{	"neck",			ITEM_WEAR_NECK,			TRUE	},
	{	"body",			ITEM_WEAR_BODY,			TRUE	},
	{	"head",			ITEM_WEAR_HEAD,			TRUE	},
	{	"legs",			ITEM_WEAR_LEGS,			TRUE	},
	{	"feet",			ITEM_WEAR_FEET,			TRUE	},
	{	"hands",		ITEM_WEAR_HANDS,		TRUE	},
	{	"undertop",		ITEM_WEAR_UNDERTOP,		TRUE	},
	{	"underpants",	ITEM_WEAR_UNDERPANTS,	TRUE	},
	{	"about",		ITEM_WEAR_ABOUT,		TRUE	},
	{	"waist",		ITEM_WEAR_WAIST,		TRUE	},
	{	"hip",			ITEM_WEAR_HIP,			TRUE	},
	{	"wrist",		ITEM_WEAR_WRIST,		TRUE	},
	{	"wield",		ITEM_WIELD,				TRUE	},
	{	"hold",			ITEM_HOLD,				TRUE	},
	{	"face",			ITEM_WEAR_FACE,			TRUE	},
	{	"arms",			ITEM_WEAR_ARMS,			TRUE	},
	{	"armpit",		ITEM_WEAR_ARMPIT,		TRUE	},
	{   "twohands",		ITEM_TWO_HANDS,			TRUE	},
	{	NULL,			0,						0		}
};

/*
 * Used when adding an affect to tell where it goes.
 * See addaffect and delaffect in act_olc.c
 */
const struct flag_type apply_flags[] =
{
	{	"none",				APPLY_NONE,			TRUE	},
	{	"strength",			APPLY_STR,			TRUE	},
	{	"dexterity",		APPLY_DEX,			TRUE	},
	{	"stamina",			APPLY_STA,			TRUE	},
	{	"charisma",			APPLY_CHA,			TRUE	},
	{	"manipulation",		APPLY_MAN,			TRUE	},
	{	"appearance",		APPLY_APP,			TRUE	},
	{	"perception",		APPLY_PER,			TRUE	},
	{	"intelligence",		APPLY_INT,			TRUE	},
	{	"wits",				APPLY_WIT,			TRUE	},
	{	"sex",				APPLY_SEX,			TRUE	},
	{	"age",				APPLY_AGE,			TRUE	},
	{	"height",			APPLY_HEIGHT,		TRUE	},
	{	"weight",			APPLY_WEIGHT,		TRUE	},
	{	"spell_aff",		APPLY_SPELL_AFFECT,	TRUE	},
	{	"gn_hum_ban",		APPLY_GHB,			TRUE	},
	{	"r_bp_gl",			APPLY_RBPG,			TRUE	},
	{	"willpower",		APPLY_WILLPOWER,	TRUE	},
	{	"conscience",		APPLY_CONSCIENCE,	TRUE	},
	{	"self-control",		APPLY_SELF_CONTROL,	TRUE	},
	{	"courage",			APPLY_COURAGE,		TRUE	},
	{	"pain",				APPLY_PAIN,			TRUE	},
	{	"anger",			APPLY_ANGER,		TRUE	},
	{	"fear",				APPLY_FEAR,			TRUE	},
	{	"frenzy",			APPLY_FRENZY,		TRUE	},
	{	"health",			APPLY_HEALTH,		TRUE	},
	{	"dice",				APPLY_DICE,			TRUE	},
	{	"difficulty",		APPLY_DIFFICULTY,	TRUE	},
	{	"sex",				APPLY_SEX,			TRUE	},
	{	"height",			APPLY_HEIGHT,		TRUE	},
	{	"weight",			APPLY_WEIGHT,		TRUE	},
	{	"spellaffect",		APPLY_SPELL_AFFECT,	TRUE	},
	{	"special",			APPLY_SPECIAL,		TRUE	},
	{	"roomlight",		APPLY_ROOM_LIGHT,	TRUE	},
	{	"generation",		APPLY_GENERATION,	TRUE	},
	{	NULL,				0,					0		}
};


/*
 * What is seen.
 */
const struct flag_type wear_loc_strings[] =
{
/*    {	"in the inventory",	WEAR_NONE,	TRUE	},*/
	{	"on the face",		WEAR_FACE,	TRUE	},
	{	"on the left finger",	WEAR_FINGER_L,	TRUE	},
	{	"on the right finger",	WEAR_FINGER_R,	TRUE	},
	{	"around the neck",	WEAR_NECK,	TRUE	},
	{	"as an undertop",	WEAR_UNDERTOP,	TRUE	},
	{	"on the body",		WEAR_BODY,	TRUE	},
	{	"over the head",	WEAR_HEAD,	TRUE	},
	{	"as underpants",	WEAR_UNDERPANTS, TRUE	},
	{	"on the legs",		WEAR_LEGS,	TRUE	},
	{	"on the feet",		WEAR_FEET,	TRUE	},
	{	"on the hands",		WEAR_HANDS,	TRUE	},
	{	"on the arms",		WEAR_ARMS,	TRUE	},
	{	"about the shoulders",	WEAR_ABOUT,	TRUE	},
	{	"around the waist",	WEAR_WAIST,	TRUE	},
	{	"on the left wrist",	WEAR_WRIST_L,	TRUE	},
	{	"on the right wrist",	WEAR_WRIST_R,	TRUE	},
	{	"wielded",		WEAR_WIELD,	TRUE	},
	{	"held in the hands",	WEAR_HOLD,	TRUE	},
	{	"on the left hip",	WEAR_LEFT_HIP,	TRUE	},
	{	"on the right hip",	WEAR_RIGHT_HIP,	TRUE	},
	{	"under left armpit",	WEAR_LEFT_PIT,	TRUE	},
	{	"under right armpit",	WEAR_RIGHT_PIT,	TRUE	},
	{	"at home",		WEAR_HOME,	FALSE	},
	{	"in a flap of skin",	WEAR_SKINFLAP,	TRUE	},
	{	NULL,			0	      , 0	}
};

const struct flag_type wear_loc_flags[] =
{
	{	"none",		WEAR_NONE,	TRUE	},
	{	"face",		WEAR_FACE,	TRUE	},
	{	"lfinger",	WEAR_FINGER_L,	TRUE	},
	{	"rfinger",	WEAR_FINGER_R,	TRUE	},
	{	"neck",		WEAR_NECK,	TRUE	},
	{	"undertop",	WEAR_UNDERTOP,	TRUE	},
	{	"body",		WEAR_BODY,	TRUE	},
	{	"head",		WEAR_HEAD,	TRUE	},
	{	"underpants",	WEAR_UNDERPANTS, TRUE	},
	{	"legs",		WEAR_LEGS,	TRUE	},
	{	"feet",		WEAR_FEET,	TRUE	},
	{	"hands",	WEAR_HANDS,	TRUE	},
	{	"arms",		WEAR_ARMS,	TRUE	},
	{	"about",	WEAR_ABOUT,	TRUE	},
	{	"waist",	WEAR_WAIST,	TRUE	},
	{	"lwrist",	WEAR_WRIST_L,	TRUE	},
	{	"rwrist",	WEAR_WRIST_R,	TRUE	},
	{	"wielded",	WEAR_WIELD,	TRUE	},
	{	"hold",		WEAR_HOLD,	TRUE	},
	{	"larmpit",	WEAR_LEFT_PIT,	TRUE	},
	{	"rarmpit",	WEAR_RIGHT_PIT,	TRUE	},
	{	"lhip",		WEAR_LEFT_HIP,	TRUE	},
	{	"rhip",		WEAR_RIGHT_HIP,	TRUE	},
	{	"home",		WEAR_HOME,	FALSE	},
	{	"skinflap",	WEAR_SKINFLAP,	TRUE	},
	{	NULL,		0,		0	}
};

const 	struct flag_type weapon_flags[]=
{
	{ 	"hit",		0,		TRUE	},  /*  0 */
	{	"slice", 	1,		TRUE	},
	{   "stab",		2,		TRUE	},
	{	"slash",	3,		TRUE	},
	{	"whip",		4,		TRUE	},
	{   "claw",		5,		TRUE	},  /*  5 */
	{	"blast",	6,		TRUE	},
	{   "pound",	7,		TRUE	},
	{	"crush",	8,		TRUE	},
	{   "grep",		9,		TRUE	},
	{	"bite",		10,		TRUE	},  /* 10 */
	{   "pierce",	11,		TRUE	},
	{   "suction",	12,		TRUE	},
	{	"beating",	13,		TRUE	},
	{   "digestion",	14,		TRUE	},
	{	"charge",	15,		TRUE	},  /* 15 */
	{ 	"slap",		16,		TRUE	},
	{	"punch",	17,		TRUE	},
	{	"wrath",	18,		TRUE	},
	{	"magic",	19,		TRUE	},
	{   "divinepower",	20,		TRUE	},  /* 20 */
	{	"cleave",	21,		TRUE	},
	{	"scratch",	22,		TRUE	},
	{   "peckpierce",	23,		TRUE	},
	{   "peckbash",	24,		TRUE	},
	{   "chop",		25,		TRUE	},  /* 25 */
	{   "sting",	26,		TRUE	},
	{   "smash",	27,		TRUE	},
	{   "shockingbite",28,		TRUE	},
	{	"flamingbite", 29,		TRUE	},
	{	"freezingbite", 30,		TRUE	},  /* 30 */
	{	"acidicbite", 	31,		TRUE	},
	{	"chomp",	32,		TRUE	},
	{  	"lifedrain",	33,		TRUE	},
	{   "thrust",	34,		TRUE	},
	{   "slime",	35,		TRUE	},
	{	"shocker",	36,		TRUE	},
	{   "thwack",	37,		TRUE	},
	{   "flame",	38,		TRUE	},
	{   "chill",	39,		TRUE	},
	{   NULL,		0,		0	}
};

const 	struct flag_type weapon_dambonus[]=
{
	{ 	"hit",		0,		FALSE	},  /*  0 */
	{	"slice", 	1,		TRUE	},
	{   "stab",		2,		TRUE	},
	{	"slash",	3,		TRUE	},
	{	"whip",		4,		FALSE	},
	{   "claw",		5,		TRUE	},  /*  5 */
	{	"blast",	6,		FALSE	},
	{   "pound",	7,		FALSE	},
	{	"crush",	8,		FALSE	},
	{   "grep",		9,		FALSE	},
	{	"bite",		10,		TRUE	},  /* 10 */
	{   "pierce",	11,		TRUE	},
	{   "suction",	12,		FALSE	},
	{	"beating",	13,		FALSE	},
	{   "digestion",	14,		TRUE	},
	{	"charge",	15,		FALSE	},  /* 15 */
	{ 	"slap",		16,		FALSE	},
	{	"punch",	17,		FALSE	},
	{	"wrath",	18,		TRUE	},
	{	"magic",	19,		TRUE	},
	{   "divinepower",	20,		TRUE	},  /* 20 */
	{	"cleave",	21,		TRUE	},
	{	"scratch",	22,		FALSE	},
	{   "peckpierce",	23,		TRUE	},
	{   "peckbash",	24,		FALSE	},
	{   "chop",		25,		TRUE	},  /* 25 */
	{   "sting",	26,		TRUE	},
	{   "smash",	27,		FALSE	},
	{   "shockingbite",28,		TRUE	},
	{	"flamingbite", 29,		TRUE	},
	{	"freezingbite", 30,		TRUE	},  /* 30 */
	{	"acidicbite", 	31,		TRUE	},
	{	"chomp",	32,		TRUE	},
	{  	"lifedrain",	33,		TRUE	},
	{   "thrust",	34,		TRUE	},
	{   "slime",	35,		FALSE	},
	{	"shock",	36,		TRUE	},
	{   "thwack",	37,		FALSE	},
	{   "flame",	38,		TRUE	},
	{   "chill",	39,		TRUE	},
	{   NULL,		0,		0	}
};

const struct flag_type container_flags[] =
{
	{	"closeable",		1,		TRUE	},
	{	"pickproof",		2,		TRUE	},
	{	"closed",		4,		TRUE	},
	{	"locked",		8,		TRUE	},
	{	"puton",		16,		TRUE	},
	{	NULL,			0,		0	}
};

/*****************************************************************************
Thanks to ROM for the following tables:
****************************************************************************/
const struct flag_type size_flags[] =
{
	{   "minute",        SIZE_MINUTE,          TRUE    },
	{   "tiny",          SIZE_TINY,            TRUE    },
	{   "small",         SIZE_SMALL,           TRUE    },
	{   "medium",        SIZE_MEDIUM,          TRUE    },
	{   "large",         SIZE_LARGE,           TRUE    },
	{   "huge",          SIZE_HUGE,            TRUE    },
	{   "giant",         SIZE_GIANT,           TRUE    },
	{   "gargantuan",    SIZE_GARGANTUAN,      TRUE    },
	{   NULL,              0,                    0       },
};

const struct flag_type weapon_class[] =
{
	{   "exotic",        0,                    TRUE    },
	{   "firearm",       1,                    TRUE    },
	{   "blade",         2,                    TRUE    },
	{   "blunt",         3,                    TRUE    },
	{   "whip",          4,                    TRUE    },
	{   "grapple",       5,                    TRUE    },
	{   NULL,            0,                    0       }
};

const struct flag_type weapon_type2[] =
{
	{   "none",		 		0,					TRUE	},
	{   "flaming",			WEAPON_FLAMING,		TRUE    },
	{   "frost",			WEAPON_FROST,		TRUE    },
	{   "vampiric",			WEAPON_VAMPIRIC,	TRUE    },
	{	"shocking",			WEAPON_SHOCKING,	TRUE    },
	{	"poison",			WEAPON_POISON,		TRUE	},
	{   NULL,				0,					0		}
};

const struct flag_type res_flags[] =
{
	{	"summon",	 RES_SUMMON,		TRUE	},
	{   "charm",         RES_CHARM,            TRUE    },
	{   "magic",         RES_MAGIC,            TRUE    },
	{   "weapon",        RES_WEAPON,           TRUE    },
	{   "bash",          RES_BASH,             TRUE    },
	{   "pierce",        RES_PIERCE,           TRUE    },
	{   "slash",         RES_SLASH,            TRUE    },
	{   "fire",          RES_FIRE,             TRUE    },
	{   "cold",          RES_COLD,             TRUE    },
	{   "lightning",     RES_LIGHTNING,        TRUE    },
	{   "acid",          RES_ACID,             TRUE    },
	{   "poison",        RES_POISON,           TRUE    },
	{   "negative",      RES_NEGATIVE,         TRUE    },
	{   "holy",          RES_HOLY,             TRUE    },
	{   "energy",        RES_ENERGY,           TRUE    },
	{   "mental",        RES_MENTAL,           TRUE    },
	{   "disease",       RES_DISEASE,          TRUE    },
	{   "drowning",      RES_DROWNING,         TRUE    },
	{   "light",         RES_LIGHT,            TRUE    },
	{	"sound",	RES_SOUND,		TRUE	},
	{	"wood",		RES_WOOD,		TRUE	},
	{	"silver",	RES_SILVER,		TRUE	},
	{	"iron",		RES_IRON,		TRUE	},
	{   NULL,          0,            0    }
};

const struct flag_type vuln_flags[] =
{
	{	"summon",	 VULN_SUMMON,		TRUE	},
	{	"charm",	VULN_CHARM,		TRUE	},
	{   "magic",         VULN_MAGIC,           TRUE    },
	{   "weapon",        VULN_WEAPON,          TRUE    },
	{   "bash",          VULN_BASH,            TRUE    },
	{   "pierce",        VULN_PIERCE,          TRUE    },
	{   "slash",         VULN_SLASH,           TRUE    },
	{   "fire",          VULN_FIRE,            TRUE    },
	{   "cold",          VULN_COLD,            TRUE    },
	{   "lightning",     VULN_LIGHTNING,       TRUE    },
	{   "acid",          VULN_ACID,            TRUE    },
	{   "poison",        VULN_POISON,          TRUE    },
	{   "negative",      VULN_NEGATIVE,        TRUE    },
	{   "holy",          VULN_HOLY,            TRUE    },
	{   "energy",        VULN_ENERGY,          TRUE    },
	{   "mental",        VULN_MENTAL,          TRUE    },
	{   "disease",       VULN_DISEASE,         TRUE    },
	{   "drowning",      VULN_DROWNING,        TRUE    },
	{   "light",         VULN_LIGHT,           TRUE    },
	{	"sound",	 VULN_SOUND,		TRUE	},
	{   "wood",          VULN_WOOD,            TRUE    },
	{   "silver",        VULN_SILVER,          TRUE    },
	{   "iron",          VULN_IRON,            TRUE    },
	{   NULL,              0,                    0       }
};

const struct flag_type position_flags[] =
{
	{   "dead",           P_DEAD,            FALSE   },
	{   "mortal",         P_MORT,            FALSE   },
	{   "incap",          P_INCAP,           FALSE   },
	{   "stunned",        P_STUN,            FALSE   },
	{   "sleeping",       P_SLEEP,           TRUE    },
	{   "resting",        P_REST,            TRUE    },
	{   "sitting",        P_SIT,             TRUE    },
	{   "fighting",       P_FIGHT,           FALSE   },
	{   "standing",       P_STAND,           TRUE    },
	{   NULL,              0,                    0       }
};

const struct flag_type portal_flags[]=
{
	{   "normal_exit",	  GATE_NORMAL_EXIT,	TRUE	},
	{	"no_curse",	  GATE_NOCURSE,		TRUE	},
	{   "go_with",	  GATE_GOWITH,		TRUE	},
	{   "buggy",	  GATE_BUGGY,		TRUE	},
	{	"random",	  GATE_RANDOM,		TRUE	},
	{   NULL,		  0,			0	}
};
const struct flag_type furniture_flags[]=
{
	{	"none",		  0,			TRUE	},
	{   "stand_at",	  STAND_AT,		TRUE	},
	{	"stand_on",	  STAND_ON,		TRUE	},
	{	"stand_in",	  STAND_IN,		TRUE	},
	{	"sit_at",	  SIT_AT,		TRUE	},
	{	"sit_on",	  SIT_ON,		TRUE	},
	{	"sit_in",	  SIT_IN,		TRUE	},
	{	"rest_at",	  REST_AT,		TRUE	},
	{	"rest_on",	  REST_ON,		TRUE	},
	{	"rest_in",	  REST_IN,		TRUE	},
	{	"sleep_at",	  SLEEP_AT,		TRUE	},
	{	"sleep_on",	  SLEEP_ON,		TRUE	},
	{	"sleep_in",	  SLEEP_IN,		TRUE	},
	{	"put_at",	  PUT_AT,		TRUE	},
	{	"put_on",	  PUT_ON,		TRUE	},
	{	"put_in",	  PUT_IN,		TRUE	},
	{	"put_inside",	  PUT_INSIDE,		TRUE	},
	{	NULL,		  0,			0	}
};

const   struct  exit_table_type         exit_table      [MAX_EXITS] =
{
	{	"north",	DIR_NORTH	},
	{	"east",		DIR_EAST	},
	{	"south",	DIR_SOUTH	},
	{	"west",		DIR_WEST	},
	{	"up",		DIR_UP		},
	{	"down",		DIR_DOWN	}/*,
	{	NULL,		-1		}*/
};

/*********************************************************************
		Project Twilight Specific Tables
**********************************************************************/

const	struct	move_type combat_move_table [] =
{
	/*
	{
	IS_END_MOVE,
	PART_INVOLVED,	MOVE_TYPE
	}

	{
	TRUE,
	AP_PART_NONE,	MOVE_NONE
	},
	*/

	{
	FALSE,
	AP_LEGS,	MOVE_STAND
	},

	{
	FALSE,
	AP_PART_NONE,	MOVE_FLIP
	},

	{
	FALSE,
	AP_BODY,	MOVE_ROLL
	},

	{
	FALSE,
	AP_LEGS,	MOVE_JUMP
	},

	{
	TRUE,
	AP_LEFT_HAND,	MOVE_LBLOW
	},

	{
	TRUE,
	AP_RIGHT_HAND,	MOVE_RBLOW
	},

	{
	FALSE,
	AP_LEFT_HAND,	MOVE_LGRAB
	},

	{
	FALSE,
	AP_RIGHT_HAND,	MOVE_RGRAB
	},

	{
	TRUE,
	AP_FEET,	MOVE_KICK
	},

	{
	TRUE,
	AP_LEGS,	MOVE_SKICK
	},

	{
	TRUE,
	AP_HEAD,	MOVE_BITE
	},

	{
	TRUE,
	AP_LEFT_HAND,	MOVE_LBLOCK
	},

	{
	TRUE,
	AP_RIGHT_HAND,	MOVE_RBLOCK
	},

	{
	TRUE,
	AP_FEET,	MOVE_FBLOCK
	},

	{
	FALSE,
	AP_BODY,	MOVE_SPIN
	},

	{
	TRUE,
	AP_LEGS,	MOVE_SWEEP
	},

	{
	TRUE,
	AP_RIGHT_HAND,	MOVE_SHOOT
	},

	{
	TRUE,
	AP_BODY,	MOVE_DODGE
	},

	{
	TRUE,
	AP_BODY,	MOVE_TOUCH
	},

	{
	TRUE,
	AP_BODY,	MOVE_TONGUE
	}

};

const struct combo_type combo_table [] =
{
	/*
	{
	{ move1, move2, move3, move4 },
		"To character string",
		"To victim string",
		"To room string",
	"Botch string to character",
	"Botch string to victim",
	"Botch string to room",
	difficulty,
	damage,
	is_footstrike
	}

	{
	{ -1, -1, -1, -1 },
	"You stop dead.",
	"$n stops in $m tracks.",
	"$n stops in $m tracks.",
	"You stop off balance.",
	"$n stops suddenly, slightly off balance.",
	"$n stops suddenly, slightly off balance.",
	1,
	0,
	FALSE
	},
	*/

	{
	{ 0, -2, -2, -2 },
	"You stand up.",
	"$n stands up.",
	"$n stands up.",
	"You stand up, taking a moment to regain balance.",
	"$n stands up, taking a moment to regain $m balance.",
	"$n stands up, taking a moment to regain $m balance.",
	0,
	0,
	FALSE
	},

	{
	{ 1, -2, -2, -2 },
	"You flip and land on your feet.",
	"$n flips to land on $m feet.",
	"$n flips to land on $m feet.",
	"You flail your body and land on the ground.",
	"$n flails trying to flip, landing on $m back.",
	"$n flails trying to flip, landing on $m back.",
	2,
	0,
	FALSE
	},

	{
	{ 2, -2, -2, -2 },
	"You do a tumbling roll.",
	"$n curls into a tumbling roll.",
	"$n curls into a tumbling roll.",
	"You fail to tuck in time and fall flat.",
	"$n flops onto the ground.",
	"$n flops onto the ground.",
	1,
	0,
	FALSE
	},

	{
	{ 3, -2, -2, -2 },
	"You leap into the air.",
	"$n leaps into the air.",
	"$n leaps into the air.",
	"You stumble trying to get airborne.",
	"$n stumbles trying to get airborne.",
	"$n stumbles trying to get airborne.",
	1,
	0,
	FALSE
	},

	{
	{ 4, -2, -2, -2 },
	"You swing from the left at $N.",
	"$n swings from the left at $N.",
	"$n swings from the left at you.",
	"You throw yourself off balance!",
	"$n falls off balance trying to hit $N.",
	"$n falls off balance trying to hit you.",
	2,
	0,
	FALSE
	},

	{
	{ 5, -2, -2, -2 },
	"You swing from the right at $N.",
	"$n swings from the right at $N.",
	"$n swings from the right at you.",
	"You throw yourself off balance!",
	"$n falls off balance trying to hit $N.",
	"$n falls off balance trying to hit you.",
	2,
	0,
	FALSE
	},

	{
	{ 6, -2, -2, -2 },
	"You make a grab from the left at $N.",
	"$n makes a grab from the left at $N.",
	"$n makes a grab from the left at you.",
	"You throw yourself off balance!",
	"$n falls off balance trying to grab $N.",
	"$n falls off balance trying to grab you.",
	3,
	0,
	FALSE
	},

	{
	{ 7, -2, -2, -2 },
	"You make a grab from the right at $N.",
	"$n makes a grab from the right at $N.",
	"$n makes a grab from the right at you.",
	"You throw yourself off balance!",
	"$n falls off balance trying to grab $N.",
	"$n falls off balance trying to grab you.",
	3,
	0,
	FALSE
	},

	{
	{ 8, -2, -2, -2 },
	"You flick a snap kick at $N.",
	"$n flicks a snap kick at $N.",
	"$n flicks a snap kick at you.",
	"You throw yourself off balance!",
	"$n falls off balance trying to kick $N.",
	"$n falls off balance trying to kick you.",
	2,
	2,
	TRUE
	},

	{
	{ 9, -2, -2, -2 },
	"You swing a side kick at $N.",
	"$n swings a side kick at $N.",
	"$n swings a side kick at you.",
	"You throw yourself off balance!",
	"$n falls off balance trying to kick $N.",
	"$n falls off balance trying to kick you.",
	3,
	3,
	TRUE
	},

	{
	{ 10, -2, -2, -2 },
	"You bite at $N.",
	"$n bites at $N.",
	"$n bites at you.",
	"You bite your own lip.",
	"$n bites $m own lip snapping at $N.",
	"$n bites $m own lip snapping at you.",
	1,
	1,
	TRUE
	},

	{
	{ 11, -2, -2, -2 },
	"You prepare to block from the left.",
	"$n guards $m left.",
	"$n guards $m left.",
	"You prepare to block from the left.",
	"$n uses a totally ineffective block on the left.",
	"$n uses a totally ineffective block on the left.",
	3,
	0,
	FALSE
	},

	{
	{ 12, -2, -2, -2 },
	"You prepare to block from the right.",
	"$n guards $m right.",
	"$n guards $m right.",
	"You prepare to block from the right.",
	"$n uses a totally ineffective block on the right.",
	"$n uses a totally ineffective block on the right.",
	3,
	0,
	FALSE
	},

	{
	{ 13, -2, -2, -2 },
	"You take the weight off one foot, preparing it to block attacks.",
	"$n bounces slightly.",
	"$n bounces slightly.",
	"You take the weight off one foot, preparing it to block attacks.",
	"$n undermines $m stance.",
	"$n undermines $m stance.",
	4,
	0,
	FALSE
	},

	{
	{ 14, -2, -2, -2 },
	"You gather momentum by spinning in a circle.",
	"$n spins, gaining momentum.",
	"$n spins, gaining momentum.",
	"You start to spin and unbalance yourself.",
	"$n starts to turn and stumbles.",
	"$n starts to turn and stumbles.",
	5,
	0,
	FALSE
	},

	{
	{ 15, -2, -2, -2 },
	"You sweep at $N's legs.",
	"$n sweeps at your legs.",
	"$n sweeps at $N's legs.",
	"You sweep at $N's legs, but fall over.",
	"$n sweeps at $N's legs, but falls over.",
	"$n sweeps at your legs, but falls over.",
	2,
	1,
	TRUE
	},

	{
	{ 16, -2, -2, -2 },
	"You shoot at $N.",
	"$n shoots at you.",
	"$n shoots at $N.",
	"You shoot at $N, but the shot goes wild.",
	"$n shoots at $N, but the shot goes wild.",
	"$n shoots at you, but the shot goes wild.",
	2,
	1,
	TRUE
	},

	{
	{ 17, -2, -2, -2 },
	"You prepare to dodge.",
	"$n tenses and relaxes.",
	"$n tenses and relaxes.",
	"You shift your weight, but it doesn't feel right.",
	"$n tenses and relaxes.",
	"$n tenses and relaxes.",
	2,
	1,
	TRUE
	},

	{
	{ 18, -2, -2, -2 },
	"You reach to touch $N.",
	"$n reaches to touch you.",
	"$n reaches to touch $N.",
	"You reach for $N, but miss.",
	"$n reaches for $N, but miss.",
	"$n reaches for you, but miss.",
	2,
	1,
	TRUE
	},


	{
	{ 19, -2, -2, -2 },
	"You shoot your tongue at $N.",
	"$n shoots $s tongue at you.",
	"$n shoots $s tongue at $N.",
	"You shoot your tongue at $N, but miss.",
	"$n shoots $s tongue at $N, but misses.",
	"$n shoots $s tongue at you, but misses.",
	2,
	1,
	TRUE
	},

	{
	{ 2, 15, -2, -2 },
	"You roll at $N and swing a sweeping blow at $m legs.",
	"$n rolls at you and swings a sweeping blow at your legs.",
	"$n rolls at $N and swings a sweeping blow at $m legs.",
	"",
	"",
	"",
	3,
	1,
	TRUE
	},

	{
	{ 14, 8, -2, -2 },
	"You spin and swing a circle kick at $N.",
	"$n spins and swings a circle kick at you.",
	"$n spins and swings a circle kick at $N.",
	"",
	"",
	"",
	3,
	3,
	TRUE
	},

	{
	{ 3, 8, -2, -2 },
	"You perform a flying kick at $N.",
	"$n performs a flying kick at you.",
	"$n performs a flying kick at $N.",
	"",
	"",
	"",
	3,
	3,
	TRUE
	},

	{
	{ 3, 14, 8, -2 },
	"You perform a flying circle kick at $N.",
	"$n performs a flying circle kick at you.",
	"$n performs a flying circle kick at $N.",
	"",
	"",
	"",
	4,
	4,
	TRUE
	},

	{
	{ 3, 2, 8, -2 },
	"You perform a flying roll kick at $N.",
	"$n performs a flying roll kick at you.",
	"$n performs a flying roll kick at $N.",
	"",
	"",
	"",
	4,
	3,
	TRUE
	},

	{
	{ 3, 1, 8, -2 },
	"You perform a flying backflip and kick at $N.",
	"$n performs a flying backflip and kick at you.",
	"$n performs a flying backflip and kick at $N.",
	"",
	"",
	"",
	4,
	4,
	TRUE
	},

	{
	{ 3, 4, -2, -2 },
	"You leap into the air and strike down at $N.",
	"$n leaps into the air and strikes down at you.",
	"$n leaps into the air and strikes down at $N.",
	"",
	"",
	"",
	3,
	1,
	FALSE
	},

	{
	{ 3, 5, -2, -2 },
	"You leap into the air and strike down at $N.",
	"$n leaps into the air and strikes down at you.",
	"$n leaps into the air and strikes down at $N.",
	"",
	"",
	"",
	3,
	1,
	FALSE
	},

	{
	{ 6, 7, 10, -2 },
	"You take hold of $N in both hands and bite $m hard.",
	"$n takes hold of you in both hands and bites you hard.",
	"$n takes hold of $N in both hands and bites $m hard.",
	"",
	"",
	"",
	-1,
	1,
	TRUE
	},

	{
	{ 6, 5, -2, -2 },
	"You grab $N with your left and strike from the right.",
	"$n grabs you with $m left and strikes from the right.",
	"$n grabs $N with $m left and strikes from the right.",
	"",
	"",
	"",
	-1,
	0,
	FALSE
	},

	{
	{ 7, 4, -2, -2 },
	"You grab $N with your right and strike from the left.",
	"$n grabs you with $m right and strikes from the left.",
	"$n grabs $N with $m right and strikes from the left.",
	"",
	"",
	"",
	-1,
	0,
	FALSE
	},

	{
	{ 6, 7, 8, -2 },
	"You grab $N and hurl $M away.",
	"$n grabs you and hurls you away.",
	"$n grabs $N and hurls $M away.",
	"",
	"",
	"",
	3,
	2,
	TRUE
	},

	{
	{ 6, 7, 9, -2 },
	"You grab $N and hurl $M over your head.",
	"$n grabs you and hurls you over $m head.",
	"$n grabs $N and hurls $M over $m head.",
	"",
	"",
	"",
	4,
	3,
	TRUE
	},

	{
	{ 2, 8, -2, -2 },
	"You roll at $N and kick out at $M low.",
	"$n rolls at you and kicks out low at you.",
	"$n rolls at $N and kicks out at $M low.",
	"",
	"",
	"",
	3,
	2,
	TRUE
	},

	{
	{ 1, 8, -2, -2 },
	"You do a backflip, kicking $N in the chest.",
	"$n does a backflip, kicking you in the chest.",
	"$n does a backflip, kicking $N in the chest.",
	"",
	"",
	"",
	3,
	3,
	TRUE
	},

	{
	{ 2, 0, 3, 8 },
	"You tumble and leap at $N kicking $M.",
	"$n tumbles and leaps at you kicking out at you.",
	"$n tumbles and leaps at $N kicking $M.",
	"",
	"",
	"",
	4,
	3,
	TRUE
	},

	{
	{ 14, 1, 5, -2 },
	"You spin and flip, coming out with a blow at $N.",
	"$n spins and flips, coming out with a blow at you.",
	"$n spins and flips, coming out with a blow at $N.",
	"",
	"",
	"",
	2,
	2,
	FALSE
	},

	{
	{ 14, 1, 4, -2 },
	"You spin and flip, coming out with a blow at $N.",
	"$n spins and flips, coming out with a blow at you.",
	"$n spins and flips, coming out with a blow at $N.",
	"",
	"",
	"",
	2,
	2,
	FALSE
	},

	{
	{ 14, 4, -2, -2 },
	"You swing in a circle striking out at $N.",
	"$n swings in a circle striking out at you.",
	"$n swings in a circle striking out at $N.",
	"",
	"",
	"",
	2,
	2,
	FALSE
	},

	{
	{ 14, 5, -2, -2 },
	"You swing in a circle striking out at $N.",
	"$n swings in a circle striking out at you.",
	"$n swings in a circle striking out at $N.",
	"",
	"",
	"",
	2,
	2,
	FALSE
	},

	{
	{ 11, 12, 13, -2 },
	"You prepare for any attack.",
	"$n prepares for incoming attacks.",
	"$n prepares for incoming attacks.",
	"",
	"",
	"",
	5,
	0,
	TRUE
	},

	{
	{ -2, -2, -2, -2 },
	NULL,
	"",
	"",
	"",
	"",
	"",
	0,
	0
	}

};

const struct flag_type material_type[] =
{
	{   "none",              0,            TRUE    },
	{   "flesh",             0,            TRUE    },
	{   "wood",              0,            TRUE    },
	{   "steel",             0,            TRUE    },
	{   "poly-carbide steel",0,            TRUE    },
	{   "iron",              0,            TRUE    },
	{   "silver",            0,            TRUE    },
	{   "gold",              0,            TRUE    },
	{   "copper",            0,            TRUE    },
	{   "rubber",            0,            TRUE    },
	{   "cotton",            0,            TRUE    },
	{   "leather",           0,            TRUE    },
	{   "plastic",           0,            TRUE    },
	{   "bread",             0,            TRUE    },
	{   "meat",              0,            TRUE    },
	{   "egg",               0,            TRUE    },
	{   "vinyl",             0,            TRUE    },
	{   "wool",              0,            TRUE    },
	{   "glass",             0,            TRUE    },
	{   "platinum",          0,            TRUE    },
	{   "silicon",           0,            TRUE    },
	{   "silicon gel",       0,            TRUE    },
	{   "granite",           0,            TRUE    },
	{   "diamond",           0,            TRUE    },
	{   "ruby",              0,            TRUE    },
	{   "emerald",           0,            TRUE    },
	{   "sapphire",          0,            TRUE    },
	{   "opal",              0,            TRUE    },
	{   "marble",            0,            TRUE    },
	{   "titanium",          0,            TRUE    },
	{   "lead",              0,            TRUE    },
	{   "pewter",            0,            TRUE    },
	{   "aluminium",         0,            TRUE    },
	{   "paper",             0,            TRUE    },
	{   "cardboard",         0,            TRUE    },
	{   "slime",             0,            TRUE    },
	{   "concrete",          0,            TRUE    },
	{   "polyester",         0,            TRUE    },
	{   "compost",           0,            TRUE    },
	{   "c-4",               0,            TRUE    },
	{   "dynamite",          0,            TRUE    },
	{   "rock",              0,            TRUE    },
	{   "basalt",            0,            TRUE    },
	{   "obsidian",          0,            TRUE    },
	{   "uranium",           0,            TRUE    },
	{   "alloy",             0,            TRUE    },
	{   "cereal",            0,            TRUE    },
	{   "spandex",           0,            TRUE    },
	{   "brick",             0,            TRUE    },
	{   "fibreglass",        0,            TRUE    },
	{   "carbon fibre",      0,            TRUE    },
	{   "light",             0,            TRUE    },
	{   "bitumen",           0,            TRUE    },
	{   "tile",              0,            TRUE    },
	{   "terracotta",        0,            TRUE    },
	{   "clay",              0,            TRUE    },
	{   "felt",              0,            TRUE    },
	{   "plexi-glass",       0,            TRUE    },
	{   "slate",             0,            TRUE    },
	{   "corrugated iron",   0,            TRUE    },
	{   "wire mesh",         0,            TRUE    },
	{   "shingle",           0,            TRUE    },
	{   "lace",              0,            TRUE    },
	{   "satin",             0,            TRUE    },
	{   "silk",              0,            TRUE    },
	{   "string",            0,            TRUE    },
	{   "rope",              0,            TRUE    },
	{   "hemp",              0,            TRUE    },
	{   "cocaine",           0,            TRUE    },
	{   "lsd",               0,            TRUE    },
	{   "heroin",            0,            TRUE    },
	{   "flannelette",       0,            TRUE    },
	{   "coconut",           0,            TRUE    },
	{   "nuts",              0,            TRUE    },
	{   "cheese",            0,            TRUE    },
	{   "butter",            0,            TRUE    },
	{   "curry",             0,            TRUE    },
	{   "garlic",            0,            TRUE    },
	{   "berries",           0,            TRUE    },
	{   "onion",             0,            TRUE    },
	{   "salt",              0,            TRUE    },
	{   "sugar",             0,            TRUE    },
	{   "fruit",             0,            TRUE    },
	{   "pasta",             0,            TRUE    },
	{   "mushroom",          0,            TRUE    },
	{   "vegetables",        0,            TRUE    },
	{   "seaweed",           0,            TRUE    },
	{   "tofu",              0,            TRUE    },
	{   "pastry",            0,            TRUE    },
	{   "chocolate",         0,            TRUE    },
	{   "ceramic",           0,            TRUE    },
	{   "water",	     0,            TRUE    },
	{   "beer",		     0,		   TRUE    },
	{   "red_wine",	     0,		   TRUE    },
	{   "ale",		     0,		   TRUE    },
	{   "dark_ale",	     0,		   TRUE    },
	{   "whisky",	     0,		   TRUE    },
	{   "lemonade",	     0,		   TRUE    },
	{   "rotgut",	     0,            TRUE    },
	{   "specialty",	     0,		   TRUE    },
	{   "ooze",		     0,		   TRUE    },
	{   "milk",		     0,		   TRUE    },
	{   "tea",		     0,		   TRUE    },
	{   "coffee",	     0,		   TRUE    },
	{   "blood",	     0,		   TRUE    },
	{   "salt_water",	     0,		   TRUE    },
	{   "coke",		     0,		   TRUE    },
	{   "cordial",	     0,		   TRUE    },
	{   "root_beer",	     0,		   TRUE    },
	{   "white_wine",	     0,		   TRUE    },
	{   "champagne",	     0,		   TRUE    },
	{   "mead",		     0,		   TRUE    },
	{   "rose_wine",	     0,		   TRUE    },
	{   "benedictine",	     0,		   TRUE    },
	{   "vodka",	     0,		   TRUE    },
	{   "cranberry_juice",   0,		   TRUE    },
	{   "oj",		     0,		   TRUE    },
	{   "absinthe",	     0,		   TRUE    },
	{   "brandy",	     0,		   TRUE    },
	{   "aquavit",	     0,		   TRUE    },
	{   "schnapps",	     0,		   TRUE    },
	{   "icewine",	     0,		   TRUE    },
	{   "amontillado",	     0,		   TRUE    },
	{   "sherry",	     0,		   TRUE    },
	{   "framboise",	     0,		   TRUE    },
	{   "rum",		     0,		   TRUE    },
	{   "ink",		     0,		   TRUE    },
	{   "paint",	     0,		   TRUE    },
	{   NULL,                0,            0       }
};

/* Kept for conversion of objects. */
const struct flag_type liquid_flags[]=
{
	{ "water",		0,	TRUE},
	{ "beer",		1,	TRUE},
	{ "red_wine",	2,	TRUE},
	{ "ale",		3,	TRUE},
	{ "dark_ale",	4,	TRUE},

	{ "whisky",		5,	TRUE},
	{ "lemonade",	6,	TRUE},
	{ "rotgut",		7,	TRUE},
	{ "specialty",	8,	TRUE},
	{ "ooze",		9,	TRUE},

	{ "milk",		10,	TRUE},
	{ "tea",		11,	TRUE},
	{ "coffee",		12,	TRUE},
	{ "blood",		13,	TRUE},
	{ "salt_water",	14,	TRUE},

	{ "coke",		15,	TRUE},
	{ "cordial",	16,	TRUE},
	{ "root_beer",	17,	TRUE},
	{ "white_wine",	18,	TRUE},
	{ "champagne",	19,	TRUE},

	{ "mead",		20,	TRUE},
	{ "rose_wine",	21,	TRUE},
	{ "benedictine",	22,	TRUE},
	{ "vodka",		23,	TRUE},
	{ "cranberry_juice",24,	TRUE},

	{ "oj",		25,	TRUE},
	{ "absinthe",	26,	TRUE},
	{ "brandy",		27,	TRUE},
	{ "aquavit",	28,	TRUE},
	{ "schnapps",	29,	TRUE},

	{ "icewine",	30,	TRUE},
	{ "amontillado",	31,	TRUE},
	{ "sherry",		32,	TRUE},
	{ "framboise",	33,	TRUE},
	{ "rum",		34,	TRUE},

	{ "ink",		35,	TRUE},
	{ "paint",		36,	TRUE},

	{ NULL,		0,	FALSE}
};

const struct flag_type liquid_special_flags[]=
{
		{ "vampire_blood",		LIQ_VAMPIRE,	TRUE},
		{ "werewolf_blood",		LIQ_WEREWOLF,	TRUE},
		{ "faerie_blood",		LIQ_FAERIE,	TRUE},
		{ "mage_blood",		LIQ_MAGE,	TRUE},
		{ "animal_blood",		LIQ_ANIMAL,	TRUE},
		{ "human_blood",		LIQ_HUMAN,	TRUE},
		{ "mutagen",		LIQ_MUTAGEN,	TRUE},
		{ NULL,		-1,	FALSE}
};

const struct material material_table [] =
{
	/*
	{ name,		colour,
	proof, full, thirst, food, ssize, agg, aggsoak, normsoak }
	*/

	{ "none",		"grey",
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,0,0,0,0,0 },

	{ "flesh",		"pink",
	FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,-2,1,5,40,3,0,0,0 },

	{ "wood",		"tan",
	FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,5,30,4,0,0,1 },

	{ "steel",		"steel",
	FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,100,7,0,1,2 },

	{ "poly-carbide steel",		"steel with bronze highlights",
	FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,120,7,0,2,3 },

	{ "iron",		"dark grey",
	FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,80,8,0,1,2 },

	{ "silver",		"silver",
	FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,130,8,0,0,1 },

	{ "gold",		"gold",
	FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,150,9,0,0,1 },

	{ "copper",		"copper",
	FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,80,7,0,1,1 },

	{ "rubber",		"black",
	FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,50,3,0,0,1 },

	{ "cotton",		"",
	FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, TRUE, FALSE,
	0,0,0,0,0,0,30,2,0,0,0 },

	{ "leather",	"brown",
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE,
	0,0,0,0,0,0,80,3,0,0,1 },

	{ "plastic",	"",
	FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,20,2,0,0,1 },

	{ "bread",		"fawn",
	FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,5,10,2,0,0,0 },

	{ "meat",		"brown",
	FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,8,30,4,0,0,0 },

	{ "egg",		"yellow",
	FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,5,10,1,0,0,0 },

	{ "vinyl",		"black",
	FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, TRUE, FALSE,
	0,0,0,0,0,0,20,2,0,0,0 },

	{ "wool",		"white",
	FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, TRUE, FALSE,
	0,0,0,0,0,0,20,2,0,0,1 },

	{ "glass",		"clear",
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE,
	0,0,0,0,0,0,30,3,0,0,1 },

	{ "platinum",	"silver",
	FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,180,9,0,0,1 },

	{ "silicon",	"white",
	FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, TRUE,
	0,0,0,0,0,0,50,3,0,1,1 },

	{ "silicon gel",	"clear",
	FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,60,3,0,0,0 },

	{ "granite",	"speckled grey",
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,50,8,0,1,1 },

	{ "diamond",	"clear",
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,150,7,0,1,1 },

	{ "ruby",		"red",
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,130,7,0,1,1 },

	{ "emerald",	"green",
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,160,7,0,1,1 },

	{ "sapphire",	"blue",
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,140,7,0,1,1 },

	{ "opal",		"swirled and speckled colours",
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,100,7,0,1,1 },

	{ "marble",		"white with coloured veins",
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,70,8,0,1,1 },

	{ "titanium",	"matt grey",
	FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,140,6,0,2,3 },

	{ "lead",		"dull, dark grey",
	FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,30,10,0,1,1 },

	{ "pewter",		"silver",
	FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,100,7,0,1,1 },

	{ "aluminium",	"silver",
	FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,40,4,0,1,1 },

	{ "paper",		"white",
	FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,10,1,10,1,0,0,0 },

	{ "cardboard",	"brown",
	FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,10,2,0,0,0 },

	{ "slime",		"green",
	FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE,
	0,0,0,0,0,0,0,2,0,0,0 },

	{ "concrete",	"light grey",
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,40,7,0,1,1 },

	{ "polyester",	"white",
	FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, TRUE, FALSE,
	0,0,0,0,0,0,40,2,0,0,0 },

	{ "compost",	"light grey",
	FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE,
	0,0,0,0,0,0,0,7,0,0,0 },

	{ "c-4",		"play-doh green",
	FALSE, FALSE, 3, FALSE, FALSE, TRUE, FALSE, FALSE,
	0,0,0,0,0,0,100,5,0,0,0 },

	{ "dynamite",	"dark grey",
	FALSE, FALSE, 2, FALSE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,70,6,0,0,0 },

	{ "rock",		"mottled",
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,0,7,0,1,1 },

	{ "basalt",		"dark grey",
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,10,7,0,1,1 },

	{ "obsidian",	"black",
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE,
	0,0,0,0,0,0,40,7,0,1,1 },

	{ "uranium",	"brassy amber",
	FALSE, FALSE, 50, FALSE, TRUE, TRUE, FALSE, FALSE,
	0,0,0,0,0,0,150,10,1,1,1 },

	{ "alloy",		"coppery",
	FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,80,8,0,1,2 },

	{ "cereal",		"tan and brown",
	FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,20,2,0,0,0 },

	{ "spandex",	"neon",
	FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, TRUE, FALSE,
	0,0,0,0,0,0,40,1,0,0,0 },

	{ "brick",		"red",
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE,
	0,0,0,0,0,0,40,7,0,1,1 },

	{ "fibreglass",		"white",
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,60,3,0,0,1 },

	{ "carbon fibre",	"black",
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,90,2,0,1,3 },

	{ "light",		"bright and sparkling",
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE,
	0,0,0,0,0,0,0,0,0,0,0 },

	{ "bitumen",	"black",
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,40,7,0,1,1 },

	{ "tile",		"orange",
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE,
	0,0,0,0,0,0,40,7,0,0,1 },

	{ "terracotta",	"orange",
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE,
	0,0,0,0,0,0,40,7,0,0,1 },

	{ "clay",		"reddish",
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,30,6,0,0,1 },

	{ "felt",		"brown",
	FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, TRUE, FALSE,
	0,0,0,0,0,0,50,3,0,0,0 },

	{ "plexi-glass",		"clear",
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE,
	0,0,0,0,0,0,30,7,0,1,3 },

	{ "slate",		"slate",
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE,
	0,0,0,0,0,0,40,7,0,0,1 },

	{ "corrugated iron","light, dull grey",
	FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,60,7,0,1,1 },

	{ "wire mesh",	"tarnished grey",
	FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,40,7,0,0,1 },

	{ "shingle",	"weathered brown",
	FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,10,5,0,0,1 },

	{ "lace",		"white",
	FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, TRUE, FALSE,
	0,0,0,0,0,0,60,2,0,0,0 },

	{ "satin",		"shiny white",
	FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, TRUE, FALSE,
	0,0,0,0,0,0,60,2,0,0,0 },

	{ "silk",		"white",
	FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, TRUE, FALSE,
	0,0,0,0,0,0,80,1,0,0,0 },

	{ "string",		"light brown",
	FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, TRUE, FALSE,
	0,0,0,0,0,0,20,4,0,0,0 },

	{ "rope",		"brown",
	FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,40,4,0,0,0 },

	{ "hemp",		"green",
	FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, TRUE, FALSE,
	0,0,0,0,0,0,140,4,0,0,0 },

	{ "cocaine",	"white",
	FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE,
	0,0,0,0,0,0,180,4,0,0,0 },

	{ "heroin",		"clear",
	FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE,
	0,0,0,0,0,0,180,4,0,0,0 },

	{ "flannelette",	"plaid",
	FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, TRUE, FALSE,
	0,0,0,0,0,0,30,3,0,0,0 },

	{ "coconut",	"white",
	FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,20,4,0,0,1 },

	{ "nuts",		"brown",
	FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,70,4,0,0,0 },

	{ "cheese",		"yellow",
	FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,40,4,0,0,0 },

	{ "butter",		"yellow",
	FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,40,4,0,0,0 },

	{ "curry",		"yellow",
	FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,50,3,0,0,0 },

	{ "garlic",		"off white",
	FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,50,3,0,0,0 },

	{ "berries",	"red and blue",
	FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,90,3,0,0,0 },

	{ "onion",		"white",
	FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,20,3,0,0,0 },

	{ "salt",		"white granulated",
	FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,10,4,0,0,0 },

	{ "sugar",		"white granulated",
	FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,10,4,0,0,0 },

	{ "fruit",		"brightly coloured",
	FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,60,4,0,0,0 },

	{ "pasta",		"tan",
	FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,30,4,0,0,0 },

	{ "mushroom",	"light grey and off white",
	FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,90,1,0,0,0 },

	{ "vegetables",	"brightly coloured",
	FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,60,4,0,0,0 },

	{ "seaweed",	"dark green",
	FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,100,3,0,0,0 },

	{ "tofu",		"ivory",
	FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,90,4,0,0,0 },

	{ "pastry",		"off white",
	FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,80,3,0,0,0 },

	{ "chocolate",	"rich, dark brown",
	FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,120,4,0,0,0 },

	{ "ceramic",	"white",
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE,
	0,0,0,0,0,0,120,4,0,0,0 },

/* @@@@@ */
	{ "water",		"clear",
	TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE,
	0,0,0,10,1,0,0,2,0,0,0 },

	{ "beer",		"amber",
	TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE,
	12,0,0,8,1,1,200,2,0,0,0 },

	{ "red wine",	"burgundy",
	TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE,
	30,0,0,8,1,1,400,2,0,0,0 },

	{ "ale",		"brown",
	TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE,
	15,0,0,8,1,1,250,2,0,0,0 },

	{ "dark ale",	"dark",
	TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE,
	16,0,0,8,1,1,300,2,0,0,0 },

	{ "whisky",		"golden",
	TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, TRUE,
	120,0,0,5,1,0,500,2,0,0,0 },

	{ "lemonade",	"pink",
	TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE,
	0,0,0,9,1,2,120,2,0,0,0 },

	{ "rotgut",		"boiling",
	TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, TRUE,
	190,0,0,4,0,0,400,2,0,0,0 },

	{ "specialty",	"clear",
	TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, TRUE,
	151,0,0,3,1,0,700,2,0,0,0 },

	{ "ooze",			"green",
	TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE,
	0,0,0,-8,2,1,0,2,0,0,0 },

	{ "milk",		"white",
	TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE,
	0,0,0,9,2,3,100,2,0,0,0 },

	{ "tea",		"black",
	TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE,
	0,0,0,8,1,0,150,2,0,0,0 },

	{ "coffee",		"black",
	TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE,
	0,0,0,8,1,0,150,2,0,0,0 },

	{ "blood",		"red",
	TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE,
	0,0,0,-10,2,2,0,2,0,0,0 },

	{ "salt water",	"clear",
	TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE,
	0,0,0,-2,1,0,0,2,0,0,0 },

	{ "coke",		"brown",
	TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE,
	0,0,0,9,2,2,120,2,0,0,0 },

	{ "cordial",	"clear",
	TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, TRUE,
	100,0,0,5,1,0,600,2,0,0,0 },

	{ "root beer",	"brown",
	TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE,
	0,0,0,9,2,2,120,2,0,0,0 },

	{ "white wine",	"golden",
	TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE,
	28,0,0,8,1,1,400,2,0,0,0 },

	{ "champagne",	"golden",
	TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE,
	32,0,0,8,1,1,700,2,0,0,0 },

	{ "mead",		"honey-coloured",
	TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE,
	34,0,0,8,2,2,400,2,0,0,0 },

	{ "rose wine",	"pink",
	TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE,
	26,0,0,8,1,1,600,2,0,0,0 },

	{ "benedictine",	"burgundy",
	TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE,
	40,0,0,8,1,1,600,2,0,0,0 },

	{ "vodka",		"clear",
	TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, TRUE,
	130,0,0,5,1,0,500,2,0,0,0 },

	{ "cranberry juice","red",
	TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE,
	0,0,0,9,1,2,120,2,0,0,0 },

	{ "orange juice",	"orange",
	TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE,
	0,0,0,9,2,3,120,2,0,0,0 },

	{ "absinthe",	"green",
	TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, TRUE,
	200,0,0,4,1,0,800,2,0,0,0 },

	{ "brandy",		"golden",
	TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE,
	80,0,0,5,1,0,800,2,0,0,0 },

	{ "aquavit",	"clear",
	TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, TRUE,
	140,0,0,5,1,0,600,2,0,0,0 },

	{ "schnapps",	"clear",
	TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE,
	90,0,0,5,1,0,400,2,0,0,0 },

	{ "icewine",	"purple",
	TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE,
	50,0,0,6,2,1,700,2,0,0,0 },

	{ "amontillado",	"burgundy",
	TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE,
	35,0,0,8,2,1,800,2,0,0,0 },

	{ "sherry",		"red",
	TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE,
	38,0,0,7,2,1,700,2,0,0,0 },

	{ "framboise",	"red",
	TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE,
	50,0,0,7,1,1,500,2,0,0,0 },

	{ "rum",		"amber",
	TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, TRUE,
	151,0,0,4,1,0,300,2,0,0,0 },

	{ "ink",		"black",
	TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE,
	0,0,0,0,1,0,0,2,0,0,0 },

	{ "paint",		"white",
	TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE,
	0,0,0,0,1,0,0,2,0,0,0 },

	{ "unknown",	"grey",
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,1,0,0,0 },

	{ NULL,		"grey",
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
	0,0,0,0,0,0,1,0,0,0 },
};

/***************************************************************************
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was written by Jason Dinkel and inspired by Russ Taylor,     *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 ***************************************************************************/
/*
 The code below uses a table lookup system that is based on suggestions
 from Russ Taylor.  There are many routines in handler.c that would benefit
 with the use of tables.  You may consider simplifying your code base by
 implementing a system like below with such functions. -Jason Dinkel
 */


struct flag_stat_type
{
	const struct flag_type *structure;
	bool stat;
};

/*****************************************************************************
 Name:		flag_stat_table
 Purpose:	This table catagorizes the tables following the lookup
		functions below into stats and flags.  Flags can be toggled
		but stats can only be assigned.  Update this table when a
		new set of flags is installed.
 ****************************************************************************/
const struct flag_stat_type flag_stat_table[] =
{
/*  {	structure		stat	}, */
	{	area_flags,		FALSE	},
	{   sex_flags,		TRUE	},
	{   exit_flags,		FALSE	},
	{   door_resets,		TRUE	},
	{   room_flags,		FALSE	},
	{   sector_flags,		TRUE	},
	{   type_flags,		TRUE	},
	{   extra_flags,		FALSE	},
	{   wear_flags,		FALSE	},
	{   act_flags,		FALSE	},
	{   act2_flags,		FALSE	},
	{   affect_flags,		FALSE	},
	{   apply_flags,		TRUE	},
	{   wear_loc_flags,		TRUE	},
	{   wear_loc_strings,	TRUE	},
	{   weapon_flags,		TRUE	},
	{   container_flags,	FALSE	},
	{   material_type,          TRUE    },
	{   form_flags,             FALSE   },
	{   part_flags,             FALSE   },
	{   size_flags,             TRUE    },
	{   position_flags,         TRUE    },
	{   off_flags,              FALSE   },
	{   imm_flags,              FALSE   },
	{   res_flags,              FALSE   },
	{   vuln_flags,             FALSE   },
	{   weapon_class,           TRUE    },
	{   weapon_type2,            FALSE   },
	{   0,			0	}
};



/*****************************************************************************
 Name:		is_stat( table )
 Purpose:	Returns TRUE if the table is a stat table and FALSE if flag.
 Called by:	flag_value and flag_string.
 Note:		This function is local and used only in bit.c.
 ****************************************************************************/
bool is_stat( const struct flag_type *flag_table )
{
	int flag;

	for (flag = 0; flag_stat_table[flag].structure; flag++)
	{
	if ( flag_stat_table[flag].structure == flag_table
	  && flag_stat_table[flag].stat )
		return TRUE;
	}
	return FALSE;
}



/*
 * This function is Russ Taylor's creation.  Thanks Russ!
 * All code copyright (C) Russ Taylor, permission to use and/or distribute
 * has NOT been granted.  Use only in this OLC package has been granted.
 */
/*****************************************************************************
 Name:		flag_lookup2( flag, table )
 Purpose:	Returns the value of a single, settable flag from the table.
 Called by:	flag_value and flag_string.
 Note:		This function is local and used only in bit.c.
 ****************************************************************************/
int flag_lookup2 (const char *name, const struct flag_type *flag_table)
{
	int flag;

	for (flag = 0; flag_table[flag].name != NULL; flag++)
	{
		if ( !str_prefix( name, flag_table[flag].name )
		  && flag_table[flag].settable )
			return flag_table[flag].bit;
	}

	return NO_FLAG;
}

/*****************************************************************************
 Name:		flag_value( table, flag )
 Purpose:	Returns the value of the flags entered.  Multi-flags accepted.
 Called by:	olc.c and olc_act.c.
 ****************************************************************************/
int flag_value( const struct flag_type *flag_table, char *argument)
{
	char word[MAX_INPUT_LENGTH]={'\0'};
	int  bit;
	int  marked = 0;
	bool found = FALSE;

	if ( is_stat( flag_table ) )
	{
	one_argument( argument, word );

	if ( ( bit = flag_lookup2( word, flag_table ) ) != NO_FLAG )
		return bit;
	else
		return NO_FLAG;
	}

	/*
	 * Accept multiple flags.
	 */
	for (; ;)
	{
		argument = one_argument( argument, word );

		if ( IS_NULLSTR(word) )
		break;

		if ( ( bit = flag_lookup2( word, flag_table ) ) != NO_FLAG )
		{
			SET_BIT( marked, bit );
			found = TRUE;
		}
	}

	if ( found )
	return marked;
	else
	return NO_FLAG;
}



/*****************************************************************************
 Name:		flag_string( table, flags/stat )
 Purpose:	Returns string with name(s) of the flags or stat entered.
 Called by:	act_olc.c, olc.c, and olc_save.c.
 ****************************************************************************/
char *flag_string( const struct flag_type *flag_table, int bits )
{
	static char buf[512]={'\0'};
	int  flag;

	buf[0] = '\0';

	for (flag = 0; flag_table[flag].name != NULL; flag++)
	{
	if ( !is_stat( flag_table ) && IS_SET(bits, flag_table[flag].bit) )
	{
		strncat( buf, " ", sizeof(buf) - strlen(buf) - 1 );
		strncat( buf, flag_table[flag].name, sizeof(buf) - strlen(buf) - 1 );
	}
	else
	if ( flag_table[flag].bit == bits )
	{
		strncat( buf, " ", sizeof(buf) - strlen(buf) - 1 );
		strncat( buf, flag_table[flag].name, sizeof(buf) - strlen(buf) - 1 );
		break;
	}
	}
	return (!IS_NULLSTR(buf)) ? buf+1 : (char *)"none";
}

/*****************************************************************************
 Name:		power_string( table, flags/stat )
 Purpose:	Returns string with name(s) of the powers or stat entered.
 Called by:	act_info.c.
 ****************************************************************************/
char *power_string( const struct gift_type *power_table, int bits, int level )
{
	static char buf[512]={'\0'};
	int  flag;

	buf[0] = '\0';

	for (flag = 0; power_table[flag].name != NULL; flag++)
	{
	if ( IS_SET(bits, power_table[flag].flag)
	&& power_table[flag].level == level )
	{
		strncat( buf, power_table[flag].name, sizeof(buf) - strlen(buf) - 1 );
		strncat( buf, ": ", sizeof(buf) - strlen(buf) - 1 );
		strncat( buf, power_table[flag].desc, sizeof(buf) - strlen(buf) - 1 );
		strncat( buf, "\n\r", sizeof(buf) - strlen(buf) - 1 );
	}
	}
	return (!IS_NULLSTR(buf)) ? buf : (char *)"none";
}

const int race_predisp_table[22][22] =
{
{ 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0 },
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5 }
};

const struct job_cmd_type mayor_cmd_table [] =
{
	{	"tax",		3,	60,	mayor_tax	},
	{	"appoint",	5,	30,	mayor_appoint	},
	{	"commands",	0,	0,	job_commands	},
	{	"advance",	0,	0,	job_advance	},
	{	NULL,		0,	0,	NULL		}
};

const struct job_cmd_type alder_cmd_table [] =
{
	{	"commands",	0,	0,	job_commands	},
	{	"advance",	0,	0,	job_advance	},
	{	NULL,		0,	0,	NULL		}
};

const struct job_cmd_type judge_cmd_table [] =
{
	{	"commands",	0,	0,	job_commands	},
	{	"advance",	0,	0,	job_advance	},
	{	"marry",	5,	20,	judge_marry	},
	{	NULL,		0,	0,	NULL		}
};

const struct job_cmd_type pchief_cmd_table [] =
{
	{	"commands",	0,	0,	job_commands	},
	{	"advance",	0,	0,	job_advance	},
	{	NULL,		0,	0,	NULL		}
};

const struct job_cmd_type no_job_cmd_table [] =
{
	{	"commands",	0,	0,	job_commands	},
	{	"advance",	0,	0,	job_advance	},
	{	"nohires",	3,	0,	job_nohire	},
	{	"apply",	3,	0,	job_apply	},
	{	NULL,		0,	0,	NULL		}
};

const struct job_cmd_type janitor_cmd_table [] =
{
	{	"clean",	3,	15,	janitor_clean	},
	{	"empty",	3,	50,	janitor_empty	},
	{	"commands",	0,	0,	job_commands	},
	{	"advance",	0,	0,	job_advance	},
	{	"nohires",	0,	0,	job_nohire	},
	{	"apply",	3,	0,	job_apply	},
	{	"resign",	3,	0,	job_quit	},
	{	NULL,		0,	0,	NULL		}
};

const struct job_cmd_type sales_cmd_table [] =
{
	{	"buy",		2,	5,	sales_buy	},
	{	"sell",		2,	5,	sales_sell	},
	{	"markup",	2,	10,	sales_markup	},
	{	"commands",	0,	0,	job_commands	},
	{	"advance",	0,	0,	job_advance	},
	{	"nohires",	0,	0,	job_nohire	},
	{	"apply",	3,	0,	job_apply	},
	{	"resign",	3,	0,	job_quit	},
	{	NULL,		0,	0,	NULL		}
};

const struct job_cmd_type maker_cmd_table [] =
{
	{	"make",		8,	40,	maker_creation	},
	{	"desc",		2,	5,	maker_desc	},
	{	"long",		1,	5,	maker_long_desc	},
	{	"name",		1,	5,	maker_name	},
	{	"short",	1,	5,	maker_short_desc},
	{	"sell",		2,	5,	maker_sell	},
	{	"type",		2,	5,	maker_type	},
	{	"materiallist",	0,	0,	maker_materials	},
	{	"wearlist",	0,	0,	maker_wear	},
	{	"sizelist",	0,	0,	maker_size	},
	{	"typelist",	0,	0,	maker_typehelp	},
	{	"commands",	0,	0,	job_commands	},
	{	"advance",	0,	0,	job_advance	},
	{	"nohires",	0,	0,	job_nohire	},
	{	"apply",	3,	0,	job_apply	},
	{	"resign",	3,	0,	job_quit	},
	{	NULL,		0,	0,	NULL		}
};

const struct job_cmd_type chef_cmd_table [] =
{
	{	"cook",		8,	40,	maker_creation	},
	{	"desc",		2,	5,	maker_desc	},
	{	"string",	1,	5,	maker_long_desc	},
	{	"name",		1,	5,	maker_short_desc},
	{	"sell",		2,	5,	maker_sell	},
	{	"materiallist",	0,	0,	maker_materials	},
	{	"wearlist",	0,	0,	maker_wear	},
	{	"sizelist",	0,	0,	maker_size	},
	{	"typelist",	0,	0,	maker_typehelp	},
	{	"commands",	0,	0,	job_commands	},
	{	"advance",	0,	0,	job_advance	},
	{	"nohires",	0,	0,	job_nohire	},
	{	"apply",	3,	0,	job_apply	},
	{	"resign",	3,	0,	job_quit	},
	{	NULL,		0,	0,	NULL		}
};

const struct job_cmd_type police_cmd_table [] =
{
	{	"arrest",	2,	15,	police_arrest		},
	{	"breathalise",	3,	10,	police_breathalise	},
	{	"background",	3,	10,	police_background	},
	{	"commands",	0,	0,	job_commands		},
	{	"advance",	0,	0,	job_advance		},
	{	"nohires",	0,	0,	job_nohire		},
	{	"apply",	3,	0,	job_apply		},
	{	"resign",	3,	0,	job_quit		},
	{	NULL,		0,	0,	NULL			}
};

const struct job_cmd_type crim_cmd_table [] =
{
	{	"pick",		3,	5,	crim_pick	},
	{	"whack",	1,	5,	crim_whack	},
	{	"hotwire",	3,	10,	crim_hotwire	},
	{	"stickup",	3,	10,	crim_stickup	},
	{	"case",		3,	5,	crim_case	},
	{	"commands",	0,	0,	job_commands	},
	{	"advance",	0,	0,	job_advance	},
	{	"nohires",	0,	0,	job_nohire	},
	{	"apply",	3,	0,	job_apply	},
	{	"resign",	3,	0,	job_quit	},
	{	NULL,		0,	0,	NULL		}
};

const struct job_cmd_type music_cmd_table [] =
{
	{	"jam",		0,	20,	muso_jam	},
	{	"sing",		0,	2,	muso_sing	},
	{	"play",		0,	2,	muso_play	},
	{	"commands",	0,	0,	job_commands	},
	{	"advance",	0,	0,	job_advance	},
	{	"nohires",	0,	0,	job_nohire	},
	{	"apply",	3,	0,	job_apply	},
	{	"resign",	3,	0,	job_quit	},
	{	NULL,		0,	0,	NULL		}
};

const struct job_cmd_type reprt_cmd_table [] =
{
	{	"subject",	0,	0,	reporter_subject	},
	{	"category",	0,	0,	reporter_category	},
	{	"body",		0,	0,	reporter_body		},
	{	"post",		5,	70,	reporter_post		},
	{	"commands",	0,	0,	job_commands		},
	{	"advance",	0,	0,	job_advance		},
	{	"nohires",	0,	0,	job_nohire		},
	{	"apply",	3,	0,	job_apply		},
	{	"resign",	3,	0,	job_quit		},
	{	NULL,		0,	0,	NULL			}
};

const struct office_type office_table [] =
{
	{	'M',	"Mayor",	mayor_cmd_table,	3	 },
	{	'A',	"Alderman",	alder_cmd_table,	2	 },
	{	'J',	"Judge",	judge_cmd_table,	2	 },
	{	'C',	"Chief",	pchief_cmd_table,	2	 },
	{	' ',	NULL,		NULL,	 		0	 }
};

const struct  job_type job_table [] =
{
	{	"Mayor",	-1,	mayor_cmd_table,  STAT_CHA,PERFORMANCE, 5 },
	{	"Alderman",	-1,	alder_cmd_table,  STAT_MAN,POLITICS,    5 },
	{	"Judge",	-1,	judge_cmd_table,  STAT_PER,LAW,		5 },
	{	"Police_Chief",	-1,	pchief_cmd_table, STAT_WIT,INVESTIGATION,7},
	{	"None",		-1,	no_job_cmd_table,  -1,	-1,		99},
	{	"Janitor",	0,	janitor_cmd_table,STAT_STR,STREETWISE,	5 },
	{	"Salesman",	0,	sales_cmd_table,  STAT_CHA,EMPATHY,	5 },
	{	"Maker",	0,	maker_cmd_table,  STAT_INT,REPAIR,	2 },
	{	"Chef",		0,	chef_cmd_table,   STAT_INT,SURVIVAL,	2 },
	{	"Constable",	0,	police_cmd_table, STAT_WIT,LAW,		4 },
	{	"Criminal",	0,	crim_cmd_table,   STAT_DEX,LARCENY,	5 },
	{	"Reporter",	0,	reprt_cmd_table,  STAT_MAN,EXPRESSION,	1 },
	{	"Musician",	0,	music_cmd_table,  STAT_CHA,PERFORMANCE,	7 },
	{	NULL,		0,	NULL,		   -1,	-1,		99}
};


const struct flag_type card_suit [] =
{
	{ "diamonds",	 141,  TRUE  },
	{ "clubs",		 142,  TRUE  },
	{ "hearts",		 143,  TRUE  },
	{ "spades",		 144,  TRUE  },
	{ NULL,		 -1, FALSE }
};

const struct flag_type card_values [] =
{
	{ "ace",		 1,  TRUE  },
	{ "two",		 2,  TRUE  },
	{ "three",		 3,  TRUE  },
	{ "four",		 4,  TRUE  },
	{ "five",		 5,  TRUE  },
	{ "six",		 6,  TRUE  },
	{ "seven",		 7,  TRUE  },
	{ "eight",		 8,  TRUE  },
	{ "nine",		 9,  TRUE  },
	{ "ten",		 10, TRUE  },
	{ "jack",		 11, TRUE  },
	{ "queen",		 12, TRUE  },
	{ "king",		 13, TRUE  },
	{ "joker",		 14, TRUE  },
	{ NULL,		 -1, FALSE }
};

const struct flag_type card_face_char [] =
{
	{ " A",		 1,  TRUE  },
	{ " 2",		 2,  TRUE  },
	{ " 3",		 3,  TRUE  },
	{ " 4",		 4,  TRUE  },
	{ " 5",		 5,  TRUE  },
	{ " 6",		 6,  TRUE  },
	{ " 7",		 7,  TRUE  },
	{ " 8",		 8,  TRUE  },
	{ " 9",		 9,  TRUE  },
	{ "10",		 10, TRUE  },
	{ " J",		 11, TRUE  },
	{ " Q",		 12, TRUE  },
	{ " K",		 13, TRUE  },
	{ "JK",		 14, TRUE  },
	{ NULL,		 -1, FALSE }
};

const struct breed_type breed_table [] =
{
	{	"Homid",	0,	1	},
	{	"Metis",	1,	2	},
	{	"Lupus",	2,	3	},
	{	NULL,		-1,	-1	}
};

const struct auspice_type auspice_table [] =
{
	{	"Ragabash",	0,	1	},
	{	"Theurge",	1,	1	},
	{	"Philodox",	2,	2	},
	{	"Galliard",	3,	2	},
	{	"Ahroun",	4,	3	},
	{	NULL,		-1,	-1	}
};

const char *hallucination_table [MAX_HALLUC] =
{
	"a snarling beast",
	"a creature that seems to be all fangs",
	"a pink Christina Aguilera monster",
	"an evil looking couch",
	"a swarm of bugs",
	"a shoehorn with teeth",
	"a lunatic with a chainsaw",
	"Caine",
	"a forty-foot humming bird",
	"your great aunt hilda",
	"Jackie Chan",
	"Mr. Ed",
	"a squirrel",
	"the Queen",
	"Wonder Woman"
};

const struct colcode_type colcode_table [MAX_COLOURS] =
{
	{ 'x', "cancel",		"{x" },
	{ 'r', "red",		"{r" },
	{ 'g', "green",		"{g" },
	{ 'y', "yellow",		"{y" },
	{ 'b', "blue",		"{b" },
	{ 'm', "magenta",		"{m" },
	{ 'c', "cyan",		"{c" },
	{ 'W', "white",		"{W" },
	{ 'D', "grey",		"{D" },
	{ 'R', "bright red",	"{R" },
	{ 'G', "bright green",	"{G" },
	{ 'Y', "bright yellow",	"{Y" },
	{ 'B', "bright blue",	"{B" },
	{ 'M', "bright magenta",	"{M" },
	{ 'C', "bright cyan",	"{C" },
	{ 'w', "bright white",	"{w" },
};

const struct sire_type sire_table [] =
{
		/* Nosferatu Sires */
	{ 12, "nosferatu", "Job Crockett", "male",
	"Something of a loner, Job calls a Chicago warehouse home, living there with his 'gator companions." },
	{ 12, "nosferatu", "Tanya Carmichael", "female",
	  "Tanya is one of the city's more elusive residents. She has been known to appear at the more major functions and gatherings." },
	{ 11, "nosferatu", "Lawrence Smith", "male",
	"Lawrence started life as a local boy, but moved to South America suddenly some time ago under questionable circumstances." },
	{ 11, "nosferatu", "Ava Porter", "female",
	"Ava was a quiet and shy Nosferatu.  She was known to have criminal connections." },
	{ 11, "nosferatu", "Mikaela Fallow", "female",
	"Mikaela is believed to have gone to the final death following the 'shootout' in the Blue Room." },
	{ 10, "nosferatu", "Ivan Rothchild", "male",
	"Ivan was a serial killer in life, being labelled 'Ivan the Terrible' by the news media at the time. His unlife has been decidedly quiet however." },
	{ 10, "nosferatu", "Hairy Pete", "male",
	"Hairy Pete is a surprisingly well-informed, though not terribly bright, thug. He has been known to frequent dance clubs periodically." },
	{ 10, "nosferatu", "Simon Taylor", "male",
	"Simor Taylor was the sire of Ava Porter.  He was a well-connected Nosferatu who moved up and down the east coast." },
	{ 9, "nosferatu", "Mickey 'The Fink'", "male",
	"A former New York resident, Mickey learnt how to deal in knowledge long before his embrace, often working as an information source for both the cops and the mob." },
	{ 9, "nosferatu", "Henry Cabal", "male",
	"Henry was a notorious black market trader, until the 'foreshore' deal, following the shootout in the Blue Room. He hasn't been seen since." },
	{ 8, "nosferatu", "Alberich ", "male", "Progeny of Jacob Durgal."},
	{ 7, "nosferatu", "Black Simon", "male",
	"Far older than most of the Bal Harbour's kindred population, the ex-Nosferatu primogen appears quiet and reticent, but remains a strong force even after retiring to other parts." },

	/* Toreador Sires */
	{ 12, "toreador",  "Alicia Rail", "female",
	"Alicia owns one of the most renowned Art Galleries in New York, though rarely leaves it, preferring to entertain her aquaintinces there." },
	{ 12, "toreador",  "Portia Krelborne", "female",
	"Portia can be found at any major gathering, and manages a Fashion Magazine in Paris." },
	{ 12, "toreador",  "Andrew de Gabroni", "male",
	"Though usually found at his Vinyards, Andrew frequents New York City and Paris for many business interests." },
	{ 12, "toreador",  "Oliver Nation", "male",
	"Oliver lives in Vienna, where he produces an amazing amount of paintings under various monikers." },
	{ 12, "toreador",  "Marcus Carrigan", "male",
	"Seeming of no fixed residance, Marcus can be found anywhere with a major Opera, and has connections all over the world." },
	{ 12, "toreador",  "Natalie Watt", "female",
	"A Curator at New York's Metropolitan Museum, Natalie is well respected in the Antiquities circles." },
	{ 12, "toreador",  "DJ Hudda-Hooddia", "male",
	"This Miami resident is well known for large, loud parties in his hometown, but is generally considered shallow by his peers." },
	{ 12, "toreador",  "Julio Rodriguez", "male",
	"Resident of Manhattan, has not been seen since the World Trade Centre bombing." },

	{ 12, "ventrue",   "Edward Michaelson", "male",
	"Making his home as a lecturer of Modern History at Cambridge University, Edward Never misses any of the Grand Council Meetings in London." },
	{ 12, "ventrue",   "Anastasia Ivanov", "female",
	"" },
	{ 11, "ventrue",   "Cleo Dartmoor", "female",
	"" },
	{ 11, "ventrue",   "Jack Elrich", "male",
	"" },
	{ 10, "ventrue",   "Jason Stryker", "male",
	"Absent from his Chicago home, Stryker is conniving and largely unwelcome in the areas in which he has previously resided." },
	{ 9, "ventrue",   "Derek Hastings", "male",
	"" },
	{ 9,  "ventrue",   "Xavier deMarco", "male",
	"A clever businessman and likeable sort, deMarco stands as being on of Chicago's rising stars." },
	{ 8, "ventrue",   "Anna Thompson", "female",
	"" },

	{ 12, "brujah",    "Axel Knacker", "male",
	"" },
	{ 12, "brujah",    "Maria", "female",
	"" },
	{ 11, "brujah",    "Melissa Eykman", "female",
	"" },
	{ 11, "brujah",    "Yuri Delotnikov", "male",
	"" },
	{ 10, "brujah",    "Igor Khenston", "male",
	"" },
	{ 10, "brujah",    "Isis", "female",
	"" },
	{ 9, "brujah",    "Stephanie Bruchold", "female",
	"" },
	{ 9,  "brujah",    "Leonie", "female",
	"Leonie has played the part of assassin for hire for the last half century, often unwittingly winding up involved in the grand schemes of elder vampires. She has been a regular resident of Chicago for some time, usually associating with the anarchs of Gary." },
	{ 8, "brujah",    "Fast Eddie", "male",
	"" },

	{ 12, "malkavian", "Zeb", "male",
	"" },
	{ 11, "malkavian", "Jeb", "male",
	"" },
	{ 12, "malkavian", "Hezekiah", "male",
	"" },
	{ 11, "malkavian", "Helen Karyoh", "female",
	"" },
	{ 10, "malkavian", "Sophia", "female",
	"" },
	{ 10, "malkavian", "Daedalus", "male",
	"" },
	{ 9, "malkavian", "Bridge Boy", "male",
	"Bridge Boy acquired his name from his habitual ensconcement beneath a city bridge. He tends to be wild-eyed and frantically gibbers about bridge tolls and billy goats when under stress." },
	{ 9, "malkavian", "Gabrielle", "female",
	"" },
	{ 8, "malkavian", "Ernest Serime", "male",
	"'The Doctor' rarely goes by his real name, constantly swapping aliases in an attempt to hide from his enemies both real an imagined. Currently residing in Milwaukee, this former member of the White Masque is, quite simply, stark, raving mad." },

	{ 0,  NULL, NULL, NULL, NULL }
};

const struct flag_type personality_archetypes [] =
{
	{ "Addict",		 0,  TRUE  },
	{ "Architect",	 1,  TRUE  },
	{ "Autocrat",	 2,  TRUE  },
	{ "Avant-garde", 3,  TRUE  },
	{ "Bon vivant",	 4,  TRUE  },
	{ "Bravo",		 5,  TRUE  },
	{ "Bureaucrat",	 6,  TRUE  },
	{ "Caregiver",	 7,  TRUE  },
	{ "Celebrant",	 8,  TRUE  },
	{ "Child",		 9,  TRUE  },
	{ "Confidant",	 10,  TRUE  },
	{ "Conformist",	 11,  TRUE  },
	{ "Conniver",	 12,  TRUE  },
	{ "Critic",		 13,  TRUE  },
	{ "Curmudgeon",	 14,  TRUE  },
	{ "Director",	 15,  TRUE  },
	{ "Dreamer",	 16,  TRUE  },
	{ "Explorer",	 17,  TRUE  },
	{ "Fanatic",	 18,  TRUE  },
	{ "Follower",	 19,  TRUE  },
	{ "Gallant",	 20,  TRUE  },
	{ "Gambler",	 21,  TRUE  },
	{ "Jester",		 22,  TRUE  },
	{ "Judge",		 23,  TRUE  },
	{ "Leader",		 24,  TRUE  },
	{ "Martyr",		 25,  TRUE  },
	{ "Mediator",	 26,  TRUE  },
	{ "Pedagogue",	 27,  TRUE  },
	{ "Penitent",	 28,  TRUE  },
	{ "Perfectionist",	 29,  TRUE  },
	{ "Poltroon",	 30,  TRUE  },
	{ "Rebel",		 31,  TRUE  },
	{ "Rogue",		 32,  TRUE  },
	{ "Scientist",	 33,  TRUE  },
	{ "Survivor",	 34,  TRUE  },
	{ "Traditionalist",	 35,  TRUE  },
	{ "Trickster",	 36,  TRUE  },
	{ "Visionary",	 37,  TRUE  },
	{ NULL,		 -1, FALSE }
};

const struct flag_type staff_status [] =
{
	{ "NEW PLAYER",		0,  FALSE	},
	{ "Player",			1,  TRUE	},
	{ "Staff",			2,  TRUE	}, /* Formerly Watcher */
	{ "Storyteller",	3,  TRUE	},
	{ "Administrator",	4,  TRUE	}, /* Formerly Master  */
	{ "Implementor",	5,  TRUE	},
	{ NULL,		 -1,  FALSE	}
};

/*
 * As yet some backgrounds remain un-implemented.
 * Implemented:
 * Generation, resources, contacts, herd, statuses (in as much as they exist),
 * fame
 */

 /**
  * Background Tables
  * Background Name | Background | Settable
  */
const struct flag_type background_table [] =
{
	{ "Allies",         ALLIES,         TRUE    },
	{ "Breeding",       PUREBREED,      FALSE   },
	{ "Contacts",       CONTACTS,       TRUE    },
	{ "Fetishes",       FETISHES,       FALSE   },
	{ "Generation",     GENERATION,     FALSE   },
	{ "Herd",           HERD,           TRUE    },
	{ "Mentor",         MENTOR,         TRUE    },
	{ "Pastlife",       PASTLIFE,       FALSE   },
	{ "Resources",		RESOURCES,		TRUE	},
	{ "Retainers",		RETAINERS,		TRUE	},
	{ "Clan",           CLAN_STATUS,    TRUE    },
	{ "Fame",           FAME_STATUS,    TRUE    },
	{ "Race",           RACE_STATUS,    TRUE    },
	{ NULL,				0,				TRUE	}
};

/**
 * Influence Table
 * Influence Name | Influence | Settable
 */
const struct flag_type influence_table [] =
{
	{ "Church",		INFL_CHURCH,	TRUE	},
	{ "Criminal",	INFL_CRIMINAL,	TRUE	},
	{ "Economic",	INFL_ECONOMIC,	TRUE	},
	{ "Judicial",	INFL_JUDICIAL,	TRUE	},
	{ "Media",		INFL_MEDIA,		TRUE	},
	{ "Police",		INFL_POLICE,	TRUE	},
	{ "Political",	INFL_POLITICAL,	TRUE	},
	{ "Scientific",	INFL_SCIENTIFIC,TRUE	},
	{ NULL,		0,		TRUE	}
};


/***********************************************
* Part One: Name of Influence Command          *
* Part Two: Type of Influence (-1 for all)     *
* Part Three: Level of Influence required      *
* Part Four: Delay                             *
* Part Five: Function executed                 *
************************************************/
const struct influence_cmd_type influence_cmd_table [] =
{
  { "commands", 	-1,					0, 0,	influence_commands	},
  { "advance",  	-1,					0, 0,	influence_advance	},
  /* BREAK */
  { "collection",	INFL_CHURCH,		1, 5,	church_collection	},
  { "research",  	INFL_CHURCH,		2, 10,	church_research		},
  { "tipoff",		INFL_CHURCH,		3, 5,	church_tipoff		},
  { "findrelic",	INFL_CHURCH,		5, 5,	church_findrelic	},
/* BREAK */
  { "racketeer",	INFL_CRIMINAL,		1, 5,	criminal_racket 	},
  { "scout",		INFL_CRIMINAL,		3, 5,	criminal_scout	 	},
/*{ "gang",			INFL_CRIMINAL,		5, 10,	criminal_gang		}, */
  /* BREAK */
/*{ "investment",	INFL_ECONOMIC,		1, 5,	economic_investment	}, */
  { "raisecapital",	INFL_ECONOMIC,		1, 5,	economic_raise		},
  { "trade",		INFL_ECONOMIC,		2, 5,	economic_trade		},
  { "market",		INFL_ECONOMIC,		4, 5,	economic_market		},
  /* BREAK */
  { "sentence", 	INFL_JUDICIAL,		1, 5,	judicial_sentence	},
  { "pardon",		INFL_JUDICIAL,		4, 5,	judicial_pardon		},
  /* BREAK */
  { "articles", 	INFL_MEDIA,			1, 5,	media_articles		},
  { "suppress", 	INFL_MEDIA,			2, 5,	media_suppress		},
  { "promote",  	INFL_MEDIA,			3, 5,	media_promote		},
  /* BREAK */
  { "warrant",  	INFL_POLICE,		1, 5,	police_warrant		},
  { "apb",			INFL_POLICE,		4, 5,	police_apb			},
/*{ "squad",		INFL_POLICE,		5, 10,	police_squad		}, */
  /* BREAK */
  { "campaign", 	INFL_POLITICAL,		1, 10,	political_campaign	},
  { "raisefunds",	INFL_POLITICAL,		2, 10,	political_raise		},
  { "smear",		INFL_POLITICAL,		4, 10,	political_negcampaign	},
  /* BREAK */
/*{ "study",		INFL_SCIENTIFIC,	1, 10,	scientific_study	},*/
  { "document",		INFL_SCIENTIFIC,	1, 5,	scientific_tipoff	},
  { "materials",	INFL_SCIENTIFIC,	2, 5,	scientific_materials	},
  { NULL,	  -1,		  0, 0, NULL }
};

const struct influence_cmd_type bg_cmd_table [] =
{
	{ "herd",	  HERD,		  1, 3,  background_herd	},
	{ "commands", -1,		  0, 0,  background_commands	},
	{ "advance",  -1,		  0, 0,  background_advance	},
	{ NULL,	  -1,		  0, 0,  NULL }
};


const struct news_cmd_type news_cmd_table [] =
{
	{ "commands",	0,	newspaper_commands	},
	{ "list",		0,	newspaper_list		},
	{ "article",	0,	newspaper_articles	},
	{ "release",	0,	newspaper_release	},
	{ "rename",		0,	newspaper_rename	},
	{ "price",		0,	newspaper_price		},
	{ "stop",		0,	newspaper_stop		},
	{ "show",		0,	newspaper_show		},
	{ "clear",		0,	newspaper_clear		},
	{ "place",		0,	newspaper_place		},
	{ "create",		0,	newspaper_new		},
	{ "delete",		0,	newspaper_delete	},
	{ "save",		0,	newspaper_save		},
	{ NULL,		0,	NULL			}
};

const struct flag_type rp_area_table [] =
{
	{ "City Center",		10000,	TRUE	},
	{ "Newbie School",		8280,	TRUE	},
	{ "Mall Entrance",		10050,	TRUE	},
	{ NULL,					0,		FALSE	}
};

const struct flag_type taxi_area_table [] =
{
	{ "The Tenements",		5001,	TRUE	},
	{ "Nightlight",			5501,	TRUE	},
	{ "Pawn Shop",			6003,	TRUE	},
	{ "Blue Door",			7001,	TRUE	},
	{ "Haven",				8001,	TRUE	},
	{ "Newbie School",		8280,	TRUE	},
	{ "Zoo",				9000,	TRUE	},
	{ "City Center",		10000,	TRUE	},
	{ "Mall Entrance",		10050,	TRUE	},
	{ "Police Station",		10047,	TRUE	},
	{ "Rodon",				12201,	TRUE	},
	{ "FBI Building",		12501,	TRUE	},
	{ "Bank of America",	10800,	TRUE	},
	{ NULL,					0,		FALSE	}
};

const struct flag_type concede_flags [] =
{
	{ "death",		CEED_DIE,		FALSE	},
	{ "embrace",	CEED_EMBRACE,	FALSE	},
	{ "feed",		CEED_FEED,		FALSE	},
	{ "ghoul",		CEED_GHOUL,		FALSE	},
	{ "arrest",		CEED_ARREST,	FALSE	},
	{ NULL,			0,				FALSE	}
};

const struct flag_type insult_table [] =
{
	{ "you curse on humanity!",									0,	TRUE	},
	{ "you spawn of satan!",									0,	TRUE	},
	{ "you evil filth!",										0,	TRUE	},
	{ "you putrid scum!",										0,	TRUE	},
	{ "I'm going to rip you a new one!",						0,	TRUE	},
	{ "you aren't walking away from here.",						0,	TRUE	},
	{ "I'm going to take your fangs as a trophy.",				0,	TRUE	},
	{ "you aren't going to hurt anyone ever again.",			0,	TRUE	},
	{ "you won't darken the streets ever again.",				0,	TRUE	},
	{ "I'm going to make sure you don't hurt anyone again.",	0,	TRUE	},
	{ NULL,							0,	FALSE	}
};

const struct smarket_cmd_type smarket_cmd_table [] =
{
	{ "commands",	0,	smarket_commands	},
	{ "list",		0,	smarket_list		},
	{ "show",		0,	smarket_show		},
	{ "create",		0,	smarket_create		},
	{ "delete",		0,	smarket_delete		},
	{ "save",		0,	smarket_save		},
	{ "price",		0,	smarket_price		},
	{ "rename",		0,	smarket_rename		},
	{ "ticker",		0,	smarket_ticker		},
	{ NULL,		0,	NULL			}
};

const struct flag_type spirit_table [] =
{
	{	"None",		0,		FALSE	},
	{	"War",		A,		FALSE	},
	{	"Pain",		B,		FALSE	},
	{	"Lune",		C,		TRUE	},
	{	"Falcon",	D,		TRUE	},
	{	"Thunder",	E,		TRUE	},
	{	"Pegasus",	F,		TRUE	},
	{	"Stag",		G,		TRUE	},
	{	"Bear",		H,		TRUE	},
	{	"Boar",		I,		TRUE	},
	{	"Fenris",	J,		TRUE	},
	{	"Griffin",	K,		TRUE	},
	{	"Rat",		L,		TRUE	},
	{	"Wendigo",	M,		TRUE	},
	{	"Chimera",	N,		TRUE	},
	{	"Owl",		O,		TRUE	},
	{	"Cockroach",	P,		TRUE	},
	{	"Raven",	Q,		TRUE	},
	{	"Uktena",	R,		TRUE	},
	{	"Unicorn",	S,		TRUE	},
	{	"Coyote",	T,		TRUE	},
	{	"Cuckoo",	U,		TRUE	},
	{	"Fox",		V,		TRUE	},
	{	"Animal",	W,		FALSE	},
	{	"Gladechild",	X,		FALSE	},
	{	"Wild hunt",	Y,		FALSE	},
	{	"Ancestor",	Z,		FALSE	},
	{	"Earth",	aa,		FALSE	},
	{	"Air",		bb,		FALSE	},
	{	"Fire",		cc,		FALSE	},
	{	"Water",	dd,		FALSE	},
	{	"Epiphling",	ee,		FALSE	},
	{	NULL,		0,		FALSE	}
};

const struct spirit_power_type spirit_power_table [] =
{
	{	"None",		0,	0, NULL },
	{	"War",		A,	B, spirit_war },
	{	"Pain",		B,	B, spirit_pain },
	{	"Lune",		C,	C, spirit_lune },
	{	"Falcon",	D,	A, spirit_falcon },
	{	"Thunder",	E,	A, spirit_thunder },
/*    {	"Pegasus",	F,	A, spirit_pegasus },
	{	"Stag",		G,	A, spirit_stag },
	{	"Bear",		H,	B, spirit_bear },
	{	"Boar",		I,	B, spirit_boar },
	{	"Fenris",	J,	B, spirit_fenris },
	{	"Griffin",	K,	B, spirit_griffin },
	{	"Rat",		L,	B, spirit_rat },
	{	"Wendigo",	M,	B, spirit_wendigo },
	{	"Chimera",	N,	C, spirit_chimera },
	{	"Owl",		O,	C, spirit_owl },
	{	"Cockroach",	P,	C, spirit_cockroach },
	{	"Raven",	Q,	C, spirit_raven },
	{	"Uktena",	R,	C, spirit_uktena },
	{	"Unicorn",	S,	C, spirit_unicorn },
	{	"Coyote",	T,	D, spirit_coyote },
	{	"Cuckoo",	U,	D, spirit_cuckoo },
	{	"Fox",		V,	D, spirit_fox },
	{	"Animal",	W,	E, spirit_animal },
	{	"Gladechild",	X,	E, spirit_glade },
	{	"Wild hunt",	Y,	E, spirit_hunt },
	{	"Ancestor",	Z,	E, spirit_ancestor },
	{	"Earth",	aa,	E, spirit_earth },
	{	"Air",		bb,	E, spirit_air },
	{	"Fire",		cc,	E, spirit_fire },
	{	"Water",	dd,	E, spirit_water },
	{	"Epiphling",	ee,	E, spirit_epiphling },
*/    {	NULL,		0,	0, NULL }
};

const struct totem_power_type totem_table [] =
{
	{	"None",			0,
	{"", "", "", "", ""} },
	{	"Cockroach",		A,
	{"", "", "", "", ""} },
	{	"Rat",			B,
	{"", "", "", "", ""} },
	{	"Falcon",		C,
	{"", "", "", "", ""} },
	{	"Ahroun",		D,
	{"", "", "", "", ""} },
	{	"Theurge",		E,
	{"", "", "", "", ""} },
	{	"Galliard",		F,
	{"", "", "", "", ""} },
	{	"Ragabash",		G,
	{"", "", "", "", ""} },
	{	"Philodox",		H,
	{"", "", "", "", ""} },
	{	"Homid",		I,
	{"", "", "", "", ""} },
	{	"Metis",		J,
	{"", "", "", "", ""} },
	{	"Lupus",		K,
	{"", "", "", "", ""} },
	{	"Fenris",		L,
	{"", "", "", "", ""} },
	{	"Unicorn",		M,
	{"", "", "", "", ""} },
	{	"Owl",			N,
	{"", "", "", "", ""} },
	{	"Chimera",		O,
	{"", "", "", "", ""} },
	{	"Wyrm",			P,
	{"", "", "", "", ""} },
	{	"Thunder",		Q,
	{"", "", "", "", ""} },
	{	"Pegasus",		R,
	{"", "", "", "", ""} },
	{	"Stag",			S,
	{"", "", "", "", ""} },
	{	"Bear",			T,
	{"", "", "", "", ""} },
	{	"Boar",			U,
	{"", "", "", "", ""} },
	{	"Griffin",		V,
	{"", "", "", "", ""} },
	{	"Wendigo",		W,
	{"", "", "", "", ""} },
	{	"Raven",		X,
	{"", "", "", "", ""} },
	{	"Uktena",		Y,
	{"", "", "", "", ""} },
	{	"Coyote",		Z,
	{"", "", "", "", ""} },
	{	"Cuckoo",		aa,
	{"", "", "", "", ""} },
	{	"Fox",			bb,
	{"", "", "", "", ""} },
	{	NULL,		0,
	{"", "", "", "", ""} }
};

const struct flag_type	totem_attitudes [] =
{
	{ "hatred",				0,	TRUE	},
	{ "very unfavourable",	1,	TRUE	},
	{ "unfavourable",		2,	TRUE	},
	{ "favourable",			3,	TRUE	},
	{ "very favourable",	4,	TRUE	},
	{ "love",				5,	TRUE	},
	{ NULL,			-1,	FALSE	}
};

const struct gift_type	gift_table [] =
{
	{ "Cooking", "Making the inedible, edible.",
	/*Done*/
	A,		1,	},
	{ "Sense Wyrm",
	  "(SENSEWYRM) Detecting the taint of the Wyrm.",
	/*Done*/
	B,		1,	},
	{ "Heightened Senses",
	  "(HEIGHTEN) Expand your senses to incredible proportions.",
	/*Done*/
	C,		1,	},
	{ "Smell of Man",
	  "(MANSMELL) Scare animals with the scent of humanity.",
	/*Done*/
	D,		1,	},
	{ "Scent of Running Water",
	  "(auto) Making yourself untrackable.",
	E,		1,	},
	{ "Resist Pain",
	  "(PAINRESIST) Pain does not slow you down.",
	/*Done*/
	F,		1,	},
	{ "Beast Speech",
	  "(auto) Understand animal speech.",
	G,		1,	},
	{ "Falling Touch",
	  "(FALLTOUCH) Knock an opponent down with a touch.",
	H,		1,	},
	{ "Persuasion",
	  "Improve your ability to convince.",
	/*Done*/
	I,		1,	},
	{ "Shroud",
	  "Plunge the room into darkness.",
	/*Done*/
	J,		1,	},
	{ "Gift of the Skunk",
	  "(ODOUR) Releasing a defensive scent.",
	/*Done*/
	A,		2,	},
	{ "Armor of Luna",
	  "(MOONSHIELD) Donning the armor of the moon.",
	/*Done*/
	B,		2,	},
	{ "Control Technology",
	  "(SIPHON) Control the trappings of the modern world.",
	/*Done*/
	C,		2,	},
	{ "Shed",
	  "Move faster at the expense of your stamina.",
	/*Done*/
	D,		2,	},
	{ "Scent of the True Form",
	  "(TRUESCENT) Sniff out the wolf in sheep's clothing.",
	/*Done*/
	E,		2,	},
	{ "Blur",
	  "Hide in the shadows.",
	/*Done*/
	F,		2,	},
	{ "Mother's Touch",
	  "Heal one wound.",
	G,		2,	},
	{ "Beckon",
	  "Summoning animals to your side.",
	/*Done*/
	H,		2,	},
	{ "Call of the Wyld",
	  "(WYLDCALL) Howl to all Garou.",
	I,		2,	},
	{ "Razor Claws",
	  "(auto) Increased damage from razor sharp claws.",
	J,		2,	},
	{ "Staredown",
	  "Scare your opponent into fleeing.",
	/*Done*/
	K,		2,	},
	{ "Gift of the Termite",
	  "(TERMITE) Eating wooden objects.",
	/*Done*/
	A,		3,	},
	{ "Silver Claws",
	  "(SILVERCLAWS) Baring claws of shining silver.",
	/*Done*/
	B,		3,	},
	{ "Heat Metal",
	  "(HEATMETAL) Turn up the heat on your opponent's metal equipment.",
	/*Done*/
	C,		3,	},
	{ "Disquiet",
	  "Cause your target to suffer depression.",
	/*Done*/
	D,		3,	},
	{ "Mind Speak",
	  "(MINDSPEAK) Telepathic messages.",
	E,		3,	},
	{ "Vanish",
	  "Become invisible.",
	/*Done*/
	F,		3,	},
	{ "Weak Arm",
	  "(WEAKARM) Weaken an opponent.",
	/*Done*/
	G,		3,	},
	{ "Dreamspeak",
	  "Speak into the target's dreams.",
	H,		3,	},
	{ "Sense Silver",
	  "(SENSESILVER) Detecting the presence of silver.",
	/*Done*/
	I,		3,	},
	{ "Compel Spirits",
	  "(COMPEL) Command spirits to follow your command.",
	J,		3,	},
	{ "Attune",
	  "Sensing those in the area.",
	/*Done*/
	A,		4,	},
	{ "Wrath of Gaia",
	  "(WRATH) Unleash the wrath of Gaia on the Wyrm's minions.",
	/*Done*/
	B,		4,	},
	{ "Doppleganger",
	  "Take on another face.",
	/*Done*/
	C,		4,	},
	{ "Tongues",
	  "Not yet implemented.",
	D,		4,	},
	{ "Grovel",
	  "Ingratiating yourself to stop a fight.",
	/*Done*/
	E,		4,	},
	{ "Gnaw",
	  "Not yet implemented.",
	F,		4,	},
	{ "Taking the Forgotten",
	  "(FILCH) Sneak items away from someone.",
	G,		4,	},
	{ "Exorcism",
	  "Driving away spirits.",
	H,		4,	},
	{ "Umbral Scent",
	  "(SNIFFUMBRA) Scry a location in the umbra.",
	I,		4,	},
	{ "Cobra Eyes",
	  "(COBRAEYES) Render a target docile.",
	J,		4,	},
	{ "Silver Claws",
	  "(SILVERCLAWS) Baring claws of shining silver.",
	K,		4,	},
	{ "Survivor",
	  "(auto) Surviving death itself.",
	A,		5,	},
	{ "Luna's Avenger",
	  "(LUNASAVENGER) Become a giant silver werewolf.",
	/*Done*/
	B,		5,	},
	{ "Phone Travel",
	  "(PTRAVEL) Travelling from phone to phone.",
	C,		5,	},
	{ "Summon Elemental",
	  "Not yet implemented.",
	D,		5,	},
	{ "Luna's Blessing",
	  "(LUNASBLESSING) No aggravated damage from silver.",
	E,		5,	},
	{ "Spirit Drain",
	  "Replenish your gnosis by injuring a spirit.",
	F,		5,	},
	{ "True Form",
	  "(TRUEFORM) Force shapeshifter into their true form.",
	G,		5,	},
	{ "Bridge Walker",
	  "(BRIDGEWALK) Step from place to place.",
	H,		5,	},
	{ "Strength of Will",
	  "(SHAREWILL) Share some of your willpower with your allies.",
	I,		5,	},
	{ NULL,	0,		0,	}
};

const struct group_gift_type group_gift_table [] =
{
	{ "silver fangs",		{ B, B, B, B, B }	},
	{ "bone gnawers",		{ A, A, A, A, A }	},
	{ "glass walkers",		{ C, C, C, C, C }	},
	{ "fenrir",				{ F, 0, 0, 0, 0 }	},
	{ "red talons",			{ G, 0, 0, 0, 0 }	},
	{ "wendigo",			{ 0, 0, 0, 0, 0 }	},
	{ "shadow lords",		{ 0, B, 0, 0, 0 }	},
	{ "uktena",				{ 0, 0, 0, 0, 0 }	},
	{ "children of gaia",	{ 0, 0, 0, 0, 0 }	},
	{ "black furies",		{ B, 0, 0, 0, 0 }	},
	{ "fianna",				{ I, 0, 0, 0, 0 }	},
	{ "silent striders",	{ B, F, 0, 0, 0 }	},
	{ "homid",				{ D, D, D, D, 0 }	},
	{ "metis",				{ B, 0, 0, 0, 0 }	},
	{ "lupus",				{ C, F, I, F, D }	},
	{ "ragabash",			{ E, G, F, G, E }	},
	{ "theurge",			{ B, H, J, H, F }	},
	{ "philodox",			{ F, I, G, I, G }	},
	{ "galliard",			{ G, G, H, J, H }	},
	{ "ahroun",				{ H, H, I, K, I }	},
	{ "wyrm",				{ 0, 0, 0, 0, 0 }	},
	{ NULL,					{ 0, 0, 0, 0, 0 }	}
};

const struct gift_spirit_teacher spirit_teacher_table [] =
{
	{ 1, A, { "raccoon", "rat", NULL, NULL } },
	{ 1, B, { "gaia", NULL, NULL, NULL } },
	{ 1, C, { "wolf", NULL, NULL, NULL } },
	{ 1, D, { "ancestor", NULL, NULL, NULL } },
	{ 1, E, { "fox", NULL, NULL, NULL } },
	{ 1, F, { "bear", NULL, NULL, NULL } },
	{ 1, G, { "nature", NULL, NULL, NULL } },
	{ 1, H, { "bird", "bat", "wind", "air" } },
	{ 1, I, { "ancestor", NULL, NULL, NULL } },
	{ 1, J, { "night", NULL, NULL, NULL } },
	{ 2, A, { "skunk", NULL, NULL, NULL } },
	{ 2, B, { "lune", NULL, NULL, NULL } },
	{ 2, C, { "technology", NULL, NULL, NULL } },
	{ 2, D, { "lizard", "snake", NULL, NULL } },
	{ 2, E, { "gaia", NULL, NULL, NULL } },
	{ 2, F, { "chameleon", NULL, NULL, NULL } },
	{ 2, G, { "unicorn", NULL, NULL, NULL } },
	{ 2, H, { "incarna", NULL, NULL, NULL } },
	{ 2, I, { "wolf", NULL, NULL, NULL } },
	{ 2, J, { "cat", "wolf", NULL, NULL } },
	{ 2, K, { "ram", "snake", NULL, NULL } },
	{ 3, A, { "termite", NULL, NULL, NULL} },
	{ 3, B, { "lune", NULL, NULL, NULL } },
	{ 3, C, { "fire", "metal", "earth", NULL } },
	{ 3, D, { "ancestor", NULL, NULL, NULL } },
	{ 3, E, { "bird", "thought", NULL, NULL } },
	{ 3, F, { "chameleon", "tiger", NULL, NULL } },
	{ 3, G, { "snake", "wind", NULL, NULL } },
	{ 3, H, { "chimerling", NULL, NULL, NULL } },
	{ 3, I, { "lune", NULL, NULL, NULL } },
	{ 3, J, { "incarna", NULL, NULL, NULL } },
	{ 4, A, { "cockroach", "rat", "owl", NULL } },
	{ 4, B, { "gaia", NULL, NULL, NULL } },
	{ 4, C, { "chameleon", NULL, NULL, NULL } },
	{ 4, D, { "raven", NULL, NULL, NULL } },
	{ 4, E, { "canine", "dog", "wolf", "fox" } },
	{ 4, F, { "wolf", "rat", NULL, NULL } },
	{ 4, G, { "mouse", NULL, NULL, NULL } },
	{ 4, H, { "incarna", NULL, NULL, NULL } },
	{ 4, I, { "bird", NULL, NULL, NULL } },
	{ 4, J, { "snake", NULL, NULL, NULL } },
	{ 5, A, { "bear", NULL, NULL, NULL } },
	{ 5, B, { "lune", NULL, NULL, NULL } },
	{ 5, C, { NULL, NULL, NULL, NULL } },
	{ 5, D, { "earth", "air", "fire", "elemental" } },
	{ 5, E, { "lune", NULL, NULL, NULL } },
	{ 5, F, { "uktena", NULL, NULL, NULL } },
	{ 5, G, { "wolf", NULL, NULL, NULL } },
	{ 5, H, { "lune", NULL, NULL, NULL } },
	{ 5, I, { "wolf", NULL, NULL, NULL } },
	{ 0, 0, { NULL, NULL, NULL, NULL } }
};

const struct gift_type	ww_ritual_table [] =
{
	{ "", "",
	/*Done*/
	A,		1,	},
	{ NULL,	0,		0,	}
};

const struct gift_type	vamp_ritual_table [] =
{
	{ "", "",
	/*Done*/
	A,		1,	},
	{ NULL,	0,		0,	}
};


const struct colour_type colour_table [] =
{
	{ "x", CLEAR,	 	 1 },
	{ "b", C_BLUE,		 1 },
	{ "c", C_CYAN,		 1 },
	{ "g", C_GREEN,		 1 },
	{ "m", C_MAGENTA,		 1 },
	{ "r", C_RED,		 1 },
	{ "w", C_WHITE,		 1 },
	{ "y", C_YELLOW,		 1 },
	{ "B", C_B_BLUE,		 1 },
	{ "C", C_B_CYAN,		 1 },
	{ "G", C_B_GREEN,		 1 },
	{ "M", C_B_MAGENTA,		 1 },
	{ "R", C_B_RED,		 1 },
	{ "W", C_B_WHITE,		 1 },
	{ "Y", C_B_YELLOW,		 1 },
	{ "D", C_D_GREY,		 1 },
	{ "x", WEBCLEAR,		 2 },
	{ "b", WEBC_BLUE,		 2 },
	{ "c", WEBC_CYAN,		 2 },
	{ "g", WEBC_GREEN,		 2 },
	{ "m", WEBC_MAGENTA,	 2 },
	{ "r", WEBC_RED,		 2 },
	{ "w", WEBC_WHITE,		 2 },
	{ "y", WEBC_YELLOW,		 2 },
	{ "B", WEBC_B_BLUE,		 2 },
	{ "C", WEBC_B_CYAN,		 2 },
	{ "G", WEBC_B_GREEN,	 2 },
	{ "M", WEBC_B_MAGENTA,	 2 },
	{ "R", WEBC_B_RED,		 2 },
	{ "W", WEBC_B_WHITE,	 2 },
	{ "Y", WEBC_B_YELLOW,	 2 },
	{ "D", WEBC_D_GREY,		 2 },
	{ "*", C_DING,		 0 },
	{ "/", CR,			 0 },
	{ "-", TILDE,		 0 },
	{ "{", CURLY_BRACKET,	 0 },
	{ NULL,NULL,		 0 }
};

const struct trait_struct derangement_table [] =
{
	{ "mania1",			 0, -1, TRUE }, /* Mild mania. Manias will have sub-flags.*/
	{ "mania2",			 1, -2, TRUE }, /* Serious mania. */
	{ "mania3",			 2, -3, TRUE }, /* Extreme mania. */
	{ "phobia1",		 3, -1, TRUE }, /* Mild phobia. Phobias will have sub-flags*/
	{ "phobia2",		 4, -2, TRUE }, /* Serious phobia. */
	{ "phobia3",		 5, -3, TRUE }, /* Extreme phobia. */
	{ "obsessive/compulsive",	 6, -1, TRUE }, /* Has sub-flags. */
	{ "schizophrenia",		 7, -1, TRUE }, /* Serious mood swings. */
	{ "paranoia",		 8, -1, TRUE }, /* Paranoia. */
	{ "paranoidschiz",		 9, -2, TRUE }, /* Serious mood swings + paranoia. */
	{ "mpd",			10, -1, TRUE }, /* Multiple personalities. */
	{ "tick",			11, -1, TRUE }, /* Nervous tick. */
	{ "amnesia",		12, -2, TRUE }, /* Forget everything on a regular basis. */
	{ "sociopathic",		13, -1, TRUE }, /* Has to attempt to cause indirect injury*/
	{ "autistic",		14, -1, TRUE }, /* Autism. For more information ask Dsarky*/
	{ "catatonia",		15, -2, TRUE }, /* Fetal position, unaware of surroundings*/
	{ "stupefaction",		16, -5, TRUE }, /* 0 willpower, no interest or talking.*/
	{ "pantophobia",		17, -3, TRUE }, /* Fear of everything.*/
	{ "quixotism",		18, -1, TRUE }, /* Everything appears supernatural.*/
	{ "panzaism",		19, -1, TRUE }, /* Everything is normal.*/
	{ "angelism",		20, -1, TRUE }, /* Angels talk to character.*/
	{ NULL,			-1, 0, FALSE }
};

const struct flag_type phobia_table [] =
{
	{ "acrophobia",	 0, TRUE }, /* Fear of heights. */
	{ "agoraphobia",	 1, TRUE }, /* Fear of open spaces. */
	{ "ailurophobia",	 2, TRUE }, /* Fear of cats. */
	{ "algophobia",	 3, TRUE }, /* Fear of pain. */
	{ "androphobia",	 4, TRUE }, /* Fear of men. */
	{ "anthophobia",	 5, TRUE }, /* Fear of flowers. */
	{ "anthrophobia",	 6, TRUE }, /* Fear of people. */
	{ "arachnophobia",	 7, TRUE }, /* Fear of archnids/spiders. */
	{ "bacteriophobia",	 8, TRUE }, /* Fear of bacteria. */
	{ "ballistophobia",	 9, TRUE }, /* Fear of bullets. */
	{ "bathophobia",	10, TRUE }, /* Fear of depth. */
	{ "belonaphobia",	11, TRUE }, /* Fear of pins and needles. */
	{ "botanophobia",	12, TRUE }, /* Fear of plants. */
	{ "claustrophobia",	13, TRUE }, /* Fear of enclosed spaces. */
	{ "clinophobia",	14, TRUE }, /* Fear of beds. */
	{ "demophobia",	15, TRUE }, /* Fear of crowds. */
	{ "dendrophobia",	16, TRUE }, /* Fear of trees. */
	{ "doraphobia",	17, TRUE }, /* Fear of fur. */
	{ "gynephobia",	18, TRUE }, /* Fear of women. */
	{ "hematophobia",	19, TRUE }, /* Fear of blood. */
	{ "hydrophobia",	20, TRUE }, /* Fear of water. */
	{ "iatrophobia",	21, TRUE }, /* Fear of doctors. */
	{ "necrophobia",	22, TRUE }, /* Fear of dead things. */
	{ "nyctophobia",	23, TRUE }, /* Fear of nightfall. */
	{ "ombrophobia",	24, TRUE }, /* Fear of rain. */
	{ "ophiophobia",	25, TRUE }, /* Fear of snakes. */
	{ "pediphobia",	26, TRUE }, /* Fear of children. */
	{ "scotophobia",	27, TRUE }, /* Fear of darkness. */
	{ "teratophobia",	28, TRUE }, /* Fear of monsters. */
	{ "xenophobia",	29, TRUE }, /* Fear of "aliens"/foreigners. */
	{ "zoophobia",	30, TRUE }, /* Fear of animals. */
	{ NULL,		-1, FALSE }
};

const struct flag_type compulsion_table [] =
{
	{ "avoidance",	 0, TRUE }, /* Avoids contact with others. */
	{ "cleaning",	 1, TRUE }, /* Insists on cleaning up everything. */
	{ "hygene",		 2, TRUE }, /* Must be clean/avoid germs. */
	{ "pain (self)",	 3, TRUE }, /* Insists on inflictin pain on self. */
	{ "pain (others)",	 4, TRUE }, /* Insists on inflictin pain on others. */
	{ "time",		 5, TRUE }, /* Has to know what the time is. */
	{ "expert",		 6, TRUE }, /* "Absolute" expertise in an area. */
	{ "critic",		 7, TRUE }, /* Cannot avoid criticising. */
	{ "turets",		 8, TRUE }, /* Yells/swears loudly. */
	{ "nudity",		 9, TRUE }, /* Must remove any clothes worn. */
	{ NULL,		-1, FALSE }
};

const struct flag_type mania_table [] =
{
	{ "pyromania",	 0, TRUE }, /* Has to play with fire/burning/exploding*/
	{ "hydromania",	 1, TRUE }, /* Has to play with water/puddles/etc. */
	{ "emotional mania", 2, TRUE }, /* Uncontrollable laughing/sobbing/etc. */
	{ "hysteria",	 3, TRUE }, /* Uncontrollable shrieking. */
	{ "habromania",	 4, TRUE }, /* Uncontrolled giggles at morbidity. */
	{ NULL,		-1, FALSE }
};

const struct flag_type feeding_restriction [] =
{
	{ "male",		 0, TRUE },
	{ "female",		 1, TRUE },
	{ "youth",		 2, TRUE },
	{ "elderly",	 3, TRUE },
	{ "ugly",		 4, TRUE },
	{ "attractive",	 5, TRUE },
	{ "stunning",	 6, TRUE },
	{ "profession",	 7, TRUE }, /* sub-flags */
	{ "wealth",		 8, TRUE },
	{ "strong willed",	 9, TRUE },
	{ "afraid",		10, TRUE },
	{ "angry",		11, TRUE },
	{ "drunk",		12, TRUE },
	{ "high",		13, TRUE },
	{ "tripping",	14, TRUE },
	{ "strong",		15, TRUE },
	{ "weak",		16, TRUE },
	{ "intelligent",	17, TRUE },
	{ "dumb",		18, TRUE },
	{ "crazy",		19, TRUE },
	{ "nature",		20, TRUE }, /* sub-flags */
	{ "demeanor",	21, TRUE }, /* sub-flags */
	{ "blonde",		22, TRUE },
	{ "brunette",	23, TRUE },
	{ "redhead",	24, TRUE },
	{ NULL,		-1, FALSE }
};

const struct flag_type trait_type[] =
{
	{	"quirks",	TRAIT_QUIRK,		TRUE	},
	{	"merits",	TRAIT_MERIT,		TRUE	},
	{	"flaws",	TRAIT_FLAW,		TRUE	},
	{	"derangements",	TRAIT_DERANGEMENT,	TRUE	},
	{	NULL,		-1,			FALSE	}
};

const struct flag_type quirk_type [] =
{
	{ "like",		 0, TRUE },
	{ "dislike",	 1, TRUE },
	{ "hatred", 	 2, TRUE },
	{ "tick",	 	 3, TRUE },
	{ "accent", 	 4, TRUE },
	{ "mannerism", 	 5, TRUE },
	{ "personality", 	 6, TRUE },
	{ NULL,		-1, FALSE }
};

/*************************************************
*  Part One: Name of Trait                       *
*  Part Two: Bit number                          *
*  Part Three: Trait cost                        *
*  Part Four: Settable (True/False)              *
**************************************************/
const struct trait_struct merit_table [] =
{
	{ "acute hearing",			 0, 1, TRUE },
	{ "acute sense of smell",		 1, 1, TRUE },
	{ "acute sense of taste",		 2, 1, TRUE },
	{ "acute vision",			 3, 1, TRUE },
	{ "ambidextrous",			 4, 1, TRUE },
	{ "baby face",			 5, 2, TRUE },
	{ "berserker",			 6, 2, TRUE },
	{ "boon1",				 7, 1, TRUE },
	{ "boon2",				 8, 2, TRUE },
	{ "boon3",				 9, 3, TRUE },
	{ "calm heart",			10, 3, TRUE },
	{ "charmed existence",		11, 5, TRUE },
	{ "church ties",			12, 3, TRUE },
	{ "clan friendship",		13, 3, TRUE },
	{ "code of honor",			14, 1, TRUE },
	{ "common sense",			15, 1, TRUE },
	{ "computer aptitude",		16, 1, TRUE },
	{ "concentration",			17, 1, TRUE },
	{ "corporate ties",			18, 3, TRUE },
	{ "CEO",				19, 5, TRUE },
	{ "crack driver",			20, 1, TRUE },
	{ "danger sense",			21, 2, TRUE },
	{ "daredevil",			22, 3, TRUE },
	{ "destiny",			23, 4, TRUE },
	{ "double-jointed",			24, 1, TRUE },
	{ "eat food",			25, 1, TRUE },
	{ "eidetic memory",			26, 2, TRUE },
	{ "efficient digestion",		27, 3, TRUE },
	{ "faerie affinity",		28, 2, TRUE },
	{ "fast learner",			29, 3, TRUE },
	{ "guardian angel",			30, 6, TRUE },
	{ "higher purpose",			31, 1, TRUE },
	{ "huge size",			32, 4, TRUE },
	{ "iron will",			33, 3, TRUE },
	{ "jack-of-all-trades",		34, 5, TRUE },
	{ "judicial ties",			35, 2, TRUE },
	{ "light sleeper",			36, 2, TRUE },
	{ "lightning calculator",		37, 1, TRUE },
	{ "luck",				38, 3, TRUE },
	{ "magic resistance",		39, 2, TRUE },
	{ "mansion",			40, 2, TRUE },
	{ "mechanical aptitude",		41, 1, TRUE },
	{ "media ties",			42, 2, TRUE },
	{ "medium",				43, 2, TRUE },
	{ "misplaced heart",		44, 2, TRUE },
	{ "natural linguist",		45, 2, TRUE },
	{ "nightclub",			46, 2, TRUE },
	{ "occult library",			47, 2, TRUE },
	{ "pawn",				48, 3, TRUE },
	{ "pitiable",			49, 1, TRUE },
	{ "police ties",			50, 3, TRUE },
	{ "political ties",			51, 3, TRUE },
	{ "prestigious sire",		52, 1, TRUE },
	{ "reputation",			53, 2, TRUE },
	{ "self-confident",			54, 5, TRUE },
	{ "special gift1",			55, 1, TRUE },
	{ "special gift2",			56, 2, TRUE },
	{ "special gift3",			57, 3, TRUE },
	{ "spirit mentor",			58, 3, TRUE },
	{ "time sense",			59, 1, TRUE },
	{ "true faith",			60, 7, TRUE },
	{ "inoffensive to animals",		61, 1, TRUE },
	{ "true love",			62, 1, TRUE },
	{ "unbondable",			63, 3, TRUE },
	{ "underworld ties",		64, 3, TRUE },
	{ "werewolf companion",		65, 3, TRUE },
	{ NULL,				-1, 0, FALSE }
};

/*************************************************
*  Part One: Name of Trait                       *
*  Part Two: Bit number                          *
*  Part Three: Trait cost                        *
*  Part Four: Settable (True/False)              *
**************************************************/
const struct trait_struct flaw_table [] =
{
	{ "feedrestriction",		 0, -1, TRUE },
	{ "allergic",			 1, -1, TRUE },
	{ "amnesia",			 2, -1, TRUE },
	{ "absent minded",			 3, -1, TRUE },
	{ "anachronism",			 4, -1, TRUE },
	{ "bad sight",			 5, -2, TRUE },
	{ "blind",				 6, -6, TRUE },
	{ "cant cross running water",	 7, -3, TRUE },
	{ "child",				 8, -3, TRUE },
	{ "clan enmity",			 9, -2, TRUE },
	{ "color blindness",		10, -1, TRUE },
	{ "confused",			11, -2, TRUE },
	{ "cursed1",			12, -1, TRUE },
	{ "cursed2",			13, -2, TRUE },
	{ "cursed3",			14, -3, TRUE },
	{ "cursed4",			15, -4, TRUE },
	{ "cursed5",			16, -5, TRUE },
	{ "dark fate",			17, -5, TRUE },
	{ "dark secret",			18, -1, TRUE },
	{ "deaf",				19, -4, TRUE },
	{ "deep sleeper",			20, -1, TRUE },
	{ "deformity",			21, -3, TRUE },
	{ "diabolic sire",			22, -2, TRUE },
	{ "disfigured",			23, -2, TRUE },
	{ "driving goal",			24, -3, TRUE },
	{ "enemy1",				25, -1, TRUE },
	{ "enemy2",				26, -2, TRUE },
	{ "enemy3",				27, -3, TRUE },
	{ "enemy4",				28, -4, TRUE },
	{ "enemy5",				29, -5, TRUE },
	{ "hard of hearing",		30, -1, TRUE },
	{ "hatred",				31, -3, TRUE },
	{ "haunted",			32, -3, TRUE },
	{ "hunted",				33, -4, TRUE },
	{ "illiterate",			34, -1, TRUE },
	{ "inept",				35, -5, TRUE },
	{ "infamous sire",			36, -1, TRUE },
	{ "insane sire",			37, -1, TRUE },
	{ "intolerance",			38, -1, TRUE },
	{ "lame",				39, -3, TRUE },
	{ "light sensitive",		40, -5, TRUE },
	{ "low self-image",			41, -2, TRUE },
	{ "magic susceptibility",		42, -3, TRUE },
	{ "mistaken identity",		43, -1, TRUE },
	{ "monstrous",			44, -3, TRUE },
	{ "mute",				45, -4, TRUE },
	{ "nightmares",			46, -1, TRUE },
	{ "notoriety",			47, -3, TRUE },
	{ "one arm",			48, -3, TRUE },
	{ "one eye",			49, -2, TRUE },
	{ "overconfident",			50, -1, TRUE },
	{ "paraplegic",			51, -6, TRUE },
	{ "permanent wound",		52, -3, TRUE },
	{ "repelled by crosses",		53, -3, TRUE },
	{ "repulsed by garlic",		54, -1, TRUE },
	{ "selective digestion",		55, -2, TRUE },
	{ "short",				56, -1, TRUE },
	{ "short fuse",			57, -2, TRUE },
	{ "shy",				58, -1, TRUE },
	{ "resentful sire",			59, -1, TRUE },
	{ "soft hearted",			60, -1, TRUE },
	{ "speech impediment",		61, -1, TRUE },
	{ "taint of corruption",		62, -1, TRUE },
	{ "territorial",			63, -2, TRUE },
	{ "thin-blooded",			64, -4, TRUE },
	{ "twisted upbringing",		65, -1, TRUE },
	{ "uneducated",			66, -5, TRUE },
	{ "unskilled",			67, -5, TRUE },
	{ "vengeance",			68, -2, TRUE },
	{ "ward",				69, -3, TRUE },
	{ "weak-willed",			70, -2, TRUE },
	{ "unlucky1",			71, -1, TRUE },
	{ "unlucky2",			72, -2, TRUE },
	{ "unlucky3",			73, -3, TRUE },
	{ "unlucky4",			74, -4, TRUE },
	{ "unlucky5",			75, -5, TRUE },
	{ NULL,				-1, 0, FALSE }
};

const struct flag_type org_type [] =
{
	{ "anarchistic",	0,	TRUE },
	{ "dictatorial",	1, 	TRUE },
	{ "democratic",	2, 	TRUE },
	{ "no_leader",	3, 	TRUE },
	{ NULL,		-1,	FALSE }
};

const struct flag_type org_auths [] =
{
	{ "induct",		A,	TRUE	},
	{ "boot",		B,	TRUE	},
	{ "build",		C,	TRUE	},
	{ "cmd1",		D,	TRUE	},
	{ "cmd2",		E,	TRUE	},
	{ "cmd3",		F,	TRUE	},
	{ "cmd4",		G,	TRUE	},
	{ "cmd5",		H,	TRUE	},
	{ "members",	I,	TRUE	},
	{ "vote",		J,	TRUE	},
	{ "status",		K,	TRUE	},
	{ NULL,		-1,	FALSE	}
};

const struct org_cmd_type org_cmd_table [] =
{
	{ "rite",	do_rituals	},
/*    { "",	do_	}, */
	{ NULL,	NULL	}
};

const struct home_cmd_type home_cmd_table [] =
{
	{ "commands",	0,	0,	home_commands		},
	{ "buy",		2,	2,	home_buy		},
	{ "prices",		2,	2,	home_prices		},
	{ "sell",		2,	2,	home_sell		},
	{ "desc",		2,	2,	home_desc		},
	{ "name",		2,	2,	home_name		},
	{ "list",		0,	0,	home_list		},
/*    { "key",		2,	3,	home_key		},
	{ "furnish",	2,	2,	home_furnish		},
	{ "unfurnish",	0,	2,	home_unfurnish		},
	{ "place",		0,	2,	home_place		},
	{ "remove",		0,	2,	home_remove		},
	{ "",		0,	home_		}, */
	{ NULL,		0,	0,	NULL			}
};

const struct home_price_type home_price_table [] =
{
	{ "Home",		"None",		50000	},
	{ "Room",		"None",		10000	},
	{ "Desc",		"None",		2000	},
	{ "Name",		"None",		500	},
	{ NULL,		NULL,		0	}
};

const struct flag_type quality_flags [] =
{
	{ "poor",			0,	FALSE	},
	{ "functional",		1,	TRUE	},
	{ "functional",		2,	TRUE	},
	{ "good",			3,	TRUE	},
	{ "good",			4,	TRUE	},
	{ "very good",		5,	TRUE	},
	{ "very good",		6,	TRUE	},
	{ "exceptional",		7,	TRUE	},
	{ "exceptional",		8,	TRUE	},
	{ "exceptional",		9,	TRUE	},
	{ "exceptional",		10,	TRUE	},
	{ "extraordinary",		11,	TRUE	},
	{ "extraordinary",		12,	TRUE	},
	{ "master craftsmanship",	13,	TRUE	},
	{ "master craftsmanship",	14,	TRUE	},
	{ "artistic perfection",	15,	TRUE	},
	{ NULL,			0,	FALSE	}
};

const struct flag_type raw_material_table [] =
{
	{ "none",		MATERIAL_NONE,		FALSE	},
	{ "edible",		MATERIAL_EDIBLE,	TRUE	},
	{ "poison",		MATERIAL_POISON,	TRUE	},
	{ "metal",		MATERIAL_METAL,		TRUE	},
	{ "explosive",	MATERIAL_EXPLOSIVE,	TRUE	},
	{ "liquid",		MATERIAL_LIQUID,	FALSE	},
	{ "drugs",		MATERIAL_DRUGS,		TRUE	},
	{ "alcohol",	MATERIAL_ALCOHOL,	TRUE	},
	{ "fabric",		MATERIAL_FABRIC,	TRUE	},
	{ "stone",		MATERIAL_STONE,		TRUE	},
	{ NULL,		0,			FALSE	}
};

const struct flag_type flag_list[] =
{
	{	"A",		A,		TRUE	},
	{	"B",		B,		TRUE	},
	{	"C",		C,		TRUE	},
	{	"D",		D,		TRUE	},
	{	"E",		E,		TRUE	},
	{	"F",		F,		TRUE	},
	{	"G",		G,		TRUE	},
	{	"H",		H,		TRUE	},
	{	"I",		I,		TRUE	},
	{	"J",		J,		TRUE	},
	{	"K",		K,		TRUE	},
	{	"L",		L,		TRUE	},
	{	"M",		M,		TRUE	},
	{	"N",		N,		TRUE	},
	{	"O",		O,		TRUE	},
	{	"P",		P,		TRUE	},
	{	"Q",		Q,		TRUE	},
	{	"R",		R,		TRUE	},
	{	"S",		S,		TRUE	},
	{	"T",		T,		TRUE	},
	{	"U",		U,		TRUE	},
	{	"V",		V,		TRUE	},
	{	"W",		W,		TRUE	},
	{	"X",		X,		TRUE	},
	{	"Y",		Y,		TRUE	},
	{	"Z",		Z,		TRUE	},
	{	"a",		aa,		TRUE	},
	{	"b",		bb,		TRUE	},
	{	"c",		cc,		TRUE	},
	{	"d",		dd,		TRUE	},
	{	"e",		ee,		TRUE	},
	{	NULL,		0,		FALSE	}
};


const	struct	ritemove_type rite_actions [] =
{
  /*

	{
	action name,
	to_char,
	to_room,
	beats
	},

   */

	{
	"step",
	"You take a slow, measured step.",
	"$n takes a solemn step.",
	2
	},

	{
	"temples",
	"You press your fingertips to your temples firmly.",
	"$n rubs $s temples.",
	1
	},

	{
	"circles",
	"You draw circles in the air.",
	"$n waves $s hands in circles.",
	2
	},

	{
	"focus",
	"You focus your mind, drawing in energies.",
	"$n stares intently.",
	2
	},

	{
	"raise",
	"You raise your hands in the air.",
	"$n raises $s hands in the air.",
	2
	},

	{
	"point",
	"You point your finger.",
	"$n points $s finger.",
	1
	},

	{
	"draw",
	"You draw shapes and sigils in the air.",
	"$n waves $s hands in the air.",
	2
	},

	{
	"howl",
	"You let out a howl.",
	"$n howls.",
	2
	},

	{
	"chant",
	"You chant the words...",
	"$n begins to chant, but you don't recognise the language.",
	4
	},

	{
	"trace",
	"You trace out symbols on nearby surfaces.",
	"$n traces out symbols.",
	3
	},

	{
	"entreat",
	"You entreat the powers to your aid.",
	"$n seems to call on some occult source of power.",
	4
	},

	{
	"scratch",
	"You scratch symbols in the earth.",
	"$n scratches around on the ground.",
	5
	},

	{
	"splay",
	"You splay your hand before you with an outstretched arm.",
	"$n splays $s hand at arms length as if directing something.",
	1
	},

	{
	"halt",
	"You demand a halt!",
	"$n holds up $s hand, as if to demand 'Halt!'",
	1
	},

	{
	"dance",
	"You dance around in a circle.",
	"$n dances around in a circle.",
	3
	},

	{
	"breathe",
	"You perform the breathing exercises.",
	"$n starts controlling $s breathing.",
	1
	},

	{
	"blood",
	"You draw blood from your hand.",
	"$n draws some of $s own blood!",
	1
	},

	{
	"beat",
	"You beat your chest rhythmically.",
	"$n beats $s chest.",
	2
	},

	{
	"groan",
	"You groan loudly.",
	"$n groans loudly.",
	3
	},

	{
	NULL,
	NULL,
	NULL,
	0
	}
};


const struct ritual_type ritual_table [] =
{

	{
	"rite of introduction",
	"vampire", DISC_THAUMATURGY, 1,
	{ 15, 3, 1, -1, -1, -1, -1, -1, -1, -1 },
	2
	},

	{
	"rite of introduction",
	"werewolf", -1, 1,
	{ 15, 3, 1, -1, -1, -1, -1, -1, -1, -1 },
	1
	},

	{
	"rite of attunement",
	"werewolf", -1, 1,
	{ 1, 4, 5, 10, -1, -1, -1, -1, -1, -1 },
	1
	},

	{
	"rite of pack creation",
	"werewolf", -1, 1,
	{ 10, 8, 0, 2, 8, 4, 7, 10, 14, -1 },
	1
	},

	{
	"rite of recognition",
	"werewolf", -1, 1,
	{ 0, 4, 0, 4, 5, 12, 13, 16, 5, -1 },
	1
	},

	{
	"rite of the returning hero",
	"werewolf", -1, 1,
	{ 0, 4, 0, 4, 7, 17, 7, 13, -1, -1 },
	1
	},

	{
	NULL,
	NULL, -1, -1,
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	0
	}
};


const struct flag_type fame_table	[] =
{
	{ "Unknown",		0,		TRUE	},
	{ "Recognised",		1,		TRUE	},
	{ "Local celebrity",	2,		TRUE	},
	{ "Fairly famous",		3,		TRUE	},
	{ "National icon",		4,		TRUE	},
	{ "International star",	5,		TRUE	},
	{ NULL,			-1,		FALSE	}
};


const struct flag_type mprog_flags[] =
{
	{  "act",                  TRIG_ACT,               TRUE    },
	{  "bribe",                TRIG_BRIBE,             TRUE    },
	{  "death",                TRIG_DEATH,             TRUE    },
	{  "entry",                TRIG_ENTRY,             TRUE    },
	{  "fight",                TRIG_FIGHT,             TRUE    },
	{  "give",                 TRIG_GIVE,              TRUE    },
	{  "greet",                TRIG_GREET,             TRUE    },
	{  "grall",                TRIG_GRALL,             TRUE    },
	{  "kill",                 TRIG_KILL,              TRUE    },
	{  "hpcnt",                TRIG_HPCNT,             TRUE    },
	{  "random",               TRIG_RANDOM,            TRUE    },
	{  "speech",               TRIG_SPEECH,            TRUE    },
	{  "exit",                 TRIG_EXIT,              TRUE    },
	{  "exall",                TRIG_EXALL,             TRUE    },
	{  "delay",                TRIG_DELAY,             TRUE    },
	{  "surr",                 TRIG_SURR,              TRUE    },
	{  "antiobj",              TRIG_ANTIOBJ,           TRUE    },
	{  "powerdisplay",         TRIG_POWERDISP,         TRUE    },
	{  NULL,                   0,                      TRUE    }
};

const  struct  bit_type        bitvector_type  []      =
{
	{       affect_flags,   "affect"        },
	{       apply_flags,    "apply"         },
	{       imm_flags,      "imm"           },
	{       res_flags,      "res"           },
	{       vuln_flags,     "vuln"          },
	{       weapon_type2,   "weapon"        }
};

const	struct	flag_type		train_messages[] =
{
	{	"The trains in spain can drive one insane.",	1,	TRUE	},
	{	"Graffiti marks the concrete wall along the side of the track, declaring that Gab has visited the location.",	20,	TRUE	},
	{	"The lights of a tunnel flash outside the windows as the train races on.",	50,	TRUE	},
	{	"A flock of birds is visible in the distance.",	20,	TRUE	},
	{	"A voice crackles over the speakers, muttering unintelligibly in syllables so distorted as to be meaningless.",	50,	TRUE	},
	{	"A smell like hot oil fill your nostrils for a moment, but passes quickly.",	10,	TRUE	},
	{	"The brakes squeal as the train driver adjusts speeds.",	70,	TRUE	},
	{	"The constant drone and vibration dulls your senses, making it hard not to let your mind drift.",	80,	TRUE	},
	{	"A loud rushing noise and flashing lights fill the car for a moment as another train passes in a blinding flash.",	30,	TRUE	},
	{	NULL,	0,	FALSE	}
};


const struct flag_type con_state_flags[] =
{
	{ "    PLAYING    ",	CON_PLAYING,			TRUE	},
	{ "   Get Name    ",	CON_GET_NAME,			TRUE	},
	{ "Get Old Passwd ",	CON_GET_OLD_PASSWORD,		TRUE	},
	{ " Confirm Name  ",	CON_CONFIRM_NEW_NAME,		TRUE	},
	{ "Get New Passwd ",	CON_GET_NEW_PASSWORD,		TRUE	},
	{ "Confirm Passwd ",	CON_CONFIRM_NEW_PASSWORD,	TRUE	},
	{ "  Get New Race ",	CON_GET_NEW_RACE,		TRUE	},
	{ "  Get New Sex  ",	CON_GET_NEW_SEX,		TRUE	},
	{ " Get New Class ",	CON_GET_NEW_CLAN,		TRUE	},
	{ "Select Physical",	CON_ASSIGN_PHYS,		TRUE	},
	{ " Select Mental ",	CON_ASSIGN_MENT,		TRUE	},
	{ " Select Social ",	CON_ASSIGN_SOC,			TRUE	},
	{ " Select Talent ",	CON_ASSIGN_TAL,			TRUE	},
	{ " Select Skills ",	CON_ASSIGN_SKILL,		TRUE	},
	{ "  Knowledges   ",	CON_ASSIGN_KNOWLEDGES,		TRUE	},
	{ " Reading IMOTD ",	CON_READ_IMOTD,			TRUE	},
	{ "   LINKDEAD    ",	CON_BREAK_CONNECT,		TRUE	},
	{ "  Reading MOTD ",	CON_READ_MOTD,			TRUE	},
	{ "Prioritise Attr",	CON_CHOOSE_ATTRIB,		TRUE	},
	{ "Prioritise Abil",	CON_CHOOSE_ABIL,		TRUE	},
	{ " Assign Virtues",	CON_ASSIGN_VIRTUES,		TRUE	},
	{ " Recently Dead ",	CON_DECEASED,			TRUE	},
	{ "   Ethereal    ",	CON_ETHEREAL,			TRUE	},
	{ " Reincarnation ",	CON_REINCARNATE,		TRUE	},
	{ "Confrm New Name",	CON_CONFIRM_REINCARNATE_NAME,	TRUE	},
	{ " Copyover Recov",	CON_COPYOVER_RECOVER,		TRUE	},
	{ " AI Descriptor ",	CON_AIDESC,			TRUE	},
	{ " Get Acct Name ",	CON_GET_ACCT_NAME,		TRUE	},
	{ " Get Acct Pass ",	CON_GET_ACCT_PASS,		TRUE	},
	{ NULL,			-1,				FALSE	}
};


#include <stdlib.h>
#include "twilight.h"

/* Power configuration data set */
struct power_config {
    char *name;
    int  effect_type;
    int  flag_slot_changed;
    int  flags_set;
    int  range;
    char *init_msg_cast;
    char *init_msg_vict;
    char *init_msg_room;
    char *success_msg_cast;
    char *success_msg_vict;
    char *success_msg_room;
    char *fail_msg_cast;
    char *fail_msg_vict;
    char *fail_msg_room;
    char *botch_msg_cast;
    char *botch_msg_vict;
    char *botch_msg_room;
    int  stat_roll_1;
    int  stat_roll_2;
    int  tsk_roll;
    int  virt_roll;
    int  power_roll;
    int  GHB_roll;
    int  RBPG_roll;
    int  diff;
    int  wait;
    int  charge_time;
    int  hear_pos;
    int  cross_plane;
    int  duration; /* -1 = perm, 0 = from roll, >0 = fixed */
};

typedef struct power_config POW;


/* Range definitions
flag_type range_flags {
{    "self",		0,	TRUE	},
{    "character",	1,	TRUE	},
{    "group",		2,	TRUE	},
{    "room",		3,	TRUE	},
{    "remote",		4,	TRUE	},
{    "exit",		5,	TRUE	},
{    "None",		-1	FALSE	}
};


effect type definitions
flag_type effect_type_flags {
{    "damage",		A,	TRUE	},
{    "heal",		B,	TRUE	},
{    "gnosis",		C,	TRUE	},
{    "humanity",	C,	TRUE	},
{    "banality",	C,	TRUE	},
{    "rage",		D,	TRUE	},
{    "blood",		D,	TRUE	},
{    "glamour",		D,	TRUE	},
{    "stat",		E,	TRUE	},
{    "virtue",	F,	TRUE	},
{    "knowledge",	G,	TRUE	},
{    "talent",	G,	TRUE	},
{    "skill",	G,	TRUE	},
{    "flag",	F,	TRUE	},
{    "None",		-1	FALSE	}
};
*/


void do_castspell (CHAR_DATA *ch, char *arg)
{
    /* Find target and power */

    /* Initiate power - Send messages */

    /* Execute power
	- Roll dice
	- Cycle through effects
     */

}

/* Power Messages */
void power_messages(CHAR_DATA *ch, CHAR_DATA *vict, OBJ_DATA *obj, POW *pow, int success)
{
    /* Messages to groups need a slightly different handler. */
	if(pow->range == 2)
	{
		switch ( success ) {
	case 100: /* Initiate */
			if(str_cmp("", pow->init_msg_cast))
			{
				act_new(pow->init_msg_cast, ch, vict, NULL, TO_CHAR, pow->hear_pos, pow->cross_plane);
			}
			if(str_cmp("", pow->init_msg_vict))
			{
				act_new(pow->init_msg_vict, ch, vict, NULL, TO_VICT, pow->hear_pos, pow->cross_plane);
			}
			if(str_cmp("", pow->init_msg_room))
			{
				act_new(pow->init_msg_room, ch, vict, NULL, TO_NOTVICT, pow->hear_pos, pow->cross_plane);
			}
			break;
	case -1: /* Botch */
			if(str_cmp("", pow->botch_msg_cast))
			{
				act_new(pow->botch_msg_cast, ch, vict, NULL, TO_CHAR, pow->hear_pos, pow->cross_plane);
			}
			if(str_cmp("", pow->botch_msg_vict))
			{
				act_new(pow->botch_msg_vict, ch, vict, NULL, TO_VICT, pow->hear_pos, pow->cross_plane);
			}
			if(str_cmp("", pow->botch_msg_room))
			{
				act_new(pow->botch_msg_room, ch, vict, NULL, TO_NOTVICT, pow->hear_pos, pow->cross_plane);
			}
			break;
	case 0: /* Fail */
			if(str_cmp("", pow->fail_msg_cast))
			{
				act_new(pow->fail_msg_cast, ch, vict, NULL, TO_CHAR, pow->hear_pos, pow->cross_plane);
			}
			if(str_cmp("", pow->fail_msg_vict))
			{
				act_new(pow->fail_msg_vict, ch, vict, NULL, TO_VICT, pow->hear_pos, pow->cross_plane);
			}
			if(str_cmp("", pow->fail_msg_room))
			{
				act_new(pow->fail_msg_room, ch, vict, NULL, TO_NOTVICT, pow->hear_pos, pow->cross_plane);
			}
			break;
	case 1: /* Succeed */
			if(str_cmp("", pow->success_msg_cast))
			{
				act_new(pow->success_msg_cast, ch, vict, NULL, TO_CHAR, pow->hear_pos, pow->cross_plane);
			}
			if(str_cmp("", pow->success_msg_vict))
			{
				act_new(pow->success_msg_vict, ch, vict, NULL, TO_VICT, pow->hear_pos, pow->cross_plane);
			}
			if(str_cmp("", pow->success_msg_room))
			{
				act_new(pow->success_msg_room, ch, vict, NULL, TO_NOTVICT, pow->hear_pos, pow->cross_plane);
			}
			break;
		};
		return;
	}

    /* Otherwise the messages can just follow this. */
	switch ( success ) {
	case 100: /* Initiate */
		if(str_cmp("", pow->init_msg_cast))
			act_new(pow->init_msg_cast, ch, vict, NULL, TO_CHAR, pow->hear_pos, pow->cross_plane);
		if(str_cmp("", pow->init_msg_vict))
			act_new(pow->init_msg_vict, ch, vict, NULL, TO_VICT, pow->hear_pos, pow->cross_plane);
		if(str_cmp("", pow->init_msg_room))
			act_new(pow->init_msg_room, ch, vict, NULL, TO_NOTVICT, pow->hear_pos, pow->cross_plane);
		break;
	case -1: /* Botch */
		if(str_cmp("", pow->botch_msg_cast))
			act_new(pow->botch_msg_cast, ch, vict, NULL, TO_CHAR, pow->hear_pos, pow->cross_plane);
		if(str_cmp("", pow->botch_msg_vict))
			act_new(pow->botch_msg_vict, ch, vict, NULL, TO_VICT, pow->hear_pos, pow->cross_plane);
		if(str_cmp("", pow->botch_msg_room))
			act_new(pow->botch_msg_room, ch, vict, NULL, TO_NOTVICT, pow->hear_pos, pow->cross_plane);
		break;
	case 0: /* Fail */
		if(str_cmp("", pow->fail_msg_cast))
			act_new(pow->fail_msg_cast, ch, vict, NULL, TO_CHAR, pow->hear_pos, pow->cross_plane);
		if(str_cmp("", pow->fail_msg_vict))
			act_new(pow->fail_msg_vict, ch, vict, NULL, TO_VICT, pow->hear_pos, pow->cross_plane);
		if(str_cmp("", pow->fail_msg_room))
			act_new(pow->fail_msg_room, ch, vict, NULL, TO_NOTVICT, pow->hear_pos, pow->cross_plane);
		break;
	case 1: /* Succeed */
		if(str_cmp("", pow->success_msg_cast))
		{
			act_new(pow->success_msg_cast, ch, vict, NULL, TO_CHAR,	pow->hear_pos, pow->cross_plane);
		}
		if(str_cmp("", pow->success_msg_vict))
		{
			act_new(pow->success_msg_vict, ch, vict, NULL, TO_VICT,	pow->hear_pos, pow->cross_plane);
		}
		if(str_cmp("", pow->success_msg_room))
		{
			act_new(pow->success_msg_room, ch, vict, NULL, TO_NOTVICT, pow->hear_pos, pow->cross_plane);
		}
		break;
	};
}

/* Standardized Character Affect Functions */
int effect_health_char (CHAR_DATA *ch, CHAR_DATA *victim, POW *pow, char *arg, int fail)
{
	return 1;
}

int effect_stat_char (CHAR_DATA *ch, CHAR_DATA *victim, POW *pow, char *arg, int fail)
{
	return 1;
}

int effect_tsk_char (CHAR_DATA *ch, CHAR_DATA *victim, POW *pow, char *arg, int fail)
{
	return 1;
}

int effect_virtues_char (CHAR_DATA *ch, CHAR_DATA *victim, POW *pow, char *arg, int fail)
{
	return 1;
}

int effect_GHB_char (CHAR_DATA *ch, CHAR_DATA *victim, POW *pow, char *arg, int fail)
{
	return 1;
}

int effect_RBPG_char (CHAR_DATA *ch, CHAR_DATA *victim, POW *pow, char *arg, int fail)
{
	return 1;
}

int effect_aff_char (CHAR_DATA *ch, CHAR_DATA *victim, POW *pow, char *arg, int fail)
{
	return 1;
}

int effect_off_char (CHAR_DATA *ch, CHAR_DATA *victim, POW *pow, char *arg, int fail)
{
	return 1;
}

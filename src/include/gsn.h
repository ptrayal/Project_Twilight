/***************************************************************************
 * Much of the code is the original work of Peter Fitzgerald who turned    *
 * it over to Brandon Morrison who has adopted and improved the code.      *
 * Copyright (C) 2012 - 2019                                               *
 **************************************************************************/

#if !defined(GSN_H)
	#define GSN_H
  #if defined(IN_DB_C)
		  #if defined(GSN)
				#undef GSN
		  #endif
		  #define GSN(gsn) sh_int gsn
  #else
		  #if defined(GSN)
				#undef GSN
		  #endif
		  #define GSN(gsn) extern sh_int gsn
  #endif

GSN(gsn_abyss_arms);
GSN(gsn_adder_skin);
GSN(gsn_backstab);
GSN(gsn_bash);
GSN(gsn_blade);
GSN(gsn_blindness);
GSN(gsn_blunt);
GSN(gsn_brawl);
GSN(gsn_calm);
GSN(gsn_charm_person);
GSN(gsn_curse);
GSN(gsn_darkness);
GSN(gsn_dirt);
GSN(gsn_disarm);
GSN(gsn_dodge);
GSN(gsn_dread_gaze);
GSN(gsn_envenom);
GSN(gsn_firearm);
GSN(gsn_fly);
GSN(gsn_grapple);
GSN(gsn_haggle);
GSN(gsn_hide);
GSN(gsn_horrid_form);
GSN(gsn_hsenses);
GSN(gsn_illusion);
GSN(gsn_invis);
GSN(gsn_kick);
GSN(gsn_lore);
GSN(gsn_majesty);
GSN(gsn_mask);
GSN(gsn_meditation);
GSN(gsn_melee);
GSN(gsn_parry);
GSN(gsn_peek);
GSN(gsn_pick_lock);
GSN(gsn_plague);
GSN(gsn_poison);
GSN(gsn_rescue);
GSN(gsn_scrolls);
GSN(gsn_serpent_eyes);
GSN(gsn_serpent_tongue);
GSN(gsn_shadow_play);
GSN(gsn_siren_beckon);
GSN(gsn_sleep);
GSN(gsn_sneak);
GSN(gsn_steal);
GSN(gsn_trip);
GSN(gsn_whip);

#endif

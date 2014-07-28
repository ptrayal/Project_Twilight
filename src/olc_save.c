/**************************************************************************
 *  File: olc_save.c                                                       *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  Benefiting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/
/* OLC_SAVE.C
 * This takes care of saving all the .are information.
 * Notes:
 * -If a good syntax checker is used for setting vnum ranges of areas
 *  then it would become possible to just cycle through vnums instead
 *  of using the iHash stuff and checking that the room or reset or
 *  mob etc is part of that area.
 */

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

char * mprog_type_to_name ( int type );

/*
 *  Verbose writes reset data in plain english into the comments
 *  section of the resets.  It makes areas considerably larger but
 *  may aid in debugging.
 */

/* #define VERBOSE */

/*****************************************************************************
 Name:		fix_string
 Purpose:	Returns a string without \r and ~.
 ****************************************************************************/
char *fix_string( const char *str )
{
	static char strfix[15*MSL]={'\0'};
	int i = 0;
	int o = 0;

	if ( str == NULL )
		return '\0';

	for ( o = i = 0; str[i+o] != '\0'; i++ )
	{
		if (str[i+o] == '\r' || str[i+o] == '~')
			o++;
		strfix[i] = str[i+o];
	}
	strfix[i] = '\0';
	return strfix;
}



/*****************************************************************************
 Name:		save_area_list
 Purpose:	Saves the listing of files to be loaded at startup.
 Called by:	do_asave(olc_save.c).
 ****************************************************************************/
void save_area_list()
{
	FILE *fp;
	AREA_DATA *pArea;

	if ( ( fp = fopen( "area.lst", "w" ) ) == NULL )
	{
		log_string(LOG_BUG, "Save_area_list: fopen");
		perror( "area.lst" );
	}
	else
	{
		/*
		 * Add any help files that need to be loaded at
		 * startup to this section.
		 */
		fprintf( fp, "help.are\n"   );
		fprintf( fp, "tips.are\n"   );
		fprintf( fp, "plots.are\n"   );

		for( pArea = area_first; pArea; pArea = pArea->next )
		{
			fprintf( fp, "%s\n", pArea->file_name );
		}

		fprintf( fp, "$\n" );
		fclose( fp );
	}

	return;
}


void save_org_list()
{
	FILE *fp;
	ORG_DATA *pOrg;

	if ( ( fp = fopen( (char *)Format("%s%s", ORG_DIR, ORG_LIST), "w" ) ) == NULL )
	{
		log_string(LOG_BUG, "Save_org_list: fopen");
		perror( "org.lst" );
	}
	else
	{
		for( pOrg = org_list; pOrg; pOrg = pOrg->next )
		{
			fprintf( fp, "%s\n", pOrg->file_name );
		}

		fprintf( fp, "$\n" );
		fclose( fp );
	}

	return;
}


/*
 * ROM OLC
 * Used in save_mobile and save_object below.  Writes
 * flags on the form fread_flag reads.
 * 
 * buf[] must hold at least 32+1 characters.
 *
 * -- Hugin
 */
char *fwrite_flag( long flags, char buf[] )
{
	char offset;
	char *cp;

	buf[0] = '\0';

	if ( flags == 0 )
	{
		strncpy( buf, "0", sizeof(buf) );
		return buf;
	}

	/* 32 -- number of bits in a long */

	for ( offset = 0, cp = buf; offset < 32; offset++ )
		if ( flags & ( (long)1 << offset ) )
		{
			if ( offset <= 'Z' - 'A' )
				*(cp++) = 'A' + offset;
			else
				*(cp++) = 'a' + offset - ( 'Z' - 'A' + 1 );
		}

	*cp = '\0';

	return buf;
}


void save_mobprogs( FILE *fp, AREA_DATA *pArea )
{
	MPROG_CODE *pMprog;
	int i = 0;

	fprintf(fp, "#MOBPROGS\n");

	for( i = pArea->min_vnum; i <= pArea->max_vnum; i++ )
	{
		if ( (pMprog = get_mprog_index(i) ) != NULL)
		{
			fprintf(fp, "#%d\n", i);
			fprintf(fp, "%s~\n", fix_string(pMprog->code));
		}
	}

	fprintf(fp,"#0\n\n");
	return;
}


/*****************************************************************************
 Name:		save_mobile
 Purpose:	Save one mobile to file, new format -- Hugin
 Called by:	save_mobiles (below).
 ****************************************************************************/
void save_mobile( FILE *fp, MOB_INDEX_DATA *pMobIndex )
{
	sh_int race = pMobIndex->race;
	char buf[MSL]={'\0'};
	SCRIPT_DATA *ps;
	MPROG_LIST *pMprog;

	fprintf( fp, "#%d\n",         pMobIndex->vnum );
	fprintf( fp, "%s~\n",         pMobIndex->player_name );
	fprintf( fp, "%s~\n",         pMobIndex->short_descr );
	fprintf( fp, "%s~\n",         fix_string( pMobIndex->long_descr ) );
	fprintf( fp, "%s~\n",         fix_string( pMobIndex->description) );
	fprintf( fp, "%s~\n",         race_table[race].name );
	fprintf( fp, "%s ",	          fwrite_flag( pMobIndex->act,         buf ) );
	fprintf( fp, "%s ",	          fwrite_flag( pMobIndex->affected_by, buf ) );
	fprintf( fp, "%d ",	          pMobIndex->level );
	fprintf( fp, "%d %d %d ",     pMobIndex->hit[DICE_NUMBER], pMobIndex->hit[DICE_TYPE], pMobIndex->hit[DICE_BONUS] );
	fprintf( fp, "%s\n",          attack_table[pMobIndex->dam_type].name );
	fprintf( fp, "%s ",           fwrite_flag( pMobIndex->off_flags,  buf ) );
	fprintf( fp, "%s ",	          fwrite_flag( pMobIndex->imm_flags,  buf ) );
	fprintf( fp, "%s ",           fwrite_flag( pMobIndex->res_flags,  buf ) );
	fprintf( fp, "%s\n",          fwrite_flag( pMobIndex->vuln_flags, buf ) );
	fprintf( fp, "%d %d %d %ld\n", pMobIndex->start_pos, pMobIndex->default_pos, pMobIndex->sex, pMobIndex->wealth );
	fprintf( fp, "%s ",           fwrite_flag( pMobIndex->form,  buf ) );
	fprintf( fp, "%s ",      	  fwrite_flag( pMobIndex->parts, buf ) );

	fprintf( fp, "%s ",           size_table[pMobIndex->size].name );
	fprintf( fp, "%s\n" , ((!IS_NULLSTR(pMobIndex->material)) ? pMobIndex->material : "unknown") );
	fprintf( fp, "A %s\n",	  fwrite_flag( pMobIndex->act2,	buf ) );
	fprintf( fp, "I %s\n",	  fwrite_flag( pMobIndex->affected_by2, buf ) );

	/* @@@@@ */
	if(pMobIndex->triggers) {
		for(ps = pMobIndex->triggers; ps; ps = ps->next_in_event)
			fprintf( fp, "Tr %d %s~ %s~\n", ps->delay, ps->trigger, ps->reaction);
	}

	for (pMprog = pMobIndex->mprogs; pMprog; pMprog = pMprog->next)
	{
		fprintf(fp, "M %s %d %s~\n", mprog_type_to_name(pMprog->trig_type), pMprog->vnum, pMprog->trig_phrase);
	}

	return;
}


/*****************************************************************************
 Name:		save_mobiles
 Purpose:	Save #MOBILES secion of an area file.
 Called by:	save_area(olc_save.c).
 Notes:         Changed for ROM OLC.
 ****************************************************************************/
void save_mobiles( FILE *fp, AREA_DATA *pArea )
{
	int i = 0;
	MOB_INDEX_DATA *pMob;

	fprintf( fp, "#MOBILES\n" );

	for( i = pArea->min_vnum; i <= pArea->max_vnum; i++ )
	{
		if ( (pMob = get_mob_index( i )) )
			save_mobile( fp, pMob );
	}

	fprintf( fp, "#0\n\n\n\n" );
	return;
}





/*****************************************************************************
 Name:		save_object
 Purpose:	Save one object to file.
                new ROM format saving -- Hugin
 Called by:	save_objects (below).
 ****************************************************************************/
void save_object( FILE *fp, OBJ_INDEX_DATA *pObjIndex )
{
    AFFECT_DATA *pAf;
    EXTRA_DESCR_DATA *pEd;
    char buf[MSL]={'\0'};

    fprintf( fp, "#%d\n",    pObjIndex->vnum );
    fprintf( fp, "%s~\n",    pObjIndex->name );
    fprintf( fp, "%s~\n",    pObjIndex->short_descr );
    fprintf( fp, "%s~\n",    fix_string( pObjIndex->description ) );
    fprintf( fp, "%s~\n",    fix_string( pObjIndex->full_desc ) );
    fprintf( fp, "%s~\n",    pObjIndex->material );
    fprintf( fp, "%d ",      pObjIndex->item_type );
    fprintf( fp, "%s ",      fwrite_flag( pObjIndex->extra_flags, buf ) );
    fprintf( fp, "%s\n",     fwrite_flag( pObjIndex->wear_flags,  buf ) );

/*
 *  Using fwrite_flag to write most values gives a strange
 *  looking area file, consider making a case for each
 *  item type later.
 */

    switch ( pObjIndex->item_type )
    {
        default:
	    fprintf( fp, "%s ",  fwrite_flag( pObjIndex->value[0], buf ) );
	    fprintf( fp, "%s ",  fwrite_flag( pObjIndex->value[1], buf ) );
	    fprintf( fp, "%s ",  fwrite_flag( pObjIndex->value[2], buf ) );
	    fprintf( fp, "%s ",  fwrite_flag( pObjIndex->value[3], buf ) );
	    fprintf( fp, "%s\n", fwrite_flag( pObjIndex->value[4], buf ) );
	    fprintf( fp, "%s\n", fwrite_flag( pObjIndex->value[5], buf ) );
	    break;

        case ITEM_LIGHT:
	    fprintf( fp, "0 0 %d 0 0 0\n",
		     pObjIndex->value[2] < 1 ? 999  /* infinite */
		     : pObjIndex->value[2] );
	    break;

        case ITEM_MONEY:
            fprintf( fp, "%d %d 0 0 0 0\n",
                     pObjIndex->value[0],
                     pObjIndex->value[1]);
            break;

        case ITEM_DRINK_CON:
            fprintf( fp, "%d %d '%s' %d 0 0\n",
                     pObjIndex->value[0],
                     pObjIndex->value[1],
                     liq_table[pObjIndex->value[2]].liq_name,
		     pObjIndex->value[3]);
            break;

	case ITEM_FOUNTAIN:
	    fprintf( fp, "%d %d '%s' 0 0 0\n",
	             pObjIndex->value[0],
	             pObjIndex->value[1],
	             liq_table[pObjIndex->value[2]].liq_name);
	    break;
	    
        case ITEM_CONTAINER:
            fprintf( fp, "%d %s %d %d %d 0\n",
                     pObjIndex->value[0],
                     fwrite_flag( pObjIndex->value[1], buf ),
                     pObjIndex->value[2],
                     pObjIndex->value[3],
                     pObjIndex->value[4]);
            break;
            
        case ITEM_FOOD:
            fprintf( fp, "%d %d 0 %s 0 0\n",
                     pObjIndex->value[0],
                     pObjIndex->value[1],
                     fwrite_flag( pObjIndex->value[3], buf ) );
            break;
            
        case ITEM_PORTAL:
            fprintf( fp, "%d %s %s %d 0 0\n",
                     pObjIndex->value[0],
                     fwrite_flag( pObjIndex->value[1], buf ),
                     fwrite_flag( pObjIndex->value[2], buf ),
                     pObjIndex->value[3]);
            break;
            
        case ITEM_FURNITURE:
            fprintf( fp, "%d %d %s %d %d 0\n",
                     pObjIndex->value[0],
                     pObjIndex->value[1],
                     fwrite_flag( pObjIndex->value[2], buf),
                     pObjIndex->value[3],
                     pObjIndex->value[4]);
            break;
            
        case ITEM_WEAPON:
            fprintf( fp, "%d %d %d %s %s %d\n",
                     pObjIndex->value[0],
                     pObjIndex->value[1],
                     pObjIndex->value[2],
                     attack_table[pObjIndex->value[3]].name,
                     fwrite_flag( pObjIndex->value[4], buf ),
                     pObjIndex->value[5] );
            break;
            
        case ITEM_ARMOR:
            fprintf( fp, "%d %d %d %d %d 0\n",
                     pObjIndex->value[0],
                     pObjIndex->value[1],
                     pObjIndex->value[2],
                     pObjIndex->value[3],
                     pObjIndex->value[4]);
            break;
            
        case ITEM_PILL:
        case ITEM_POTION:
        case ITEM_SCROLL:
	    fprintf( fp, "%d '%s' '%s' '%s' '%s' 0\n",
		     pObjIndex->value[0] > 0 ? /* no negative numbers */
		     pObjIndex->value[0]
		     : 0,
		     pObjIndex->value[1] != -1 ?
		     skill_table[pObjIndex->value[1]].name
		     : 0,
		     pObjIndex->value[2] != -1 ?
		     skill_table[pObjIndex->value[2]].name
		     : 0,
		     pObjIndex->value[3] != -1 ?
		     skill_table[pObjIndex->value[3]].name
		     : 0,
		     pObjIndex->value[4] != -1 ?
		     skill_table[pObjIndex->value[4]].name
		     : 0);
	    break;
    }

    fprintf( fp, "%d ", pObjIndex->weight );
    fprintf( fp, "%d ", pObjIndex->cost );

    fprintf( fp, "%d\n", pObjIndex->condition );

    for( pAf = pObjIndex->affected; pAf; pAf = pAf->next )
    {
        fprintf( fp, "A\n%d %d\n",  pAf->location, pAf->modifier );
    }

    if(!IS_NULLSTR(pObjIndex->to_user_none))
    	fprintf( fp, "E\n--TUNONE~\n%s~\n", fix_string( pObjIndex->to_user_none ) );
    if(!IS_NULLSTR(pObjIndex->to_user_self))
    	fprintf( fp, "E\n--TUSELF~\n%s~\n", fix_string( pObjIndex->to_user_self ) );
    if(!IS_NULLSTR(pObjIndex->to_user_other))
    	fprintf( fp, "E\n--TUOTH~\n%s~\n", fix_string( pObjIndex->to_user_other ) );
    if(!IS_NULLSTR(pObjIndex->to_room_none))
    	fprintf( fp, "E\n--TRNONE~\n%s~\n", fix_string( pObjIndex->to_room_none ) );
    if(!IS_NULLSTR(pObjIndex->to_room_self))
    	fprintf( fp, "E\n--TRSELF~\n%s~\n", fix_string( pObjIndex->to_room_self ) );
    if(!IS_NULLSTR(pObjIndex->to_room_other))
    	fprintf( fp, "E\n--TROTH~\n%s~\n", fix_string( pObjIndex->to_room_other ) );
    if(!IS_NULLSTR(pObjIndex->to_vict_other))
    	fprintf( fp, "E\n--TVOTH~\n%s~\n", fix_string( pObjIndex->to_vict_other ) );
    if(!IS_NULLSTR(pObjIndex->to_room_used))
    	fprintf( fp, "E\n--TRUSE~\n%s~\n", fix_string( pObjIndex->to_room_used ) );
    if(!IS_NULLSTR(pObjIndex->to_user_used))
    	fprintf( fp, "E\n--TUUSE~\n%s~\n", fix_string( pObjIndex->to_user_used ) );
    if(!IS_NULLSTR(pObjIndex->company))
    	fprintf( fp, "E\n--COMPANY~\n%s~\n", fix_string( pObjIndex->company ) );

    if(pObjIndex->uses >= -2)
    	fprintf( fp, "E\n--USES~\n%d~\n", pObjIndex->uses );

    for( pEd = pObjIndex->extra_descr; pEd; pEd = pEd->next )
    {
    	fprintf( fp, "E\n%s~\n%s~\n", pEd->keyword, fix_string( pEd->description ) );
    }

    if(pObjIndex->fetish_level > 0)
    {
    	fprintf( fp, "S\n%s %d %d\n",
    			fwrite_flag(pObjIndex->fetish_flags, buf), pObjIndex->fetish_level, pObjIndex->fetish_target );
    }

   	fprintf( fp, "Q %d\n", pObjIndex->quality);

    return;
}
 



/*****************************************************************************
 Name:		save_objects
 Purpose:	Save #OBJECTS section of an area file.
 Called by:	save_area(olc_save.c).
 Notes:         Changed for ROM OLC.
 ****************************************************************************/
void save_objects( FILE *fp, AREA_DATA *pArea )
{
	int i = 0;
	OBJ_INDEX_DATA *pObj;

	fprintf( fp, "#OBJECTS\n" );

	for( i = pArea->min_vnum; i <= pArea->max_vnum; i++ )
	{
		if ( (pObj = get_obj_index( i )) )
			save_object( fp, pObj );
	}

	fprintf( fp, "#0\n\n\n\n" );
	return;
}
 




/*****************************************************************************
 Name:		save_rooms
 Purpose:	Save #ROOMS section of an area file.
 Called by:	save_area(olc_save.c).
 ****************************************************************************/
void save_rooms( FILE *fp, AREA_DATA *pArea )
{
	ROOM_INDEX_DATA *pRoomIndex;
	EXTRA_DESCR_DATA *pEd;
	EXIT_DATA *pExit;
	int iHash = 0;
	int door = 0;
	char buf[MSL]={'\0'};

	fprintf( fp, "#ROOMS\n" );
	for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	{
		for( pRoomIndex = room_index_hash[iHash]; pRoomIndex; pRoomIndex = pRoomIndex->next )
		{
			if ( pRoomIndex->area == pArea )
			{
				fprintf( fp, "#%d\n",		pRoomIndex->vnum );
				fprintf( fp, "%s~\n",		pRoomIndex->name );
				fprintf( fp, "%s~\n",		fix_string( pRoomIndex->description ) );
				fprintf( fp, "0 " );
				fprintf( fp, "%d ",		pRoomIndex->room_flags );
				fprintf( fp, "%d\n",		pRoomIndex->sector_type );

				if(pRoomIndex->car > 0 && get_room_index(pRoomIndex->car))
				{
					fprintf( fp, "V\n%d %d\n", pRoomIndex->car,
							get_stop(get_room_index(pRoomIndex->car), pRoomIndex) );
				}
				fprintf( fp, "E\nXNUMB1~\n%s~\n", pRoomIndex->uname ? pRoomIndex->uname : "");
				fprintf( fp, "E\nXDUMB1~\n%s~\n", fix_string(pRoomIndex->udescription) );
				fprintf( fp, "E\nXNDREAM~\n%s~\n", pRoomIndex->dname ? pRoomIndex->dname : "" );
				fprintf( fp, "E\nXDDREAM~\n%s~\n", fix_string(pRoomIndex->ddescription) );
				for ( pEd = pRoomIndex->extra_descr; pEd;
						pEd = pEd->next )
				{
					fprintf( fp, "E\n%s~\n%s~\n", pEd->keyword,
							fix_string( pEd->description ) );
				}
				for( door = 0; door < MAX_DIR; door++ )	/* I hate this! */
				{
					pExit = pRoomIndex->exit[door];
					if(pExit)
					{
						fprintf( fp, "D%d\n",      pExit->orig_door );
						fprintf( fp, "%s~\n",      fix_string(pExit->description ? pExit->description : "none" ) );
						fprintf( fp, "%s~\n",      pExit->keyword );
						fprintf( fp, "%s %d %d\n", fwrite_flag( pExit->exit_info, buf ),
								pExit->key,
								pExit->u1.to_room == NULL ? 0 : pExit->u1.to_room->vnum );
						if(pExit->jumpto.to_room)
						{
							fprintf( fp, "J%d %d\n", pExit->orig_door,
									pExit->jumpto.to_room == NULL ? 0 : pExit->jumpto.to_room->vnum);
						}
					}
				}
				if (pRoomIndex->mana_rate != 100 || pRoomIndex->heal_rate != 100)
					fprintf ( fp, "M %d H %d\n",pRoomIndex->mana_rate,
							pRoomIndex->heal_rate);
				if (pRoomIndex->clan > 0)
					fprintf ( fp, "C %s~\n" , clan_table[pRoomIndex->clan].name );
				if (!IS_NULLSTR(pRoomIndex->owner))
					fprintf ( fp, "O %s~\n" , pRoomIndex->owner );

				fprintf( fp, "S\n" );
			}
		}
	}
	fprintf( fp, "#0\n\n\n\n" );
	return;
}

/*
 * This function is obsolete.  It it not needed but has been left here
 * for historical reasons.  It is used currently for the same reason.
 *
 * I don't think it's obsolete in ROM -- Hugin.
 */
void save_door_resets( FILE *fp, AREA_DATA *pArea )
{
    int iHash = 0;
    ROOM_INDEX_DATA *pRoomIndex;
    EXIT_DATA *pExit;
    int door = 0;

    for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
        for( pRoomIndex = room_index_hash[iHash]; pRoomIndex; pRoomIndex = pRoomIndex->next )
        {
            if ( pRoomIndex->area == pArea )
            {
                for( door = 0; door < MAX_DIR; door++ )
                {
                    if ( ( pExit = pRoomIndex->exit[door] )
                          && pExit->u1.to_room 
                          && ( IS_SET( pExit->rs_flags, EX_CLOSED )
                          || IS_SET( pExit->rs_flags, EX_LOCKED ) ) )
#if defined( VERBOSE )
			fprintf( fp, "D 0 %d %d %d The %s door of %s is %s\n", 
				pRoomIndex->vnum,
				pExit->orig_door,
				pExit->rs_flags,
				dir_name[ pExit->orig_door ],
				pRoomIndex->name,
				IS_SET( pExit->rs_flags, EX_LOCKED) ? "closed and locked"
				    : "closed" );
#endif
#if !defined( VERBOSE )
			fprintf( fp, "D 0 %d %d %d\n", 
				pRoomIndex->vnum,
				pExit->orig_door,
				pExit->rs_flags );
#endif
		}
	    }
	}
    }
    return;
}




/*****************************************************************************
 Name:		save_resets
 Purpose:	Saves the #RESETS section of an area file.
 Called by:	save_area(olc_save.c)
 ****************************************************************************/
void save_resets( FILE *fp, AREA_DATA *pArea )
{
	RESET_DATA *pReset;
	MOB_INDEX_DATA *pLastMob = NULL;
	OBJ_INDEX_DATA *pLastObj;
	ROOM_INDEX_DATA *pRoom;
	int iHash = 0;

	fprintf( fp, "#RESETS\n" );

	save_door_resets( fp, pArea );

	for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	{
		for( pRoom = room_index_hash[iHash]; pRoom; pRoom = pRoom->next )
		{
			if ( pRoom->area == pArea )
			{
				for ( pReset = pRoom->reset_first; pReset; pReset = pReset->next )
				{
					switch ( pReset->command )
					{
					default:
						log_string(LOG_BUG, Format("Save_resets: bad command %c.", pReset->command ));
						break;

#if defined( VERBOSE )
					case 'M':
						pLastMob = get_mob_index( pReset->arg1 );
						fprintf( fp, "M 1 %d %d %d %d Load %s\n",
								pReset->arg1,
								pReset->arg2,
								pReset->arg3,
								pReset->arg4,
								pLastMob->short_descr );
						break;

					case 'O':
						pLastObj = get_obj_index( pReset->arg1 );
						pRoom = get_room_index( pReset->arg3 );
						fprintf( fp, "O 0 %d 0 %d %s loaded to %s\n",
								pReset->arg1,
								pReset->arg3,
								capitalize(pLastObj->short_descr),
								pRoom->name );
						break;

					case 'P':
						pLastObj = get_obj_index( pReset->arg1 );
						fprintf( fp, "P 0 %d %d %d %d %s put inside %s\n",
								pReset->arg1,
								pReset->arg2,
								pReset->arg3,
								pReset->arg4,
								capitalize(get_obj_index( pReset->arg1 )->short_descr),
								pLastObj->short_descr );
						break;

					case 'G':
						fprintf( fp, "G 0 %d 0 %s is given to %s\n",
								pReset->arg1,
								capitalize(get_obj_index( pReset->arg1 )->short_descr),
								pLastMob ? pLastMob->short_descr : "!NO_MOB!" );
						if ( !pLastMob )
						{
							log_string(LOG_BUG, Format("Save_resets: !NO_MOB! in [%s]", pArea->file_name));
						}
						break;

					case 'E':
						fprintf( fp, "E 0 %d 0 %d %s is loaded %s of %s\n",
								pReset->arg1,
								pReset->arg3,
								capitalize(get_obj_index( pReset->arg1 )->short_descr),
								flag_string( wear_loc_strings, pReset->arg3 ),
								pLastMob ? pLastMob->short_descr : "!NO_MOB!" );
						if ( !pLastMob )
						{
							log_string(LOG_BUG, Format("Save_resets: !NO_MOB! in [%s]", pArea->file_name));
						}
						break;

					case 'D':
						break;

					case 'R':
						pRoom = get_room_index( pReset->arg1 );
						fprintf( fp, "R 0 %d %d Randomize %s\n",
								pReset->arg1,
								pReset->arg2,
								pRoom->name );
						break;
					}
#endif
#if !defined( VERBOSE )
					case 'M':
						pLastMob = get_mob_index( pReset->arg1 );
						fprintf( fp, "M 1 %d %d %d %d\n",
								pReset->arg1,
								pReset->arg2,
								pReset->arg3,
								pReset->arg4 );
						break;

					case 'O':
						pLastObj = get_obj_index( pReset->arg1 );
						pRoom = get_room_index( pReset->arg3 );
						fprintf( fp, "O 0 %d 0 %d\n",
								pReset->arg1,
								pReset->arg3 );
						break;

					case 'P':
						pLastObj = get_obj_index( pReset->arg1 );
						fprintf( fp, "P 0 %d %d %d %d\n",
								pReset->arg1,
								pReset->arg2,
								pReset->arg3,
								pReset->arg4 );
						break;

					case 'G':
						fprintf( fp, "G 0 %d 0\n", pReset->arg1 );
						if ( !pLastMob )
						{
							log_string(LOG_BUG, Format("Save_resets: !NO_MOB! in [%s]", pArea->file_name));
						}
						break;

					case 'E':
						fprintf( fp, "E 0 %d 0 %d\n",
								pReset->arg1,
								pReset->arg3 );
						if ( !pLastMob )
						{
							log_string(LOG_BUG, Format("Save_resets: !NO_MOB! in [%s]", pArea->file_name));
						}
						break;

					case 'D':
						break;

					case 'R':
						pRoom = get_room_index( pReset->arg1 );
						fprintf( fp, "R 0 %d %d\n",
								pReset->arg1,
								pReset->arg2 );
						break;
				}
#endif
			}
		}	/* End if correct area */
	}	/* End for pRoom */
}	/* End for iHash */
fprintf( fp, "S\n\n\n\n" );
return;
}



/*****************************************************************************
 Name:		save_shops
 Purpose:	Saves the #SHOPS section of an area file.
 Called by:	save_area(olc_save.c)
 ****************************************************************************/
void save_shops( FILE *fp, AREA_DATA *pArea )
{
	SHOP_DATA *pShopIndex;
	MOB_INDEX_DATA *pMobIndex;
	char buf[MSL]={'\0'};
	int iTrade = 0;
	int iHash = 0;

	fprintf( fp, "#SHOPS\n" );

	for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	{
		for( pMobIndex = mob_index_hash[iHash]; pMobIndex; pMobIndex = pMobIndex->next )
		{
			if ( pMobIndex && pMobIndex->area == pArea && pMobIndex->pShop )
			{
				pShopIndex = pMobIndex->pShop;

				fprintf( fp, "%d ", pShopIndex->keeper );
				for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ )
				{
					if ( pShopIndex->buy_type[iTrade] != 0 )
					{
						fprintf( fp, "%d ", pShopIndex->buy_type[iTrade] );
					}
					else
						fprintf( fp, "0 ");
				}
				fprintf( fp, "%d %d ", pShopIndex->profit_buy, pShopIndex->profit_sell );
				fprintf( fp, "%d %d ", pShopIndex->open_hour, pShopIndex->close_hour );
				fprintf( fp, "%s\n", fwrite_flag(pShopIndex->raw_materials, buf));
			}
		}
	}

	fprintf( fp, "0\n\n\n\n" );
	return;
}




/*****************************************************************************
 Name:		save_area
 Purpose:	Save an area, note that this format is new.
 Called by:	do_asave(olc_save.c).
 ****************************************************************************/
void save_area( AREA_DATA *pArea )
{
	FILE *fp;
	char buf[MSL]={'\0'};

	closeReserve();
	if ( !( fp = fopen( pArea->file_name, "w" ) ) )
	{
		log_string(LOG_BUG, "Open_area: fopen");
		perror( pArea->file_name );
	}

	fprintf( fp, "#AREADATA\n" );
	/*    fprintf( fp, "%s~\n", pArea->file_name ); */
	fprintf( fp, "Name %s~\n",		pArea->name );
	fprintf( fp, "Builders %s~\n",	fix_string( pArea->builders ) );
	fprintf( fp, "VNUMs %d %d\n",	pArea->min_vnum, pArea->max_vnum );
	fprintf( fp, "Credits %s~\n",	pArea->credits );
	fprintf( fp, "Security %d\n",	pArea->security );

	if(IS_SET(pArea->area_flags, A)) REMOVE_BIT(pArea->area_flags, A);
	fprintf( fp, "Flags %s\n",		fwrite_flag(pArea->area_flags, buf));
	fprintf( fp, "Price %d\n",		pArea->pricemod );
	/*    fprintf( fp, "Recall      %d\n",         pArea->recall );  ROM OLC */
	fprintf( fp, "End\n\n\n\n" );

	save_mobiles( fp, pArea );
	save_objects( fp, pArea );
	save_rooms( fp, pArea );
	save_resets( fp, pArea );
	save_shops( fp, pArea );
	save_mobprogs( fp, pArea );

	fprintf( fp, "#$\n" );

	fclose( fp );
	openReserve();
	system((char *)Format("cp %s bak/", pArea->file_name));
	return;
}


/*****************************************************************************
 Name:		do_asave
 Purpose:	Entry point for saving area data.
 Called by:	interpreter(interp.c)
 ****************************************************************************/
void do_asave( CHAR_DATA *ch, char *argument )
{
	char arg1 [MAX_INPUT_LENGTH]={'\0'};
	AREA_DATA *pArea;
	FILE *fp;
	int value = 0;

	fp = NULL;

	if ( !ch )       /* Do an autosave */
	{
		save_area_list();
		for( pArea = area_first; pArea; pArea = pArea->next )
		{
			save_area( pArea );
			REMOVE_BIT( pArea->area_flags, AREA_CHANGED );
		}
		return;
	}
	smash_tilde( argument );
	strncpy( arg1, argument, sizeof(arg1) );
	if ( IS_NULLSTR(arg1) )
	{
		send_to_char( "Syntax:\n\r", ch );
		send_to_char( "  asave <vnum>   - saves a particular area\n\r",	ch );
		send_to_char( "  asave list     - saves the area.lst file\n\r",	ch );
		send_to_char( "  asave area     - saves the area being edited\n\r",	ch );
		send_to_char( "  asave changed  - saves all changed zones\n\r",	ch );
		send_to_char( "  asave world    - saves the world! (db dump)\n\r",	ch );
		send_to_char( "\n\r", ch );
		return;
	}

	/* Snarf the value (which need not be numeric). */
	value = atoi( arg1 );
	if ( !( pArea = get_area_data( value ) ) && is_number( arg1 ) )
	{
		send_to_char( "That area does not exist.\n\r", ch );
		return;
	}
	/* Save area of given vnum. */
	/* ------------------------ */

	if ( is_number( arg1 ) )
	{
		if ( !IS_BUILDER( ch, pArea ) )
		{
			send_to_char( "You are not a builder for this area.\n\r", ch );
			return;
		}
		save_area_list();
		save_area( pArea );
		return;
	}
	/* Save the world, only authorized areas. */
	/* -------------------------------------- */

	if ( !str_cmp( "world", arg1 ) )
	{
		save_area_list();
		for( pArea = area_first; pArea; pArea = pArea->next )
		{
			/* Builder must be assigned this area. */
			if ( !IS_BUILDER( ch, pArea ) )
				continue;

			save_area( pArea );
			REMOVE_BIT( pArea->area_flags, AREA_CHANGED );
		}
		send_to_char( "You saved the world.\n\r", ch );
		/*	send_to_all_char( "Database saved.\n\r" );                 ROM OLC */
		return;
	}

	/* Save changed areas, only authorized areas. */
	/* ------------------------------------------ */

	if ( !str_cmp( "changed", arg1 ) )
	{
		char buf[MSL]={'\0'};

		save_area_list();

		send_to_char( "Saved zones:\n\r", ch );
		send_to_char( "None.\n\r", ch );

		for( pArea = area_first; pArea; pArea = pArea->next )
		{
			/* Builder must be assigned this area. */
			if ( !IS_BUILDER( ch, pArea ) )
				continue;

			/* Save changed areas. */
			if ( IS_SET(pArea->area_flags, AREA_CHANGED) )
			{
				save_area( pArea );
				send_to_char( Format("%24s - '%s'\n\r", pArea->name, pArea->file_name), ch );
				REMOVE_BIT( pArea->area_flags, AREA_CHANGED );
			}

			/* Save added areas. */
			if ( IS_SET(pArea->area_flags, AREA_ADDED)
					&& !IS_SET(pArea->area_flags, AREA_CHANGED) )
			{
				save_area( pArea );
				send_to_char( Format("%24s - '%s'\n\r", pArea->name, pArea->file_name), ch );
				REMOVE_BIT( pArea->area_flags, AREA_ADDED );
			}
		}
		if ( !str_cmp( buf, "None.\n\r" ) )
			send_to_char( buf, ch );
		return;
	}

	/* Save the area.lst file. */
	/* ----------------------- */
	if ( !str_cmp( arg1, "list" ) )
	{
		save_area_list();
		return;
	}

	/* Save area being edited, if authorized. */
	/* -------------------------------------- */
	if ( !str_cmp( arg1, "area" ) )
	{
		/* Is character currently editing. */
		if ( ch->desc->editor == 0 )
		{
			send_to_char( "You are not editing an area, "
					"therefore an area vnum is required.\n\r", ch );
			return;
		}

		/* Find the area to save. */
		switch (ch->desc->editor)
		{
		case ED_AREA:
			pArea = (AREA_DATA *)ch->desc->pEdit;
			break;
		case ED_ROOM:
			pArea = ch->in_room->area;
			break;
		case ED_OBJECT:
			pArea = ( (OBJ_INDEX_DATA *)ch->desc->pEdit )->area;
			break;
		case ED_MOBILE:
			pArea = ( (MOB_INDEX_DATA *)ch->desc->pEdit )->area;
			break;
		default:
			pArea = ch->in_room->area;
			break;
		}

		if ( !IS_BUILDER( ch, pArea ) )
		{
			send_to_char( "You are not a builder for this area.\n\r", ch );
			return;
		}

		save_area_list();
		save_area( pArea );
		REMOVE_BIT( pArea->area_flags, AREA_CHANGED );
		send_to_char( "Area saved.\n\r", ch );
		return;
	}

	/* Show correct syntax. */
	/* -------------------- */
	do_asave( ch, "" );
	return;
}

void save_scripts(FILE *fp, EVENT_DATA *event)
{
	SCRIPT_DATA *script;

	if(event->script_list == NULL)
	{
		return;
	}

	for(script = event->script_list; script; script = script->next_in_event)
	{
		fprintf(fp, "S%d\n", script->vnum);
		fprintf(fp, "%s~\n", script->author);
		fprintf(fp, "%d\n", script->actor > 0 ? script->actor : 0);
		fprintf(fp, "%d\n", script->delay);
		fprintf(fp, "%d\n", script->first_script);
		fprintf(fp, "%s~\n", script->trigger);
		fprintf(fp, "%s~\n", script->reaction);
	}
	fprintf(fp, "S0");
}

void save_actors(FILE *fp, EVENT_DATA *event)
{
	ACTOR_DATA *actor;

	for(actor = event->actors; actor; actor = actor->next)
	{
		fprintf(fp, "A %d\n", actor->mob);
	}
	if(event->actors)
	{
		fprintf(fp, "A 0\n");
	}
	fprintf(fp, "\n");
}

void save_events(FILE *fp, PLOT_DATA *plot)
{
	EVENT_DATA *event;
	char buf[MSL]={'\0'};

	if(plot->event_list == NULL)
	{
		return;
	}

	for(event = plot->event_list; event; event = event->next_in_plot)
	{
		fprintf(fp, "E%d\n", event->vnum);
		fprintf(fp, "%s~\n", event->title);
		fprintf(fp, "%s~\n", event->author);
		fprintf(fp, "%s\n", fwrite_flag(event->races, buf) );
		fprintf(fp, "%d\n", event->loc);
		save_actors(fp, event);
		save_scripts(fp, event);
		fprintf(fp, "E0\n");
	}
}

void save_plots(FILE *fp)
{
	PLOT_DATA *plot;
	char buf[MSL]={'\0'};

	for(plot = plot_list; plot; plot = plot->next)
	{
		fprintf(fp, "#%d\n", plot->vnum);
		fprintf(fp, "%s~\n", plot->title);
		fprintf(fp, "%s~\n", plot->author);
		fprintf(fp, "%s\n", fwrite_flag(plot->races, buf) );
		save_events(fp, plot);
	}
	fprintf(fp, "#0\n");
}

void do_save_plots (CHAR_DATA *ch, char *argument)
{
	FILE *fp;

	closeReserve();
	if ( !( fp = fopen( "plots.are", "w" ) ) )
	{
		log_string(LOG_BUG, "Save_plots: fopen");
		perror( "plots.are" );
	}

	fprintf(fp, "#PLOTS\n");
	save_plots(fp);

	fprintf( fp, "#$\n" );

	fclose( fp );
	openReserve();
	return;
}

void save_reactions(FILE *fp, REACT *trig)
{
	REACT_DATA *react;

	if(trig->react == NULL)
	{
		return;
	}

	for(react = trig->react; react; react = react->next)
	{
		fprintf(fp, "R %d %s~\n", react->priority, react->reaction);
	}
}

void save_triggers(FILE *fp, PERSONA *persona)
{
	REACT *trig;
	int i = 0;

	for(i = 0; i < MAX_ATTITUDE; i++)
	{
		if(persona->matrix[i] == NULL)
		{
			continue;
		}

		for(trig = persona->matrix[i]; trig; trig = trig->next_in_matrix_loc)
		{
			fprintf(fp, "T %d %s~\n", i, trig->trig);
			save_reactions(fp, trig);
			fprintf(fp, "End\n");
		}
	}
	fprintf(fp, "A0\n");
}

void save_personas(FILE *fp)
{
	PERSONA *persona;

	for(persona = persona_first; persona; persona = persona->next)
	{
		fprintf(fp, "#%d\n", persona->vnum);
		fprintf(fp, "%s~\n", persona->name);
		save_triggers(fp, persona);
		fprintf(fp, "#0\n");
	}
}

void do_save_ai (CHAR_DATA *ch, char *argument)
{
	FILE *fp;

	closeReserve();
	if ( !( fp = fopen( "ai.are", "w" ) ) )
	{
		log_string(LOG_BUG, "Save_AI: fopen");
		perror( "ai.are" );
	}

	fprintf(fp, "#PERSONALITIES\n");
	save_personas(fp);

	fprintf( fp, "#$\n" );

	fclose( fp );
	openReserve();
	return;
}


void do_hsave (CHAR_DATA *ch, char *argument)
{
	FILE *fp;
	HELP_DATA *help;

	closeReserve();
	if ( !( fp = fopen( "help.are", "w" ) ) )
	{
		log_string(LOG_BUG, "HSave: fopen");
		perror( "help.are" );
	}

	fprintf(fp, "#HELPS\n");

	for(help = help_list; help; help = help->next)
	{
		fprintf(fp, "%d %s~\n", help->level, help->keyword);
		fprintf(fp, "%s~\n%s~\n", help->races, help->clans);

		if(!IS_NULLSTR(help->topic))
			fprintf(fp, "Topic %s~\n", help->topic);
		if(!IS_NULLSTR(help->quote))
			fprintf(fp, "Quote %s~\n", help->quote);
		if(!IS_NULLSTR(help->syntax))
			fprintf(fp, "Syntax %s~\n", help->syntax);
		if(!IS_NULLSTR(help->description))
			fprintf(fp, "Desc %s~\n", help->description);
		if(!IS_NULLSTR(help->see_also))
			fprintf(fp, "See %s~\n", help->see_also);
		if(!IS_NULLSTR(help->website))
			fprintf(fp, "Web %s~\n", help->website);
		if(!IS_NULLSTR(help->unformatted))
			fprintf(fp, "Unformatted %s~\n", help->unformatted);
		fprintf(fp, "End\n");
	}

	fprintf( fp, "0 $~\n" );
	fprintf( fp, "#$\n" );

	fclose( fp );
	openReserve();

	send_to_char("Helps saved.\n\r", ch);
	return;
}


void do_tsave (CHAR_DATA *ch, char *argument)
{
	FILE *fp;
	HELP_DATA *help;

	closeReserve();
	if ( !( fp = fopen( "tips.are", "w" ) ) )
	{
		log_string(LOG_BUG, "TSave: fopen");
		perror( "tips.are" );
	}

	fprintf(fp, "#TIPS\n");

	for(help = tip_list; help; help = help->next)
	{
		fprintf(fp, "%d %s~\n", help->level, help->keyword);
		fprintf(fp, "%s~\n%s~\n", help->races, help->clans);

		if(!IS_NULLSTR(help->topic))
			fprintf(fp, "Topic %s~\n", help->topic);
		if(!IS_NULLSTR(help->quote))
			fprintf(fp, "Quote %s~\n", help->quote);
		if(!IS_NULLSTR(help->syntax))
			fprintf(fp, "Syntax %s~\n", help->syntax);
		if(!IS_NULLSTR(help->description))
			fprintf(fp, "Desc %s~\n", help->description);
		if(!IS_NULLSTR(help->see_also))
			fprintf(fp, "See %s~\n", help->see_also);
		if(!IS_NULLSTR(help->website))
			fprintf(fp, "Web %s~\n", help->website);
		if(!IS_NULLSTR(help->unformatted))
			fprintf(fp, "Unformatted %s~\n", help->unformatted);
	}

	fprintf( fp, "0 $~\n" );
	fprintf( fp, "#$\n" );

	fclose( fp );
	openReserve();

	send_to_char("Tips saved.\n\r", ch);
	return;
}

void fwrite_votes ()
{
	FILE *fp;
	int i = 0,j = 0;
	PACK_DATA *pack;

	closeReserve();
	if ( !( fp = fopen( "votes.txt", "w" ) ) )
	{
		log_string(LOG_BUG, "Write_votes: fopen");
		perror( "votes.txt" );
	}

	for(i=0; i<MAX_OFFICES; i++)
	{
		for(j=0; j<MAX_NOMINEES; j++)
			fprintf(fp, "#%c %s %d\n",
					office_table[i].letter,
					vote_tally[i][j].name,
					vote_tally[i][j].votes);
	}

	fprintf( fp, "#I mayor %s~\n", incumbent_mayor );
	for(i=0; i<MAX_JUDGES; i++)
	{
		fprintf( fp, "#I judge %s~\n", incumbent_judge[i] );
	}
	for(i=0; i<MAX_ALDERMEN; i++)
	{
		fprintf( fp, "#I alderman %s~\n", incumbent_alder[i] );
	}
	fprintf( fp, "#I pchief %s~\n", incumbent_pchief );

	fprintf( fp, "#R %ld %ld\n", last_election_result, next_election_result);
	fprintf( fp, "#T %d\n", tax_rate);

	for(pack = pack_list; pack; pack = pack->next)
	{
		fprintf( fp, "#P %s~ %s\n", pack->name, print_flags(pack->totem));
	}

	fprintf( fp, "#$\n" );

	fclose( fp );
	openReserve();
	return;
}

void fwrite_org (ORG_DATA *org)
{
	FILE *fp;
	ORGMEM_DATA *mem;
	int i = 0;

	closeReserve();
	if ( !( fp = fopen( (char *)Format("%s%s", ORG_DIR, org->file_name), "w" ) ) )
	{
		log_string(LOG_BUG, "Write_org: fopen");
		perror( (char *)Format("%s%s", ORG_DIR, org->file_name) );
	}

	fprintf( fp, "#ORG\n%s~ %s~\n", org->name, org->who_name );
	fprintf( fp, "%s~\n", org->leader );
	fprintf( fp, "%ld %d %ld\n", org->step_point, org->type, org->funds );
	fprintf( fp, "%s~\n", org->applicants );
	fprintf( fp, "%s~\n", org->races );
	fprintf( fp, "%s\n\n", print_flags(org->default_auths) );

	for(i = 0; i<5; i++)
	{
		fprintf( fp, "%s~\n", org->commands[i] );
	}

	fprintf( fp, "\n" );

	for(i = 0; i<6; i++)
	{
		fprintf( fp, "%s~\n", org->title[i] );
	}

	fprintf( fp, "\n" );

	fprintf( fp, "#MEMBERS\n" );
	for(mem = org->members; mem; mem = mem->next)
	{
		fprintf( fp, "#%s~ %d %s %d\n", mem->name, mem->status,
				print_flags(mem->auth_flags), mem->status_votes);
	}

	fprintf( fp, "#$\n#$\n" );

	fclose( fp );
	openReserve();
	return;
}


void do_hsave2 (CHAR_DATA *ch, char *argument)
{
	FILE *fp;
	HELP_DATA *help;

	closeReserve();
	if ( !( fp = fopen( "help.xml", "w" ) ) )
	{
		log_string(LOG_BUG, "HSave: fopen");
		perror( "help.xml" );
	}

	fprintf(fp, "<?xml version='1.0' encoding='ISO-8859-1'?>");
	fprintf(fp, "<Help>");

	for(help = help_list; help; help = help->next)
	{
		fprintf(fp,"<help_object>");
		fprintf(fp, "<level>%d</level>\n", help->level);
		fprintf(fp, "<keyword>%s~</keyword>\n", help->keyword);
		fprintf(fp, "<race>%s~</race>\n<clan>%s~</clan>\n", help->races, help->clans);

		if(!IS_NULLSTR(help->topic))
			fprintf(fp, "<Topic>%s~</Topic>\n", help->topic);
		if(!IS_NULLSTR(help->quote))
			fprintf(fp, "<Quote>%s~</Quote>\n", help->quote);
		if(!IS_NULLSTR(help->syntax))
			fprintf(fp, "<Syntax>%s~</Syntax>\n", help->syntax);
		if(!IS_NULLSTR(help->description))
			fprintf(fp, "<Desc>%s~</Desc>\n", help->description);
		if(!IS_NULLSTR(help->see_also))
			fprintf(fp, "<See>%s~</See>\n", help->see_also);
		if(!IS_NULLSTR(help->website))
			fprintf(fp, "<Web>%s~</Web>\n", help->website);
		if(!IS_NULLSTR(help->unformatted))
			fprintf(fp, "<Unformatted>%s~</Unformatted>\n", help->unformatted);
		fprintf(fp, "</help_object>\n");
	}

	fprintf( fp, "</Help>\n" );

	fclose( fp );
	openReserve();

	send_to_char("Helps saved.\n\r", ch);
	return;
}

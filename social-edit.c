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
 *                                                                         *
 * Online Social Editting Module                                           *
 * (c) 1996, 1997 Erwin S. Andreasen <erwin@andreasen.org>                 *
 * This version contains modifications to support ROM 2.4b4                *
 *                                                                         *
 * Much of the code is the original work of Peter Fitzgerald who turned    *
 * it over to Brandon Morrison who has adopted and improved the code.      *
 * Copyright (C) 2012 - 2019                                               *
 **************************************************************************/
  
#if defined(Macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "twilight.h"
#include "db.h"
#include "tables.h"

/* #define CONST_SOCIAL  remove this in Step 2 */

#ifdef CONST_SOCIAL
#define SOCIAL_FILE "../area/social.are"
#else
#define SOCIAL_FILE "../area/social.txt"
#endif


#ifndef CONST_SOCIAL
struct social_type *social_table;	   /* and social table */

void purge_socials(void) 
{
	int x = 0;
	for( x = 0;  x < maxSocial; x++ ) 
	{
		struct social_type *social = &social_table[x];

		PURGE_DATA(social->name);
		PURGE_DATA(social->char_no_arg);
		PURGE_DATA(social->others_no_arg);
		PURGE_DATA(social->char_found);
		PURGE_DATA(social->vict_found);
		PURGE_DATA(social->char_auto);
		PURGE_DATA(social->others_auto);
	}

	// the last thing to go. (purges the social_table array);
	PURGE_DATA(social_table);
}

void load_social (FILE *fp, struct social_type *social)
{
	//strncpy(social->name, fread_string (fp), MSL);
	social->name 	   =   fread_string(fp);
	social->char_no_arg =   fread_string (fp);
	social->others_no_arg = fread_string (fp);
	social->char_found =    fread_string (fp);
	social->others_found =  fread_string (fp);
	social->vict_found =    fread_string (fp);
	social->char_auto =     fread_string (fp);
	social->others_auto =   fread_string (fp);
}

void load_social_table ()
{
	FILE *fp;
	int i;
	fp = fopen (SOCIAL_FILE, "r");

	if (!fp)
	{
		log_string(LOG_BUG, Format("Could not open " SOCIAL_FILE " for reading."));
		exit(1);
	}

	if (fscanf (fp, "%d\n", &maxSocial) == -1)
    {
        log_string(LOG_ERR,"Problem scanning for Social Files in load_social_table.");
        return;
    }

	// fscanf (fp, "%d\n", &maxSocial);

	/* IMPORTANT to use malloc so we can realloc later on */
	//	ALLOC_DATA(social_table, maxSocial+1);
	social_table = (struct social_type *)malloc (sizeof(struct social_type) * (maxSocial+1));

	for (i = 0; i < maxSocial; i++)
		load_social (fp,&social_table[i]);

	/* For backwards compatibility */

	//strncpy(social_table[maxSocial].name, NULL, MSL); /* empty! */
	social_table[maxSocial].name = NULL;
	fclose (fp);
}

#endif /* CONST_SOCIAL */

void save_social (const struct social_type *s, FILE *fp)
{
	/* get rid of (null) */
	fprintf (fp, "%s~\n%s~\n%s~\n%s~\n%s~\n%s~\n%s~\n%s~\n\n",
			       s->name 			 ? s->name          : "" , 
			       s->char_no_arg 	 ? s->char_no_arg   : "" , 
			       s->others_no_arg  ? s->others_no_arg : "" ,
			       s->char_found     ? s->char_found    : "" , 
			       s->others_found   ? s->others_found  : "" , 
			       s->vict_found     ? s->vict_found    : "" ,
			       s->char_auto      ? s->char_auto     : "" , 
			       s->others_auto    ? s->others_auto   : ""
			       );
}


void save_social_table()
{
	FILE *fp;
	int i;
	
	fp = fopen (SOCIAL_FILE, "w");
	
	if (!fp)
	{
		log_string(LOG_BUG, Format("Could not open " SOCIAL_FILE " for writing."));
		return;
	}

#ifdef CONST_SOCIAL /* If old table still in use, count socials first */
	
	for (maxSocial = 0 ; social_table[maxSocial].name[0] ; maxSocial++)
		; /* empty */
#endif	
	
	
	fprintf (fp, "%d\n", maxSocial);
	
	for ( i = 0 ; i < maxSocial ; i++)
		save_social (&social_table[i], fp);
		
	fclose (fp);
}


/* Find a social based on name */ 
int social_lookup (const char *name)
{
	int i;
	
	for (i = 0; i < maxSocial ; i++)
		if (!str_cmp(name, social_table[i].name))
			return i;
			
	return -1;
}

/*
 * Social editting command
 */
#ifndef CONST_SOCIAL
void do_socedit (CHAR_DATA *ch, char *argument)
{
	char cmd[MAX_INPUT_LENGTH]={'\0'};
	char social[MAX_INPUT_LENGTH]={'\0'};
	int iSocial;

	CheckCH(ch);
	
	smash_tilde (argument);
	
	argument = one_argument (argument,cmd);
	argument = one_argument (argument,social);
	
	if (!cmd[0])
	{
		send_to_char ("Huh? Type HELP SEDIT to see syntax.\n\r",ch);
		return;
	}
		
	if (!social[0])
	{
		send_to_char ("What social do you want to operate on?\n\r",ch);
		return;
	}
	
	iSocial = social_lookup (social);
	
	if (str_cmp(cmd,"new") && (iSocial == -1))
	{
		send_to_char ("No such social exists.\n\r",ch);
		return;
	}

	if (!str_cmp(cmd, "delete")) /* Remove a social */
	{
		int i,j;
		struct social_type *new_table = (struct social_type *)malloc (sizeof(struct social_type) * maxSocial);
		
		if (!new_table)
		{
			send_to_char ("Memory allocation failed. Brace for impact...\n\r",ch);
			return;
		}
		
		/* Copy all elements of old table into new table, except the deleted social */
		for (i = 0, j = 0; i < maxSocial+1; i++)
			if (i != iSocial) /* copy, increase only if copied */
			{
				new_table[j] = social_table[i];
				j++;
			}
	
		free (social_table);
		social_table = new_table;
		
		maxSocial--; /* Important :() */
		
		send_to_char ("That social is history now.\n\r",ch);
				
	}
	
	else if (!str_cmp(cmd, "new")) /* Create a new social */
	{
		struct social_type *new_table;
		
		if (iSocial != -1)
		{
			send_to_char ("A social with that name already exists\n\r",ch);
			return;
		}
		
		/* reallocate the table */
		/* Note that the table contains maxSocial socials PLUS one empty spot! */
		
		maxSocial++;
		new_table = (struct social_type *)realloc (social_table, sizeof(struct social_type) * (maxSocial + 1));
		
		if (!new_table) /* realloc failed */
		{
			send_to_char ("Memory allocation failed. Brace for impact.\n\r",ch);
			return;
		}
		
		social_table = new_table;
		
		strncpy(social_table[maxSocial-1].name, str_dup (social), MSL);
		social_table[maxSocial-1].char_no_arg = NULL;
		social_table[maxSocial-1].others_no_arg = NULL;
		social_table[maxSocial-1].char_found = NULL;
		social_table[maxSocial-1].others_found = NULL;
		social_table[maxSocial-1].vict_found = NULL;
		social_table[maxSocial-1].char_auto = NULL;
		social_table[maxSocial-1].others_auto = NULL;
		
		strncpy(social_table[maxSocial].name, str_dup(""), MSL); /* 'terminating' empty string */
		
		send_to_char ("New social added.\n\r",ch);
			
	}
	
	else if (!str_cmp(cmd, "show")) /* Show a certain social */
	{
		send_to_char(Format("Social: %s\n\r", social_table[iSocial].name), ch);
		send_to_char("(cnoarg) No argument given, character sees:\n\r", ch);
		send_to_char(Format("%s\n\r\n\r", social_table[iSocial].char_no_arg), ch);
		send_to_char("(onoarg) No argument given, others see:\n\r", ch);
		send_to_char(Format("%s\n\r\n\r", social_table[iSocial].others_no_arg), ch);
		send_to_char("(cfound) Target found, character sees:\n\r", ch);
		send_to_char(Format("%s\n\r\n\r", social_table[iSocial].char_found), ch);
		send_to_char("(ofound) Target found, others see:\n\r", ch);
		send_to_char(Format("%s\n\r\n\r", social_table[iSocial].others_found), ch);
		send_to_char("(vfound) Target found, victim sees:\n\r", ch);
		send_to_char(Format("%s\n\r\n\r", social_table[iSocial].vict_found), ch);
		send_to_char("(cself) Target is character himself:\n\r", ch);
		send_to_char(Format("%s\n\r\n\r", social_table[iSocial].char_auto), ch);
		send_to_char("(oself) Target is character himself, others see:\n\r", ch);
		send_to_char(Format("%s\n\r", social_table[iSocial].others_auto), ch);

		return; /* return right away, do not save the table */
	}
	
	else if (!str_cmp(cmd, "cnoarg")) /* Set that argument */
	{
		PURGE_DATA (social_table[iSocial].char_no_arg);
		social_table[iSocial].char_no_arg = str_dup(argument);

		if (!argument[0])
			send_to_char ("Character will now see nothing when this social is used without arguments.\n\r",ch);
		else
			printf_to_char (ch,"New message is now:\n\r%s\n\r", argument);
	}
	
	else if (!str_cmp(cmd, "onoarg"))
	{
		PURGE_DATA (social_table[iSocial].others_no_arg);
		social_table[iSocial].others_no_arg = str_dup(argument);

		if (!argument[0])
			send_to_char ("Others will now see nothing when this social is used without arguments.\n\r",ch);
		else
			printf_to_char (ch,"New message is now:\n\r%s\n\r", argument);
			
	}
	
	else if (!str_cmp(cmd, "cfound"))
	{
		PURGE_DATA (social_table[iSocial].char_found);
		social_table[iSocial].char_found = str_dup(argument);

		if (!argument[0])
			send_to_char ("The character will now see nothing when a target is found.\n\r",ch);
		else
			printf_to_char (ch,"New message is now:\n\r%s\n\r", argument);
			
	}
	
	else if (!str_cmp(cmd, "ofound"))
	{
		PURGE_DATA (social_table[iSocial].others_found);
		social_table[iSocial].others_found = str_dup(argument);

		if (!argument[0])
			send_to_char ("Others will now see nothing when a target is found.\n\r",ch);
		else
			printf_to_char (ch,"New message is now:\n\r%s\n\r", argument);
			
	}
	
	else if (!str_cmp(cmd, "vfound"))
	{
		PURGE_DATA (social_table[iSocial].vict_found);
		social_table[iSocial].vict_found = str_dup(argument);

		if (!argument[0])
			send_to_char ("Victim will now see nothing when a target is found.\n\r",ch);
		else
			printf_to_char (ch,"New message is now:\n\r%s\n\r", argument);
	}
	
	else if (!str_cmp(cmd, "cself"))
	{
		PURGE_DATA (social_table[iSocial].char_auto);
		social_table[iSocial].char_auto = str_dup(argument);

		if (!argument[0])
			send_to_char ("Character will now see nothing when targetting self.\n\r",ch);
		else
			printf_to_char (ch,"New message is now:\n\r%s\n\r", argument);

	}
	
	else if (!str_cmp(cmd, "oself"))
	{
		PURGE_DATA (social_table[iSocial].others_auto);
		social_table[iSocial].others_auto = str_dup(argument);

		if (!argument[0])
			send_to_char ("Others will now see nothing when character targets self.\n\r",ch);
		else
			printf_to_char (ch,"New message is now:\n\r%s\n\r", argument);
	}
	
	else
	{
		send_to_char ("Huh? Try HELP SEDIT.\n\r",ch);
		return;
	}
	
	/* We have done something. update social table */
	
	save_social_table();
}
#endif /* CONST_SOCIAL */

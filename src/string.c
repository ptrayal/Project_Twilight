/***************************************************************************
 *  File: string.c                                                         *
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


/*****************************************************************************
 Name:		string_edit
 Purpose:	Clears string and puts player into editing mode.
 Called by:	none
 ****************************************************************************/
void string_edit( CHAR_DATA *ch, char **pString )
{
	send_to_char( "-========- Entering EDIT Mode -=========-\n\r", ch );
	send_to_char( "    Type .h on a new line for help\n\r", ch );
	send_to_char( " Terminate with a ~ or @ on a blank line.\n\r", ch );
	send_to_char( "-=======================================-\n\r", ch );

	if ( *pString == NULL )
	{
		*pString = str_dup("");
	}
	else
	{
		**pString = '\0';
	}

	ch->desc->pString = pString;

	return;
}



/*****************************************************************************
 Name:		string_append
 Purpose:	Puts player into append mode for given string.
 Called by:	(many)olc_act.c
 ****************************************************************************/
void string_append( CHAR_DATA *ch, char **pString )
{
	send_to_char( "-=======- Entering APPEND Mode -========-\n\r", ch );
	send_to_char( "    Type .h on a new line for help\n\r", ch );
	send_to_char( " Terminate with a ~ or @ on a blank line.\n\r", ch );
	send_to_char( "-=======================================-\n\r", ch );

	if ( *pString == NULL )
	{
		*pString = NULL;
	}
	send_to_char( *pString, ch );

	ch->desc->pString = pString;

	return;
}



/*****************************************************************************
 Name:		string_insert_return
 Purpose:	Inserts a carriage return after a selected point in the string.
 Called by:	string_add(string.c) (aedit_builder)olc_act.c.
 ****************************************************************************/
char * string_insert_return( char * orig, char * old )
{
		char xbuf[MSL]={'\0'};
		int i = 0;

		strncpy( xbuf, orig, sizeof(xbuf) );
		if ( strstr( orig, old ) != NULL )
		{
				i = strlen( orig ) - strlen( strstr( orig, old ) );
				xbuf[i] = '\0';
				strncat( xbuf, old, sizeof(xbuf)  );
				strncat( xbuf, "\n", sizeof(xbuf)  );
				strncat( xbuf, &orig[i+strlen( old )], sizeof(xbuf)  );
				PURGE_DATA( orig );
		}

		return str_dup( xbuf );
}



/*****************************************************************************
 Name:		string_replace
 Purpose:	Substitutes one string for another.
 Called by:	string_add(string.c) (aedit_builder)olc_act.c.
 ****************************************************************************/
char * string_replace( char * orig, char * old, char * bnew )
{
		char xbuf[MSL]={'\0'};
		int i = 0;

		strncpy( xbuf, orig, sizeof(xbuf) );
		if ( strstr( orig, old ) != NULL )
		{
				i = strlen( orig ) - strlen( strstr( orig, old ) );
				xbuf[i] = '\0';
				strncat( xbuf, bnew, sizeof(xbuf)  );
				strncat( xbuf, &orig[i+strlen( old )], sizeof(xbuf)  );
				PURGE_DATA( orig );
		}

		return str_dup( xbuf );
}



/*****************************************************************************
 Name:		string_add
 Purpose:	Interpreter for string editing.
 Called by:	game_loop_xxxx(comm.c).
 ****************************************************************************/
void string_add( CHAR_DATA *ch, char *argument )
{
	char buf[MSL]={'\0'};

		/*
		 * Thanks to James Seng
		 */
		smash_tilde( argument );

		if ( *argument == '.' )
		{
				char arg1 [MAX_INPUT_LENGTH]={'\0'};
				char arg2 [MAX_INPUT_LENGTH]={'\0'};
				char arg3 [MAX_INPUT_LENGTH]={'\0'};

				argument = one_argument( argument, arg1 );
				argument = first_arg( argument, arg2, FALSE );
				argument = first_arg( argument, arg3, FALSE );

				if ( !str_cmp( arg1, ".c" ) )
				{
						send_to_char( "String cleared.\n\r", ch );
						PURGE_DATA(*ch->desc->pString);
						*ch->desc->pString = '\0';
						return;
				}

				if ( !str_cmp( arg1, ".s" ) )
				{
						send_to_char( "String so far:\n\r", ch );
						send_to_char( *ch->desc->pString, ch );
						return;
				}

				if ( !str_cmp( arg1, ".r" ) )
				{
						if ( IS_NULLSTR(arg2) )
						{
								send_to_char( "usage:  .r \"old string\" \"new string\"\n\r", ch );
								return;
						}

			smash_tilde( arg3 );   /* Just to be sure -- Hugin */
						*ch->desc->pString = string_replace( *ch->desc->pString, arg2, arg3 );
						send_to_char( Format("'%s' replaced with '%s'.\n\r", arg2, arg3), ch );
						return;
				}

				if ( !str_cmp( arg1, ".n" ) )
				{
						if ( IS_NULLSTR(arg2) )
						{
								send_to_char( "usage:  .n \"substring to follow\"\n\r", ch );
								return;
						}

						*ch->desc->pString = string_insert_return( *ch->desc->pString, arg2 );
						send_to_char( Format("'%s' is now followed by a newline.\n\r", arg2), ch );
						return;
				}

				if ( !str_cmp( arg1, ".f" ) )
				{
						*ch->desc->pString = format_string( *ch->desc->pString );
						send_to_char( "String formatted.\n\r", ch );
						return;
				}
				
				if ( !str_cmp( arg1, ".h" ) )
				{
						send_to_char( "Sedit help (commands on blank line):   \n\r", ch );
						send_to_char( ".r 'old' 'new'   - replace a substring \n\r", ch );
						send_to_char( "                   (requires '', \"\") \n\r", ch );
						send_to_char( ".n 'location'    - insert a return after substring.\n\r", ch );
						send_to_char( "                   (requires '', \"\") \n\r", ch );
						send_to_char( ".h               - get help (this info)\n\r", ch );
						send_to_char( ".s               - show string so far  \n\r", ch );
						send_to_char( ".l               - show string so far with line numbers  \n\r", ch );
						send_to_char( ".f               - (word wrap) string  \n\r", ch );
						send_to_char( ".c               - clear string so far \n\r", ch );
						send_to_char( "@                - end string          \n\r", ch );
						return;
				}
						

				send_to_char( "SEdit:  Invalid dot command.\n\r", ch );
				return;
		}

		if ( *argument == '~' || *argument == '@' || !str_cmp(argument, "END") || !str_cmp(argument, "end") )
		{
				ch->desc->pString = NULL;
				return;
		}

	if(IS_NULLSTR(ch->desc->pString))
		strcpy( buf, "");
	else	
		strcpy( buf, *ch->desc->pString );

		/*
		 * Truncate strings to MAX_STRING_LENGTH.
		 * --------------------------------------
		 */
		if ( strlen( buf ) + strlen( argument ) >= ( MSL - 4 ) )
		{
				send_to_char( "String too long, last line skipped.\n\r", ch );

	/* Force character out of editing mode. */
				ch->desc->pString = NULL;
				return;
		}

		/*
		 * Ensure no tilde's inside string.
		 * --------------------------------
		 */
		smash_tilde( argument );

		strncat( buf, argument, sizeof(buf)  );
		strncat( buf, "\n\r", sizeof(buf)  );
		PURGE_DATA( *ch->desc->pString );
		*ch->desc->pString = str_dup( buf );
		return;
}



/*
 * Thanks to Kalgen for the new procedure (no more bug!)
 * Original wordwrap() written by Surreality.
 * Modifications for color codes and blank lines by Geoff.
 */
/*****************************************************************************
 Name: format_string
 Purpose: Special string formating and word-wrapping.
 Called by: string_add(string.c) (many)olc_act.c
 ****************************************************************************/
char *format_string (char *oldstring /*, bool fSpace */ )
 {
	char xbuf[MAX_STRING_LENGTH];
	char xbuf2[MAX_STRING_LENGTH];
	char *rdesc;
	int i = 0;
	int end_of_line;
	bool cap = TRUE;
	bool bFormat = TRUE;

	xbuf[0] = xbuf2[0] = 0;

	for (rdesc = oldstring; *rdesc; rdesc++)
	{

		if (*rdesc != '`')
		{
			if (bFormat)
			{
				if (*rdesc == '\n')
				{
					if (*(rdesc + 1) == '\r' && *(rdesc + 2) == ' ' && *(rdesc + 3) == '\n' && xbuf[i - 1] != '\r')
					{
						xbuf[i] = '\n';
						xbuf[i + 1] = '\r';
						xbuf[i + 2] = '\n';
						xbuf[i + 3] = '\r';
						i += 4;
						rdesc += 2;
					}
					else if (*(rdesc + 1) == '\r' && *(rdesc + 2) == ' ' && *(rdesc + 2) == '\n' && xbuf[i - 1] == '\r')
					{
						xbuf[i] = '\n';
						xbuf[i + 1] = '\r';
						i += 2;
					}
					else if (*(rdesc + 1) == '\r' && *(rdesc + 2) == '\n' && xbuf[i - 1] != '\r')
					{
						xbuf[i] = '\n';
						xbuf[i + 1] = '\r';
						xbuf[i + 2] = '\n';
						xbuf[i + 3] = '\r';
						i += 4;
						rdesc += 1;
					}
					else if (*(rdesc + 1) == '\r' && *(rdesc + 2) == '\n' && xbuf[i - 1] == '\r')
					{
						xbuf[i] = '\n';
						xbuf[i + 1] = '\r';
						i += 2;
					}
					else if (xbuf[i - 1] != ' ' && xbuf[i - 1] != '\r')
					{
						xbuf[i] = ' ';
						i++;
					}
				}
				else if (*rdesc == '\r') ;
				else if (*rdesc == 'i' && *(rdesc + 1) == '.' && *(rdesc + 2) == 'e' && *(rdesc + 3) == '.')
				{
					xbuf[i] = 'i';
					xbuf[i + 1] = '.';
					xbuf[i + 2] = 'e';
					xbuf[i + 3] = '.';
					i += 4;
					rdesc += 3;
				}
				else if (*rdesc == ' ')
				{
					if (xbuf[i - 1] != ' ')
					{
						xbuf[i] = ' ';
						i++;
					}
				}
				else if (*rdesc == ')')
				{
					if (xbuf[i - 1] == ' ' && xbuf[i - 2] == ' '
						&& (xbuf[i - 3] == '.' || xbuf[i - 3] == '?' || xbuf[i - 3] == '!'))
					{
						xbuf[i - 2] = *rdesc;
						xbuf[i - 1] = ' ';
						xbuf[i] = ' ';
						i++;
					}
					else if (xbuf[i - 1] == ' ' && (xbuf[i - 2] == ',' || xbuf[i - 2] == ';'))
					{
						xbuf[i - 1] = *rdesc;
						xbuf[i] = ' ';
						i++;
					}
					else
					{
						xbuf[i] = *rdesc;
						i++;
					}
				}
				else if (*rdesc == ',' || *rdesc == ';')
				{
					if (xbuf[i - 1] == ' ')
					{
						xbuf[i - 1] = *rdesc;
						xbuf[i] = ' ';
						i++;
					}
					else
					{
						xbuf[i] = *rdesc;
						if (*(rdesc + 1) != '\"')
						{
							xbuf[i + 1] = ' ';
							i += 2;
						}
						else
						{
							xbuf[i + 1] = '\"';
							xbuf[i + 2] = ' ';
							i += 3;
							rdesc++;
						}
					}

				}
				else if (*rdesc == '.' || *rdesc == '?' || *rdesc == '!')
				{
					if (xbuf[i - 1] == ' ' && xbuf[i - 2] == ' '
						&& (xbuf[i - 3] == '.' || xbuf[i - 3] == '?' || xbuf[i - 3] == '!'))
					{
						xbuf[i - 2] = *rdesc;
						if (*(rdesc + 1) != '\"')
						{
							xbuf[i - 1] = ' ';
							xbuf[i] = ' ';
							i++;
						}
						else
						{
							xbuf[i - 1] = '\"';
							xbuf[i] = ' ';
							xbuf[i + 1] = ' ';
							i += 2;
							rdesc++;
						}
					}
					else
					{
						xbuf[i] = *rdesc;
						if (*(rdesc + 1) != '\"')
						{
							xbuf[i + 1] = ' ';
							xbuf[i + 2] = ' ';
							i += 3;
						}
						else
						{
							xbuf[i + 1] = '\"';
							xbuf[i + 2] = ' ';
							xbuf[i + 3] = ' ';
							i += 4;
							rdesc++;
						}
					}
					cap = TRUE;
				}
				else
				{
					xbuf[i] = *rdesc;
					if (cap)
					{
						cap = FALSE;
						xbuf[i] = UPPER (xbuf[i]);
					}
					i++;
				}
			}
			else
			{
				xbuf[i] = *rdesc;
				i++;
			}
		}
		else
		{
			if (*(rdesc + 1) == 'Z')
				bFormat = !bFormat;
			xbuf[i] = *rdesc;
			i++;
			rdesc++;
			xbuf[i] = *rdesc;
			i++;
		}
	}
	xbuf[i] = 0;
	strncpy (xbuf2, xbuf, sizeof(xbuf));

	rdesc = xbuf2;

	xbuf[0] = 0;

	for (;;)
	{
		end_of_line = 77;
		for (i = 0; i < end_of_line; i++)
		{
			if (*(rdesc + i) == '`')
			{
				end_of_line += 2;
				i++;
			}

			if (!*(rdesc + i))
				break;

			if (*(rdesc + i) == '\r')
				end_of_line = i;
		}
		if (i < end_of_line)
		{
			break;
		}
		if (*(rdesc + i - 1) != '\r')
		{
			for (i = (xbuf[0] ? (end_of_line - 1) : (end_of_line - 4)); i; i--)
			{
				if (*(rdesc + i) == ' ')
					break;
			}
			if (i)
			{
				*(rdesc + i) = 0;
				strncat (xbuf, rdesc, sizeof(xbuf));
				strncat (xbuf, "\n\r", sizeof(xbuf));
				rdesc += i + 1;
				while (*rdesc == ' ')
					rdesc++;
			}
			else
			{
				log_string(LOG_BUG, "`5Wrap_string: `@No spaces``");
				*(rdesc + (end_of_line - 2)) = 0;
				strncat (xbuf, rdesc, sizeof(xbuf));
				strncat (xbuf, "-\n\r", sizeof(xbuf));
				rdesc += end_of_line - 1;
			}
		}
		else
		{
			*(rdesc + i - 1) = 0;
			strncat (xbuf, rdesc, sizeof(xbuf));
			strncat (xbuf, "\r", sizeof(xbuf));
			rdesc += i;
			while (*rdesc == ' ')
				rdesc++;
		}
	}
	while (*(rdesc + i) && (*(rdesc + i) == ' ' ||
		*(rdesc + i) == '\n' ||
		*(rdesc + i) == '\r'))
		i--;
	*(rdesc + i + 1) = 0;
	strncat (xbuf, rdesc, sizeof(xbuf));
	if (xbuf[strlen (xbuf) - 2] != '\n')
		strncat (xbuf, "\n\r", sizeof(xbuf));

	PURGE_DATA(oldstring);
	return (str_dup (xbuf));
}
 



/*
 * Used above in string_add.  Because this function does not
 * modify case if fCase is FALSE and because it understands
 * parenthesis, it would probably make a nice replacement
 * for one_argument.
 */
/*****************************************************************************
 Name:		first_arg
 Purpose:	Pick off one argument from a string and return the rest.
		Understands quates, parenthesis (barring ) ('s) and
		percentages.
 Called by:	string_add(string.c)
 ****************************************************************************/
char *first_arg( char *argument, char *arg_first, bool fCase )
{
		char cEnd;

		while ( *argument == ' ' )
	argument++;

		cEnd = ' ';
		if ( *argument == '\'' || *argument == '"'
			|| *argument == '%'  || *argument == '(' )
		{
				if ( *argument == '(' )
				{
						cEnd = ')';
						argument++;
				}
				else cEnd = *argument++;
		}

		while ( *argument != '\0' )
		{
	if ( *argument == cEnd )
	{
			argument++;
			break;
	}
		if ( fCase ) *arg_first = LOWER(*argument);
						else *arg_first = *argument;
	arg_first++;
	argument++;
		}
		*arg_first = '\0';

		while ( *argument == ' ' )
	argument++;

		return argument;
}




/*
 * Used in olc_act.c for aedit_builders.
 */
char * string_unpad( char * argument )
{
	char buf[MSL]={'\0'};
	char *s;

		s = argument;

		while ( *s == ' ' )
				s++;

		strncpy( buf, s, sizeof(buf) );
		s = buf;

		if ( *s != '\0' )
		{
				while ( *s != '\0' )
						s++;
				s--;

				while( *s == ' ' )
						s--;
				s++;
				*s = '\0';
		}

		PURGE_DATA( argument );
		return str_dup( buf );
}



/*
 * Same as capitalize but changes the pointer's data.
 * Used in olc_act.c in aedit_builder.
 */
char * string_proper( char * argument )
{
		char *s;

		s = argument;

		while ( *s != '\0' )
		{
				if ( *s != ' ' )
				{
						*s = UPPER(*s);
						while ( *s != ' ' && *s != '\0' )
								s++;
				}
				else
				{
						s++;
				}
		}

		return argument;
}

/*
 *  Thanks to Rayal for pretty strings :}
 */
void pretty_proc( char *buf, char *word )
{
	static char *pbuf;
	static int index;
	int i = 0;

	/* special cue to do inits */
	if( word == NULL)
	{
		pbuf = buf;
		index = 0;
		return;
	}

	/* forced newline */
	if( !str_cmp( "\n\r", word ) )
	{
		/* strip trailing spaces */
		for( i = strlen(pbuf) - 1; i >= 0 && pbuf[i] == ' '; i--)
			pbuf[i] = 0;
		strncat( pbuf, word, sizeof(pbuf)  );
		index = 0;
		return;
	}

	/* see if it's a soft space */
	if( !str_cmp( " ", word ) )
	{
		if(index == 0)
			return;
	}

	/* add a word */
	if( strlen(word) > LINE_LENGTH )
		word[LINE_LENGTH] = 0;
	if( strlen(word) + index > LINE_LENGTH )
	{
		/* strip trailing spaces */
		for( i = strlen(pbuf) - 1; i >= 0 && pbuf[i] == ' '; i--)
			pbuf[i] = 0;
		strncat(pbuf,"\n\r", sizeof(pbuf) );
		index = 0;
		while(*word == ' ')
			word++;
	}
	strncat( pbuf, word, sizeof(pbuf)  );
	index += strlen(word);
}


char *desc_pretty( char *string, int start, int lines, bool no_free )
{
	char buf[MSL]={'\0'};
	char wordbuf[MAX_INPUT_LENGTH]={'\0'};
	char *p, *bp, *wp;
	int i = 0,inword = 0;
	/* find starting line to pretty-ify */
	for( i = 1, p = string, bp = buf; *p != 0 && i < start; p++)
	{
		*bp++ = *p;
		if( *p == '\r' )
			i++;
	}
	*bp = 0;
	/* now build pretty lines from raw ones */
	pretty_proc( bp, NULL );            /* init pretty processor */
	for( i = inword = 0, wp = wordbuf; i < lines && *p != 0; p++ )
	{
		if( *p == ' ' )
		{
			if( inword )
			{
				inword = 0;
				*wp = 0;
				pretty_proc( NULL, wordbuf );
				wp = wordbuf;
			}
			*wp++ = ' ';
		}
		else if( *p == '\r' )
		{
			i++;                /* inc line count */
			if( inword )
			{
				inword = 0;
				*wp = 0;
				pretty_proc( NULL, wordbuf );
				wp = wordbuf;
				if( p[1] == '\n' || p[1] == ' ' || p[1] == 0)
					pretty_proc( NULL, "\n\r" );
				else
					pretty_proc( NULL, " " );
			}
			else
			{
				pretty_proc( NULL, "\n\r" );
				wp = wordbuf;
			}
		}
		else if( *p == '\n' )
			continue;
		else {
			inword = 1;
			*wp++ = *p;
		}
	}
	/* and append any leftover lines directly */
	strncat( buf, p, sizeof(buf)  );
	/* and swap in the new editted description */
	if(no_free)
	{
		snprintf(string, sizeof(string), "%s", buf);
		return string;
	}
	PURGE_DATA( string );
	return str_dup( buf );
}

char *get_line( char *str, char *buf )
{
	int tmp = 0;
	bool found = FALSE;

	while ( *str )
	{
		if ( *str == '\n' )
		{
			found = TRUE;
			break;
		}

		buf[tmp++] = *(str++);
	}

	if ( found )
	{
		if ( *(str + 1) == '\r' )
			str += 2;
		else
			str += 1;
	} /* para que quedemos en el inicio de la prox linea */

	buf[tmp] = '\0';

	return str;
}

char *numlineas( char *string )
{
	static char buf[MAX_STRING_LENGTH*2];
	char buf2[MSL]={'\0'};
	char tmpb[MSL]={'\0'};
	int cnt = 1;
	
	while ( *string )
	{
		string = get_line( string, tmpb );
		sprintf( buf2, "%2d. %s\n\r", cnt++, tmpb );
		strcat( buf, buf2 );
	}

	return buf;
}

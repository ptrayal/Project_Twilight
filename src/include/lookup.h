/***************************************************************************
 * Much of the code is the original work of Peter Fitzgerald who turned    *
 * it over to Brandon Morrison who has adopted and improved the code.      *
 * Copyright (C) 2012 - 2019                                               *
 **************************************************************************/

#ifndef LOOKUP_H
#define LOOKUP_H

int	position_lookup	args( (const char *name) );
int 	sex_lookup	args( (const char *name) );
int 	size_lookup	args( (const char *name) );
int flag_lookup args( (const char *name, const struct flag_type *flag_table) );
int flag_lookup2 args( (const char *name, const struct flag_type *flag_table) );
int	job_lookup	args( (char *name) );
int	job_cmd_lookup args( ( char *name, void *table) );
int	office_lookup	args( (char *name) );
int	influence_cmd_lookup	args( (char *name) );
int	bg_cmd_lookup	args( (char *name) );
int	influence_lookup	args( (char *name) );
int	background_lookup	args( (char *name) );
int	news_cmd_lookup	args( (char *name) );
NEWSPAPER *newspaper_lookup args( (char *name) );
STOCKS *stock_lookup args( (char *name) );
ORG_DATA *org_lookup args( (char *name) );
ORGMEM_DATA *mem_lookup args( (ORG_DATA *org, char *name) );
PACK_DATA *pack_lookup args( (char *name) );
int	gift_lookup	args( (char *name) );
bool CAN_USE_CMD(CHAR_DATA *ch, int cmd);
int command_lookup(char *command);
int command_available(CHAR_DATA *ch, char *command);
int smarket_cmd_lookup (char *name);
long group_gift_lookup (char *name, int level);
int	home_cmd_lookup	args( (char *name) );
int	home_price_lookup args( (char *name) );
char * colour_lookup (char name, bool colour);
int trait_lookup(char *name, const struct trait_struct *trait_table);
int rite_lookup(CHAR_DATA *ch);
int riteaction_lookup(char *name);

#endif
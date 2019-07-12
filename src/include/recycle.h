/***************************************************************************
 * Much of the code is the original work of Peter Fitzgerald who turned    *
 * it over to Brandon Morrison who has adopted and improved the code.      *
 * Copyright (C) 2012 - 2019                                               *
 **************************************************************************/

#ifndef RECYCLE_H
#define RECYCLE_H

/* externs */
extern char str_empty[1];
extern int mobile_count;

/* stuff for providing a crash-proof buffer */

#define MAX_BUF		16384
#define MAX_BUF_LIST 	10
#define BASE_BUF 	1024

/* valid states */
#define BUFFER_SAFE	0
#define BUFFER_OVERFLOW	1
#define BUFFER_FREED 	2


/* note recycling */
#define ND NOTE_DATA
ND	*new_note args( (void) );
void	free_note args( (NOTE_DATA *note) );
#undef ND

/* news recycling */
#define NP NEWSPAPER
NP	*new_newspaper args( (void) );
void	free_newspaper args( (NEWSPAPER *news) );
#undef NP

/* stock recycling */
#define STO STOCKS
STO	*new_stock args( (void) );
void	free_stock args( (STO *news) );
#undef STO

/* ban data recycling */
#define BD BAN_DATA
BD	*new_ban args( (void) );
void	free_ban args( (BAN_DATA *ban) );
#undef BD

/* combat data recycling */
#define CD COMBAT_DATA
CD	*new_combat_move args( (void) );
void	free_combat_move args( (COMBAT_DATA *move) );
#undef CD

/* descriptor recycling */
#define DD DESCRIPTOR_DATA
DD	*new_descriptor args( (void) );
void	free_descriptor args( (DESCRIPTOR_DATA *d) );
#undef DD

/* char gen data recycling */
#define GD GEN_DATA
GD 	*new_gen_data args( (void) );
void	free_gen_data args( (GEN_DATA * gen) );
#undef GD

/* extra descr recycling */
#define ED EXTRA_DESCR_DATA
ED	*new_extra_descr args( (void) );
void	free_extra_descr args( (EXTRA_DESCR_DATA *ed) );
#undef ED

/* affect recycling */
#define AD AFFECT_DATA
AD	*new_affect args( (void) );
void	free_affect args( (AFFECT_DATA *af) );
#undef AD

/* object recycling */
#define OD OBJ_DATA
OD	*new_obj args( (void) );
void	free_obj args( (OBJ_DATA *obj) );
#undef OD

/* character recyling */
#define CD CHAR_DATA
#define PD PC_DATA
CD	*new_char args( (void) );
void	free_char args( (CHAR_DATA *ch) );
PD	*new_pcdata args( (void) );
void	free_pcdata args( (PC_DATA *pcdata) );
#undef PD
#undef CD

/* org recycling */
#define OD ORG_DATA
OD	*new_org args( (void) );
void	free_org args( (ORG_DATA *org) );
#undef OD

/* orgmem recycling */
#define OD ORGMEM_DATA
OD	*new_orgmem args( (void) );
void	free_orgmem args( (ORGMEM_DATA *mem) );
#undef OD

/* pack recycling */
#define PD PACK_DATA
PD	*new_pack args( (void) );
void	free_pack args( (PACK_DATA *pack) );
#undef PD

/* mob id and memory procedures */
#define MD MEMORY
MD	*new_memory args( (void) );
long	get_mob_id  args( (void) );
long	get_pc_id   args( (void) );
#undef MD

/* buffer procedures */
BUFFER	*__new_buf args( (const char *file, const char *function, int line) );
#define new_buf() __new_buf(__FILE__, __FUNCTION__, __LINE__)
void BufPrintf args(( BUFFER * buffer, char * fmt, ... ));

void	free_buf args( (BUFFER *buffer) );
bool	add_buf args( (BUFFER *buffer, char *string) );
void	clear_buf args( (BUFFER *buffer) );
char	*buf_string args( (BUFFER *buffer) );

/* trait procedures */
TRAIT_DATA *new_trait();
void free_trait(TRAIT_DATA *trait);

/* liquid procedures */
LIQUID_DATA *new_liqdata();
void free_liquid(LIQUID_DATA *liquid);

/* mprog recycling */
#define MP MPROG_LIST
MP	*new_mprog args( (void) );
void	free_mprog args( (MP *mprog) );
#undef MP

/* mpcode recycling */
#define MPC MPROG_CODE
MPC	*new_mpcode args( (void) );
void	free_mpcode args( (MPC *mpcode) );
#undef MPC

#endif
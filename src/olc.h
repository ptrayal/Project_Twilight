/***************************************************************************
 *  File: olc.h                                                            *
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
/*
 * This is a header file for all the OLC files.  Feel free to copy it into
 * merc.h if you wish.  Many of these routines may be handy elsewhere in
 * the code.  -Jason Dinkel
 */


/*
 * The version info.  Please use this info when reporting bugs.
 * It is displayed in the game by typing 'version' while editing.
 * Do not remove these from the code - by request of Jason Dinkel
 */
#define VERSION	"ILAB Online Creation [Beta 1.0, ROM 2.3 modified]\n\r" \
		"     Port a ROM 2.4 v1.00\n\r" \
		"     Port a PT 1.5 v1.00\n\r"
#define AUTHOR	"     By Jason(jdinkel@mines.colorado.edu)\n\r" \
                "     Modified for use with ROM 2.3\n\r"        \
                "     By Hans Birkeland (hansbi@ifi.uio.no)\n\r" \
                "     Modificado para uso en ROM 2.4v4\n\r"	\
                "     Por Birdie (itoledo@ramses.centic.utem.cl)\n\r" \
		"     Modified for use with Project Twilight 1.5\n\r" \
		"     By Peter Fitzgerald (pfitzger@mcs.une.edu.au)\n\r"
#define DATE	"     (Apr. 7, 1995 - ROM mod, Apr 16, 1995)\n\r" \
		"     (Port a ROM 2.4 - Nov 2, 1996)\n\r"  \
		"     (Port to PTv1.0 - Dec 27, 2000)\n\r"
#define CREDITS "     Original by Surreality(cxw197@psu.edu) and Locke(locke@lm.com)"

#define OLC_VERSION "1.00"



/*
 * New typedefs.
 */
typedef	bool OLC_FUN		args( ( CHAR_DATA *ch, char *argument ) );
#define DECLARE_OLC_FUN( fun )	OLC_FUN    fun


/* Command procedures needed ROM OLC */
DECLARE_DO_FUN(    do_help    );
DECLARE_SPELL_FUN( spell_null );





/*
 * Connected states for editor.
 */
#define ED_AREA 1
#define ED_ROOM 2
#define ED_OBJECT 3
#define ED_MOBILE 4
#define ED_PLOT 5
#define ED_EVENT 6
#define ED_SCRIPT 7
#define ED_REACT 8
#define ED_PERSONA 9
#define ED_HELP 10
#define ED_KNOW 11
#define ED_BG   12
#define ED_TIP   13
#define ED_MPCODE   14

/*
 * Interpreter Prototypes
 */
void    aedit           args( ( CHAR_DATA *ch, char *argument ) );
void    redit           args( ( CHAR_DATA *ch, char *argument ) );
void    medit           args( ( CHAR_DATA *ch, char *argument ) );
void    oedit           args( ( CHAR_DATA *ch, char *argument ) );
void    hedit           args( ( CHAR_DATA *ch, char *argument, int type ) );
void    mpedit          args( ( CHAR_DATA *ch, char *argument ) );

/*
 * Interpreter Prototypes for Storyteller module editors
 */
void    pedit           args( ( CHAR_DATA *ch, char *argument ) );
void    eedit           args( ( CHAR_DATA *ch, char *argument ) );
void    sedit           args( ( CHAR_DATA *ch, char *argument ) );
void    kbedit           args( ( CHAR_DATA *ch, char *argument, int type ) );

/*
 * Interpreter Prototypes for Artificial Intelligence module editors
 */
void    aiedit           args( ( CHAR_DATA *ch, char *argument ) );
void    rsedit           args( ( CHAR_DATA *ch, char *argument ) );


/*
 * OLC Constants
 */
#define MAX_MOB	1		/* Default maximum number for resetting mobs */



/*
 * Structure for an OLC editor command.
 */
struct olc_cmd_type
{
    char * const	name;
    OLC_FUN *		olc_fun;
};



/*
 * Structure for an OLC editor startup command.
 */
struct	editor_cmd_type
{
    char * const	name;
    DO_FUN *		do_fun;
};



/*
 * Utils.
 */
AREA_DATA *get_vnum_area	args ( ( int vnum ) );
AREA_DATA *get_area_data	args ( ( int vnum ) );
int flag_value			args ( ( const struct flag_type *flag_table,
				         char *argument) );
void add_reset			args ( ( ROOM_INDEX_DATA *room, 
				         RESET_DATA *pReset, int index ) );
char *flag_string       args ( (const struct flag_type *flag_table,int bits) );
int get_stop		args ( (ROOM_INDEX_DATA *car, ROOM_INDEX_DATA *room) );


/*
 * Interpreter Table Prototypes
 */
extern const struct olc_cmd_type	aedit_table[];
extern const struct olc_cmd_type	redit_table[];
extern const struct olc_cmd_type	oedit_table[];
extern const struct olc_cmd_type	medit_table[];
extern const struct olc_cmd_type	pedit_table[];
extern const struct olc_cmd_type	eedit_table[];
extern const struct olc_cmd_type	sedit_table[];
extern const struct olc_cmd_type	aiedit_table[];
extern const struct olc_cmd_type	rsedit_table[];
extern const struct olc_cmd_type	hedit_table[];
extern const struct olc_cmd_type	kbedit_table[];
extern const struct olc_cmd_type	mpedit_table[];


/*
 * Editor Commands.
 */
DECLARE_DO_FUN( do_aedit        );
DECLARE_DO_FUN( do_redit        );
DECLARE_DO_FUN( do_oedit        );
DECLARE_DO_FUN( do_medit        );
DECLARE_DO_FUN( do_pedit        );
DECLARE_DO_FUN( do_eedit        );
DECLARE_DO_FUN( do_sedit        );
DECLARE_DO_FUN( do_aiedit        );
DECLARE_DO_FUN( do_rsedit        );
DECLARE_DO_FUN( do_hedit         );
DECLARE_DO_FUN( do_know          );
DECLARE_DO_FUN( do_bg            );
DECLARE_DO_FUN( do_tipedit       );
DECLARE_DO_FUN( do_mpedit       );



/*
 * General Functions
 */
bool show_commands		args ( ( CHAR_DATA *ch, char *argument ) );
bool show_help			args ( ( CHAR_DATA *ch, char *argument ) );
bool edit_done			args ( ( CHAR_DATA *ch ) );
bool show_version		args ( ( CHAR_DATA *ch, char *argument ) );



/*
 * Area Editor Prototypes
 */
DECLARE_OLC_FUN( aedit_show		);
DECLARE_OLC_FUN( aedit_create		);
DECLARE_OLC_FUN( aedit_name		);
DECLARE_OLC_FUN( aedit_file		);
DECLARE_OLC_FUN( aedit_age		);
DECLARE_OLC_FUN( aedit_reset		);
DECLARE_OLC_FUN( aedit_security		);
DECLARE_OLC_FUN( aedit_builder		);
DECLARE_OLC_FUN( aedit_vnum		);
DECLARE_OLC_FUN( aedit_lvnum		);
DECLARE_OLC_FUN( aedit_uvnum		);
DECLARE_OLC_FUN( aedit_credits		);
DECLARE_OLC_FUN( aedit_flags		);


/*
 * Room Editor Prototypes
 */
DECLARE_OLC_FUN( redit_show		);
DECLARE_OLC_FUN( redit_sector		);
DECLARE_OLC_FUN( redit_create		);
DECLARE_OLC_FUN( redit_name		);
DECLARE_OLC_FUN( redit_uname		);
DECLARE_OLC_FUN( redit_dname		);
DECLARE_OLC_FUN( redit_desc		);
DECLARE_OLC_FUN( redit_udesc		);
DECLARE_OLC_FUN( redit_ddesc		);
DECLARE_OLC_FUN( redit_ed		);
DECLARE_OLC_FUN( redit_flags		);
DECLARE_OLC_FUN( redit_format		);
DECLARE_OLC_FUN( redit_north		);
DECLARE_OLC_FUN( redit_south		);
DECLARE_OLC_FUN( redit_east		);
DECLARE_OLC_FUN( redit_west		);
DECLARE_OLC_FUN( redit_up		);
DECLARE_OLC_FUN( redit_down		);
DECLARE_OLC_FUN( redit_nearnorth	);
DECLARE_OLC_FUN( redit_nearsouth	);
DECLARE_OLC_FUN( redit_neareast		);
DECLARE_OLC_FUN( redit_nearwest		);
DECLARE_OLC_FUN( redit_nearup		);
DECLARE_OLC_FUN( redit_neardown		);
DECLARE_OLC_FUN( redit_mreset		);
DECLARE_OLC_FUN( redit_oreset		);
DECLARE_OLC_FUN( redit_mlist		);
DECLARE_OLC_FUN( redit_rlist		);
DECLARE_OLC_FUN( redit_olist		);
DECLARE_OLC_FUN( redit_mshow		);
DECLARE_OLC_FUN( redit_oshow		);
DECLARE_OLC_FUN( redit_heal		);
DECLARE_OLC_FUN( redit_mana		);
DECLARE_OLC_FUN( redit_clan		);
DECLARE_OLC_FUN( redit_stop		);


/*
 * Object Editor Prototypes
 */
DECLARE_OLC_FUN( oedit_show		);
DECLARE_OLC_FUN( oedit_create		);
DECLARE_OLC_FUN( oedit_name		);
DECLARE_OLC_FUN( oedit_short		);
DECLARE_OLC_FUN( oedit_long		);
DECLARE_OLC_FUN( oedit_desc		);
DECLARE_OLC_FUN( oedit_addaffect	);
DECLARE_OLC_FUN( oedit_delaffect	);
DECLARE_OLC_FUN( oedit_value0		);
DECLARE_OLC_FUN( oedit_value1		);
DECLARE_OLC_FUN( oedit_value2		);
DECLARE_OLC_FUN( oedit_value3		);
DECLARE_OLC_FUN( oedit_value4		);  /* ROM */
DECLARE_OLC_FUN( oedit_weight		);
DECLARE_OLC_FUN( oedit_cost		);
DECLARE_OLC_FUN( oedit_ed		);
DECLARE_OLC_FUN( oedit_fetish		);
DECLARE_OLC_FUN( oedit_fetlevel		);

DECLARE_OLC_FUN( oedit_extra            );  /* ROM */
DECLARE_OLC_FUN( oedit_extra2           );  /* PT */
DECLARE_OLC_FUN( oedit_wear             );  /* ROM */
DECLARE_OLC_FUN( oedit_type             );  /* ROM */
DECLARE_OLC_FUN( oedit_affect           );  /* ROM */
DECLARE_OLC_FUN( oedit_material		);  /* ROM */
DECLARE_OLC_FUN( oedit_level            );  /* ROM */
DECLARE_OLC_FUN( oedit_condition        );  /* ROM */
DECLARE_OLC_FUN( oedit_liqlist		);
DECLARE_OLC_FUN( oedit_use_strings      );
DECLARE_OLC_FUN( oedit_quality          );
DECLARE_OLC_FUN( oedit_company          );

/*
 * Mobile Editor Prototypes
 */
DECLARE_OLC_FUN( medit_show		);
DECLARE_OLC_FUN( medit_create		);
DECLARE_OLC_FUN( medit_name		);
DECLARE_OLC_FUN( medit_short		);
DECLARE_OLC_FUN( medit_long		);
DECLARE_OLC_FUN( medit_shop		);
DECLARE_OLC_FUN( medit_desc		);
DECLARE_OLC_FUN( medit_level		);
DECLARE_OLC_FUN( medit_align		);
DECLARE_OLC_FUN( medit_spec		);
DECLARE_OLC_FUN( medit_triggers		);

DECLARE_OLC_FUN( medit_sex		);
DECLARE_OLC_FUN( medit_act		);
DECLARE_OLC_FUN( medit_act2		);
DECLARE_OLC_FUN( medit_affect		);
DECLARE_OLC_FUN( medit_ac		);
DECLARE_OLC_FUN( medit_form		);
DECLARE_OLC_FUN( medit_part		);
DECLARE_OLC_FUN( medit_imm		);
DECLARE_OLC_FUN( medit_res		);
DECLARE_OLC_FUN( medit_vuln		);
DECLARE_OLC_FUN( medit_material		);
DECLARE_OLC_FUN( medit_off		);
DECLARE_OLC_FUN( medit_size		);
DECLARE_OLC_FUN( medit_hitdice		);
DECLARE_OLC_FUN( medit_damdice		);
DECLARE_OLC_FUN( medit_race		);
DECLARE_OLC_FUN( medit_position		);
DECLARE_OLC_FUN( medit_gold		);
DECLARE_OLC_FUN( medit_hitroll		);
DECLARE_OLC_FUN( medit_damtype		);
DECLARE_OLC_FUN( medit_stat		);
DECLARE_OLC_FUN( medit_addmprog		);
DECLARE_OLC_FUN( medit_delmprog		);

/* Mobprog editor */
DECLARE_OLC_FUN( mpedit_create          );
DECLARE_OLC_FUN( mpedit_code            );
DECLARE_OLC_FUN( mpedit_show            );
DECLARE_OLC_FUN( mpedit_list            );

/*
 * Help Editor Prototypes
 */
DECLARE_OLC_FUN( hedit_keyword		);
DECLARE_OLC_FUN( hedit_trust		);
DECLARE_OLC_FUN( hedit_body		);
DECLARE_OLC_FUN( hedit_create		);
DECLARE_OLC_FUN( hedit_remove		);
DECLARE_OLC_FUN( hedit_show		);
DECLARE_OLC_FUN( hedit_create		);
DECLARE_OLC_FUN( hedit_keyword		);
DECLARE_OLC_FUN( hedit_trust		);
DECLARE_OLC_FUN( hedit_races		);
DECLARE_OLC_FUN( hedit_clans		);
DECLARE_OLC_FUN( hedit_topic		);
DECLARE_OLC_FUN( hedit_quote		);
DECLARE_OLC_FUN( hedit_syntax		);
DECLARE_OLC_FUN( hedit_website		);
DECLARE_OLC_FUN( hedit_see_also		);
DECLARE_OLC_FUN( hedit_no_format	);

/*
 * Tip Editor Prototype (Otherwise uses hedits)
 */
DECLARE_OLC_FUN( tipedit_create		);

/*
 * Plot Editor Prototypes
 */
DECLARE_OLC_FUN( pedit_list		);
DECLARE_OLC_FUN( pedit_show		);
DECLARE_OLC_FUN( pedit_create		);
DECLARE_OLC_FUN( pedit_remove		);
DECLARE_OLC_FUN( pedit_addevent		);
DECLARE_OLC_FUN( pedit_delevent		);
DECLARE_OLC_FUN( pedit_assignevent	);
DECLARE_OLC_FUN( pedit_races		);
DECLARE_OLC_FUN( pedit_title		);
DECLARE_OLC_FUN( pedit_save		);

/*
 * Event Editor Prototypes
 */
DECLARE_OLC_FUN( eedit_list		);
DECLARE_OLC_FUN( eedit_show		);
DECLARE_OLC_FUN( eedit_create		);
DECLARE_OLC_FUN( eedit_remove		);
DECLARE_OLC_FUN( eedit_races		);
DECLARE_OLC_FUN( eedit_title		);
DECLARE_OLC_FUN( eedit_actors		);
DECLARE_OLC_FUN( eedit_scripts		);
DECLARE_OLC_FUN( eedit_delscript	);
DECLARE_OLC_FUN( eedit_save		);
DECLARE_OLC_FUN( eedit_location		);

/*
 * Script Editor Prototypes
 */
DECLARE_OLC_FUN( sedit_list		);
DECLARE_OLC_FUN( sedit_show		);
DECLARE_OLC_FUN( sedit_trigger		);
DECLARE_OLC_FUN( sedit_reaction		);
DECLARE_OLC_FUN( sedit_create		);
DECLARE_OLC_FUN( sedit_remove		);
DECLARE_OLC_FUN( sedit_delay		);
DECLARE_OLC_FUN( sedit_actor		);
DECLARE_OLC_FUN( sedit_event		);
DECLARE_OLC_FUN( sedit_eventfirst	);
DECLARE_OLC_FUN( sedit_save		);

/*
 * Personality Editor Prototypes.
 */
DECLARE_OLC_FUN( aiedit_create		);
DECLARE_OLC_FUN( aiedit_remove		);
DECLARE_OLC_FUN( aiedit_show		);
DECLARE_OLC_FUN( aiedit_name		);
DECLARE_OLC_FUN( aiedit_trigger		);

/*
 * AI Script Editor Prototypes.
 */
DECLARE_OLC_FUN( rsedit_create		);
DECLARE_OLC_FUN( rsedit_remove		);
DECLARE_OLC_FUN( rsedit_trigger		);
DECLARE_OLC_FUN( rsedit_reaction	);
DECLARE_OLC_FUN( rsedit_show		);
DECLARE_OLC_FUN( rsedit_attitude	);
DECLARE_OLC_FUN( rsedit_persona		);

/*
 * Note Editor Prototypes.
 */
DECLARE_OLC_FUN( kbedit_keyword		);
DECLARE_OLC_FUN( kbedit_diff		);
DECLARE_OLC_FUN( kbedit_body		);
DECLARE_OLC_FUN( kbedit_create		);
DECLARE_OLC_FUN( kbedit_remove		);
DECLARE_OLC_FUN( kbedit_show		);
DECLARE_OLC_FUN( kbedit_races		);
DECLARE_OLC_FUN( kbedit_successes	);
DECLARE_OLC_FUN( bedit_create		);
DECLARE_OLC_FUN( kedit_create		);


/*
 * Macros
 */

#define IS_STORYTELLER(ch, bog) ( !IS_SWITCHED( ch ) &&			  \
				( ch->trust >= 3			  \
				|| strstr( bog->author, ch->name )	  \
				|| strstr( bog->author, "All" ) ) )

#define TOGGLE_BIT(var, bit)    ((var) ^= (bit))

/* Return pointers to what is being edited. */
#define EDIT_MOB(Ch, Mob)	( Mob = (MOB_INDEX_DATA *)Ch->desc->pEdit )
#define EDIT_OBJ(Ch, Obj)	( Obj = (OBJ_INDEX_DATA *)Ch->desc->pEdit )
#define EDIT_ROOM(Ch, Room)	( Room = Ch->in_room )
#define EDIT_AREA(Ch, Area)	( Area = (AREA_DATA *)Ch->desc->pEdit )
#define EDIT_PLOT(Ch, Plot)	( Plot = (PLOT_DATA *)Ch->desc->pEdit )
#define EDIT_EVENT(Ch, Event)	( Event = (EVENT_DATA *)Ch->desc->pEdit )
#define EDIT_SCRIPT(Ch, Script)	( Script = (SCRIPT_DATA *)Ch->desc->pEdit )
#define EDIT_PERSONA(Ch, perso)	( perso = (PERSONA *)Ch->desc->pEdit )
#define EDIT_REACT(Ch, react)	( react = (REACT *)Ch->desc->pEdit )
#define EDIT_HELP(Ch, help)	( help = (HELP_DATA *)Ch->desc->pEdit )
#define EDIT_NOTE(Ch, note)	( note = (NOTE_DATA *)Ch->desc->pEdit )
#define EDIT_MPCODE(Ch, Code)   ( Code = (MPROG_CODE*)Ch->desc->pEdit )





/*
 * Prototypes
 */
/* mem.c - memory prototypes. */
#define ED	EXTRA_DESCR_DATA
RESET_DATA	*new_reset_data		args ( ( void ) );
void		free_reset_data		args ( ( RESET_DATA *pReset ) );
AREA_DATA	*new_area		args ( ( void ) );
void		free_area		args ( ( AREA_DATA *pArea ) );
EXIT_DATA	*new_exit		args ( ( void ) );
void		free_exit		args ( ( EXIT_DATA *pExit ) );
ED 		*new_extra_descr	args ( ( void ) );
void		free_extra_descr	args ( ( ED *pExtra ) );
ROOM_INDEX_DATA *new_room_index		args ( ( void ) );
void		free_room_index		args ( ( ROOM_INDEX_DATA *pRoom ) );
AFFECT_DATA	*new_affect		args ( ( void ) );
void		free_affect		args ( ( AFFECT_DATA* pAf ) );
SHOP_DATA	*new_shop		args ( ( void ) );
void		free_shop		args ( ( SHOP_DATA *pShop ) );
OBJ_INDEX_DATA	*new_obj_index		args ( ( void ) );
void		free_obj_index		args ( ( OBJ_INDEX_DATA *pObj ) );
MOB_INDEX_DATA	*new_mob_index		args ( ( void ) );
void		free_mob_index		args ( ( MOB_INDEX_DATA *pMob ) );
PLOT_DATA	*new_plot		args ( ( void ) );
void		free_plot		args ( ( PLOT_DATA *pPlot ) );
EVENT_DATA	*new_event		args ( ( void ) );
void		free_event		args ( ( EVENT_DATA *pEvent ) );
SCRIPT_DATA	*new_script		args ( ( void ) );
void		free_script		args ( ( SCRIPT_DATA *pScript ) );
ACTOR_DATA	*new_actor		args ( ( void ) );
void		free_actor		args ( ( ACTOR_DATA *pActor ) );
REACT		*new_react		args ( ( void ) );
void		free_react		args ( ( REACT *pMatrix ) );
REACT_DATA	*new_react_data		args ( ( void ) );
void		free_react_data		args ( ( REACT_DATA *pData ) );
PERSONA		*new_persona		args ( ( void ) );
void		free_persona		args ( ( PERSONA *pPersona ) );
HELP_DATA	*new_help		args ( ( void ) );
void		free_help		args ( ( HELP_DATA *help ) );
#undef	ED
